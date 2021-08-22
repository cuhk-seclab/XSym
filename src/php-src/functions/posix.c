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
   | Authors: Kristian Koehntopp (kris@koehntopp.de)                      |
   +----------------------------------------------------------------------+
 */
 
/* $Id: posix.c,v 1.19 2000/05/31 00:21:02 sas Exp $ */


#include "php.h"
#include "internal_functions.h"
#include "php3_string.h"
#include "php3_posix.h"

#if HAVE_POSIX
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <unistd.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/times.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include "php3_list.h"

/* OS/2's gcc defines _POSIX_PATH_MAX but not PATH_MAX */
#if !defined(PATH_MAX) && defined(_POSIX_PATH_MAX)
#define PATH_MAX _POSIX_PATH_MAX
#endif

#define SAFE_STRING(s) ((s)?(s):"")

function_entry posix_functions[] = {
    /* POSIX.1, 3.3 */
	{"posix_kill",          php3_posix_kill,    NULL},

	/* POSIX.1, 4.1 */
	{"posix_getpid",		php3_posix_getpid,	NULL},
	{"posix_getppid",		php3_posix_getppid,	NULL},

	/* POSIX.1,  4.2 */
	{"posix_getuid",		php3_posix_getuid,	NULL},
	{"posix_geteuid",		php3_posix_geteuid,	NULL},
	{"posix_getgid",		php3_posix_getgid,	NULL},
	{"posix_getegid",		php3_posix_getegid,	NULL},
	{"posix_setuid",		php3_posix_setuid,	NULL},
	{"posix_setgid",		php3_posix_setgid,	NULL},
	{"posix_getgroups",		php3_posix_getgroups,	NULL},
	{"posix_getlogin",		php3_posix_getlogin,	NULL},

	/* POSIX.1, 4.3 */
	{"posix_getpgrp",		php3_posix_getpgrp,	NULL},
	{"posix_setsid",		php3_posix_setsid,	NULL},
	{"posix_setpgid",		php3_posix_setpgid,	NULL},
	/* Non-Posix functions which are common */
	{"posix_getpgid",		php3_posix_getpgid,	NULL},
	{"posix_getsid",		php3_posix_getsid,	NULL},

	/* POSIX.1, 4.4 */
	{"posix_uname",			php3_posix_uname,	NULL},

	/* POSIX.1, 4.5 */
	{"posix_times",			php3_posix_times,	NULL},

	/* POSIX.1, 4.7 */
	{"posix_ctermid",		php3_posix_ctermid,	NULL},
	{"posix_ttyname",		php3_posix_ttyname,	NULL},
	{"posix_isatty",		php3_posix_isatty,	NULL},

    /* POSIX.1, 5.2 */
	{"posix_getcwd",        php3_posix_getcwd, NULL},

	/* POSIX.1, 5.4 */
	{"posix_mkfifo",		php3_posix_mkfifo, NULL},

	/* POSIX.1, 9.2 */
	{"posix_getgrnam",		php3_posix_getgrnam,	NULL},
	{"posix_getgrgid",		php3_posix_getgrgid,	NULL},
	{"posix_getpwnam",		php3_posix_getpwnam,	NULL},
	{"posix_getpwuid",		php3_posix_getpwuid,	NULL},

	{"posix_getrlimit",		php3_posix_getrlimit,	NULL},

	{NULL, NULL, NULL}
};

