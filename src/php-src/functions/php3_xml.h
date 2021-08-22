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
   | Authors: Stig S�ther Bakken <ssb@fast.no>                            |
   +----------------------------------------------------------------------+
 */

/* $Id */

#if HAVE_LIBEXPAT
# ifndef _PHP_XML_H
#  define _PHP_XML_H
# endif

#if RAW_XML_INCLUDEPATH
#include <xmltok.h>
#include <xmlparse.h>
#else
#include <xml/xmltok.h>
#include <xml/xmlparse.h>
#endif

#ifdef XML_UNICODE
# error "UTF-16 Unicode support not implemented!"
#endif

typedef struct {
	int le_xml_parser;
	XML_Char *default_encoding;
} xml_module;

typedef struct {
	int index;
	int case_folding;
	XML_Parser parser;
	XML_Char *target_encoding;
	char *startElementHandler;
	char *endElementHandler;
	char *characterDataHandler;
	char *processingInstructionHandler;
	char *defaultHandler;
	char *unparsedEntityDeclHandler;
	char *notationDeclHandler;
	char *externalEntityRefHandler;
	char *unknownEncodingHandler;

	pval *data;
	pval *info;
	int level;
	int toffset;
	int curtag;
	pval *ctag;
	char **ltags;
	int lastwasopen;
	int skipwhite;
	
	XML_Char *baseURI;
} xml_parser;


typedef struct {
	XML_Char *name;
	char (*decoding_function)(unsigned short);
	unsigned short (*encoding_function)(unsigned char);
} xml_encoding;


extern php3_module_entry xml_module_entry;
# define xml_module_ptr &xml_module_entry

enum php3_xml_option {
    PHP3_XML_OPTION_CASE_FOLDING = 1,
    PHP3_XML_OPTION_TARGET_ENCODING,
    PHP3_XML_OPTION_SKIP_TAGSTART,
    PHP3_XML_OPTION_SKIP_WHITE
};

# define RETURN_OUT_OF_MEMORY \
	php3_error(E_WARNING, "Out of memory");\
	RETURN_FALSE

/* for xml_parse_into_struct */

#define XML_MAXLEVEL 255 /* XXX this shoud be dynamic */

PHP_FUNCTION(xml_parser_create);
PHP_FUNCTION(xml_set_element_handler);
PHP_FUNCTION(xml_set_character_data_handler);
PHP_FUNCTION(xml_set_processing_instruction_handler);
PHP_FUNCTION(xml_set_default_handler);
PHP_FUNCTION(xml_set_unparsed_entity_decl_handler);
PHP_FUNCTION(xml_set_notation_decl_handler);
PHP_FUNCTION(xml_set_external_entity_ref_handler);
PHP_FUNCTION(xml_parse);
PHP_FUNCTION(xml_parse_into_struct);
PHP_FUNCTION(xml_get_error_code);
PHP_FUNCTION(xml_error_string);
PHP_FUNCTION(xml_get_current_line_number);
PHP_FUNCTION(xml_get_current_column_number);
PHP_FUNCTION(xml_get_current_byte_index);
PHP_FUNCTION(xml_parser_free);
PHP_FUNCTION(xml_parser_set_option);
PHP_FUNCTION(xml_parser_get_option);
PHP_FUNCTION(utf8_encode);
PHP_FUNCTION(utf8_decode);

#else /* !HAVE_LIBEXPAT */

# define xml_module_ptr NULL

#endif /* HAVE_LIBEXPAT */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
