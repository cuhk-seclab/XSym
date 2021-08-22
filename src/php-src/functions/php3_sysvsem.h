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
   | Authors: Tom May <tom@go2net.com>                                    |
   +----------------------------------------------------------------------+
 */


/* $Id: php3_sysvsem.h,v 1.3 2000/01/01 04:44:09 sas Exp $ */

#ifndef _PHP3_SYSVSEM_H
#define _PHP3_SYSVSEM_H

#if COMPILE_DL
#undef HAVE_SYSVSEM
#define HAVE_SYSVSEM 1
#endif

#if HAVE_SYSVSEM

extern php3_module_entry sysvsem_module_entry;
#define sysvsem_module_ptr &sysvsem_module_entry

extern int php3_minit_sysvsem(INIT_FUNC_ARGS);
extern int php3_rinit_sysvsem(INIT_FUNC_ARGS);
extern int php3_mshutdown_sysvsem(void);
extern int php3_rshutdown_sysvsem(void);
extern void php3_info_sysvsem(void);
extern void php3_sysvsem_get(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_sysvsem_acquire(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_sysvsem_release(INTERNAL_FUNCTION_PARAMETERS);

typedef struct {
	int le_sem;
} sysvsem_module;

typedef struct {
	int id;						/* For error reporting. */
	int key;					/* For error reporting. */
	int semid;					/* Returned by semget(). */
	int count;					/* Acquire count for auto-release. */
} sysvsem_sem;

extern sysvsem_module php3_sysvsem_module;

#else

#define sysvsem_module_ptr NULL

#endif

#endif /* _PHP3_SYSVSEM_H */
