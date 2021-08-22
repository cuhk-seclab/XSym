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

/* $Id: formatted_print.c,v 1.55 2000/07/21 02:58:17 coar Exp $ */

#include "php.h"
#include "internal_functions.h"
#include "head.h"
#include "php3_string.h"
#include <stdio.h>
#include <math.h>

#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1
#define ADJ_WIDTH 1
#define ADJ_PRECISION 2
#define NUM_BUF_SIZE 500
#define	NDIG 80
#define FLOAT_DIGITS 6
#define FLOAT_PRECISION 6
#define MAX_FLOAT_DIGITS 38
#define MAX_FLOAT_PRECISION 40

#if 0
/* trick to control varargs functions through cpp */
# define PRINTF_DEBUG(arg) printf arg
#else
# define PRINTF_DEBUG(arg)
#endif

static char hexchars[] = "0123456789abcdef";
static char HEXCHARS[] = "0123456789ABCDEF";


/*
 * cvt.c - IEEE floating point formatting routines for FreeBSD
 * from GNU libc-4.6.27
 */

/*
 *    _php3_cvt converts to decimal
 *      the number of digits is specified by ndigit
 *      decpt is set to the position of the decimal point
 *      sign is set to 0 for positive, 1 for negative
 */
static char *
_php3_cvt(double arg, int ndigits, int *decpt, int *sign, int eflag)
{
	register int r2;
	double fi, fj;
	register char *p, *p1;
	static char cvt_buf[NDIG];
	TLS_VARS;

	if (ndigits >= NDIG - 1)
		ndigits = NDIG - 2;
	r2 = 0;
	*sign = 0;
	p = &STATIC(cvt_buf)[0];
	if (arg < 0) {
		*sign = 1;
		arg = -arg;
	}
	arg = modf(arg, &fi);
	p1 = &STATIC(cvt_buf)[NDIG];
	/*
	 * Do integer part
	 */
	if (fi != 0) {
		p1 = &STATIC(cvt_buf)[NDIG];
		while (fi != 0) {
			fj = modf(fi / 10, &fi);
			*--p1 = (int) ((fj + .03) * 10) + '0';
			r2++;
		}
		while (p1 < &STATIC(cvt_buf)[NDIG])
			*p++ = *p1++;
	} else if (arg > 0) {
		while ((fj = arg * 10) < 1) {
			arg = fj;
			r2--;
		}
	}
	p1 = &STATIC(cvt_buf)[ndigits];
	if (eflag == 0)
		p1 += r2;
	*decpt = r2;
	if (p1 < &STATIC(cvt_buf)[0]) {
		STATIC(cvt_buf)[0] = '\0';
		return (STATIC(cvt_buf));
	}
	while (p <= p1 && p < &STATIC(cvt_buf)[NDIG]) {
		arg *= 10;
		arg = modf(arg, &fj);
		*p++ = (int) fj + '0';
	}
	if (p1 >= &STATIC(cvt_buf)[NDIG]) {
		STATIC(cvt_buf)[NDIG - 1] = '\0';
		return (STATIC(cvt_buf));
	}
	p = p1;
	*p1 += 5;
	while (*p1 > '9') {
		*p1 = '0';
		if (p1 > STATIC(cvt_buf))
			++ * --p1;
		else {
			*p1 = '1';
			(*decpt)++;
			if (eflag == 0) {
				if (p > STATIC(cvt_buf))
					*p = '0';
				p++;
			}
		}
	}
	*p = '\0';
	return (STATIC(cvt_buf));
}


inline static void
_php3_sprintf_appendchar(char **buffer, int *pos, int *size, char add)
{
	if ((*pos + 1) >= *size) {
		*size <<= 1;
		PRINTF_DEBUG(("%s: ereallocing buffer to %d bytes\n", GLOBAL(function_state.function_name), *size));
		*buffer = erealloc(*buffer, *size);
	}
	PRINTF_DEBUG(("sprintf: appending '%c', pos=\n", add, *pos));
	(*buffer)[(*pos)++] = add;
}


