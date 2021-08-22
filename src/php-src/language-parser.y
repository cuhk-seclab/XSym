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
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/


/* $Id: language-parser.y,v 1.186 2000/08/11 22:24:55 martin Exp $ */


/* 
 * LALR shift/reduce conflicts and how they are resolved:
 *
 * - 2 shift/reduce conflicts due to the dangeling elseif/else ambiguity.  Solved by shift.
 * - 1 shift/reduce conflict due to arrays within encapsulated strings. Solved by shift. 
 * - 1 shift/reduce conflict due to objects within encapsulated strings.  Solved by shift.
 * - 1 shift/reduce conflict due to while loop nesting ambiguity.
 * - 1 shift/reduce conflict due to for loop nesting ambiguity.
 * 
 */

extern char *phptext;
extern int phpleng;
extern int wanted_exit_status;
#define YY_TLS_VARS
#include "php.h"
#include "php3_list.h"
#include "control_structures.h"


#include "main.h"
#include "functions/head.h"


#define YYERROR_VERBOSE

HashTable symbol_table;		/* main symbol table */
HashTable function_table;	/* function symbol table */
HashTable include_names;	/* included file names are stored here, for error messages */
TokenCacheManager token_cache_manager;
Stack css;					/* Control Structure Stack, used to store execution states */
Stack for_stack;			/* For Stack, used in order to determine
							 * whether we're in the first iteration of 
							 * a for loop or not
							 */
Stack input_source_stack;		/* Include Stack, used to keep track of
							 * of the file pointers when include()'ing
							 * files.
							 */
Stack function_state_stack;	/* Function State Stack, used to keep inforation
							 * about the function call, such as its
							 * loop nesting level, symbol table, etc.
							 */
Stack switch_stack;			/* Switch stack, used by the switch() control
							 * to determine whether a case was already
							 * matched.
							 */

Stack variable_unassign_stack; /* Stack used to unassign variables that weren't properly defined */

HashTable *active_symbol_table;  /* this always points to the
								  * currently active symbol
								  * table.
								  */

int Execute;	/* This flag is used by all parts of the program, in order
				 * to determine whether or not execution should take place.
				 * It's value is dependant on the value of the ExecuteFlag,
				 * the loop_change_level (if we're during a break or 
				 * continue) and the function_state.returned (whether
				 * our current function has been return()ed from).
				 * In order to restore the correct value, after changing
				 * one of the above (e.g. when popping the css),
				 * one should use the macro Execute = SHOULD_EXECUTE;
				 */
int ExecuteFlag;
int current_lineno;		/* Current line number, broken with include()s */
int include_count;

/* This structure maintains information about the current active scope.
 * Kept are the loop nesting level, information about break's and continue's
 * we might be in, whether or not our current function has been return()'d
 * from, and also, a pointer to the active symbol table.  During variable
 * -passing in a function call, it also contains a pointer to the
 * soon-to-be-called function symbol table.
 */
FunctionState function_state;
FunctionState php3g_function_state_for_require; /* to save state when forcing execution in require() evaluation */

/* The following two variables are used when inside class definitions. */
char *class_name=NULL;
HashTable *class_symbol_table=NULL;

/* Variables used in function calls */
pval globals;
unsigned int param_index;  /* used during function calls */

/* Temporary variable used for multi dimensional arrays */
pval *array_ptr;

extern int shutdown_requested;

#include "control_structures_inline.h"

HashTable list, plist;

int init_resource_list(void)
{
	TLS_VARS;
	
	return _php3_hash_init(&GLOBAL(list), 0, NULL, list_entry_destructor, 0);
}

int init_resource_plist(void)
{
	TLS_VARS;
	
	return _php3_hash_init(&GLOBAL(plist), 0, NULL, plist_entry_destructor, 1);
}

void destroy_resource_list(void)
{
	TLS_VARS;
	
	_php3_hash_destroy(&GLOBAL(list));
}

void destroy_resource_plist(void)
{
	TLS_VARS;
	
	_php3_hash_destroy(&GLOBAL(plist));
}

int clean_module_resource(list_entry *le, int *resource_id)
{
	if (le->type == *resource_id) {
#if DEBUG
		printf("Cleaning resource %d\n",*resource_id);
#endif
		return 1;
	} else {
		return 0;
	}
}


int clean_module_resource_destructors(list_destructors_entry *ld, int *module_number)
{
	TLS_VARS;
	if (ld->module_number == *module_number) {
		_php3_hash_apply_with_argument(&GLOBAL(plist), (int (*)(void *,void *)) clean_module_resource, (void *) &(ld->resource_id));
		return 1;
	} else {
		return 0;
	}
}

%}

%pure_parser
%expect 6

