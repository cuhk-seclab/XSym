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
   | Author: Rasmus Lerdorf   <rasmus@php.net>                            |
   +----------------------------------------------------------------------+
 */
/* $Id: exec.c,v 1.92 2000/03/20 14:23:12 andrei Exp $ */
#include <stdio.h>
#include "php.h"
#include <ctype.h>
#include "internal_functions.h"
#include "php3_string.h"
#include "safe_mode.h"
#include "head.h"
#include "exec.h"

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/*
 * If type==0, only last line of output is returned (exec)
 * If type==1, all lines will be printed and last lined returned (system)
 * If type==2, all lines will be saved to given array (exec with &$array)
 * If type==3, output will be printed binary, no lines will be saved or returned (passthru)
 *
 */
static int _Exec(int type, char *cmd, pval *array, pval *return_value)
{
	FILE *fp;
	char *buf, *tmp=NULL;
    int buflen=0;
	int t, l, ret, output=1;
	int overflow_limit, lcmd, ldir;
	char *b, *c, *d=NULL;
	TLS_VARS;

    buf = (char*) emalloc(EXEC_INPUT_BUF);
    if (!buf) {
		php3_error(E_WARNING, "Unable to emalloc %d bytes", EXEC_INPUT_BUF);
		return -1;
    }
    buflen = EXEC_INPUT_BUF;

#ifdef WIN32
	(void)AllocConsole(); /* We don't care if this fails. */
#endif

	if (php3_ini.safe_mode) {
		lcmd = strlen(cmd);
		ldir = strlen(php3_ini.safe_mode_exec_dir);
		l = lcmd + ldir + 2;
		overflow_limit = l;
		c = strchr(cmd, ' ');
		if (c) *c = '\0';
		if (strstr(cmd, "..")) {
			php3_error(E_WARNING, "No '..' components allowed in path");
            efree(buf);
			return -1;
		}
		d = emalloc(l);
		strcpy(d, php3_ini.safe_mode_exec_dir);
		overflow_limit -= ldir;
		b = strrchr(cmd, '/');
		if (b) {
			strcat(d, b);
			overflow_limit -= strlen(b);
		} else {
			strcat(d, "/");
			strcat(d, cmd);
			overflow_limit-=(strlen(cmd)+1);
		}
		if (c) {
			*c = ' ';
			strncat(d, c, overflow_limit);
		}
		tmp = _php3_escapeshellcmd(d);
		efree(d);
		d = tmp;
#if WIN32|WINNT
		fp = popen(d, "rb");
#else
		fp = popen(d, "r");
#endif
		if (!fp) {
			php3_error(E_WARNING, "Unable to fork [%s]", d);
			efree(d);
            efree(buf);
			return -1;
		}
	} else { /* not safe_mode */
#if WIN32|WINNT
		fp = popen(cmd, "rb");
#else
		fp = popen(cmd, "r");
#endif
		if (!fp) {
			php3_error(E_WARNING, "Unable to fork [%s]", cmd);
            efree(buf);
			return -1;
		}
	}
	buf[0] = '\0';
	if (type == 1 || type == 3) {
		output=php3_header();
	}
	if (type==2) {
		if (array->type != IS_ARRAY) {
			pval_destructor(array _INLINE_TLS);
			array_init(array);
		}
	}
	if (type != 3) {
		l = 0;
		while ( !feof(fp) || l != 0 ) {
			l = 0;
			/* Read a line or fill the buffer, whichever comes first */
			do {
				if ( buflen <= (l+1) ) {
					buf = erealloc(buf, buflen + EXEC_INPUT_BUF);
					if ( buf == NULL ) {
						php3_error(E_WARNING, "Unable to erealloc %d bytes", 
								buflen + EXEC_INPUT_BUF);
						return -1;
					}
					buflen += EXEC_INPUT_BUF;
				}

				if ( fgets(&(buf[l]), buflen - l, fp) == NULL ) {
					/* eof */
					break;
				}
				l += strlen(&(buf[l]));
			} while ( (l > 0) && (buf[l-1] != '\n') );

			if ( feof(fp) && (l == 0) ) {
				break;
			}

			if (type == 1) {
				if (output) PUTS(buf);
#if APACHE
#  if MODULE_MAGIC_NUMBER > 19970110
				if (output) rflush(GLOBAL(php3_rqst));
#  else
				if (output) bflush(GLOBAL(php3_rqst)->connection->client);
#  endif
#endif
#if CGI_BINARY
				fflush(stdout);
#endif
#if FHTTPD
                               /* fhttpd doesn't flush */
#endif
#if USE_SAPI
				GLOBAL(sapi_rqst)->flush(GLOBAL(sapi_rqst)->scid);
#endif
			}
			else if (type == 2) {
				pval tmp;
			
				/* strip trailing whitespaces */	
				l = strlen(buf);
				t = l;
				while (l-- && isspace((int)buf[l]));
				if (l < t) buf[l + 1] = '\0';
				tmp.value.str.len = strlen(buf);
				tmp.value.str.val = estrndup(buf,tmp.value.str.len);
				tmp.type = IS_STRING;
				_php3_hash_next_index_insert(array->value.ht,(void *) &tmp, sizeof(pval), NULL);
			}
		}

		/* strip trailing spaces */
		l = strlen(buf);
		t = l;
		while (l && isspace((int)buf[--l]));
		if (l < t) buf[l + 1] = '\0';

		/* Return last line from the shell command */
		if(php3_ini.magic_quotes_runtime) {
			int len;
		
			tmp = _php3_addslashes(buf, 0, &len, 0);
			RETVAL_STRINGL(tmp,len,0);
		} else {
			RETVAL_STRINGL(buf,l+1,1);
		}
	} else {
		int b, i;

		while ((b = fread(buf, 1, buflen, fp)) > 0) {
			for (i = 0; i < b; i++)
				if (output) PUTC(buf[i]);
		}
	}

	ret = pclose(fp);
#ifdef HAVE_SYS_WAIT_H
	if (WIFEXITED(ret)) {
		ret = WEXITSTATUS(ret);
	}
#endif

	if (d) efree(d);
    efree(buf);
	return ret;
}

