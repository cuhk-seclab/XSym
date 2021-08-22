%{
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
   | Authors: Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */



/* $Id: configuration-parser.y,v 1.81 2000/02/07 23:54:49 zeev Exp $ */

#define DEBUG_CFG_PARSER 1
#include "php.h"
#include "modules.h"
#include "functions/dl.h"
#include "functions/file.h"
#include "functions/php3_browscap.h"
#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include "win32/wfile.h"
#endif

#define PARSING_MODE_CFG 0
#define PARSING_MODE_BROWSCAP 1

static HashTable configuration_hash;
extern HashTable browser_hash;
extern char *php3_ini_path;
static HashTable *active__php3_hash_table;
static pval *current_section;
static char *currently_parsed_filename;

static int parsing_mode;

pval yylval;

extern int cfglex(pval *cfglval);
extern FILE *cfgin;
extern int cfglineno;
extern void init_cfg_scanner(void);

PHPAPI int cfg_get_long(char *varname,long *result)
{
	pval *tmp,var;
	
	if (_php3_hash_find(&configuration_hash,varname,strlen(varname)+1,(void **) &tmp)==FAILURE) {
		*result=(long)NULL;
		return FAILURE;
	}
	var = *tmp;
	pval_copy_constructor(&var);
	convert_to_long(&var);
	*result = var.value.lval;
	return SUCCESS;
}


PHPAPI int cfg_get_double(char *varname,double *result)
{
	pval *tmp,var;
	
	if (_php3_hash_find(&configuration_hash,varname,strlen(varname)+1,(void **) &tmp)==FAILURE) {
		*result=(double)0;
		return FAILURE;
	}
	var = *tmp;
	pval_copy_constructor(&var);
	convert_to_double(&var);
	*result = var.value.dval;
	return SUCCESS;
}


PHPAPI int cfg_get_string(char *varname, char **result)
{
	pval *tmp;

	if (_php3_hash_find(&configuration_hash,varname,strlen(varname)+1,(void **) &tmp)==FAILURE) {
		*result=NULL;
		return FAILURE;
	}
	*result = tmp->value.str.val;
	return SUCCESS;
}


static void yyerror(char *str)
{
	fprintf(stderr,"PHP:  Error parsing %s on line %d\n",currently_parsed_filename,cfglineno);
}


static void pvalue_config_destructor(pval *pvalue)
{
	if (pvalue->type == IS_STRING && pvalue->value.str.val != empty_string) {
		free(pvalue->value.str.val);
	}
}


static void pvalue_browscap_destructor(pval *pvalue)
{
	if (pvalue->type == IS_OBJECT || pvalue->type == IS_ARRAY) {
		_php3_hash_destroy(pvalue->value.ht);
		free(pvalue->value.ht);
	}
}


int php3_init_config(void)
{
	TLS_VARS;

	if (_php3_hash_init(&configuration_hash, 0, NULL, (void (*)(void *))pvalue_config_destructor, 1)==FAILURE) {
		return FAILURE;
	}

#if USE_CONFIG_FILE
	{
		char *env_location,*default_location,*php_ini_path;
		int safe_mode_state = php3_ini.safe_mode;
		char *opened_path;
		
		env_location = getenv("PHPRC");
		if (!env_location) {
			env_location="";
		}
#if WIN32|WINNT
		{
			if (GLOBAL(php3_ini_path)) {
				default_location = GLOBAL(php3_ini_path);
			} else {
				default_location = (char *) malloc(512);
			
				if (!GetWindowsDirectory(default_location,255)) {
					default_location[0]=0;
				}
			}
		}
#elif OS2
        if (GLOBAL(php3_ini_path)) {
            default_location = GLOBAL(php3_ini_path);
        } else {
            default_location = getenv("ETC");

            if (!default_location) {
                default_location = CONFIGURATION_FILE_PATH;
            }
        }
#else
		if (!GLOBAL(php3_ini_path)) {
			default_location = CONFIGURATION_FILE_PATH;
		} else {
			default_location = GLOBAL(php3_ini_path);
		}
#endif

/* build a path */
		php_ini_path = (char *) malloc(sizeof(".")+strlen(env_location)+strlen(default_location)+2+1);

		if (!GLOBAL(php3_ini_path)) {
#if WIN32|WINNT|OS2
			sprintf(php_ini_path,".;%s;%s",env_location,default_location);
#else
			sprintf(php_ini_path,".:%s:%s",env_location,default_location);
#endif
		} else {
			/* if path was set via -c flag, only look there */
			strcpy(php_ini_path,default_location);
		}
		php3_ini.safe_mode = 0;
		cfgin = php3_fopen_with_path("php3.ini","r",php_ini_path,&opened_path);
		free(php_ini_path);
		php3_ini.safe_mode = safe_mode_state;

		if (!cfgin) {
# if WIN32|WINNT
			return FAILURE;
# else
			return SUCCESS;  /* having no configuration file is ok */
# endif
		}

		if (opened_path) {
			pval tmp;
			
			tmp.value.str.val = opened_path;
			tmp.value.str.len = strlen(opened_path);
			tmp.type = IS_STRING;
			_php3_hash_update(&configuration_hash,"cfg_file_path",sizeof("cfg_file_path"),(void *) &tmp,sizeof(pval),NULL);
#if 0
			php3_printf("INI file opened at '%s'\n",opened_path);
#endif
		}
			
		init_cfg_scanner();
		active__php3_hash_table = &configuration_hash;
		parsing_mode = PARSING_MODE_CFG;
		currently_parsed_filename = "php3.ini";
		yyparse();
		fclose(cfgin);
	}
	
#endif
	
	return SUCCESS;
}


int php3_minit_browscap(INIT_FUNC_ARGS)
{
	TLS_VARS;

	if (php3_ini.browscap) {
		if (_php3_hash_init(&GLOBAL(browser_hash), 0, NULL, (void (*)(void *))pvalue_browscap_destructor, 1)==FAILURE) {
			return FAILURE;
		}

		cfgin = fopen(php3_ini.browscap,"r");
		if (!cfgin) {
			php3_error(E_WARNING,"Cannot open '%s' for reading",php3_ini.browscap);
			return FAILURE;
		}
		init_cfg_scanner();
		active__php3_hash_table = &GLOBAL(browser_hash);
		parsing_mode = PARSING_MODE_BROWSCAP;
		currently_parsed_filename = php3_ini.browscap;
		yyparse();
		fclose(cfgin);
	}

	return SUCCESS;
}


int php3_shutdown_config(void)
{
	_php3_hash_destroy(&configuration_hash);
	return SUCCESS;
}


int php3_mshutdown_browscap(void)
{
	TLS_VARS;

	if (php3_ini.browscap) {
		_php3_hash_destroy(&GLOBAL(browser_hash));
	}
	return SUCCESS;
}


static void convert_browscap_pattern(pval *pattern)
{
	register int i,j;
	char *t;

	for (i=0; i<pattern->value.str.len; i++) {
		if (pattern->value.str.val[i]=='*' || pattern->value.str.val[i]=='?') {
			break;
		}
	}

	if (i==pattern->value.str.len) { /* no wildcards */
		return;
	}

	t = (char *) malloc(pattern->value.str.len*2);
	
	for (i=0,j=0; i<pattern->value.str.len; i++,j++) {
		switch (pattern->value.str.val[i]) {
			case '?':
				t[j] = '.';
				break;
			case '*':
				t[j++] = '.';
				t[j] = '*';
				break;
			case '.':
				t[j++] = '\\';
				t[j] = '.';
				break;
			default:
				t[j] = pattern->value.str.val[i];
				break;
		}
	}
	t[j]=0;
	free(pattern->value.str.val);
	pattern->value.str.val = t;
	pattern->value.str.len = j;
	return;
}

%}

