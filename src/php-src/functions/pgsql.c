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
   | Authors: Zeev Suraski <zeev@zend.com>                                |
   |          Jouni Ahto <jah@mork.net> (large object interface)          |
   +----------------------------------------------------------------------+
 */
 
/* $Id: pgsql.c,v 1.92 2000/02/22 15:13:57 eschmid Exp $ */

#include <stdlib.h>

#ifndef MSVC5
#include "config.h"
#endif
#include "php.h"
#include "internal_functions.h"
#include "php3_pgsql.h"
#include "php3_string.h"

#if !(WIN32|WINNT)
#include "build-defs.h"
#endif

#if HAVE_PGSQL

#include "php3_list.h"


function_entry pgsql_functions[] = {
	{"pg_connect",		php3_pgsql_connect,			NULL},
	{"pg_pconnect",		php3_pgsql_pconnect,		NULL},
	{"pg_close",		php3_pgsql_close,			NULL},
	{"pg_cmdtuples",	php3_pgsql_cmdtuples,		NULL},
	{"pg_dbname",		php3_pgsql_dbname,			NULL},
	{"pg_errormessage",	php3_pgsql_error_message,	NULL},
	{"pg_options",		php3_pgsql_options,			NULL},
	{"pg_port",			php3_pgsql_port,			NULL},
	{"pg_tty",			php3_pgsql_tty,				NULL},
	{"pg_host",			php3_pgsql_host,			NULL},
	{"pg_exec",			php3_pgsql_exec,			NULL},
	{"pg_numrows",		php3_pgsql_num_rows,		NULL},
	{"pg_numfields",	php3_pgsql_num_fields,		NULL},
	{"pg_fieldname",	php3_pgsql_field_name,		NULL},
	{"pg_fieldsize",	php3_pgsql_field_size,		NULL},
	{"pg_fieldtype",	php3_pgsql_field_type,		NULL},
	{"pg_fieldnum",		php3_pgsql_field_number,	NULL},
	{"pg_result",		php3_pgsql_result,			NULL},
	{"pg_fetch_row",	php3_pgsql_fetch_row,		NULL},
	{"pg_fetch_array",	php3_pgsql_fetch_array,		NULL},
	{"pg_fetch_object",	php3_pgsql_fetch_object,	NULL},
	{"pg_fieldprtlen",	php3_pgsql_data_length,		NULL},
	{"pg_fieldisnull",	php3_pgsql_data_isnull,		NULL},
	{"pg_freeresult",	php3_pgsql_free_result,		NULL},
	{"pg_getlastoid",	php3_pgsql_last_oid,		NULL},
	{"pg_locreate",		php3_pgsql_lo_create,		NULL},
	{"pg_lounlink", 	php3_pgsql_lo_unlink,		NULL},
	{"pg_loopen",		php3_pgsql_lo_open,			NULL},
	{"pg_loclose",		php3_pgsql_lo_close,		NULL},
	{"pg_loread",		php3_pgsql_lo_read,			NULL},
	{"pg_lowrite",		php3_pgsql_lo_write,		NULL},
	{"pg_loreadall",	php3_pgsql_lo_readall,		NULL},
	{NULL, NULL, NULL}
};

