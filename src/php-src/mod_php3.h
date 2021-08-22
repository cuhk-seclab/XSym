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
/* $Id: mod_php3.h,v 1.54 2000/01/29 19:33:04 rasmus Exp $ */

#ifndef _MOD_PHP3_H
#define _MOD_PHP3_H

#if !defined(WIN32) && !defined(WINNT)
#ifndef MODULE_VAR_EXPORT
#define MODULE_VAR_EXPORT
#endif
#endif

typedef struct {
    char *smtp; /*win 32 only*/
    char *sendmail_path;
    char *sendmail_from; /*win 32 only*/
    long precision;
    long errors;
    long magic_quotes_gpc;
    long magic_quotes_runtime;
    long magic_quotes_sybase;
    long track_errors;
    long display_errors;
    long log_errors;
    long warn_plus_overloading;
    char *doc_root;
    char *user_dir;
    long safe_mode;
    long track_vars;
    char *safe_mode_exec_dir;
    char *cgi_ext;
    char *isapi_ext;
    char *nsapi_ext;
    char *include_path;
    char *auto_prepend_file;
    char *auto_append_file;
    char *upload_tmp_dir;
	long upload_max_filesize;
    char *extension_dir;
    long  short_open_tag;
    long asp_tags;
    char *debugger_host;
    long  debugger_port;
    char *error_log;
    char *highlight_comment;
    char *highlight_default;
    char *highlight_html;
    char *highlight_string;
    char *highlight_bg;
    char *highlight_keyword;
    long sql_safe_mode;
    long xbithack;
    long engine;
    long last_modified;
    long max_execution_time;
    long memory_limit;
    char *browscap;
    char *arg_separator;
    char *gpc_order;
    long y2k_compliance;
    long define_syslog_variables;
	char *error_prepend_string;
	char *error_append_string;
	char *open_basedir;
	long enable_dl;
	long ignore_user_abort;
	char *dav_script;
	long expose_php;
	char *charset;
} php3_ini_structure;

#if MSVC5
#define S_IXUSR _S_IEXEC
#endif

#endif							/* _MOD_PHP3_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