inline static void
_php3_sprintf_appendstring(char **buffer, int *pos, int *size, char *add,
						   int min_width, int max_width, char padding,
						   int alignment, int len, int sign, int expprec)
{
	register int npad;

	npad = min_width - MIN(len, (expprec ? max_width : len));

	if (npad < 0) {
		npad = 0;
	}

	PRINTF_DEBUG(("sprintf: appendstring(%x, %d, %d, \"%s\", %d, '%c', %d)\n",
				  *buffer, *pos, *size, add, min_width, padding, alignment));
	if ((max_width == 0) && (! expprec)) {
		max_width = MAX(min_width, len);
	}
	if ((*pos + max_width) >= *size) {
		while ((*pos + max_width) >= *size) {
			*size <<= 1;
		}
		PRINTF_DEBUG(("sprintf ereallocing buffer to %d bytes\n", *size));
		*buffer = erealloc(*buffer, *size);
	}
	if (alignment == ALIGN_RIGHT) {
		if (sign && padding == '0') {
			(*buffer)[(*pos)++] = '-';
			add++;
		}
		while (npad-- > 0) {
			(*buffer)[(*pos)++] = padding;
		}
	}
	PRINTF_DEBUG(("sprintf: appending \"%s\"\n", add));
	strncpy(&(*buffer)[*pos], add, max_width);
	*pos += MIN(max_width, len);
	if (alignment == ALIGN_LEFT) {
		while (npad--) {
			(*buffer)[(*pos)++] = padding;
		}
	}
}


inline static void
_php3_sprintf_appendint(char **buffer, int *pos, int *size, int number,
						int width, char padding, int alignment)
{
	char numbuf[NUM_BUF_SIZE];
	register unsigned int magn, nmagn, i = NUM_BUF_SIZE - 1, neg = 0;

	PRINTF_DEBUG(("sprintf: appendint(%x, %x, %x, %d, %d, '%c', %d)\n",
				  *buffer, pos, size, number, width, padding, alignment));
	if (number < 0) {
		neg = 1;
		magn = ((unsigned int) -(number + 1)) + 1;
	} else {
		magn = (unsigned int) number;
	}

	/* Can't right-pad 0's on integers */
	if(alignment==0 && padding=='0') padding=' ';

	numbuf[i] = '\0';

	do {
		nmagn = magn / 10;

		numbuf[--i] = (magn - (nmagn * 10)) + '0';
		magn = nmagn;
	}
	while (magn > 0 && i > 0);
	if (neg) {
		numbuf[--i] = '-';
	}
	PRINTF_DEBUG(("sprintf: appending %d as \"%s\", i=%d\n",
				  number, &numbuf[i], i));
	_php3_sprintf_appendstring(buffer, pos, size, &numbuf[i], width, 0,
							   padding, alignment, (NUM_BUF_SIZE - 1) - i,
							   neg, 0);
}


inline static void
_php3_sprintf_appenddouble(char **buffer, int *pos,
						   int *size, double number,
						   int width, char padding,
						   int alignment, int precision,
						   int adjust, char fmt)
{
	char numbuf[NUM_BUF_SIZE];
	char *cvt;
	register int i = 0, j = 0;
	int sign, decpt;

	PRINTF_DEBUG(("sprintf: appenddouble(%x, %x, %x, %f, %d, '%c', %d, %c)\n",
				  *buffer, pos, size, number, width, padding, alignment, fmt));
	if ((adjust & ADJ_PRECISION) == 0) {
		precision = FLOAT_PRECISION;
	} else if (precision > MAX_FLOAT_PRECISION) {
		precision = MAX_FLOAT_PRECISION;
	}
	cvt = _php3_cvt(number, precision, &decpt, &sign, (fmt == 'e'));

	if (sign) {
		numbuf[i++] = '-';
	}

	if (fmt == 'f') {
		if (decpt <= 0) {
			numbuf[i++] = '0';
			if (precision > 0) {
				int k = precision;
				numbuf[i++] = '.';
				while ((decpt++ < 0) && k--) {
					numbuf[i++] = '0';
				}
			}
		} else {
			while (decpt-- > 0)
				numbuf[i++] = cvt[j++];
			if (precision > 0)
				numbuf[i++] = '.';
		}
	} else {
		numbuf[i++] = cvt[j++];
		if (precision > 0)
			numbuf[i++] = '.';
	}

	while (cvt[j]) {
		numbuf[i++] = cvt[j++];
	}

	numbuf[i] = '\0';

	if (precision > 0) {
		width += (precision + 1);
	}
	_php3_sprintf_appendstring(buffer, pos, size, numbuf, width, 0, padding,
							   alignment, i, sign, 0);
}


