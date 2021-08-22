/* 
   +----------------------------------------------------------------------+
   | PHP HTML Embedded Scripting Language Version 3.0                     |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2000 PHP Development Team (See Credits file)      |
   +----------------------------------------------------------------------+
   | This program is free software;you can redistribute it and/or modify |
   | it under the terms of one of the following licenses:                 |
   |                                                                      |
   |  A) the GNU General Public License as published by the Free Software |
   |     Foundation;either version 2 of the License, or (at your option) |
   |     any later version.                                               |
   |                                                                      |
   |  B) the PHP License as published by the PHP Development Team and     |
   |     included in the distribution in the file: LICENSE                |
   |                                                                      |
   | This program is distributed in the hope that it will be useful,      |
   | but WITHOUT ANY WARRANTY;without even the implied warranty of       |
   | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |
   | GNU General Public License for more details.                         |
   |                                                                      |
   | You should have received a copy of both licenses referred to here.   |
   | If you did not, or have any questions about PHP licensing, please    |
   | contact core@php.net.                                                |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <bourbon@netvision.net.il>                     |
   |          Rasmus Lerdorf <rasmus@lerdorf.on.ca>                       |
   +----------------------------------------------------------------------+
 */

/* $Id: main.c,v 1.517 2000/10/16 17:21:08 sas Exp $ */

/* #define CRASH_DETECTION */

#define SHUTDOWN_DEBUG(resource) fprintf(stderr, "*** Shutting down " resource "\n" )
#undef SHUTDOWN_DEBUG
#define SHUTDOWN_DEBUG(resource)

#include <stdio.h>
#include "php.h"
#ifdef MSVC5
#include "win32/time.h"
#include "win32/signal.h"
#include <process.h>
#else
#include "build-defs.h"
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif
#include "language-parser.tab.h"
#include "main.h"
#include "control_structures.h"
#include "modules.h"
#include "internal_functions.h"
#include "fopen-wrappers.h"
#include "functions/basic_functions.h"
#include "functions/info.h"
#include "functions/head.h"
#include "functions/post.h"
#include "functions/head.h"
#include "functions/type.h"
#include "highlight.h"
#include "php3_list.h"
#include "snprintf.h"
#if WIN32|WINNT
#include <io.h>
#include <fcntl.h>
#include "win32/syslog.h"
#elif HAVE_SYSLOG_H
#include <syslog.h>
#endif

#if PHP_SIGCHILD
#include <sys/types.h>
#include <sys/wait.h>
#endif

#if USE_SAPI
#include "serverapi/sapi.h"
void *gLock;
struct sapi_request_info *sapi_rqst;
#endif

#if MSVC5 || !defined(HAVE_GETOPT)
#include "php_getopt.h"
#endif

void *gLock;					/*mutex variable */

int php_connection_status;
int ignore_user_abort;
int error_reporting, tmp_error_reporting;
int initialized;				/* keep track of which resources were successfully initialized */
static int module_initialized = 0;
char *php3_ini_path = NULL;
int wanted_exit_status = 0;
int shutdown_requested;
unsigned char header_is_being_sent;
unsigned int max_execution_time = 0;
#if APACHE
request_rec *php3_rqst = NULL;	/* request record pointer for apache module version */
#endif

/* This one doesn't exists on QNX */
#ifndef SIGPROF
#define SIGPROF 27
#endif

#if PHP_DEBUGGER
extern int debugger_on;
#endif
#if WIN32|WINNT
unsigned int wintimer;
unsigned int timerstart;
unsigned int wintimer_counter = 0;
#endif

HashTable configuration_hash;

extern FILE *phpin;

php3_ini_structure php3_ini;
php3_ini_structure php3_ini_master;

void _php3_build_argv(char *);
static void php3_timeout(int dummy);
static void php3_set_timeout(long seconds INLINE_TLS);


/* string destructor for hash */
static void str_free(void **ptr)
{
	efree(*ptr);
}

int php3_get_lineno(int lineno)
{
	return LINE_OFFSET(lineno);
}

char *php3_get_filename(int lineno)
{
	char **filename_ptr;
	int file_offset = FILE_OFFSET(lineno);
	TLS_VARS;

	if ((GLOBAL(initialized) & INIT_INCLUDE_NAMES_HASH) && _php3_hash_index_find(&GLOBAL(include_names), file_offset, (void **) &filename_ptr) == SUCCESS) {
		return *filename_ptr;
	} else {
		return "-";
	}
}


/* this should be compatible with the standard phperror */
void phperror(char *error)
{
	php3_error(E_PARSE, error);
}


PHPAPI int php3_write(const void *a, int n)
{
	int ret;
	TLS_VARS;

#if APACHE
	ret = rwrite(a,n,GLOBAL(php3_rqst));
#else
#if FHTTPD
	ret = php3_fhttpd_write((void*)a,n);
#else /* CGI */
	ret = fwrite(a,1,n,stdout);
#endif
#endif

	if (ret != n) {
		GLOBAL(php_connection_status) |= PHP_CONNECTION_ABORTED;
	}

	return ret;
}

PHPAPI void php3_puts(const char *s)
{
	TLS_VARS;

#if APACHE
	if (GLOBAL(php3_rqst)) {
		if (rputs(s, GLOBAL(php3_rqst)) == -1) {
			GLOBAL(php_connection_status) |= PHP_CONNECTION_ABORTED;
		}
	} else {
		fputs(s, stdout);
	}
#else
#if FHTTPD
	php3_fhttpd_puts((char*)s);
#else /* CGI */
	if (fputs(s, stdout) < 0) {
		GLOBAL(php_connection_status) |= PHP_CONNECTION_ABORTED;
	}
#endif
#endif
}

PHPAPI void php3_putc(char c)
{
	TLS_VARS;

#if APACHE
	if (GLOBAL(php3_rqst)) {
		if (rputc(c, GLOBAL(php3_rqst)) != c) {
			GLOBAL(php_connection_status) |= PHP_CONNECTION_ABORTED;
		}
	} else {
		fputc(c, stdout);
	}
#else
#if FHTTPD
	php3_fhttpd_putc(c);
#else /* CGI */
	if (fputc(c, stdout) != c) {
		GLOBAL(php_connection_status) |= PHP_CONNECTION_ABORTED;
	}
#endif
#endif
}

void php3_log_err(char *log_message)
{
	FILE *log_file;
	TLS_VARS;

	/* Try to use the specified logging location. */
	if (php3_ini.error_log != NULL) {
#if HAVE_SYSLOG_H
		if (!strcmp(php3_ini.error_log, "syslog")) {
			syslog(LOG_NOTICE, "%s", log_message);
			return;
		} else {
#endif
			log_file = fopen(php3_ini.error_log, "a");
			if (log_file != NULL) {
				fprintf(log_file, "%s", log_message);
				fprintf(log_file, "\n");
				fclose(log_file);
				return;
			}
#if HAVE_SYSLOG_H
		}
#endif
	}
	/* Otherwise fall back to the default logging location. */
#if APACHE
	if (GLOBAL(php3_rqst)) {
#if MODULE_MAGIC_NUMBER >= 19970831
		aplog_error(NULL, 0, APLOG_ERR | APLOG_NOERRNO, php3_rqst->server,
					"%s", log_message);
#else
		log_error(log_message, php3_rqst->server);
#endif
	} else {
		fprintf(stderr, "%s", log_message);
		fprintf(stderr, "\n");
	}
#endif							/*APACHE */

#if CGI_BINARY
	if (php3_header()) {
		fprintf(stderr, "%s", log_message);
		fprintf(stderr, "\n");
	}
#endif
#if USE_SAPI
	if (php3_header()) {
		GLOBAL(sapi_rqst)->log(GLOBAL(sapi_rqst)->scid, log_message);
	}
#endif
}


/* is 4K big enough? */
#define PRINTF_BUFFER_SIZE 1024*4

PHPAPI int php3_printf(const char *format,...)
{
	va_list args;
	int ret;
#if WIN32_SERVER_MOD || USE_SAPI || FHTTPD
	char buffer[PRINTF_BUFFER_SIZE];
	int size;
#endif
	TLS_VARS;

	va_start(args, format);
#if APACHE
	if (GLOBAL(php3_rqst)) {
#if USE_TRANSFER_TABLES
		ret = charset_vbprintf(GLOBAL(php3_rqst)->connection->client, GLOBAL(php3_rqst), format, args);
#else
		ret = vbprintf(GLOBAL(php3_rqst)->connection->client, format, args);
#endif
	} else {
		ret = vfprintf(stdout, format, args);
	}
#endif

#if CGI_BINARY
	ret = vfprintf(stdout, format, args);
#endif

#if FHTTPD
	size = vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, args);
	ret = php3_fhttpd_write(buffer, size);
#endif

#if USE_SAPI
	size = vsprintf(buffer, format, args);
	ret = GLOBAL(sapi_rqst)->writeclient(GLOBAL(sapi_rqst)->scid, buffer, size);
#endif
	va_end(args);
	return ret;
}


/* extended error handling function */
PHPAPI void php3_error(int type, const char *format,...)
{
	va_list args;
	char *filename = NULL;
	char buffer[1024];
	int size = 0;
	TLS_VARS;

	if (!(type & E_CORE)) {
		if (!GLOBAL(initialized) || GLOBAL(shutdown_requested)) {	/* don't display further errors after php3_request_shutdown() */
			return;
		}
	}
	if (GLOBAL(error_reporting) & type || (type & E_CORE)) {
		char *error_type_str;

		switch (type) {
			case E_ERROR:		/* 0x01 */
			case E_CORE_ERROR:	/* 0x10 */
				error_type_str = "Fatal error";
				break;
			case E_WARNING:	/* 0x02 */
			case E_CORE_WARNING:	/* 0x20 */
				error_type_str = "Warning";
				break;
			case E_PARSE:		/* 0x04 */
				error_type_str = "Parse error";
				break;
			case E_NOTICE:		/* 0x08 */
				error_type_str = "Warning";
				break;
			default:
				error_type_str = "Unknown error";
				break;
		}

		/* get include file name */
		if (!(type & E_CORE)) {
			filename = php3_get_filename(GLOBAL(current_lineno));
		}
		if (php3_ini.log_errors || php3_ini.display_errors) {
			va_start(args, format);
			size = vsnprintf(buffer, sizeof(buffer) - 1, format, args);
			va_end(args);
			buffer[sizeof(buffer) - 1] = 0;

			if (php3_ini.log_errors) {
				char log_buffer[1024];

				snprintf(log_buffer, 1024, "PHP 3 %s:  %s in %s on line %d", error_type_str, buffer, filename, php3_get_lineno(GLOBAL(current_lineno)));
				php3_log_err(log_buffer);
			}
			if (php3_ini.display_errors) {
				if (!php3_header()) {
					return;
				}
				if(php3_ini.error_prepend_string) {
					PUTS(php3_ini.error_prepend_string);
				}		
				php3_printf("<br>\n<b>%s</b>:  %s in <b>%s</b> on line <b>%d</b><br>\n", error_type_str, buffer, filename, php3_get_lineno(GLOBAL(current_lineno)));
				if(php3_ini.error_append_string) {
					PUTS(php3_ini.error_append_string);
				}		
			}
		}
	}
	if (php3_ini.track_errors && (GLOBAL(initialized) & INIT_SYMBOL_TABLE)) {
		pval tmp;

		va_start(args, format);
		size = vsnprintf(buffer, sizeof(buffer) - 1, format, args);
		va_end(args);
		buffer[sizeof(buffer) - 1] = 0;

		tmp.value.str.val = (char *) estrndup(buffer, size);
		tmp.value.str.len = size;
		tmp.type = IS_STRING;

		_php3_hash_update(GLOBAL(active_symbol_table), "php_errormsg", sizeof("php_errormsg"), (void *) & tmp, sizeof(pval), NULL);
	}
#if PHP_DEBUGGER
	/* Send a message to the debugger no matter if we are configured
	 * not to display this error.
	 */
	if (GLOBAL(debugger_on)) {
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer) - 1, format, args);
		php3_debugger_error(buffer, type, filename, php3_get_lineno(GLOBAL(current_lineno)));
		va_end(args);
	}
