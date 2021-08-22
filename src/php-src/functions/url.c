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
   | Author: Jim Winstead (jimw@php.net)                                  |
   +----------------------------------------------------------------------+
 */
/* $Id: url.c,v 1.46 2000/08/04 12:28:41 martin Exp $ */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

/* php.h includes the correct regex.h */
#include "php.h"
#include "internal_functions.h"

#include "url.h"
#ifdef _OSD_POSIX
#ifndef APACHE
#error On this EBCDIC platform, PHP3 is only supported as an Apache module.
#else /*APACHE*/
#ifndef CHARSET_EBCDIC
#define CHARSET_EBCDIC /* this machine uses EBCDIC, not ASCII! */
#endif
#include "ebcdic.h"
#endif /*APACHE*/
#endif /*_OSD_POSIX*/


void free_url(url * theurl)
{
	if (theurl->scheme)
		efree(theurl->scheme);
	if (theurl->user)
		efree(theurl->user);
	if (theurl->pass)
		efree(theurl->pass);
	if (theurl->host)
		efree(theurl->host);
	if (theurl->path)
		efree(theurl->path);
	if (theurl->query)
		efree(theurl->query);
	if (theurl->fragment)
		efree(theurl->fragment);
	efree(theurl);
}

url *url_parse(char *string)
{
	regex_t re;
	regmatch_t subs[10];
	int err;
	int length = strlen(string);
	char *result;

	url *ret = (url *) emalloc(sizeof(url));
	if (!ret) {
		/*php3_error(E_WARNING,"Unable to allocate memory\n");*/
		return NULL;
	}
	memset(ret, 0, sizeof(url));

	/* from Appendix B of draft-fielding-url-syntax-09,
	   http://www.ics.uci.edu/~fielding/url/url.txt */
	err = regcomp(&re, "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?", REG_EXTENDED);
	if (err) {
		/*php3_error(E_WARNING,"Unable to compile regex: %d\n", err);*/
		efree(ret);
		return NULL;
	}
	err = regexec(&re, string, 10, subs, 0);
	if (err) {
		/*php3_error(E_WARNING,"Error with regex\n");*/
		efree(ret);
		return NULL;
	}
	/* no processing necessary on the scheme */
	if (subs[2].rm_so != -1 && subs[2].rm_so < length) {
		ret->scheme = estrndup(string + subs[2].rm_so, subs[2].rm_eo - subs[2].rm_so);
	}

	/* the path to the resource */
	if (subs[5].rm_so != -1 && subs[5].rm_so < length) {
		ret->path = estrndup(string + subs[5].rm_so, subs[5].rm_eo - subs[5].rm_so);
	}

	/* the query part */
	if (subs[7].rm_so != -1 && subs[7].rm_so < length) {
		ret->query = estrndup(string + subs[7].rm_so, subs[7].rm_eo - subs[7].rm_so);
	}

	/* the fragment */
	if (subs[9].rm_so != -1 && subs[9].rm_so < length) {
		ret->fragment = estrndup(string + subs[9].rm_so, subs[9].rm_eo - subs[9].rm_so);
	}

	/* extract the username, pass, and port from the hostname */
	if (subs[4].rm_so != -1 && subs[4].rm_so < length) {
		/* extract username:pass@host:port from regex results */
		result = estrndup(string + subs[4].rm_so, subs[4].rm_eo - subs[4].rm_so);
		length = strlen(result);

		regfree(&re);			/* free the old regex */
		
		if ((err=regcomp(&re, "^(([^@:]+)(:([^@:]+))?@)?([^:@]+)(:([^:@]+))?", REG_EXTENDED))
			|| (err=regexec(&re, result, 10, subs, 0))) {
			STR_FREE(ret->scheme);
			STR_FREE(ret->path);
			STR_FREE(ret->query);
			STR_FREE(ret->fragment);
			efree(ret);
			efree(result);
			/*php3_error(E_WARNING,"Unable to compile regex: %d\n", err);*/
			return NULL;
		}
		/* now deal with all of the results */
		if (subs[2].rm_so != -1 && subs[2].rm_so < length) {
			ret->user = estrndup(result + subs[2].rm_so, subs[2].rm_eo - subs[2].rm_so);
		}
		if (subs[4].rm_so != -1 && subs[4].rm_so < length) {
			ret->pass = estrndup(result + subs[4].rm_so, subs[4].rm_eo - subs[4].rm_so);
		}
		if (subs[5].rm_so != -1 && subs[5].rm_so < length) {
			ret->host = estrndup(result + subs[5].rm_so, subs[5].rm_eo - subs[5].rm_so);
		}
		if (subs[7].rm_so != -1 && subs[7].rm_so < length) {
			ret->port = (unsigned short) strtol(result + subs[7].rm_so, NULL, 10);
		}
		efree(result);
	}
	regfree(&re);
	return ret;
}

