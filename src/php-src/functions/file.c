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
   +----------------------------------------------------------------------+
 */
/* $Id: file.c,v 1.233 2000/09/27 16:15:07 sas Exp $ */
#include "php.h"

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
#include "win32/param.h"
#include "win32/winutil.h"
#else
#include <sys/param.h>
#include <sys/socket.h>
/* #include <sys/uio.h> */
#endif
#include "head.h"
#include "internal_functions.h"
#include "safe_mode.h"
#include "php3_list.h"
#include "php3_string.h"
#include "exec.h"
#include "file.h"
#if HAVE_PWD_H
#if MSVC5
#include "win32/pwd.h"
#else
#include <pwd.h>
#endif
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if WIN32|WINNT
# include <winsock.h>
#else
# include <netinet/in.h>
# include <netdb.h>
# include <arpa/inet.h>
#endif
#include "snprintf.h"
#include "fsock.h"
#include "fopen-wrappers.h"

#include "php_compat.h"

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#if MISSING_FCLOSE_DECL
extern int fclose();
#endif

static int fgetss_state = 0;
int le_fp,le_pp;
int wsa_fp; /*to handle reading and writing to windows sockets*/
static int pclose_ret;
extern int le_uploads;

#ifndef HAVE_TEMPNAM
/*
 * Copyright (c) 1988, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <unistd.h>

#ifndef MAXPATHLEN
# ifdef PATH_MAX
#  define MAXPATHLEN PATH_MAX
# else
#  define MAXPATHLEN 255
# endif
#endif

char *
tempnam(const char *dir, const char *pfx)
{
	int save_errno;
	char *f, *name;
	static char path_tmp[] = "/tmp";

	if (!(name = emalloc(MAXPATHLEN))) {
		return(NULL);
	}

	if (!pfx) {
		pfx = "tmp.";
	}

	if (f = getenv("TMPDIR")) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
					   *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if (f = mktemp(name))
			return(f);
	}

	if (f = (char *)dir) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
					   *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if (f = mktemp(name))
			return(f);
	}

	f = P_tmpdir;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if (f = mktemp(name))
		return(f);

	f = path_tmp;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if (f = mktemp(name))
		return(f);

	save_errno = errno;
	efree(name);
	errno = save_errno;
	return(NULL);
}
#endif

function_entry php3_file_functions[] = {
	PHP_FE(readfile, NULL)
	PHP_FE(rewind, NULL)
	{"umask",		php3_fileumask,	NULL},
	PHP_FE(rmdir, NULL)
	PHP_FE(fclose, NULL)
	PHP_FE(feof, NULL)
	PHP_FE(fgetc, NULL)
	PHP_FE(fgets, NULL)
	PHP_FE(fgetss, NULL)
	PHP_FE(fpassthru, NULL)
	PHP_FE(fread, NULL)
	PHP_FE(fopen, NULL)
	PHP_FE(fseek, NULL)
	PHP_FE(ftell, NULL)
	PHP_FE(fwrite, NULL)
	PHP_FE(set_file_buffer, NULL)
	{"fputs",		php3_fwrite,	NULL},
	PHP_FE(mkdir, NULL)
	PHP_FE(rename, NULL)
	{"copy",		php3_file_copy,	NULL},
	PHP_FE(tempnam, NULL)
	PHP_FE(tmpfile, NULL)
	PHP_FE(file, NULL)
	PHP_FE(flock, NULL)
	PHP_FE(fgetcsv, NULL)
	PHP_FE(get_meta_tags, NULL)
	PHP_FE(set_socket_blocking, NULL)
#if (0 && defined(HAVE_SYS_TIME_H) && HAVE_SETSOCKOPT && defined(SO_SNDTIMEO) && defined(SO_RCVTIMEO))
	PHP_FE(set_socket_timeout, NULL)
#endif
	/* Can't use the PHP_FE() macro here because of #defines on Windows. */
	{"pclose",		php3_pclose,	NULL},
	{"popen",		php3_popen,	NULL},
	{NULL, NULL, NULL}
};

php3_module_entry php3_file_module_entry = {
	"PHP_file", php3_file_functions, php3_minit_file, NULL, NULL, NULL, NULL, STANDARD_MODULE_PROPERTIES
};

/* {{{ proto array get_meta_tags(string filename [, int use_include_path])
   Extracts all meta tag content attributes from a file and returns an array */