php3_module_entry pgsql_module_entry = {
	"PostgreSQL", pgsql_functions, php3_minit_pgsql, NULL, php3_rinit_pgsql, NULL, php3_info_pgsql, STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL
php3_module_entry *get_module() { return &pgsql_module_entry; }
#endif

extern int php3_header(void);

THREAD_LS pgsql_module php3_pgsql_module;

static void _close_pgsql_link(PGconn *link)
{
	PQfinish(link);
	php3_pgsql_module.num_links--;
}


static void _close_pgsql_plink(PGconn *link)
{
	PQfinish(link);
	php3_pgsql_module.num_persistent--;
	php3_pgsql_module.num_links--;
}


static void _free_ptr(pgLofp *lofp)
{
	efree(lofp);
}


static void _free_result(pgsql_result_handle *pg_result)
{
	PQclear(pg_result->result);
	efree(pg_result);
}


int php3_minit_pgsql(INIT_FUNC_ARGS)
{
	if (cfg_get_long("pgsql.allow_persistent",&php3_pgsql_module.allow_persistent)==FAILURE) {
		php3_pgsql_module.allow_persistent=1;
	}
	if (cfg_get_long("pgsql.max_persistent",&php3_pgsql_module.max_persistent)==FAILURE) {
		php3_pgsql_module.max_persistent=-1;
	}
	if (cfg_get_long("pgsql.max_links",&php3_pgsql_module.max_links)==FAILURE) {
		php3_pgsql_module.max_links=-1;
	}
	php3_pgsql_module.num_persistent=0;
	php3_pgsql_module.le_link = register_list_destructors(_close_pgsql_link,NULL);
	php3_pgsql_module.le_plink = register_list_destructors(NULL,_close_pgsql_plink);
	/*	php3_pgsql_module.le_result = register_list_destructors(PQclear,NULL); */
	php3_pgsql_module.le_result = register_list_destructors(_free_result,NULL);
	php3_pgsql_module.le_lofp = register_list_destructors(_free_ptr,NULL);
	php3_pgsql_module.le_string = register_list_destructors(_free_ptr,NULL);
	return SUCCESS;
}


int php3_rinit_pgsql(INIT_FUNC_ARGS)
{
	php3_pgsql_module.default_link=-1;
	php3_pgsql_module.num_links = php3_pgsql_module.num_persistent;
	return SUCCESS;
}


void php3_pgsql_do_connect(INTERNAL_FUNCTION_PARAMETERS,int persistent)
{
	char *host=NULL,*port=NULL,*options=NULL,*tty=NULL,*dbname=NULL,*connstring=NULL;
	char *hashed_details;
	int hashed_details_length;
	PGconn *pgsql;
	
	switch(ARG_COUNT(ht)) {
		case 1: { /* new style, using connection string */
				pval *yyconnstring;
				if (getParameters(ht, 1, &yyconnstring) == FAILURE) {
					RETURN_FALSE;
				}
				convert_to_string(yyconnstring);
				connstring = yyconnstring->value.str.val;
				hashed_details_length = yyconnstring->value.str.len+5+1;
				hashed_details = (char *) emalloc(hashed_details_length+1);
				sprintf(hashed_details,"pgsql_%s",connstring); /* SAFE */
			}
			break;
		case 3: { /* host, port, dbname */
				pval *yyhost, *yyport, *yydbname;
				
				if (getParameters(ht, 3, &yyhost, &yyport, &yydbname) == FAILURE) {
					RETURN_FALSE;
				}
				convert_to_string(yyhost);
				convert_to_string(yyport);
				convert_to_string(yydbname);
				host = yyhost->value.str.val;
				port = yyport->value.str.val;
				dbname = yydbname->value.str.val;
				options=tty=NULL;
				hashed_details_length = yyhost->value.str.len+yyport->value.str.len+yydbname->value.str.len+5+5;
				hashed_details = (char *) emalloc(hashed_details_length+1);
				sprintf(hashed_details,"pgsql_%s_%s___%s",host,port,dbname);  /* SAFE */
			}
			break;
		case 4: { /* host, port, options, dbname */
				pval *yyhost, *yyport, *yyoptions, *yydbname;
				
				if (getParameters(ht, 4, &yyhost, &yyport, &yyoptions, &yydbname) == FAILURE) {
					RETURN_FALSE;
				}
				convert_to_string(yyhost);
				convert_to_string(yyport);
				convert_to_string(yyoptions);
				convert_to_string(yydbname);
				host = yyhost->value.str.val;
				port = yyport->value.str.val;
				options = yyoptions->value.str.val;
				dbname = yydbname->value.str.val;
				tty=NULL;
				hashed_details_length = yyhost->value.str.len+yyport->value.str.len+yyoptions->value.str.len+yydbname->value.str.len+5+5;
				hashed_details = (char *) emalloc(hashed_details_length+1);
				sprintf(hashed_details,"pgsql_%s_%s_%s__%s",host,port,options,dbname);  /* SAFE */
			}
			break;
		case 5: { /* host, port, options, tty, dbname */
				pval *yyhost, *yyport, *yyoptions, *yytty, *yydbname;
				
				if (getParameters(ht, 5, &yyhost, &yyport, &yyoptions, &yytty, &yydbname) == FAILURE) {
					RETURN_FALSE;
				}
				convert_to_string(yyhost);
				convert_to_string(yyport);
				convert_to_string(yyoptions);
				convert_to_string(yytty);
				convert_to_string(yydbname);
				host = yyhost->value.str.val;
				port = yyport->value.str.val;
				options = yyoptions->value.str.val;
				tty = yytty->value.str.val;
				dbname = yydbname->value.str.val;
				hashed_details_length = yyhost->value.str.len+yyport->value.str.len+yyoptions->value.str.len+yytty->value.str.len+yydbname->value.str.len+5+5;
				hashed_details = (char *) emalloc(hashed_details_length+1);
				sprintf(hashed_details,"pgsql_%s_%s_%s_%s_%s",host,port,options,tty,dbname);  /* SAFE */
			}
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	if (persistent) {
		list_entry *le;
		
		/* try to find if we already have this link in our persistent list */
		if (_php3_hash_find(plist, hashed_details, hashed_details_length+1, (void **) &le)==FAILURE) {  /* we don't */
			list_entry new_le;
			
			if (php3_pgsql_module.max_links!=-1 && php3_pgsql_module.num_links>=php3_pgsql_module.max_links) {
				php3_error(E_WARNING,"PostgresSQL:  Too many open links (%d)",php3_pgsql_module.num_links);
				efree(hashed_details);
				RETURN_FALSE;
			}
			if (php3_pgsql_module.max_persistent!=-1 && php3_pgsql_module.num_persistent>=php3_pgsql_module.max_persistent) {
				php3_error(E_WARNING,"PostgresSQL:  Too many open persistent links (%d)",php3_pgsql_module.num_persistent);
				efree(hashed_details);
				RETURN_FALSE;
			}

			/* create the link */
			if (connstring) {
				pgsql=PQconnectdb(connstring);
			} else {
				pgsql=PQsetdb(host,port,options,tty,dbname);
			}
			if (pgsql==NULL || PQstatus(pgsql)==CONNECTION_BAD) {
				php3_error(E_WARNING,"Unable to connect to PostgresSQL server:  %s",PQerrorMessage(pgsql));
				efree(hashed_details);
				RETURN_FALSE;
			}

			/* hash it up */
			new_le.type = php3_pgsql_module.le_plink;
			new_le.ptr = pgsql;
			if (_php3_hash_update(plist, hashed_details, hashed_details_length+1, (void *) &new_le, sizeof(list_entry), NULL)==FAILURE) {
				efree(hashed_details);
				RETURN_FALSE;
			}
			php3_pgsql_module.num_links++;
			php3_pgsql_module.num_persistent++;
		} else {  /* we do */
			if (le->type != php3_pgsql_module.le_plink) {
				RETURN_FALSE;
			}
			/* ensure that the link did not die */
			if (PQstatus(le->ptr)==CONNECTION_BAD) { /* the link died */
				if (connstring) {
					le->ptr=PQconnectdb(connstring);
				} else {
					le->ptr=PQsetdb(host,port,options,tty,dbname);
				}
				if (le->ptr==NULL || PQstatus(le->ptr)==CONNECTION_BAD) {
					php3_error(E_WARNING,"PostgresSQL link lost, unable to reconnect");
					_php3_hash_del(plist,hashed_details,hashed_details_length+1);
					efree(hashed_details);
					RETURN_FALSE;
				}
			}
			pgsql = (PGconn *) le->ptr;
		}
		return_value->value.lval = php3_list_insert(pgsql,php3_pgsql_module.le_plink);
		return_value->type = IS_LONG;
	} else {
		list_entry *index_ptr,new_index_ptr;
		
		/* first we check the hash for the hashed_details key.  if it exists,
		 * it should point us to the right offset where the actual pgsql link sits.
		 * if it doesn't, open a new pgsql link, add it to the resource list,
		 * and add a pointer to it with hashed_details as the key.
		 */
		if (_php3_hash_find(list,hashed_details,hashed_details_length+1,(void **) &index_ptr)==SUCCESS) {
			int type,link;
			void *ptr;

			if (index_ptr->type != le_index_ptr) {
				RETURN_FALSE;
			}
			link = (int) index_ptr->ptr;
			ptr = php3_list_find(link,&type);   /* check if the link is still there */
			if (ptr && (type==php3_pgsql_module.le_link || type==php3_pgsql_module.le_plink)) {
				return_value->value.lval = php3_pgsql_module.default_link = link;
				return_value->type = IS_LONG;
				efree(hashed_details);
				return;
			} else {
				_php3_hash_del(list,hashed_details,hashed_details_length+1);
			}
		}
		if (php3_pgsql_module.max_links!=-1 && php3_pgsql_module.num_links>=php3_pgsql_module.max_links) {
			php3_error(E_WARNING,"PostgresSQL:  Too many open links (%d)",php3_pgsql_module.num_links);
			efree(hashed_details);
			RETURN_FALSE;
		}
		if (connstring) {
			pgsql=PQconnectdb(connstring);
		} else {
			pgsql=PQsetdb(host,port,options,tty,dbname);
		}
		if (pgsql==NULL || PQstatus(pgsql)==CONNECTION_BAD) {
			php3_error(E_WARNING,"Unable to connect to PostgresSQL server:  %s",PQerrorMessage(pgsql));
			efree(hashed_details);
			RETURN_FALSE;
		}

		/* add it to the list */
		return_value->value.lval = php3_list_insert(pgsql,php3_pgsql_module.le_link);
		return_value->type = IS_LONG;

		/* add it to the hash */
		new_index_ptr.ptr = (void *) return_value->value.lval;
		new_index_ptr.type = le_index_ptr;
		if (_php3_hash_update(list,hashed_details,hashed_details_length+1,(void *) &new_index_ptr, sizeof(list_entry), NULL)==FAILURE) {
			efree(hashed_details);
			RETURN_FALSE;
		}
		php3_pgsql_module.num_links++;
	}
	efree(hashed_details);
	php3_pgsql_module.default_link=return_value->value.lval;
}


int php3_pgsql_get_default_link(INTERNAL_FUNCTION_PARAMETERS)
{
	if (php3_pgsql_module.default_link==-1) { /* no link opened yet, implicitly open one */
		HashTable tmp;
		
		_php3_hash_init(&tmp,0,NULL,NULL,0);
		php3_pgsql_do_connect(&tmp,return_value,list,plist,0);
		_php3_hash_destroy(&tmp);
	}
	return php3_pgsql_module.default_link;
}

/* {{{ proto int pg_connect([string connection_string] | [string host, string port, [string options, [string tty,]] string database)
   Open a PostgreSQL connection */
void php3_pgsql_connect(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU,0);
}
/* }}} */

/* {{{ proto int pg_pconnect([string connection_string] | [string host, string port, [string options, [string tty,]] string database])
   Open a persistent PostgreSQL connection */
void php3_pgsql_pconnect(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_do_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU,1);
}
/* }}} */

