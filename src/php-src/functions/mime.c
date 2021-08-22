/*
   +----------------------------------------------------------------------+
   | PHP HTML Embedded Scripting Language Version 3.0                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2000 PHP Development Team (See Credits file)      |
   +----------------------------------------------------------------------+
   | This program is free software; you can redistribute it and/or modify |
   | it under the terms of one of the following licenses:                 |
   |                                                                      |
   |  A) the GNU General Public License as published by the Free Software |
   |     Foundation; either version 2 of the License, or (at your option) |
   |     any later version.                                               |
   |                                                                      |
   |  B) the PHP License as published by the PHP Development Team and     |
   |     included in the distribution in the file: LICENSE                |
   |                                                                      |
   | This program is distributed in the hope that it will be useful,      |
   | but WITHOUT ANY WARRANTY; without even the implied warranty of       |
   | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |
   | GNU General Public License for more details.                         |
   |                                                                      |
   | You should have received a copy of both licenses referred to here.   |
   | If you did not, or have any questions about PHP licensing, please    |
   | contact core@php.net.                                                |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@php.net>                             |
   +----------------------------------------------------------------------+
 */
/* $Id: mime.c,v 1.64 2000/10/17 01:30:59 sas Exp $ */
#include <stdio.h>
#include "php.h"
#include "internal_functions.h"
#include "type.h"
#include "post.h"
#include "mime.h"
#include "php3_list.h"

int le_uploads;
extern HashTable list;

#define NEW_BOUNDARY_CHECK 1
#define SAFE_RETURN { if (namebuf) efree(namebuf); if (filenamebuf) efree(filenamebuf); if (lbuf) efree(lbuf); if (abuf) efree(abuf); return; }

/*
 * Split raw mime stream up into appropriate components
 */