void php3_get_meta_tags(INTERNAL_FUNCTION_PARAMETERS) {
	pval *filename, *arg2;
	FILE *fp;
	char buf[8192];
	int use_include_path = 0;
	int issock=0, socketd=0;
	int len, var_namelen;
	char var_name[50],*val=NULL,*tmp,*end,*slashed;
	TLS_VARS;
	
	/* check args */
	switch (ARG_COUNT(ht)) {
	case 1:
		if (getParameters(ht,1,&filename) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 2:
		if (getParameters(ht,2,&filename,&arg2) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg2);
		use_include_path = arg2->value.lval;
		break;
	default:
		WRONG_PARAM_COUNT;
	}
	convert_to_string(filename);

	fp = php3_fopen_wrapper(filename->value.str.val,"r", use_include_path|ENFORCE_SAFE_MODE, &issock, &socketd);
	if (!fp && !socketd) {
		if (issock != BAD_URL) {
			php3_strip_url_passwd(filename->value.str.val);
			php3_error(E_WARNING,"File(\"%s\") - %s",filename->value.str.val,strerror(errno));
		}
		RETURN_FALSE;
	}

	if (array_init(return_value)==FAILURE) {
		if (issock) {
			SOCK_FCLOSE(socketd);
		} else {
			fclose(fp);
		}
		RETURN_FALSE;
	}
	/* Now loop through the file and do the magic quotes thing if needed */
	memset(buf,0,8191);
	while((issock?SOCK_FGETS(buf,8191,socketd):fgets(buf,8191,fp)) != NULL
		&& !php3i_stristr(buf,"</head>")) {
		if(php3i_stristr(buf,"<meta")) {

			memset(var_name,0,50);
			/* get the variable name from the name attribute of the meta tag */
			tmp=php3i_stristr(buf,"name=\"");
			if(tmp) {
				tmp+=6;
				end=strstr(tmp,"\"");
				if(end) {
					unsigned char *c;
					*end='\0';
					snprintf(var_name,50,"%s",tmp);
					*end='"';

					c = (unsigned char*)var_name;
					while (*c) {
						switch(*c) {
							case '.':
							case '\\':
							case '+':
							case '*':
							case '?':
							case '[':
							case '^':
							case ']':
							case '$':
							case '(':
							case ')':
							case ' ':
								*c++ ='_';
								break;
							default:
								*c++ = tolower((unsigned char)*c);
						}
					}
					var_namelen=strlen(var_name);
				}

				/* get the variable value from the content attribute of the meta tag */
				tmp=php3i_stristr(buf,"content=\"");
				val = NULL;
				if(tmp) {
					tmp+=9;
					end=strstr(tmp,"\"");
					if(end) {
						*end='\0';
						val=estrdup(tmp);
						*end='"';
					}
				}
			}
			if(*var_name && val) {
				if (php3_ini.magic_quotes_runtime) {
					slashed = _php3_addslashes(val,0,&len,0);
				} else {
					slashed = estrndup(val,strlen(val));
				}
				add_assoc_string(return_value, var_name, slashed, 0);
				efree(val);
			}
		}
	}
	if (issock) {
		SOCK_FCLOSE(socketd);
	} else {
		fclose(fp);
	}
}
/* }}} */

/* {{{ proto array file(string filename [, int use_include_path])
   Read entire file into an array */
void php3_file(INTERNAL_FUNCTION_PARAMETERS) {
	pval *filename, *arg2;
	FILE *fp;
	char *slashed, buf[8192];
	register int i=0;
	int use_include_path = 0;

	int issock=0, socketd=0;
	TLS_VARS;
	
	/* check args */
	switch (ARG_COUNT(ht)) {
	case 1:
		if (getParameters(ht,1,&filename) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 2:
		if (getParameters(ht,2,&filename,&arg2) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg2);
		use_include_path = arg2->value.lval;
		break;
	default:
		WRONG_PARAM_COUNT;
	}
	convert_to_string(filename);

	fp = php3_fopen_wrapper(filename->value.str.val,"r", use_include_path|ENFORCE_SAFE_MODE, &issock, &socketd);
	if (!fp && !socketd) {
		if (issock != BAD_URL) {
			php3_strip_url_passwd(filename->value.str.val);
			php3_error(E_WARNING,"File(\"%s\") - %s",filename->value.str.val,strerror(errno));
		}
		RETURN_FALSE;
	}

	/* Initialize return array */
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}	

	/* Now loop through the file and do the magic quotes thing if needed */
	memset(buf,0,8191);
	while((issock?SOCK_FGETS(buf,8191,socketd):fgets(buf,8191,fp)) != NULL) {
		if (php3_ini.magic_quotes_runtime) {
			int len;
			
			slashed = _php3_addslashes(buf,0,&len,0); /* 0 = don't free source string */
            add_index_stringl(return_value, i++, slashed, len, 0);
		} else {
			add_index_string(return_value, i++, buf, 1);
		}
	}
	if (issock) {
		SOCK_FCLOSE(socketd);
	} else {
		fclose(fp);
	}
}
/* }}} */


static void __pclose(FILE *pipe)
{
	TLS_VARS;
	GLOBAL(pclose_ret) = pclose(pipe);
}

static void _php3_closesocket(int *sock) {
	if(sock) {
		SOCK_FCLOSE(*sock);
		efree(sock);
	}
}

static void _php3_unlink_uploaded_file(char *file) {
	if(file) {
		unlink(file);
	}
}

static void php3i_destructor_fclose(FILE *fp) {
	(void)fclose(fp);
}

int php3_minit_file(INIT_FUNC_ARGS)
{
	TLS_VARS;
	
	GLOBAL(le_fp) = register_list_destructors(php3i_destructor_fclose,NULL);
	GLOBAL(le_pp) = register_list_destructors(__pclose,NULL);
	GLOBAL(wsa_fp) = register_list_destructors(_php3_closesocket,NULL);
	GLOBAL(le_uploads) = register_list_destructors(_php3_unlink_uploaded_file,NULL);
	return SUCCESS;
}

/* {{{ proto string tempnam(string dir, string prefix)
   Create a unique filename in a directory */
void php3_tempnam(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1, *arg2;
	char *d;
	char *t;
	char p[64];
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	convert_to_string(arg2);
	d = estrndup(arg1->value.str.val,arg1->value.str.len);
	strncpy(p,arg2->value.str.val,sizeof(p));

	t = tempnam(d,p);
	efree(d);
	if(!t) {
		RETURN_FALSE;
	}
	RETURN_STRING(t,1);
}
/* }}} */

/* {{{ proto int tmpfile(void)
   Create a temporary file that will be deleted automatically after use */
PHP_FUNCTION(tmpfile)
{
	FILE *fp;
	int id;
	if (ARG_COUNT(ht) != 0) {
		WRONG_PARAM_COUNT;
	}
	fp = tmpfile();
	if (fp == NULL) {
		php3_error(E_WARNING, "tmpfile: %s", strerror(errno));
		RETURN_FALSE;
	}
	id = php3_list_insert(fp, GLOBAL(le_fp));
	RETURN_LONG(id);
}
/* }}} */

/* {{{ proto int fopen(string filename, string mode [, int use_include_path])
   Open a file or a URL and return a file pointer */
void php3_fopen(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1, *arg2, *arg3;
	FILE *fp;
	char *p;
	int *sock;
	int id;
	int use_include_path = 0;
	int issock=0, socketd=0;
	TLS_VARS;
	
	switch(ARG_COUNT(ht)) {
	case 2:
		if (getParameters(ht,2,&arg1,&arg2) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 3:
		if (getParameters(ht,3,&arg1,&arg2,&arg3) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg3);
		use_include_path = arg3->value.lval;
		break;
	default:
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	convert_to_string(arg2);
	p = estrndup(arg2->value.str.val,arg2->value.str.len);

	/*
	 * We need a better way of returning error messages from
	 * php3_fopen__wrapper().
	 */
	fp = php3_fopen_wrapper(arg1->value.str.val, p, use_include_path|ENFORCE_SAFE_MODE, &issock, &socketd);
	if (!fp && !socketd) {
		if (issock != BAD_URL) {
			php3_strip_url_passwd(arg1->value.str.val);
			php3_error(E_WARNING,"fopen(\"%s\",\"%s\") - %s",
						arg1->value.str.val, p, strerror(errno));
		}
		efree(p);
		RETURN_FALSE;
	}
	GLOBAL(fgetss_state)=0;
	if (issock) {
		sock=emalloc(sizeof(int));
		*sock=socketd;
		id = php3_list_insert(sock,GLOBAL(wsa_fp));
	} else {
		id = php3_list_insert(fp,GLOBAL(le_fp));
	}
	efree(p);
	RETURN_LONG(id);
}
/* }}} */	

/* {{{ proto int fclose(int fp)
   Close an open file pointer */
void php3_fclose(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	int id, type;
	FILE *fp;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id=arg1->value.lval;
	fp = php3_list_find(id,&type);
	if (!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	php3_list_delete(id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int popen(string command, string mode)
   Execute a command and open either a read or a write pipe to it */
PHP_FUNCTION(popen) {
	pval *arg1, *arg2;
	FILE *fp;
	int id;
	char *p, *tmp = NULL;
	char *b, buf[1024];
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	convert_to_string(arg2);
	p = estrndup(arg2->value.str.val,arg2->value.str.len);
	if (php3_ini.safe_mode){
		b = strchr(arg1->value.str.val,' ');
		if (!b) {
			b = strrchr(arg1->value.str.val,'/');
		} else {
			char *c;
			c = arg1->value.str.val;
			while((*b!='/')&&(b!=c)) b--;
			if (b==c) b=NULL;
		}
		if (b) {
			snprintf(buf,sizeof(buf),"%s%s",php3_ini.safe_mode_exec_dir,b);
		} else {
			snprintf(buf,sizeof(buf),"%s/%s",php3_ini.safe_mode_exec_dir,arg1->value.str.val);
		}
		
		tmp = _php3_escapeshellcmd(buf);
		fp = popen(tmp,p);
		efree(tmp); /* temporary copy, no longer necessary */

		if (!fp) {
			php3_error(E_WARNING,"popen(\"%s\",\"%s\") - %s",buf,p,strerror(errno));
			RETURN_FALSE;
		}
	} else {
		fp = popen(arg1->value.str.val,p);
		if (!fp) {
			php3_error(E_WARNING,"popen(\"%s\",\"%s\") - %s",arg1->value.str.val,p,strerror(errno));
			efree(p);
			RETURN_FALSE;
		}
	}
	id = php3_list_insert(fp,GLOBAL(le_pp));
	efree(p);
	RETURN_LONG(id);
}
/* }}} */

/* {{{ proto int pclose(int fp)
   Close a file pointer opened by popen() */
void php3_pclose(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	int id,type;
	FILE *fp;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id = arg1->value.lval;

	fp = php3_list_find(id,&type);
	if (!fp || type!=GLOBAL(le_pp)) {
		php3_error(E_WARNING,"Unable to find pipe identifier %d",id);
		RETURN_FALSE;
	}
	php3_list_delete(id);
	RETURN_LONG(GLOBAL(pclose_ret));
}
/* }}} */

/* {{{ proto int feof(int fp)
   Test for end-of-file on a file pointer */
void php3_feof(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	FILE *fp;
	int id, type;
	int issock=0;
	int socketd=0, *sock;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id = arg1->value.lval;
	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		/* we're at the eof if the file doesn't exist */
		RETURN_TRUE;
	}
	if ((issock?SOCK_FEOF(socketd):feof(fp))) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

PHPAPI int _php3_set_sock_blocking(int socketd, int block)
{
	int ret = SUCCESS;
	int flags;
	int myflag = 0;
	
#if WIN32|WINNT
	/* with ioctlsocket, a non-zero sets nonblocking, a zero sets blocking */
	flags=block;
	if (ioctlsocket(socketd,FIONBIO,&flags)==SOCKET_ERROR){
		php3_error(E_WARNING,"%s",WSAGetLastError());
		ret = FALSE;
	}
#else
	flags = fcntl(socketd, F_GETFL);
#ifdef O_NONBLOCK
	myflag = O_NONBLOCK; /* POSIX version */
#elif defined(O_NDELAY)
	myflag = O_NDELAY;   /* old non-POSIX version */
#endif
	if(!block) {
		flags |= myflag;
	} else {
		flags &= ~myflag;
	}
	fcntl(socketd,F_SETFL,flags);
#endif
	return ret;
}

/* {{{ proto int set_socket_blocking(int socket descriptor, int mode)
   Set blocking/non-blocking mode on a socket */
void php3_set_socket_blocking(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2;
	int id, type, block;
	int socketd=0, *sock;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	id = arg1->value.lval;
	block = arg2->value.lval;
	
	sock = php3_list_find(id,&type);
	if (type!=GLOBAL(wsa_fp)) {
		php3_error(E_WARNING,"%d is not a socket id",id);
		RETURN_FALSE;
	}
	socketd=*sock;
	if(_php3_set_sock_blocking(socketd, block) == FAILURE)
		RETURN_FALSE;
	_php3_sock_set_blocking(socketd, block == 0 ? 0 : 1);
	RETURN_TRUE;
}
/* }}} */


#if (0 && defined(HAVE_SYS_TIME_H) && HAVE_SETSOCKOPT && defined(SO_SNDTIMEO) && defined(SO_RCVTIMEO))
/* this doesn't work, as it appears those properties are read-only :( */
void php3_set_socket_timeout(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *socket,*timeout;
	int type, *sock;
	struct timeval t;

	if (ARG_COUNT(ht)!=2 || getParameters(ht, 2, &socket, &timeout)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(socket);
	convert_to_long(timeout);
	
	sock = php3_list_find(socket->value.lval, &type);
	if (type!=GLOBAL(wsa_fp)) {
		php3_error(E_WARNING,"%d is not a socket id",socket->value.lval);
		RETURN_FALSE;
	}
	t.tv_sec = timeout->value.lval;
	t.tv_usec = 0;
	setsockopt(*sock,SOL_SOCKET,SO_SNDTIMEO,(void *) &t,sizeof(struct timeval));
	setsockopt(*sock,SOL_SOCKET,SO_RCVTIMEO,(void *) &t,sizeof(struct timeval));
	RETURN_TRUE;
}
#endif

/* {{{ proto string fgets(int fp, int length)
   Get a line from file pointer */
void php3_fgets(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2;
	FILE *fp;
	int id, len, type;
	char *buf;
	int issock=0;
	int *sock, socketd=0;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	id = arg1->value.lval;
	len = arg2->value.lval;

	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	buf = emalloc(sizeof(char) * (len + 1));
	/* needed because recv doesnt put a null at the end*/
	memset(buf,0,len+1);
	if ((issock?SOCK_FGETS(buf,len,socketd):fgets(buf,len,fp)) == NULL) {
		efree(buf);
		RETVAL_FALSE;
	} else {
		if (php3_ini.magic_quotes_runtime) {
			return_value->value.str.val = _php3_addslashes(buf,0,&return_value->value.str.len,1);
		} else {
			return_value->value.str.val = buf;
			return_value->value.str.len = strlen(return_value->value.str.val);
		}
		return_value->type = IS_STRING;
	}
	return;
}
/* }}} */

/* {{{ proto string fgetc(int fp)
   Get a character from file pointer */
void php3_fgetc(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	FILE *fp;
	int id, type;
	char *buf;
	int issock=0;
	int *sock, socketd=0;
	int result;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id = arg1->value.lval;

	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd = *sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	buf = emalloc(sizeof(char) * 2);
#define FP_FGETC(a, b, c) (a?SOCK_FGETC(b):fgetc(c))
	if ((result = FP_FGETC(issock, socketd, fp)) == EOF) {
		efree(buf);
		RETVAL_FALSE;
	} else {
		buf[0]=result;
		buf[1]='\0';
		return_value->value.str.val = buf; 
		return_value->value.str.len = 1; 
		return_value->type = IS_STRING;
	}
}
/* }}} */

/* Strip any HTML tags while reading */
/* {{{ proto string fgetss(int fp, int length [, string allowable_tags])
   Get a line from file pointer and strip HTML tags */
void php3_fgetss(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *fd, *bytes, *allow=NULL;
	FILE *fp;
	int id, len, type;
	char *buf;
	int issock=0;
	int *sock,socketd=0;
	TLS_VARS;
	
	switch(ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &fd, &bytes) == FAILURE) {
				RETURN_FALSE;
			}
			break;
		case 3:
			if (getParameters(ht, 3, &fd, &bytes, &allow) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_string(allow);
			break;
		default:
			WRONG_PARAM_COUNT;
			/* NOTREACHED */
			break;
	}

	convert_to_long(fd);
	convert_to_long(bytes);

	id = fd->value.lval;
	len = bytes->value.lval;

	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING, "Unable to find file identifier %d", id);
		RETURN_FALSE;
	}

	buf = emalloc(sizeof(char) * (len + 1));
	/*needed because recv doesnt set null char at end*/
	memset(buf,0,len+1);
	if (!((issock?SOCK_FGETS(buf,len,socketd):fgets(buf, len, fp)) != NULL)) {
		efree(buf);
		RETURN_FALSE;
	}

	/* strlen() can be used here since we are doing it on the return of an fgets() anyway */
	_php3_strip_tags(buf,strlen(buf),GLOBAL(fgetss_state),allow?allow->value.str.val:NULL);	
	RETURN_STRING(buf,0);
}
/* }}} */

/* {{{ proto int fputs(int fp, string str [, int length])
   An alias for fwrite */
/* }}} */

/* {{{ proto int fwrite(int fp, string str [, int length])
   Binary-safe file write */
void php3_fwrite(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2, *arg3=NULL;
	FILE *fp;
	int ret,id,type;
	int num_bytes;
	int issock=0;
	int *sock, socketd=0;
	TLS_VARS;
	
	switch (ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &arg1, &arg2)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_string(arg2);
			num_bytes = arg2->value.str.len;
			break;
		case 3:
			if (getParameters(ht, 3, &arg1, &arg2, &arg3)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_string(arg2);
			convert_to_long(arg3);
			num_bytes = MIN(arg3->value.lval, arg2->value.str.len);
			break;
		default:
			WRONG_PARAM_COUNT;
			/* NOTREACHED */
			break;
	}				
	convert_to_long(arg1);
	id = arg1->value.lval;	

	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	/* strip slashes only if the length wasn't specified explicitly */
	if (!arg3 && php3_ini.magic_quotes_runtime) {
		_php3_stripslashes(arg2->value.str.val,&num_bytes);
	}

	if (issock){
		ret = SOCK_WRITEL(arg2->value.str.val,num_bytes,socketd);
	} else {
		ret = fwrite(arg2->value.str.val,1,num_bytes,fp);
	}
	RETURN_LONG(ret);
}
/* }}} */	

/* {{{ proto int set_file_buffer(int fp, int buffer)
   Set file write buffer */
/*
   wrapper for setvbuf()
*/
void php3_set_file_buffer(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2;
	FILE *fp;
	int ret,id,type,buff;
	int issock=0;
	int *sock, socketd=0;
	TLS_VARS;
	
  switch (ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &arg1, &arg2)==FAILURE) {
				RETURN_FALSE;
			} 
    	convert_to_long(arg1);
    	convert_to_long(arg2);
      break;
		default:
			WRONG_PARAM_COUNT;
			/* NOTREACHED */
			break;
	}				

	id = arg1->value.lval;	
  buff = arg2->value.lval;	
	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

  /* if buff is 0 then set to non-buffered */
  if(buff==0){
    ret=setvbuf(fp, NULL, _IONBF, 0);
  }
  else{
    ret=setvbuf(fp, NULL, _IOFBF, buff);
  }

	RETURN_LONG(ret);
}
/* }}} */	

/* {{{ proto int rewind(int fp)
   Rewind the position of a file pointer */
void php3_rewind(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	int id,type;
	FILE *fp;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id = arg1->value.lval;	
	fp = php3_list_find(id,&type);
	if (!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	rewind(fp);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftell(int fp)
   Get file pointer's read/write position */
void php3_ftell(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	int id, type;
	long pos;
	FILE *fp;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id = arg1->value.lval;	
	fp = php3_list_find(id,&type);
	if (!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	pos = ftell(fp);
	RETURN_LONG(pos);
}
/* }}} */

/* {{{ proto int fseek(int fp, int offset)
   Seek on a file pointer */
void php3_fseek(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1, *arg2;
	int ret,id,type;
	long pos;
	FILE *fp;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	pos = arg2->value.lval;
	id = arg1->value.lval;
	fp = php3_list_find(id,&type);
	if (!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
/*fseek is flaky on windows, use setfilepointer, but we have to live with
	it until we use win32 api for all the file functions in 3.1 */
#if 0
	ret = SetFilePointer (fp, pos, NULL, FILE_BEGIN);
	if (ret == 0xFFFFFFFF){
		php3_error(E_WARNING,"Unable to move file postition: %s",php3_win_err());
		RETURN_FALSE;
	}
#else
 	ret = fseek(fp,pos,SEEK_SET);
#endif
	RETURN_LONG(ret);
}
/* }}} */

/* {{{ proto int mkdir(string pathname, int mode)
   Create a directory */
void php3_mkdir(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1, *arg2;
	int ret,mode;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	convert_to_long(arg2);
	mode = arg2->value.lval;
	if (php3_ini.safe_mode &&(!_php3_checkuid(arg1->value.str.val,3))) {
		RETURN_FALSE;
	}
	ret = mkdir(arg1->value.str.val,mode);
	if (ret < 0) {
		php3_error(E_WARNING,"MkDir failed (%s)", strerror(errno));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */	

/* {{{ proto int rmdir(string dirname)
   Remove a directory */
void php3_rmdir(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	int ret;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	if (php3_ini.safe_mode &&(!_php3_checkuid(arg1->value.str.val,1))) {
		RETURN_FALSE;
	}
	ret = rmdir(arg1->value.str.val);
	if (ret < 0) {
		php3_error(E_WARNING,"RmDir failed (%s)", strerror(errno));
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */	

/* {{{ proto int readfile(string filename [, int use_include_path])
   Output a file or a URL */
void php3_readfile(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1, *arg2;
	char buf[8192];
	FILE *fp;
	int b, size;
	int use_include_path = 0;

	int issock=0, socketd=0;
	TLS_VARS;
	
	/* check args */
	switch (ARG_COUNT(ht)) {
	case 1:
		if (getParameters(ht,1,&arg1) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 2:
		if (getParameters(ht,2,&arg1,&arg2) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg2);
		use_include_path = arg2->value.lval;
		break;
	default:
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);	

	/*
	 * We need a better way of returning error messages from
	 * php3_fopen_wrapper().
	 */
	fp = php3_fopen_wrapper(arg1->value.str.val,"r", use_include_path|ENFORCE_SAFE_MODE, &issock, &socketd);
	if (!fp && !socketd){
		if (issock != BAD_URL) {
			php3_strip_url_passwd(arg1->value.str.val);
			php3_error(E_WARNING,"ReadFile(\"%s\") - %s",arg1->value.str.val,strerror(errno));
		}
		RETURN_FALSE;
	}
	size= 0;
	if (php3_header()) {  /* force header if not already sent */
		while(issock?(b=SOCK_FREAD(buf,sizeof(buf),socketd)):(b = fread(buf, 1, sizeof(buf), fp)) > 0) {
			PHPWRITE(buf,b);
			size += b ;
		}
	}
	if (issock) {
		SOCK_FCLOSE(socketd);
	} else {
		fclose(fp);
	}
	RETURN_LONG(size);
}
/* }}} */

/* {{{ proto int umask([int mask])
   Return or change the umask */
void php3_fileumask(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	int oldumask;
	int arg_count = ARG_COUNT(ht);
	TLS_VARS;
	
	oldumask = umask(077);

	if (arg_count == 0) {
		umask(oldumask);
	}
	else {
		if (arg_count > 1 || getParameters(ht, 1, &arg1) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg1);
		umask(arg1->value.lval);
	}
	RETURN_LONG(oldumask);
}
/* }}} */

/*
 * Read to EOF on a file descriptor and write the output to stdout.
 */
/* {{{ proto int fpassthru(int fp)
   Output all remaining data from a file pointer */
void php3_fpassthru(INTERNAL_FUNCTION_PARAMETERS) {
	pval *arg1;
	FILE *fp;
	char buf[8192];
	int id, size, b, type;
	int issock=0;
	int socketd=0, *sock;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	id = arg1->value.lval;
	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	size = 0;
	if (php3_header()) { /* force headers if not already sent */
		while(issock?(b=SOCK_FREAD(buf,sizeof(buf),socketd)):(b = fread(buf, 1, sizeof(buf), fp)) > 0) {
			PHPWRITE(buf,b);
			size += b ;
		}
	}
	php3_list_delete(id);
	RETURN_LONG(size);
}
/* }}} */

/* {{{ proto int rename(string old_name, string new_name)
   Rename a file */
void php3_rename(INTERNAL_FUNCTION_PARAMETERS) {
	pval *OLD, *NEW;
	char *old, *new;
	int ret;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &OLD, &NEW) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string(OLD);
	convert_to_string(NEW);

	old = OLD->value.str.val;
	new = NEW->value.str.val;

	if (php3_ini.safe_mode &&(!_php3_checkuid(old,2))) {
		RETURN_FALSE;
	}
	ret = rename(old, new);

	if (ret == -1) {
		php3_error(E_WARNING,
					"Rename failed (%s)", strerror(errno));
		RETURN_FALSE;
	}

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int copy(string source_file, string destination_file)
   Copy a file */
void php3_file_copy(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *source, *target;
	char buffer[8192];
	int fd_s,fd_t,read_bytes;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &source, &target) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string(source);
	convert_to_string(target);

	if (php3_ini.safe_mode &&(!_php3_checkuid(source->value.str.val,2))) {
		RETURN_FALSE;
	}
	
#if WIN32|WINNT
	if ((fd_s=open(source->value.str.val,O_RDONLY|_O_BINARY))==-1) {
#else
	if ((fd_s=open(source->value.str.val,O_RDONLY))==-1) {
#endif
		php3_error(E_WARNING,"Unable to open '%s' for reading:  %s",source->value.str.val,strerror(errno));
		RETURN_FALSE;
	}
#if WIN32|WINNT
	if ((fd_t=open(target->value.str.val,_O_WRONLY|_O_CREAT|_O_TRUNC|_O_BINARY,_S_IREAD|_S_IWRITE))==-1){
#else
	if ((fd_t=creat(target->value.str.val,0777))==-1) {
#endif
		php3_error(E_WARNING,"Unable to create '%s':  %s", target->value.str.val,strerror(errno));
		close(fd_s);
		RETURN_FALSE;
	}

	while ((read_bytes=read(fd_s,buffer,8192))!=-1 && read_bytes!=0) {
		if (write(fd_t,buffer,read_bytes)==-1) {
			php3_error(E_WARNING,"Unable to write to '%s':  %s",target->value.str.val,strerror(errno));
			close(fd_s);
			close(fd_t);
			RETURN_FALSE;
		}
	}
	
	close(fd_s);
	close(fd_t);

	RETVAL_TRUE;
}
/* }}} */

/* {{{ proto int fread(int fp, int length)
   Binary-safe file read */
void php3_fread(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2;
	FILE *fp;
	int id, len, type;
	int issock=0;
	int *sock, socketd=0;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(arg1);
	convert_to_long(arg2);
	id = arg1->value.lval;
	len = arg2->value.lval;

	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	return_value->value.str.val = emalloc(sizeof(char) * (len + 1));
	/* needed because recv doesnt put a null at the end*/
	
	if (!issock) {
		return_value->value.str.len = fread(return_value->value.str.val, 1, len, fp);
		return_value->value.str.val[return_value->value.str.len] = 0;
	} else {
		return_value->value.str.len = SOCK_FREAD(return_value->value.str.val, len, socketd);
	}
	if (php3_ini.magic_quotes_runtime) {
		return_value->value.str.val = _php3_addslashes(return_value->value.str.val,return_value->value.str.len,&return_value->value.str.len,1);
	}
	return_value->type = IS_STRING;
}
/* }}} */

/* aparently needed for pdf to be compiled as a module under windows */
PHPAPI int php3i_get_le_fp(void){
	TLS_VARS;
	return GLOBAL(le_fp);
}

static int flock_values[] = { LOCK_SH, LOCK_EX, LOCK_UN };

/* {{{ proto bool flock(int fp, int operation)
   Portable file locking */
PHP_FUNCTION(flock)
{
	pval *arg1, *arg2;
	FILE *fp;
	int type;
	int issock=0;
	int *sock, fd=0;
	int act = 0;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_long(arg2);

	fp = php3_list_find(arg1->value.lval, &type);
	if (type == GLOBAL(wsa_fp)){
		issock = 1;
		sock = php3_list_find(arg1->value.lval, &type);
		fd = *sock;
	}
	
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!fd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING,"Unable to find file identifier %d",arg1->value.lval);
		RETURN_FALSE;
	}
	
	if (!issock) {
		fd = fileno(fp);
	}
	
	act = arg2->value.lval & 3;
	if(act < 1 || act > 3) {
			php3_error(E_WARNING, "illegal value for second argument");
			RETURN_FALSE;
	}
	/* flock_values contains all possible actions 
	   if (arg2 & 4) we won't block on the lock */
	act = flock_values[act - 1] | (arg2->value.lval & 4 ? LOCK_NB : 0);
	if (flock(fd, act) == -1) {
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto array fgetcsv(int fp, int length)
   Get line from file pointer and parse for CSV fields */
void php3_fgetcsv(INTERNAL_FUNCTION_PARAMETERS) {
	char *temp, *tptr, *bptr;
	char delimiter = ',';	/* allow this to be set as parameter */

	/* first section exactly as php3_fgetss */

	pval *fd, *bytes, *p_delim;
	FILE *fp;
	int id, len, type;
	char *buf, *lineEnd;
	int issock=0;
	int *sock,socketd=0;
	TLS_VARS;

	switch(ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &fd, &bytes) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
		break;
		
		case 3:
			if (getParameters(ht, 3, &fd, &bytes, &p_delim) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
			convert_to_string(p_delim);
			/* Make sure that there is at least one character in string */
			if (p_delim->value.str.len < 1) {
				WRONG_PARAM_COUNT;
			}
			/* use first character from string */
			delimiter = p_delim->value.str.val[0];
		break;
		
		default:
			WRONG_PARAM_COUNT;
		break;
	}

	convert_to_long(fd);
	convert_to_long(bytes);

	id = fd->value.lval;
	len = bytes->value.lval;

	fp = php3_list_find(id,&type);
	if (type==GLOBAL(wsa_fp)){
		issock=1;
		sock = php3_list_find(id,&type);
		socketd=*sock;
	}
	if ((!fp || (type!=GLOBAL(le_fp) && type!=GLOBAL(le_pp))) && (!socketd || type!=GLOBAL(wsa_fp))) {
		php3_error(E_WARNING, "Unable to find file identifier %d", id);
		RETURN_FALSE;
	}

	buf = emalloc(sizeof(char) * (len + 1));
	/*needed because recv doesnt set null char at end*/
	memset(buf,0,len+1);
	if ((issock?SOCK_FGETS(buf,len,socketd):fgets(buf, len, fp)) == NULL) {
		efree(buf);
		RETURN_FALSE;
	}

	/* Now into new section that parses buf for comma/quote delimited fields */

	/* Strip trailing space from buf, saving end of line in case required for quoted field */

	lineEnd = emalloc(sizeof(char) * (len + 1));
	bptr = buf;
	tptr = buf + strlen(buf) -1;
	while ( isspace(*tptr) && (tptr > bptr) ) tptr--;
	tptr++;
	strcpy(lineEnd, tptr);

	/* add single space - makes it easier to parse trailing null field */
	*tptr++ = ' ';
	*tptr = 0;

	/* reserve workspace for building each individual field */

	temp = emalloc(sizeof(char) * len);	/*	unlikely but possible! */
	tptr=temp;

	/* Initialize return array */
	if (array_init(return_value) == FAILURE) {
		efree(lineEnd);
		efree(temp);
		efree(buf);
		RETURN_FALSE;
	}

	/* Main loop to read CSV fields */
	/* NB this routine will return a single null entry for a blank line */

	do	{
		/* 1. Strip any leading space */		
		while(isspace(*bptr)) bptr++;	
		/* 2. Read field, leaving bptr pointing at start of next field */
		if (*bptr == '"') {
			/* 2A. handle quote delimited field */
			bptr++;		/* move on to first character in field */
			while (*bptr) {
				if (*bptr == '"') {
					/* handle the double-quote */
					if ( *(bptr+1) == '"') {
					/* embedded double quotes */
						*tptr++ = *bptr; bptr +=2;
					} else {
					/* must be end of string - skip to start of next field or end */
						while ( (*bptr != delimiter) && *bptr ) bptr++;
						if (*bptr == delimiter) bptr++;
						*tptr=0;	/* terminate temporary string */
						break;	/* .. from handling this field - resumes at 3. */
					}
				} else {
				/* normal character */
					*tptr++ = *bptr++;

					if (*bptr == 0)	{	/* embedded line end? */
						*(tptr-1)=0;		/* remove space character added on reading line */
						strcat(temp,lineEnd);	/* add the embedded line end to the field */

							/* read a new line from input, as at start of routine */
						memset(buf,0,len+1);
						if (!(issock?SOCK_FGETS(buf,len,socketd):fgets(buf, len, fp))) {
							efree(lineEnd); efree(temp); efree(buf);
							RETURN_FALSE;
							}
						bptr = buf;
						tptr = buf + strlen(buf) -1;
						while ( isspace(*tptr) && (tptr > bptr) ) tptr--;
						tptr++; strcpy(lineEnd, tptr);
						*tptr++ = ' ';	*tptr = 0;

						tptr=temp;	/* reset temp pointer to end of field as read so far */
						while (*tptr) tptr++;
					}
				}
			}
		} else {
			/* 2B. Handle non-quoted field */
			while ( (*bptr != delimiter) && *bptr ) *tptr++ = *bptr++;
			*tptr=0;	/* terminate temporary string */
			if (strlen(temp)) {
				tptr--;
				while (isspace(*tptr)) *tptr-- = 0;	/* strip any trailing spaces */
			}
			if (*bptr == delimiter) bptr++;
				
		}
		/* 3. Now pass our field back to php */
		add_next_index_string(return_value, temp, 1);
		tptr=temp;
	} while (*bptr);
	efree(lineEnd);
	efree(temp);
	efree(buf);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * End:
 */
