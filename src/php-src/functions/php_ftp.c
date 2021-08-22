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
   | Authors:                                                             |
   |          Andrew Skalski      <askalski@chek.com>                     |
   +----------------------------------------------------------------------+
 */

/* $Id: php_ftp.c,v 1.9 2000/02/22 20:51:05 askalski Exp $ */

#include "php.h"

#if HAVE_FTP

#ifndef ZEND_VERSION
#include "internal_functions.h"
#include "php3_list.h"
#define php_error php3_error
#endif

#include "php_ftp.h"
#include "ftp.h"

static int	le_ftpbuf;


function_entry php3_ftp_functions[] = {
	PHP_FE(ftp_connect,			NULL)
	PHP_FE(ftp_login,			NULL)
	PHP_FE(ftp_pwd,				NULL)
	PHP_FE(ftp_cdup,			NULL)
	PHP_FE(ftp_chdir,			NULL)
	PHP_FE(ftp_mkdir,			NULL)
	PHP_FE(ftp_rmdir,			NULL)
	PHP_FE(ftp_nlist,			NULL)
	PHP_FE(ftp_rawlist,			NULL)
	PHP_FE(ftp_systype,			NULL)
	PHP_FE(ftp_pasv,			NULL)
	PHP_FE(ftp_get,				NULL)
	PHP_FE(ftp_fget,			NULL)
	PHP_FE(ftp_put,				NULL)
	PHP_FE(ftp_fput,			NULL)
	PHP_FE(ftp_size,			NULL)
	PHP_FE(ftp_mdtm,			NULL)
	PHP_FE(ftp_rename,			NULL)
	PHP_FE(ftp_delete,			NULL)
	PHP_FE(ftp_site,			NULL)
	PHP_FE(ftp_quit,			NULL)
	{NULL, NULL, NULL}
};

