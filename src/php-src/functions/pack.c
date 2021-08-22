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
   | Authors: Chris Schneider <cschneid@relog.ch>                         |
   +----------------------------------------------------------------------+
 */
/* $Id: pack.c,v 1.26 2000/07/09 14:26:48 eschmid Exp $ */
#include "php.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if MSVC5
#include <windows.h>
#include <winsock.h>
#define O_RDONLY _O_RDONLY
#include "win32/param.h"
#else
#include <sys/param.h>
#endif
#include "head.h"
#include "internal_functions.h"
#include "safe_mode.h"
#include "php3_list.h"
#include "php3_string.h"
#include "pack.h"
#if HAVE_PWD_H
#if MSVC5
#include "win32/pwd.h"
#else
#include <pwd.h>
#endif
#endif
#include "snprintf.h"
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include "fsock.h"

function_entry pack_functions[] = {
	{"pack",		php3_pack,	NULL},
	{"unpack",		php3_unpack,		NULL},
	{NULL, NULL, NULL}
};

php3_module_entry pack_module_entry = {
	"PHP_pack", pack_functions, php3_minit_pack, NULL, NULL, NULL, NULL, STANDARD_MODULE_PROPERTIES
};

/* Whether machine is little endian */
char machine_little_endian;

/* Mapping of byte from char (8bit) to long for machine endian */
static int byte_map[1];

/* Mappings of bytes from int (machine dependant) to int for machine endian */
static int int_map[sizeof(int)];

/* Mappings of bytes from shorts (16bit) for all endian environments */
static int machine_endian_short_map[2];
static int big_endian_short_map[2];
static int little_endian_short_map[2];

/* Mappings of bytes from longs (32bit) for all endian environments */
static int machine_endian_long_map[4];
static int big_endian_long_map[4];
static int little_endian_long_map[4];


static void _php3_pack(pval *val, int size, int *map, char *output)
{
	int i;
	char *v;

	convert_to_long(val);
	v = (char *)&val->value.lval;

	for (i = 0; i < size; i++) {
		*(output++) = v[map[i]];
	}
}


/* pack() idea stolen from Perl (implemented formats behave the same as there)
 * Implemented formats are A,a,h,H,c,C,s,S,i,I,l,L,n,N,f,d,x,X,@.
 */
/* {{{ proto string pack(string format, mixed arg1 [, mixed arg2 [, mixed ...]])
   Takes one or more arguments and packs them into a binary string according to the format argument */
