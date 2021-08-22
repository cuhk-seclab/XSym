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
   | Authors: Uwe Steinmann <Uwe.Steinmann@fernuni-hagen.de>              |
   +----------------------------------------------------------------------+
 */

/* $Id: pdf.c,v 1.60 2000/02/23 15:05:08 steinm Exp $ */

/* pdflib 0.6 is subject to the ALADDIN FREE PUBLIC LICENSE.
   Copyright (C) 1997 Thomas Merz. */

/* Note that there is no code from the pdflib package in this file */

#include "php.h"
#include "internal_functions.h"
#include "php3_list.h"
#include "head.h"
#include <math.h>
#include "php3_pdf.h"
#if HAVE_LIBGD
#include <gd.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#if WIN32|WINNT
# include <io.h>
# include <fcntl.h>
#endif

#if HAVE_PDFLIB

#  define PDF_GLOBAL(a) a
#  define PDF_TLS_VARS
static int le_pdf_image;
#if HAVE_PDFLIB2
static int le_outline;
#else
static int le_pdf_info;
#endif
static int le_pdf;

function_entry pdf_functions[] = {
#if HAVE_PDFLIB2
#else
	PHP_FE(pdf_get_info, NULL)
#endif
	PHP_FE(pdf_set_info_creator, NULL)
	PHP_FE(pdf_set_info_title, NULL)
	PHP_FE(pdf_set_info_subject, NULL)
	PHP_FE(pdf_set_info_author, NULL)
	PHP_FE(pdf_set_info_keywords, NULL)
	PHP_FE(pdf_open, NULL)
	PHP_FE(pdf_close, NULL)
	PHP_FE(pdf_begin_page, NULL)
	PHP_FE(pdf_end_page, NULL)
	PHP_FE(pdf_show, NULL)
	PHP_FE(pdf_show_xy, NULL)
	PHP_FE(pdf_set_font, NULL)
	PHP_FE(pdf_set_leading, NULL)
	PHP_FE(pdf_set_text_rendering, NULL)
	PHP_FE(pdf_set_horiz_scaling, NULL)
	PHP_FE(pdf_set_text_rise, NULL)
	PHP_FE(pdf_set_text_matrix, NULL)
	PHP_FE(pdf_set_text_pos, NULL)
	PHP_FE(pdf_set_char_spacing, NULL)
	PHP_FE(pdf_set_word_spacing, NULL)
	PHP_FE(pdf_continue_text, NULL)
	PHP_FE(pdf_stringwidth, NULL)
	PHP_FE(pdf_save, NULL)
	PHP_FE(pdf_restore, NULL)
	PHP_FE(pdf_translate, NULL)
	PHP_FE(pdf_scale, NULL)
	PHP_FE(pdf_rotate, NULL)
	PHP_FE(pdf_setflat, NULL)
	PHP_FE(pdf_setlinejoin, NULL)
	PHP_FE(pdf_setlinecap, NULL)
	PHP_FE(pdf_setmiterlimit, NULL)
	PHP_FE(pdf_setlinewidth, NULL)
	PHP_FE(pdf_setdash, NULL)
	PHP_FE(pdf_moveto, NULL)
	PHP_FE(pdf_lineto, NULL)
	PHP_FE(pdf_curveto, NULL)
	PHP_FE(pdf_circle, NULL)
	PHP_FE(pdf_arc, NULL)
	PHP_FE(pdf_rect, NULL)
	PHP_FE(pdf_closepath, NULL)
	PHP_FE(pdf_stroke, NULL)
	PHP_FE(pdf_closepath_stroke, NULL)
	PHP_FE(pdf_fill, NULL)
	PHP_FE(pdf_fill_stroke, NULL)
	PHP_FE(pdf_closepath_fill_stroke, NULL)
	PHP_FE(pdf_endpath, NULL)
	PHP_FE(pdf_clip, NULL)
	PHP_FE(pdf_setgray_fill, NULL)
	PHP_FE(pdf_setgray_stroke, NULL)
	PHP_FE(pdf_setgray, NULL)
	PHP_FE(pdf_setrgbcolor_fill, NULL)
	PHP_FE(pdf_setrgbcolor_stroke, NULL)
	PHP_FE(pdf_setrgbcolor, NULL)
	PHP_FE(pdf_add_outline, NULL)
	PHP_FE(pdf_set_transition, NULL)
	PHP_FE(pdf_set_duration, NULL)
	PHP_FE(pdf_open_jpeg, NULL)
#if HAVE_LIBGD && HAVE_PDFLIB2
	PHP_FE(pdf_open_memory_image, NULL)
#endif
	PHP_FE(pdf_open_gif, NULL)
	PHP_FE(pdf_close_image, NULL)
	PHP_FE(pdf_place_image, NULL)
	PHP_FE(pdf_put_image, NULL)
	PHP_FE(pdf_execute_image, NULL)
#if HAVE_PDFLIB2
	PHP_FE(pdf_add_weblink, NULL)
	PHP_FE(pdf_add_pdflink, NULL)
	PHP_FE(pdf_add_annotation, NULL)
	PHP_FE(pdf_set_border_style, NULL)
	PHP_FE(pdf_set_border_color, NULL)
	PHP_FE(pdf_get_image_height, NULL)
	PHP_FE(pdf_get_image_width, NULL)
#endif
	{NULL, NULL, NULL}
};

