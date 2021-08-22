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
   | Authors: Andrey Zmievski <andrey@ispi.net>                           |
   +----------------------------------------------------------------------+
 */

/* $Id: pcre.c,v 1.24 2000/02/24 14:36:49 andrei Exp $ */

#include "php.h"

#if HAVE_PCRE

#include "php3_pcre.h"

#define PREG_PATTERN_ORDER	0
#define PREG_SET_ORDER		1

HashTable pcre_cache;

/* {{{ module definition structures */

unsigned char third_arg_force_ref[] = { 3, BYREF_NONE, BYREF_NONE, BYREF_FORCE };

function_entry pcre_functions[] = {
	PHP_FE(preg_match,		third_arg_force_ref)
	PHP_FE(preg_match_all,	third_arg_force_ref)
	PHP_FE(preg_replace,	NULL)
	PHP_FE(preg_split,		NULL)
	PHP_FE(preg_quote,		NULL)
	{NULL, 		NULL, 		NULL}
};

php3_module_entry pcre_module_entry = {
   "PCRE", pcre_functions, php_minit_pcre, php_mshutdown_pcre,
		   php_rinit_pcre, NULL,
		   php_info_pcre, STANDARD_MODULE_PROPERTIES
};

#if COMPILE_DL
DLEXPORT php3_module_entry *get_module(void) { return &pcre_module_entry; }
#endif

/* }}} */


static void *php_pcre_malloc(size_t size)
{
	return pemalloc(size, 1);
}


static void php_pcre_free(void *ptr)
{
	pefree(ptr, 1);
}


static void _php_free_pcre_cache(void *data)
{
	pcre_cache_entry *pce = (pcre_cache_entry *) data;
	pefree(pce->re, 1);
#if HAVE_SETLOCALE
	pefree((void*)pce->tables, 1);
#endif
}

/* {{{ void php_info_pcre(_php3_MODULE_INFO_FUNC_ARGS) */
void php_info_pcre(void)
{
	php3_printf("Perl Compatible Regular Expressions");
	php3_printf("<table cellpadding=5>"
				"<tr><td>PCRE library version:</td>"
				"<td>%s</td></tr>"
				"</table>", pcre_version());
}
/* }}} */