%left ','
%left LOGICAL_OR
%left LOGICAL_XOR
%left LOGICAL_AND
%right PHP_PRINT
%left '=' PLUS_EQUAL MINUS_EQUAL MUL_EQUAL DIV_EQUAL CONCAT_EQUAL MOD_EQUAL AND_EQUAL OR_EQUAL XOR_EQUAL SHIFT_LEFT_EQUAL SHIFT_RIGHT_EQUAL
%left '?' ':'
%left BOOLEAN_OR
%left BOOLEAN_AND
%left '|'
%left '^'
%left '&'
%nonassoc IS_EQUAL IS_NOT_EQUAL
%nonassoc '<' IS_SMALLER_OR_EQUAL '>' IS_GREATER_OR_EQUAL
%left SHIFT_LEFT SHIFT_RIGHT
%left '+' '-' '.'
%left '*' '/' '%'
%right '!' '~' INCREMENT DECREMENT INT_CAST DOUBLE_CAST STRING_CAST ARRAY_CAST OBJECT_CAST '@'
%right '['
%nonassoc NEW
%token EXIT
%token IF
%left ELSEIF
%left ELSE
%left ENDIF
%token LNUMBER
%token DNUMBER
%token STRING
%token NUM_STRING
%token INLINE_HTML
%token CHARACTER
%token BAD_CHARACTER
%token ENCAPSED_AND_WHITESPACE
%token PHP_ECHO
%token DO
%token WHILE
%token ENDWHILE
%token FOR
%token ENDFOR
%token SWITCH
%token ENDSWITCH
%token CASE
%token DEFAULT
%token BREAK
%token CONTINUE
%token CONTINUED_WHILE
%token CONTINUED_DOWHILE
%token CONTINUED_FOR
%token OLD_FUNCTION
%token FUNCTION
%token IC_FUNCTION
%token PHP_CONST
%token RETURN
%token INCLUDE
%token REQUIRE
%token HIGHLIGHT_FILE
%token HIGHLIGHT_STRING
%token PHP_GLOBAL
%token PHP_STATIC
%token PHP_UNSET
%token PHP_ISSET
%token PHP_EMPTY
%token CLASS
%token EXTENDS
%token PHP_CLASS_OPERATOR
%token PHP_DOUBLE_ARROW
%token PHP_LIST
%token PHP_ARRAY
%token VAR
%token EVAL
%token DONE_EVAL
%token PHP_LINE
%token PHP_FILE
%token STRING_CONSTANT

%% /* Rules */

statement_list:	
		statement_list statement
	|	/* empty */
;


statement:
		'{' statement_list '}'
	|	IF '(' expr ')' { cs_start_if (&$3 _INLINE_TLS); } statement elseif_list else_single { cs_end_if ( _INLINE_TLS_VOID); }
	|	IF '(' expr ')' ':' { cs_start_if (&$3 _INLINE_TLS); } statement_list new_elseif_list new_else_single { cs_end_if ( _INLINE_TLS_VOID); } ENDIF ';'
	|	WHILE '(' expr ')' { cs_start_while(&$1, &$3 _INLINE_TLS); } while_statement { cs_end_while(&$1,&yychar _INLINE_TLS); } while_iterations
	|	DO { cs_start_do_while(&$1 _INLINE_TLS); } statement WHILE { cs_force_eval_do_while( _INLINE_TLS_VOID); } '(' expr ')' ';'{ cs_end_do_while(&$1,&$7,&yychar _INLINE_TLS); } do_while_iterations 
	|	FOR { for_pre_expr1(&$1 _INLINE_TLS); }
			'(' for_expr { if (GLOBAL(Execute)) pval_destructor(&$4 _INLINE_TLS); for_pre_expr2(&$1 _INLINE_TLS); }
			';' for_expr { for_pre_expr3(&$1,&$7 _INLINE_TLS); }
			';' for_expr { for_pre_statement(&$1,&$7,&$10 _INLINE_TLS); }
			')' for_statement { for_post_statement(&$1,&$6,&$9,&$12,&yychar _INLINE_TLS); } for_iterations
	|	SWITCH '(' expr ')' { cs_switch_start(&$1,&$3 _INLINE_TLS); } switch_case_list { cs_switch_end(&$3 _INLINE_TLS); }
	|	BREAK ';' { DO_OR_DIE(cs_break_continue(NULL,DO_BREAK _INLINE_TLS)) }	
	|	BREAK expr ';' { DO_OR_DIE(cs_break_continue(&$2,DO_BREAK _INLINE_TLS)) }
	|	CONTINUE ';' { DO_OR_DIE(cs_break_continue(NULL,DO_CONTINUE _INLINE_TLS)) }
	|	CONTINUE expr ';' { DO_OR_DIE(cs_break_continue(&$2,DO_CONTINUE _INLINE_TLS)) }
	|	OLD_FUNCTION STRING { start_function_decleration(_INLINE_TLS_VOID); } 
			parameter_list '(' statement_list ')' ';' { end_function_decleration(&$1,&$2 _INLINE_TLS); }
	|	FUNCTION STRING '(' { start_function_decleration(_INLINE_TLS_VOID); }
			parameter_list ')' '{' statement_list '}' { end_function_decleration(&$1,&$2 _INLINE_TLS); }
	|	IC_FUNCTION STRING { tc_set_token(&GLOBAL(token_cache_manager), $1.offset, FUNCTION); } '(' parameter_list ')' '{' statement_list '}' { if (!GLOBAL(shutdown_requested)) GLOBAL(shutdown_requested) = TERMINATE_CURRENT_PHPPARSE; }
	|	RETURN ';'   { cs_return(NULL _INLINE_TLS); }
	|	RETURN expr ';'   { cs_return(&$2 _INLINE_TLS); }
	|	PHP_GLOBAL global_var_list
	|	PHP_STATIC static_var_list
	|	CLASS STRING { cs_start_class_decleration(&$2,NULL _INLINE_TLS); } '{' class_statement_list '}' { cs_end_class_decleration( _INLINE_TLS_VOID); }
	|	CLASS STRING EXTENDS STRING { cs_start_class_decleration(&$2,&$4 _INLINE_TLS); } '{' class_statement_list '}' { cs_end_class_decleration( _INLINE_TLS_VOID); }
	|	PHP_ECHO echo_expr_list ';'
	|	INLINE_HTML { if (GLOBAL(Execute)) { if (php3_header()) PUTS($1.value.str.val); } }
	|	expr ';'  { if (GLOBAL(Execute)) pval_destructor(&$1 _INLINE_TLS); }
	|	REQUIRE { php3cs_start_require(&$1 _INLINE_TLS); } expr ';' { php3cs_end_require(&$1,&$3 _INLINE_TLS); }
	|	HIGHLIGHT_FILE expr ';' { if (GLOBAL(Execute)) cs_show_source(&$2 _INLINE_TLS); }
	|	HIGHLIGHT_STRING '(' expr ')' ';'  { if (GLOBAL(Execute)) eval_string(&$3,NULL,1 _INLINE_TLS); }
	|	HIGHLIGHT_STRING '(' expr ',' expr ')' ';' { if (GLOBAL(Execute)) eval_string(&$3,&$5,2 _INLINE_TLS); }
	|	EVAL '(' expr ')' ';' { if (GLOBAL(Execute)) eval_string(&$3,&$5,0 _INLINE_TLS); }
	|	INCLUDE expr ';' { if (GLOBAL(Execute)) conditional_include_file(&$2,&$3 _INLINE_TLS); }
	|	';'		/* empty statement */
