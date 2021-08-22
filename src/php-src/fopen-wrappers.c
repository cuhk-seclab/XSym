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
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Jim Winstead <jimw@php.net>                                 |
   +----------------------------------------------------------------------+
 */
/* $Id: fopen-wrappers.c,v 1.78 2000/06/05 00:28:18 jimw Exp $ */

#include "php.h"
#include "modules.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if MSVC5
#include <windows.h>
#include <winsock.h>
#define O_RDONLY _O_RDONLY
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#include "win32/param.h"
#else
#include <sys/param.h>
#endif

#include "safe_mode.h"
#include "php3_list.h"
#include "php3_realpath.h"
#include "functions/head.h"
#include "functions/url.h"
#include "functions/base64.h"
#include "functions/php3_string.h"

#if HAVE_PWD_H
#if MSVC5
#include "win32/pwd.h"
#else
#include <pwd.h>
#endif
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifndef S_ISREG
#define S_ISREG(mode)	(((mode)&S_IFMT) == S_IFREG)
#endif

#if MSVC5
#include <winsock.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "functions/fsock.h"

#if MSVC5
#undef AF_UNIX
#endif

#if defined(AF_UNIX)
#include <sys/un.h>
#endif

static FILE *php3_fopen_url_wrapper(const char *path, char *mode, int options, int *issock, int *socketd);

int _php3_getftpresult(int socketd);