#endif

	switch (type) {
		case E_ERROR:
		case E_CORE_ERROR:
		case E_PARSE:
			GLOBAL(shutdown_requested) = ABNORMAL_SHUTDOWN;
			break;
	}
}


/* phpparse()'s front end to the token cache */
int phplex(pval *phplval)
{
	Token *token;

	/* this is timing for windows */
#if (WIN32|WINNT)
	if (GLOBAL(wintimer) && !(++GLOBAL(wintimer_counter) & 0xff) 
		  && (GLOBAL(wintimer) < (unsigned int) clock())) {
		php3_error(E_NOTICE, "PHP Timed out!<br>\n");
		GLOBAL(shutdown_requested) = ABNORMAL_SHUTDOWN;
		GLOBAL(php_connection_status) |= PHP_CONNECTION_TIMEOUT;
		/*GLOBAL(ignore_user_abort) = 1;*/
	}
#endif
	
	if (!GLOBAL(initialized) || GLOBAL(shutdown_requested)) {
		if (GLOBAL(shutdown_requested)==TERMINATE_CURRENT_PHPPARSE) {
			GLOBAL(shutdown_requested)=0;
		}
		return 0;
	}


#if APACHE
	if ((php3_rqst->connection->aborted || GLOBAL(php_connection_status)&PHP_CONNECTION_ABORTED) 
		  && !GLOBAL(ignore_user_abort)) {
		GLOBAL(shutdown_requested) = ABNORMAL_SHUTDOWN;
		/* 
			ignore_user_abort is used to tell phplex() that even though we know that the
			remote client is no longer listening to us, we still want it to continue in case
			we come back here as part of a registered shutdown function.  Without this flag
			a user-registered shutdown function would never run to completion.
		*/
		GLOBAL(ignore_user_abort) = 1;
		return 0;
	}
#else /* CGI */
#if CGI_CHECK_ABORT
	if ((GLOBAL(php_connection_status)&PHP_CONNECTION_ABORTED) && !GLOBAL(ignore_user_abort)) {
		GLOBAL(shutdown_requested) = ABNORMAL_SHUTDOWN;
		GLOBAL(ignore_user_abort) = 1;
		return 0;
	}
#endif
#endif

	switch (read_next_token(&GLOBAL(token_cache_manager), &token, phplval)) {
		case FAILURE:
			php3_error(E_ERROR, "Unable to read next token!\n");
			return 0;
			break;
		case DONE_EVAL:
			return phplex(phplval);
			break;
	}
	*phplval = token->phplval;
	GLOBAL(current_lineno) = token->lineno;
	return token->token_type;
}


#if HAVE_SETITIMER
static void php3_timeout(int dummy)
{
	TLS_VARS;

	if (!GLOBAL(shutdown_requested)) {
		php3_error(E_ERROR, "Maximum execution time exceeded");
		GLOBAL(php_connection_status) |= PHP_CONNECTION_TIMEOUT;
		/* Now, schedule another alarm.  If we're stuck in a code portion that will not go through
		 * phplex() or if the parser is broken, end the process ungracefully
		 */
		php3_set_timeout(3);	/* allow 3 seconds for shutdown... */
	} else {					/* we're here for a second time.  exit ungracefully */
		exit(1);
	}
}
#endif


static void php3_set_timeout(long seconds INLINE_TLS)
{
#if WIN32|WINNT
	if (seconds > 0) {
		GLOBAL(timerstart) = (unsigned int) clock();
		GLOBAL(wintimer) = GLOBAL(timerstart) + (CLOCKS_PER_SEC * seconds);
	} else {
		GLOBAL(wintimer) = 0;
	}
#else
#if HAVE_SETITIMER
	struct itimerval t_r;		/* timeout requested */

	t_r.it_value.tv_sec = seconds;
	t_r.it_value.tv_usec = t_r.it_interval.tv_sec = t_r.it_interval.tv_usec = 0;

	setitimer(ITIMER_PROF, &t_r, NULL);
	signal(SIGPROF, php3_timeout);
#endif
#endif
}


static void php3_unset_timeout(INLINE_TLS_VOID)
{
#if WIN32|WINNT
	GLOBAL(wintimer) = 0;
#else
#if HAVE_SETITIMER
	struct itimerval no_timeout;

	no_timeout.it_value.tv_sec = no_timeout.it_value.tv_usec = no_timeout.it_interval.tv_sec = no_timeout.it_interval.tv_usec = 0;

	setitimer(ITIMER_PROF, &no_timeout, NULL);
#endif
#endif
}


/* {{{ proto void set_time_limit(int seconds)
   Sets the maximum time a script can run */
void php3_set_time_limit(INTERNAL_FUNCTION_PARAMETERS)
{
	pval *new_timeout;
	TLS_VARS;

	if (php3_ini.safe_mode) {
		php3_error(E_WARNING, "Cannot set time limit in safe mode");
		RETURN_FALSE;
	}
	if (ARG_COUNT(ht) != 1 || getParameters(ht, 1, &new_timeout) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
	convert_to_long(new_timeout);
	/* FIXME ** This is BAD...in a threaded situation, any user
	   can set the timeout for php on a server wide basis. 
	   INI variables should not be reset via a user script

	   Fix what?  At least on Unix, timers like these are
	   per-thread timers.  Well, with a little work they will
	   be.  If we use a bound thread and proper masking it
	   should work fine.  Is this FIXME a WIN32 problem?  Is
	   there no way to do per-thread timers on WIN32?

	   Something to keep in mind here is that the SIGPROF itimer
	   we are currently using is not a real-time timer.  It is 
	   only active when the process is in user or kernel space.
	   ie. a sleep(10);call in a script will not count towards
	   the timeout limit. -RL
	 */
	GLOBAL(max_execution_time) = new_timeout->value.lval;
	php3_unset_timeout(_INLINE_TLS_VOID);
	php3_set_timeout(new_timeout->value.lval _INLINE_TLS);
}
/* }}} */


#if PHP_SIGCHILD
static void sigchld_handler(int apar)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	signal(SIGCHLD,sigchld_handler);
}
#endif