;


for_statement:
		statement
	|	':' statement_list ENDFOR ';'
;

switch_case_list:
		'{' case_list '}'
	|	'{' ';' case_list '}'
	|	':' case_list ENDSWITCH ';'
	|	':' ';' case_list ENDSWITCH ';'
;

while_statement:
		statement
	|	':' statement_list ENDWHILE ';'
;

while_iterations:
		/* empty */
	|	while_iterations CONTINUED_WHILE '(' expr ')' { cs_start_while(&$2, &$4 _INLINE_TLS); } while_statement { cs_end_while(&$2,&yychar _INLINE_TLS); }
;


do_while_iterations:

	|	do_while_iterations CONTINUED_DOWHILE { cs_start_do_while(&$2 _INLINE_TLS); } statement WHILE { cs_force_eval_do_while( _INLINE_TLS_VOID); } '(' expr ')' ';'{ cs_end_do_while(&$2,&$8,&yychar _INLINE_TLS); }
;


for_iterations:
		/* empty */
	|	for_iterations CONTINUED_FOR { for_pre_expr1(&$2 _INLINE_TLS); }
			'(' for_expr { if (GLOBAL(Execute)) pval_destructor(&$5 _INLINE_TLS); for_pre_expr2(&$2 _INLINE_TLS); }
			';' for_expr { for_pre_expr3(&$2,&$8 _INLINE_TLS); }
			';' for_expr { for_pre_statement(&$2,&$8,&$11 _INLINE_TLS); }
			')' for_statement { for_post_statement(&$2,&$7,&$10,&$13,&yychar _INLINE_TLS); }
;


elseif_list:
		/* empty */
	|	elseif_list ELSEIF '(' { cs_elseif_start_evaluate( _INLINE_TLS_VOID); } 
			expr { cs_elseif_end_evaluate( _INLINE_TLS_VOID); } ')' { cs_start_elseif (&$5 _INLINE_TLS); } statement
;


new_elseif_list:
		/* empty */
	|	new_elseif_list ELSEIF '(' { cs_elseif_start_evaluate( _INLINE_TLS_VOID); } 
			expr { cs_elseif_end_evaluate( _INLINE_TLS_VOID); } ')' ':' { cs_start_elseif (&$6 _INLINE_TLS); } statement_list
;


else_single:
		/* empty */
	|	ELSE { cs_start_else( _INLINE_TLS_VOID); } statement
;


new_else_single:
		/* empty */
	|	ELSE ':' { cs_start_else( _INLINE_TLS_VOID); } statement_list
;


case_list:
		/* empty */
	|	CASE expr ':' { cs_switch_case_pre(&$2 _INLINE_TLS); } statement_list { cs_switch_case_post( _INLINE_TLS_VOID); } case_list
	|	DEFAULT ':' { cs_switch_case_pre(NULL _INLINE_TLS); } statement_list { cs_switch_case_post( _INLINE_TLS_VOID); }
	|	CASE expr ';' { cs_switch_case_pre(&$2 _INLINE_TLS); } statement_list { cs_switch_case_post( _INLINE_TLS_VOID); } case_list
	|	DEFAULT ';' { cs_switch_case_pre(NULL _INLINE_TLS); } statement_list { cs_switch_case_post( _INLINE_TLS_VOID); }
;


parameter_list: 
		{ GLOBAL(param_index)=0; } non_empty_parameter_list
	|	/* empty */
;


non_empty_parameter_list:
		'$' varname_scalar		{ get_function_parameter(&$2, BYREF_NONE, NULL _INLINE_TLS); }
	|	'&' '$' varname_scalar	{ get_function_parameter(&$3, BYREF_FORCE, NULL _INLINE_TLS); }
	|	PHP_CONST '$' varname_scalar	{ get_function_parameter(&$3, BYREF_ALLOW, NULL _INLINE_TLS); }
	|	'$' varname_scalar '=' signed_scalar { get_function_parameter(&$2, BYREF_NONE, &$4 _INLINE_TLS); }
	|	non_empty_parameter_list ',' '$' varname_scalar { get_function_parameter(&$4, BYREF_NONE, NULL _INLINE_TLS); }
	|	non_empty_parameter_list ',' '&' '$' varname_scalar { get_function_parameter(&$5, BYREF_FORCE, NULL  _INLINE_TLS); }
	|	non_empty_parameter_list ',' PHP_CONST '$' varname_scalar { get_function_parameter(&$5, BYREF_ALLOW, NULL _INLINE_TLS); }
	|	non_empty_parameter_list ',' '$' varname_scalar '=' signed_scalar { get_function_parameter(&$4, BYREF_NONE, &$6 _INLINE_TLS); }
