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
   | (with helpful hints from Dean Gaudet <dgaudet@arctic.org>            |
   +----------------------------------------------------------------------+
 */
/* $Id: mod_php3.c,v 1.100 2000/01/30 21:25:21 rasmus Exp $ */

#include "httpd.h"
#include "http_config.h"
#if MODULE_MAGIC_NUMBER > 19980712
#include "ap_compat.h"
#else
#if MODULE_MAGIC_NUMBER > 19980324
#include "compat.h"
#endif
#endif
#include "http_core.h"
#include "http_main.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_log.h"

#include "util_script.h"

#include "php_version.h"
#include "mod_php3.h"
#if HAVE_MOD_DAV
# include "mod_dav.h"
#endif

/* ### these should be defined in mod_php3.h or somewhere else */
#define USE_PATH 1
#define IGNORE_URL 2

module MODULE_VAR_EXPORT php3_module;

int saved_umask;

#ifndef TLS_VARS
#define TLS_VARS
#endif

#ifndef GLOBAL
#define GLOBAL(x) x
#endif

#if WIN32|WINNT
/* popenf isn't working on Windows, use open instead*/
# ifdef popenf
#  undef popenf
# endif
# define popenf(p,n,f,m) open((n),(f),(m))
# ifdef pclosef
#  undef pclosef
# endif
# define pclosef(p,f) close(f)
#else
# define php3i_popenf(p,n,f,m) popenf((p),(n),(f),(m))
#endif

extern php3_ini_structure php3_ini;  /* active config */
extern php3_ini_structure php3_ini_master;  /* master copy of config */

extern int apache_php3_module_main(request_rec *, int, int, int);
extern int php3_module_startup();
extern void php3_module_shutdown();
extern void php3_module_shutdown_for_exec();

extern int tls_create(void);
extern int tls_destroy(void);
extern int tls_startup(void);
extern int tls_shutdown(void);

#if WIN32|WINNT

/* 
	we will want to change this to the apache api
	process and thread entry and exit functions
*/
BOOL WINAPI DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
    switch( ul_reason_for_call ) {
    case DLL_PROCESS_ATTACH:
		/* 
		   I should be loading ini vars here 
		   and doing whatever true global inits
		   need to be done
		*/
		if (!tls_startup())
			return 0;
		if (!tls_create())
			return 0;

		break;
    case DLL_THREAD_ATTACH:
		if (!tls_create())
			return 0;
		/*		if (php3_module_startup()==FAILURE) {
			return FAILURE;
		}
*/		break;
    case DLL_THREAD_DETACH:
		if (!tls_destroy())
			return 0;
/*		if (initialized) {
			php3_module_shutdown();
			return SUCCESS;
		} else {
			return FAILURE;
		}
*/		break;
    case DLL_PROCESS_DETACH:
		/*
		    close down anything down in process_attach 
		*/
		if (!tls_destroy())
			return 0;
		if (!tls_shutdown())
			return 0;
		break;
    }
    return 1;
}
#endif

void php3_save_umask()
{
	TLS_VARS;
	GLOBAL(saved_umask) = umask(077);
	umask(GLOBAL(saved_umask));
}

void php3_restore_umask()
{
	TLS_VARS;
	umask(GLOBAL(saved_umask));
}