php3_module_entry posix_module_entry = {
	"Posix", 
	posix_functions, 
	php3_minit_posix, 
	php3_mshutdown_posix, 
	NULL,
	NULL, 
	php3_info_posix, 
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL
DLEXPORT php3_module_entry *get_module(void) { return &posix__module_entry; }
#endif

#if APACHE
extern void timeout(int sig);
#endif

int php3_minit_posix(INIT_FUNC_ARGS)
{
	return SUCCESS;
}


int php3_mshutdown_posix(void){
	return SUCCESS;
}


void php3_info_posix(void)
{
    php3_printf("$Revision: 1.19 $\n");
}

/* {{{ proto int posix_kill(int pid, int sig)
   Send a signal to a process (POSIX.1, 3.3.2) */

void php3_posix_kill(INTERNAL_FUNCTION_PARAMETERS)
{
	pval   *pid;
	pval   *sig;
	int     result;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &pid, &sig)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(pid);                        
	convert_to_long(sig);
  
	result = kill(pid->value.lval, sig->value.lval);
	if (result< 0) {
		php3_error(E_WARNING, "posix_kill(%d, %d) failed with '%s'",
    		pid->value.lval,
			sig->value.lval,
			strerror(errno));
		RETURN_FALSE;
  	}
  	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto long posix_getpid(void) 
   Get the current process id (POSIX.1, 4.1.1) */
void php3_posix_getpid(INTERNAL_FUNCTION_PARAMETERS)
{
	pid_t  pid;

	pid = getpid();
	RETURN_LONG(pid);
}
/* }}} */

/* {{{ proto long posix_getppid(void) 
   Get the parent process id (POSIX.1, 4.1.1) */
void php3_posix_getppid(INTERNAL_FUNCTION_PARAMETERS)
{
	pid_t  ppid;

	ppid = getppid();
	RETURN_LONG(ppid);
}
/* }}} */

/* {{{ proto long posix_getuid(void) 
   Get the current user id (POSIX.1, 4.2.1) */
void php3_posix_getuid(INTERNAL_FUNCTION_PARAMETERS)
{
	uid_t  uid;

	uid = getuid();
	RETURN_LONG(uid);
}
/* }}} */

/* {{{ proto long posix_getgid(void) 
   Get the current group id (POSIX.1, 4.2.1) */
void php3_posix_getgid(INTERNAL_FUNCTION_PARAMETERS)
{
	gid_t  gid;

	gid = getgid();
	RETURN_LONG(gid);
}
/* }}} */

/* {{{ proto long posix_geteuid(void) 
   Get the current effective user id (POSIX.1, 4.2.1) */
void php3_posix_geteuid(INTERNAL_FUNCTION_PARAMETERS)
{
	uid_t  uid;

	uid = geteuid();
	RETURN_LONG(uid);
}
/* }}} */

/* {{{ proto long posix_getegid(void) 
   Get the current effective group id (POSIX.1, 4.2.1) */
void php3_posix_getegid(INTERNAL_FUNCTION_PARAMETERS)
{
	gid_t  gid;

	gid = getegid();
	RETURN_LONG(gid);
}
/* }}} */

/* {{{ proto long posix_setuid(long uid)
   Set user id (POSIX.1, 4.2.2) */
void php3_posix_setuid(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *uid;
	int   result;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &uid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(uid);
  
	result = setuid(uid->value.lval);
	if (result < 0) {
		php3_error(E_WARNING, "posix_setuid(%d) failed with '%s'. Must be root",
	    	uid->value.lval,
			strerror(errno));
			RETURN_FALSE;
	}
	
	RETURN_TRUE;                                  
}
/* }}} */

/* {{{ proto long posix_setgid(long uid)
   Set group id (POSIX.1, 4.2.2) */
void php3_posix_setgid(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *gid;
	int   result;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &gid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(gid);
  
	result = setgid(gid->value.lval);
	if (result < 0) {
		php3_error(E_WARNING, "posix_setgid(%d) failed with '%s'. Must be root",
	    	gid->value.lval,
			strerror(errno));
			RETURN_FALSE;
	}
	
	RETURN_TRUE;                                  
}
/* }}} */

/* {{{ proto long posix_getgroups(void) 
   Get supplementary group id's (POSIX.1, 4.2.3) */
void php3_posix_getgroups(INTERNAL_FUNCTION_PARAMETERS)
{
	gid_t  gidlist[NGROUPS_MAX];
	int    result;
	int    i;

	result = getgroups(NGROUPS_MAX, gidlist);
	if (result < 0) {
		php3_error(E_WARNING, "posix_getgroups() failed with '%s'",
			strerror(errno));
		RETURN_FALSE;
	}

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	for (i=0; i<result; i++) {
		add_next_index_long(return_value,gidlist[i]);
	}
}
/* }}} */

/* {{{ proto string posix_getlogin(void) 
   Get user name (POSIX.1, 4.2.4) */
void php3_posix_getlogin(INTERNAL_FUNCTION_PARAMETERS)
{
	char *p;
	
	p = getlogin();
	if (p == NULL) {
		php3_error(E_WARNING, "Cannot determine your login name. Something is really wrong here.");
		RETURN_FALSE;
	}
	
	RETURN_STRING(p, 1);
}
/* }}} */

/* {{{ proto long posix_getpgrp(void) 
   Get current process group id (POSIX.1, 4.3.1) */
void php3_posix_getpgrp(INTERNAL_FUNCTION_PARAMETERS)
{
	pid_t  pgrp;

	pgrp = getpgrp();
	RETURN_LONG(pgrp);
}
/* }}} */

/* {{{ proto long posix_setsid(void) 
   Create session and set process group id (POSIX.1, 4.3.2) */
void php3_posix_setsid(INTERNAL_FUNCTION_PARAMETERS)
{
#if HAVE_SETSID
	pid_t  sid;

	sid = setsid();
	RETURN_LONG(sid);
#else
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ proto long posix_setpgid(long pid, long pgid) 
   Set process group id for job control (POSIX.1, 4.3.3) */
void php3_posix_setpgid(INTERNAL_FUNCTION_PARAMETERS)
{
	pval   *pid;
	pval   *pgid;
	int     result;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &pid, &pgid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(pid);
	convert_to_long(pgid);
  
	result = setpgid(pid->value.lval, pgid->value.lval);
	if (result< 0) {
		php3_error(E_WARNING, "posix_setpgid(%d, %d) failed with '%s'",
    		pid->value.lval,
			pgid->value.lval,
			strerror(errno));
		RETURN_FALSE;
  	}
  	
	RETURN_LONG(result);
}
/* }}} */

/* {{{ proto long posix_getpgid(void) 
   Get the process group id of the specified process (This is not a POSIX function, but a SVR4ism, so we compile conditionally) */
void php3_posix_getpgid(INTERNAL_FUNCTION_PARAMETERS)
{
#if HAVE_GETPGID
	pid_t  pgid;
	pval  *pid;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &pid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(pid);
	pgid = getpgid(pid->value.lval);
	if (pgid < 0) {
		php3_error(E_WARNING, "posix_getpgid(%d) failed with '%s'", 
			pid->value.lval,
			strerror(errno));
		RETURN_FALSE;
	}

	return_value->type= IS_LONG;
	return_value->value.lval = pgid;
#else
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ proto long posix_getsid(void) 
   Get process group id of session leader (This is not a POSIX function, but a SVR4ism, so be compile conditionally) */
void php3_posix_getsid(INTERNAL_FUNCTION_PARAMETERS)
{
#if HAVE_GETSID
	pid_t  sid;
	pval  *pid;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &pid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(pid);
	sid = getsid(pid->value.lval);
	if (sid < 0) {
		php3_error(E_WARNING, "posix_getsid(%d) failed with '%s'", 
			pid->value.lval,
			strerror(errno));
		RETURN_FALSE;
	}

	return_value->type= IS_LONG;
	return_value->value.lval = sid;
#else
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ proto array posix_uname(void) 
   Get system name (POSIX.1, 4.4.1) */
void php3_posix_uname(INTERNAL_FUNCTION_PARAMETERS)
{
	struct utsname u;

	uname(&u);

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}
	add_assoc_string(return_value, "sysname",  u.sysname,  strlen(u.sysname));
	add_assoc_string(return_value, "nodename", u.nodename, strlen(u.nodename));
    add_assoc_string(return_value, "release",  u.release,  strlen(u.release));
    add_assoc_string(return_value, "version",  u.version,  strlen(u.version));
    add_assoc_string(return_value, "machine",  u.machine,  strlen(u.machine));
}
/* }}} */

/* POSIX.1, 4.5.1 time() - Get System Time
							already covered by PHP3
 */

/* {{{ proto array posix_times(void) 
   Get process times (POSIX.1, 4.5.2) */
void php3_posix_times(INTERNAL_FUNCTION_PARAMETERS)
{
	struct tms t;
	clock_t    ticks;

	ticks = times(&t);
	if (ticks < 0) {
		php3_error(E_WARNING, "posix_times failed with '%s'",
			strerror(errno));
	}

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}
	add_assoc_long(return_value, "ticks",		ticks);
	add_assoc_long(return_value, "utime",	t.tms_utime);
    add_assoc_long(return_value, "stime",	t.tms_stime);
    add_assoc_long(return_value, "cutime",	t.tms_cutime);
    add_assoc_long(return_value, "cstime",	t.tms_cstime);
}
/* }}} */

/* POSIX.1, 4.6.1 getenv() - Environment Access
							already covered by PHP3
*/

/* {{{ proto string posix_ctermid(void) 
   Generate terminal path name (POSIX.1, 4.7.1) */
void php3_posix_ctermid(INTERNAL_FUNCTION_PARAMETERS)
{
#if HAVE_CTERMID
	char  buffer[L_ctermid];
	char *p;
	
	p = ctermid(buffer);
	if (p == NULL) {
		php3_error(E_WARNING, "posix_ctermid() failed with '%s'",
			strerror(errno));
		RETURN_FALSE;
	}
	
	RETURN_STRING(buffer, 1);
#else
	RETURN_FALSE;
#endif
}
/* }}} */

/* {{{ proto string posix_ttyname(int fd) 
   Determine terminal device name (POSIX.1, 4.7.2) */
void php3_posix_ttyname(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *fd;
	char *p;

    if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &fd)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(fd);

	p = ttyname(fd->value.lval);
	if (p == NULL) {
		php3_error(E_WARNING, "posix_ttyname(%d) failed with '%s'",
			fd->value.lval,
			strerror(errno));
		RETURN_FALSE;
	}
	
	RETURN_STRING(p, 1);
}
/* }}} */

/* {{{ proto bool posix_isatty(int fd) 
   Determine if filedesc is a tty (POSIX.1, 4.7.1) */
void php3_posix_isatty(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *fd;
	int   result;
	
    if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &fd)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(fd);

	result = isatty(fd->value.lval);
	if (!result)
		RETURN_FALSE;

	RETURN_TRUE;	
}
/* }}} */