%pure_parser
%token STRING
%token ENCAPSULATED_STRING
%token SECTION
%token CFG_TRUE
%token CFG_FALSE
%token EXTENSION

%%

statement_list:
		statement_list statement
	|	/* empty */
;

statement:
		string '=' string_or_value {
#if 0
			printf("'%s' = '%s'\n",$1.value.str.val,$3.value.str.val);
#endif
			$3.type = IS_STRING;
			if (parsing_mode==PARSING_MODE_CFG) {
				_php3_hash_update(active__php3_hash_table, $1.value.str.val, $1.value.str.len+1, &$3, sizeof(pval), NULL);
			} else if (parsing_mode==PARSING_MODE_BROWSCAP) {
				php3_str_tolower($1.value.str.val,$1.value.str.len);
				_php3_hash_update(current_section->value.ht, $1.value.str.val, $1.value.str.len+1, &$3, sizeof(pval), NULL);
			}
			free($1.value.str.val);
		}
	|	string { free($1.value.str.val); }
	|	EXTENSION '=' string {
			pval dummy;
#if 0
			printf("Loading '%s'\n",$3.value.str.val);
#endif
			
			php3_dl(&$3,MODULE_PERSISTENT,&dummy);
		}
	|	SECTION { 
			if (parsing_mode==PARSING_MODE_BROWSCAP) {
				pval tmp;

				/*printf("'%s' (%d)\n",$1.value.str.val,$1.value.str.len+1);*/
				tmp.value.ht = (HashTable *) malloc(sizeof(HashTable));
				_php3_hash_init(tmp.value.ht, 0, NULL, (void (*)(void *))pvalue_config_destructor, 1);
				tmp.type = IS_OBJECT;
				_php3_hash_update(active__php3_hash_table, $1.value.str.val, $1.value.str.len+1, (void *) &tmp, sizeof(pval), (void **) &current_section);
				tmp.value.str.val = php3_strndup($1.value.str.val,$1.value.str.len);
				tmp.value.str.len = $1.value.str.len;
				tmp.type = IS_STRING;
				convert_browscap_pattern(&tmp);
				_php3_hash_update(current_section->value.ht,"browser_name_pattern",sizeof("browser_name_pattern"),(void *) &tmp, sizeof(pval), NULL);
			}
			free($1.value.str.val);
		}
	|	'\n'
;


string:
		STRING { $$ = $1; }
	|	ENCAPSULATED_STRING { $$ = $1; }
;

string_or_value:
		string { $$ = $1; }
	|	CFG_TRUE { $$ = $1; }
	|	CFG_FALSE { $$ = $1; }
	|	'\n' { $$.value.str.val = strdup(""); $$.value.str.len=0; $$.type = IS_STRING; }
;


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