inline static void
_php3_sprintf_append2n(char **buffer, int *pos, int *size, int number,
					   int width, char padding, int alignment, int n,
					   char *chartable, int expprec)
{
	char numbuf[NUM_BUF_SIZE];
	register unsigned int num, i = NUM_BUF_SIZE - 1, neg = 0;
	register int andbits = (1 << n) - 1;

	PRINTF_DEBUG(("sprintf: append2n(%x, %x, %x, %d, %d, '%c', %d, %d, %x)\n",
				  *buffer, pos, size, number, width, padding, alignment, n,
				  chartable));
	PRINTF_DEBUG(("sprintf: append2n 2^%d andbits=%x\n", n, andbits));

	if (number < 0) {
		neg = 1;
		num = ((unsigned int) -(number + 1)) + 1;
	} else {
		num = (unsigned int) number;
	}

	numbuf[i] = '\0';

	do {
		numbuf[--i] = chartable[(num & andbits)];
		num >>= n;
	}
	while (num > 0);

	if (neg) {
		numbuf[--i] = '-';
	}
	_php3_sprintf_appendstring(buffer, pos, size, &numbuf[i], width, 0,
							   padding, alignment, (NUM_BUF_SIZE - 1) - i,
							   neg, expprec);
}


inline static int
_php3_sprintf_getnumber(char *buffer, int *pos)
{
	char *endptr;
	register int num = strtol(&buffer[*pos], &endptr, 10);
	register int i = 0;

	if (endptr != NULL) {
		i = (endptr - &buffer[*pos]);
	}
	PRINTF_DEBUG(("sprintf_getnumber: number was %d bytes long\n", i));
	*pos += i;
	return num;
}


/*
 * New sprintf implementation for PHP.
 *
 * Modifiers:
 *
 *  " "   pad integers with spaces
 *  "-"   left adjusted field
 *   n    field size
 *  "."n  precision (floats only)
 *
 * Type specifiers:
 *
 *  "%"   literal "%", modifiers are ignored.
 *  "b"   integer argument is printed as binary
 *  "c"   integer argument is printed as a single character
 *  "d"   argument is an integer
 *  "f"   the argument is a float
 *  "o"   integer argument is printed as octal
 *  "s"   argument is a string
 *  "x"   integer argument is printed as lowercase hexadecimal
 *  "X"   integer argument is printed as uppercase hexadecimal
 *
 */