/*
	When open_basedir is not NULL, check if the given filename is located in
	open_basedir. Returns -1 if error or not in the open_basedir, else 0
	
	When open_basedir is NULL, always return 0
*/
PHPAPI int _php3_check_specific_open_basedir(char *basedir, char *path)
{
	char resolved_name[MAXPATHLEN];
	char resolved_basedir[MAXPATHLEN];
	char local_open_basedir[MAXPATHLEN];
	int local_open_basedir_pos;
	
	/* Special case basedir==".": Use script-directory */
	if ((strcmp(basedir, ".") == 0) && 
		GLOBAL(request_info).filename &&
		*GLOBAL(request_info).filename
	) {
		strcpy(local_open_basedir, GLOBAL(request_info).filename);
		local_open_basedir_pos = strlen(local_open_basedir) - 1;

		/* Strip filename */
		while ((
#if WIN32|WINNT
			(local_open_basedir[local_open_basedir_pos] != '\\') ||
#endif
			(local_open_basedir[local_open_basedir_pos] != '/')
			) &&
			(local_open_basedir_pos >= 0)
		) {
			local_open_basedir[local_open_basedir_pos--] = 0;
		}
/* stripping unnecessary slashes is left 
   as an exercise to the underlying OS */
#if 0	
		/* Strip double (back)slashes */
		if (local_open_basedir_pos > 0) {
			while ((
#if WIN32|WINNT
				(local_open_basedir[local_open_basedir_pos-1] == '\\') ||
#endif
				(local_open_basedir[local_open_basedir_pos-1] == '/')
				) &&
				(local_open_basedir_pos > 0)
			) {
				local_open_basedir[local_open_basedir_pos--] = 0;
			}
		}
#endif		
	} else {
		/* Else use the unmodified path */
		strcpy(local_open_basedir, basedir);
	}
	
	/* Resolve the real path into resolved_name */
	if ((_php3_realpath(path, resolved_name) != NULL) && (_php3_realpath(local_open_basedir, resolved_basedir) != NULL)) {
		/* Check the path */
#if WIN32|WINNT
		if (strncasecmp(resolved_basedir, resolved_name, strlen(resolved_basedir)) == 0) {
#else
		if (strncmp(resolved_basedir, resolved_name, strlen(resolved_basedir)) == 0) {
#endif
			/* File is in the right directory */
			return 0;
		} else {
			return -1;
		}
	} else {
		/* Unable to resolve the real path, return -1 */
		return -1;
	}
}

PHPAPI int _php3_check_open_basedir(char *path)
{
	/* Only check when open_basedir is available */
	if (php3_ini.open_basedir && *php3_ini.open_basedir) {
		char *pathbuf;
		char *ptr;
		char *end;
			
		pathbuf = estrdup(php3_ini.open_basedir);

		ptr = pathbuf;

		while (ptr && *ptr) {
#if WIN32|WINNT|OS2
			end = strchr(ptr, ';');
#else
			end = strchr(ptr, ':');
#endif
			if (end != NULL) {
				*end = '\0';
				end++;
			}
			
			if (_php3_check_specific_open_basedir(ptr, path) == 0) {
				efree(pathbuf);
				return 0;
			}
			
			ptr = end;
		}
		php3_error(E_WARNING, "open_basedir restriction in effect. File is in wrong directory.");
		efree(pathbuf);
		return -1;
	}
	
	/* Nothing to check... */
	return 0;
}

PHPAPI FILE *php3_fopen_wrapper(char *path, char *mode, int options, int *issock, int *socketd)
{
	int cm=2;  /* checkuid mode: 2 = if file does not exist, check directory */
	/* FIXME  Lets not get in the habit of doing stuff like this.  This should
	be runtime enabled, NOT compile time. */
#if PHP3_URL_FOPEN
	if (!(options & IGNORE_URL)) {
		return php3_fopen_url_wrapper(path, mode, options, issock, socketd);
	}
#endif

	if (options & USE_PATH && php3_ini.include_path != NULL) {
		return php3_fopen_with_path(path, mode, php3_ini.include_path, NULL);
	} else {
		if(!strcmp(mode,"r") || !strcmp(mode,"r+")) cm=0;
		if (options & ENFORCE_SAFE_MODE && php3_ini.safe_mode && (!_php3_checkuid(path, cm))) {
			return NULL;
		}
		if (_php3_check_open_basedir(path)) return NULL;
		return fopen(path, mode);
	}
}

#if CGI_BINARY || FHTTPD || USE_SAPI

FILE *php3_fopen_for_parser(void)
{
	FILE *fp;
	struct stat st;
	char *temp, *path_info, *fn;
	int l;
	TLS_VARS;


	fn = GLOBAL(request_info).filename;
	path_info = GLOBAL(request_info).path_info;
#if HAVE_PWD_H
	if (php3_ini.user_dir && *php3_ini.user_dir
		&& path_info && '/' == path_info[0] && '~' == path_info[1]) {

		char user[32];
		struct passwd *pw;
		char *s = strchr(path_info + 2, '/');

		fn = NULL;				/* discard the original filename, it must not be used */
		if (s) {				/* if there is no path name after the file, do not bother
								   to try open the directory */
			l = s - (path_info + 2);
			if (l > sizeof(user) - 1)
				l = sizeof(user) - 1;
			memcpy(user, path_info + 2, l);
			user[l] = '\0';

			pw = getpwnam(user);
			if (pw && pw->pw_dir) {
				fn = emalloc(strlen(php3_ini.user_dir) + strlen(path_info) + strlen(pw->pw_dir) + 4);
				if (fn) {
					strcpy(fn, pw->pw_dir);		/* safe */
					strcat(fn, "/");	/* safe */
					strcat(fn, php3_ini.user_dir);	/* safe */
					strcat(fn, "/");	/* safe */
					strcat(fn, s + 1);	/* safe (shorter than path_info) */
					STR_FREE(GLOBAL(request_info).filename);
					GLOBAL(request_info).filename = fn;
				}
			}
		}
	} else
#endif
#if WIN32
	if (php3_ini.doc_root && path_info && ('/' == *php3_ini.doc_root ||
		'\\' == *php3_ini.doc_root || strstr(php3_ini.doc_root,":\\") ||
		strstr(php3_ini.doc_root,":/"))) {
#else
	if (php3_ini.doc_root && '/' == *php3_ini.doc_root && path_info) {
#endif
		l = strlen(php3_ini.doc_root);
		fn = emalloc(l + strlen(path_info) + 2);
		if (fn) {
			memcpy(fn, php3_ini.doc_root, l);
			if ('/' != fn[l - 1] || '\\' != fn[l - 1])	/* l is never 0 */
				fn[l++] = '/';
			if ('/' == path_info[0])
				l--;
			strcpy(fn + l, path_info);
			STR_FREE(GLOBAL(request_info).filename);
			GLOBAL(request_info).filename = fn;
		}
	}							/* if doc_root && path_info */
	if (!fn) {
		/* we have to free request_info.filename here because
		   php3_destroy_request_info assumes that it will get
		   freed when the include_names hash is emptied, but
		   we're not adding it in this case */
		STR_FREE(GLOBAL(request_info).filename);
		GLOBAL(request_info).filename = NULL;
		return NULL;
	}
	fp = fopen(fn, "r");

	/* refuse to open anything that is not a regular file */
	if (fp && (0 > fstat(fileno(fp), &st) || !S_ISREG(st.st_mode))) {
		fclose(fp);
		fp = NULL;
	}
	if (!fp) {
		php3_error(E_ERROR, "Unable to open %s", fn);
		STR_FREE(GLOBAL(request_info).filename);	/* for same reason as above */
		return NULL;
	}
	_php3_hash_index_update(&GLOBAL(include_names), 0, (void *) &fn, sizeof(char *), NULL);

	temp = strdup(fn);
	_php3_dirname(temp, strlen(temp));
	if (*temp)
		chdir(temp);
	free(temp);

	return fp;
}

#endif							/* CGI_BINARY || USE_SAPI */

/*
 * Tries to open a file with a PATH-style list of directories.
 * If the filename starts with "." or "/", the path is ignored.
 */
PHPAPI FILE *php3_fopen_with_path(char *filename, char *mode, char *path, char **opened_path)
{
	char *pathbuf, *ptr, *end;
	char trypath[MAXPATHLEN + 1];
	struct stat sb;
	FILE *fp;
	int cm=2;
	TLS_VARS;

	if(!strcmp(mode,"r") || !strcmp(mode,"r+")) cm=0;
	if (opened_path) {
		*opened_path = NULL;
	}
	/* Relative path open */
	if (*filename == '.') {
		if (php3_ini.safe_mode && (!_php3_checkuid(filename, cm))) {
			return NULL;
		}
		if (_php3_check_open_basedir(filename)) return NULL;
		fp = fopen(filename, mode);
		if (fp && opened_path) {
			*opened_path = expand_filepath(filename);
		}
		return fp;
	}
	/* Absolute path open - prepend document_root in safe mode */
#if WIN32|WINNT|OS2
	if ((*filename == '\\') || (*filename == '/') || (filename[1] == ':')) {
#else
	if (*filename == '/') {
#endif
		if (php3_ini.safe_mode) {
			if(php3_ini.doc_root) {
				snprintf(trypath, MAXPATHLEN, "%s%s", php3_ini.doc_root, filename);
			} else {
				strncpy(trypath,filename,MAXPATHLEN);
			}
			if (!_php3_checkuid(trypath, cm)) {
				return NULL;
			}
			if (_php3_check_open_basedir(trypath)) return NULL;
			fp = fopen(trypath, mode);
			if (fp && opened_path) {
				*opened_path = expand_filepath(trypath);
			}
			return fp;
		} else {
			if (_php3_check_open_basedir(filename)) return NULL;
			return fopen(filename, mode);
		}
	}
	if (!path || (path && !*path)) {
		if (php3_ini.safe_mode && (!_php3_checkuid(filename, cm))) {
			return NULL;
		}
		if (_php3_check_open_basedir(filename)) return NULL;
		fp = fopen(filename, mode);
		if (fp && opened_path) {
			*opened_path = strdup(filename);
		}
		return fp;
	}
	pathbuf = estrdup(path);

	ptr = pathbuf;

	while (ptr && *ptr) {
#if WIN32|WINNT|OS2
		end = strchr(ptr, ';');
#else
		end = strchr(ptr, ':');
#endif
		if (end != NULL) {
			*end = '\0';
			end++;
		}
		snprintf(trypath, MAXPATHLEN, "%s/%s", ptr, filename);
		if (php3_ini.safe_mode) {
			if (stat(trypath, &sb) == 0 && (!_php3_checkuid(trypath, cm))) {
				efree(pathbuf);
				return NULL;
			}
		}
		if ((fp = fopen(trypath, mode)) != NULL) {
			if (_php3_check_open_basedir(trypath)) {
				fclose(fp);
				efree(pathbuf);
				return NULL;
			}
			if (opened_path) {
				*opened_path = expand_filepath(trypath);
			}
			efree(pathbuf);
			return fp;
		}
		ptr = end;
	}
	efree(pathbuf);
	return NULL;
}

/*
 * If the specified path starts with "http://" (insensitive to case),
 * a socket is opened to the specified web server and a file pointer is
 * position at the start of the body of the response (past any headers).
 * This makes a HTTP/1.0 request, and knows how to pass on the username
 * and password for basic authentication.
 *
 * If the specified path starts with "ftp://" (insensitive to case),
 * a pair of sockets are used to request the specified file and a file
 * pointer to the requested file is returned. Passive mode ftp is used,
 * so if the server doesn't support this, it will fail!
 *
 * Otherwise, fopen is called as usual and the file pointer is returned.
 */

static FILE *php3_fopen_url_wrapper(const char *path, char *mode, int options, int *issock, int *socketd)
{
	url *resource;
	int result;
	char *scratch;
	unsigned char *tmp;

	char tmp_line[512];
	char location[512];
	char hdr_line[8192];
	char *tpath, *ttpath;
	int body = 0;
	int reqok = 0;
	int i, len;

	FILE *fp = NULL;
	struct sockaddr_in server;
	unsigned short portno;

	if (!strncasecmp(path, "http://", 7)) {
		if (strlen(path) < 8)
			return NULL;
		resource = url_parse((char *) path);
		if (resource == NULL) {
			php3_error(E_WARNING, "Invalid URL specified, %s", path);
			*issock = BAD_URL;
			return NULL;
		}
		/* use port 80 if one wasn't specified */
		if (resource->port == 0)
			resource->port = 80;

		*socketd = socket(AF_INET, SOCK_STREAM, 0);
		if (*socketd == SOCK_ERR) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
		server.sin_family = AF_INET;

		if (lookup_hostname(resource->host, &server.sin_addr)) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
		server.sin_port = htons(resource->port);

		if (connect(*socketd, (struct sockaddr *) &server, sizeof(server)) == SOCK_CONN_ERR) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
#if 0
		if ((fp = fdopen(*socketd, "r+")) == NULL) {
			free_url(resource);
			return NULL;
		}
#ifdef HAVE_SETVBUF
		if ((setvbuf(fp, NULL, _IONBF, 0)) != 0) {
			free_url(resource);
			return NULL;
		}
#endif
#endif							/*win32 */

		strcpy(hdr_line, "GET ");
		len = 4;
		/* tell remote http which file to get */
		if (resource->path != NULL) {
			strncat(hdr_line, resource->path, sizeof(hdr_line)-len);
			len += strlen(resource->path);
		} else {
			strncat(hdr_line, "/", sizeof(hdr_line)-len);
			len++;
		}
		/* append the query string, if any */
		if (resource->query != NULL) {
			strncat(hdr_line, "?", sizeof(hdr_line)-len);
			len++;
			strncat(hdr_line, resource->query, sizeof(hdr_line)-len);
			len += strlen(resource->query);
		}
		strncat(hdr_line, " HTTP/1.0\r\n", sizeof(hdr_line)-len);
		hdr_line[sizeof(hdr_line)-1] = '\0';
		SOCK_WRITE(hdr_line, *socketd);
		
		/* send authorization header if we have user/pass */
		if (resource->user != NULL && resource->pass != NULL) {
			scratch = (char *) emalloc(strlen(resource->user) + strlen(resource->pass) + 2);
			if (!scratch) {
				free_url(resource);
				return NULL;
			}
			strcpy(scratch, resource->user);
			strcat(scratch, ":");
			strcat(scratch, resource->pass);
			tmp = _php3_base64_encode((unsigned char *)scratch, strlen(scratch), NULL);

			if (snprintf(hdr_line, sizeof(hdr_line),
						 "Authorization: Basic %s\r\n", tmp) > 0) {
				SOCK_WRITE(hdr_line, *socketd);
			}

			efree(scratch);
			efree(tmp);
		}
		/* if the user has configured who they are, send a From: line */
		if (cfg_get_string("from", &scratch) == SUCCESS) {
			if (snprintf(hdr_line, sizeof(hdr_line),
						 "From: %s\r\n", scratch) > 0) {
				SOCK_WRITE(hdr_line, *socketd);
			}

		}
		/* send a Host: header so name-based virtual hosts work */
		if (resource->port != 80) {
			len = snprintf(hdr_line, sizeof(hdr_line),
						   "Host: %s:%i\r\n", resource->host, resource->port);
		} else {
			len = snprintf(hdr_line, sizeof(hdr_line),
						   "Host: %s\r\n", resource->host);
		}
		if (len > 0) {
			SOCK_WRITE(hdr_line, *socketd);
		}

		/* identify ourselves and end the headers */
		SOCK_WRITE("User-Agent: PHP/" PHP_VERSION "\r\n\r\n", *socketd);

		body = 0;
		location[0] = '\0';
		if (!SOCK_FEOF(*socketd)) {
			/* get response header */
			if (SOCK_FGETS(tmp_line, sizeof(tmp_line), *socketd) != NULL) {
				if (strncmp(tmp_line + 8, " 200 ", 5) == 0) {
					reqok = 1;
				}
			}
		}
		/* Read past HTTP headers */
		while (!body && !SOCK_FEOF(*socketd)) {
			if (SOCK_FGETS(tmp_line, sizeof(tmp_line), *socketd) != NULL) {
				char *p = tmp_line;

				tmp_line[sizeof(tmp_line)-1] = '\0';

				while (*p) {
					if (*p == '\n' || *p == '\r') {
						*p = '\0';
					}
					p++;
				}

				if (!strncasecmp(tmp_line, "Location: ", 10)) {
					strcpy(location, tmp_line + 10);
				}

				if (tmp_line[0] == '\0') {
					body = 1;
				}
			}
		}
		if (!reqok) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			if (location[0] != '\0') {
				return php3_fopen_url_wrapper(location, mode, options, issock, socketd);
			} else {
				return NULL;
			}
		}
		free_url(resource);
		*issock = 1;
		return (fp);
	} else if (!strncasecmp(path, "php://", 6)) {
		const char *res = path + 6;

		if (!strcasecmp(res, "stdin")) {
			return fdopen(STDIN_FILENO, mode);
		} else if (!strcasecmp(res, "stdout")) {
			return fdopen(STDOUT_FILENO, mode);
		} else if (!strcasecmp(res, "stderr")) {
			return fdopen(STDERR_FILENO, mode);
		}
	} else if (!strncasecmp(path, "ftp://", 6)) {
		if (strlen(path) < 7)
			return NULL;
		resource = url_parse((char *) path);
		if (resource == NULL) {
			php3_error(E_WARNING, "Invalid URL specified, %s", path);
			*issock = BAD_URL;
			return NULL;
		} else if (resource->path == NULL) {
			php3_error(E_WARNING, "No file-path specified");
			free_url(resource);
			*issock = BAD_URL;
			return NULL;
		}
		/* use port 21 if one wasn't specified */
		if (resource->port == 0)
			resource->port = 21;

		*socketd = socket(AF_INET, SOCK_STREAM, 0);
		if (*socketd == SOCK_ERR) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
		server.sin_family = AF_INET;

		if (lookup_hostname(resource->host, &server.sin_addr)) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
		server.sin_port = htons(resource->port);

		if (connect(*socketd, (struct sockaddr *) &server, sizeof(server)) == SOCK_CONN_ERR) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
#if 0
		if ((fpc = fdopen(*socketd, "r+")) == NULL) {
			free_url(resource);
			return NULL;
		}
#ifdef HAVE_SETVBUF
		if ((setvbuf(fpc, NULL, _IONBF, 0)) != 0) {
			free_url(resource);
			fclose(fpc);
			return NULL;
		}
#endif
#endif

		/* Start talking to ftp server */
		result = _php3_getftpresult(*socketd);
		if (result > 299 || result < 200) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}
		/* send the user name */
		SOCK_WRITE("USER ", *socketd);
		if (resource->user != NULL) {
			_php3_rawurldecode(resource->user, strlen(resource->user));
			SOCK_WRITE(resource->user, *socketd);
		} else {
			SOCK_WRITE("anonymous", *socketd);
		}
		SOCK_WRITE("\r\n", *socketd);

		/* get the response */
		result = _php3_getftpresult(*socketd);

		/* if a password is required, send it */
		if (result >= 300 && result <= 399) {
			SOCK_WRITE("PASS ", *socketd);
			if (resource->pass != NULL) {
				_php3_rawurldecode(resource->pass, strlen(resource->pass));
				SOCK_WRITE(resource->pass, *socketd);
			} else {
				/* if the user has configured who they are,
				   send that as the password */
				if (cfg_get_string("from", &scratch) == SUCCESS) {
					SOCK_WRITE(scratch, *socketd);
				} else {
					SOCK_WRITE("anonymous", *socketd);
				}
			}
			SOCK_WRITE("\r\n", *socketd);

			/* read the response */
			result = _php3_getftpresult(*socketd);
			if (result > 299 || result < 200) {
				free_url(resource);
				SOCK_FCLOSE(*socketd);
				*socketd = 0;
				return NULL;
			}
		} else if (result > 299 || result < 200) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}

		/* set the connection to be binary */
		SOCK_WRITE("TYPE I\r\n", *socketd);
		result = _php3_getftpresult(*socketd);
		if (result > 299 || result < 200) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}

		/* find out the size of the file (verifying it exists) */
		SOCK_WRITE("SIZE ", *socketd);
		SOCK_WRITE(resource->path, *socketd);
		SOCK_WRITE("\r\n", *socketd);

		/* read the response */
		result = _php3_getftpresult(*socketd);
		if (mode[0] == 'r') {
			/* when reading file, it must exist */
			if (result != 500 && result != 502 && (result > 299 || result < 200)) {
				php3_error(E_WARNING, "File not found");
				free_url(resource);
				SOCK_FCLOSE(*socketd);
				*socketd = 0;
				errno = ENOENT;
				return NULL;
			}
		} else {
			/* when writing file, it must NOT exist */
			if (result != 500 && result != 502 && (result <= 299 && result >= 200)) {
				php3_error(E_WARNING, "File already exists");
				free_url(resource);
				SOCK_FCLOSE(*socketd);
				*socketd = 0;
				errno = EEXIST;
				return NULL;
			}
		}

		/* set up the passive connection */
		SOCK_WRITE("PASV\r\n", *socketd);
		while (SOCK_FGETS(tmp_line, sizeof(tmp_line), *socketd) &&
			!(isdigit((int) tmp_line[0]) && isdigit((int) tmp_line[1]) &&
			  isdigit((int) tmp_line[2]) && tmp_line[3] == ' '));

		/* make sure we got a 227 response */
		if (strncmp(tmp_line, "227", 3)) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}
		/* parse pasv command (129,80,95,25,13,221) */
		tpath = tmp_line;

		/* skip over the "227 Some message " part */
		for (tpath += 4; *tpath && !isdigit((int) *tpath); tpath++);
		if (!*tpath) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}
		/* skip over the host ip, we just assume it's the same */
		for (i = 0; i < 4; i++) {
			for (; isdigit((int) *tpath); tpath++);
			if (*tpath == ',') {
				tpath++;
			} else {
				SOCK_FCLOSE(*socketd);
				*socketd = 0;
				return NULL;
			}
		}

		/* pull out the MSB of the port */
		portno = (unsigned short) strtol(tpath, &ttpath, 10) * 256;
		if (ttpath == NULL) {
			/* didn't get correct response from PASV */
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}
		tpath = ttpath;
		if (*tpath == ',') {
			tpath++;
		} else {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}

		/* pull out the LSB of the port */
		portno += (unsigned short) strtol(tpath, &ttpath, 10);

		if (ttpath == NULL) {
			/* didn't get correct response from PASV */
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}

		if (mode[0] == 'r') {
			/* retrieve file */
			SOCK_WRITE("RETR ", *socketd);
		} else {
			/* store file */
			SOCK_WRITE("STOR ", *socketd);
		} 
		if (resource->path != NULL) {
			SOCK_WRITE(resource->path, *socketd);
		} else {
			SOCK_WRITE("/", *socketd);
		}

		/* close control connection */
		SOCK_WRITE("\r\nQUIT\r\n", *socketd);
		SOCK_FCLOSE(*socketd);

		/* open the data channel */
		*socketd = socket(AF_INET, SOCK_STREAM, 0);
		if (*socketd == SOCK_ERR) {
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			free_url(resource);
			return NULL;
		}
		server.sin_family = AF_INET;

		if (lookup_hostname(resource->host, &server.sin_addr)) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}
		server.sin_port = htons(portno);

		if (connect(*socketd, (struct sockaddr *) &server, sizeof(server)) == SOCK_CONN_ERR) {
			free_url(resource);
			SOCK_FCLOSE(*socketd);
			*socketd = 0;
			return NULL;
		}