int php3_request_startup(INLINE_TLS_VOID)
{
#if APACHE && defined(CRASH_DETECTION)
	{
		char log_message[256];
		
		snprintf(log_message,256,"php3_request_startup(): script='%s', pid=%d",php3_rqst->filename,getpid());
		log_error(log_message, php3_rqst->server);
	}
#endif

#if PHP_SIGCHILD
	signal(SIGCHLD,sigchld_handler);
#endif


	GLOBAL(max_execution_time) = php3_ini.max_execution_time;
	php3_set_timeout(GLOBAL(max_execution_time) _INLINE_TLS);

	GLOBAL(initialized) = 0;

	start_memory_manager();

#if APACHE
	/*
	 * For the Apache module version, this bit of code registers a cleanup
	 * function that gets triggered when our request pool is destroyed.
	 * We need this because at any point in our code we can be interrupted
	 * and that may happen before we have had time to free our memory.
	 * The php3_shutdown function needs to free all outstanding allocated
	 * memory.  
	 */
	block_alarms();
	register_cleanup(GLOBAL(php3_rqst)->pool, NULL, php3_request_shutdown, php3_request_shutdown_for_exec);
	unblock_alarms();
#endif

	/* initialize global variables */
	{
		GLOBAL(ExecuteFlag) = EXECUTE;
		GLOBAL(Execute) = 1;
		GLOBAL(php3_display_source) = 0;
		GLOBAL(php3_preprocess) = PREPROCESS_NONE;
		GLOBAL(include_count) = 0;
		GLOBAL(active_symbol_table) = &GLOBAL(symbol_table);
		GLOBAL(function_state.loop_nest_level) = GLOBAL(function_state.loop_change_level) = GLOBAL(function_state.loop_change_type) = 0;
		GLOBAL(function_state.returned) = GLOBAL(function_state.function_type) = 0;
		GLOBAL(function_state.symbol_table) = &GLOBAL(symbol_table);
		GLOBAL(function_state.function_symbol_table) = NULL;
		GLOBAL(function_state.function_name) = NULL;
		GLOBAL(function_state.handler) = NULL;
		GLOBAL(function_state.func_arg_types) = NULL;
		GLOBAL(phplineno) = 1;
		GLOBAL(error_reporting) = php3_ini.errors;
		GLOBAL(shutdown_requested) = 0;
		GLOBAL(header_is_being_sent) = 0;
		GLOBAL(php3_track_vars) = php3_ini.track_vars;
		GLOBAL(php_connection_status) = PHP_CONNECTION_NORMAL;
		GLOBAL(ignore_user_abort) = 0;
	}

	if (php3_init_request_info((void *) &php3_ini)) {
		php3_printf("Unable to initialize request info.\n");
		return FAILURE;
	}
	_php3_hash_init(&GLOBAL(request_info).rfc1867_uploaded_files, 5, NULL, NULL, 0);

	GLOBAL(initialized) |= INIT_REQUEST_INFO;

	/* prepare general symbol table hash */
	if (_php3_hash_init(&GLOBAL(symbol_table), 50, NULL, PVAL_DESTRUCTOR, 0) == FAILURE) {
		php3_printf("Unable to initialize symbol table.\n");
		return FAILURE;
	}
	/* this pval will be used for the GLOBALS[] array implementation */
	GLOBAL(globals.value.ht) = &GLOBAL(symbol_table);
	GLOBAL(globals.type) = IS_ARRAY;
	_php3_hash_pointer_update(&GLOBAL(symbol_table), "GLOBALS", sizeof("GLOBALS"), (void *) & GLOBAL(globals));
	GLOBAL(initialized) |= INIT_SYMBOL_TABLE;


	/* prepare token cache */
	if (tcm_init(&GLOBAL(token_cache_manager)) == FAILURE) {
		php3_printf("Unable to initialize token cache.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_TOKEN_CACHE;

	/* initialize stacks */
	if (php3i_stack_init(&GLOBAL(css)) == FAILURE) {
		php3_printf("Unable to initialize Control Structure stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_CSS;

	if (php3i_stack_init(&GLOBAL(for_stack)) == FAILURE) {
		php3_printf("Unable to initialize for stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_FOR_STACK;

	if (php3i_stack_init(&GLOBAL(switch_stack)) == FAILURE) {
		php3_printf("Unable to initialize switch stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_SWITCH_STACK;

	if (php3i_stack_init(&GLOBAL(input_source_stack)) == FAILURE) {
		php3_printf("Unable to initialize include stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_INCLUDE_STACK;

	if (php3i_stack_init(&GLOBAL(function_state_stack)) == FAILURE) {
		php3_printf("Unable to initialize function state stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_FUNCTION_STATE_STACK;

	if (php3i_stack_init(&GLOBAL(variable_unassign_stack)) == FAILURE) {
		php3_printf("Unable to initialize variable unassignment stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_VARIABLE_UNASSIGN_STACK;

	/* call request startup for modules */
	_php3_hash_apply(&GLOBAL(module_registry), (int (*)(void *)) module_registry_request_startup);

	/* include file names */
	if (_php3_hash_init(&GLOBAL(include_names), 0, NULL, (void (*)(void *ptr)) str_free, 0) == FAILURE) {
		php3_printf("Unable to start include names stack.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_INCLUDE_NAMES_HASH;


	if (init_resource_list() == FAILURE) {
		php3_printf("Unable to start object list hash.\n");
		return FAILURE;
	}
	GLOBAL(initialized) |= INIT_LIST;

	return SUCCESS;
}


void php3_request_shutdown_for_exec(void *dummy)
{
	TLS_VARS;
	
	/* used to close fd's in the 3..255 range here, but it's problematic
	 */
	GLOBAL(error_reporting) = 0;
	shutdown_memory_manager();
}


void php3_request_shutdown(void *dummy INLINE_TLS)
{
#if FHTTPD
	char tmpline[128];
	int i, serverdefined;
#endif
#if APACHE && defined(CRASH_DETECTION)
	{
		char log_message[256];
		
		snprintf(log_message,256,"php3_request_shutdown(): script='%s', pid=%d",php3_rqst->filename,getpid());
		log_error(log_message, php3_rqst->server);
	}
#endif
	php3_call_shutdown_functions();
	
	if (GLOBAL(initialized) & INIT_LIST) {
		SHUTDOWN_DEBUG("Resource list");
		destroy_resource_list();
		GLOBAL(initialized) &= ~INIT_LIST;
	}

	/* clean temporary dl's, run request shutdown's for modules */
	SHUTDOWN_DEBUG("Modules");
	_php3_hash_apply(&GLOBAL(module_registry), (int (*)(void *)) module_registry_cleanup);

	if (GLOBAL(initialized) & INIT_SYMBOL_TABLE) {
		SHUTDOWN_DEBUG("Symbol table");
		_php3_hash_destroy(&GLOBAL(symbol_table));
		GLOBAL(initialized) &= ~INIT_SYMBOL_TABLE;
	}
	GLOBAL(initialized) &= ~INIT_ENVIRONMENT;	/* does not require any special shutdown */

	/* remove classes and user-functions */
	if (GLOBAL(module_initialized) & INIT_FUNCTION_TABLE) {
		SHUTDOWN_DEBUG("Function table");
		_php3_hash_apply(&GLOBAL(function_table), (int (*)(void *)) is_not_internal_function);
	}
	if (GLOBAL(initialized) & INIT_TOKEN_CACHE) {
		SHUTDOWN_DEBUG("Token cache");
		tcm_destroy(&GLOBAL(token_cache_manager));
		GLOBAL(initialized) &= ~INIT_TOKEN_CACHE;
	}
	if (GLOBAL(initialized) & INIT_CSS) {
		SHUTDOWN_DEBUG("Control Structures Stack");
		php3i_stack_destroy(&GLOBAL(css));
		GLOBAL(initialized) &= ~INIT_CSS;
	}
	if (GLOBAL(initialized) & INIT_FOR_STACK) {
		SHUTDOWN_DEBUG("For stack");
		php3i_stack_destroy(&GLOBAL(for_stack));
		GLOBAL(initialized) &= ~INIT_FOR_STACK;
	}
	if (GLOBAL(initialized) & INIT_SWITCH_STACK) {
		switch_expr *se;

		SHUTDOWN_DEBUG("Switch stack");
		while (php3i_stack_top(&GLOBAL(switch_stack), (void **) &se) != FAILURE) {
			pval_destructor(&se->expr _INLINE_TLS);
			php3i_stack_del_top(&GLOBAL(switch_stack));
		}
		php3i_stack_destroy(&GLOBAL(switch_stack));
		GLOBAL(initialized) &= ~INIT_SWITCH_STACK;
	}

	if (GLOBAL(initialized) & INIT_INCLUDE_STACK) {
		SHUTDOWN_DEBUG("Input source stack");
		clean_input_source_stack();
	}
	if (GLOBAL(initialized) & INIT_FUNCTION_STATE_STACK) {
		FunctionState *tmp;
		HashTable *last_symbol_table=NULL;/* to protect against freeing the same symtable twice,
											* if we 'crashed' in a nested function call
											*/

		SHUTDOWN_DEBUG("Function state stack");
		while (php3i_stack_top(&GLOBAL(function_state_stack), (void **) &tmp) != FAILURE) {
			if (tmp->function_name) {
				efree(tmp->function_name);
				if (tmp->symbol_table 
					&& (tmp->symbol_table != &GLOBAL(symbol_table))
					&& (tmp->symbol_table != last_symbol_table)) {
					_php3_hash_destroy(tmp->symbol_table);
					efree(tmp->symbol_table);
					last_symbol_table = tmp->symbol_table;
				}
			}
			php3i_stack_del_top(&GLOBAL(function_state_stack));
		}
		if (GLOBAL(function_state).function_name) {
			efree(GLOBAL(function_state).function_name);
			if (GLOBAL(function_state).symbol_table
				&& (GLOBAL(function_state).symbol_table != &GLOBAL(symbol_table))
				&& (GLOBAL(function_state).symbol_table != last_symbol_table)) {
				_php3_hash_destroy(GLOBAL(function_state).symbol_table);
				efree(GLOBAL(function_state).symbol_table);
			}
		}
		php3i_stack_destroy(&GLOBAL(function_state_stack));
		GLOBAL(initialized) &= ~INIT_FUNCTION_STATE_STACK;
	}
	
	if (GLOBAL(initialized) & INIT_VARIABLE_UNASSIGN_STACK) {
		variable_tracker *tmp;

		SHUTDOWN_DEBUG("Variable unassign stack");
		while (php3i_stack_top(&GLOBAL(variable_unassign_stack), (void **) &tmp) != FAILURE) {
			if (tmp->type == IS_STRING) {
				STR_FREE(tmp->strval);
			}
			php3i_stack_del_top(&GLOBAL(variable_unassign_stack));
		}
		php3i_stack_destroy(&GLOBAL(variable_unassign_stack));
		GLOBAL(initialized) &= ~INIT_VARIABLE_UNASSIGN_STACK;
	}

	if (GLOBAL(module_initialized) & INIT_CONSTANTS) {
		/* clean temporary defined constants */
		SHUTDOWN_DEBUG("Non persistent constants");
		clean_non_persistent_constants();
	}

	if (GLOBAL(initialized) & INIT_REQUEST_INFO) {
		SHUTDOWN_DEBUG("Request info");
		php3_destroy_request_info((void *) &php3_ini);
		GLOBAL(initialized) &= ~INIT_REQUEST_INFO;
		_php3_hash_destroy(&GLOBAL(request_info).rfc1867_uploaded_files);
	}
	if (GLOBAL(initialized) & INIT_INCLUDE_NAMES_HASH) {
		SHUTDOWN_DEBUG("Include names hash");
		_php3_hash_destroy(&GLOBAL(include_names));
		GLOBAL(initialized) &= ~INIT_INCLUDE_NAMES_HASH;
	}
	if (GLOBAL(initialized) & INIT_SCANNER) {
		SHUTDOWN_DEBUG("Scanner");
		reset_scanner();
		GLOBAL(initialized) &= ~INIT_SCANNER;
	}
	if (GLOBAL(initialized) & INIT_MEMORY_MANAGER) {
		SHUTDOWN_DEBUG("Memory manager");
		shutdown_memory_manager();
	}
	if (GLOBAL(initialized)) {
		php3_error(E_WARNING, "Unknown resources in request shutdown function");
	}
	php3_unset_timeout(_INLINE_TLS_VOID);

#if CGI_BINARY
	fflush(stdout);
	if(GLOBAL(request_info).php_argv0) {
		free(GLOBAL(request_info).php_argv0);
		GLOBAL(request_info).php_argv0 = NULL;
	}
#endif
#if FHTTPD
	if (response) {
		if (!headermade) {
			makestandardheader(response, 200, "text/html", "fhttpd", req && req->keepalive);
		} else {
			if (headerfirstline)
				putlinetoheader(response, headerfirstline);
			else
				putlinetoheader(response, "HTTP/1.0 200 OK\r\n");
			serverdefined = 0;
			for (i = 0;i < headerlines;i++) {
				if (!strncmp(currentheader[i], "Server:", 7))
					serverdefined = 1;
				putlinetoheader(response, currentheader[i]);
			}
			if (!serverdefined)
				putlinetoheader(response, "Server: fhttpd\r\n");
			if (response->datasize) {
				sprintf(tmpline, "Content-Length: %ld\r\n", response->datasize);
				putlinetoheader(response, tmpline);
				if (req && req->keepalive)
					putlinetoheader(response,
									"Connection: Keep-Alive\r\nKeep-Alive: max=0, timeout=30\r\n");
			}
			php3_fhttpd_free_header();
		}
		sendresponse(server, response);
		if (response->datasize)
			finishresponse(server, response);
		else
			finishdropresponse(server, response);
		deleteresponse(response);
	}
	response = NULL;
	if (req)
		deleterequest(req);
	req = NULL;
#endif
#if USE_SAPI
	GLOBAL(sapi_rqst)->flush(GLOBAL(sapi_rqst)->scid);
#endif
}

static int php3_config_ini_startup(INLINE_TLS_VOID)
{
	/* set the memory limit to a reasonable number so that we can get
	 * through this startup phase properly
	 */
	php3_ini.memory_limit=1<<23;/* 8MB */
	
	if (php3_init_config() == FAILURE) {
		php3_printf("PHP:  Unable to parse configuration file.\n");
		return FAILURE;
	}
#if !(USE_SAPI)
	GLOBAL(module_initialized) |= INIT_CONFIG;
#endif
	/* initialize run-time variables */
	/* I have remarked out some stuff 
	   that may or may not be needed */
	{
		char *temp;

		if (cfg_get_long("max_execution_time", &php3_ini.max_execution_time) == FAILURE) {
			php3_ini.max_execution_time = 30;
		}
		if (cfg_get_long("memory_limit", &php3_ini.memory_limit) == FAILURE) {
			php3_ini.memory_limit = 1<<23; /* 8MB */
		}
		if (cfg_get_long("precision", &php3_ini.precision) == FAILURE) {
			php3_ini.precision = 14;
		}
		if (cfg_get_string("SMTP", &php3_ini.smtp) == FAILURE) {
			php3_ini.smtp = "localhost";
		}
		if (cfg_get_string("sendmail_path", &php3_ini.sendmail_path) == FAILURE
			|| !php3_ini.sendmail_path[0]) {
#ifdef PHP_PROG_SENDMAIL
			php3_ini.sendmail_path = PHP_PROG_SENDMAIL " -t";
#else
			php3_ini.sendmail_path = NULL;
#endif
		}
		if (cfg_get_string("sendmail_from", &php3_ini.sendmail_from) == FAILURE) {
			php3_ini.sendmail_from = NULL;
		}
		if (cfg_get_long("error_reporting", &php3_ini.errors) == FAILURE) {
			php3_ini.errors = E_ALL & ~E_NOTICE;
		}
		if (cfg_get_string("error_log", &php3_ini.error_log) == FAILURE) {
			php3_ini.error_log = NULL;
		}
		GLOBAL(error_reporting) = php3_ini.errors;

		if (cfg_get_long("track_errors", &php3_ini.track_errors) == FAILURE) {
			php3_ini.track_errors = 0;
		}
		if (cfg_get_long("display_errors", &php3_ini.display_errors) == FAILURE) {
			php3_ini.display_errors = 1;
		}
		if (cfg_get_long("log_errors", &php3_ini.log_errors) == FAILURE) {
			php3_ini.log_errors = 0;
		}
		if (cfg_get_long("warn_plus_overloading", &php3_ini.warn_plus_overloading) == FAILURE) {
			php3_ini.warn_plus_overloading = 0;
		}
		if (cfg_get_long("magic_quotes_gpc", &php3_ini.magic_quotes_gpc) == FAILURE) {
			php3_ini.magic_quotes_gpc = MAGIC_QUOTES;
		}
		if (cfg_get_long("magic_quotes_runtime", &php3_ini.magic_quotes_runtime) == FAILURE) {
			php3_ini.magic_quotes_runtime = MAGIC_QUOTES;
		}
		if (cfg_get_long("magic_quotes_sybase", &php3_ini.magic_quotes_sybase) == FAILURE) {
			php3_ini.magic_quotes_sybase = 0;
		}
		if (cfg_get_long("y2k_compliance", &php3_ini.y2k_compliance) == FAILURE) {
			php3_ini.y2k_compliance = 0;
		}
		if (cfg_get_long("define_syslog_variables", &php3_ini.define_syslog_variables) == FAILURE) {
			php3_ini.define_syslog_variables = 0;
		}
		if (cfg_get_string("doc_root", &php3_ini.doc_root) == FAILURE) {
			if ((temp = getenv("PHP_DOCUMENT_ROOT"))) {
				php3_ini.doc_root = temp;
			} else {
				php3_ini.doc_root = NULL;
			}
		}
		if (cfg_get_long("short_open_tag", &php3_ini.short_open_tag) == FAILURE) {
			php3_ini.short_open_tag = DEFAULT_SHORT_OPEN_TAG;
		}
		if (cfg_get_long("asp_tags", &php3_ini.asp_tags) == FAILURE) {
			php3_ini.asp_tags = 0;
		}
		if (cfg_get_string("user_dir", &php3_ini.user_dir) == FAILURE) {
			if ((temp = getenv("PHP_USER_DIR"))) {
				php3_ini.user_dir = temp;
			} else {
				php3_ini.user_dir = NULL;
			}
		}
		if (cfg_get_long("safe_mode", &php3_ini.safe_mode) == FAILURE) {
			php3_ini.safe_mode = PHP_SAFE_MODE;
		}
		if (cfg_get_string("safe_mode_exec_dir", &php3_ini.safe_mode_exec_dir) == FAILURE) {
#ifdef PHP_SAFE_MODE_EXEC_DIR
			php3_ini.safe_mode_exec_dir = PHP_SAFE_MODE_EXEC_DIR;
#else
			php3_ini.safe_mode_exec_dir = "/";
#endif
		}
		if (cfg_get_long("track_vars", &php3_ini.track_vars) == FAILURE) {
			php3_ini.track_vars = PHP_TRACK_VARS;
		}
		if (cfg_get_string("include_path", &php3_ini.include_path) == FAILURE) {
			if ((temp = getenv("PHP_INCLUDE_PATH"))) {
				php3_ini.include_path = temp;
			} else {
				php3_ini.include_path = NULL;
			}
		}
		if (cfg_get_string("charset", &php3_ini.charset) == FAILURE) {
			if ((temp = getenv("PHP_CHARSET"))) {
				php3_ini.charset = temp;
			} else {
				php3_ini.charset = NULL;
			}
		}
		if (cfg_get_string("auto_prepend_file", &php3_ini.auto_prepend_file) == FAILURE) {
			if ((temp = getenv("PHP_AUTO_PREPEND_FILE"))) {
				php3_ini.auto_prepend_file = temp;
			} else {
				php3_ini.auto_prepend_file = NULL;
			}
		}
		if (cfg_get_string("auto_append_file", &php3_ini.auto_append_file) == FAILURE) {
			if ((temp = getenv("PHP_AUTO_APPEND_FILE"))) {
				php3_ini.auto_append_file = temp;
			} else {
				php3_ini.auto_append_file = NULL;
			}
		}
		if (cfg_get_string("upload_tmp_dir", &php3_ini.upload_tmp_dir) == FAILURE) {
			/* php3_ini.upload_tmp_dir = UPLOAD_TMPDIR;*/
			php3_ini.upload_tmp_dir = NULL;
		}
		if (cfg_get_long("upload_max_filesize", &php3_ini.upload_max_filesize) == FAILURE) {
			php3_ini.upload_max_filesize = 2097152;/* 2 Meg default */
		}
		if (cfg_get_string("extension_dir", &php3_ini.extension_dir) == FAILURE) {
			php3_ini.extension_dir = NULL;
		}
		if (cfg_get_long("sql.safe_mode", &php3_ini.sql_safe_mode) == FAILURE) {
			php3_ini.sql_safe_mode = 0;
		}
		/* Syntax highlighting */
		if (cfg_get_string("highlight.comment", &php3_ini.highlight_comment) == FAILURE) {
			php3_ini.highlight_comment = HL_COMMENT_COLOR;
		}
		if (cfg_get_string("highlight.default", &php3_ini.highlight_default) == FAILURE) {
			php3_ini.highlight_default = HL_DEFAULT_COLOR;
		}
		if (cfg_get_string("highlight.html", &php3_ini.highlight_html) == FAILURE) {
			php3_ini.highlight_html = HL_HTML_COLOR;
		}
		if (cfg_get_string("highlight.string", &php3_ini.highlight_string) == FAILURE) {
			php3_ini.highlight_string = HL_STRING_COLOR;
		}
		if (cfg_get_string("highlight.bg", &php3_ini.highlight_bg) == FAILURE) {
			php3_ini.highlight_bg = HL_BG_COLOR;
		}
		if (cfg_get_string("highlight.keyword", &php3_ini.highlight_keyword) == FAILURE) {
			php3_ini.highlight_keyword = HL_KEYWORD_COLOR;
		}
		if (cfg_get_long("engine", &php3_ini.engine) == FAILURE) {
			php3_ini.engine = 1;
		}
		if (cfg_get_long("last_modified", &php3_ini.last_modified) == FAILURE) {
			php3_ini.last_modified = 0;
		}
		if (cfg_get_long("xbithack", &php3_ini.xbithack) == FAILURE) {
			php3_ini.xbithack = 0;
		}
		if (cfg_get_long("expose_php", &php3_ini.expose_php) == FAILURE) {
			php3_ini.expose_php = 1;
		}
		if (cfg_get_string("browscap", &php3_ini.browscap) == FAILURE) {
			php3_ini.browscap = NULL;
		}
		if (cfg_get_string("arg_separator", &php3_ini.arg_separator) == FAILURE) {
			php3_ini.arg_separator = "&";
		}
		if (cfg_get_string("gpc_order", &php3_ini.gpc_order) == FAILURE) {
			php3_ini.gpc_order = "GPC";
		}
		if (cfg_get_string("error_prepend_string", &php3_ini.error_prepend_string) == FAILURE) {
			php3_ini.error_prepend_string = NULL;
		}
		if (cfg_get_string("error_append_string", &php3_ini.error_append_string) == FAILURE) {
			php3_ini.error_append_string = NULL;
		}
		if (cfg_get_string("open_basedir", &php3_ini.open_basedir) == FAILURE) {
			php3_ini.open_basedir = NULL;
		}
		if (cfg_get_long("enable_dl", &php3_ini.enable_dl) == FAILURE) {
			php3_ini.enable_dl = 1;
		}
		if (cfg_get_long("ignore_user_abort", &php3_ini.ignore_user_abort) == FAILURE) {
			php3_ini.ignore_user_abort = 0;
		}
		/* THREADX  Will have to look into this on windows
		 * Make a master copy to use as a basis for every per-dir config.
		 * Without two copies we would have a previous requst's per-dir
		 * config carry forward to the next one.
		 */
		memcpy(&php3_ini_master, &php3_ini, sizeof(php3_ini));
	}
	return SUCCESS;
}

static void php3_config_ini_shutdown(INLINE_TLS_VOID)
{
#if USE_SAPI
	php3_shutdown_config();
#else
	if (GLOBAL(module_initialized) & INIT_CONFIG) {
		php3_shutdown_config();
		GLOBAL(module_initialized) &= ~INIT_CONFIG;
	}
#endif
}

int php3_module_startup(INLINE_TLS_VOID)
{
#if (WIN32|WINNT) && !(USE_SAPI)
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 0);
#else
	if (GLOBAL(module_initialized)) {
		return SUCCESS;
	}
#endif

	start_memory_manager();

#ifdef HAVE_SETLOCALE
	setlocale(LC_CTYPE, "");
#endif

	GLOBAL(error_reporting) = E_ALL;

#if (WIN32|WINNT) && !(USE_SAPI)
	/* start up winsock services */
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		php3_printf("\nwinsock.dll unusable. %d\n", WSAGetLastError());
		return FAILURE;
	}
	GLOBAL(module_initialized) |= INIT_WINSOCK;
#endif

	/* prepare function table hash */
	if (_php3_hash_init(&GLOBAL(function_table), 100, NULL, PVAL_DESTRUCTOR, 1) == FAILURE) {
		php3_printf("Unable to initialize function table.\n");
		return FAILURE;
	}
	GLOBAL(module_initialized) |= INIT_FUNCTION_TABLE;

	/* prepare the module registry */
	if (_php3_hash_init(&GLOBAL(module_registry), 50, NULL, (void (*)(void *)) module_destructor, 1) == FAILURE) {
		php3_printf("Unable to initialize module registry.\n");
		return FAILURE;
	}
	GLOBAL(module_initialized) |= INIT_MODULE_REGISTRY;

	/* resource-list destructors */
	if (_php3_hash_init(&GLOBAL(list_destructors), 50, NULL, NULL, 1) == FAILURE) {
		php3_printf("Unable to initialize resource list destructors hash.\n");
		return FAILURE;
	}
	SET_MUTEX(gLock);
	le_index_ptr = _register_list_destructors(NULL, NULL, 0);
	FREE_MUTEX(gLock);
	GLOBAL(module_initialized) |= INIT_LIST_DESTRUCTORS;

	/* persistent list */
	if (init_resource_plist() == FAILURE) {
		php3_printf("PHP:  Unable to start persistent object list hash.\n");
		return FAILURE;
	}
	GLOBAL(module_initialized) |= INIT_PLIST;

	if (php3_startup_constants()==FAILURE) {
		return FAILURE;
	}
	GLOBAL(module_initialized) |= INIT_CONSTANTS;

	/* We cannot do the config starup until after all the above
	happens, otherwise loading modules from ini file breaks */
#if !USE_SAPI
	if (php3_config_ini_startup(_INLINE_TLS_VOID) == FAILURE) {
		return FAILURE;
	}
#endif	


	if (module_startup_modules() == FAILURE) {
		php3_printf("Unable to start modules\n");
		return FAILURE;
	}
	shutdown_memory_manager();
	return SUCCESS;
}


void php3_module_shutdown_for_exec(void)
{
	/* used to close fd's in the range 3.255 here, but it's problematic */
}

void php3_module_shutdown(INLINE_TLS_VOID)
{
	if (GLOBAL(module_initialized) & INIT_PLIST) {
		destroy_resource_plist();
		GLOBAL(module_initialized) &= ~INIT_PLIST;
	}
	if (GLOBAL(module_initialized) & INIT_LIST_DESTRUCTORS) {
		_php3_hash_destroy(&GLOBAL(list_destructors));
		GLOBAL(module_initialized) &= ~INIT_LIST_DESTRUCTORS;
	}
	if (GLOBAL(module_initialized) & INIT_CONSTANTS) {
		php3_shutdown_constants();
		GLOBAL(module_initialized) &= ~INIT_CONSTANTS;
	}

	/* THIS MUST HAPPEN AFTER THE ABOVE, otherwise it causes crashes
	 * in some modules
	 */
	if (GLOBAL(module_initialized) & INIT_MODULE_REGISTRY) {
		_php3_hash_destroy(&GLOBAL(module_registry));
		GLOBAL(module_initialized) &= ~INIT_MODULE_REGISTRY;
	}
#if !USE_SAPI
	/* close down the ini config */
	php3_config_ini_shutdown(_INLINE_TLS_VOID);
#endif

	if (GLOBAL(module_initialized) & INIT_FUNCTION_TABLE) {
		_php3_hash_destroy(&GLOBAL(function_table));
		GLOBAL(module_initialized) &= ~INIT_FUNCTION_TABLE;
	}
#if (WIN32|WINNT) && !(USE_SAPI)
	/*close winsock */
	if (GLOBAL(module_initialized) & INIT_WINSOCK) {
		WSACleanup();
		GLOBAL(module_initialized) &= ~INIT_WINSOCK;
	}
#endif

	if (GLOBAL(module_initialized)) {
		php3_error(E_WARNING, "Unknown resource in module shutdown");
	}
#if CGI_BINARY
	fflush(stdout);
#endif
#if 0							/* SAPI */
	GLOBAL(sapi_rqst)->flush(GLOBAL(sapi_rqst)->scid);
#endif
}


/* in 3.1 some of this should move into sapi */
int _php3_hash_environment(void)
{
	char **env, *p, *t;
	unsigned char _gpc_flags[3] = {0,0,0};
	pval tmp;
	TLS_VARS;

	p = php3_ini.gpc_order;
	while(*p) {
		switch(*p++) {
			case 'p':
			case 'P':
                              if (!_gpc_flags[0] && php3_headers_unsent() && GLOBAL(request_info).request_method) {
                                      if(!strcasecmp(GLOBAL(request_info).request_method, "post")) {
                                              php3_treat_data(PARSE_POST, NULL);     /* POST Data */
                                      } else {
                                              if(!strcasecmp(GLOBAL(request_info).request_method, "put")) {
                                                      php3_treat_data(PARSE_PUT, NULL);      /* PUT Data */
                                              }
                                      }
					_gpc_flags[0]=1;
				}
				break;
			case 'c':
			case 'C':
				if (!_gpc_flags[1]) {
					php3_treat_data(PARSE_COOKIE, NULL);	/* Cookie Data */
					_gpc_flags[1]=1;
				}
				break;
			case 'g':
			case 'G':
				if (!_gpc_flags[2]) {
					php3_treat_data(PARSE_GET, NULL);	/* GET Data */
					_gpc_flags[2]=1;
				}
				break;
		}
	}

	for (env = environ;env != NULL && *env != NULL;env++) {
		p = strchr(*env, '=');
		if (!p) {				/* malformed entry? */
			continue;
		}
		t = estrndup(*env, p - *env);
		tmp.value.str.len = strlen(p + 1);
		tmp.value.str.val = estrndup(p + 1, tmp.value.str.len);
		tmp.type = IS_STRING;
		/* environmental variables never take precedence over get/post/cookie variables */
		if (_php3_hash_add(&GLOBAL(symbol_table), t, p - *env + 1, &tmp, sizeof(pval), NULL) == FAILURE) {
			efree(tmp.value.str.val);
		}
		efree(t);
	}

#if APACHE
	{
		pval *tmp_ptr, tmp2;
		register int i;
		array_header *arr = table_elts(GLOBAL(php3_rqst)->subprocess_env);
		table_entry *elts = (table_entry *) arr->elts;

		for (i = 0;i < arr->nelts;i++) {
			t = elts[i].key;
			if (elts[i].val) {
				tmp.value.str.len = strlen(elts[i].val);
				tmp.value.str.val = estrndup(elts[i].val, tmp.value.str.len);
			} else {
				tmp.value.str.len = 0;
				tmp.value.str.val = empty_string;
			}
			tmp.type = IS_STRING;
			if (_php3_hash_update(&GLOBAL(symbol_table), t, strlen(t)+1, &tmp, sizeof(pval), NULL) == FAILURE) {
				STR_FREE(tmp.value.str.val);
			}
		}
		/* insert special variables */
		if (_php3_hash_find(&GLOBAL(symbol_table), "SCRIPT_FILENAME", sizeof("SCRIPT_FILENAME"), (void **) & tmp_ptr) == SUCCESS) {
			tmp2 = *tmp_ptr;
			pval_copy_constructor(&tmp2);
			_php3_hash_update(&GLOBAL(symbol_table), "PATH_TRANSLATED", sizeof("PATH_TRANSLATED"), (void *) & tmp2, sizeof(pval), NULL);
		}
		tmp.value.str.len = strlen(GLOBAL(php3_rqst)->uri);
		tmp.value.str.val = estrndup(GLOBAL(php3_rqst)->uri, tmp.value.str.len);
		tmp.type = IS_STRING;
		_php3_hash_update(&GLOBAL(symbol_table), "PHP_SELF", sizeof("PHP_SELF"), (void *) & tmp, sizeof(pval), NULL);
	}
#else
#if FHTTPD
	{
		int i, j;
		if (req) {
			for (i = 0;i < req->nlines;i++) {
				if (req->lines[i].paramc > 1 && req->lines[i].params[0] && req->lines[i].params[1]) {
					tmp.value.str.len = strlen(req->lines[i].params[1]);
					tmp.value.str.val = estrndup(req->lines[i].params[1], tmp.value.str.len);
					tmp.type = IS_STRING;
					if (_php3_hash_update(&GLOBAL(symbol_table), req->lines[i].params[0],
						strlen(req->lines[i].params[0]) + 1,
						&tmp, sizeof(pval), NULL) == FAILURE) {
						efree(tmp.value.str.val);
					}
				}
			}
			if (req->script_name_resolved) {
				i = strlen(req->script_name_resolved);
				tmp.value.str.len = i;
				tmp.value.str.val = estrndup(req->script_name_resolved, i);
				tmp.type = IS_STRING;
				if (_php3_hash_update(&GLOBAL(symbol_table), "PATH_TRANSLATED",
								sizeof("PATH_TRANSLATED"),
								&tmp, sizeof(pval), NULL) == FAILURE) {
					efree(tmp.value.str.val);
				}
				if (req->script_name) {
					j = i - strlen(req->script_name);
					if (j > 0
						&& !strcmp(req->script_name_resolved + j,
								   req->script_name)) {
						tmp.value.str.len = j;
						tmp.value.str.val = estrndup(req->script_name_resolved, j);
						tmp.type = IS_STRING;
						if (_php3_hash_update(&GLOBAL(symbol_table), "DOCUMENT_ROOT",
										sizeof("DOCUMENT_ROOT"),
							   &tmp, sizeof(pval), NULL) == FAILURE) {
							efree(tmp.value.str.val);
						}
					}
				}
			}
		}
	}
#endif
	{
		/* Build the special-case PHP_SELF variable for the CGI version */
		char *pi;
#if FORCE_CGI_REDIRECT
		pi = GLOBAL(request_info).path_info;
		tmp.value.str.val = emalloc(((pi)?strlen(pi):0) + 1);
		tmp.value.str.len = _php3_sprintf(tmp.value.str.val, "%s", (pi ? pi : ""));	/* SAFE */
		tmp.type = IS_STRING;
#else
		int l = 0;
		char *sn;
		sn = GLOBAL(request_info).script_name;
		pi = GLOBAL(request_info).path_info;
		if (sn)
			l += strlen(sn);
		if (pi)
			l += strlen(pi);
		if (pi && sn && !strcmp(pi, sn)) {
			l -= strlen(pi);
			pi = NULL;
		}
		tmp.value.str.val = emalloc(l + 1);
		tmp.value.str.len = _php3_sprintf(tmp.value.str.val, "%s%s", (sn ? sn : ""), (pi ? pi : ""));	/* SAFE */
		tmp.type = IS_STRING;
#endif
		_php3_hash_update(&GLOBAL(symbol_table), "PHP_SELF", sizeof("PHP_SELF"), (void *) & tmp, sizeof(pval), NULL);
	}
#endif


	/* need argc/argv support as well */
	_php3_build_argv(GLOBAL(request_info).query_string);

	GLOBAL(initialized) |= INIT_ENVIRONMENT;

	return SUCCESS;
}

void _php3_build_argv(char *s)
{
	pval arr, tmp;
	int count = 0;
	char *ss, *space;
	TLS_VARS;

	arr.value.ht = (HashTable *) emalloc(sizeof(HashTable));
	if (!arr.value.ht || _php3_hash_init(arr.value.ht, 0, NULL, PVAL_DESTRUCTOR, 0) == FAILURE) {
		php3_error(E_WARNING, "Unable to create argv array");
	} else {
		arr.type = IS_ARRAY;
		_php3_hash_update(&GLOBAL(symbol_table), "argv", sizeof("argv"), &arr, sizeof(pval), NULL);
	}
	/* now pick out individual entries */
	ss = s;
	while (ss) {
		space = strchr(ss, '+');
		if (space) {
			*space = '\0';
		}
		/* auto-type */
		tmp.type = IS_STRING;
		tmp.value.str.len = strlen(ss);
		tmp.value.str.val = estrndup(ss, tmp.value.str.len);
		count++;
		if (_php3_hash_next_index_insert(arr.value.ht, &tmp, sizeof(pval), NULL) == FAILURE) {
			if (tmp.type == IS_STRING)
				efree(tmp.value.str.val);
		}
		if (space) {
			*space = '+';
			ss = space + 1;
		} else {
			ss = space;
		}
	}
	tmp.value.lval = count;
	tmp.type = IS_LONG;
	_php3_hash_add(&GLOBAL(symbol_table), "argc", sizeof("argc"), &tmp, sizeof(pval), NULL);
}

static void php3_parse(FILE * yyin INLINE_TLS)
{
	int original_lineno = GLOBAL(phplineno);
#if 0
	int issock=0, socketd=0;;
#endif

	initialize_input_file_buffer(yyin);
	if (php3_ini.auto_prepend_file && php3_ini.auto_prepend_file[0]) {
		pval tmp;
		
		tmp.value.str.val = php3_ini.auto_prepend_file;
		tmp.value.str.len = strlen(tmp.value.str.val);
		tmp.type = IS_STRING;
		
		include_file(&tmp,0);
		(void) phpparse();
	}
	/* Call Parser on the main input file */
	reset_scanner();
	GLOBAL(phplineno) = original_lineno;
	(void) phpparse();

	if (php3_ini.auto_append_file && php3_ini.auto_append_file[0]) {
		pval tmp;
		
		tmp.value.str.val = php3_ini.auto_append_file;
		tmp.value.str.len = strlen(tmp.value.str.val);
		tmp.type = IS_STRING;
		
		include_file(&tmp,0);
		(void) phpparse();
	}
}


void html_putc(char c)
{
	TLS_VARS;
	switch (c) {
		case '\n':
			PUTS("<br>");
			break;
		case '<':
			PUTS("&lt;");
			break;
		case '>':
			PUTS("&gt;");
			break;
		case '&':
			PUTS("&amp;");
			break;
		case ' ':
			PUTS("&nbsp;");
			break;
		case '\t':
			PUTS("&nbsp;&nbsp;&nbsp;&nbsp;");
			break;
		default:
			PUTC(c);
			break;
	}
}

#if CGI_BINARY

static void _php3_usage(char *argv0)
{
	char *prog;

	prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "php";
	}

	php3_printf("Usage: %s [-q] [-h]"
				" [-s]"
				" [-v] [-i] [-f <file>] | "
				"{<file> [args...]}\n"
				"  -q       Quiet-mode.  Suppress HTTP Header output.\n"
				"  -s       Display colour syntax highlighted source.\n"
				"  -f<file> Parse <file>.  Implies `-q'\n"
				"  -v       Version number\n"
			  "  -p       Pretokenize a script (creates a .php3p file)\n"
				"  -e       Execute a pretokenized (.php3p) script\n"
				"  -c<path> Look for php3.ini file in this directory\n"
				"  -i       PHP information\n"
				"  -h       This help\n", prog);
}

/* some systems are missing these from their header files */
extern char *optarg;
extern int optind;

/**
 * Start my work here.
 *
 **/
#include "klee/klee.h"
#include <assert.h>


#define VALID_FUNCTION (IS_USER_FUNCTION|IS_INTERNAL_FUNCTION)


// separate f
/*
char *phli_map2[33] = { "0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32"};
char *my_map[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
    "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
    "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
    "31", "32", "33", "34", "35", "36", "37", "38", "39", "40",
    "41", "42", "43", "44", "45", "46", "47", "48", "49", "50",
    "51", "52", "53", "54", "55", "56", "57", "58", "59", "60",
    "61", "62", "63", "64", "65", "66", "67", "68", "69", "70",
    "71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
    "81", "82", "83", "84", "85", "86", "87", "88", "89", "90",
    "91", "92", "93", "94", "95", "96", "97", "98", "99", "100", "101"};





// arraies
unsigned char *phli_create_unsigned_char_array(char *name){
    unsigned char *old = (unsigned char *) malloc(phli_MAX_ARRAY_SIZE * sizeof(unsigned char));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(unsigned char), name);
    *(old + phli_MAX_ARRAY_SIZE - 1) = '\0'; // @todo test
    return old;
}

char *phli_create_char_array(char *name){
    char *old = (char *)malloc(phli_MAX_ARRAY_SIZE * sizeof(char));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(char), name);
    *(old + phli_MAX_ARRAY_SIZE - 1) = '\0'; // @todo test
    return old;
}

int *phli_create_int_array(char *name){
    int *old = (int *)malloc(phli_MAX_ARRAY_SIZE * sizeof(int));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(int), name);
    return old;
}

size_t *phli_create_size_t_array(char *name){
    size_t *old = (size_t *)malloc(phli_MAX_ARRAY_SIZE * sizeof(size_t));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(size_t), name);
    return old;
}
unsigned int *phli_create_unsigned_int_array(char *name){
    unsigned int *old = (unsigned int *)malloc(phli_MAX_ARRAY_SIZE * sizeof(unsigned int));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(unsigned int), name);
    return old;
}





// single variable
unsigned char *phli_create_unsigned_char(char *name){
    unsigned char *old = (unsigned char *)malloc(sizeof(unsigned char));
    klee_make_symbolic(old, sizeof(unsigned char), name);
    return old;
}

char *phli_create_char(char *name){
    char *old = (char *)malloc(sizeof(char));
    klee_make_symbolic(old, sizeof(char), name);
    return old;
}

long *phli_create_long(char *name){
    long *old = (long *)malloc(sizeof(long));
    klee_make_symbolic(old, sizeof(long), name);
    return old;
}

int *phli_create_int(char *name){
    int *old = (int *)malloc(sizeof(int));
    klee_make_symbolic(old, sizeof(int), name);
    return old;
}

unsigned int *phli_create_unsignedint(char *name){
    unsigned int *old = (unsigned int *)malloc(sizeof(unsigned int));
    klee_make_symbolic(old, sizeof(unsigned int), name);
    return old;
}

size_t *phli_create_size_t(char *name){
    size_t *old = (size_t *)malloc(sizeof(size_t));
    klee_make_symbolic(old, sizeof(size_t), name);
    return old;
}


uint *phli_create_uint(char *name){
    uint *old = (uint *)malloc(sizeof(uint));
    klee_make_symbolic(old, sizeof(uint), name);
    return old;
}

void phli_print_string(char* array, int len, char *dir){
    if (len == 0){
        klee_print_expr(my_map[0], '\0', dir);
    }
    for(int i = 0; i < len; i ++){
        klee_print_expr(my_map[i], array[i], dir);
    }
}

void phli_print_int(int *val, char *dir){
    klee_print_expr("int", *val, dir);
}

void phli_print_long2int(long *val, char *dir){
    klee_print_expr("long_int", *(int*)val, dir);
    klee_print_expr("long_int_1", *((int*)val + 1), dir);
}

void phli_print_long(long *val, char *dir){
    klee_print_expr("long", *val, dir);
}

pval *phli_construct_pval_long(char *name){
    pval *re = (pval *)malloc(phli_PVAL_SIZE);
    klee_make_symbolic(re, phli_PVAL_SIZE, name);// 24 bytes
    re->type = IS_LONG;
    char *value_name = (char *)malloc(40);
    strcpy(value_name, name);
    long *tmp = phli_create_long(strcat(value_name, "long_lval"));
    re->value.lval = *tmp;
    free(value_name);
    return re;
}

pval *phli_construct_pval_double(char *name){

    pval *re = (pval *)malloc(phli_PVAL_SIZE);
    klee_make_symbolic(re, phli_PVAL_SIZE, name);// 24 bytes
    re->type = IS_DOUBLE;
    char *value_name = (char *)malloc(40);
    strcpy(value_name, name);
    //re->value.lval = phli_create_double(strcat(value_name, "long_lval"))
    free(value_name);
    return re;
}
pval *phli_construct_pval_string(char *name){

    pval *re = (pval *)malloc(phli_PVAL_SIZE);
    klee_make_symbolic(re, phli_PVAL_SIZE, name);// 24 bytes
    re->type = IS_STRING;// @todo
    char *value_name = (char *)malloc(40);
    strcpy(value_name, name);
    re->value.str.val =  phli_create_char_array(strcat(value_name, "str_val"));
    free(value_name);
    re->value.str.len =  strlen(re->value.str.val);
    return re;
}


void phli_construct_HashTable(HashTable ht){
}

void phli_construct_control_structure_data(control_structure_data re){
}
*/



unsigned char *php_bin2hex(unsigned char*, size_t, size_t*);
/*
int main(int argc, char *argv[]) { 

    char result_dir[] = "/data/phli/results/php_bin2hex/";
    unsigned char* arg_0 = phli_create_unsigned_char_array("arg_0");
	size_t* arg_1 = phli_create_size_t("arg_1");
	size_t* arg_2 = phli_create_size_t_array("arg_2");
    klee_assume(*arg_1 < phli_MAX_ARRAY_SIZE);
	unsigned char* phli_result;
	phli_result = php_bin2hex(arg_0, *arg_1, arg_2);
    phli_print_string(arg_0, (int)arg_1*2 - 1, result_dir);

}
*/

char* _php3_memnstr (char*, char*, int, char*);
/*
int main(int argc, char *argv[]) { 

    char result_dir[] = "/data/phli/results/_php3_memnstr/";
	char* arg_0 = phli_create_char_array("arg_0");
	char* arg_1 = phli_create_char_array("arg_1");
	int* arg_2 = phli_create_int("arg_2");
	char* arg_3 = phli_create_char_array("arg_3");
	char* phli_result;
	phli_result = _php3_memnstr(arg_0, arg_1, *arg_2, arg_3);
    phli_print_string(arg_0, phli_MAX_ARRAY_SIZE, result_dir);
}
*/

// for testing only
/*
int main(int argc, char* argv[]){
    char * arg_0 = phli_create_char_array("arg_0");
    char test[200];
    int len = strlen(arg_0);
    klee_assume(len < 10);
    memcpy(test, arg_0, len);
	char *result_dir = "/data/phli/results/tmp/";
    phli_print_string(test, len, result_dir);
}
*/

//void php3_substr(INTERNAL_FUNCTION_PARAMETERS);
void php3_substr(pval *, pval *, pval *, pval*);
/*
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php3_substr/";
    pval *arg_0 = phli_construct_pval_string("arg_0");
    pval *arg_1 = phli_construct_pval_long("arg_1");
    pval *arg_2 = phli_construct_pval_long("arg_2");
    pval *arg_3 = phli_construct_pval_string("arg_3");
    php3_substr(arg_0, arg_1, arg_2, arg_3);
    //phli_print_string(arg_3->value.str.val, arg_3->value.str.len, result_dir);
    phli_print_int(&arg_3->value.str.len, result_dir);
}
*/



/*
int main(int argc, char *argv[])
{
    char result_dir[] = "/data/phli/results/";
    unsigned char *old = (unsigned char *)malloc(MAX_STRING_VAL_LEN);
    klee_make_symbolic(old, MAX_STRING_VAL_LEN,  "arg1");
    size_t oldlen;
    klee_make_symbolic(&oldlen, sizeof(size_t), "arg2");
    klee_assume(oldlen < 10);
    klee_assume(oldlen > 1);
    size_t newlen;
    klee_make_symbolic(&newlen, sizeof(size_t), "arg3");
    char *rr = NULL;
    rr = php_bin2hex(old, oldlen, &newlen);
    //printf("outside %lx\n", rr);
    //printf("sizeof %d\n", sizeof(rr));
    char filename[50];
    for(int i = 0;i < oldlen * 2;i ++){
        strcpy(filename, result_dir);
        klee_print_expr(my_map[i], rr[i], result_dir);
    }
}
*/

#endif							/* CGI_BINARY */


#if APACHE
PHPAPI int apache_php3_module_main(request_rec * r, int fd, int display_source_mode, int preprocessed)
{
	FILE *in = NULL;
	TLS_VARS;

	GLOBAL(php3_rqst) = r;

	if (php3_request_startup(_INLINE_TLS_VOID) == FAILURE) {
		return FAILURE;
	}
	php3_TreatHeaders();
	in = fdopen(fd, "r");
	if (in) {
		GLOBAL(phpin) = in;
		phprestart(GLOBAL(phpin));
		GLOBAL(initialized) |= INIT_SCANNER;
		_php3_hash_index_update(&GLOBAL(include_names), 0, (void *) &GLOBAL(request_info).filename, sizeof(char *), NULL);
	} else {
		return OK;
	}
	if (display_source_mode) {
		GLOBAL(Execute) = 0;
		GLOBAL(ExecuteFlag) = DONT_EXECUTE;
		GLOBAL(php3_display_source) = 1;
		if (!php3_header())
			return (OK);
		PUTS("<html><head><title>Source for ");
		PUTS(r->uri);
		PUTS("</title></head><body bgcolor=\"");
		PUTS(php3_ini.highlight_bg);
		PUTS("\" text=\"");
		PUTS(php3_ini.highlight_html);
		PUTS("\">\n");			/* color: seashell */
	}
	if (preprocessed) {
		if (tcm_load(&GLOBAL(token_cache_manager))==FAILURE) {
			return OK;
		}
	}

	(void) php3_parse(GLOBAL(phpin));
	
	if (GLOBAL(php3_display_source)) {
		php3_printf("\n</html>\n");
	}

	if (GLOBAL(initialized)) {
		php3_header();			/* Make sure headers have been sent */
	}
	return (OK);
}
#endif							/* APACHE */

#if FHTTPD

char *get_pretokenized_name(void)
{
	char *pretokenized_name = NULL;

	if (GLOBAL(request_info).filename) {
		int length = strlen(GLOBAL(request_info).filename);

		if (length > (sizeof(".php3") - 1) && !strcmp(GLOBAL(request_info).filename + length - sizeof(".php3") + 1, ".php3")) {
			pretokenized_name = (char *) emalloc(length + 2);
			strcpy(pretokenized_name, GLOBAL(request_info).filename);
			strcat(pretokenized_name, "p");
		} else {
			length += sizeof(".php3p");
			pretokenized_name = (char *) emalloc(length + 1);
			strcpy(pretokenized_name, GLOBAL(request_info).filename);
			strcat(pretokenized_name, ".php3p");
		}
	} else {
		pretokenized_name = estrdup("stdin.php3p");
	}
	return pretokenized_name;
}


void _php3_usage(char *progname)
{
	fprintf(stderr,
	   "Usage: %s [options] [appname] [username] [hostname] [portname]\n"
			"Options:\n"
			"  -d       Daemon mode -- never attempt terminal I/O\n"
			"  -s       Socket mode, fhttpd internal use only\n"
			"  -p       Pipe mode, fhttpd internal use only\n"
			"  -u<mask> Set umask\n"
			"  -t<time> Idle timeout in seconds, 0 - disable\n"
			"  -S       Display colour syntax highlighted source\n"
			"  -P       Make and execute a pretokenized script\n"
			"           (.php3p file) or, if pretokenized script, newer\n"
			"           than original file exists, execute it instead\n"
			"  -E       Execute a pretokenized (.php3p) script\n"
			"  -c<path> Look for php3.ini file in this directory\n"
			"           (must appear before any other options)\n"
			"  -v       Version number\n"
			"  -h       This help\n",
			progname);
}

int main(int argc, char **argv)
{
	int c, i, processing_error;
	FILE *in = NULL;
	FILE *in2;
	int display_source_mode = 0;
	int preprocess_mode = PREPROCESS_NONE;
	int argc1;
	char **argv1;
	int human = 1, fd2;
	int i0 = 0, i1 = 0;
	char *pn;
	struct stat statbuf, pstatbuf;

#ifdef HAVE_SETLOCALE
	setlocale(LC_CTYPE, "");
#endif

	if (php3_module_startup(_INLINE_TLS_VOID) == FAILURE) {
		return FAILURE;
	}
	signal(SIGPIPE, SIG_IGN);
	signal(SIGFPE, SIG_IGN);
	umask(077);

	while ((c = getopt(argc, argv, "spdu:t:c:PESvh")) != -1) {
		switch (c) {
			case 'd':
				human = 0;
				break;
			case 's':
				i0 = 1;
				break;
			case 'p':
				i1 = 1;
				break;
			case 'u':
				if (*optarg == '0')
					umask(strtoul(optarg, NULL, 8));
				else
					umask(strtoul(optarg, NULL, 10));
				break;
			case 't':
				idle_timeout = atoi(optarg);
				break;
			case 'c':
				GLOBAL(php3_ini_path) = strdup(optarg);		/* intentional leak */
				break;
			case 'P':			/* preprocess */
				preprocess_mode = PREPROCESS_PREPROCESS;
				break;
			case 'E':			/* execute preprocessed script */
				preprocess_mode = PREPROCESS_EXECUTE;
				break;
			case 'S':
				display_source_mode = 1;
				break;
			case 'v':
				printf("%s\n", PHP_VERSION);
				exit(1);
				break;
			case 'h':
			case ':':
			case '?':
				_php3_usage(argv[0]);
				return -1;
		}
	}

	argc1 = argc - optind;
	argv1 = (char **) malloc(sizeof(char *) * (argc1 + 2));
	if (!argv1)
		return -1;
	argv1 += 2;
	for (i = optind;i < argc;i++)
		argv1[i - optind] = argv[i];

	if (i0) {
		argv1--;
		*argv1 = "-s";
		argc1++;
	} else {
		if (i1) {
			argv1--;
			*argv1 = "-p";
			argc1++;
		}
	}
	argv1--;
	argc1++;
	*argv1 = *argv;

	server = createserver();
	if (!server)
		return -1;

	switch (servproc_init(server, human, argc1, argv1)) {
		case 0:
			break;
		case APP_ERR_HUMAN:
			_php3_usage(argv[0]);
			exit(1);
			break;
		case APP_ERR_CONFIG:
			fprintf(stderr, "%s: configuration error\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_READ:
			fprintf(stderr, "%s: read error\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_HOSTNAME:
			fprintf(stderr, "%s: can't resolve server hostname\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_SOCKET:
			fprintf(stderr, "%s: can't create socket\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_CONNECT:
			fprintf(stderr, "%s: can't connect\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_APPCONNECT:
			fprintf(stderr, "%s: connect error\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_USER:
			fprintf(stderr, "%s: login error\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_PASSWORD:
			fprintf(stderr, "%s: login error\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_APPLICATION:
			fprintf(stderr, "%s: application rejected by server\n", server->app_progname);
			exit(1);
			break;
		case APP_ERR_INSANE:
		case APP_ERR_DAEMON:
		case APP_ERR_AUTH:
		default:
			if (server->infd < 0)
				exit(1);
	}

	if (server->infd == 0 && server->outfd == 1) {
		close(2);
		fd2 = open("/dev/null", O_WRONLY);
		if (fd2 != 2) {
			dup2(fd2, 2);
			close(fd2);
		}
	}
	setcapabilities(server, APP_CAP_KEEPALIVE);

	exit_status = 0;
	while (!exit_status) {
		processing_error = 0;
		if (php3_request_startup(_INLINE_TLS_VOID) == FAILURE) {
			processing_error = 1;
		}
		if (!processing_error) {
			GLOBAL(phpin) = NULL;
			GLOBAL(current_lineno) = 0;

			php3_TreatHeaders();

			in = php3_fopen_for_parser();

			GLOBAL(php3_preprocess) = preprocess_mode;

			if (!in) {
				if (php3_header())
					PUTS("No input file specified.\n");
				php3_request_shutdown((void *) 0 _INLINE_TLS);
				processing_error = 1;
			} else {
				if (GLOBAL(php3_preprocess) == PREPROCESS_PREPROCESS) {
					pn = get_pretokenized_name();
					if (pn) {
						if (!stat(pn, &pstatbuf)
							&& !fstat(fileno(in), &statbuf)
							&& S_ISREG(pstatbuf.st_mode)
							&& statbuf.st_mtime < pstatbuf.st_mtime) {
							in2 = fopen(pn, "r");
							if (in2) {
								fclose(in);
								in = in2;
								GLOBAL(php3_preprocess) = PREPROCESS_EXECUTE;
							}
						}
						efree(pn);
					}
				}
				if (GLOBAL(php3_preprocess) != PREPROCESS_EXECUTE) {
					/* #!php support */
					c = fgetc(in);
					if (c == '#') {
						while (c != 10 && c != 13) {
							c = fgetc(in);	/* skip to end of line */
						}
						GLOBAL(phplineno)++;
					} else {
						rewind(in);
					}
				}
				GLOBAL(phpin) = in;
				GLOBAL(initialized) |= INIT_SCANNER;
				phprestart(GLOBAL(phpin));

				if (display_source_mode) {
					GLOBAL(Execute) = 0;
					GLOBAL(ExecuteFlag) = DONT_EXECUTE;
					GLOBAL(php3_display_source) = 1;
					if (php3_header()) {
						PUTS("<html><head><title>Source for ");
						PUTS(GLOBAL(request_info).filename);
						PUTS("</title></head><body bgcolor=\"");
						PUTS(php3_ini.highlight_bg);
						PUTS("\" text=\"");
						PUTS(php3_ini.highlight_html);
						PUTS("\">\n");	/* color: seashell */
					} else {
						processing_error = 1;
					}
				}
				if (GLOBAL(php3_display_source) && GLOBAL(php3_preprocess) == PREPROCESS_PREPROCESS) {
					php3_printf("Can't preprocess while displaying source.<br>\n");
					processing_error = 1;
				}
				if (!processing_error) {
					if (GLOBAL(php3_preprocess) == PREPROCESS_EXECUTE) {
						if (tcm_load(&GLOBAL(token_cache_manager), GLOBAL(phpin))==FAILURE) {
							/* should bail out on an error, don't know how to do it in fhttpd */
						}
						GLOBAL(php3_preprocess) = PREPROCESS_NONE;
					}
					if (GLOBAL(php3_preprocess)!=PREPROCESS_NONE) {
						pval yylval;

						while (phplex(&yylval));	/* create the token cache */
						tcm_save(&GLOBAL(token_cache_manager));
						seek_token(&GLOBAL(token_cache_manager), 0, NULL);
						GLOBAL(php3_preprocess) = PREPROCESS_NONE;
					}
					php3_parse(GLOBAL(phpin));

					if (GLOBAL(php3_display_source)) {
						php3_printf("\n</html>\n");
					}
				}
			}
		}
		if (GLOBAL(initialized)) {
			php3_header();		/* Make sure headers have been sent */
			php3_request_shutdown((void *) 0 _INLINE_TLS);
		}
	}
	php3_module_shutdown(_INLINE_TLS_VOID);
	return 0;
}
#endif							/* FHTTPD */

#if USE_SAPI

PHPAPI int php3_sapi_main(struct sapi_request_info *sapi_info)
{
#if DEBUG
	char logmessage[1024];
#endif
	FILE *in = NULL;
	int c;
	YY_TLS_VARS;
	TLS_VARS;

	GLOBAL(php3_preprocess) = sapi_info->preprocess;
	GLOBAL(php3_display_source) = sapi_info->display_source_mode;
	GLOBAL(sapi_rqst) = sapi_info;

#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: entry\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif

/*	if (php3_module_startup(php3_globals) == FAILURE) {
		return FAILURE;
	}*/

	if (php3_request_startup(_INLINE_TLS_VOID) == FAILURE) {
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: request starup failed\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		return FAILURE;
	}
	if (sapi_info->preprocess == PREPROCESS_PREPROCESS || sapi_info->quiet_mode) {
		php3_noheader();
	}
	if (sapi_info->info_only) {
		_php3_info();
		php3_request_shutdown((void *) GLOBAL(sapi_rqst), php3_globals);
		/*php3_module_shutdown(php3_globals);*/
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: info_only\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		return (1);
	}
	/* if its not cgi, require that we have a filename now */
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: File: %s\n",GLOBAL(sapi_rqst)->scid,GLOBAL(sapi_rqst)->filename);
	OutputDebugString(logmessage);
#endif
	if (!sapi_info->cgi && !sapi_info->filename) {
		php3_printf("No input file specified.\n");
		php3_request_shutdown((void *) GLOBAL(sapi_rqst), php3_globals);
		/*php3_module_shutdown(php3_globals);*/
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: No input file specified\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		return FAILURE;
	}
	/* 
	   if request_info.filename is null and cgi, fopen_for_parser is responsible
	   request_info.filename will only be estrduped in fopen_for parser
	   if it is null at this point
	 */
	in = php3_fopen_for_parser();

	if (sapi_info->cgi && !in) {
		php3_printf("No input file specified for cgi.\n");
		php3_request_shutdown((void *) GLOBAL(sapi_rqst), php3_globals);
		/*php3_module_shutdown(php3_globals);*/
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: No input file specified for cgi.\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		return FAILURE;
	}
	if (sapi_info->cgi && in) {
		/* #!php support */
		c = fgetc(in);
		if (c == '#') {
			while (c != 10 && c != 13) {
				c = fgetc(in);	/* skip to end of line */
			}
		} else {
			rewind(in);
		}
	}
	if (in) {
		GLOBAL(phpin) = in;
		phprestart(GLOBAL(phpin));
		GLOBAL(initialized) |= INIT_SCANNER;
	}
	if (sapi_info->display_source_mode) {
		GLOBAL(Execute) = 0;
		GLOBAL(ExecuteFlag) = DONT_EXECUTE;
		GLOBAL(php3_display_source) = 1;
		if (!php3_header())
			exit(0);
		PUTS("<html><head><title>Source for ");
		PUTS(sapi_info->filename);
		PUTS("</title></head><body bgcolor=\"");
		PUTS(php3_ini.highlight_bg);
		PUTS("\" text=\"");
		PUTS(php3_ini.highlight_html);
		PUTS("\">\n");			/* color: seashell */
	}
	if (sapi_info->display_source_mode && sapi_info->preprocess == PREPROCESS_PREPROCESS) {
		php3_printf("Can't preprocess while displaying source.<br>\n");
		return FAILURE;
	}
	if (sapi_info->preprocess == PREPROCESS_EXECUTE) {
		tcm_load(&GLOBAL(token_cache_manager));
		GLOBAL(php3_preprocess) = PREPROCESS_NONE;
	}
	if (sapi_info->preprocess==PREPROCESS_NONE) {
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: start php3_parse() file:%s\n",GLOBAL(sapi_rqst)->scid,GLOBAL(sapi_rqst)->filename);
	OutputDebugString(logmessage);
#endif
		php3_parse(GLOBAL(phpin) _INLINE_TLS);
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: done php3_parse()\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
	} else {
		pval yylval;

#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: entering phplex()\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		while (phplex(&yylval));	/* create the token cache */
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: done phplex()\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		tcm_save(&GLOBAL(token_cache_manager));
	}

	if (sapi_info->display_source_mode) {
		php3_printf("\n</html>\n");
	}
	if (GLOBAL(initialized)) {
		php3_header();			/* Make sure headers have been sent */
		php3_request_shutdown((void *) GLOBAL(sapi_rqst), php3_globals);
		/*php3_module_shutdown(php3_globals);*/
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: success!\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		return SUCCESS;
	} else {
#if DEBUG
	snprintf(logmessage,1024,"%d:php3_sapi_main: request not initialized!\n",GLOBAL(sapi_rqst)->scid);
	OutputDebugString(logmessage);
#endif
		return FAILURE;
	}
}
#if WIN32|WINNT
extern int tls_create(void);
extern int tls_destroy(void);
extern int tls_startup(void);
extern int tls_shutdown(void);
extern flex_globals *yy_init_tls(void);
extern void yy_destroy_tls(void);
extern VOID ErrorExit(LPTSTR lpszMessage);

BOOL WINAPI DllMain(HANDLE hModule,
					DWORD ul_reason_for_call,
					LPVOID lpReserved)
{
	php3_globals_struct *php3_globals;
#if DEBUG
	OutputDebugString("PHP_Core DllMain Entry\n");
#endif
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			/* 
			   I should be loading ini vars here 
			   and doing whatever true global inits
			   need to be done
			 */
			_fmode = _O_BINARY;	/*sets default for file streams to binary */
			/* make the stdio mode be binary */
			setmode(_fileno(stdin), O_BINARY);
			setmode(_fileno(stdout), O_BINARY);
			setmode(_fileno(stderr), O_BINARY);
			setlocale(LC_CTYPE, "");

			CREATE_MUTEX(gLock, "GENERAL");

			if (!tls_startup())
				return 0;
			if (!tls_create())
				return 0;
			php3_globals = TlsGetValue(TlsIndex);
			yy_init_tls();
			if (php3_config_ini_startup(_INLINE_TLS_VOID) == FAILURE) {
				return 0;
			}
			if (php3_module_startup(php3_globals) == FAILURE) {
				ErrorExit("module startup failed");
				return 0;
			}

#if DEBUG
	OutputDebugString("PHP_Core DllMain Process Attached\n");
#endif
			break;
		case DLL_THREAD_ATTACH:
#if DEBUG
	OutputDebugString("PHP_Core DllMain Thread Attach\n");
#endif
			if (!tls_create())
				return 0;
			php3_globals = TlsGetValue(TlsIndex);
			yy_init_tls();
			if (php3_module_startup(php3_globals) == FAILURE) {
				ErrorExit("module startup failed");
#if DEBUG
	OutputDebugString("PHP_Core DllMain module startup failed\n");
#endif
				return 0;
			}
			break;
		case DLL_THREAD_DETACH:
#if DEBUG
	OutputDebugString("PHP_Core DllMain Detache\n");
#endif
			php3_globals = TlsGetValue(TlsIndex);
			php3_module_shutdown(php3_globals);
			if (!tls_destroy())
#if DEBUG
	OutputDebugString("PHP_Core DllMain Detache Error tls_destroy\n");
#endif
				return 0;
			yy_destroy_tls();
			break;
		case DLL_PROCESS_DETACH:
			/*
			   close down anything down in process_attach 
			 */
			php3_globals = TlsGetValue(TlsIndex);
			php3_module_shutdown(php3_globals);

			php3_config_ini_shutdown(_INLINE_TLS_VOID);

			if (!tls_destroy())
#if DEBUG
	OutputDebugString("PHP_Core DllMain tls_destroy failed\n");
#endif
				return 0;
			if (!tls_shutdown())
#if DEBUG
	OutputDebugString("PHP_Core DllMain tls_shutdown failed\n");
#endif
				return 0;
			yy_destroy_tls();
#if DEBUG
	OutputDebugString("PHP_Core DllMain Process Detatched\n");
#endif
			break;
	}
#if DEBUG
	OutputDebugString("PHP_Core DllMain Successful Exit\n");
#endif
	return 1;
}

#endif
#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