PHP_FUNCTION(pack)
{
	pval **argv;
	int argc, i;
	int currentarg;
	char *format;
	int formatlen;
	char *formatcodes;
	int *formatargs;
	int formatcount = 0;
	int outputpos = 0, outputsize = 0;
	char *output;
	TLS_VARS;

	argc = ARG_COUNT(ht);

	if (argc < 1) {
		WRONG_PARAM_COUNT;
	}

	argv = emalloc(argc * sizeof(pval *));

	if (getParametersArray(ht, argc, argv) == FAILURE) {
		efree(argv);
		WRONG_PARAM_COUNT;
	}

	convert_to_string(argv[0]);
	format = argv[0]->value.str.val;
	formatlen = argv[0]->value.str.len;

	/* We have a maximum of <formatlen> format codes to deal with */
	formatcodes = emalloc(formatlen * sizeof(*formatcodes));
	formatargs = emalloc(formatlen * sizeof(*formatargs));
	currentarg = 1;

	/* Preprocess format into formatcodes and formatargs */
	for (i = 0; i < formatlen; formatcount++) {
		char code = format[i++];
		int arg = 1;

		/* Handle format arguments if any */
		if (i < formatlen) {
			char c = format[i];

			if (c == '*') {
				arg = -1;
				i++;
			}
			else if ((c >= '0') && (c <= '9')) {
				arg = atoi(&format[i]);
		  
				while (format[i] >= '0' && format[i] <= '9' && i < formatlen) {
					i++;
				}
			}
		}

		/* Handle special arg '*' for all codes and check argv overflows */
		switch ((int)code) {
			/* Never uses any args */
			case 'x': case 'X':	case '@': {
				if (arg < 0) {
					php3_error(E_WARNING, "pack type %c: '*' ignored", code);
					arg = 1;
				}
				break;
			}

			/* Always uses one arg */
			case 'a': case 'A': case 'h': case 'H': {
				if (currentarg >= argc) {
					efree(argv);
					efree(formatcodes);
					efree(formatargs);
					php3_error(E_ERROR, "pack type %c: not enough arguments", code);
					RETURN_FALSE;
				}

				if (arg < 0) {
					arg = argv[currentarg]->value.str.len;
				}

				currentarg++;
				break;
			}

			/* Use as many args as specified */
			case 'c': case 'C': case 's': case 'S': case 'i': case 'I':
			case 'l': case 'L': case 'n': case 'N': case 'v': case 'V':
			case 'f': case 'd': {
				if (arg < 0) {
					arg = argc - currentarg;
				}

				currentarg += arg;

				if (currentarg > argc) {
					efree(argv);
					efree(formatcodes);
					efree(formatargs);
					php3_error(E_ERROR, "pack type %c: too few arguments", code);
					RETURN_FALSE;
				}
				break;
			}

			default: {
				php3_error(E_ERROR, "pack type %c: unknown format code", code);
				RETURN_FALSE;
			}
		}

		formatcodes[formatcount] = code;
		formatargs[formatcount] = arg;
	}

	if (currentarg < argc) {
		php3_error(E_WARNING, "pack %d arguments unused", (argc - currentarg));
	}

	/* Calculate output length and upper bound while processing*/
	for (i = 0; i < formatcount; i++) {
	    int code = (int)formatcodes[i];
		int arg = formatargs[i];

		switch ((int)code) {
			case 'h': case 'H': {
				outputpos += (arg + 1) / 2;		/* 4 bit per arg */
				break;
			}

			case 'a': case 'A':
			case 'c': case 'C':
			case 'x': {
				outputpos += arg;		/* 8 bit per arg */
				break;
			}

			case 's': case 'S': case 'n': case 'v': {
				outputpos += arg * 2;	/* 16 bit per arg */
				break;
			}

			case 'i': case 'I': {
				outputpos += arg * sizeof(int);
				break;
			}

			case 'l': case 'L': case 'N': case 'V': {
				outputpos += arg * 4;	/* 32 bit per arg */
				break;
			}

			case 'f': {
				outputpos += arg * sizeof(float);
				break;
			}

			case 'd': {
				outputpos += arg * sizeof(double);
				break;
			}

			case 'X': {
				outputpos -= arg;

				if (outputpos < 0) {
					php3_error(E_WARNING, "pack type %c: outside of string", code);
					outputpos = 0;
				}
				break;
			}

			case '@': {
				outputpos = arg;
				break;
			}
		}

		if (outputsize < outputpos) {
			outputsize = outputpos;
		}
	}

	output = emalloc(outputsize + 1);
	outputpos = 0;
	currentarg = 1;

	/* Do actual packing */
	for (i = 0; i < formatcount; i++) {
	    int code = (int)formatcodes[i];
		int arg = formatargs[i];
		pval *val;

		switch ((int)code) {
			case 'a': case 'A': {
				memset(&output[outputpos], (code == 'a') ? '\0' : ' ', arg);
				val = argv[currentarg++];
				convert_to_string(val);
				memcpy(&output[outputpos], val->value.str.val,
					   (val->value.str.len < arg) ? val->value.str.len : arg);
				outputpos += arg;
				break;
			}

			case 'h': case 'H': {
				int nibbleshift = (code == 'h') ? 0 : 4;
				int first = 1;
				char *v;

				val = argv[currentarg++];
				convert_to_string(val);
				v = val->value.str.val;
				outputpos--;

				while (arg-- > 0) {
					char n = *(v++);

					if ((n >= '0') && (n <= '9')) {
						n -= '0';
					} else if ((n >= 'A') && (n <= 'F')) {
						n -= ('A' - 10);
					} else if ((n >= 'a') && (n <= 'f')) {
						n -= ('a' - 10);
					} else {
						php3_error(E_WARNING, "pack type %c: illegal hex digit %c", code, n);
						n = 0;
					}

					if (first--) {
						output[++outputpos] = 0;
					} else {
					  first = 1;
					}

					output[outputpos] |= (n << nibbleshift);
					nibbleshift = (nibbleshift + 4) & 7;
				}

				outputpos++;
				break;
			}

			case 'c': case 'C': {
				while (arg-- > 0) {
					_php3_pack(argv[currentarg++], 1, byte_map, &output[outputpos]);
					outputpos++;
				}
				break;
			}

			case 's': case 'S': case 'n': case 'v': {
				int *map = machine_endian_short_map;

				if (code == 'n') {
					map = big_endian_short_map;
				} else if (code == 'v') {
					map = little_endian_short_map;
				}

				while (arg-- > 0) {
					_php3_pack(argv[currentarg++], 2, map, &output[outputpos]);
					outputpos += 2;
				}
				break;
			}

			case 'i': case 'I': {
				while (arg-- > 0) {
					_php3_pack(argv[currentarg++], sizeof(int), int_map, &output[outputpos]);
					outputpos += sizeof(int);
				}
				break;
			}

			case 'l': case 'L': case 'N': case 'V': {
				int *map = machine_endian_long_map;

				if (code == 'N') {
					map = big_endian_long_map;
				} else if (code == 'V') {
					map = little_endian_long_map;
				}

				while (arg-- > 0) {
					_php3_pack(argv[currentarg++], 4, map, &output[outputpos]);
					outputpos += 4;
				}
				break;
			}

			case 'f': {
				float v;

				while (arg-- > 0) {
					val = argv[currentarg++];
					convert_to_double(val);
					v = (float)val->value.dval;
					memcpy(&output[outputpos], &v, sizeof(v));
					outputpos += sizeof(v);
				}
				break;
			}

			case 'd': {
				double v;

				while (arg-- > 0) {
					val = argv[currentarg++];
					convert_to_double(val);
					v = (double)val->value.dval;
					memcpy(&output[outputpos], &v, sizeof(v));
					outputpos += sizeof(v);
				}
				break;
			}

			case 'x': {
				memset(&output[outputpos], '\0', arg);
				outputpos += arg;
				break;
			}

			case 'X': {
				outputpos -= arg;

				if (outputpos < 0) {
					outputpos = 0;
				}
				break;
			}

			case '@': {
				if (arg > outputpos) {
					memset(&output[outputpos], '\0', arg - outputpos);
				}
				outputpos = arg;
				break;
			}
		}
	}

	efree(argv);
	efree(formatcodes);
	efree(formatargs);
	output[outputpos] = '\0';
	RETVAL_STRINGL(output, outputpos, 1);
	efree(output);
}
/* }}} */