;


function_call_parameter_list:
		non_empty_function_call_parameter_list
	|	/* empty */
;


non_empty_function_call_parameter_list:
		expr_without_variable { pass_parameter_by_value(&$1 _INLINE_TLS); }
	|	assignment_variable_pointer { pass_parameter(&$1,0 _INLINE_TLS); }
	|	'&' assignment_variable_pointer { pass_parameter(&$2,1 _INLINE_TLS); }
	|	non_empty_function_call_parameter_list ',' expr_without_variable { pass_parameter_by_value(&$3 _INLINE_TLS); }
	|	non_empty_function_call_parameter_list ',' assignment_variable_pointer { pass_parameter(&$3,0 _INLINE_TLS); }
	|	non_empty_function_call_parameter_list ',' '&' assignment_variable_pointer { pass_parameter(&$4,1 _INLINE_TLS); }
;

global_var_list:
		global_var_list ',' '$' var { cs_global_variable(&$4 _INLINE_TLS); }
	|	'$' var { cs_global_variable(&$2 _INLINE_TLS); }
;


static_var_list:
		static_var_list ',' '$' var { cs_static_variable(&$4,NULL _INLINE_TLS); }
	|	static_var_list ',' '$' var '=' unambiguous_static_assignment { cs_static_variable(&$4,&$6 _INLINE_TLS); }
	|	'$' var { cs_static_variable(&$2,NULL _INLINE_TLS); }
	|	'$' var '=' unambiguous_static_assignment { cs_static_variable(&$2,&$4 _INLINE_TLS); }

;


unambiguous_static_assignment:
		signed_scalar { if (GLOBAL(Execute)) $$ = $1; }
	|	'(' expr ')' { if (GLOBAL(Execute)) $$ = $2; }
;


class_statement_list:
		class_statement_list class_statement
	|	/* empty */
;


class_statement:
		VAR class_variable_declaration ';'
	|	OLD_FUNCTION STRING { start_function_decleration(_INLINE_TLS_VOID); } 
			parameter_list '(' statement_list ')' ';' { end_function_decleration(&$1,&$2 _INLINE_TLS); }
	|	FUNCTION STRING '(' { start_function_decleration(_INLINE_TLS_VOID); }
			parameter_list ')' '{' statement_list '}' { end_function_decleration(&$1,&$2 _INLINE_TLS); }

;


class_variable_declaration:
		class_variable_declaration ',' '$' varname_scalar { declare_class_variable(&$4,NULL _INLINE_TLS); }
	|	class_variable_declaration ',' '$' varname_scalar '=' expr { declare_class_variable(&$4,&$6 _INLINE_TLS); }
	|	'$' varname_scalar { declare_class_variable(&$2,NULL _INLINE_TLS); }
	|	'$' varname_scalar '=' expr { declare_class_variable(&$2,&$4 _INLINE_TLS); }
;

	
echo_expr_list:	
		/* empty */
	|	echo_expr_list ',' expr { if (GLOBAL(Execute)) { php3i_print_variable(&$3 _INLINE_TLS); pval_destructor(&$3 _INLINE_TLS); } }
	|	expr { if (GLOBAL(Execute)) { php3i_print_variable(&$1 _INLINE_TLS); pval_destructor(&$1 _INLINE_TLS); } }
;


for_expr:
		/* empty */ { $$.value.lval=1;  $$.type=IS_LONG; }  /* ensure empty truth values are considered TRUE */
	|	for_expr ',' expr { if (GLOBAL(Execute)) { $$ = $3; pval_destructor(&$1 _INLINE_TLS); } }
	|	expr { if (GLOBAL(Execute)) $$ = $1; }
;