#if 0
		if (mode[0] == 'r') {
			if ((fp = fdopen(*socketd, "r+")) == NULL) {
				free_url(resource);
				return NULL;
			}
		} else {
			if ((fp = fdopen(*socketd, "w+")) == NULL) {
				free_url(resource);
				return NULL;
			}
		}
#ifdef HAVE_SETVBUF
		if ((setvbuf(fp, NULL, _IONBF, 0)) != 0) {
			free_url(resource);
			fclose(fp);
			return NULL;
		}
#endif
#endif
		free_url(resource);
		*issock = 1;
		return (fp);

	} else {
		if (options & USE_PATH) {
			fp = php3_fopen_with_path((char *) path, mode, php3_ini.include_path, NULL);
		} else {
			int cm=2;
			if(!strcmp(mode,"r") || !strcmp(mode,"r+")) cm=0;
			if (options & ENFORCE_SAFE_MODE && php3_ini.safe_mode && (!_php3_checkuid(path, cm))) {
				fp = NULL;
			} else {
				if (_php3_check_open_basedir((char *) path)) {
					fp = NULL;
				} else {
					fp = fopen(path, mode);
				}
			}
		}

		*issock = 0;

		return (fp);
	}

	/* NOTREACHED */
	SOCK_FCLOSE(*socketd);
	*socketd = 0;
	return NULL;
}

