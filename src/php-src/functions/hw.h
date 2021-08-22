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
   +----------------------------------------------------------------------+
 */


/* $Id: hw.h,v 1.11 2000/01/01 04:44:09 sas Exp $ */

#ifndef _HW_H
#define _HW_H

#if HYPERWAVE
#include "hg_comm.h"

extern php3_module_entry hw_module_entry;
#define hw_module_ptr &hw_module_entry

typedef struct {
  long default_link;
  long num_links,num_persistent;
  long max_links,max_persistent;
  long allow_persistent;
  int le_socketp, le_psocketp, le_document;
} hw_module;

extern hw_module php3_hw_module;

typedef struct {
        int size;
        char *data;
        char *attributes;
        char *bodytag;
} hw_document;

extern hw_connection php3_hw_connection;

extern int php3_minit_hw(INIT_FUNC_ARGS);
extern void php3_info_hw(void);
PHP_FUNCTION(hw_connect);
PHP_FUNCTION(hw_pconnect);
PHP_FUNCTION(hw_close);
PHP_FUNCTION(hw_root);
PHP_FUNCTION(hw_info);
PHP_FUNCTION(hw_error);
PHP_FUNCTION(hw_errormsg);
PHP_FUNCTION(hw_mv);
PHP_FUNCTION(hw_cp);
PHP_FUNCTION(hw_deleteobject);
PHP_FUNCTION(hw_changeobject);
PHP_FUNCTION(hw_modifyobject);
PHP_FUNCTION(hw_getparents);
PHP_FUNCTION(hw_getparentsobj);
PHP_FUNCTION(hw_children);
PHP_FUNCTION(hw_childrenobj);
PHP_FUNCTION(hw_getchildcoll);
PHP_FUNCTION(hw_getchildcollobj);
PHP_FUNCTION(hw_getobject);
PHP_FUNCTION(hw_getandlock);
PHP_FUNCTION(hw_unlock);
PHP_FUNCTION(hw_gettext);
PHP_FUNCTION(hw_edittext);
PHP_FUNCTION(hw_getcgi);
PHP_FUNCTION(hw_getremote);
PHP_FUNCTION(hw_getremotechildren);
PHP_FUNCTION(hw_pipedocument);
PHP_FUNCTION(hw_pipecgi);
PHP_FUNCTION(hw_insertdocument);
PHP_FUNCTION(hw_docbyanchorobj);
PHP_FUNCTION(hw_docbyanchor);
PHP_FUNCTION(hw_getobjectbyquery);
PHP_FUNCTION(hw_getobjectbyqueryobj);
PHP_FUNCTION(hw_getobjectbyquerycoll);
PHP_FUNCTION(hw_getobjectbyquerycollobj);
PHP_FUNCTION(hw_getchilddoccoll);
PHP_FUNCTION(hw_getchilddoccollobj);
PHP_FUNCTION(hw_getanchors);
PHP_FUNCTION(hw_getanchorsobj);
PHP_FUNCTION(hw_getusername);
PHP_FUNCTION(hw_setlinkroot);
PHP_FUNCTION(hw_inscoll);
PHP_FUNCTION(hw_incollections);
PHP_FUNCTION(hw_insertobject);
PHP_FUNCTION(hw_insdoc);
PHP_FUNCTION(hw_identify);
PHP_FUNCTION(hw_free_document);
PHP_FUNCTION(hw_new_document);
PHP_FUNCTION(hw_output_document);
PHP_FUNCTION(hw_document_size);
PHP_FUNCTION(hw_document_attributes);
PHP_FUNCTION(hw_document_bodytag);
PHP_FUNCTION(hw_document_content);
PHP_FUNCTION(hw_document_setcontent);
PHP_FUNCTION(hw_objrec2array);
PHP_FUNCTION(hw_array2objrec);
PHP_FUNCTION(hw_connection_info);
PHP_FUNCTION(hw_getsrcbydestobj);
PHP_FUNCTION(hw_getrellink);
PHP_FUNCTION(hw_dummy);
PHP_FUNCTION(hw_who);
PHP_FUNCTION(hw_stat);
PHP_FUNCTION(hw_mapid);
#else
#define hw_module_ptr NULL
#endif /* HYPERWAVE */
#endif /* _HW_H */

