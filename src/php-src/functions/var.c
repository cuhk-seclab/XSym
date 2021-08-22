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
   | Authors: Jani Lehtim�ki <jkl@njet.net>                               |
   +----------------------------------------------------------------------+
 */

/* $Id: var.c,v 1.19 2000/01/01 04:31:17 sas Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "php.h"
#include "head.h"
#include "internal_functions.h"
#include "fopen-wrappers.h"
#include "reg.h"
#include "post.h"
#include "php3_string.h"
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include "php3_var.h"

void php3api_var_dump(pval *struc, int level)
{
	ulong index;
	char *key;
	int i, c = 0;
	pval *data;
	char buf[512];

	if(!php3_header()) return;

	switch (struc->type) {
		case IS_LONG:
			i = sprintf(buf, "%*cint(%ld)\n", level, ' ', struc->value.lval);
			PHPWRITE(&buf[1], i - 1);
			break;

		case IS_DOUBLE:
			i = sprintf(buf, "%*cfloat(%.*G)\n", level, ' ', (int) php3_ini.precision, struc->value.dval);
			PHPWRITE(&buf[1], i - 1);
			break;

		case IS_STRING:
			i = sprintf(buf, "%*cstring(%d) \"", level, ' ', struc->value.str.len);
			PHPWRITE(&buf[1], i - 1);
			PHPWRITE(struc->value.str.val, struc->value.str.len);
			strcpy(buf, "\"\n");
			PHPWRITE(buf, strlen(buf));
			break;

		case IS_ARRAY:
			i = sprintf(buf, "%*carray(%d) {\n", level, ' ', _php3_hash_num_elements(struc->value.ht));
			PHPWRITE(&buf[1], i - 1);
			goto head_done;

		case IS_OBJECT:
			i = sprintf(buf, "%*cobject(%d) {\n", level, ' ', _php3_hash_num_elements(struc->value.ht));
			PHPWRITE(&buf[1], i - 1);
		  head_done:

			_php3_hash_internal_pointer_reset(struc->value.ht);
			for (;; _php3_hash_move_forward(struc->value.ht)) {
				if ((i = _php3_hash_get_current_key(struc->value.ht, &key, &index)) == HASH_KEY_NON_EXISTANT)
					break;
				if (c > 0) {
					strcpy(buf, "\n");
					PHPWRITE(buf, strlen(buf));
				}
				if (_php3_hash_get_current_data(struc->value.ht, (void **) (&data)) != SUCCESS || !data || (data == struc))
					continue;
				if (data->type==IS_STRING && data->value.str.val==undefined_variable_string) {
					continue;
				}
				c++;
				switch (i) {
					case HASH_KEY_IS_LONG:{
							pval d;

							d.type = IS_LONG;
							d.value.lval = index;
							php3api_var_dump(&d, level + 2);
						}
						break;

					case HASH_KEY_IS_STRING:{
							pval d;

							d.type = IS_STRING;
							d.value.str.val = key;
							d.value.str.len = strlen(key);
							php3api_var_dump(&d, level + 2);
							efree(key);
						}
						break;
				}
				php3api_var_dump(data, level + 2);
			}
			i = sprintf(buf, "%*c}\n", level, ' ');
			PHPWRITE(&buf[1], i - 1);
			break;

		default:
			i = sprintf(buf, "%*ci:0\n", level, ' ');
			PHPWRITE(&buf[1], i - 1);
	}
}

/* {{{ proto void var_dump(mixed var)
   Dumps a string representation of variable to output */
PHP_FUNCTION(var_dump)
{
	pval *struc;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &struc) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	php3api_var_dump(struc, 1);
}
/* }}} */


#define STR_CAT(P,S,I) {\
	pval *__p = (P);\
	ulong __i = __p->value.str.len;\
	__p->value.str.len += (I);\
	if (__p->value.str.val) {\
		__p->value.str.val = (char *)erealloc(__p->value.str.val, __p->value.str.len + 1);\
	} else {\
		__p->value.str.val = emalloc(__p->value.str.len + 1);\
		*__p->value.str.val = 0;\
	}\
	strcat(__p->value.str.val + __i, (S));\
}