void php3_mime_split(char *buf, int cnt, char *boundary, pval *http_post_vars)
{
	char *ptr, *loc, *loc2, *s, *name, *filename, *u, *fn;
	int len, state = 0, Done = 0, rem, urem;
	long bytes, max_file_size = 0;
	char *namebuf=NULL, *filenamebuf=NULL, *lbuf=NULL, *abuf=NULL, *sbuf=NULL;
	char sbytes[16];
	FILE *fp;
	int itype;
	TLS_VARS;

	ptr = buf;
	rem = cnt;
	len = strlen(boundary);
	while ((ptr - buf < cnt) && !Done) {
		switch (state) {
			case 0:			/* Looking for mime boundary */
				loc = memchr(ptr, *boundary, cnt);
				if (loc) {
					if (!strncmp(loc, boundary, len)) {

						state = 1;
						rem -= (loc - ptr) + len + 2;
						ptr = loc + len + 2;
					} else {
						rem -= (loc - ptr) + 1;
						ptr = loc + 1;
					}
				} else {
					Done = 1;
				}
				break;
			case 1:			/* Check content-disposition */
				if (strncasecmp(ptr, "Content-Disposition: form-data;", 31)) {
					if (rem < 31) {
						SAFE_RETURN;
					}
					php3_error(E_WARNING, "File Upload Mime headers garbled [%c%c%c%c%c]", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4));
					SAFE_RETURN;
				}
				loc = memchr(ptr, '\n', rem);
				name = strstr(ptr, " name=");
				if (name && name < loc) {
					name += 6;
					s = memchr(name, '\"', loc - name);
					if ( name == s ) {
						name++;
						s = memchr(name, '\"', loc - name);
						if (!s) {
							php3_error(E_WARNING, "File Upload Mime headers garbled [%c%c%c%c%c]", *name, *(name + 1), *(name + 2), *(name + 3), *(name + 4));
							SAFE_RETURN;
						}
					} else if (!s) {
						s = loc;
					} else {
						php3_error(E_WARNING, "File Upload Mime headers garbled [%c%c%c%c%c]", *name, *(name + 1), *(name + 2), *(name + 3), *(name + 4));
						SAFE_RETURN;
					}
					if (namebuf) {
						efree(namebuf);
					}
					namebuf = estrndup(name, s-name);
					if (lbuf) {
						efree(lbuf);
					}
					lbuf = emalloc(s-name + MAX(MAX(sizeof("_name"),sizeof("_size")),sizeof("_type")) + 4);
					state = 2;
					loc2 = memchr(loc + 1, '\n', rem);
					rem -= (loc2 - ptr) + 1;
					ptr = loc2 + 1;
				} else {
					php3_error(E_WARNING, "File upload error - no name component in content disposition");
					SAFE_RETURN;
				}
				filename = strstr(s, " filename=\"");
				if (filename && filename < loc) {
					filename += 11;
					s = memchr(filename, '\"', loc - filename);
					if (!s) {
						php3_error(E_WARNING, "File Upload Mime headers garbled [%c%c%c%c%c]", *filename, *(filename + 1), *(filename + 2), *(filename + 3), *(filename + 4));
						SAFE_RETURN;
					}
					if (filenamebuf) {
						efree(filenamebuf);
					}
					filenamebuf = estrndup(filename, s-filename);
					if (strstr(namebuf, "[]")) {
						if (abuf) {
							efree(abuf);
						}
						abuf = estrndup(namebuf, strlen(namebuf) - 2);
						sprintf(lbuf, "%s_name[]", abuf);
						sbuf=estrdup(abuf);
					} else {
						sprintf(lbuf, "%s_name", namebuf);
						sbuf=estrdup(namebuf);
					}
					s = strrchr(filenamebuf, '\\');
					if (s && s > filenamebuf) {
						/* SET_VAR_STRING(lbuf, estrdup(s + 1)); */
						_php3_parse_gpc_data(estrdup(s + 1), lbuf, http_post_vars);
					} else {
						/* SET_VAR_STRING(lbuf, estrdup(filenamebuf)); */
						_php3_parse_gpc_data(estrdup(filenamebuf), lbuf, http_post_vars);
					}
					state = 3;
					if ((loc2 - loc) > 2) {
						if (!strncasecmp(loc + 1, "Content-Type:", 13)) {
							*(loc2 - 1) = '\0';
							if (abuf) {
								sprintf(lbuf, "%s_type[]", abuf);
							} else {
								sprintf(lbuf, "%s_type", namebuf);
							}
							SET_VAR_STRING(lbuf, estrdup(loc + 15));
							*(loc2 - 1) = '\n';
						}
						rem -= 2;
						ptr += 2;
					}
				}
				break;

			case 2:			/* handle form-data fields */
				loc = memchr(ptr, *boundary, rem);
				u = ptr;
				while (loc) {
					if (!strncmp(loc, boundary, len))
						break;
					u = loc + 1;
					urem = rem - (loc - ptr) - 1;
					loc = memchr(u, *boundary, urem);
				}
				if (!loc) {
					php3_error(E_WARNING, "File Upload Field Data garbled");
					SAFE_RETURN;
				}
				*(loc - 4) = '\0';

				_php3_parse_gpc_data(ptr,namebuf,http_post_vars);

				/* And a little kludge to pick out special MAX_FILE_SIZE */
				itype = php3_check_ident_type(namebuf);
				if (itype) {
					u = strchr(namebuf, '[');
					if (u)
						*u = '\0';
				}
				if (!strcmp(namebuf, "MAX_FILE_SIZE")) {
					max_file_size = atol(ptr);
				}
				if (itype) {
					if (u)
						*u = '[';
				}
				rem	-= (loc - ptr);
				ptr = loc;
				state = 0;
				break;

			case 3:			/* Handle file */
				loc = memchr(ptr, *boundary, rem);
				u = ptr;
				while (loc) {
					if (!strncmp(loc, boundary, len)
#if NEW_BOUNDARY_CHECK
						&& (loc-2>buf && *(loc-2)=='-' && *(loc-1)=='-') /* ensure boundary is prefixed with -- */
						&& (loc-2==buf || *(loc-3)=='\n') /* ensure beginning of line */
#endif
						) {
						break;
					}
					u = loc + 1;
					urem = rem - (loc - ptr) - 1;
					loc = memchr(u, *boundary, urem);
				}
				if (!loc) {
					php3_error(E_WARNING, "File Upload Error - No Mime boundary found after start of file header");
					SAFE_RETURN;
				}
				fn = tempnam(php3_ini.upload_tmp_dir, "php");
				if ((loc - ptr - 4) > php3_ini.upload_max_filesize) {
					php3_error(E_WARNING, "Max file size of %ld bytes exceeded - file [%s] not saved", php3_ini.upload_max_filesize,namebuf);
					bytes=0;	
					/* SET_VAR_STRING(namebuf, estrdup("none")); */
					_php3_parse_gpc_data(estrdup("none"), namebuf, http_post_vars);
				} else if (max_file_size && ((loc - ptr - 4) > max_file_size)) {
					php3_error(E_WARNING, "Max file size exceeded - file [%s] not saved", namebuf);
					bytes = 0;
					/* SET_VAR_STRING(namebuf, estrdup("none")); */
					if(memcmp(namebuf,sbuf,strlen(sbuf)))
						_php3_parse_gpc_data(estrdup("none"), namebuf, http_post_vars);
				} else if ((loc - ptr - 4) <= 0) {
					bytes = 0;
					/* SET_VAR_STRING(namebuf, estrdup("none")); */
					if(memcmp(namebuf,sbuf,strlen(sbuf)))
						_php3_parse_gpc_data(estrdup("none"), namebuf, http_post_vars);
				} else {
					fp = fopen(fn, "w");
					if (!fp) {
						php3_error(E_WARNING, "File Upload Error - Unable to open temporary file [%s]", fn);
						SAFE_RETURN;
					}
					bytes = fwrite(ptr, 1, loc - ptr - 4, fp);
					fclose(fp);
					{
						int dummy=1;

						_php3_hash_add(&GLOBAL(request_info).rfc1867_uploaded_files, fn, strlen(fn)+1, &dummy, sizeof(int), NULL);
					}
					php3_list_do_insert(&GLOBAL(list),fn,GLOBAL(le_uploads));  /* Tell PHP about the file so the destructor can unlink it later */
					if (bytes < (loc - ptr - 4)) {
						php3_error(E_WARNING, "Only %d bytes were written, expected to write %ld", bytes, loc - ptr - 4);
					}
					/* SET_VAR_STRING(namebuf, estrdup(fn)); */
					_php3_parse_gpc_data(estrdup(fn), namebuf, http_post_vars);
				}
				
				/* SET_VAR_LONG(lbuf, bytes); */

				if (abuf) {
					sprintf(lbuf, "%s_size[]", abuf);
				} else {
					sprintf(lbuf, "%s_size", namebuf);
				}
				memset(sbytes, 0, 16);
				sprintf(sbytes, "%ld", bytes);
				_php3_parse_gpc_data(estrdup(sbytes), lbuf, http_post_vars);
								
				state = 0;
				rem -= (loc - ptr);
				ptr = loc;
				break;
		}
	}
	if(sbuf) efree(sbuf);
	SAFE_RETURN;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
