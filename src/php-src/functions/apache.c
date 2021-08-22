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
   |          Stig S�ther Bakken <ssb@fast.no>                            |
   |          David Sklar <sklar@student.net>                             |
   +----------------------------------------------------------------------+
 */

/* $Id: apache.c,v 1.53 2000/08/20 16:22:45 martin Exp $ */
#include "php.h"
#include "internal_functions.h"
#include "functions/head.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <string.h>
#include <errno.h>
#include <ctype.h>

#if APACHE
#include "http_request.h"
#include "build-defs.h"

extern module *top_module;

void php3_virtual(INTERNAL_FUNCTION_PARAMETERS);
void php3_getallheaders(INTERNAL_FUNCTION_PARAMETERS);
void php3_apachelog(INTERNAL_FUNCTION_PARAMETERS);
void php3_info_apache(void);
void php3_apache_note(INTERNAL_FUNCTION_PARAMETERS);
void php3_apache_lookup_uri(INTERNAL_FUNCTION_PARAMETERS);
#ifdef CHARSET_EBCDIC
static void php3_ebcdic2ascii(INTERNAL_FUNCTION_PARAMETERS);
static void php3_ascii2ebcdic(INTERNAL_FUNCTION_PARAMETERS);
#endif /*CHARSET_EBCDIC*/

function_entry apache_functions[] = {
	{"virtual",			php3_virtual,		NULL},
	{"getallheaders",		php3_getallheaders,	NULL},
	{"apache_note", php3_apache_note,NULL},
	{"apache_lookup_uri", php3_apache_lookup_uri,NULL},
#ifdef CHARSET_EBCDIC
	PHP_FE(ebcdic2ascii, NULL)
	PHP_FE(ascii2ebcdic, NULL)
#endif /*CHARSET_EBCDIC*/
	{NULL, NULL, NULL}
};

php3_module_entry apache_module_entry = {
	"Apache", apache_functions, NULL, NULL, NULL, NULL, php3_info_apache, STANDARD_MODULE_PROPERTIES
};

/* {{{ proto string apache_note(string note_name [, string note_value])
   Get and set Apache request notes */