/*
	POSIX.1, 4.8.1 sysconf() - TODO
	POSIX.1, 5.7.1 pathconf(), fpathconf() - TODO

	POSIX.1, 5.1.2 opendir(), readdir(), rewinddir(), closedir()
	POSIX.1, 5.2.1 chdir()
				already supported by PHP
 */

/* {{{ proto string posix_getcwd() 
   Get working directory pathname (POSIX.1, 5.2.2) */
void php3_posix_getcwd(INTERNAL_FUNCTION_PARAMETERS)
{
	char  buffer[PATH_MAX];
	char *p;

	p = getcwd(buffer, PATH_MAX);
	if (!p) {
		php3_error(E_WARNING, "posix_getcwd() failed with '%s'",
			strerror(errno));
		RETURN_FALSE;
	}

	RETURN_STRING(buffer, 1);
}
/* }}} */

/*
	POSIX.1, 5.3.x open(), creat(), umask()
	POSIX.1, 5.4.1 link()
		already supported by PHP.
 */

/* {{{ proto string posix_mkfifo()
   Make a FIFO special file (POSIX.1, 5.4.2) */
void php3_posix_mkfifo(INTERNAL_FUNCTION_PARAMETERS)
{
#if HAVE_MKFIFO
	pval   *path;
	pval   *mode;
	int     result;
	
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &path, &mode) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(path);
	convert_to_long(mode);

	if (php3_ini.safe_mode && (!_php3_checkuid(path->value.str.val, 3))) {
		RETURN_FALSE;
	}
	result = mkfifo(path->value.str.val, mode->value.lval);
	if (result < 0) {
		php3_error(E_WARNING, "posix_mkfifo(%s) failed with '%s'",
			path->value.str.val,
			strerror(errno));
		RETURN_FALSE;
	}

	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}    