static char *
php3_formatted_print(HashTable *ht, int *len)
{
	pval **args;
	int argc, size = 240, inpos = 0, outpos = 0;
	int alignment, width, precision, currarg, adjusting;
	char *format, *result, padding;

	argc = ARG_COUNT(ht);

	if (argc < 1) {
		WRONG_PARAM_COUNT_WITH_RETVAL(NULL);
	}
	args = emalloc(argc * sizeof(pval *));

	if (getParametersArray(ht, argc, args) == FAILURE) {
		efree(args);
		WRONG_PARAM_COUNT_WITH_RETVAL(NULL);
	}
	convert_to_string(args[0]);
	format = args[0]->value.str.val;
	result = emalloc(size);

	currarg = 1;

	while (format[inpos]) {
		int expprec = 0;

		PRINTF_DEBUG(("sprintf: format[%d]='%c'\n", inpos, format[inpos]));
		PRINTF_DEBUG(("sprintf: outpos=%d\n", outpos));
		if (format[inpos] != '%') {
			_php3_sprintf_appendchar(&result, &outpos, &size, format[inpos++]);
		} else if (format[inpos + 1] == '%') {
			_php3_sprintf_appendchar(&result, &outpos, &size, '%');
			inpos += 2;
		} else {
			if (currarg >= argc && format[inpos + 1] != '%') {
				efree(result);
				efree(args);
				php3_error(E_WARNING, "%s(): too few arguments",GLOBAL(function_state.function_name));
				return NULL;
			}
			/* starting a new format specifier, reset variables */
			alignment = ALIGN_RIGHT;
			adjusting = 0;
			padding = ' ';
			inpos++;			/* skip the '%' */

			PRINTF_DEBUG(("sprintf: first looking at '%c', inpos=%d\n",
						  format[inpos], inpos));
			if (isascii((int)format[inpos]) && !isalpha((int)format[inpos])) {
				/* first look for modifiers */
				PRINTF_DEBUG(("sprintf: looking for modifiers\n"
							  "sprintf: now looking at '%c', inpos=%d\n",
							  format[inpos], inpos));
				for (;; inpos++) {
					if (format[inpos] == ' ' || format[inpos] == '0') {
						padding = format[inpos];
					} else if (format[inpos] == '-') {
						alignment = ALIGN_LEFT;
						/* space padding, the default */
					} else if (format[inpos] == '\'') {
						padding = format[++inpos];
					} else {
						PRINTF_DEBUG(("sprintf: end of modifiers\n"));
						break;
					}
				}
				PRINTF_DEBUG(("sprintf: padding='%c'\n", padding));
				PRINTF_DEBUG(("sprintf: alignment=%s\n",
							  (alignment == ALIGN_LEFT) ? "left" : "right"));


				/* after modifiers comes width */
				if (isdigit((int)format[inpos])) {
					PRINTF_DEBUG(("sprintf: getting width\n"));
					width = _php3_sprintf_getnumber(format, &inpos);
					adjusting |= ADJ_WIDTH;
				} else {
					width = 0;
				}
				PRINTF_DEBUG(("sprintf: width=%d\n", width));

				/* after width comes precision */
				if (format[inpos] == '.') {
					inpos++;
					PRINTF_DEBUG(("sprintf: getting precision\n"));
					if (isdigit((int)format[inpos])) {
						precision = _php3_sprintf_getnumber(format, &inpos);
						adjusting |= ADJ_PRECISION;
						expprec = 1;
					} else {
						precision = 0;
					}
				} else {
					precision = 0;
				}
				PRINTF_DEBUG(("sprintf: precision=%d\n", precision));
			} else {
				width = precision = 0;
			}

			if (format[inpos] == 'l') {
				inpos++;
			}
			PRINTF_DEBUG(("sprintf: format character='%c'\n", format[inpos]));
			/* now we expect to find a type specifier */
			switch (format[inpos]) {
				case 's':
					convert_to_string(args[currarg]);
					_php3_sprintf_appendstring(&result, &outpos, &size,
											   args[currarg]->value.str.val,
											   width, precision, padding,
											   alignment,
											   args[currarg]->value.str.len, 0,
											   expprec);
					break;

				case 'd':
					convert_to_long(args[currarg]);
					_php3_sprintf_appendint(&result, &outpos, &size,
											args[currarg]->value.lval,
											width, padding, alignment);
					break;

				case 'e':
				case 'f':
					/* XXX not done */
					convert_to_double(args[currarg]);
					_php3_sprintf_appenddouble(&result, &outpos, &size,
											   args[currarg]->value.dval,
											   width, padding, alignment,
											   precision, adjusting,
											   format[inpos]);
					break;

				case 'c':
					convert_to_long(args[currarg]);
					_php3_sprintf_appendchar(&result, &outpos, &size,
									   (char) args[currarg]->value.lval);
					break;

				case 'o':
					convert_to_long(args[currarg]);
					_php3_sprintf_append2n(&result, &outpos, &size,
										   args[currarg]->value.lval,
										   width, padding, alignment, 3,
										   hexchars, expprec);
					break;

				case 'x':
					convert_to_long(args[currarg]);
					_php3_sprintf_append2n(&result, &outpos, &size,
										   args[currarg]->value.lval,
										   width, padding, alignment, 4,
										   hexchars, expprec);
					break;

				case 'X':
					convert_to_long(args[currarg]);
					_php3_sprintf_append2n(&result, &outpos, &size,
										   args[currarg]->value.lval,
										   width, padding, alignment, 4,
										   HEXCHARS, expprec);
					break;

				case 'b':
					convert_to_long(args[currarg]);
					_php3_sprintf_append2n(&result, &outpos, &size,
										   args[currarg]->value.lval,
										   width, padding, alignment, 1,
										   hexchars, expprec);
					break;

				case '%':
					_php3_sprintf_appendchar(&result, &outpos, &size, '%');

					break;
				default:
					break;
			}
			currarg++;
			inpos++;
		}
	}
	
	efree(args);
	
	/* possibly, we have to make sure we have room for the terminating null? */
	result[outpos]=0;
	*len = outpos;	
	return result;
}


/* {{{ proto string sprintf(string format [, mixed arg1 [, ...]])
   Return a formatted string */
PHP_FUNCTION(user_sprintf)
{
	char *result;
	int len;
	TLS_VARS;
	
	if ((result=php3_formatted_print(ht,&len))==NULL) {
		RETURN_FALSE;
	}
	RETVAL_STRINGL(result,len,1);
	efree(result);
}
/* }}} */

/* {{{ proto int printf(string format [, mixed arg1 [, ...]])
   Output a formatted string */
PHP_FUNCTION(user_printf)
{
	char *result;
	int len;
	TLS_VARS;
	
	if ((result=php3_formatted_print(ht,&len))==NULL) {
		RETURN_FALSE;
	}
	if (php3_header())
		PHPWRITE(result,len);
	efree(result);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
