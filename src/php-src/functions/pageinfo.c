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
   | Authors: Jim Winstead <jimw@php.net>                                 |
   +----------------------------------------------------------------------+
 */
/* $Id: pageinfo.c,v 1.44 2000/01/01 04:31:16 sas Exp $ */
#include "php.h"
#include "pageinfo.h"

#include <stdio.h>
#include <stdlib.h>
#if HAVE_PWD_H
#if MSVC5
#include "win32/pwd.h"
#else
#include <pwd.h>
#endif
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/stat.h>
#if MSVC5
#include <process.h>
#endif

static long page_uid   = -1;
static long page_inode = -1;
static long page_mtime = -1;

static void _php3_statpage(void)
{
#if !APACHE
	char *path;
	struct stat sb;
#endif
	TLS_VARS;
	
#if APACHE
	/* Apache has already gone through the trouble of doing
	   the stat(), so the only real overhead is copying three
	   values. We can afford it, and it means we don't have to
	   worry about resetting the static variables after every
	   hit. */
	GLOBAL(page_uid)   = GLOBAL(php3_rqst)->finfo.st_uid;
	GLOBAL(page_inode) = GLOBAL(php3_rqst)->finfo.st_ino;
	GLOBAL(page_mtime) = GLOBAL(php3_rqst)->finfo.st_mtime;
#else
	if (GLOBAL(page_uid) == -1) {
		path = GLOBAL(request_info).filename;
		if (path != NULL) {
			if (stat(path, &sb) == -1) {
				php3_error(E_WARNING, "Unable to find file:  '%s'", path);
				return;
			}
			GLOBAL(page_uid)   = sb.st_uid;
			GLOBAL(page_inode) = sb.st_ino;
			GLOBAL(page_mtime) = sb.st_mtime;
		}
	}
#endif
}

long _php3_getuid(void)
{
	TLS_VARS;
	_php3_statpage();
	return (GLOBAL(page_uid));
}

/* {{{ proto int getmyuid(void)
   Get PHP script owner's UID */
void php3_getmyuid(INTERNAL_FUNCTION_PARAMETERS)
{
	long uid;
	TLS_VARS;
	
	uid = _php3_getuid();
	if (uid < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(uid);
	}
}
/* }}} */

/* {{{ proto int getmypid(void)
   Get current process ID */
void php3_getmypid(INTERNAL_FUNCTION_PARAMETERS)
{
	int pid;
	TLS_VARS;
	
	pid = getpid();
	if (pid < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG((long) pid);
	}
}
/* }}} */

/* {{{ proto int getmyinode(void)
   Get the inode of the current script being parsed */
void php3_getmyinode(INTERNAL_FUNCTION_PARAMETERS)
{
	TLS_VARS;
	
	_php3_statpage();
	if (GLOBAL(page_inode) < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(GLOBAL(page_inode));
	}
}
/* }}} */

/* {{{ proto int getlastmod(void)
   Get time of last page modification */
void php3_getlastmod(INTERNAL_FUNCTION_PARAMETERS)
{
	time_t mtime;

	mtime = _php3_getlastmod();
	if (mtime < 0) {
		RETURN_FALSE;
	} else {
		RETURN_LONG(mtime);
	}
}
/* }}} */

PHPAPI time_t _php3_getlastmod(void)
{
	TLS_VARS;

	_php3_statpage();
	return GLOBAL(page_mtime);
}
