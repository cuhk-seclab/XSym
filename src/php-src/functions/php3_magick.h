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
   | Authors: Thies C. Arntzen (thies@digicol.de)						  |
   +----------------------------------------------------------------------+
 */


/* $Id: php3_magick.h,v 1.6 2000/01/01 04:44:09 sas Exp $ */

#ifndef _PHP3_MAGICK_H
#define _PHP3_MAGICK_H

#if HAVE_LIBMAGICK
extern php3_module_entry magick_module_entry;
# define magick_module_ptr &magick_module_entry
# else
#  define magick_module_ptr NULL
# endif 
#endif /* _PHP3_MAGICK_H */

/* i had to remove include "magick.h" from here, cause it may clash some stuff in internal functions! */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