/* {{{ proto bool pg_close([int connection])
   Close a PostgreSQL connection */ 
void php3_pgsql_close(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *pgsql_link;
	int id,type;
	PGconn *pgsql;
	
	switch (ARG_COUNT(ht)) {
		case 0:
			id = php3_pgsql_module.default_link;
			break;
		case 1:
			if (getParameters(ht, 1, &pgsql_link)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_link);
			id = pgsql_link->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	pgsql = (PGconn *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_link && type!=php3_pgsql_module.le_plink) {
		php3_error(E_WARNING,"%d is not a PostgresSQL link index",id);
		RETURN_FALSE;
	}
	
	php3_list_delete(pgsql_link->value.lval);
	RETURN_TRUE;
}
/* }}} */


#define PHP3_PG_DBNAME 1
#define PHP3_PG_ERROR_MESSAGE 2
#define PHP3_PG_OPTIONS 3
#define PHP3_PG_PORT 4
#define PHP3_PG_TTY 5
#define PHP3_PG_HOST 6

void php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type)
{
	pval *pgsql_link;
	int id,type;
	PGconn *pgsql;
	
	switch(ARG_COUNT(ht)) {
		case 0:
			id = php3_pgsql_module.default_link;
			break;
		case 1:
			if (getParameters(ht, 1, &pgsql_link)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_link);
			id = pgsql_link->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	pgsql = (PGconn *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_link && type!=php3_pgsql_module.le_plink) {
		php3_error(E_WARNING,"%d is not a PostgresSQL link index",id);
		RETURN_FALSE;
	}
	
	switch(entry_type) {
		case PHP3_PG_DBNAME:
			return_value->value.str.val = PQdb(pgsql);
			break;
		case PHP3_PG_ERROR_MESSAGE:
			return_value->value.str.val = PQerrorMessage(pgsql);
			break;
		case PHP3_PG_OPTIONS:
			return_value->value.str.val = PQoptions(pgsql);
			break;
		case PHP3_PG_PORT:
			return_value->value.str.val = PQport(pgsql);
			break;
		case PHP3_PG_TTY:
			return_value->value.str.val = PQtty(pgsql);
			break;
		case PHP3_PG_HOST:
			return_value->value.str.val = PQhost(pgsql);
			break;
		default:
			RETURN_FALSE;
	}
	return_value->value.str.len = strlen(return_value->value.str.val);
	return_value->value.str.val = (char *) estrdup(return_value->value.str.val);
	return_value->type = IS_STRING;
}

/* {{{ proto string pg_dbname([int connection])
   Get the database name */ 
void php3_pgsql_dbname(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_DBNAME);
}
/* }}} */