expr_without_variable:	
		PHP_LIST '(' assignment_list ')' '=' expr { assign_to_list(&$$, &$3, &$6 _INLINE_TLS);}
	|	assignment_variable_pointer '=' expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,NULL _INLINE_TLS); }
	|	assignment_variable_pointer PLUS_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,add_function _INLINE_TLS); }
	|	assignment_variable_pointer	MINUS_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,sub_function _INLINE_TLS); }
	|	assignment_variable_pointer	MUL_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,mul_function _INLINE_TLS); }	
	|	assignment_variable_pointer DIV_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,div_function _INLINE_TLS); }
	|	assignment_variable_pointer	CONCAT_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,concat_function_with_free _INLINE_TLS); }
	|	assignment_variable_pointer MOD_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,mod_function _INLINE_TLS); }
	|	assignment_variable_pointer	AND_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,bitwise_and_function _INLINE_TLS); }	
	|	assignment_variable_pointer	OR_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,bitwise_or_function _INLINE_TLS); }
	|	assignment_variable_pointer	XOR_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,bitwise_xor_function _INLINE_TLS); }
	|	assignment_variable_pointer SHIFT_LEFT_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,shift_left_function _INLINE_TLS); }
	|	assignment_variable_pointer SHIFT_RIGHT_EQUAL expr { if (GLOBAL(Execute)) assign_to_variable(&$$,&$1,&$3,shift_right_function _INLINE_TLS); }
	|	assignment_variable_pointer  INCREMENT { if (GLOBAL(Execute)) incdec_variable(&$$,&$1,increment_function,1 _INLINE_TLS); }
	|	INCREMENT assignment_variable_pointer { if (GLOBAL(Execute)) incdec_variable(&$$,&$2,increment_function,0 _INLINE_TLS); }
	|	assignment_variable_pointer DECREMENT { if (GLOBAL(Execute)) incdec_variable(&$$,&$1,decrement_function,1 _INLINE_TLS); }
	|	DECREMENT assignment_variable_pointer { if (GLOBAL(Execute)) incdec_variable(&$$,&$2,decrement_function,0 _INLINE_TLS); }
	|	expr BOOLEAN_OR { cs_pre_boolean_or(&$1 _INLINE_TLS); } expr { cs_post_boolean_or(&$$,&$1,&$4 _INLINE_TLS); }
	|	expr BOOLEAN_AND { cs_pre_boolean_and(&$1 _INLINE_TLS); } expr { cs_post_boolean_and(&$$,&$1,&$4 _INLINE_TLS); }
	|	expr LOGICAL_OR { cs_pre_boolean_or(&$1 _INLINE_TLS); } expr { cs_post_boolean_or(&$$,&$1,&$4 _INLINE_TLS); }
	|	expr LOGICAL_AND { cs_pre_boolean_and(&$1 _INLINE_TLS); } expr { cs_post_boolean_and(&$$,&$1,&$4 _INLINE_TLS); }
	|	expr LOGICAL_XOR expr { if (GLOBAL(Execute)) { boolean_xor_function(&$$,&$1,&$3); } }
	|	expr '|' expr { if (GLOBAL(Execute)) bitwise_or_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '^' expr { if (GLOBAL(Execute)) bitwise_xor_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '&' expr { if (GLOBAL(Execute)) bitwise_and_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '.' expr { if (GLOBAL(Execute)) concat_function(&$$,&$1,&$3,1 _INLINE_TLS); }
	|	expr '+' expr { if (GLOBAL(Execute)) add_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '-' expr { if (GLOBAL(Execute)) sub_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '*' expr { if (GLOBAL(Execute)) mul_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '/' expr { if (GLOBAL(Execute)) div_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '%' expr { if (GLOBAL(Execute)) mod_function(&$$,&$1,&$3 _INLINE_TLS); }
	| 	expr SHIFT_LEFT expr { if (GLOBAL(Execute)) shift_left_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr SHIFT_RIGHT expr { if (GLOBAL(Execute)) shift_right_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	'+' expr { if (GLOBAL(Execute)) { pval tmp;  tmp.value.lval=0;  tmp.type=IS_LONG;  add_function(&$$,&tmp,&$2 _INLINE_TLS); } }
	|	'-' expr { if (GLOBAL(Execute)) { pval tmp;  tmp.value.lval=0;  tmp.type=IS_LONG;  sub_function(&$$,&tmp,&$2 _INLINE_TLS); } }
	|	'!' expr { if (GLOBAL(Execute)) boolean_not_function(&$$,&$2); }
	|	'~' expr { if (GLOBAL(Execute)) bitwise_not_function(&$$,&$2 _INLINE_TLS); }
	|	expr IS_EQUAL expr { if (GLOBAL(Execute)) is_equal_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr IS_NOT_EQUAL expr { if (GLOBAL(Execute)) is_not_equal_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '<' expr { if (GLOBAL(Execute)) is_smaller_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr IS_SMALLER_OR_EQUAL expr { if (GLOBAL(Execute)) is_smaller_or_equal_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr '>' expr { if (GLOBAL(Execute)) is_greater_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	expr IS_GREATER_OR_EQUAL expr { if (GLOBAL(Execute)) is_greater_or_equal_function(&$$,&$1,&$3 _INLINE_TLS); }
	|	'(' expr ')' { if (GLOBAL(Execute)) $$ = $2; }
	|	'(' expr error {  php3_error(E_PARSE,"'(' unmatched",GLOBAL(current_lineno)); if (GLOBAL(Execute)) $$ = $2; yyerrok; }
	|	expr '?' { cs_questionmark_op_pre_expr1(&$1 _INLINE_TLS); }
			expr ':' { cs_questionmark_op_pre_expr2(&$1 _INLINE_TLS); }
			expr { cs_questionmark_op_post_expr2(&$$,&$1,&$4,&$7 _INLINE_TLS); }
	|	var { cs_functioncall_pre_variable_passing(&$1,NULL,1 _INLINE_TLS); }
			'(' function_call_parameter_list ')' { cs_functioncall_post_variable_passing(&$1,&yychar _INLINE_TLS); }
			possible_function_call { cs_functioncall_end(&$$,&$1,&$5,&yychar,1 _INLINE_TLS);}
	|	internal_functions_in_yacc
	|	'$' unambiguous_class_name PHP_CLASS_OPERATOR var { cs_functioncall_pre_variable_passing(&$4,&$2,1 _INLINE_TLS); }
			'(' function_call_parameter_list ')' { cs_functioncall_post_variable_passing(&$4,&yychar _INLINE_TLS); }
			possible_function_call { cs_functioncall_end(&$$,&$4,&$8,&yychar,1 _INLINE_TLS); }
	|	NEW var { assign_new_object(&$$,&$2,1 _INLINE_TLS); }
	|	NEW var	{ assign_new_object(&$$,&$2,0 _INLINE_TLS); }
		'(' { if (!GLOBAL(shutdown_requested)) { pval object_pointer; object_pointer.value.varptr.pvalue = &$3; cs_functioncall_pre_variable_passing(&$2, &object_pointer, 1 _INLINE_TLS); } }
		function_call_parameter_list ')' { cs_functioncall_post_variable_passing(&$2, &yychar _INLINE_TLS); }
		possible_function_call { cs_functioncall_end(&$$, &$2, &$7, &yychar, 1 _INLINE_TLS);  $$ = $3; }
	|	INT_CAST expr { if (GLOBAL(Execute)) { convert_to_long(&$2); $$ = $2; } }
	|	DOUBLE_CAST expr { if (GLOBAL(Execute)) { convert_to_double(&$2); $$ = $2; } }
	|	STRING_CAST expr { if (GLOBAL(Execute)) { convert_to_string(&$2); $$ = $2; } }
	|	ARRAY_CAST expr { if (GLOBAL(Execute)) { convert_to_array(&$2); $$ = $2; } }
	|	OBJECT_CAST expr { if (GLOBAL(Execute)) { convert_to_object(&$2); $$ = $2; } }
	|	EXIT { if (GLOBAL(Execute)) { php3_header(); GLOBAL(shutdown_requested)=ABNORMAL_SHUTDOWN; $$.type=IS_LONG; $$.value.lval=1; } }
	|	EXIT '(' ')'  { if (GLOBAL(Execute)) { php3_header(); GLOBAL(shutdown_requested)=ABNORMAL_SHUTDOWN; $$.type=IS_LONG; $$.value.lval=1; } }
	|	EXIT '(' expr ')'  { if (GLOBAL(Execute)) { if (php3_header()) { convert_to_string(&$3);  PUTS($3.value.str.val); convert_to_long(&$3); wanted_exit_status = $3.value.lval;  pval_destructor(&$3 _INLINE_TLS); } GLOBAL(shutdown_requested)=ABNORMAL_SHUTDOWN; $$.type=IS_LONG; $$.value.lval=1; } }
	|	'@' { $1.cs_data.error_reporting=GLOBAL(error_reporting); GLOBAL(error_reporting)=0; } expr { GLOBAL(error_reporting)=$1.cs_data.error_reporting; $$ = $3; }
	|	'@' error { php3_error(E_ERROR,"@ operator may only be used on expressions"); }
	|   scalar { if (GLOBAL(Execute)) $$ = $1; }
	|	PHP_ARRAY '(' array_pair_list ')' { if (GLOBAL(Execute)) $$ = $3; }
	|	'`' encaps_list '`'  { cs_system(&$$,&$2 _INLINE_TLS); }
	|	PHP_PRINT expr { if (GLOBAL(Execute)) { php3i_print_variable(&$2 _INLINE_TLS);  pval_destructor(&$2 _INLINE_TLS);  $$.value.lval=1; $$.type=IS_LONG; } }
;

scalar:
		LNUMBER { if (GLOBAL(Execute)) $$ = $1; }
	|	DNUMBER     { if (GLOBAL(Execute)) $$ = $1; }
	|	'"' encaps_list '"' { if (GLOBAL(Execute)) $$ = $2; }
	|	'\'' encaps_list '\'' { if (GLOBAL(Execute)) $$ = $2; }
	|	STRING_CONSTANT { if (GLOBAL(Execute)) { $$ = $1; COPY_STRING($$); } }
	|	STRING { if (GLOBAL(Execute)) { $$ = $1; COPY_STRING($$); php3_error(E_NOTICE,"'%s' is not a valid constant - assumed to be \"%s\"",$1.value.str.val,$1.value.str.val); } }
	|	PHP_LINE { if (GLOBAL(Execute)) { $$ = $1; } }
	|	PHP_FILE { if (GLOBAL(Execute)) { $$ = $1; COPY_STRING($$); } }
;

signed_scalar:
		scalar				{ if (GLOBAL(Execute)) $$ = $1; }
	|	'+' signed_scalar	{ if (GLOBAL(Execute)) $$ = $2; }
	|	'-' signed_scalar	{ if (GLOBAL(Execute)) { pval tmp;  tmp.value.lval=0;  tmp.type=IS_LONG;  sub_function(&$$,&tmp,&$2 _INLINE_TLS); } }
;

varname_scalar:
       STRING     { if (GLOBAL(Execute)){ $$ = $1; }}
;



expr:
		assignment_variable_pointer { if (GLOBAL(Execute)) read_pointer_value(&$$,&$1 _INLINE_TLS); }
	|	expr_without_variable { if (GLOBAL(Execute)) $$ = $1; }
;


unambiguous_variable_name:
		var { if (GLOBAL(Execute)) $$ = $1; }
	|	'{' expr '}' { if (GLOBAL(Execute)) $$ = $2; }
;


unambiguous_array_name:
		varname_scalar { if (GLOBAL(Execute)) {$$ = $1;COPY_STRING($$); }}
	|	'{' expr '}' { if (GLOBAL(Execute)) $$ = $2; }

;


unambiguous_class_name:
		unambiguous_class_name PHP_CLASS_OPERATOR varname_scalar { get_object_symtable(&$$,&$1,&$3 _INLINE_TLS); }
	|	multi_dimensional_array { if (GLOBAL(Execute)) { if ($1.value.varptr.pvalue && ((pval *)$1.value.varptr.pvalue)->type == IS_OBJECT) { $$=$1; } else { $$.value.varptr.pvalue=NULL; } } }
	|	varname_scalar { get_object_symtable(&$$,NULL,&$1 _INLINE_TLS); }
;

assignment_list:
		assignment_list ',' assignment_variable_pointer {

	if (GLOBAL(Execute)) {
		$$=$1;
		_php3_hash_next_index_insert($$.value.ht,&$3,sizeof(pval),NULL);
	}
}
	|	assignment_variable_pointer {

	if (GLOBAL(Execute)) {
		$$.value.ht = (HashTable *) emalloc(sizeof(HashTable));
		_php3_hash_init($$.value.ht,0,NULL,NULL,0);
		$$.type = IS_ARRAY;
		_php3_hash_next_index_insert($$.value.ht,&$1,sizeof(pval),NULL);
	}
}
	|	assignment_list ',' {
	if (GLOBAL(Execute)) {
		$$=$1;
		$2.value.varptr.pvalue = NULL; /* $2 is just used as temporary space */
		_php3_hash_next_index_insert($$.value.ht,&$2,sizeof(pval),NULL);
	}
}
	|	/* empty */ {
	if (GLOBAL(Execute)) {
		pval tmp;

		$$.value.ht = (HashTable *) emalloc(sizeof(HashTable));
		_php3_hash_init($$.value.ht,0,NULL,NULL,0);
		$$.type = IS_ARRAY;
		tmp.value.varptr.pvalue = NULL;
		_php3_hash_next_index_insert($$.value.ht,&tmp,sizeof(pval),NULL);
	}
}
;


assignment_variable_pointer:
		'$' unambiguous_variable_name { get_regular_variable_pointer(&$$,&$2 _INLINE_TLS); }
	|	'$' multi_dimensional_array { if (GLOBAL(Execute)) $$=$2; }
	|	'$' unambiguous_class_name PHP_CLASS_OPERATOR unambiguous_variable_name { get_class_variable_pointer(&$$,&$2,&$4 _INLINE_TLS); }
	|	'$' unambiguous_class_name PHP_CLASS_OPERATOR unambiguous_array_name '[' { start_array_parsing(&$4,&$2 _INLINE_TLS); } dimensions { end_array_parsing(&$$,&$7 _INLINE_TLS); }
;


var:	
		'$' unambiguous_variable_name { if (GLOBAL(Execute)) get_regular_variable_contents(&$$,&$2,1 _INLINE_TLS); }
	|	varname_scalar { if (GLOBAL(Execute)) {$$ = $1;COPY_STRING($$);} }
	|	'$' multi_dimensional_array {  if (GLOBAL(Execute)) read_pointer_value(&$$,&$2 _INLINE_TLS); }
;

multi_dimensional_array:
		unambiguous_array_name '[' { start_array_parsing(&$1,NULL _INLINE_TLS); } dimensions { end_array_parsing(&$$,&$4 _INLINE_TLS); }
;


dimensions:
		dimensions '[' expr ']' { fetch_array_index(&$$,&$3,&$1 _INLINE_TLS); }
	|	dimensions '[' ']' { fetch_array_index(&$$,NULL,&$1 _INLINE_TLS); }
	|	{ start_dimensions_parsing(&$$ _INLINE_TLS); } ']' { fetch_array_index(&$$,NULL,&$$ _INLINE_TLS); }
	|	{ start_dimensions_parsing(&$$ _INLINE_TLS); } expr ']' { fetch_array_index(&$$,&$2,&$$ _INLINE_TLS); }
;


array_pair_list:
		/* empty */ { if (GLOBAL(Execute)) { $$.value.ht = (HashTable *) emalloc(sizeof(HashTable));  _php3_hash_init($$.value.ht,0,NULL,PVAL_DESTRUCTOR,0); $$.type = IS_ARRAY; } }
	|	non_empty_array_pair_list { $$ = $1; }
;

non_empty_array_pair_list:
		non_empty_array_pair_list ',' expr PHP_DOUBLE_ARROW expr { if (GLOBAL(Execute)) {$$=$1; add_array_pair_list(&$$, &$3, &$5, 0 _INLINE_TLS);} }
	|	non_empty_array_pair_list ',' expr { if (GLOBAL(Execute)) {$$=$1; add_array_pair_list(&$$, NULL, &$3, 0 _INLINE_TLS);} }
	|	expr PHP_DOUBLE_ARROW expr { if (GLOBAL(Execute)) add_array_pair_list(&$$, &$1, &$3, 1 _INLINE_TLS); }
	|	expr { if (GLOBAL(Execute)) add_array_pair_list(&$$, NULL, &$1, 1 _INLINE_TLS); }
;


encaps_list:
		encaps_list '$' STRING { add_regular_encapsed_variable(&$$,&$1,&$3 _INLINE_TLS); }
	|	encaps_list '$' STRING '[' STRING ']' { add_assoc_array_encapsed_variable(&$$,&$1,&$3,&$5 _INLINE_TLS); }
	|	encaps_list '$' STRING '[' NUM_STRING ']' { add_regular_array_encapsed_variable(&$$,&$1,&$3,&$5 _INLINE_TLS); }
	|	encaps_list '$' STRING '[' '$' STRING ']' { add_variable_array_encapsed_variable(&$$,&$1,&$3,&$6 _INLINE_TLS); }
	|	encaps_list '$' STRING PHP_CLASS_OPERATOR STRING { add_encapsed_object_property(&$$,&$1,&$3,&$5 _INLINE_TLS); }
	|	encaps_list '$' '{' STRING '}' { add_regular_encapsed_variable(&$$,&$1,&$4 _INLINE_TLS); }
	|	encaps_list '$' '{' STRING '[' STRING ']' '}' { add_assoc_array_encapsed_variable(&$$,&$1,&$4,&$6 _INLINE_TLS); }
	|	encaps_list '$' '{' STRING '[' NUM_STRING ']' '}' { add_regular_array_encapsed_variable(&$$,&$1,&$4,&$6 _INLINE_TLS); }
	|	encaps_list '$' '{' STRING '[' '$' STRING ']' '}' { add_variable_array_encapsed_variable(&$$,&$1,&$4,&$7 _INLINE_TLS); }
	|	encaps_list '$' '{' STRING PHP_CLASS_OPERATOR STRING '}' { add_encapsed_object_property(&$$,&$1,&$4,&$6 _INLINE_TLS); }
	|	encaps_list '$' '{' '$' STRING '}' { add_indirect_encapsed_variable(&$$,&$1,&$5 _INLINE_TLS); }
	|	encaps_list STRING	{ if (GLOBAL(Execute)) { concat_function(&$$,&$1,&$2,0 _INLINE_TLS); } }
	|	encaps_list NUM_STRING	{ if (GLOBAL(Execute)) { concat_function(&$$,&$1,&$2,0 _INLINE_TLS); } }
	|	encaps_list ENCAPSED_AND_WHITESPACE { if (GLOBAL(Execute)) { concat_function(&$$,&$1,&$2,0 _INLINE_TLS); } }
	|	encaps_list CHARACTER { if (GLOBAL(Execute)) add_char_to_string(&$$,&$1,&$2 _INLINE_TLS); }
	|	encaps_list BAD_CHARACTER { if (GLOBAL(Execute)) { php3_error(E_NOTICE,"Bad escape sequence:  %s",$2.value.str.val); concat_function(&$$,&$1,&$2,0 _INLINE_TLS); } }
	|	encaps_list '['		{ if (GLOBAL(Execute)) add_char_to_string(&$$,&$1,&$2 _INLINE_TLS); }
	|	encaps_list ']'		{ if (GLOBAL(Execute)) add_char_to_string(&$$,&$1,&$2 _INLINE_TLS); }
	|	encaps_list '{'	{ if (GLOBAL(Execute)) add_char_to_string(&$$,&$1,&$2 _INLINE_TLS); }
	|	encaps_list '}'	{ if (GLOBAL(Execute)) add_char_to_string(&$$,&$1,&$2 _INLINE_TLS); }
	|	encaps_list PHP_CLASS_OPERATOR {  if (GLOBAL(Execute)) { pval tmp;  tmp.value.str.val="->"; tmp.value.str.len=2; tmp.type=IS_STRING; concat_function(&$$,&$1,&tmp,0 _INLINE_TLS); } }
	|	/* empty */			{ if (GLOBAL(Execute)) var_reset(&$$); }
;


internal_functions_in_yacc:
		PHP_UNSET '(' assignment_variable_pointer ')' { php3_unset(&$$, &$3); }
	|	PHP_ISSET '(' assignment_variable_pointer ')' { php3_isset(&$$, &$3); }
	|	PHP_EMPTY '(' assignment_variable_pointer ')' { php3_empty(&$$, &$3); }
;


possible_function_call:
		/* empty */
	|	OLD_FUNCTION STRING parameter_list '(' statement_list ')' ';'
	|	FUNCTION STRING '(' parameter_list ')' '{' statement_list '}'
;

%%

inline void clear_lookahead(int *yychar)
{
	if (yychar) {
		*yychar=YYEMPTY;
	}
}


/*** Manhattan project ***/
/* Be able to call user-levle functions from C */
/* "A beer and serious lack of sleep do wonders" -- Zeev */
PHPAPI int call_user_function(HashTable *function_table, pval *object, pval *function_name, pval *retval, int param_count, pval *params[])
{
	pval *func;
	pval return_offset;
	int i;
	pval class_ptr;
	int original_shutdown_requested=GLOBAL(shutdown_requested);
	int original_execute_flag = GLOBAL(ExecuteFlag);
	FunctionState original_function_state = GLOBAL(function_state);
	pval p_function_name;

	if (GLOBAL(shutdown_requested)==ABNORMAL_SHUTDOWN) {
		return FAILURE;
	}

	/* save the location to go back to */
	return_offset.offset = tc_get_current_offset(&GLOBAL(token_cache_manager))-1;	
		
	if (object) {
		function_table = object->value.ht;
	}

	/* if phpparse() triggers shutdown, function_name would get erased, so it'd end up
	 * being freed twice.  Avoid this.
	 */
	p_function_name = *function_name;
	pval_copy_constructor(&p_function_name);
	php3_str_tolower(p_function_name.value.str.val, p_function_name.value.str.len);
	if (_php3_hash_find(function_table, p_function_name.value.str.val, p_function_name.value.str.len+1, (void **) &func)==FAILURE
		|| func->type != IS_USER_FUNCTION) {
		return FAILURE;
	}
	
	/* Do some magic... */
	GLOBAL(shutdown_requested) = 0;
	GLOBAL(function_state).loop_nest_level = GLOBAL(function_state).loop_change_level = GLOBAL(function_state).loop_change_type = 0;
	GLOBAL(function_state).returned = 0;
	GLOBAL(function_state).function_name = p_function_name.value.str.val;
	GLOBAL(ExecuteFlag) = EXECUTE;
	GLOBAL(Execute) = SHOULD_EXECUTE;

	tc_set_token(&token_cache_manager, func->offset, IC_FUNCTION);
	if (object) {
		class_ptr.value.varptr.pvalue = object;
		cs_functioncall_pre_variable_passing(&p_function_name, &class_ptr, 0 _INLINE_TLS);
	} else {
		cs_functioncall_pre_variable_passing(&p_function_name,NULL, 0 _INLINE_TLS);
	}
	for (i=0; i<param_count; i++) {
		_php3_hash_next_index_pointer_insert(GLOBAL(function_state).function_symbol_table, params[i]);
	}
	cs_functioncall_post_variable_passing(&p_function_name, NULL);
	phpparse();
	if (GLOBAL(shutdown_requested)) { /* we died during this function call */
		return FAILURE;
	}
	cs_functioncall_end(retval,&p_function_name,&return_offset,NULL,0);
	pval_destructor(&p_function_name);
	GLOBAL(function_state) = original_function_state;
	GLOBAL(ExecuteFlag) = original_execute_flag;
	GLOBAL(shutdown_requested) = original_shutdown_requested;
	GLOBAL(Execute) = SHOULD_EXECUTE;

	return SUCCESS;
}