/* {{{ proto array parse_url(string url)
   Parse a URL and return its components */
void php3_parse_url(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *string;
	url *resource;
	TLS_VARS;

	if ( getParameters(ht, 1, &string) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(string);

	resource = url_parse(string->value.str.val);

	if (resource == NULL) {
		php3_error(E_WARNING, "unable to parse url (%s)", string->value.str.val);
		RETURN_FALSE;
	}
	/* allocate an array for return */
	if (array_init(return_value) == FAILURE) {
		free_url(resource);
		RETURN_FALSE;
	}
	/* add the various elements to the array */
	if (resource->scheme != NULL)
		add_assoc_string(return_value, "scheme", resource->scheme, 1);
	if (resource->host != NULL)
		add_assoc_string(return_value, "host", resource->host, 1);
	if (resource->port != 0)
		add_assoc_long(return_value, "port", resource->port);
	if (resource->user != NULL)
		add_assoc_string(return_value, "user", resource->user, 1);
	if (resource->pass != NULL)
		add_assoc_string(return_value, "pass", resource->pass, 1);
	if (resource->path != NULL)
		add_assoc_string(return_value, "path", resource->path, 1);
	if (resource->query != NULL)
		add_assoc_string(return_value, "query", resource->query, 1);
	if (resource->fragment != NULL)
		add_assoc_string(return_value, "fragment", resource->fragment, 1);
	free_url(resource);
}
/* }}} */

static int php3_htoi(char *s)
{
	int value;
	int c;

	c = s[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = s[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

/* rfc1738:

   ...The characters ";",
   "/", "?", ":", "@", "=" and "&" are the characters which may be
   reserved for special meaning within a scheme...

   ...Thus, only alphanumerics, the special characters "$-_.+!*'(),", and
   reserved characters used for their reserved purposes may be used
   unencoded within a URL...

   For added safety, we only leave -_. unencoded.
 */

static unsigned char hexchars[] = "0123456789ABCDEF";

char *_php3_urlencode(char *s, int len)
{
	register int x, y;
	unsigned char *str;

	str = (unsigned char *) emalloc(3 * strlen(s) + 1);
	for (x = 0, y = 0; len--; x++, y++) {
		str[y] = (unsigned char) s[x];
		if (str[y] == ' ') {
			str[y] = '+';
#ifndef CHARSET_EBCDIC
		} else if ((str[y] < '0' && str[y] != '-' && str[y] != '.') ||
				   (str[y] < 'A' && str[y] > '9') ||
				   (str[y] > 'Z' && str[y] < 'a' && str[y] != '_') ||
				   (str[y] > 'z')) {
			str[y++] = '%';
			str[y++] = hexchars[(unsigned char) s[x] >> 4];
			str[y] = hexchars[(unsigned char) s[x] & 15];
		}
#else /*CHARSET_EBCDIC*/
		} else if (!isalnum(str[y]) && strchr("_-.", str[y]) == NULL) {
			/* Allow only alphanumeric chars and '_', '-', '.'; escape the rest */
			str[y++] = '%';
			str[y++] = hexchars[os_toascii[(unsigned char) s[x]] >> 4];
			str[y] = hexchars[os_toascii[(unsigned char) s[x]] & 0x0F];
		}
#endif /*CHARSET_EBCDIC*/
	}
	str[y] = '\0';
	return ((char *) str);
}

/* {{{ proto string urlencode(string str)
   URL-encodes string */
void php3_urlencode(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg;
	char *str;
	TLS_VARS;

	if ( getParameters(ht, 1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);

	if (!arg->value.str.len) {
		RETURN_FALSE;
	}
	str = _php3_urlencode(arg->value.str.val, arg->value.str.len);
	RETVAL_STRING(str, 1);
	efree(str);
}
/* }}} */

/* {{{ proto string urldecode(string str)
   Decodes URL-encoded string */
void php3_urldecode(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg;
	int len;
	TLS_VARS;

	if ( getParameters(ht, 1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);

	if (!arg->value.str.len) {
		RETURN_FALSE;
	}
	len = _php3_urldecode(arg->value.str.val, arg->value.str.len);

	RETVAL_STRINGL(arg->value.str.val, len, 1);
}
/* }}} */

int _php3_urldecode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '+')
			*dest = ' ';
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
#ifndef CHARSET_EBCDIC
			*dest = (char) php3_htoi(data + 1);
#else
			*dest = os_toebcdic[(char) php3_htoi(data + 1)];
#endif
			data += 2;
			len -= 2;
		} else
			*dest = *data;
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

char *_php3_rawurlencode(char *s, int len)
{
	register int x, y;
	unsigned char *str;

	str = (unsigned char *) emalloc(3 * len + 1);
	for (x = 0, y = 0; len--; x++, y++) {
		str[y] = (unsigned char) s[x];
#ifndef CHARSET_EBCDIC
		if ((str[y] < '0' && str[y] != '-' && str[y] != '.') ||
			(str[y] < 'A' && str[y] > '9') ||
			(str[y] > 'Z' && str[y] < 'a' && str[y] != '_') ||
			(str[y] > 'z')) {
			str[y++] = '%';
			str[y++] = hexchars[(unsigned char) s[x] >> 4];
			str[y] = hexchars[(unsigned char) s[x] & 15];
#else /*CHARSET_EBCDIC*/
		if (!isalnum(str[y]) && strchr("_-.", str[y]) != NULL) {
			str[y++] = '%';
			str[y++] = hexchars[os_toascii[(unsigned char) s[x]] >> 4];
			str[y] = hexchars[os_toascii[(unsigned char) s[x]] & 15];
#endif /*CHARSET_EBCDIC*/
		}
	}
	str[y] = '\0';
	return ((char *) str);
}

/* {{{ proto string rawurlencode(string str)
   URL-encodes string */
void php3_rawurlencode(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg;
	char *str;
	TLS_VARS;

	if ( getParameters(ht, 1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);

	if (!arg->value.str.len) {
		RETURN_FALSE;
	}
	str = _php3_rawurlencode(arg->value.str.val, arg->value.str.len);
	RETVAL_STRING(str, 1);
	efree(str);
}
/* }}} */

/* {{{ proto string rawurldecode(string str)
   Decodes URL-encodes string */
void php3_rawurldecode(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *arg;
	int len;
	TLS_VARS;

	if (getParameters(ht, 1, &arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_string(arg);

	if (!arg->value.str.len) {
		RETURN_FALSE;
	}
	len = _php3_rawurldecode(arg->value.str.val, arg->value.str.len);

	RETVAL_STRINGL(arg->value.str.val, len, 1);
}
/* }}} */

int _php3_rawurldecode(char *str, int len)
{
	char *dest = str;
	char *data = str;

	while (len--) {
		if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) {
#ifndef CHARSET_EBCDIC
			*dest = (char) php3_htoi(data + 1);
#else
			*dest = os_toebcdic[(char) php3_htoi(data + 1)];
#endif
			data += 2;
			len -= 2;
		} else
			*dest = *data;
		data++;
		dest++;
	}
	*dest = '\0';
	return dest - str;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