int send_php3(request_rec *r, int display_source_mode, int preprocessed, char *filename)
{
	int fd, retval;
	php3_ini_structure *conf;

	/* We don't accept OPTIONS requests, but take everything else */
	if (r->method_number == M_OPTIONS) {
		r->allowed |= (1 << METHODS) - 1;
		return DECLINED;
	}

	/* Make sure file exists */
	if (filename == NULL && r->finfo.st_mode == 0) {
		return NOT_FOUND;
	}

	/* grab configuration settings */
	conf = (php3_ini_structure *) get_module_config(r->per_dir_config,
													&php3_module);
	/* copy to active configuration */
	memcpy(&php3_ini,conf,sizeof(php3_ini_structure));

	/* If PHP parser engine has been turned off with a "php3_engine off"
	 * directive, then decline to handle this request
	 */
	if (!conf->engine) {
		r->content_type = "text/html";
		r->allowed |= (1 << METHODS) - 1;
		return DECLINED;
	}
	if (filename == NULL) {
		filename = r->filename;
	}
	/* Open the file */
	if ((fd = popenf(r->pool, filename, O_RDONLY, 0)) == -1) {
		log_reason("file permissions deny server access", filename, r);
		return FORBIDDEN;
	}

	/* Apache 1.2 has a more complex mechanism for reading POST data */
#if MODULE_MAGIC_NUMBER > 19961007
	if ((retval = setup_client_block(r, REQUEST_CHUNKED_ERROR)))
		return retval;
#endif

	if (conf->last_modified) {
#if MODULE_MAGIC_NUMBER < 19970912
		if ((retval = set_last_modified(r, r->finfo.st_mtime))) {
			return retval;
		}
#else
		update_mtime (r, r->finfo.st_mtime);
		set_last_modified(r);
		set_etag(r);
#endif
	}
	/* Assume output will be HTML.  Individual scripts may change this 
	   further down the line */
	if(conf->charset) {
		r->content_type = (char *)malloc(strlen(conf->charset)+19);
		strcpy((char *)r->content_type,"text/html;charset=");
		strcpy((char *)r->content_type+18,conf->charset);
	} else {
		r->content_type = "text/html";
	}

	/* Init timeout */
	hard_timeout("send", r);

	php3_save_umask();
	chdir_file(filename);
	add_common_vars(r);
	add_cgi_vars(r);
	if (php3_ini.expose_php) {
		table_add(r->headers_out, "X-Powered-By", "PHP/" PHP_VERSION);
	}
	apache_php3_module_main(r, fd, display_source_mode, preprocessed);

	/* Done, restore umask, turn off timeout, close file and return */
	php3_restore_umask();
	kill_timeout(r);
	pclosef(r->pool, fd);
	if(conf->charset) free((char *)r->content_type);
	return OK;
}

int send_parsed_preprocessed_php3(request_rec * r)
{
	return send_php3(r, 0, 1, NULL);
}

int send_parsed_php3(request_rec * r)
{
	return send_php3(r, 0, 0, NULL);
}

int send_parsed_php3_source(request_rec * r)
{
	return send_php3(r, 1, 0, NULL);
}

/*
 * Create the per-directory config structure with defaults from php3_ini_master
 */
static void *php3_create_dir(pool * p, char *dummy)
{
    php3_ini_structure *new;
    static int first_time = 1;

	php3_module_startup();
    new = (php3_ini_structure *) palloc(p, sizeof(php3_ini_structure));
    
    if(first_time) {
        memcpy(new,&php3_ini_master,sizeof(php3_ini_structure));
        first_time=0;
    } else {
        memcpy(new,&php3_ini,sizeof(php3_ini_structure));
    }
    return new;
}

/*
 * Merge in per-directory .conf directives
 */
