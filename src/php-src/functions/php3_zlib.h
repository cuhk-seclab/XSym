/*
   +----------------------------------------------------------------------+
   | PHP HTML Embedded Scripting Language Version 3.0                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2000 PHP Development Team (See Credits file)      |
   +----------------------------------------------------------------------+
   | This program is free software; you can redistribute it and/or modify |
   | it under the terms of the GNU General Public License as published by |
   | the Free Software Foundation; either version 2 of the License, or    |
   | (at your option) any later version.                                  |
   |                                                                      |
   | This program is distributed in the hope that it will be useful,      |
   | but WITHOUT ANY WARRANTY; without even the implied warranty of       |
   | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |
   | GNU General Public License for more details.                         |
   |                                                                      |
   | You should have received a copy of the GNU General Public License    |
   | along with this program; if not, write to the Free Software          |
   | Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            |
   +----------------------------------------------------------------------+
   | Authors: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   |          Stefan R�hrich <sr@linux.de>                                |
   +----------------------------------------------------------------------+
 */

/* $Id: php3_zlib.h,v 1.4 2000/01/01 04:44:10 sas Exp $ */

#ifndef _PHP3_ZLIB_H
#define _PHP3_ZLIB_H

#if COMPILE_DL
#undef HAVE_ZLIB
#define HAVE_ZLIB 1
#endif

#if HAVE_ZLIB
#ifndef INIT_FUNC_ARGS
#include "modules.h"
#endif

extern php3_module_entry php3_zlib_module_entry;
#define php3_zlib_module_ptr &php3_zlib_module_entry

extern int php3_minit_zlib(INIT_FUNC_ARGS);
extern int php3_mshutdown_zlib(void);
extern void php3_info_zlib(void);
extern void php3_gzopen(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzclose(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzeof(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzread(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzgetc(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzgets(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzgetss(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzwrite(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzrewind(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gztell(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzseek(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzpassthru(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_readgzfile(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_gzfile(INTERNAL_FUNCTION_PARAMETERS);

#else
#define php3_zlib_module_ptr NULL
#endif /* HAVE_ZLIB */

#endif /* _PHP3_ZLIB_H */