/* {{{ proto int exec(string command [, array output [, int return_value]])
   Execute an external program */
void php3_exec(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2, *arg3;
	int arg_count = ARG_COUNT(ht);
	int ret;

	if (arg_count > 3 || getParameters(ht, arg_count, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	switch (arg_count) {
		case 1:
			ret = _Exec(0, arg1->value.str.val, NULL, return_value);
			break;
		case 2:
			if (!ParameterPassedByReference(ht,2)) {
				php3_error(E_WARNING,"Array argument to exec() not passed by reference");
			}
			ret = _Exec(2, arg1->value.str.val, arg2, return_value);
			break;
		case 3:
			if (!ParameterPassedByReference(ht,2)) {
				php3_error(E_WARNING,"Array argument to exec() not passed by reference");
			}
			if (!ParameterPassedByReference(ht,3)) {
				php3_error(E_WARNING,"return_status argument to exec() not passed by reference");
			}
			ret = _Exec(2, arg1->value.str.val, arg2, return_value);
			arg3->type = IS_LONG;
			arg3->value.lval=ret;
			break;
	}
}
/* }}} */

/* {{{ proto int system(string command [, int return_value])
   Execute an external program and display output */
void php3_system(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2;
	int arg_count = ARG_COUNT(ht);
	int ret;

	if (arg_count > 2 || getParameters(ht, arg_count, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	switch (arg_count) {
		case 1:
			ret = _Exec(1, arg1->value.str.val, NULL, return_value);
			break;
		case 2:
			if (!ParameterPassedByReference(ht,2)) {
				php3_error(E_WARNING,"return_status argument to system() not passed by reference");
			}
			ret = _Exec(1, arg1->value.str.val, NULL, return_value);
			arg2->type = IS_LONG;
			arg2->value.lval=ret;
			break;
	}
}
/* }}} */

/* {{{ proto void passthru(string command [, int return_value])
   Execute an external program and display raw output */
void php3_passthru(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1, *arg2;
	int arg_count = ARG_COUNT(ht);
	int ret;

	if (arg_count > 2 || getParameters(ht, arg_count, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	switch (arg_count) {
		case 1:
			ret = _Exec(3, arg1->value.str.val, NULL, return_value);
			break;
		case 2:
			if (!ParameterPassedByReference(ht,2)) {
				php3_error(E_WARNING,"return_status argument to system() not passed by reference");
			}
			ret = _Exec(3, arg1->value.str.val, NULL, return_value);
			arg2->type = IS_LONG;
			arg2->value.lval=ret;
			break;
	}
}
/* }}} */

static int php3_ind(char *s, char c)
{
	register int x;

	for (x = 0; s[x]; x++)
		if (s[x] == c)
			return x;

	return -1;
}

/* Escape all chars that could possibly be used to
   break out of a shell command

   This function emalloc's a string and returns the pointer.
   Remember to efree it when done with it.

   *NOT* safe for binary strings
*/
char * _php3_escapeshellcmd(char *str) {
	register int x, y, l;
	char *cmd;

	l = strlen(str);
	cmd = emalloc(2 * l + 1);
	strcpy(cmd, str);
	for (x = 0; cmd[x]; x++) {
		if (php3_ind("&;`'\"|*?~<>^()[]{}$\\\x0A\xFF", cmd[x]) != -1) {
			for (y = l + 1; y > x; y--)
				cmd[y] = cmd[y - 1];
			l++;				/* length has been increased */
			cmd[x] = '\\';
			x++;				/* skip the character */
		}
	}
	return cmd;
}

/* {{{ proto string escapeshellcmd(string command)
   Escape shell metacharacters */
void php3_escapeshellcmd(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1;
	char *cmd;
	TLS_VARS;

	if (getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	cmd = _php3_escapeshellcmd(arg1->value.str.val);

	RETVAL_STRING(cmd,1);
	efree(cmd);
}
/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