static long _php3_unpack(char *data, int size, int issigned, int *map)
{
	long result;
	char *cresult = (char *)&result;
	int i;

	result = issigned ? -1 : 0;

	for (i = 0; i < size; i++) {
		cresult[map[i]] = *(data++);
	}

	return result;
}


/* unpack() is based on Perl's unpack(), but is modified a bit from there.
 * Rather than depending on error-prone ordered lists or syntactically
 * unpleasant pass-by-reference, we return an object with named paramters 
 * (like *_fetch_object()). Syntax is "f[repeat]name/...", where "f" is the
 * formatter char (like pack()), "[repeatt]" is the optional repeater argument,
 * and "name" is the name of the variable to use.
 * Example: "c2chars/nints" will return an object with fields
 * chars1, chars2, and ints.
 * Numeric pack types will return numbers, a and A will return strings,
 * f and d will return doubles.
 * Implemented formats are A,a,h,H,c,C,s,S,i,I,l,L,n,N,f,d,x,X,@.
 */
/* {{{ proto array unpack(string format, string input)
   Unpack binary string into named array elements according to format argument */
PHP_FUNCTION(unpack)
{
	pval *formatarg;
	pval *inputarg;
	char *format;
	char *input;
	int formatlen;
	int inputpos, inputlen;
	int i;
	TLS_VARS;

	if ((ARG_COUNT(ht) != 2) || getParameters(ht, 2, &formatarg, &inputarg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string(formatarg);
	convert_to_string(inputarg);

	format = formatarg->value.str.val;
	formatlen = formatarg->value.str.len;
	input = inputarg->value.str.val;
	inputlen = inputarg->value.str.len;
	inputpos = 0;

	if (array_init(return_value) == FAILURE)
		return;

	while (formatlen-- > 0) {
		char type = *(format++);
		char c;
		int arg = 1;
		char *name;
		int namelen;
		int size=0;

		/* Handle format arguments if any */
		if (formatlen > 0) {
			c = *format;

			if ((c >= '0') && (c <= '9')) {
				arg = atoi(format);

				while ((formatlen > 0) && (*format >= '0') && (*format <= '9')) {
					format++;
					formatlen--;
				}
			} else if (c == '*') {
				arg = -1;
				format++;
				formatlen--;
			}
		}

		/* Get of new value in array */
		name = format;

		while ((formatlen > 0) && (*format != '/')) {
			formatlen--;
			format++;
		}

		namelen = format - name;

		if (namelen > 200)
			namelen = 200;

		switch ((int)type) {
			/* Never use any input */
			case 'X': {
				size = -1;
				break;
			}

			case '@': {
				size = 0;
				break;
			}

			case 'a': case 'A': case 'h': case 'H': {
				size = arg;
				arg = 1;
				break;
			}

			/* Use 1 byte of input */
			case 'c': case 'C': case 'x': {
				size = 1;
				break;
			}

			/* Use 2 bytes of input */
			case 's': case 'S': case 'n': case 'v': {
				size = 2;
				break;
			}

			/* Use sizeof(int) bytes of input */
			case 'i': case 'I': {
				size = sizeof(int);
				break;
			}

			/* Use 4 bytes of input */
			case 'l': case 'L': case 'N': case 'V': {
				size = 4;
				break;
			}

			/* Use sizeof(float) bytes of input */
			case 'f': {
				size = sizeof(float);
				break;
			}

			/* Use sizeof(double) bytes of input */
			case 'd': {
				size = sizeof(double);
				break;
			}
		}

		/* Do actual unpacking */
		for (i = 0; (i != arg); i++ ) {
			/* Space for name + number, safe as namelen is ensured <= 200 */
			char n[256];

			if (arg != 1) {
				/* Need to add element number to name */
				sprintf(n, "%.*s%d", namelen, name, i + 1);
			} else {
				/* Truncate name to next format code or end of string */
				sprintf(n, "%.*s", namelen, name);
			}

			if ((inputpos + size) <= inputlen) {
				switch ((int)type) {
					case 'a': case 'A': {
						char pad = (type == 'a') ? '\0' : ' ';
						int len = inputlen - inputpos;	/* Remaining string */

						/* If size was given take minimum of len and size */
						if ((size >= 0) && (len > size)) {
							len = size;
						}

						size = len;

						/* Remove padding chars from unpacked data */
						while (--len >= 0) {
							if (input[inputpos + len] != pad)
								break;
						}

						add_assoc_stringl(return_value, n, &input[inputpos], len + 1, 1);
						break;
					}
					
					case 'h': case 'H': {
						int len = (inputlen - inputpos) * 2;	/* Remaining */
						int nibbleshift = (type == 'h') ? 0 : 4;
						int first = 1;
						char *buf;
						int ipos, opos;

						/* If size was given take minimum of len and size */
						if ((size >= 0) && (len > size)) {
							len = size;
						}

						size = (len + 1) / 2;
						buf = emalloc(len + 1);

						for (ipos = opos = 0; opos < len; opos++) {
							char c = (input[inputpos + ipos] >> nibbleshift) & 0xf;

							if (c < 10) {
								c += '0';
							} else {
								c += 'a' - 10;
							}

							buf[opos] = c;
							nibbleshift = (nibbleshift + 4) & 7;

							if (first-- == 0) {
								ipos++;
								first = 1;
							}
						}

						buf[len] = '\0';
						add_assoc_stringl(return_value, n, buf, len, 1);
						efree(buf);
						break;
					}

					case 'c': case 'C': {
						int issigned = (type == 'c') ? (input[inputpos] & 0x80) : 0;
						long v = _php3_unpack(&input[inputpos], 1, issigned, byte_map);
						add_assoc_long(return_value, n, v);
						break;
					}

					case 's': case 'S': case 'n': case 'v': {
						long v;
						int issigned = 0;
						int *map = machine_endian_short_map;

						if (type == 's') {
							issigned = input[inputpos + (machine_little_endian ? 1 : 0)] & 0x80;
						} else if (type == 'n') {
							map = big_endian_short_map;
						} else if (type == 'v') {
							map = little_endian_short_map;
						}

						v = _php3_unpack(&input[inputpos], 2, issigned, map);
						add_assoc_long(return_value, n, v);
						break;
					}

					case 'i': case 'I': {
						long v;
						int issigned = 0;

						if (type == 'i') {
							issigned = input[inputpos + (machine_little_endian ? (sizeof(int) - 1) : 0)] & 0x80;
						}

						v = _php3_unpack(&input[inputpos], sizeof(int), issigned, int_map);
						add_assoc_long(return_value, n, v);
						break;
					}

					case 'l': case 'L': case 'N': case 'V': {
						int issigned = 0;
						int *map = machine_endian_long_map;
						long v;

						if (type == 'l') {
							issigned = input[inputpos + (machine_little_endian ? 3 : 0)] & 0x80;
						} else if (type == 'N') {
							map = big_endian_long_map;
						} else if (type == 'V') {
							map = little_endian_long_map;
						}

						v = _php3_unpack(&input[inputpos], 4, issigned, map);
						add_assoc_long(return_value, n, v);
						break;
					}

					case 'f': {
						float v;

						memcpy(&v, &input[inputpos], sizeof(float));
						add_assoc_double(return_value, n, (double)v);
						break;
					}

					case 'd': {
						double v;

						memcpy(&v, &input[inputpos], sizeof(float));
						add_assoc_double(return_value, n, v);
						break;
					}

					case 'x': {
						/* Do nothing with input, just skip it */
						break;
					}

					case 'X': {
						if (inputpos < size) {
							inputpos = -size;
							i = arg - 1;		/* Break out of for loop */

							if (arg >= 0) {
								php3_error(E_WARNING, "pack type %c: outside of string", type);
							}
						}
						break;
					}

					case '@': {
						if (arg <= inputlen) {
							inputpos = arg;
						} else {
							php3_error(E_WARNING, "pack type %c: outside of string", type);
						}

						i = arg - 1;	/* Done, break out of for loop */
						break;
					}
				}

				inputpos += size;
			} else if (arg < 0) {
				/* Reached end of input for '*' repeater */
				break;
			} else {
				php3_error(E_ERROR, "pack type %c: not enough input, need %d, have %d", type, size, inputlen - inputpos);
				RETURN_FALSE;
			}
		}

		formatlen--;	/* Skip '/' separator, does no harm if inputlen == 0 */
		format++;
	}
}
/* }}} */


int php3_minit_pack(INIT_FUNC_ARGS)
{
	int machine_endian_check = 1;
	int i;
	TLS_VARS;

	machine_little_endian = ((char *)&machine_endian_check)[0];

	if (machine_little_endian) {
		/* Where to get lo to hi bytes from */
		byte_map[0] = 0;

		for (i = 0; i < sizeof(int); i++) {
			int_map[i] = i;
		}

		machine_endian_short_map[0] = 0;
		machine_endian_short_map[1] = 1;
		big_endian_short_map[0] = 1;
		big_endian_short_map[1] = 0;
		little_endian_short_map[0] = 0;
		little_endian_short_map[1] = 1;

		machine_endian_long_map[0] = 0;
		machine_endian_long_map[1] = 1;
		machine_endian_long_map[2] = 2;
		machine_endian_long_map[3] = 3;
		big_endian_long_map[0] = 3;
		big_endian_long_map[1] = 2;
		big_endian_long_map[2] = 1;
		big_endian_long_map[3] = 0;
		little_endian_long_map[0] = 0;
		little_endian_long_map[1] = 1;
		little_endian_long_map[2] = 2;
		little_endian_long_map[3] = 3;
	}
	else {
		pval val;
		int size = sizeof(val.value.lval);
		val.value.lval=0; /*silence a warning*/

		/* Where to get hi to lo bytes from */
		byte_map[0] = size - 1;

		for (i = 0; i < sizeof(int); i++) {
			int_map[i] = size - (sizeof(int) - i);
		}

		machine_endian_short_map[0] = size - 2;
		machine_endian_short_map[1] = size - 1;
		big_endian_short_map[0] = size - 2;
		big_endian_short_map[1] = size - 1;
		little_endian_short_map[0] = size - 1;
		little_endian_short_map[1] = size - 2;

		machine_endian_long_map[0] = size - 4;
		machine_endian_long_map[1] = size - 3;
		machine_endian_long_map[2] = size - 2;
		machine_endian_long_map[3] = size - 1;
		big_endian_long_map[0] = size - 4;
		big_endian_long_map[1] = size - 3;
		big_endian_long_map[2] = size - 2;
		big_endian_long_map[3] = size - 1;
		little_endian_long_map[0] = size - 1;
		little_endian_long_map[1] = size - 2;
		little_endian_long_map[2] = size - 3;
		little_endian_long_map[3] = size - 4;
	}

	return SUCCESS;
}



/*
 * Local variables:
 * tab-width: 4
 * End:
 */