/* {{{ proto string pg_errormessage([int connection])
   Get the error message string */
void php3_pgsql_error_message(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_ERROR_MESSAGE);
}
/* }}} */

/* {{{ proto string pg_options([int connection])
   Get the options associated with the connection */
void php3_pgsql_options(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_OPTIONS);
}
/* }}} */

/* {{{ proto int pg_port([int connection])
   Return the port number associated with the connection */
void php3_pgsql_port(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_PORT);
}
/* }}} */

/* {{{ proto string pg_tty([int connection])
   Return the tty name associated with the connection */
void php3_pgsql_tty(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_TTY);
}
/* }}} */

/* {{{ proto string pg_host([int connection])
   Returns the host name associated with the connection */
void php3_pgsql_host(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_link_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_HOST);
}
/* }}} */

/* {{{ proto int pg_exec([int connection,] string query)
   Execute a query */
void php3_pgsql_exec(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *query,*pgsql_link;
	int id,type;
	PGconn *pgsql;
	PGresult *pgsql_result;
	ExecStatusType  status;
	pgsql_result_handle *pg_result;
	
	switch(ARG_COUNT(ht)) {
		case 1:
			if (getParameters(ht, 1, &query)==FAILURE) {
				RETURN_FALSE;
			}
			id = php3_pgsql_module.default_link;
			break;
		case 2:
			if (getParameters(ht, 2, &pgsql_link, &query)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_link);
			id = pgsql_link->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	pgsql = (PGconn *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_link && type!=php3_pgsql_module.le_plink) {
		php3_error(E_WARNING,"%d is not a PostgresSQL link index",id);
		RETURN_FALSE;
	}
	
	convert_to_string(query);
	pgsql_result=PQexec(pgsql,query->value.str.val);
	
	if (pgsql_result) {
		status = PQresultStatus(pgsql_result);
	} else {
		status = (ExecStatusType) PQstatus(pgsql);
	}
	
	
	switch (status) {
		case PGRES_EMPTY_QUERY:
		case PGRES_BAD_RESPONSE:
		case PGRES_NONFATAL_ERROR:
		case PGRES_FATAL_ERROR:
			php3_error(E_WARNING,"PostgresSQL query failed:  %s",PQerrorMessage(pgsql));
			RETURN_FALSE;
			break;
		case PGRES_COMMAND_OK: /* successful command that did not return rows */
		default:
			if (pgsql_result) {
				pg_result = (pgsql_result_handle *) emalloc(sizeof(pgsql_result_handle));
				pg_result->conn = pgsql;
				pg_result->result = pgsql_result;
				return_value->value.lval = php3_list_insert(pg_result,php3_pgsql_module.le_result);
				return_value->type = IS_LONG;
			} else {
				RETURN_FALSE;
			}
			break;
	}
}
/* }}} */

#define PHP3_PG_NUM_ROWS 1
#define PHP3_PG_NUM_FIELDS 2
#define PHP3_PG_CMD_TUPLES 3

void php3_pgsql_get_result_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type)
{
	pval *result;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type;
	
	if (ARG_COUNT(ht)!=1 || getParameters(ht, 1, &result)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);

	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	
	pgsql_result = pg_result->result;
	
	switch (entry_type) {
		case PHP3_PG_NUM_ROWS:
			return_value->value.lval = PQntuples(pgsql_result);
			break;
		case PHP3_PG_NUM_FIELDS:
			return_value->value.lval = PQnfields(pgsql_result);
			break;
		case PHP3_PG_CMD_TUPLES:
#if HAVE_PQCMDTUPLES
			return_value->value.lval = atoi(PQcmdTuples(pgsql_result));
#else
			php3_error(E_WARNING,"This compilation does not support pg_cmdtuples()");
			return_value->value.lval = 0;
#endif
			break;
		default:
			RETURN_FALSE;
	}
	return_value->type = IS_LONG;
}

/* {{{ proto int pg_numrows(int result)
   Return the number of rows in the result */
void php3_pgsql_num_rows(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_result_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_NUM_ROWS);
}
/* }}} */

/* {{{ proto int pg_numfields(int result)
   Return the number of fields in the result */
void php3_pgsql_num_fields(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_result_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_NUM_FIELDS);
}
/* }}} */

/* {{{ proto int pg_cmdtuples(int result)
   Returns the number of affected tuples */
void php3_pgsql_cmdtuples(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_result_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_CMD_TUPLES);
}
/* }}} */


char *get_field_name(PGconn *pgsql, Oid oid, HashTable *list)
{
	PGresult *result;
	char hashed_oid_key[32];
	list_entry *field_type;
	char *ret=NULL;
	
	/* try to lookup the type in the resource list */
	snprintf(hashed_oid_key,31,"pgsql_oid_%d",(int) oid);
	hashed_oid_key[31]=0;

	if (_php3_hash_find(list,hashed_oid_key,strlen(hashed_oid_key)+1,(void **) &field_type)==SUCCESS) {
		ret = estrdup((char *)field_type->ptr);
	} else { /* hash all oid's */
		int i,num_rows;
		int oid_offset,name_offset;
		char *tmp_oid, *tmp_name;
		list_entry new_oid_entry;

		if ((result=PQexec(pgsql,"select oid,typname from pg_type"))==NULL) {
			return empty_string;
		}
		num_rows=PQntuples(result);
		oid_offset = PQfnumber(result,"oid");
		name_offset = PQfnumber(result,"typname");
		
		for (i=0; i<num_rows; i++) {
			if ((tmp_oid=PQgetvalue(result,i,oid_offset))==NULL) {
				continue;
			}
			snprintf(hashed_oid_key,31,"pgsql_oid_%s",tmp_oid);
			if ((tmp_name=PQgetvalue(result,i,name_offset))==NULL) {
				continue;
			}
			new_oid_entry.type = php3_pgsql_module.le_string;
			new_oid_entry.ptr = estrdup(tmp_name);
			_php3_hash_update(list,hashed_oid_key,strlen(hashed_oid_key)+1,(void *) &new_oid_entry, sizeof(list_entry), NULL);
			if (!ret && atoi(tmp_oid)==oid) {
				ret = estrdup(tmp_name);
			}
		}
	}
	return ret;
}
			