static void *php3_merge_dir(pool *p, void *basev, void *addv) 
{
	php3_ini_structure *new = (php3_ini_structure *) palloc(p, sizeof(php3_ini_structure));
	php3_ini_structure *base = (php3_ini_structure *) basev;
	php3_ini_structure *add = (php3_ini_structure *) addv;

	/* Start with the base config */
	memcpy(new,base,sizeof(php3_ini_structure));

	/* Now, add any fields that have changed in *add compared to the master config */
	if (add->smtp != base->smtp) new->smtp = add->smtp;
	if (add->sendmail_path != base->sendmail_path) new->sendmail_path = add->sendmail_path;
	if (add->sendmail_from != base->sendmail_from) new->sendmail_from = add->sendmail_from;
	if (add->errors != base->errors) new->errors = add->errors;
	if (add->magic_quotes_gpc != base->magic_quotes_gpc) new->magic_quotes_gpc = add->magic_quotes_gpc;
	if (add->magic_quotes_runtime != base->magic_quotes_runtime) new->magic_quotes_runtime = add->magic_quotes_runtime;
	if (add->magic_quotes_sybase != base->magic_quotes_sybase) new->magic_quotes_sybase = add->magic_quotes_sybase;
	if (add->track_errors != base->track_errors) new->track_errors = add->track_errors;
	if (add->display_errors != base->display_errors) new->display_errors = add->display_errors;
	if (add->log_errors != base->log_errors) new->log_errors = add->log_errors;
	if (add->doc_root != base->doc_root) new->doc_root = add->doc_root;
	if (add->user_dir != base->user_dir) new->user_dir = add->user_dir;
	if (add->safe_mode != base->safe_mode) new->safe_mode = add->safe_mode;
	if (add->track_vars != base->track_vars) new->track_vars = add->track_vars;
	if (add->safe_mode_exec_dir != base->safe_mode_exec_dir) new->safe_mode_exec_dir = add->safe_mode_exec_dir;
	if (add->cgi_ext != base->cgi_ext) new->cgi_ext = add->cgi_ext;
	if (add->isapi_ext != base->isapi_ext) new->isapi_ext = add->isapi_ext;
	if (add->nsapi_ext != base->nsapi_ext) new->nsapi_ext = add->nsapi_ext;
	if (add->include_path != base->include_path) new->include_path = add->include_path;
	if (add->charset != base->charset) new->charset = add->charset;
	if (add->auto_prepend_file != base->auto_prepend_file) new->auto_prepend_file = add->auto_prepend_file;
	if (add->auto_append_file != base->auto_append_file) new->auto_append_file = add->auto_append_file;
	if (add->upload_tmp_dir != base->upload_tmp_dir) new->upload_tmp_dir = add->upload_tmp_dir;
	if (add->upload_max_filesize != base->upload_max_filesize) new->upload_max_filesize = add->upload_max_filesize;
	if (add->extension_dir != base->extension_dir) new->extension_dir = add->extension_dir;
	if (add->short_open_tag != base->short_open_tag) new->short_open_tag = add->short_open_tag;
	if (add->debugger_host != base->debugger_host) new->debugger_host = add->debugger_host;
	if (add->debugger_port != base->debugger_port) new->debugger_port = add->debugger_port;
	if (add->error_log != base->error_log) new->error_log = add->error_log;
	/* skip the highlight stuff */
	if (add->sql_safe_mode != base->sql_safe_mode) new->sql_safe_mode = add->sql_safe_mode;
	if (add->xbithack != base->xbithack) new->xbithack = add->xbithack;
	if (add->expose_php != base->expose_php) new->expose_php = add->expose_php;
	if (add->engine != base->engine) new->engine = add->engine;
	if (add->last_modified != base->last_modified) new->last_modified = add->last_modified;
	if (add->max_execution_time != base->max_execution_time) new->max_execution_time = add->max_execution_time;
	if (add->memory_limit != base->memory_limit) new->memory_limit = add->memory_limit;
	if (add->browscap != base->browscap) new->browscap = add->browscap;
	if (add->arg_separator != base->arg_separator) new->arg_separator = add->arg_separator;
	if (add->gpc_order != base->gpc_order) new->gpc_order = add->gpc_order;
	if (add->error_prepend_string != base->error_prepend_string) new->error_prepend_string = add->error_prepend_string;
	if (add->error_append_string != base->error_append_string) new->error_append_string = add->error_append_string;
	if (add->open_basedir != base->open_basedir) {
		if (new->open_basedir == NULL) {
			new->open_basedir = add->open_basedir;
		} else {
#if WIN32|WINNT
			new->open_basedir = pstrcat(p, add->open_basedir, ";", new->open_basedir, NULL);
#else
			new->open_basedir = pstrcat(p, add->open_basedir, ":", new->open_basedir, NULL);
#endif
		}
	}
	if (add->enable_dl != base->enable_dl) new->enable_dl = add->enable_dl;
	if (add->ignore_user_abort != base->ignore_user_abort) new->ignore_user_abort = add->ignore_user_abort;
	if (add->asp_tags != base->asp_tags) new->asp_tags = add->asp_tags;
	if (add->dav_script != base->dav_script) new->dav_script = add->dav_script;
	
	return new;
}