/* {{{ int php_minit_pcre(INIT_FUNC_ARGS) */
int php_minit_pcre(INIT_FUNC_ARGS)
{
	_php3_hash_init(&pcre_cache, 0, NULL, _php_free_pcre_cache, 1);
	
	REGISTER_LONG_CONSTANT("PREG_PATTERN_ORDER", PREG_PATTERN_ORDER, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PREG_SET_ORDER", PREG_SET_ORDER, CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
/* }}} */


/* {{{ int php_mshutdown_pcre(void) */
int php_mshutdown_pcre(SHUTDOWN_FUNC_ARGS)
{
	_php3_hash_destroy(&pcre_cache);
	return SUCCESS;
}
/* }}} */


/* {{{ int php_rinit_pcre(INIT_FUNC_ARGS) */
int php_rinit_pcre(INIT_FUNC_ARGS)
{
	pcre_malloc = php_pcre_malloc;
	pcre_free = php_pcre_free;
	
	return SUCCESS;
}
/* }}} */


/* {{{ static pcre* _pcre_get_compiled_regex(char *regex, pcre_extra *extra) */
static pcre* _pcre_get_compiled_regex(char *regex, pcre_extra *extra) {
	pcre				*re = NULL;
	int				 	 coptions = 0;
	int				 	 soptions = 0;
	const char	 		*error;
	int			 	 	 erroffset;
	char		 	 	 delimiter;
	char 				*p, *pp;
	char				*pattern;
	int				 	 regex_len;
	int				 	 do_study = 0;
	unsigned const char *tables = NULL;
#if HAVE_SETLOCALE
	char				*locale = setlocale(LC_CTYPE, NULL);
#endif
	pcre_cache_entry	*pce;
	pcre_cache_entry	 new_entry;

	/* Try to lookup the cached regex entry, and if successful, just pass
	   back the compiled pattern, otherwise go on and compile it. */
	regex_len = strlen(regex);
	if (_php3_hash_find(&pcre_cache, regex, regex_len+1, (void **)&pce) == SUCCESS) {
#if HAVE_SETLOCALE
		if (!strcmp(pce->locale, locale)) {
#endif
			extra = pce->extra;
			return pce->re;
#if HAVE_SETLOCALE
		}
#endif
	}
	
	p = regex;
	
	/* Parse through the leading whitespace, and display a warning if we
	   get to the end without encountering a delimiter. */
	while (isspace((int)*p)) p++;
	if (*p == 0) {
		php3_error(E_WARNING, "Empty regular expression");
		return NULL;
	}
	
	/* Get the delimiter and display a warning if it is alphanumeric
	   or a backslash. */
	delimiter = *p++;
	if (isalnum((int)delimiter) || delimiter == '\\') {
		php3_error(E_WARNING, "Delimiter must not be alphanumeric or backslash");
		return NULL;
	}
	
	/* We need to iterate through the pattern, searching for the ending delimiter,
	   but skipping the backslashed delimiters.  If the ending delimiter is not
	   found, display a warning. */
	pp = p;
	while (*pp != 0) {
		if (*pp == delimiter && pp[-1] != '\\')
			break;
		pp++;
	}
	if (*pp == 0) {
		php3_error(E_WARNING, "No ending delimiter found");
		return NULL;
	}
	
	/* Make a copy of the actual pattern. */
	pattern = estrndup(p, pp-p);

	/* Move on to the options */
	pp++;

	/* Parse through the options, setting appropriate flags.  Display
	   a warning if we encounter an unknown option. */	
	while (*pp != 0) {
		switch (*pp++) {
			case 'i':	coptions |= PCRE_CASELESS;		break;
			case 'm':	coptions |= PCRE_MULTILINE;		break;
			case 's':	coptions |= PCRE_DOTALL;		break;
			case 'x':	coptions |= PCRE_EXTENDED;		break;
			
			case 'A':	coptions |= PCRE_ANCHORED;		break;
			case 'D':	coptions |= PCRE_DOLLAR_ENDONLY;break;
			case 'S':	do_study  = 1;					break;
			case 'U':	coptions |= PCRE_UNGREEDY;		break;
			case 'X':	coptions |= PCRE_EXTRA;			break;

			case ' ':
			case '\n':
				break;

			default:
				php3_error(E_WARNING, "Unknown option '%c'", pp[-1]);
				efree(pattern);
				return NULL;
		}
	}

#if HAVE_SETLOCALE
	if (strcmp(locale, "C"))
		tables = pcre_maketables();
#endif

	/* Compile pattern and display a warning if compilation failed. */
	re = pcre_compile(pattern,
					  coptions,
					  &error,
					  &erroffset,
					  tables);

	if (re == NULL) {
		php3_error(E_WARNING, "Compilation failed: %s at offset %d\n", error, erroffset);
		efree(pattern);
		return NULL;
	}

	/* If study option was specified, study the pattern and
	   store the result in extra for passing to pcre_exec. */
	if (do_study) {
		extra = pcre_study(re, soptions, &error);
		if (error != NULL) {
			php3_error(E_WARNING, "Error while studying pattern");
		}
	}

	efree(pattern);

	/* Store the compiled pattern and extra info in the cache. */
	new_entry.re = re;
	new_entry.extra = extra;
#if HAVE_SETLOCALE
	new_entry.locale = locale;
	new_entry.tables = tables;
#endif
	_php3_hash_update(&pcre_cache, regex, regex_len+1, (void *)&new_entry,
						sizeof(pcre_cache_entry), NULL);

	return re;
}
/* }}} */


/* {{{ void _pcre_match(INTERNAL_FUNCTION_PARAMETERS, int global) */
void _pcre_match(INTERNAL_FUNCTION_PARAMETERS, int global)
{
	pval			*regex,				/* Regular expression */
					*subject,			/* String to match against */
					*subpats = NULL,	/* Array for subpatterns */
					*subpats_order,		/* Order of the results in the subpatterns
										   array for global match */
					*result_set,		/* Holds a set of subpatterns after
										   a global match */
				   **match_sets = NULL;	/* An array of sets of matches for each
										   subpattern after a global match */
	pcre			*re = NULL;			/* Compiled regular expression */
	pcre_extra		*extra = NULL;		/* Holds results of studying */
	int			 	 exoptions = 0;		/* Execution options */
	int			 	 count = 0;			/* Count of matched subpatterns */
	int			 	*offsets;			/* Array of subpattern offsets */
	int				 num_subpats;		/* Number of captured subpatterns */
	int			 	 size_offsets;		/* Size of the offsets array */
	int			 	 matched;			/* Has anything matched */
	int				 i;
	int				 subpats_order_val = 0;	/* Integer value of subpats_order */
	const char	   **stringlist;		/* Used to hold list of subpatterns */
	char			*match,				/* The current match */
					*piece,				/* The current piece of subject */
					*subject_end;		/* Points to the end of the subject */
	
	/* Get function parameters and do error-checking. */
	switch(ARG_COUNT(ht)) {
		case 2:
			if (global || getParameters(ht, 2, &regex, &subject) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
			break;
			
		case 3:
			if (getParameters(ht, 3, &regex, &subject, &subpats) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
			if (global)
				subpats_order_val = PREG_PATTERN_ORDER;
			if (!ParameterPassedByReference(ht, 3)) {
				php3_error(E_WARNING, "Array to be filled with matches must be passed by reference.");
				RETURN_FALSE;
			}
			break;

		case 4:
			if (getParameters(ht, 4, &regex, &subject, &subpats, &subpats_order) == FAILURE) {
				WRONG_PARAM_COUNT;
			}
			if (!ParameterPassedByReference(ht, 3)) {
				php3_error(E_WARNING, "Array to be filled with matches must be passed by reference.");
				RETURN_FALSE;
			}
	
			/* Make sure subpats_order is a number */
			convert_to_long(subpats_order);
			subpats_order_val = subpats_order->value.lval;
			if (subpats_order_val < PREG_PATTERN_ORDER ||
				subpats_order_val > PREG_SET_ORDER) {
				php3_error(E_WARNING, "Wrong value for parameter 4 in call to preg_match_all()");
			}
			break;
			
		default:
			WRONG_PARAM_COUNT;
	}

	/* Make sure we're dealing with strings. */
	convert_to_string(regex);
	convert_to_string(subject);

	/* Make sure to clean up the passed array and initialize it. */
	if (subpats != NULL) {
		php3tls_pval_destructor(subpats _INLINE_TLS);
		array_init(subpats);
	}

	/* Compile regex or get it from cache. */
	if ((re = _pcre_get_compiled_regex(regex->value.str.val, extra)) == NULL) {
		RETURN_FALSE;
	}

	/* Calculate the size of the offsets array, and allocate memory for it. */
	num_subpats = pcre_info(re, NULL, NULL) + 1;
	size_offsets = num_subpats * 3;
	offsets = (int *)emalloc(size_offsets * sizeof(int));

	/* Allocate match sets array and initialize the values */
	if (global && subpats_order_val == PREG_PATTERN_ORDER) {
		match_sets = (pval **)emalloc(num_subpats * sizeof(pval *));
		for (i=0; i<num_subpats; i++) {
			match_sets[i] = (pval *)emalloc(sizeof(pval));
			array_init(match_sets[i]);
		}
	}

	/* Start from the beginning of the string */
	piece = subject->value.str.val;
	subject_end = piece + subject->value.str.len;
	match = NULL;
	matched = 0;
	
	do {
		/* Execute the regular expression. */
		count = pcre_exec(re, extra, piece,
						  subject_end-piece, subject->value.str.val,
						  (piece==subject->value.str.val ? exoptions : exoptions|PCRE_NOTBOL),
						  offsets, size_offsets, (piece == match));

		/* Check for too many substrings condition. */	
		if (count == 0) {
			php3_error(E_NOTICE, "Matched, but too many substrings\n");
			count = size_offsets/3;
		}

		/* If something has matched */
		if (count >= 0) {
			matched++;
			match = piece + offsets[0];

			/* If subpatters array has been passed, fill it in with values. */
			if (subpats != NULL) {
				/* Try to get the list of substrings and display a warning if failed. */
				if (pcre_get_substring_list(piece, offsets, count, &stringlist) < 0) {
					efree(offsets);
					php3_error(E_WARNING, "Get subpatterns list failed");
					return;
				}

				if (global) {	/* global pattern matching */
					if (subpats_order_val == PREG_PATTERN_ORDER) {
						/* For each subpattern, insert it into the appropriate array */
						for (i=0; i<count; i++) {
							add_next_index_string(match_sets[i], (char *)stringlist[i], 1);
						}
					}
					else {
						/* Allocate the result set array */
						result_set = emalloc(sizeof(pval));
						array_init(result_set);
						
						/* Add all the subpatterns to it */
						for (i=0; i<count; i++) {
							add_next_index_string(result_set, (char *)stringlist[i], 1);
						}
						/* And add it to the output array */
						_php3_hash_next_index_insert(subpats->value.ht, result_set,
								sizeof(pval), NULL);
					}
				}
				else {			/* single pattern matching */
					/* For each subpattern, insert it into the subpatterns array. */
					for (i=0; i<count; i++) {
						add_next_index_string(subpats, (char *)stringlist[i], 1);
					}
				}

				php_pcre_free((void *)stringlist);
				
				/* Advance to the position right after the last full match */
				piece += offsets[1];
			}
		}
	} while (global && count >= 0);

	/* Add the match sets to the output array and clean up */
	if (global && subpats_order_val == PREG_PATTERN_ORDER) {
		for (i=0; i<num_subpats; i++) {
			_php3_hash_next_index_insert(subpats->value.ht, match_sets[i], sizeof(pval), NULL);
			efree(match_sets[i]);
		}
		efree(match_sets);
	}
	
	efree(offsets);

	RETVAL_LONG(matched);
}
/* }}} */


/* {{{ proto int preg_match(string pattern, string subject [, array subpattern])
   Perform a Perl-style regular expression match */
PHP_FUNCTION(preg_match)
{
	_pcre_match(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */


/* {{{ proto int preg_match_all(string pattern, string subject, array subpatterns [, int order])
   Perform a Perl-style global regular expression match */
PHP_FUNCTION(preg_match_all)
{
	_pcre_match(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */


/* {{{ int _pcre_get_backref(const char *walk, int *backref) */
static int _pcre_get_backref(const char *walk, int *backref)
{
	if (*walk && *walk >= '0' && *walk <= '9')
		*backref = *walk - '0';
	else
		return 0;
	
	if (walk[1] && walk[1] >= '0' && walk[1] <= '9')
		*backref = *backref * 10 + walk[1] - '0';

	return 1;
}
/* }}} */


/* {{{ char *_php_pcre_replace(char *regex, char *subject, char *replace) */
char *_php_pcre_replace(char *regex, char *subject, char *replace)
{
	pcre			*re = NULL;			/* Compiled regular expression */
	pcre_extra		*extra = NULL;		/* Holds results of studying */
	int			 	 exoptions = 0;		/* Execution options */
	int			 	 count = 0;			/* Count of matched subpatterns */
	int			 	*offsets;			/* Array of subpattern offsets */
	int			 	 size_offsets;		/* Size of the offsets array */
	int				 new_len;			/* Length of needed storage */
	int				 alloc_len;			/* Actual allocated length */
	int				 subject_len;		/* Length of the subject string */
	int				 result_len;		/* Current length of the result */
	int				 backref;			/* Backreference number */
	char			*result,			/* Result of replacement */
					*new_buf,			/* Temporary buffer for re-allocation */
					*walkbuf,			/* Location of current replacement in the result */
					*walk,				/* Used to walk the replacement string */
					*match,				/* The current match */
					*piece,				/* The current piece of subject */
					*subject_end;		/* Points to the end of the subject */

	/* Compile regex or get it from cache. */
	if ((re = _pcre_get_compiled_regex(regex, extra)) == NULL) {
		return NULL;
	}

	/* Calculate the size of the offsets array, and allocate memory for it. */
	size_offsets = (pcre_info(re, NULL, NULL) + 1) * 3;
	offsets = (int *)emalloc(size_offsets * sizeof(int));
	
	subject_len = strlen(subject);

	alloc_len = 2 * subject_len + 1;
	result = emalloc(alloc_len * sizeof(char));
	if (!result) {
		php3_error(E_WARNING, "Unable to allocate memory in pcre_replace");
		efree(offsets);
		return NULL;
	}

	/* Initialize */
	result[0] = '\0';
	piece = subject;
	subject_end = subject + subject_len;
	match = NULL;
	
	while (count >= 0) {
		/* Execute the regular expression. */
		count = pcre_exec(re, extra, piece,
							subject_end-piece, subject,
						  	(piece==subject ? exoptions : exoptions|PCRE_NOTBOL),
							offsets, size_offsets, (piece == match));
		
		/* Check for too many substrings condition. */
		if (count == 0) {
			php3_error(E_NOTICE, "Matched, but too many substrings\n");
			count = size_offsets/3;
		}

		if (count > 0) {
			/* Set the match location in piece */
			match = piece + offsets[0];

			new_len = strlen(result) + offsets[0]; /* part before the match */
			walk = replace;
			while (*walk)
				if ('\\' == *walk &&
					_pcre_get_backref(walk+1, &backref) &&
					backref < count) {
					new_len += offsets[(backref<<1)+1] - offsets[backref<<1];
					walk += (backref > 9) ? 3 : 2;
				} else {
					new_len++;
					walk++;
				}

			if (new_len + 1 > alloc_len) {
				alloc_len = 1 + alloc_len + 2 * new_len;
				new_buf = emalloc(alloc_len);
				strcpy(new_buf, result);
				efree(result);
				result = new_buf;
			}
			result_len = strlen(result);
			/* copy the part of the string before the match */
			strncat(result, piece, match-piece);

			/* copy replacement and backrefs */
			walkbuf = &result[result_len + offsets[0]];
			walk = replace;
			while (*walk)
				if ('\\' == *walk &&
					_pcre_get_backref(walk+1, &backref) &&
					backref < count) {
					result_len = offsets[(backref<<1)+1] - offsets[backref<<1];
					memcpy (walkbuf,
							piece + offsets[backref<<1],
							result_len);
					walkbuf += result_len;
					walk += (backref > 9) ? 3 : 2;
				} else
					*walkbuf++ = *walk++;
			*walkbuf = '\0';

			/* Advance to the next piece */
			piece += offsets[1];
		} else {
			new_len = strlen(result) + subject_end-piece;
			if (new_len + 1 > alloc_len) {
				alloc_len = new_len + 1; /* now we know exactly how long it is */
				new_buf = emalloc(alloc_len * sizeof(char));
				strcpy(new_buf, result);
				efree(result);
				result = new_buf;
			}
			/* stick that last bit of string on our output */
			strcat(result, piece);
		}
	}
	
	efree(offsets);

	return result;
}
/* }}} */


static char *_php_replace_in_subject(pval *regex, pval *replace, pval *subject)
{
	pval		*regex_entry,
				*replace_entry;
	char		*replace_value = NULL,
				*subject_value,
				*result;

	/* Make sure we're dealing with strings. */	
	convert_to_string(subject);

	/* If regex is an array */
	if (regex->type == IS_ARRAY) {
		/* Duplicating subject string for repeated replacement */
		subject_value = estrdup(subject->value.str.val);
		
		_php3_hash_internal_pointer_reset(regex->value.ht);

		if (replace->type == IS_ARRAY)
			_php3_hash_internal_pointer_reset(replace->value.ht);
		else
			/* Set replacement value to the passed one */
			replace_value = replace->value.str.val;
		
		/* For each entry in the regex array, get the entry */
		while (_php3_hash_get_current_data(regex->value.ht, (void **)&regex_entry) == SUCCESS) {

			/* Make sure we're dealing with strings. */	
			convert_to_string(regex_entry);
		
			/* If replace is an array */
			if (replace->type == IS_ARRAY) {
				/* Get current entry */
				if (_php3_hash_get_current_data(replace->value.ht, (void **)&replace_entry) == SUCCESS) {
					
					/* Make sure we're dealing with strings. */	
					convert_to_string(replace_entry);
					
					/* Set replacement value to the one we got from array */
					replace_value = replace_entry->value.str.val;

					_php3_hash_move_forward(replace->value.ht);
				}
				else
					/* We've run out of replacement strings, so use an empty one */
					replace_value = empty_string;
			}
			
			/* Do the actual replacement and put the result back into subject_value
			   for further replacements. */
			if ((result = _php_pcre_replace(regex_entry->value.str.val,
											subject_value,
											replace_value)) != NULL) {
				efree(subject_value);
				subject_value = result;
			}
			
			_php3_hash_move_forward(regex->value.ht);
		}

		return subject_value;
	}
	else {
		/* Make sure we're dealing with strings and do the replacement */
		convert_to_string(regex);
		convert_to_string(replace);
		result = _php_pcre_replace(regex->value.str.val,
									subject->value.str.val,
									replace->value.str.val);
		return result;
	}
}


/* {{{ proto string preg_replace(mixed regex, mixed replace, mixed subject)
   Perform Perl-style regular expression replacement */
PHP_FUNCTION(preg_replace)
{
	pval			*regex,
					*replace,
					*subject,
					*subject_entry;
	char			*result;
	
	/* Get function parameters and do error-checking. */
	if (ARG_COUNT(ht) != 3 || getParameters(ht, 3, &regex, &replace, &subject) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	/* if subject is an array */
	if (subject->type == IS_ARRAY) {
		array_init(return_value);
		_php3_hash_internal_pointer_reset(subject->value.ht);
		
		/* For each subject entry, convert it to string, then perform replacement
		   and add the result to the return_value array. */
		while (_php3_hash_get_current_data(subject->value.ht, (void **)&subject_entry) == SUCCESS) {
			if ((result = _php_replace_in_subject(regex, replace, subject_entry)) != NULL)
				add_next_index_string(return_value, result, 0);
		
			_php3_hash_move_forward(subject->value.ht);
		}
	}
	else {	/* if subject is not an array */
		if ((result = _php_replace_in_subject(regex, replace, subject)) != NULL) {
			RETVAL_STRING(result, 1);
			efree(result);
		}
	}	
}
/* }}} */


/* {{{ proto array preg_split(string pattern, string subject [, int limit ])
   Split string into an array using a perl-style regular expression as a delimiter */
PHP_FUNCTION(preg_split)
{
	pval			*regex,				/* Regular expression to split by */
					*subject,			/* Subject string to split */
					*limit;				/* Number of pieces to return */
	pcre			*re = NULL;			/* Compiled regular expression */
	pcre_extra		*extra = NULL;		/* Holds results of studying */
	int			 	*offsets;			/* Array of subpattern offsets */
	int			 	 size_offsets;		/* Size of the offsets array */
	int				 exoptions = 0;		/* Execution options */
	int				 argc;				/* Argument count */
	int				 limit_val;			/* Integer value of limit */
	int				 count = 0;			/* Count of matched subpatterns */
	char			*match,				/* The current match */
					*piece,				/* The current piece of subject */
					*subject_end;		/* Points to the end of subject string */

	/* Get function parameters and do error checking */	
	argc = ARG_COUNT(ht);
	if (argc < 1 || argc > 3 || getParameters(ht, argc, &regex, &subject, &limit) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	if (argc == 3) {
		convert_to_long(limit);
		limit_val = limit->value.lval;
	}
	else
		limit_val = -1;
	
	/* Make sure we're dealing with strings */
	convert_to_string(regex);
	convert_to_string(subject);
	
	/* Compile regex or get it from cache. */
	if ((re = _pcre_get_compiled_regex(regex->value.str.val, extra)) == NULL) {
		RETURN_FALSE;
	}
	
	/* Initialize return value */
	array_init(return_value);

	/* Calculate the size of the offsets array, and allocate memory for it. */
	size_offsets = (pcre_info(re, NULL, NULL) + 1) * 3;
	offsets = (int *)emalloc(size_offsets * sizeof(int));
	
	/* Start at the beginning of the string */
	piece = subject->value.str.val;
	subject_end = piece + subject->value.str.len;
	match = NULL;
	
	/* Get next piece if no limit or limit not yet reached and something matched*/
	while ((limit_val == -1 || limit_val > 1) && count >= 0) {
		count = pcre_exec(re, extra, piece,
						  subject_end-piece, subject->value.str.val,
						  (piece==subject->value.str.val ? exoptions : exoptions|PCRE_NOTBOL),
						  offsets, size_offsets, (piece==match));

		/* Check for too many substrings condition. */
		if (count == 0) {
			php3_error(E_NOTICE, "Matched, but too many substrings\n");
			count = size_offsets/3;
		}

		/* If something matched */
		if (count > 0) {
			match = piece + offsets[0];

			/* Add the piece to the return value */
			add_next_index_stringl(return_value,
								   piece,
								   offsets[0], 1);
			
			/* Advance to next position */
			piece += offsets[1];
			
			/* One less left to do */
			if (limit_val != -1)
				limit_val--;
		}
	}
	
	/* Add the last piece to the return value */
	add_next_index_stringl(return_value,
						   piece ,
						   subject_end-piece, 1);
	/* Clean up */
	efree(offsets);
}
/* }}} */


/* {{{ proto string preg_quote(string str)
   Quote regular expression characters */
PHP_FUNCTION(preg_quote)
{
	pval 	*in_str_arg;	/* Input string argument */
	char 	*in_str,		/* Input string */
			*out_str,		/* Output string with quoted characters */
		 	*p,				/* Iterator for input string */
			*q,				/* Iterator for output string */
		 	 c;				/* Current character */
	
	/* Get the arguments and check for errors */
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &in_str_arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	/* Make sure we're working with strings */
	convert_to_string(in_str_arg);
	in_str = in_str_arg->value.str.val;

	/* Nothing to do if we got an empty string */
	if (!*in_str) {
		RETVAL_STRING(empty_string, 0);
	}
	
	/* Allocate enough memory so that even if each character
	   is quoted, we won't run out of room */
	out_str = emalloc(2 * in_str_arg->value.str.len + 1);
	
	/* Go through the string and quote necessary characters */
	for(p = in_str, q = out_str; (c = *p); p++) {
		switch(c) {
			case '.':
			case '\\':
			case '+':
			case '*':
			case '?':
			case '[':
			case '^':
			case ']':
			case '$':
			case '(':
			case ')':
			case '{':
			case '}':
			case '=':
			case '!':
			case '>':
			case '<':
			case '|':
			case ':':
				*q++ = '\\';
				/* break is missing _intentionally_ */
			default:
				*q++ = c;
		}
	}
	*q = '\0';
	
	/* Reallocate string and return it */
	RETVAL_STRING(erealloc(out_str, q - out_str + 1), 0);
}
/* }}} */


#endif /* HAVE_PCRE */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