/*
	POSIX.1, 5.5.1 unlink()
	POSIX.1, 5.5.2 rmdir()
	POSIX.1, 5.5.3 rename()
	POSIX.1, 5.6.x stat(), access(), chmod(), utime()
		already supported by PHP (access() not supported, because it is
		braindead and dangerous and gives outdated results).

	POSIX.1, 6.x most I/O functions already supported by PHP.
	POSIX.1, 7.x tty functions, TODO
	POSIX.1, 8.x interactions with other C language functions
	POSIX.1, 9.x system database access	
 */

/* {{{ proto array posix_getgrnam(string groupname) 
   Group database access (POSIX.1, 9.2.1) */
void php3_posix_getgrnam(INTERNAL_FUNCTION_PARAMETERS)
{
	pval          *name;
	char           buffer[10];
	struct group  *g;
	char         **p;
	int            count;
	
    if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &name)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(name);

	g = getgrnam(name->value.str.val);
	if (!g) {
		php3_error(E_WARNING, "posix_getgrnam(%s) failed with '%s'",
			name->value.str.val,
			strerror(errno));
		RETURN_FALSE;
	}
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}
	add_assoc_string(return_value, "name",		g->gr_name,  strlen(g->gr_name));
	add_assoc_long  (return_value, "gid",		g->gr_gid);
	for (count=0, p=g->gr_mem; p[count] != NULL; count++) {
		snprintf(buffer, 10, "%d", count);
		add_assoc_string(return_value, buffer, p[count], strlen(p[count]));
	}
	add_assoc_long(return_value, "members", count);
}
/* }}} */