php3_module_entry php3_ftp_module_entry = {
	"FTP Functions",
	php3_ftp_functions,
#ifdef ZEND_VERSION
	PHP_MINIT(ftp),
#else
	php3_minit_ftp,
#endif
	NULL,
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL
DLEXPORT php3_module_entry *get_module(void) { return &php3_ftp_module_entry; }
#endif

static void ftp_destructor_ftpbuf(ftpbuf_t *ftp)
{
	ftp_close(ftp);
}

#ifndef ZEND_VERSION
int php3_minit_ftp(INIT_FUNC_ARGS)
#else
PHP_MINIT_FUNCTION(ftp)
#endif
{
	le_ftpbuf = register_list_destructors(ftp_destructor_ftpbuf, NULL);
	REGISTER_MAIN_LONG_CONSTANT("FTP_ASCII", FTPTYPE_ASCII,
		CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("FTP_BINARY", FTPTYPE_IMAGE,
		CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("FTP_IMAGE", FTPTYPE_IMAGE,
		CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("FTP_TEXT", FTPTYPE_ASCII,
		CONST_PERSISTENT | CONST_CS);
	return SUCCESS;
}


#define	FTPBUF(ftp, pval) { \
	int	id, type; \
	convert_to_long(pval); \
	id = (pval)->value.lval; \
	(ftp) = php3_list_find(id, &type); \
	if (!(ftp) || type != le_ftpbuf) { \
		php_error(E_WARNING, "Unable to find ftpbuf %d", id); \
		RETURN_FALSE; \
	} \
	}

#define	XTYPE(xtype, pval) { \
	convert_to_long(pval); \
	if (	pval->value.lval != FTPTYPE_ASCII && \
		pval->value.lval != FTPTYPE_IMAGE) \
	{ \
		php_error(E_WARNING, "arg4 must be FTP_ASCII or FTP_IMAGE"); \
		RETURN_FALSE; \
	} \
	(xtype) = pval->value.lval; \
	}

#define	FILEP(fp, pval) { \
	int	id, type; \
	int	le_fp; \
	le_fp = php3i_get_le_fp(); \
	convert_to_long(pval); \
	id = (pval)->value.lval; \
	(fp) = php3_list_find(id, &type); \
	if (!(fp) || type != le_fp) { \
		php_error(E_WARNING, "Unable to find fp %d", id); \
		RETURN_FALSE; \
	} \
	}

/* {{{ proto int ftp_connect(string host [, int port])
   Open a FTP stream */
PHP_FUNCTION(ftp_connect)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;
	short		port = 0;

	/* arg1 - hostname
	 * arg2 - [port]
	 */
	switch (ARG_COUNT(ht)) {
	case 1:
		if (getParameters(ht, 1, &arg1) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 2:
		if (getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg2);
		port = arg2->value.lval;
		break;
	default:
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg1);

	/* connect */
	ftp = ftp_open(arg1->value.str.val, htons(port));
	if (ftp == NULL)
		RETURN_FALSE;

	RETURN_LONG(php3_list_insert(ftp, le_ftpbuf));
}
/* }}} */

/* {{{ proto int ftp_login(int stream, string username, string password)
   Logs into the FTP server. */
PHP_FUNCTION(ftp_login)
{
	pval		*arg1, *arg2, *arg3;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - username
	 * arg3 - password
	 */
	if (	ARG_COUNT(ht) != 3 ||
		getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg2);
	convert_to_string(arg3);

	FTPBUF(ftp, arg1);

	/* log in */
	if (!ftp_login(ftp, arg2->value.str.val, arg3->value.str.val)) {
		php_error(E_WARNING, "ftp_login: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string ftp_pwd(int stream)
   Returns the present working directory. */
PHP_FUNCTION(ftp_pwd)
{
	pval		*arg1;
	ftpbuf_t	*ftp;
	const char	*pwd;

	/* arg1 - ftp
	 */
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);

	pwd = ftp_pwd(ftp);
	if (pwd == NULL) {
		php_error(E_WARNING, "ftp_pwd: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_STRING((char*) pwd, 1);
}
/* }}} */

/* {{{ proto int ftp_cdup(int stream)
   Changes to the parent directory */
PHP_FUNCTION(ftp_cdup)
{
	pval		*arg1;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 */
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);

	if (!ftp_cdup(ftp)) {
		php_error(E_WARNING, "ftp_cdup: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_chdir(int stream, string directory)
   Changes directories */
PHP_FUNCTION(ftp_chdir)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - directory
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg2);

	FTPBUF(ftp, arg1);

	/* change directories */
	if (!ftp_chdir(ftp, arg2->value.str.val)) {
		php_error(E_WARNING, "ftp_chdir: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string ftp_mkdir(int stream, string directory)
   Creates a directory */
PHP_FUNCTION(ftp_mkdir)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;
	char		*ret, *tmp;

	/* arg1 - ftp
	 * arg2 - directory
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg2);

	FTPBUF(ftp, arg1);

	/* change directories */
	tmp = ftp_mkdir(ftp, arg2->value.str.val);
	if (tmp == NULL) {
		php_error(E_WARNING, "ftp_mkdir: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	if ((ret = estrdup(tmp)) == NULL) {
		free(tmp);
		php_error(E_WARNING, "estrdup failed");
		RETURN_FALSE;
	}

	RETURN_STRING(ret, 0);
}
/* }}} */

/* {{{ proto int ftp_rmdir(int stream, string directory)
   Removes a directory */
PHP_FUNCTION(ftp_rmdir)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - directory
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg2);

	FTPBUF(ftp, arg1);

	/* change directories */
	if (!ftp_rmdir(ftp, arg2->value.str.val)) {
		php_error(E_WARNING, "ftp_rmdir: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto array ftp_nlist(int stream, string directory)
   Returns an array of filenames in the given directory */
PHP_FUNCTION(ftp_nlist)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;
	char		**nlist, **ptr;

	/* arg1 - ftp
	 * arg2 - directory
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg2);

	FTPBUF(ftp, arg1);

	/* get list of files */
	nlist = ftp_nlist(ftp, arg2->value.str.val);
	if (nlist == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (ptr = nlist; *ptr; ptr++)
		add_next_index_string(return_value, *ptr, 1);
	free(nlist);
}
/* }}} */

/* {{{ proto array ftp_rawlist(int stream, string directory)
   Returns a detailed listing of a directory as an array of output lines */
PHP_FUNCTION(ftp_rawlist)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;
	char		**llist, **ptr;

	/* arg1 - ftp
	 * arg2 - directory
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	convert_to_string(arg2);

	FTPBUF(ftp, arg1);

	/* get directory listing */
	llist = ftp_list(ftp, arg2->value.str.val);
	if (llist == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);
	for (ptr = llist; *ptr; ptr++)
		add_next_index_string(return_value, *ptr, 1);
	free(llist);
}
/* }}} */

/* {{{ proto string ftp_systype(int stream)
   Returns the system type identifier */
PHP_FUNCTION(ftp_systype)
{
	pval		*arg1;
	ftpbuf_t	*ftp;
	const char	*syst;


	/* arg1 - ftp
	 * arg2 - directory
	 */
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);

	syst = ftp_syst(ftp);
	if (syst == NULL) {
		php_error(E_WARNING, "ftp_syst: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_STRING((char*) syst, 1);
}
/* }}} */

/* {{{ proto int ftp_fget(int stream, int fp, string remote_file, int mode)
   Retrieves a file from the FTP server and writes it to an open file. */
PHP_FUNCTION(ftp_fget)
{
	pval		*arg1, *arg2, *arg3, *arg4;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	FILE		*fp;

	/* arg1 - ftp
	 * arg2 - fp
	 * arg3 - remote file
	 * arg4 - transfer mode
	 */
	if (	ARG_COUNT(ht) != 4 ||
		getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE)
	{
			WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	FILEP(fp, arg2);
	convert_to_string(arg3);
	XTYPE(xtype, arg4);

	if (!ftp_get(ftp, fp, arg3->value.str.val, xtype) || ferror(fp)) {
		php_error(E_WARNING, "ftp_get: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	if (ferror(fp)) {
		php_error(E_WARNING, "error writing %s", arg2->value.str.val);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_pasv(int stream, int pasv)
   Turns passive mode on or off. */
PHP_FUNCTION(ftp_pasv)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - pasv
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_long(arg2);

	if (!ftp_pasv(ftp, (arg2->value.lval) ? 1 : 0))
		RETURN_FALSE;

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_get(int stream, string local_file, string remote_file, int mode)
   Retrieves a file from the FTP server and writes it to a local file. */
PHP_FUNCTION(ftp_get)
{
	pval		*arg1, *arg2, *arg3, *arg4;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	FILE		*outfp, *tmpfp;
	int		ch;


	/* arg1 - ftp
	 * arg2 - destination (local) file
	 * arg3 - source (remote) file
	 * arg4 - transfer mode
	 */
	if (	ARG_COUNT(ht) != 4 ||
		getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);
	convert_to_string(arg3);
	XTYPE(xtype, arg4);

	/* get to temporary file, so if there is an error, no existing
	 * file gets clobbered
	 */
	if ((tmpfp = tmpfile()) == NULL) {
		php_error(E_WARNING, "error opening tmpfile");
		RETURN_FALSE;
	}

	if (	!ftp_get(ftp, tmpfp, arg3->value.str.val, xtype) ||
		ferror(tmpfp))
	{
		fclose(tmpfp);
		php_error(E_WARNING, "ftp_get: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	if ((outfp = fopen(arg2->value.str.val, "w")) == NULL) {
		fclose(tmpfp);
		php_error(E_WARNING, "error opening %s", arg2->value.str.val);
		RETURN_FALSE;
	}

	rewind(tmpfp);
	while ((ch = getc(tmpfp)) != EOF)
		putc(ch, outfp);

	if (ferror(tmpfp) || ferror(outfp)) {
		fclose(tmpfp);
		fclose(outfp);
		php_error(E_WARNING, "error writing %s", arg2->value.str.val);
		RETURN_FALSE;
	}

	fclose(tmpfp);
	fclose(outfp);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_fput(int stream, string local_file, string remote_file, int mode)
   Stores a file from an open file to the FTP server. */
PHP_FUNCTION(ftp_fput)
{
	pval		*arg1, *arg2, *arg3, *arg4;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	FILE		*fp;

	/* arg1 - ftp
	 * arg2 - remote file
	 * arg3 - fp
	 * arg4 - transfer mode
	 */
	if (	ARG_COUNT(ht) != 4 ||
		getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);
	FILEP(fp, arg3);
	XTYPE(xtype, arg4);

	if (!ftp_put(ftp, arg2->value.str.val, fp, xtype)) {
		php_error(E_WARNING, "ftp_put: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_put(int stream, string remote_file, string local_file, int mode)
   Stores a file on the FTP server */
PHP_FUNCTION(ftp_put)
{
	pval		*arg1, *arg2, *arg3, *arg4;
	ftpbuf_t	*ftp;
	ftptype_t	xtype;
	FILE		*infp;


	/* arg1 - ftp
	 * arg2 - destination (remote) file
	 * arg3 - source (local) file
	 * arg4 - transfer mode
	 */
	if (	ARG_COUNT(ht) != 4 ||
		getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);
	convert_to_string(arg3);
	XTYPE(xtype, arg4);

	if ((infp = fopen(arg3->value.str.val, "r")) == NULL) {
		php_error(E_WARNING, "error opening %s", arg3->value.str.val);
		RETURN_FALSE;
	}
	if (	!ftp_put(ftp, arg2->value.str.val, infp, xtype) ||
		ferror(infp))
	{
		fclose(infp);
		php_error(E_WARNING, "ftp_put: %s", ftp->inbuf);
		RETURN_FALSE;
	}
	fclose(infp);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_size(int stream, string path)
   Returns the size of the file, or -1 on error. */
PHP_FUNCTION(ftp_size)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - path
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);

	/* get file size */
	RETURN_LONG(ftp_size(ftp, arg2->value.str.val));
}
/* }}} */

/* {{{ proto int ftp_mdtm(int stream, string path)
   Returns the last modification time of the file, or -1 on error */
PHP_FUNCTION(ftp_mdtm)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - path
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);

	/* get file mod time */
	RETURN_LONG(ftp_mdtm(ftp, arg2->value.str.val));
}
/* }}} */

/* {{{ proto int ftp_rename(int stream, string src, string dest)
   Renames the given file to a new path */
PHP_FUNCTION(ftp_rename)
{
	pval		*arg1, *arg2, *arg3;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - src
	 * arg3 - dest
	 */
	if (	ARG_COUNT(ht) != 3 ||
		getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);
	convert_to_string(arg3);

	/* rename the file */
	if (!ftp_rename(ftp, arg2->value.str.val, arg3->value.str.val)) {
		php_error(E_WARNING, "ftp_rename: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_delete(int stream, string path)
   Deletes a file */
PHP_FUNCTION(ftp_delete)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - path
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);

	/* delete the file */
	if (!ftp_delete(ftp, arg2->value.str.val)) {
		php_error(E_WARNING, "ftp_delete: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_site(int stream, string cmd)
   Sends a SITE command to the server */
PHP_FUNCTION(ftp_site)
{
	pval		*arg1, *arg2;
	ftpbuf_t	*ftp;

	/* arg1 - ftp
	 * arg2 - cmd
	 */
	if (	ARG_COUNT(ht) != 2 ||
		getParameters(ht, 2, &arg1, &arg2) == FAILURE)
	{
		WRONG_PARAM_COUNT;
	}

	FTPBUF(ftp, arg1);
	convert_to_string(arg2);

	/* send the site command */
	if (!ftp_site(ftp, arg2->value.str.val)) {
		php_error(E_WARNING, "ftp_site: %s", ftp->inbuf);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int ftp_quit(int stream)
   Closes the FTP stream */
PHP_FUNCTION(ftp_quit)
{
	pval		*arg1;
	int		id, type;

	/* arg1 - ftp
	 */
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	id = arg1->value.lval;
	if (php3_list_find(id, &type) && type == le_ftpbuf)
		php3_list_delete(id);

	RETURN_TRUE;
}
/* }}} */

#endif /* HAVE_FTP */