#if MODULE_MAGIC_NUMBER > 19961007
const char *php3flaghandler(cmd_parms * cmd, php3_ini_structure * conf, int val)
{
#else
char *php3flaghandler(cmd_parms * cmd, php3_ini_structure * conf, int val)
{
#endif
	long c = (long) cmd->info;

	switch (c) {
		case 0:
			conf->track_errors = val;
			break;
		case 1:
			conf->magic_quotes_gpc = val;
			break;
		case 2:
			conf->magic_quotes_runtime = val;
			break;
		case 3:
			conf->short_open_tag = val;
			break;
		case 4:
			conf->safe_mode = val;
			break;
		case 5:
			conf->track_vars = val;
			break;
		case 6:
			conf->sql_safe_mode = val;
			break;
		case 7:
			conf->engine = val;
			break;
		case 8:
			conf->xbithack = val;
			break;
		case 9:
			conf->last_modified = val;
			break;
		case 10:
			conf->log_errors = val;
			break;
		case 11:
			conf->display_errors = val;
			break;
		case 12:
			conf->magic_quotes_sybase = val;
			break;
		case 13:
			conf->enable_dl = val;
			break;
		case 14:
			conf->asp_tags = val;
			break;
		case 15:
			conf->ignore_user_abort = val;
			break;
		case 16:
			conf->expose_php = val;
			break;
	}
	return NULL;
}

#if MODULE_MAGIC_NUMBER > 19961007
const char *php3take1handler(cmd_parms * cmd, php3_ini_structure * conf, char *arg)
{
#else
char *php3take1handler(cmd_parms * cmd, php3_ini_structure * conf, char *arg)
{
#endif
	long c = (long) cmd->info;

	switch (c) {
		case 0:
			conf->errors = atoi(arg);
			break;
		case 1:
			conf->doc_root = pstrdup(cmd->pool, arg);
			break;
		case 2:
			conf->user_dir = pstrdup(cmd->pool, arg);
			break;
		case 3:
			conf->safe_mode_exec_dir = pstrdup(cmd->pool, arg);
			break;
		case 4:
			conf->include_path = pstrdup(cmd->pool, arg);
			break;
		case 5:
			if (strncasecmp(arg, "none", 4)) {
				conf->auto_prepend_file = pstrdup(cmd->pool, arg);
			} else {
				conf->auto_prepend_file = "";
			}
			break;
		case 6:
			if (strncasecmp(arg, "none", 4)) {
				conf->auto_append_file = pstrdup(cmd->pool, arg);
			} else {
				conf->auto_append_file = "";
			}
			break;
		case 7:
			conf->upload_tmp_dir = pstrdup(cmd->pool, arg);
			break;
		case 8:
			conf->extension_dir = pstrdup(cmd->pool, arg);
			break;
		case 9:
			conf->error_log = pstrdup(cmd->pool, arg);
			break;
		case 10:
			conf->arg_separator = pstrdup(cmd->pool, arg);
			break;
		case 11:
			conf->max_execution_time = atoi(arg);
			break;
		case 12:
			conf->memory_limit = atoi(arg);
			break;
		case 13:
			conf->sendmail_path = pstrdup(cmd->pool, arg);
			break;
		case 14:
			conf->browscap = pstrdup(cmd->pool, arg);
			break;
		case 15:
			conf->gpc_order = pstrdup(cmd->pool, arg);
			break;
		case 16:
			conf->error_prepend_string = pstrdup(cmd->pool, arg);
			break;
		case 17:
			conf->error_append_string = pstrdup(cmd->pool, arg);
			break;
		case 18:
			if (conf->open_basedir == NULL) {
				conf->open_basedir = pstrdup(cmd->pool, arg);
			} else {
#if WIN32|WINNT
				conf->open_basedir = pstrcat(cmd->pool, conf->open_basedir, ";", arg, NULL);
#else
				conf->open_basedir = pstrcat(cmd->pool, conf->open_basedir, ":", arg, NULL);
#endif
			}
			break;
		case 19:
			conf->upload_max_filesize = atol(arg);
			break;
		case 20:
			conf->dav_script = pstrdup(cmd->pool, arg);
			break;
		case 21:
			conf->charset = pstrdup(cmd->pool, arg);
			break;
	}
	return NULL;
}

int php3_xbithack_handler(request_rec * r)
{
	php3_ini_structure *conf;

	conf = (php3_ini_structure *) get_module_config(r->per_dir_config, &php3_module);
	if (!(r->finfo.st_mode & S_IXUSR)) {
		r->allowed |= (1 << METHODS) - 1;
		return DECLINED;
	}
	if (conf->xbithack == 0) {
		r->allowed |= (1 << METHODS) - 1;
		return DECLINED;
	}
	return send_parsed_php3(r);
}

void php3_init_handler(server_rec *s, pool *p)
{
	register_cleanup(p, NULL, php3_module_shutdown, php3_module_shutdown_for_exec);
#if MODULE_MAGIC_NUMBER >= 19980527
	ap_add_version_component("PHP/" PHP_VERSION);
#endif
}


#if HAVE_MOD_DAV

extern int phpdav_mkcol_test_handler(request_rec *r);
extern int phpdav_mkcol_create_handler(request_rec *r);

/* conf is being read twice (both here and in send_php3()) */
int send_parsed_php3_dav_script(request_rec *r)
{
	php3_ini_structure *conf;

	conf = (php3_ini_structure *) get_module_config(r->per_dir_config,
													&php3_module);
	return send_php3(r, 0, 0, conf->dav_script);
}

static int php3_type_checker(request_rec *r)
{
	php3_ini_structure *conf;

	conf = (php3_ini_structure *)get_module_config(r->per_dir_config,
												   &php3_module);

    /* If DAV support is enabled, use mod_dav's type checker. */
    if (conf->dav_script) {
		dav_api_set_request_handler(r, send_parsed_php3_dav_script);
		dav_api_set_mkcol_handlers(r, phpdav_mkcol_test_handler,
								   phpdav_mkcol_create_handler);
		/* leave the rest of the request to mod_dav */
		return dav_api_type_checker(r);
	}

    return DECLINED;
}

#else /* HAVE_MOD_DAV */

# define php3_type_checker NULL

#endif /* HAVE_MOD_DAV */


handler_rec php3_handlers[] =
{
	{"application/x-httpd-php3", send_parsed_php3},
	{"application/x-httpd-php3-preprocessed", send_parsed_preprocessed_php3},
	{"application/x-httpd-php3-source", send_parsed_php3_source},
	{"text/html", php3_xbithack_handler},
	{NULL}
};


command_rec php3_commands[] =
{
	{"php3_error_reporting", php3take1handler, (void *)0, OR_OPTIONS, TAKE1, "error reporting level"},
	{"php3_doc_root", php3take1handler, (void *)1, ACCESS_CONF|RSRC_CONF, TAKE1, "directory"}, /* not used yet */
	{"php3_user_dir", php3take1handler, (void *)2, ACCESS_CONF|RSRC_CONF, TAKE1, "user directory"}, /* not used yet */
	{"php3_safe_mode_exec_dir", php3take1handler, (void *)3, ACCESS_CONF|RSRC_CONF, TAKE1, "safe mode executable dir"},
	{"php3_include_path", php3take1handler, (void *)4, OR_OPTIONS, TAKE1, "colon-separated path"},
	{"php3_auto_prepend_file", php3take1handler, (void *)5, OR_OPTIONS, TAKE1, "file name"},
	{"php3_auto_append_file", php3take1handler, (void *)6, OR_OPTIONS, TAKE1, "file name"},
	{"php3_upload_tmp_dir", php3take1handler, (void *)7,  ACCESS_CONF|RSRC_CONF, TAKE1, "directory"},
	{"php3_extension_dir", php3take1handler, (void *)8,  ACCESS_CONF|RSRC_CONF, TAKE1, "directory"},
	{"php3_error_log", php3take1handler, (void *)9, OR_OPTIONS, TAKE1, "error log file"},
	{"php3_arg_separator", php3take1handler, (void *)10, OR_OPTIONS, TAKE1, "GET method arg separator"},
	{"php3_max_execution_time", php3take1handler, (void *)11, OR_OPTIONS, TAKE1, "Max script run time in seconds"},
	{"php3_memory_limit", php3take1handler, (void *)12, OR_OPTIONS, TAKE1, "Max memory in bytes a script may use"},
	{"php3_sendmail_path", php3take1handler, (void *)13, ACCESS_CONF|RSRC_CONF, TAKE1, "Full path to sendmail binary"},
	{"php3_browscap", php3take1handler, (void *)14, OR_OPTIONS, TAKE1, "Full path to browscap file"},
	{"php3_gpc_order", php3take1handler, (void *)15, OR_OPTIONS, TAKE1, "Set GET-COOKIE-POST order [default is GPC]"},
	{"php3_error_prepend_string", php3take1handler, (void *)16, OR_OPTIONS, TAKE1, "String to add before an error message from PHP"},
	{"php3_error_append_string", php3take1handler, (void *)17, OR_OPTIONS, TAKE1, "String to add after an error message from PHP"},
	{"php3_open_basedir", php3take1handler, (void *)18, OR_OPTIONS|RSRC_CONF, ITERATE, "Limit opening of files to this directory"},
	{"php3_upload_max_filesize", php3take1handler, (void *)19, OR_OPTIONS|RSRC_CONF, TAKE1, "Limit uploaded files to this many bytes"},
#if HAVE_MOD_DAV
	{"php3_dav_script", php3take1handler, (void *)20, OR_OPTIONS|RSRC_CONF, TAKE1,
	 "Lets PHP handle DAV requests by parsing this script."},
#endif
	{"php3_charset", php3take1handler, (void *)21, OR_OPTIONS, TAKE1, "charset"},
	{"php3_track_errors", php3flaghandler, (void *)0, OR_OPTIONS, FLAG, "on|off"},
	{"php3_magic_quotes_gpc", php3flaghandler, (void *)1, OR_OPTIONS, FLAG, "on|off"},
	{"php3_magic_quotes_runtime", php3flaghandler, (void *)2, OR_OPTIONS, FLAG, "on|off"},
	{"php3_short_open_tag", php3flaghandler, (void *)3, OR_OPTIONS, FLAG, "on|off"},
	{"php3_safe_mode", php3flaghandler, (void *)4, ACCESS_CONF|RSRC_CONF, FLAG, "on|off"},
	{"php3_track_vars", php3flaghandler, (void *)5, OR_OPTIONS, FLAG, "on|off"},
	{"php3_sql_safe_mode", php3flaghandler, (void *)6,  ACCESS_CONF|RSRC_CONF, FLAG, "on|off"},
	{"php3_engine", php3flaghandler, (void *)7, RSRC_CONF|ACCESS_CONF, FLAG, "on|off"},
	{"php3_xbithack", php3flaghandler, (void *)8, OR_OPTIONS, FLAG, "on|off"},
	{"php3_last_modified", php3flaghandler, (void *)9, OR_OPTIONS, FLAG, "on|off"},
	{"php3_log_errors", php3flaghandler, (void *)10, OR_OPTIONS, FLAG, "on|off"},
	{"php3_display_errors", php3flaghandler, (void *)11, OR_OPTIONS, FLAG, "on|off"},
	{"php3_magic_quotes_sybase", php3flaghandler, (void *)12, OR_OPTIONS, FLAG, "on|off"},
	{"php3_enable_dl", php3flaghandler, (void *)13, RSRC_CONF|ACCESS_CONF, FLAG, "on|off"},
	{"php3_asp_tags", php3flaghandler, (void *)14, OR_OPTIONS, FLAG, "on|off"},
	{"php3_ignore_user_abort", php3flaghandler, (void *)13, RSRC_CONF|ACCESS_CONF, FLAG, "on|off"},
	{"php3_expose_php", php3flaghandler, (void *)15, RSRC_CONF|ACCESS_CONF, FLAG, "on|off"},
	{NULL}
};



module MODULE_VAR_EXPORT php3_module =
{
	STANDARD_MODULE_STUFF,
	php3_init_handler,			/* initializer */
	php3_create_dir,			/* per-directory config creator */
	php3_merge_dir,				/* dir merger */
	NULL,						/* per-server config creator */
	NULL, 						/* merge server config */
	php3_commands,				/* command table */
	php3_handlers,				/* handlers */
	NULL,						/* filename translation */
	NULL,						/* check_user_id */
	NULL,						/* check auth */
	NULL,						/* check access */
	php3_type_checker,			/* type_checker */
	NULL,						/* fixups */
	NULL						/* logger */
#if MODULE_MAGIC_NUMBER >= 19970103
	,NULL						/* header parser */
#endif
#if MODULE_MAGIC_NUMBER >= 19970719
	,NULL             			/* child_init */
#endif
#if MODULE_MAGIC_NUMBER >= 19970728
	,NULL						/* child_exit */
#endif
#if MODULE_MAGIC_NUMBER >= 19970902
	,NULL						/* post read-request */
#endif
};

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