void php3api_var_serialize(pval *buf, pval *struc)
{
	char s[256];
	ulong slen;
	int i, ch;

	switch (struc->type) {
		case IS_LONG:
			slen = sprintf(s, "i:%ld;", struc->value.lval);
			STR_CAT(buf, s, slen);
			return;

		case IS_DOUBLE:
			slen = sprintf(s, "d:%.*G;",  (int) php3_ini.precision, struc->value.dval);
			STR_CAT(buf, s, slen);
			return;

		case IS_STRING:{
				char *p;
				
				i = buf->value.str.len;
				slen = sprintf(s, "s:%d:\"", struc->value.str.len);
				STR_CAT(buf, s, slen + struc->value.str.len + 2);
				p = buf->value.str.val + i + slen;
				if (struc->value.str.len > 0) {
					memcpy(p, struc->value.str.val, struc->value.str.len);
					p += struc->value.str.len;
				}
				*p++ = '\"';
				*p++ = ';';
				*p = 0;
			}
			return;

		case IS_ARRAY:
			ch = 'a';
			goto got_array;

		case IS_OBJECT:
			ch = 'o';

		  got_array:
			i = _php3_hash_num_elements(struc->value.ht);
			slen = sprintf(s, "%c:%d:{", ch, i);
			STR_CAT(buf, s, slen);
			if (i > 0) {
				char *key;
				pval *data;
				pval d;
				ulong index;

				_php3_hash_internal_pointer_reset(struc->value.ht);
				for (;; _php3_hash_move_forward(struc->value.ht)) {
					if ((i = _php3_hash_get_current_key(struc->value.ht, &key, &index)) == HASH_KEY_NON_EXISTANT) {
						break;
					}
					if (_php3_hash_get_current_data(struc->value.ht, (void **) (&data)) != SUCCESS || !data || (data == struc)) {
						continue;
					}
					if (data->type==IS_STRING && data->value.str.val==undefined_variable_string) {
						continue;
					}
				
					switch (i) {
						case HASH_KEY_IS_LONG:
							d.type = IS_LONG;
							d.value.lval = index;
							php3api_var_serialize(buf, &d);
							break;
						case HASH_KEY_IS_STRING:
							d.type = IS_STRING;
							d.value.str.val = key;
							d.value.str.len = strlen(key);
							php3api_var_serialize(buf, &d);
							efree(key);
							break;
					}
					php3api_var_serialize(buf, data);
				}
			}
			STR_CAT(buf, "}", 1);
			return;

		default:
			STR_CAT(buf, "i:0;", 4);
			return;
	}
}


int php3api_var_unserialize(pval *rval, char **p, char *max)
{
	char *q;
	char *str;
	int i;

	switch (**p) {
		case 'i':
			if (*((*p) + 1) != ':') {
				return 0;
			}
			q = *p;
			while (**p && **p != ';') {
				(*p)++;
			}
			if (**p != ';') {
				return 0;
			}
			(*p)++;
			rval->type = IS_LONG;
			rval->value.lval = atol(q + 2);
			return 1;

		case 'd':
			if (*((*p) + 1) != ':') {
				return 0;
			}
			q = *p;
			while (**p && **p != ';') {
				(*p)++;
			}
			if (**p != ';') {
				return 0;
			}
			(*p)++;
			rval->type = IS_DOUBLE;
			rval->value.dval = atof(q + 2);
			return 1;

		case 's':
			if (*((*p) + 1) != ':') {
				return 0;
			}
			(*p) += 2;
			q = *p;
			while (**p && **p != ':') {
				(*p)++;
			}
			if (**p != ':') {
				return 0;
			}
			i = atoi(q);
			if (i < 0 || (*p + 3 + i) > max || *((*p) + 1) != '\"' ||
				*((*p) + 2 + i) != '\"' || *((*p) + 3 + i) != ';') {
				return 0;
			}
			(*p) += 2;
			str = emalloc(i + 1);
			if (i > 0) {
				memcpy(str, *p, i);
			}
			str[i] = 0;
			(*p) += i + 2;
			rval->type = IS_STRING;
			rval->value.str.val = str;
			rval->value.str.len = i;
			return 1;

		case 'a':
			rval->type = IS_ARRAY;
			goto got_array;

		case 'o':
			rval->type = IS_OBJECT;

		  got_array:
			(*p) += 2;
			i = atoi(*p);
			rval->value.ht = (HashTable *) emalloc(sizeof(HashTable));
			_php3_hash_init(rval->value.ht, i + 1, NULL, PVAL_DESTRUCTOR, 0);
			while (**p && **p != ':') {
				(*p)++;
			}
			if (**p != ':' || *((*p) + 1) != '{') {
				return 0;
			}
			for ((*p) += 2; **p && **p != '}' && i > 0; i--) {
				pval key;
				pval data;
				
				if (!php3api_var_unserialize(&key, p, max)) {
					return 0;
				}
				if (!php3api_var_unserialize(&data, p, max)) {
					return 0;
				}
				switch (key.type) {
					case IS_LONG:
						_php3_hash_index_update(rval->value.ht, key.value.lval, &data, sizeof(data), NULL);
						break;
					case IS_STRING:
						_php3_hash_add(rval->value.ht, key.value.str.val, key.value.str.len + 1, &data, sizeof(data), NULL);
						break;
				}
				pval_destructor(&key);
			}
			return *((*p)++) == '}';

		default:
			return 0;
	}
}


/* {{{ proto string serialize(mixed variable)
   Returns a string representation of variable (which can later be unserialized) */
PHP_FUNCTION(serialize)
{
	pval *struc;

	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &struc) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	return_value->type = IS_STRING;
	return_value->value.str.val = NULL;
	return_value->value.str.len = 0;
	php3api_var_serialize(return_value, struc);
}
/* }}} */


/* {{{ proto mixed unserialize(string variable_representation)
   Takes a string representation of variable and recreates it */
PHP_FUNCTION(unserialize)
{
	pval *buf;
	
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &buf) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	if (buf->type == IS_STRING) {
		char *p = buf->value.str.val;
		if (!php3api_var_unserialize(return_value, &p, p + buf->value.str.len)) {
			RETURN_FALSE;
		}
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