/* {{{ proto array posix_getgrgid(long gid) 
   Group database access (POSIX.1, 9.2.1) */
void php3_posix_getgrgid(INTERNAL_FUNCTION_PARAMETERS)
{
	pval          *gid;
	char           buffer[10];
	struct group  *g;
	char         **p;
	int            count;
	
    if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &gid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(gid);

	g = getgrgid(gid->value.lval);
	if (!g) {
		php3_error(E_WARNING, "posix_getgrgid(%d) failed with '%s'",
			gid->value.lval,
			strerror(errno));
		RETURN_FALSE;
	}
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}
	add_assoc_string(return_value, "name",		g->gr_name,  strlen(g->gr_name));
	add_assoc_long  (return_value, "gid",		g->gr_gid);
	for (count=0, p=g->gr_mem; p[count] != NULL; count++) {
		snprintf(buffer, 10, "%d", count);
		add_assoc_string(return_value, buffer, p[count], strlen(p[count]));
	}
	add_assoc_long(return_value, "members", count);
}
/* }}} */

/* {{{ proto array posix_getpwnam(string groupname) 
   User database access (POSIX.1, 9.2.2) */
void php3_posix_getpwnam(INTERNAL_FUNCTION_PARAMETERS)
{
	pval          *name;
	struct passwd *pw;
	
    if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &name)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(name);

	pw = getpwnam(name->value.str.val);
	if (!pw) {
		php3_error(E_WARNING, "posix_getpwnam(%s) failed with '%s'",
			name->value.str.val,
			strerror(errno));
		RETURN_FALSE;
	}
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}
	add_assoc_string(return_value, "name",      pw->pw_name, strlen(pw->pw_name));
	add_assoc_string(return_value, "passwd",    pw->pw_passwd, strlen(pw->pw_passwd));
	add_assoc_long  (return_value, "uid",       pw->pw_uid);
	add_assoc_long  (return_value, "gid",		pw->pw_gid);
	add_assoc_string(return_value, "gecos",     pw->pw_gecos, strlen(pw->pw_gecos));
	add_assoc_string(return_value, "dir",       pw->pw_dir, strlen(pw->pw_dir));
	add_assoc_string(return_value, "shell",     pw->pw_shell, strlen(pw->pw_shell));
}
/* }}} */

/* {{{ proto array posix_getpwuid(long uid) 
   User database access (POSIX.1, 9.2.2) */