#define PHP3_PG_FIELD_NAME 1
#define PHP3_PG_FIELD_SIZE 2
#define PHP3_PG_FIELD_TYPE 3

void php3_pgsql_get_field_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type)
{
	pval *result,*field;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type;
	
	if (ARG_COUNT(ht)!=2 || getParameters(ht, 2, &result, &field)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	
	pgsql_result = pg_result->result;
	convert_to_long(field);
	
	if (field->value.lval<0 || field->value.lval>=PQnfields(pgsql_result)) {
		php3_error(E_WARNING,"Bad field offset specified");
		RETURN_FALSE;
	}
	
	switch (entry_type) {
		case PHP3_PG_FIELD_NAME:
			return_value->value.str.val = PQfname(pgsql_result,field->value.lval);
			return_value->value.str.len = strlen(return_value->value.str.val);
			return_value->value.str.val = estrndup(return_value->value.str.val,return_value->value.str.len);
			return_value->type = IS_STRING;
			break;
		case PHP3_PG_FIELD_SIZE:
			return_value->value.lval = PQfsize(pgsql_result,field->value.lval);
			return_value->type = IS_LONG;
			break;
		case PHP3_PG_FIELD_TYPE:
			return_value->value.str.val = get_field_name(pg_result->conn,PQftype(pgsql_result,field->value.lval),list);
			return_value->value.str.len = strlen(return_value->value.str.val);
			return_value->type = IS_STRING;
			break;
		default:
			RETURN_FALSE;
	}
}

/* {{{ proto string pg_fieldname(int result, int field_number)
   Returns the name of the field */
void php3_pgsql_field_name(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_FIELD_NAME);
}
/* }}} */

/* {{{ proto int pg_fieldsize(int result, int field_number)
   Returns the internal size of the field */ 
void php3_pgsql_field_size(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_FIELD_SIZE);
}
/* }}} */

/* {{{ proto string pg_fieldtype(int result, int field_number)
   Returns the type name for the given field */
void php3_pgsql_field_type(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_get_field_info(INTERNAL_FUNCTION_PARAM_PASSTHRU,PHP3_PG_FIELD_TYPE);
}
/* }}} */

/* {{{ proto int pg_fieldnum(int result, string field_name)
   Returns the field number of the named field */
