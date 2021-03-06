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
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */


/* $Id: main.h,v 1.61 2000/04/10 19:29:36 andi Exp $ */


#ifndef _MAIN_H
#define _MAIN_H

#define MAX_STRING_VAL_LEN 100 // phli
#define phli_MAX_ARRAY_SIZE 100 // phli
#define phli_MAX_ARRAY_SIZE_2 500 // phli
#define phli_PVAL_SIZE 24

#define INIT_SYMBOL_TABLE 0x1
#define INIT_TOKEN_CACHE 0x2
#define INIT_CSS 0x4
#define INIT_FOR_STACK 0x8
#define INIT_SWITCH_STACK 0x10
#define INIT_INCLUDE_STACK 0x20
#define INIT_FUNCTION_STATE_STACK 0x40
#define INIT_ENVIRONMENT 0x80
#define INIT_INCLUDE_NAMES_HASH 0x100
#define INIT_FUNCTION_TABLE 0x200
#define INIT_REQUEST_INFO 0x400
#define INIT_FUNCTIONS 0x800
#define INIT_SCANNER 0x1000
#define INIT_MEMORY_MANAGER 0x2000
#define INIT_LIST 0x4000
#define INIT_PLIST 0x8000
#define INIT_CONFIG 0x10000
#define INIT_VARIABLE_UNASSIGN_STACK 0x20000
#define INIT_LIST_DESTRUCTORS 0x40000
#define INIT_MODULE_REGISTRY 0x80000
#define INIT_WINSOCK 0x100000
#define INIT_CONSTANTS 0x200000

#define NORMAL_SHUTDOWN 1
#define NO_SHUTDOWN 0
#define ABNORMAL_SHUTDOWN -1
#define TERMINATE_CURRENT_PHPPARSE 2

#define PREPROCESS_NONE 0
#define PREPROCESS_PREPROCESS 1
#define PREPROCESS_EXECUTE 2

extern int php3_request_startup(INLINE_TLS_VOID);
extern void php3_request_shutdown(void *dummy INLINE_TLS);
extern void php3_request_shutdown_for_exec(void *dummy);
extern int php3_module_startup(INLINE_TLS_VOID);
extern void php3_module_shutdown(INLINE_TLS_VOID);
extern void php3_module_shutdown_for_exec(void);

extern int php3_get_lineno(int lineno);
extern char *php3_get_filename(int lineno);

extern int shutdown_requested;
extern unsigned char header_is_being_sent;
extern int initialized;

extern void php3_call_shutdown_functions(void);

#endif
