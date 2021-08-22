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
   | Authors: Jim Winstead (jimw@php.net)                                 |
   +----------------------------------------------------------------------+
 */
/* $Id: url.h,v 1.16 2000/01/01 04:44:10 sas Exp $ */

typedef struct url {
	char *scheme;
	char *user;
	char *pass;
	char *host;
	unsigned short port;
	char *path;
	char *query;
	char *fragment;
} url;

extern void free_url(url *);
extern url *url_parse(char *);
extern int _php3_urldecode(char *, int); /* return value: length of decoded string */
extern char *_php3_urlencode(char *, int);
extern int _php3_rawurldecode(char *, int); /* return value: length of decoded string */
extern char *_php3_rawurlencode(char *, int);

extern void php3_parse_url(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_urlencode(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_urldecode(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_rawurlencode(INTERNAL_FUNCTION_PARAMETERS);
extern void php3_rawurldecode(INTERNAL_FUNCTION_PARAMETERS);

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