void php3_posix_getpwuid(INTERNAL_FUNCTION_PARAMETERS)
{
	pval          *uid;
	struct passwd *pw;
	
    if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &uid)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(uid);

	pw = getpwuid(uid->value.lval);
	if (!pw) {
		php3_error(E_WARNING, "posix_getpwuid(%d) failed with '%s'",
			uid->value.lval,
			strerror(errno));
		RETURN_FALSE;
	}
	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}
	add_assoc_string(return_value, "name",      pw->pw_name, strlen(pw->pw_name));
	add_assoc_string(return_value, "passwd",    pw->pw_passwd, strlen(pw->pw_passwd));
	add_assoc_long  (return_value, "uid",       pw->pw_uid);
	add_assoc_long  (return_value, "gid",		pw->pw_gid);
	add_assoc_string(return_value, "gecos",     pw->pw_gecos, strlen(pw->pw_gecos));
	add_assoc_string(return_value, "dir",       pw->pw_dir, strlen(pw->pw_dir));
	add_assoc_string(return_value, "shell",     pw->pw_shell, strlen(pw->pw_shell));
}
/* }}} */


#if HAVE_GETRLIMIT
static int posix_addlimit(int limit, char *name, pval *return_value) {
	int result;
	struct rlimit rl;
	char hard[80];
	char soft[80];

	snprintf(hard, 80, "hard %s", name);
	snprintf(soft, 80, "soft %s", name);

	result = getrlimit(limit, &rl);
	if (result < 0) {
		php3_error(E_WARNING, "posix_getrlimit failed to getrlimit(RLIMIT_CORE with '%s'", strerror(errno));
		return FAILURE;
	}

	if (rl.rlim_cur == RLIM_INFINITY)
		add_assoc_string(return_value,soft,"unlimited", 9);
	else
		add_assoc_long(return_value,soft,rl.rlim_cur);

	if (rl.rlim_max == RLIM_INFINITY)
		add_assoc_string(return_value,hard,"unlimited", 9);
	else
		add_assoc_long(return_value,hard,rl.rlim_max);

	return SUCCESS;
}

struct limitlist {
	int limit;
	char *name;
} limits[] = {
#ifdef RLIMIT_CORE
	{ RLIMIT_CORE,	"core" },
#endif

#ifdef RLIMIT_DATA
	{ RLIMIT_DATA,	"data" },
#endif

#ifdef RLIMIT_STACK
	{ RLIMIT_STACK,	"stack" },
#endif

#ifdef RLIMIT_VMEM
	{ RLIMIT_VMEM, "virtualmem" },
#endif

#ifdef RLIMIT_AS
	{ RLIMIT_AS, "totalmem" },
#endif

#ifdef RLIMIT_RSS
	{ RLIMIT_RSS, "rss" },
#endif

#ifdef RLIMIT_NPROC
	{ RLIMIT_NPROC, "maxproc" },
#endif

#ifdef RLIMIT_MEMLOCK
	{ RLIMIT_MEMLOCK, "memlock" },
#endif

#ifdef RLIMIT_CPU
	{ RLIMIT_CPU,	"cpu" },
#endif

#ifdef RLIMIT_FSIZE
	{ RLIMIT_FSIZE, "filesize" },
#endif

#ifdef RLIMIT_NOFILE
	{ RLIMIT_NOFILE, "openfiles" },
#endif

#ifdef RLIMIT_OFILE
	{ RLIMIT_OFILE, "openfiles" },
#endif

	{ 0, NULL }
};

#endif /* HAVE_GETRLIMIT */

/* {{{ proto long posix_getrlimit(void) 
   Get system resource consumption limits (This is not a POSIX function, but a BSDism and a SVR4ism. We compile conditionally) */
void php3_posix_getrlimit(INTERNAL_FUNCTION_PARAMETERS)
{
#if HAVE_GETRLIMIT
	struct limitlist *l = NULL;

	if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
	}

	for (l=limits; l->name; l++) {
		if (posix_addlimit(l->limit, l->name, return_value) == FAILURE)
			RETURN_FALSE;
	}
#else
	RETURN_FALSE;
#endif
}
/* }}} */


#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