void php3_apache_note(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg_name,*arg_val;
	char *note_val;
	//int arg_count = ARG_COUNT(ht);
TLS_VARS;

	if (getParameters(ht, 2 ,&arg_name,&arg_val) == FAILURE ) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_string(arg_name);
	note_val = (char *) table_get(GLOBAL(php3_rqst)->notes,arg_name->value.str.val);
	
	if (1) {
		convert_to_string(arg_val);
		table_set(GLOBAL(php3_rqst)->notes,arg_name->value.str.val,arg_val->value.str.val);
	}

	if (note_val) {
		RETURN_STRING(note_val,1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

void php3_info_apache(void) {
	module *modp = NULL;
#if !defined(WIN32) && !defined(WINNT)
	char name[64];
	char *p;
#endif
	server_rec *serv = GLOBAL(php3_rqst)->server;
	extern char server_root[MAX_STRING_LEN];
	extern uid_t user_id;
	extern char *user_name;
	extern gid_t group_id;
	extern int max_requests_per_child;

#if WIN32|WINNT
	PUTS("Apache for Windows 95/NT<br>");
#else
	php3_printf("<tt>APACHE_INCLUDE=%s<br>\n", PHP_APACHE_INCLUDE);
	php3_printf("APACHE_TARGET=%s<br></tt>\n", PHP_APACHE_TARGET);
#endif
	php3_printf("Apache Version: <b>%s</b><br>",SERVER_VERSION);
#ifdef APACHE_RELEASE
	php3_printf("Apache Release: <b>%d</b><br>",APACHE_RELEASE);
#endif
	php3_printf("Apache API Version: <b>%d</b><br>",MODULE_MAGIC_NUMBER);
	php3_printf("Hostname/port: <b>%s:%u</b><br>\n",serv->server_hostname,serv->port);
#if !defined(WIN32) && !defined(WINNT)
	php3_printf("User/Group: <b>%s(%d)/%d</b><br>\n",user_name,(int)user_id,(int)group_id);
	php3_printf("Max Requests: <b>per child: %d &nbsp;&nbsp; keep alive: %s &nbsp;&nbsp; max per connection: %d</b><br>\n",max_requests_per_child,serv->keep_alive ? "on":"off", serv->keep_alive_max);
#endif
	php3_printf("Timeouts: <b>connection: %d &nbsp;&nbsp; keep-alive: %d</b><br>",serv->timeout,serv->keep_alive_timeout);
#if !defined(WIN32) && !defined(WINNT)
	php3_printf("Server Root: <b>%s</b><br>\n",server_root);

	PUTS("Loaded modules: ");
	for(modp = top_module; modp; modp = modp->next) {
		strncpy(name, modp->name, sizeof(name) - 1);
		if ((p = strrchr(name, '.'))) {
			*p='\0'; /* Cut off ugly .c extensions on module names */
		}
		PUTS(name);
		if (modp->next) {
			PUTS(", ");
		}
	}
#endif
	PUTS("<br></td?</tr>\n");
}

/* This function is equivalent to <!--#include virtual...-->
 * in mod_include. It does an Apache sub-request. It is useful
 * for including CGI scripts or .shtml files, or anything else
 * that you'd parse through Apache (for .phtml files, you'd probably
 * want to use <?Include>. This only works when PHP is compiled
 * as an Apache module, since it uses the Apache API for doing
 * sub requests.
 */
/* {{{ proto int virtual(string filename)
   Perform an Apache sub-request */
void php3_virtual(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *filename;
	request_rec *rr = NULL;
TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht,1,&filename) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(filename);
	
	if (!(rr = sub_req_lookup_uri (filename->value.str.val, GLOBAL(php3_rqst)))) {
		php3_error(E_WARNING, "Unable to include '%s' - URI lookup failed", filename->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	}

	if (rr->status != 200) {
		php3_error(E_WARNING, "Unable to include '%s' - error finding URI", filename->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	}

	/* Cannot include another PHP file because of global conflicts */
	if ((rr->content_type && (!strcmp(rr->content_type, PHP3_MIME_TYPE))) ||
		(rr->handler && (!strcmp(rr->handler, PHP3_MIME_TYPE)))) {
		php3_error(E_WARNING, "Cannot include a PHP file "
				   "(use <code>&lt;?include \"%s\"&gt;</code> instead)",
				   filename->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	}

	if (!php3_header()) {
		RETURN_FALSE;
	}
	if (run_sub_req(rr)) {
		php3_error(E_WARNING, "Unable to include '%s' - request execution failed", filename->value.str.val);
		if (rr) destroy_sub_req (rr);
		RETURN_FALSE;
	} else {
		if (rr) destroy_sub_req (rr);
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto array getallheaders(void)
   Fetch all HTTP request headers */
void php3_getallheaders(INTERNAL_FUNCTION_PARAMETERS)
{
    array_header *env_arr;
    table_entry *tenv;
    int i;
	
    if (array_init(return_value) == FAILURE) {
		RETURN_FALSE;
    }
    env_arr = table_elts(php3_rqst->headers_in);
    tenv = (table_entry *)env_arr->elts;
    for (i = 0; i < env_arr->nelts; ++i) {
		if (!tenv[i].key ||
			(php3_ini.safe_mode &&
			 !strncasecmp(tenv[i].key, "authorization", 13))) {
			continue;
		}
		if (add_assoc_string(return_value, tenv[i].key,(tenv[i].val==NULL) ? "" : tenv[i].val, 1)==FAILURE) {
			RETURN_FALSE;
		}
    }
}
/* }}} */

/* {{{ proto class apache_lookup_uri(string URI)
   Perform a partial request of the given URI to obtain information about it */
void php3_apache_lookup_uri(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *filename;
	request_rec *rr=NULL;

	if (ARG_COUNT(ht) != 1 || getParameters(ht,1,&filename) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(filename);

	if(!(rr = sub_req_lookup_uri(filename->value.str.val, GLOBAL(php3_rqst)))) {
		php3_error(E_WARNING, "URI lookup failed", filename->value.str.val);
		RETURN_FALSE;
	}
	object_init(return_value);
	add_property_long(return_value,"status",rr->status);
	if (rr->the_request) {
		add_property_string(return_value,"the_request",rr->the_request,1);
	}
	if (rr->status_line) {
		add_property_string(return_value,"status_line",(char *)rr->status_line,1);		
	}
	if (rr->method) {
		add_property_string(return_value,"method",(char *)rr->method,1);		
	}
	if (rr->content_type) {
		add_property_string(return_value,"content_type",(char *)rr->content_type,1);
	}
	if (rr->handler) {
		add_property_string(return_value,"handler",(char *)rr->handler,1);		
	}
	if (rr->uri) {
		add_property_string(return_value,"uri",rr->uri,1);
	}
	if (rr->filename) {
		add_property_string(return_value,"filename",rr->filename,1);
	}
	if (rr->path_info) {
		add_property_string(return_value,"path_info",rr->path_info,1);
	}
	if (rr->args) {
		add_property_string(return_value,"args",rr->args,1);
	}
	if (rr->boundary) {
		add_property_string(return_value,"boundary",rr->boundary,1);
	}
	add_property_long(return_value,"no_cache",rr->no_cache);
	add_property_long(return_value,"no_local_copy",rr->no_local_copy);
	add_property_long(return_value,"allowed",rr->allowed);
	add_property_long(return_value,"sent_bodyct",rr->sent_bodyct);
	add_property_long(return_value,"bytes_sent",rr->bytes_sent);
	add_property_long(return_value,"byterange",rr->byterange);
	add_property_long(return_value,"clength",rr->clength);

#if MODULE_MAGIC_NUMBER >= 19980324
	if (rr->unparsed_uri) {
		add_property_string(return_value,"unparsed_uri",rr->unparsed_uri,1);
	}
	if(rr->mtime) {
		add_property_long(return_value,"mtime",rr->mtime);
	}
#endif
	if(rr->request_time) {
		add_property_long(return_value,"request_time",rr->request_time);
	}

	destroy_sub_req(rr);
}
/* }}} */

#if 0
This function is most likely a bad idea.  Just playing with it for now.

void php3_apache_exec_uri(INTERNAL_FUNCTION_PARAMETERS) {
	pval *filename;
	request_rec *rr=NULL;

	if (ARG_COUNT(ht) != 1 || getParameters(ht,1,&filename) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(filename);

	if(!(rr = ap_sub_req_lookup_uri(filename->value.str.val, GLOBAL(php3_rqst)))) {
		php3_error(E_WARNING, "URI lookup failed", filename->value.str.val);
		RETURN_FALSE;
	}
	RETVAL_LONG(ap_run_sub_req(rr));
	ap_destroy_sub_req(rr);
}
#endif

#ifdef CHARSET_EBCDIC
/* {{{ proto string ebcdic2ascii (string mem)
   Binary-safe ebcdic2ascii conversion */
static void php3_ebcdic2ascii(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1;
	int len;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	len = arg1->value.str.len;

	return_value->value.str.val = emalloc(sizeof(char) * (len + 1));
	/* needed because ebcdic2ascii doesnt put a null at the end*/
	
	return_value->value.str.len = len;
	ebcdic2ascii(return_value->value.str.val, arg1->value.str.val, len);
	return_value->value.str.val[return_value->value.str.len] = '\0';

	return_value->type = IS_STRING;
}
/* }}} ebcdic2ascii */


/* {{{ proto string ascii2ebcdic (string mem)
   Binary-safe ascii2ebcdic conversion */
static void php3_ascii2ebcdic(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg1;
	int len;
	TLS_VARS;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg1);
	len = arg1->value.str.len;

	return_value->value.str.val = emalloc(sizeof(char) * (len + 1));
	/* needed because ascii2ebcdic doesnt put a null at the end*/
	
	return_value->value.str.len = len;
	ascii2ebcdic(return_value->value.str.val, arg1->value.str.val, len);
	return_value->value.str.val[return_value->value.str.len] = '\0';

	return_value->type = IS_STRING;
}
/* }}} ascii2ebcdic */
#endif /*CHARSET_EBCDIC*/
#endif /*APACHE*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