void php3_pgsql_field_number(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *result,*field;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type;
	
	if (ARG_COUNT(ht)!=2 || getParameters(ht, 2, &result, &field)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	pgsql_result = pg_result->result;
	
	convert_to_string(field);
	return_value->value.lval = PQfnumber(pgsql_result,field->value.str.val);
	return_value->type = IS_LONG;
}
/* }}} */

/* {{{ proto mixed pg_result(int result, int row_number, mixed field_name)
   Returns values from a result identifier */
void php3_pgsql_result(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *result, *row, *field=NULL;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type,field_offset;
	
	
	if (ARG_COUNT(ht)!=3 || getParameters(ht, 3, &result, &row, &field)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	pgsql_result = pg_result->result;
	
	convert_to_long(row);
	if (row->value.lval<0 || row->value.lval>=PQntuples(pgsql_result)) {
		php3_error(E_WARNING,"Unable to jump to row %d on PostgresSQL result index %d",row->value.lval,result->value.lval);
		RETURN_FALSE;
	}
	switch(field->type) {
		case IS_STRING:
			field_offset = PQfnumber(pgsql_result,field->value.str.val);
			break;
		default:
			convert_to_long(field);
			field_offset = field->value.lval;
			break;
	}
	if (field_offset<0 || field_offset>=PQnfields(pgsql_result)) {
		php3_error(E_WARNING,"Bad column offset specified");
		RETURN_FALSE;
	}
	
	return_value->value.str.val = PQgetvalue(pgsql_result,row->value.lval,field_offset);
	return_value->value.str.len = (return_value->value.str.val ? strlen(return_value->value.str.val) : 0);
	return_value->value.str.val = safe_estrndup(return_value->value.str.val,return_value->value.str.len);
	return_value->type = IS_STRING;
}
/* }}} */

/* {{{ proto array pg_fetch_row(int result, int row)
   Get a row as an enumerated array */ 
void php3_pgsql_fetch_row(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *result, *row;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type;
	int i,num_fields;
	char *element;
	uint element_len;
	
	
	if (ARG_COUNT(ht)!=2 || getParameters(ht, 2, &result, &row)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	pgsql_result = pg_result->result;
	
	convert_to_long(row);
	if (row->value.lval<0 || row->value.lval>=PQntuples(pgsql_result)) {
		php3_error(E_WARNING,"Unable to jump to row %d on PostgresSQL result index %d",row->value.lval,result->value.lval);
		RETURN_FALSE;
	}
	array_init(return_value);
	for (i=0,num_fields=PQnfields(pgsql_result); i<num_fields; i++) {
		char *tmp;
		
		element = PQgetvalue(pgsql_result,row->value.lval,i);
		if (element) {
			element_len = strlen(element);
			if (php3_ini.magic_quotes_runtime) {
				tmp = _php3_addslashes(element, element_len, &element_len, 0);
			} else {
				tmp = estrndup(element, element_len);
			}
			add_index_stringl(return_value, i, tmp, element_len, 0);
		} else {
			/* element is NULL, don't add it */
		}
	}
}
/* }}} */

void php3_pgsql_fetch_hash(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *result, *row, *pval_ptr;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type;
	int i,num_fields;
	char *element,*field_name;
	uint element_len;
	
	
	if (ARG_COUNT(ht)!=2 || getParameters(ht, 2, &result, &row)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	pgsql_result = pg_result->result;
	
	convert_to_long(row);
	if (row->value.lval<0 || row->value.lval>=PQntuples(pgsql_result)) {
		php3_error(E_WARNING,"Unable to jump to row %d on PostgresSQL result index %d",row->value.lval,result->value.lval);
		RETURN_FALSE;
	}
	array_init(return_value);
	for (i=0,num_fields=PQnfields(pgsql_result); i<num_fields; i++) {
		char *tmp;

		element = PQgetvalue(pgsql_result,row->value.lval,i);
		if (element) {
			element_len = strlen(element);
			if (php3_ini.magic_quotes_runtime) {
				tmp = _php3_addslashes(element, element_len, &element_len, 0);
			} else {
				tmp = estrndup(element, element_len);
			}
		} else {
			tmp = estrdup(empty_string);
			element_len = 0;
		}

		add_get_index_stringl(return_value, i, tmp, element_len, (void **) &pval_ptr, 0);
		field_name = PQfname(pgsql_result, i);
		_php3_hash_pointer_update(return_value->value.ht, field_name, strlen(field_name) + 1, pval_ptr);
	}
}

/* ??  This is a rather odd function - why not just point pg_fetcharray() directly at fetch_hash ? -RL */
/* {{{ proto array pg_fetch_array(int result, int row)
   Fetch a row as an array */
void php3_pgsql_fetch_array(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto object pg_fetch_object(int result, int row)
   Fetch a row as an object */
void php3_pgsql_fetch_object(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	if (return_value->type==IS_ARRAY) {
		return_value->type = IS_OBJECT;
	}
}
/* }}} */

#define PHP3_PG_DATA_LENGTH 1
#define PHP3_PG_DATA_ISNULL 2

void php3_pgsql_data_info(INTERNAL_FUNCTION_PARAMETERS, int entry_type)
{
	pval *result,*row,*field;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type,field_offset;
	
	if (ARG_COUNT(ht)!=3 || getParameters(ht, 3, &result, &row, &field)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	pgsql_result = pg_result->result;
	
	convert_to_long(row);
	if (row->value.lval<0 || row->value.lval>=PQntuples(pgsql_result)) {
		php3_error(E_WARNING,"Unable to jump to row %d on PostgresSQL result index %d",row->value.lval,result->value.lval);
		RETURN_FALSE;
	}
	switch(field->type) {
		case IS_STRING:
			field_offset = PQfnumber(pgsql_result,field->value.str.val);
			break;
		default:
			convert_to_long(field);
			field_offset = field->value.lval;
			break;
	}
	if (field_offset<0 || field_offset>=PQnfields(pgsql_result)) {
		php3_error(E_WARNING,"Bad column offset specified");
		RETURN_FALSE;
	}
	
	switch (entry_type) {
		case PHP3_PG_DATA_LENGTH:
			return_value->value.lval = PQgetlength(pgsql_result,row->value.lval,field_offset);
			break;
		case PHP3_PG_DATA_ISNULL:
			return_value->value.lval = PQgetisnull(pgsql_result,row->value.lval,field_offset);
			break;
	}
	return_value->type = IS_LONG;
}

/* {{{ proto int pg_fieldprtlen(int result, int row, mixed field_name_or_number)
   Returns the printed length */
void php3_pgsql_data_length(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_data_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP3_PG_DATA_LENGTH);
}
/* }}} */

/* {{{ proto int pg_fieldisnull(int result, int row, mixed field_name_or_number)
   Test if a field is NULL */
void php3_pgsql_data_isnull(INTERNAL_FUNCTION_PARAMETERS)
{
	php3_pgsql_data_info(INTERNAL_FUNCTION_PARAM_PASSTHRU, PHP3_PG_DATA_ISNULL);
}
/* }}} */

/* {{{ proto int pg_freeresult(int result)
   Free result memory */
void php3_pgsql_free_result(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *result;
	pgsql_result_handle *pg_result;
	int type;
	
	if (ARG_COUNT(ht)!=1 || getParameters(ht, 1, &result)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	if (result->value.lval==0) {
		RETURN_FALSE;
	}
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	php3_list_delete(result->value.lval);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int pg_getlastoid(int result)
   Returns the last object identifier */
void php3_pgsql_last_oid(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *result;
	PGresult *pgsql_result;
	pgsql_result_handle *pg_result;
	int type;
	
	if (ARG_COUNT(ht)!=1 || getParameters(ht, 1, &result)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	convert_to_long(result);
	pg_result = (pgsql_result_handle *) php3_list_find(result->value.lval,&type);
	
	if (type!=php3_pgsql_module.le_result) {
		php3_error(E_WARNING,"%d is not a PostgresSQL result index",result->value.lval);
		RETURN_FALSE;
	}
	pgsql_result = pg_result->result;
	return_value->value.str.val = (char *) PQoidStatus(pgsql_result);
	if (return_value->value.str.val) {
		return_value->value.str.len = strlen(return_value->value.str.val);
		return_value->value.str.val = estrndup(return_value->value.str.val, return_value->value.str.len);
		return_value->type = IS_STRING;
	} else {
		return_value->value.str.val = empty_string;
	} 
}
/* }}} */

/* {{{ proto int pg_locreate(int connection)
   Create a large object */
void php3_pgsql_lo_create(INTERNAL_FUNCTION_PARAMETERS)
{
  	pval *pgsql_link;
	PGconn *pgsql;
	Oid pgsql_oid;
	int id, type;
	
	switch(ARG_COUNT(ht)) {
		case 0:
			id = php3_pgsql_module.default_link;
			break;
		case 1:
			if (getParameters(ht, 1, &pgsql_link)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_link);
			id = pgsql_link->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	pgsql = (PGconn *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_link && type!=php3_pgsql_module.le_plink) {
		php3_error(E_WARNING,"%d is not a PostgresSQL link index",id);
		RETURN_FALSE;
	}
	
	/* XXX: Archive modes not supported until I get some more data. Don't think anybody's
	   using it anyway. I believe it's also somehow related to the 'time travel' feature of
	   PostgreSQL, that's on the list of features to be removed... Create modes not supported.
	   What's the use of an object that can be only written to, but not read from, and vice
	   versa? Beats me... And the access type (r/w) must be specified again when opening
	   the object, probably (?) overrides this. (Jouni) 
	 */

	if ((pgsql_oid=lo_creat(pgsql, INV_READ|INV_WRITE))==0) {
		php3_error(E_WARNING,"Unable to create PostgresSQL large object");
		RETURN_FALSE;
	}

	return_value->value.lval = pgsql_oid;
	return_value->type = IS_LONG;
}
/* }}} */

/* {{{ proto void pg_lounlink([int connection,] int large_obj_id)
   Delete a large object */
void php3_pgsql_lo_unlink(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *pgsql_link, *oid;
	PGconn *pgsql;
	Oid pgsql_oid;
	int id, type;

	switch(ARG_COUNT(ht)) {
		case 1:
			if (getParameters(ht, 1, &oid)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(oid);
			pgsql_oid = oid->value.lval;
			id = php3_pgsql_module.default_link;
			break;
		case 2:
			if (getParameters(ht, 2, &pgsql_link, &oid)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_link);
			id = pgsql_link->value.lval;
			convert_to_long(oid);
			pgsql_oid = oid->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	pgsql = (PGconn *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_link && type!=php3_pgsql_module.le_plink) {
		php3_error(E_WARNING,"%d is not a PostgresSQL link index",id);
		RETURN_FALSE;
	}
	
	if (lo_unlink(pgsql, pgsql_oid)==-1) {
		php3_error(E_WARNING,"Unable to delete PostgresSQL large object %d", (int) pgsql_oid);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int pg_loopen([int connection,] int objoid, string mode)
   Open a large object and return a fd */
void php3_pgsql_lo_open(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *pgsql_link, *oid, *mode;
	PGconn *pgsql;
	Oid pgsql_oid;
	int id, type, pgsql_mode=0, pgsql_lofd;
	int create=0;
	char *mode_string=NULL;
	pgLofp *pgsql_lofp;

	switch(ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &oid, &mode)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(oid);
			pgsql_oid = oid->value.lval;
			convert_to_string(mode);
			mode_string = mode->value.str.val;
			id = php3_pgsql_module.default_link;
			break;
		case 3:
			if (getParameters(ht, 3, &pgsql_link, &oid, &mode)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_link);
			id = pgsql_link->value.lval;
			convert_to_long(oid);
			pgsql_oid = oid->value.lval;
			convert_to_string(mode);
			mode_string = mode->value.str.val;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	pgsql = (PGconn *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_link && type!=php3_pgsql_module.le_plink) {
		php3_error(E_WARNING,"%d is not a PostgresSQL link index",id);
		RETURN_FALSE;
	}
	
	/* r/w/+ is little bit more PHP-like than INV_READ/INV_WRITE and a lot of
	   faster to type. Unfortunately, doesn't behave the same way as fopen()...
	   (Jouni)
	 */

	if (strchr(mode_string, 'r')==mode_string) {
		pgsql_mode |= INV_READ;
		if (strchr(mode_string, '+')==mode_string+1) {
			pgsql_mode |= INV_WRITE;
		}
	}
	if (strchr(mode_string, 'w')==mode_string) {
		pgsql_mode |= INV_WRITE;
		create = 1;
		if (strchr(mode_string, '+')==mode_string+1) {
			pgsql_mode |= INV_READ;
		}
	}


	pgsql_lofp = (pgLofp *) emalloc(sizeof(pgLofp));

	if ((pgsql_lofd=lo_open(pgsql, pgsql_oid, pgsql_mode))==-1) {
		if (create) {
			if ((pgsql_oid=lo_creat(pgsql, INV_READ|INV_WRITE))==0) {
				efree(pgsql_lofp);
				php3_error(E_WARNING,"Unable to create PostgresSQL large object");
				RETURN_FALSE;
			} else {
				if ((pgsql_lofd=lo_open(pgsql, pgsql_oid, pgsql_mode))==-1) {
					if (lo_unlink(pgsql, pgsql_oid)==-1) {
						efree(pgsql_lofp);
						php3_error(E_WARNING,"Something's really messed up!!! Your database is badly corrupted in a way NOT related to PHP.");
						RETURN_FALSE;
					}
					efree(pgsql_lofp);
					php3_error(E_WARNING,"Unable to open PostgresSQL large object");
					RETURN_FALSE;
				} else {
					pgsql_lofp->conn = pgsql;
					pgsql_lofp->lofd = pgsql_lofd;
					return_value->value.lval = php3_list_insert(pgsql_lofp, php3_pgsql_module.le_lofp);
					return_value->type = IS_LONG;
				}
			}
		} else {
			efree(pgsql_lofp);
			php3_error(E_WARNING,"Unable to open PostgresSQL large object");
			RETURN_FALSE;
		}
	} else {
		pgsql_lofp->conn = pgsql;
		pgsql_lofp->lofd = pgsql_lofd;
		return_value->value.lval = php3_list_insert(pgsql_lofp, php3_pgsql_module.le_lofp);
		return_value->type = IS_LONG;
	}
}
/* }}} */

/* {{{ proto void pg_loclose(int fd)
   Close a large object */
void php3_pgsql_lo_close(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *pgsql_lofp;
	int id, type;
	pgLofp *pgsql;

	switch(ARG_COUNT(ht)) {
		case 1:
			if (getParameters(ht, 1, &pgsql_lofp)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_lofp);
			id = pgsql_lofp->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	pgsql = (pgLofp *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_lofp) {
		php3_error(E_WARNING,"%d is not a PostgresSQL large object index",id);
		RETURN_FALSE;
	}
	
	if (lo_close((PGconn *)pgsql->conn, pgsql->lofd)<0) {
		php3_error(E_WARNING,"Unable to close PostgresSQL large object descriptor %d", pgsql->lofd);
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	php3_list_delete(id);
	return;
}
/* }}} */

/* {{{ proto string pg_loread(int fd, int len)
   Read a large object */
void php3_pgsql_lo_read(INTERNAL_FUNCTION_PARAMETERS)
{
  	pval *pgsql_id, *len;
	int id, buf_len, type, nbytes;
	char *buf;
	pgLofp *pgsql;

	switch(ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &pgsql_id, &len)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_id);
			id = pgsql_id->value.lval;
			convert_to_long(len);
			buf_len = len->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	pgsql = (pgLofp *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_lofp) {
		php3_error(E_WARNING,"%d is not a PostgresSQL large object index",id);
		RETURN_FALSE;
	}
	
	buf = (char *) emalloc(sizeof(char)*(buf_len+1));
	if ((nbytes = lo_read((PGconn *)pgsql->conn, pgsql->lofd, buf, buf_len))<0) {
		efree(buf);
		RETURN_FALSE;
	}
	return_value->value.str.val = buf;
	return_value->value.str.len = nbytes;
	return_value->value.str.val[nbytes] = 0;
	return_value->type = IS_STRING;
}
/* }}} */

/* {{{ proto int pg_lowrite(int fd, string buf)
   Write a large object */
void php3_pgsql_lo_write(INTERNAL_FUNCTION_PARAMETERS)
{
  	pval *pgsql_id, *str;
	int id, buf_len, nbytes, type;
	char *buf;
	pgLofp *pgsql;

	switch(ARG_COUNT(ht)) {
		case 2:
			if (getParameters(ht, 2, &pgsql_id, &str)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_id);
			id = pgsql_id->value.lval;
			convert_to_string(str);
			buf = str->value.str.val;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	pgsql = (pgLofp *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_lofp) {
		php3_error(E_WARNING,"%d is not a PostgresSQL large object index",id);
		RETURN_FALSE;
	}
	
	buf_len = str->value.str.len;
	if ((nbytes = lo_write((PGconn *)pgsql->conn, pgsql->lofd, buf, buf_len))==-1) {
		RETURN_FALSE;
	}
	return_value->value.lval = nbytes;
	return_value->type = IS_LONG;
}
/* }}} */

/* {{{ proto void pg_loreadall(int fd)
   Read a large object and send straight to browser */
void php3_pgsql_lo_readall(INTERNAL_FUNCTION_PARAMETERS)
{
  	pval *pgsql_id;
	int i, id, tbytes, type;
	volatile int nbytes;
	char buf[8192];
	pgLofp *pgsql;
	int output=1;

	switch(ARG_COUNT(ht)) {
		case 1:
			if (getParameters(ht, 1, &pgsql_id)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long(pgsql_id);
			id = pgsql_id->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	pgsql = (pgLofp *) php3_list_find(id,&type);
	if (type!=php3_pgsql_module.le_lofp) {
		php3_error(E_WARNING,"%d is not a PostgresSQL large object index",id);
		RETURN_FALSE;
	}

	if (php3_header()) {
		tbytes = 0;
		while ((nbytes = lo_read((PGconn *)pgsql->conn, pgsql->lofd, buf, 8192))>0) {
			for(i=0; i<nbytes; i++) {
				if (output) PUTC(buf[i]);
			}
			tbytes += i;
		}
		return_value->value.lval = tbytes;
		return_value->type = IS_LONG;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */


/* {{{ void php3_info_pgsql(void)
   Show info about the pgsql module */
void php3_info_pgsql(void)
{
	char maxp[16],maxl[16];

	if (php3_pgsql_module.max_persistent==-1) {
		strcpy(maxp,"Unlimited");
	} else {
		snprintf(maxp,15,"%ld",php3_pgsql_module.max_persistent);
		maxp[15]=0;
	}
	if (php3_pgsql_module.max_links==-1) {
		strcpy(maxl,"Unlimited");
	} else {
		snprintf(maxl,15,"%ld",php3_pgsql_module.max_links);
		maxl[15]=0;
	}
	php3_printf("<table cellpadding=5>"
				"<tr><td>Allow persistent links:</td><td>%s</td></tr>\n"
				"<tr><td>Persistent links:</td><td>%d/%s</td></tr>\n"
				"<tr><td>Total links:</td><td>%d/%s</td></tr>\n"
#if !(WIN32|WINNT)
				"<tr><td valign=\"top\">Compilation definitions:</td><td>"
				"<tt>PGSQL_INCLUDE=%s<br>\n"
		        "PGSQL_LFLAGS=%s<br>\n"
		        "PGSQL_LIBS=%s<br></tt></td></tr>"
#endif
				"</table>\n",
				(php3_pgsql_module.allow_persistent?"Yes":"No"),
				php3_pgsql_module.num_persistent,maxp,
				php3_pgsql_module.num_links,maxl
#if !(WIN32|WINNT)
				,PHP_PGSQL_INCLUDE,PHP_PGSQL_LFLAGS,PHP_PGSQL_LIBS
#endif
				);
}
/* }}} */


#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