php3_module_entry pdf_module_entry = {
	"pdf", pdf_functions, php3_minit_pdf, php3_mend_pdf, NULL, NULL, php3_info_pdf, STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL
#include "dl/phpdl.h"
DLEXPORT php3_module_entry *get_module(void) { return &pdf_module_entry; }
#endif

#if HAVE_PDFLIB2
#else
static void _free_pdf_info(PDF_info *info)
{
	if(info->Title) efree(info->Title);
	if(info->Subject) efree(info->Subject);
	if(info->Author) efree(info->Author);
	if(info->Keywords) efree(info->Keywords);
	if(info->Creator) efree(info->Creator);
}
#endif

#if HAVE_PDFLIB2
static void _free_pdf_image(int image)
#else
static void _free_pdf_image(PDF_image *image)
#endif
{
#if HAVE_PDFLIB2
#else
	/* In pdflib 0.6 the first parameter isn't used in any of the image
           close functions. It later versions it is the PDF doc.
	   FIX ME: This probably causes in segm fault in pdflib 2.0
	 */
	PDF_close_image(NULL, image);
#endif
}

static void _free_pdf_doc(PDF *pdf)
{
	PDF_close(pdf);
#if HAVE_PDFLIB2
	PDF_delete(pdf);
#endif
}

#if HAVE_PDFLIB2
static void _free_outline(int *outline)
{
	if(outline) efree(outline);
}
#endif

int php3_minit_pdf(INIT_FUNC_ARGS)
{
	PDF_GLOBAL(le_pdf_image) = register_list_destructors(_free_pdf_image, NULL);
#if HAVE_PDFLIB2
	PDF_GLOBAL(le_outline) = register_list_destructors(_free_outline, NULL);
#else
	PDF_GLOBAL(le_pdf_info) = register_list_destructors(_free_pdf_info, NULL);
#endif
	PDF_GLOBAL(le_pdf) = register_list_destructors(_free_pdf_doc, NULL);
	return SUCCESS;
}

void php3_info_pdf() {
	/* need to use a PHPAPI function here because it is external module in windows */
#if HAVE_PDFLIB2
	php3_printf("pdflib %d.%d",  PDF_get_majorversion(), PDF_get_minorversion());
#if PDFLIB_MINORVERSION > 0
	php3_printf("The function pdf_put_image() and pdf_execute_image() are <B>not</B> available");
#else
	php3_printf("The function pdf_put_image() and pdf_execute_image() are available");
#endif
#else
	php3_printf("%s. AFM files in %s", PDFLIB_VERSION, PDF_DEFAULT_FONT_PATH);
#endif
}

int php3_mend_pdf(void){
	return SUCCESS;
}

#if HAVE_PDFLIB2
#else

/* {{{ proto int pdf_get_info(void)
   Returns a default info structure for a pdf document */
PHP_FUNCTION(pdf_get_info) {
	PDF_info *pdf_info=NULL;
	int id;
	PDF_TLS_VARS;

	pdf_info = PDF_get_info();

	/* PDF_get_info() will never fail and return NULL thought
	   it does a malloc. Anyway, once proper error checking is
	   introduced in PDF_get_info() this will be needed.
	   The author of pdflib is informed.
	*/
	if(pdf_info == NULL)
		RETURN_FALSE;

	if(!pdf_info) {
		php3_error(E_WARNING, "Could not get PDF info");
		RETURN_FALSE;
	}

	id = php3_list_insert(pdf_info,PDF_GLOBAL(le_pdf_info));
	RETURN_LONG(id);
}
/* }}} */
#endif

/* {{{ proto bool pdf_set_info_creator(int info, string creator)
   Fills the creator field of the info structure */
PHP_FUNCTION(pdf_set_info_creator) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	PDF *pdf;
#else
	PDF_info *pdf_info;
#endif
	PDF_TLS_VARS;


	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
#if HAVE_PDFLIB2
	pdf = php3_list_find(id,&type);
	if (!pdf || type!=PDF_GLOBAL(le_pdf)) {
#else
	pdf_info = php3_list_find(id,&type);
	if (!pdf_info || type!=PDF_GLOBAL(le_pdf_info)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d (type=%d)",id, type);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_set_info(pdf, "Creator", arg2->value.str.val);
#else
	pdf_info->Creator = estrdup(arg2->value.str.val);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool pdf_set_info_title(int info, string title)
   Fills the title field of the info structure */
PHP_FUNCTION(pdf_set_info_title) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	PDF *pdf;
#else
	PDF_info *pdf_info;
#endif
	PDF_TLS_VARS;


	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
#if HAVE_PDFLIB2
	pdf = php3_list_find(id,&type);
	if (!pdf || type!=PDF_GLOBAL(le_pdf)) {
#else
	pdf_info = php3_list_find(id,&type);
	if (!pdf_info || type!=PDF_GLOBAL(le_pdf_info)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d (type=%d)",id, type);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_set_info(pdf, "Title", arg2->value.str.val);
#else
	pdf_info->Title = estrdup(arg2->value.str.val);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool pdf_set_info_subject(int info, string subject)
   Fills the subject field of the info structure */
PHP_FUNCTION(pdf_set_info_subject) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	PDF *pdf;
#else
	PDF_info *pdf_info;
#endif
	PDF_TLS_VARS;


	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
#if HAVE_PDFLIB2
	pdf = php3_list_find(id,&type);
	if (!pdf || type!=PDF_GLOBAL(le_pdf)) {
#else
	pdf_info = php3_list_find(id,&type);
	if (!pdf_info || type!=PDF_GLOBAL(le_pdf_info)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d (type=%d)",id, type);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_set_info(pdf, "Subject", arg2->value.str.val);
#else
	pdf_info->Subject = estrdup(arg2->value.str.val);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool pdf_set_info_author(int info, string author)
   Fills the author field of the info structure */
PHP_FUNCTION(pdf_set_info_author) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	PDF *pdf;
#else
	PDF_info *pdf_info;
#endif
	PDF_TLS_VARS;


	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
#if HAVE_PDFLIB2
	pdf = php3_list_find(id,&type);
	if (!pdf || type!=PDF_GLOBAL(le_pdf)) {
#else
	pdf_info = php3_list_find(id,&type);
	if (!pdf_info || type!=PDF_GLOBAL(le_pdf_info)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d (type=%d)",id, type);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_set_info(pdf, "Author", arg2->value.str.val);
#else
	pdf_info->Author = estrdup(arg2->value.str.val);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool pdf_set_info_keywords(int info, string keywords)
   Fills the keywords field of the info structure */
PHP_FUNCTION(pdf_set_info_keywords) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	PDF *pdf;
#else
	PDF_info *pdf_info;
#endif
	PDF_TLS_VARS;


	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
#if HAVE_PDFLIB2
	pdf = php3_list_find(id,&type);
	if (!pdf || type!=PDF_GLOBAL(le_pdf)) {
#else
	pdf_info = php3_list_find(id,&type);
	if (!pdf_info || type!=PDF_GLOBAL(le_pdf_info)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d (type=%d)",id, type);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_set_info(pdf, "Keywords", arg2->value.str.val);
#else
	pdf_info->Keywords = estrdup(arg2->value.str.val);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int pdf_open(int filedesc, int info)
   Opens a new pdf document */
PHP_FUNCTION(pdf_open) {
	pval *file;
	pval *info;
	int id, type;
	FILE *fp;
#if HAVE_PDFLIB2
#else
	PDF_info *pdf_info;
#endif
	PDF *pdf;
	PDF_TLS_VARS;


#if HAVE_PDFLIB2
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &file) == FAILURE) {
#else
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &file, &info) == FAILURE) {
#endif
		WRONG_PARAM_COUNT;
	}

	convert_to_long(file);
	id=file->value.lval;
	fp = php3_list_find(id,&type);
	if (!fp || type!=php3i_get_le_fp()) {
		php3_error(E_WARNING,"Unable to find file identifier %d (type=%d)",id, type);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
#else
	convert_to_long(info);
	id=info->value.lval;
	pdf_info = php3_list_find(id,&type);
	if (!pdf_info || type!=PDF_GLOBAL(le_pdf_info)) {
		php3_error(E_WARNING,"Unable to find pdf info identifier %d (%d!=%d)",id, type, PDF_GLOBAL(le_pdf_info));
		RETURN_FALSE;
	}
#endif

#if HAVE_PDFLIB2
	pdf = PDF_new();
	if (0 > PDF_open_fp(pdf, fp)) {
		RETURN_FALSE;
	}
#else
	pdf = PDF_open(fp, pdf_info);
	if(!pdf)
		RETURN_FALSE;

	/* Attention pdblib 0.6:
	   Due to a problem with pdf_info() we need to get rid of it
	   right here. The problem is: When pdf_close() is called it
	   will call the pdflib function pdf_finalize() which then
	   frees the space for the struct of pdf_info. At this point the
	   elements of pdf_info still occupy space which now cannot be
	   freed anymore because the struct is already gone. Before this
	   fix, _free_pdf_info() tried to remove the elements and caused
	   an segm fault. This shouldn't happen anymore.
	*/
	php3_list_delete(id); 
#endif
	id = php3_list_insert(pdf,PDF_GLOBAL(le_pdf));
	RETURN_LONG(id);
}
/* }}} */

/* {{{ proto void pdf_close(int pdfdoc)
   Closes the pdf document */
PHP_FUNCTION(pdf_close) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	php3_list_delete(id);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_begin_page(int pdfdoc, double height, double width)
   Starts page */
PHP_FUNCTION(pdf_begin_page) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	double height, width;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	height = arg2->value.dval;
	width = arg3->value.dval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_begin_page(pdf, (float) height, (float) width);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_end_page(int pdfdoc)
   Ends page */
PHP_FUNCTION(pdf_end_page) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_end_page(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_show(int pdfdoc, string text)
   Output text at current position */
PHP_FUNCTION(pdf_show) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_show(pdf, arg2->value.str.val);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_show_xy(int pdfdoc, string text, double x-koor, double y-koor)
   Output text at position */
PHP_FUNCTION(pdf_show_xy) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_show_xy(pdf, arg2->value.str.val, (float) arg3->value.dval, (float) arg4->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_font(int pdfdoc, string font, double size, int encoding [, int embed])
   Select the current font face, size and encoding */
PHP_FUNCTION(pdf_set_font) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5;
	int id, type, font, embed;
	PDF *pdf;
	PDF_TLS_VARS;

	switch (ARG_COUNT(ht)) {
	case 4:
		if (getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		embed = 0;
		break;
	case 5:
		if (getParameters(ht, 5, &arg1, &arg2, &arg3, &arg4, &arg5) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		convert_to_long(arg5);
		embed = arg5->value.lval;
		break;
	default:
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_double(arg3);
	convert_to_long(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
	if((arg4->value.lval > 5) || (arg4->value.lval < 0)) {
		php3_error(E_WARNING,"Font encoding set to 4");
		arg4->value.lval = 4;
	}

#if HAVE_PDFLIB2
	switch(arg4->value.lval) {
		case 0:
			font = PDF_findfont(pdf, arg2->value.str.val, "builtin", embed);
			break;
		case 1:
			font = PDF_findfont(pdf, arg2->value.str.val, "pdfdoc", embed);
			break;
		case 2:
			font = PDF_findfont(pdf, arg2->value.str.val, "macroman", embed);
			break;
		case 3:
			font = PDF_findfont(pdf, arg2->value.str.val, "macexpert", embed);
			break;
		case 4:
			font = PDF_findfont(pdf, arg2->value.str.val, "winansi", embed);
			break;
		default:
			php3_error(E_WARNING,"Encoding out of range, using 0");
			font = PDF_findfont(pdf, arg2->value.str.val, "builtin", embed);
	}

	if (font < 0) {
		php3_error(E_WARNING,"Font %s not found", arg2->value.str.val);
		RETURN_FALSE;
	}

	PDF_setfont(pdf, font, (float) arg3->value.dval);
#else
	PDF_set_font(pdf, arg2->value.str.val, (float) arg3->value.dval, arg4->value.lval);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_leading(int pdfdoc, double distance)
   Sets distance between text lines */
PHP_FUNCTION(pdf_set_leading) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
	PDF_set_leading(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_text_rendering(int pdfdoc, int mode)
   Determines how text is rendered */
PHP_FUNCTION(pdf_set_text_rendering) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_long(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
#if HAVE_PDFLIB2
	PDF_set_text_rendering(pdf, arg2->value.lval);
#else
	PDF_set_text_rendering(pdf, (byte) arg2->value.lval);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_horiz_scaling(int pdfdoc, double scale)
   Sets horizontal scaling of text */
PHP_FUNCTION(pdf_set_horiz_scaling) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
	PDF_set_horiz_scaling(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_text_rise(int pdfdoc, double value)
   Sets the text rise */
PHP_FUNCTION(pdf_set_text_rise) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
	PDF_set_text_rise(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_text_matrix(int pdfdoc, arry matrix)
   Sets the text matrix */
PHP_FUNCTION(pdf_set_text_matrix) {
	pval *arg1, *arg2, *data;
	int id, type, i;
	HashTable *matrix;
	PDF *pdf;
#if HAVE_PDFLIB2
	float pdfmatrix[6];
#else
	PDF_matrix pdfmatrix;
#endif
	float *pdfmatrixptr;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_array(arg2);
	id=arg1->value.lval;
	matrix=arg2->value.ht;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
	if(_php3_hash_num_elements(matrix) != 6) {
		 php3_error(E_WARNING,"Text matrix must have 6 elements");
		RETURN_FALSE;
	}

	pdfmatrixptr = (float *) &pdfmatrix;
	_php3_hash_internal_pointer_reset(matrix);
	for(i=0; i<_php3_hash_num_elements(matrix); i++) {
		_php3_hash_get_current_data(matrix, (void *) &data);
		switch(data->type) {
			case IS_DOUBLE:
				*pdfmatrixptr++ = (float) data->value.dval;
				break;
			default:
				*pdfmatrixptr++ = 0.0;
				break;
		}
		_php3_hash_move_forward(matrix);
	}

#if HAVE_PDFLIB2
	PDF_set_text_matrix(pdf, pdfmatrix[0], pdfmatrix[1], pdfmatrix[2],
	                         pdfmatrix[3], pdfmatrix[4], pdfmatrix[5]);
#else
	PDF_set_text_matrix(pdf, pdfmatrix);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_text_pos(int pdfdoc, double x, double y)
   Set the position of text for the next pdf_show call */
PHP_FUNCTION(pdf_set_text_pos) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_set_text_pos(pdf, (float) arg2->value.dval, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_char_spacing(int pdfdoc, double space)
   Sets character spacing */
PHP_FUNCTION(pdf_set_char_spacing) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_set_char_spacing(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_word_spacing(int pdfdoc, double space)
   Sets spacing between words */
PHP_FUNCTION(pdf_set_word_spacing) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_set_word_spacing(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_continue_text(int pdfdoc, string text)
   Output text in next line */
PHP_FUNCTION(pdf_continue_text) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_continue_text(pdf, arg2->value.str.val);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto double pdf_stringwidth(int pdfdoc, string text)
   Returns width of text in current font*/
PHP_FUNCTION(pdf_stringwidth) {
	pval *arg1, *arg2;
	int id, type;
	double width;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	width = (double) PDF_stringwidth(pdf, arg2->value.str.val, PDF_get_font(pdf), PDF_get_fontsize(pdf));
#else
	width = (double) PDF_stringwidth(pdf, arg2->value.str.val);
#endif

	RETURN_DOUBLE((double)width);
}
/* }}} */

/* {{{ proto void pdf_save(int pdfdoc)
   Saves current enviroment */
PHP_FUNCTION(pdf_save) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_save(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_restore(int pdfdoc)
   Restores formerly saved enviroment */
PHP_FUNCTION(pdf_restore) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_restore(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_translate(int pdfdoc, double x, double y)
   Sets origin of coordinate system */
PHP_FUNCTION(pdf_translate) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_translate(pdf, (float) arg2->value.dval, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_scale(int pdfdoc, double x-scale, double y-scale)
   Sets scaling */
PHP_FUNCTION(pdf_scale) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_scale(pdf, (float) arg2->value.dval, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_rotate(int pdfdoc, double angle)
   Sets rotation */
PHP_FUNCTION(pdf_rotate) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_rotate(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setflat(int pdfdoc, double value)
   Sets flatness */
PHP_FUNCTION(pdf_setflat) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	if((arg2->value.lval > 100) && (arg2->value.lval < 0)) {
		php3_error(E_WARNING,"Parameter of pdf_setflat() has to between 0 and 100");
		RETURN_FALSE;
	}

	PDF_setflat(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setlinejoin(int pdfdoc, int value)
   Sets linejoin parameter */
PHP_FUNCTION(pdf_setlinejoin) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_long(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	if((arg2->value.lval > 2) && (arg2->value.lval < 0)) {
		php3_error(E_WARNING,"Parameter of pdf_setlinejoin() has to between 0 and 2");
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_setlinejoin(pdf, arg2->value.lval);
#else
	PDF_setlinejoin(pdf, (byte) arg2->value.lval);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setlinecap(int pdfdoc, int value)
   Sets linecap parameter */
PHP_FUNCTION(pdf_setlinecap) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_long(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	if((arg2->value.lval > 2) && (arg2->value.lval < 0)) {
		php3_error(E_WARNING,"Parameter of pdf_setlinecap() has to be > 0 and =< 2");
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_setlinecap(pdf, arg2->value.lval);
#else
	PDF_setlinecap(pdf, (byte) arg2->value.lval);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setmiterlimit(int pdfdoc, double value)
   Sets miter limit */
PHP_FUNCTION(pdf_setmiterlimit) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	if(arg2->value.dval < 1) {
		php3_error(E_WARNING,"Parameter of pdf_setmiterlimit() has to be >= 1");
		RETURN_FALSE;
	}

	PDF_setmiterlimit(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setlinewidth(int pdfdoc, double width)
   Sets line width */
PHP_FUNCTION(pdf_setlinewidth) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setlinewidth(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setdash(int pdfdoc, double white, double black)
   Sets dash pattern */
PHP_FUNCTION(pdf_setdash) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setdash(pdf, (float) arg2->value.dval, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_moveto(int pdfdoc, double x, double y)
   Sets current point */
PHP_FUNCTION(pdf_moveto) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_moveto(pdf, (float) arg2->value.dval, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_curveto(int pdfdoc, double x1, double y1, double x2, double y2, double x3, double y3)
   Draws a curve */
PHP_FUNCTION(pdf_curveto) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5, *arg6, *arg7;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 7 || getParameters(ht, 7, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);
	convert_to_double(arg6);
	convert_to_double(arg7);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_curveto(pdf, (float) arg2->value.dval,
	                (float) arg3->value.dval,
	                (float) arg4->value.dval,
	                (float) arg5->value.dval,
	                (float) arg6->value.dval,
	                (float) arg7->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_lineto(int pdfdoc, double x, double y)
   Draws a line */
PHP_FUNCTION(pdf_lineto) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_lineto(pdf, (float) arg2->value.dval, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_circle(int pdfdoc, double x, double y, double radius)
   Draws a circle */
PHP_FUNCTION(pdf_circle) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_circle(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_arc(int pdfdoc, double x, double y, double radius, double start, double end)
   Draws an arc */
PHP_FUNCTION(pdf_arc) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 6 || getParameters(ht, 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);
	convert_to_double(arg6);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_arc(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval, (float) arg5->value.dval, (float) arg6->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_rect(int pdfdoc, double x, double y, double width, double height)
   Draws a rectangle */
PHP_FUNCTION(pdf_rect) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 5 || getParameters(ht, 5, &arg1, &arg2, &arg3, &arg4, &arg5) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_rect(pdf, (float) arg2->value.dval,
	                (float) arg3->value.dval,
	                (float) arg4->value.dval,
	                (float) arg5->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_closepath(int pdfdoc)
   Close path */
PHP_FUNCTION(pdf_closepath) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_closepath(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_closepath_stroke(int pdfdoc)
   Close path and draw line along path */
PHP_FUNCTION(pdf_closepath_stroke) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_closepath_stroke(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_stroke(int pdfdoc)
   Draw line along path path */
PHP_FUNCTION(pdf_stroke) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_stroke(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_fill(int pdfdoc)
   Fill current path */
PHP_FUNCTION(pdf_fill) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_fill(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_fill_stroke(int pdfdoc)
   Fill and stroke current path */
PHP_FUNCTION(pdf_fill_stroke) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_fill_stroke(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_closepath_fill_stroke(int pdfdoc)
   Close, fill and stroke current path */
PHP_FUNCTION(pdf_closepath_fill_stroke) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_closepath_fill_stroke(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_endpath(int pdfdoc)
   Ends current path */
PHP_FUNCTION(pdf_endpath) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_endpath(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_clip(int pdfdoc)
   Clips to current path */
PHP_FUNCTION(pdf_clip) {
	pval *arg1;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg1) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_clip(pdf);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setgray_fill(int pdfdoc, double value)
   Sets filling color to gray value */
PHP_FUNCTION(pdf_setgray_fill) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setgray_fill(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setgray_stroke(int pdfdoc, double value)
   Sets drawing color to gray value */
PHP_FUNCTION(pdf_setgray_stroke) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setgray_stroke(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setgray(int pdfdoc, double value)
   Sets drawing and filling color to gray value */
PHP_FUNCTION(pdf_setgray) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setgray(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setrgbcolor_fill(int pdfdoc, double red, double green, double blue)
   Sets filling color to rgb color value */
PHP_FUNCTION(pdf_setrgbcolor_fill) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setrgbcolor_fill(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setrgbcolor_stroke(int pdfdoc, double red, double green, double blue)
   Sets drawing color to rgb color value */
PHP_FUNCTION(pdf_setrgbcolor_stroke) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setrgbcolor_stroke(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_setrgbcolor(int pdfdoc, double red, double green, double blue)
   Sets drawing and filling color to rgb color value */
PHP_FUNCTION(pdf_setrgbcolor) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_setrgbcolor(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int pdf_add_outline(int pdfdoc, string text [, int parent, int open]);
   Add bookmark for current page */
PHP_FUNCTION(pdf_add_outline) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	int *outline, *parent, parentid, open;
	PDF *pdf;
	PDF_TLS_VARS;

#if HAVE_PDFLIB2
	switch (ARG_COUNT(ht)) {
	case 2:
		if (getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 3:
		if (getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
		break;
	case 4:
		if (getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
			WRONG_PARAM_COUNT;
		}
	break;
	default:
		WRONG_PARAM_COUNT;
	}
#else
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
#endif
	convert_to_long(arg1);
	convert_to_string(arg2);

	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find document identifier %d",id);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	if (ARG_COUNT(ht) > 2) {
		convert_to_long(arg3);
		id = arg3->value.lval;

		if (id > 0) {
			parent = php3_list_find(id, &type);
			if (!parent || (type != PDF_GLOBAL(le_outline))) {
				php3_error(E_WARNING,"Unable to find identifier %d",id);
				RETURN_FALSE;
			} else {
				parentid = *parent;
			}
		} else {
			parentid = 0;
		}

		if (ARG_COUNT(ht) > 3) {
			convert_to_long(arg4);
			open = arg4->value.lval;
		} else {
			open = 0;
		}
	} else {
		parentid = 0;
		open = 0;
	}

	outline=emalloc(sizeof(int));
	*outline = PDF_add_bookmark(pdf, arg2->value.str.val, parentid, open);
	id = php3_list_insert(outline,PDF_GLOBAL(le_outline));
	RETURN_LONG(id);
#else
	PDF_add_outline(pdf, estrdup(arg2->value.str.val));
	RETURN_TRUE;
#endif

#if HAVE_PDFLIB2
#else
#endif
}
/* }}} */

/* {{{ proto void pdf_set_transition(int pdfdoc, int transition)
   Sets transition between pages */
PHP_FUNCTION(pdf_set_transition) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_long(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	switch(arg2->value.lval) {
		case 0:
			PDF_set_transition(pdf, "none");
			break;
		case 1:
			PDF_set_transition(pdf, "split");
			break;
		case 2:
			PDF_set_transition(pdf, "blinds");
			break;
		case 3:
			PDF_set_transition(pdf, "box");
			break;
		case 4:
			PDF_set_transition(pdf, "wipe");
			break;
		case 5:
			PDF_set_transition(pdf, "dissolve");
			break;
		case 6:
			PDF_set_transition(pdf, "glitter");
			break;
		case 7:
			PDF_set_transition(pdf, "replace");
			break;
		default:
			PDF_set_transition(pdf, "none");
			
	}
#else
	PDF_set_transition(pdf, arg2->value.lval);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_duration(int pdfdoc, double duration)
   Sets duration between pages */
PHP_FUNCTION(pdf_set_duration) {
	pval *arg1, *arg2;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_set_duration(pdf, (float) arg2->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int pdf_open_gif(int pdf, string giffile)
   Opens a gif file and returns an image for placement in a pdf document */
PHP_FUNCTION(pdf_open_gif) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	int pdf_image;
#else
	PDF_image *pdf_image;
#endif
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	pdf_image = PDF_open_GIF(pdf, arg2->value.str.val);

	if(pdf_image < 0) {
		php3_error(E_WARNING, "Could not open image");
		RETURN_FALSE;
	}

	id = php3_list_insert((void *) pdf_image,PDF_GLOBAL(le_pdf_image));
	RETURN_LONG(id);
}
/* }}} */

/* {{{ proto int pdf_open_jpeg(int pdf, string jpegfile)
   Opens a jpeg file and returns an image for placement in a pdf document */
PHP_FUNCTION(pdf_open_jpeg) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	int pdf_image;
#else
	PDF_image *pdf_image;
#endif
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	pdf_image = PDF_open_JPEG(pdf, arg2->value.str.val);

	if(pdf_image < 0) {
		php3_error(E_WARNING, "Could not open image");
		RETURN_FALSE;
	}

	id = php3_list_insert((void *) pdf_image,PDF_GLOBAL(le_pdf_image));
	RETURN_LONG(id);
}
/* }}} */

#if HAVE_LIBGD
#if HAVE_PDFLIB2
/* {{{ proto int pdf_open_memory_image(int pdf, int image)
   Takes an gd image and returns an image for placement in a pdf document */
PHP_FUNCTION(pdf_open_memory_image) {
	pval *argv[2];
	int argc;
	int i, j, id, gid, type, color, count;
	int pdf_image;
	gdImagePtr im;
	unsigned char *buffer, *ptr;
	PDF *pdf;
	PDF_TLS_VARS;

	argc = ARG_COUNT(ht);
	if (getParametersArray(ht, argc, argv) == FAILURE)
		WRONG_PARAM_COUNT;

	convert_to_long(argv[0]);
	id=argv[0]->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_long(argv[1]);
	gid=argv[1]->value.lval;
	im = php3_list_find(gid, &type);
	if (!im || type != php3i_get_le_gd()) {
		php3_error(E_WARNING, "pdf: Unable to find image pointer");
		RETURN_FALSE;
	}

	count = 3 * im->sx * im->sy;
	if(NULL == (buffer = (unsigned char *) emalloc(count)))
		RETURN_FALSE;

	ptr = buffer;
	for(i=0; i<im->sy; i++) {
		for(j=0; j<im->sx; j++) {
			color = im->pixels[i][j];
			*ptr++ = im->red[color];
			*ptr++ = im->green[color];
			*ptr++ = im->blue[color];
		}
	}


#if PDFLIB_MINORVERSION == 0
	pdf_image = PDF_open_memory_image(pdf, buffer, im->sx, im->sy, 3, 8);
#else
	pdf_image = PDF_open_image(pdf, "raw", "memory", buffer, im->sx*im->sy*3, im->sx, im->sy, 3, 8, NULL);
#endif
	efree(buffer);

	if(0 > pdf_image) {
		php3_error(E_WARNING, "Could not open image");
		RETURN_FALSE;
	}

	id = php3_list_insert((void *) pdf_image,PDF_GLOBAL(le_pdf_image));
	RETURN_LONG(id);
}
/* }}} */
#endif /* HAVE_PDFLIB2 */
#endif /* HAVE_LIBGD */

/* {{{ proto void pdf_close_image(int pdfimage)
   Closes the pdf image */
PHP_FUNCTION(pdf_close_image) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	int pdf_image;
#else
	PDF_image *pdf_image;
#endif
	PDF *pdf;
	PDF_TLS_VARS;

#if HAVE_PDFLIB2
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
#else
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
#endif

#if HAVE_PDFLIB2
	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
#else
#endif
	convert_to_long(arg2);
	id=arg2->value.lval;
#if HAVE_PDFLIB2
	pdf_image = (int) php3_list_find(id,&type);
	if(pdf_image < 0 || type!=PDF_GLOBAL(le_pdf_image)) {
#else
	pdf_image = php3_list_find(id,&type);
	if(!pdf_image || type!=PDF_GLOBAL(le_pdf_image)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

#if HAVE_PDFLIB2
	PDF_close_image(pdf, pdf_image);
	php3_list_delete(id);
#else
	/* See comment in _free_pdf_image() */
	PDF_close_image(NULL, pdf_image);
#endif

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_place_image(int pdf, int pdfimage, int x, int y, int scale)
   Places image in the pdf document */
PHP_FUNCTION(pdf_place_image) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5;
	int id, type;
#if HAVE_PDFLIB2
	int pdf_image;
#else
	PDF_image *pdf_image;
#endif
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 5 || getParameters(ht, 5, &arg1, &arg2, &arg3, &arg4, &arg5) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_long(arg2);
	id=arg2->value.lval;
#if HAVE_PDFLIB2
	pdf_image = (int) php3_list_find(id,&type);
	if(pdf_image < 0 || type!=PDF_GLOBAL(le_pdf_image)) {
#else
	pdf_image = php3_list_find(id,&type);
	if(!pdf_image || type!=PDF_GLOBAL(le_pdf_image)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);

	PDF_place_image(pdf, pdf_image, (float) arg3->value.dval, (float) arg4->value.dval, arg5->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_put_image(int pdf, int pdfimage)
   Stores image in the pdf document for later use */
PHP_FUNCTION(pdf_put_image) {
	pval *arg1, *arg2;
	int id, type;
#if HAVE_PDFLIB2
	int pdf_image;
#else
	PDF_image *pdf_image;
#endif
	PDF *pdf;
	PDF_TLS_VARS;
	
#if HAVE_PDFLIB2
#if PDFLIB_MINORVERSION > 0
	php3_error(E_WARNING, "Version 2.01 of pdflib does not need the pdf_put_image() anymore, check the docs!");
	RETURN_TRUE;
#endif
#endif
	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_long(arg2);
	id=arg2->value.lval;
#if HAVE_PDFLIB2
	pdf_image = (int) php3_list_find(id,&type);
	if(pdf_image < 0 || type!=PDF_GLOBAL(le_pdf_image)) {
#else
	pdf_image = php3_list_find(id,&type);
	if(!pdf_image || type!=PDF_GLOBAL(le_pdf_image)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}
	
	PDF_put_image(pdf, pdf_image);
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_execute_image(int pdf, int pdfimage, int x, int y, int scale)
   Places stored image in the pdf document */
PHP_FUNCTION(pdf_execute_image) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5;
	int id, type;
#if HAVE_PDFLIB2
	int pdf_image;
#else
	PDF_image *pdf_image;
#endif
	PDF *pdf;
	PDF_TLS_VARS;

#if HAVE_PDFLIB2
#if PDFLIB_MINORVERSION >= 01
	php3_error(E_WARNING, "Version 2.01 of pdflib does not need the pdf_execute_image() anymore, check the docs!");
	RETURN_TRUE;
#endif
#endif
	if (ARG_COUNT(ht) != 5 || getParameters(ht, 5, &arg1, &arg2, &arg3, &arg4, &arg5) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_long(arg2);
	id=arg2->value.lval;
#if HAVE_PDFLIB2
	pdf_image = (int) php3_list_find(id,&type);
	if(pdf_image < 0 || type!=PDF_GLOBAL(le_pdf_image)) {
#else
	pdf_image = php3_list_find(id,&type);
	if(!pdf_image || type!=PDF_GLOBAL(le_pdf_image)) {
#endif
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);

	PDF_execute_image(pdf, pdf_image, (float) arg3->value.dval, (float) arg4->value.dval, arg5->value.dval);

	RETURN_TRUE;
}
/* }}} */

#if HAVE_PDFLIB2
/* {{{ proto void pdf_get_image_width(int pdf, int pdfimage)
   Returns the width of an image */
PHP_FUNCTION(pdf_get_image_width) {
	pval *arg1, *arg2;
	int id, type;
	int width;
	int pdf_image;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_long(arg2);
	id=arg2->value.lval;
	pdf_image = (int) php3_list_find(id,&type);
	if(pdf_image < 0 || type!=PDF_GLOBAL(le_pdf_image)) {
		php3_error(E_WARNING,"Unable to find identifier %d",id);
		RETURN_FALSE;
	}

	width = PDF_get_image_width(pdf, pdf_image);

	RETURN_LONG(width);
}
/* }}} */

/* {{{ proto void pdf_get_image_height(int pdf, int pdfimage)
   Returns the height of an image */
PHP_FUNCTION(pdf_get_image_height) {
	pval *arg1, *arg2;
	int id, type;
	int height;
	int pdf_image;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 2 || getParameters(ht, 2, &arg1, &arg2) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find identifier %d",id);
		RETURN_FALSE;
	}

	convert_to_long(arg2);
	id=arg2->value.lval;
	pdf_image = (int) php3_list_find(id,&type);
	if(pdf_image < 0 || type!=PDF_GLOBAL(le_pdf_image)) {
		php3_error(E_WARNING,"Unable to find identifier %d",id);
		RETURN_FALSE;
	}

	height = PDF_get_image_height(pdf, pdf_image);

	RETURN_LONG(height);
}
/* }}} */

/* {{{ proto void pdf_add_weblink(int pdfdoc, double llx, double lly, double urx, double ury, string url)
   Adds link to web resource */
PHP_FUNCTION(pdf_add_weblink) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 6 || getParameters(ht, 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);
	convert_to_string(arg6);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_add_weblink(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval, (float) arg5->value.dval, arg6->value.str.val);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_add_pdflink(int pdfdoc, double llx, double lly, double urx, double ury, string filename, int page, string dest)
   Adds link to pdf document */
PHP_FUNCTION(pdf_add_pdflink) {
	pval *arg1, *arg2, *arg3, *arg4, *arg5, *arg6, *arg7, *arg8;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 8 || getParameters(ht, 8, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	convert_to_double(arg5);
	convert_to_string(arg6);
	convert_to_long(arg7);
	convert_to_string(arg8);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_add_pdflink(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval, (float) arg5->value.dval, arg6->value.str.val, arg7->value.lval, arg8->value.str.val);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_border_style(int pdfdoc, string style, double width)
   Set style of box surounded weblinks */
PHP_FUNCTION(pdf_set_border_style) {
	pval *arg1, *arg2, *arg3;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &arg1, &arg2, &arg3) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_string(arg2);
	convert_to_double(arg3);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_set_border_style(pdf, arg2->value.str.val, (float) arg3->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_set_border_color(int pdfdoc, double red, double green, double blue)
   Set color of box surounded weblinks */
PHP_FUNCTION(pdf_set_border_color) {
	pval *arg1, *arg2, *arg3, *arg4;
	int id, type;
	PDF *pdf;
	PDF_TLS_VARS;

	if (ARG_COUNT(ht) != 4 || getParameters(ht, 4, &arg1, &arg2, &arg3, &arg4) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(arg1);
	convert_to_double(arg2);
	convert_to_double(arg3);
	convert_to_double(arg4);
	id=arg1->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find file identifier %d",id);
		RETURN_FALSE;
	}

	PDF_set_border_color(pdf, (float) arg2->value.dval, (float) arg3->value.dval, (float) arg4->value.dval);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void pdf_add_annotation(int pdfdoc, double xll, double yll, double xur, double xur, string title, string text)
   Sets annotation */
PHP_FUNCTION(pdf_add_annotation) {
	pval *argv[11];
	int id, type, argc;
	PDF *pdf;
	PDF_TLS_VARS;

	argc = ARG_COUNT(ht);
	if(argc != 7)
		WRONG_PARAM_COUNT;
	if (getParametersArray(ht, argc, argv) == FAILURE)
		WRONG_PARAM_COUNT;

	convert_to_long(argv[0]);
	convert_to_double(argv[1]);
	convert_to_double(argv[2]);
	convert_to_double(argv[3]);
	convert_to_double(argv[4]);
	convert_to_string(argv[5]);
	convert_to_string(argv[6]);
	id=argv[0]->value.lval;
	pdf = php3_list_find(id,&type);
	if(!pdf || type!=PDF_GLOBAL(le_pdf)) {
		php3_error(E_WARNING,"Unable to find identifier %d",id);
		RETURN_FALSE;
	}

	PDF_add_note(pdf,
	             (float) argv[1]->value.dval,
	             (float) argv[2]->value.dval,
	             (float) argv[3]->value.dval,
	             (float) argv[4]->value.dval,
	             argv[6]->value.str.val,
	             argv[5]->value.str.val,
	             "note", 1);

	RETURN_TRUE;
}
/* }}} */
#endif /* HAVE_PDFLIB2 */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