int _php3_getftpresult(int socketd)
{
	char tmp_line[256];

	while (SOCK_FGETS(tmp_line, 256, socketd) &&
		   !(isdigit((int) tmp_line[0]) && isdigit((int) tmp_line[1]) &&
			 isdigit((int) tmp_line[2]) && tmp_line[3] == ' '));

	return strtol(tmp_line, NULL, 10);
}

PHPAPI int php3_isurl(char *path)
{
	return (!strncasecmp(path, "http://", 7) || !strncasecmp(path, "ftp://", 6));
}


PHPAPI char *php3_strip_url_passwd(char *url)
{
	register char *p = url, *url_start;
	
	while (*p) {
		if (*p==':' && *(p+1)=='/' && *(p+2)=='/') {
			/* found protocol */
			url_start = p = p+3;
			
			while (*p) {
				if (*p=='@') {
					int i;
					
					for (i=0; i<3 && url_start<p; i++, url_start++) {
						*url_start = '.';
					}
					for (; *p; p++) {
						*url_start++ = *p;
					}
					*url_start=0;
					break;
				}
				p++;
			}
			return url;
		}
		p++;
	}
	return url;
}


PHPAPI char *expand_filepath(char *filepath)
{
	char *retval = NULL;

	if (filepath[0] == '.') {
		char *cwd = malloc(MAXPATHLEN + 1);

		if (getcwd(cwd, MAXPATHLEN)) {
			char *cwd_end = cwd + strlen(cwd);

			if (filepath[1] == '.') {	/* parent directory - .. */
				/* erase the last directory name from the path */
				while (*cwd_end != '/') {
					*cwd_end-- = 0;
				}
				filepath++;		/* make filepath appear as a current directory path */
			}
			if (cwd_end > cwd && *cwd_end == '/') {		/* remove trailing slashes */
				*cwd_end-- = 0;
			}
			retval = (char *) malloc(strlen(cwd) + strlen(filepath) - 1 + 1);
			strcpy(retval, cwd);
			strcat(retval, filepath + 1);
			free(cwd);
		}
	}
	if (!retval) {
		retval = strdup(filepath);
	}
	return retval;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
