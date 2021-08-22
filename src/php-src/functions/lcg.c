/* 
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Sascha Schumann <sascha@schumann.cx>                        |
   +----------------------------------------------------------------------+
 */

#include "php.h"
#include "php_lcg.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef ZTS
int lcg_globals_id;
#else
static php_lcg_globals lcg_globals;
#endif

/*
 * combinedLCG() returns a pseudo random number in the range of (0,1).
 * The function combines two CGs with periods of 
 * 2^31 - 85 and 2^31 - 249. The period of this function
 * is equal to the product of both primes.
 */

#define MODMULT(a,b,c,m,s) q = s/a;s=b*(s-a*q)-c*q;if(s<0)s+=m

double php_combined_lcg(void)
{
	long q;
	long z;
	LCGLS_FETCH();

	MODMULT(53668,40014,12211,2147483563L, LCG(s1));
	MODMULT(52774,40692,3791, 2147483399L, LCG(s2));

	z = LCG(s1) - LCG(s2);
	if(z < 1) {
		z += 2147483562;
	}

	return z * 4.656613e-10;
}

static void init_globals(LCGLS_D)
{
	LCG(s1) = 1;
	LCG(s2) = (long) getpid();
}

static int php_minit_lcg(INIT_FUNC_ARGS)
{
#ifdef ZTS
	lcg_globals_id = ts_allocate_id(sizeof(*lcg_globals), init_globals, NULL);
#else
	init_globals();
#endif
	return SUCCESS;
}

zend_module_entry lcg_module_entry = {
	"LCG",
	NULL,
	php_minit_lcg, NULL,
	NULL, NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES
};
