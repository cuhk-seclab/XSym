/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         phpparse
#define yylex           phplex
#define yyerror         phperror
#define yydebug         phpdebug
#define yynerrs         phpnerrs


/* Copy the first part of user declarations.  */
#line 1 "./language-parser.y" /* yacc.c:339  */


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

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LOGICAL_OR = 258,
    LOGICAL_XOR = 259,
    LOGICAL_AND = 260,
    PHP_PRINT = 261,
    PLUS_EQUAL = 262,
    MINUS_EQUAL = 263,
    MUL_EQUAL = 264,
    DIV_EQUAL = 265,
    CONCAT_EQUAL = 266,
    MOD_EQUAL = 267,
    AND_EQUAL = 268,
    OR_EQUAL = 269,
    XOR_EQUAL = 270,
    SHIFT_LEFT_EQUAL = 271,
    SHIFT_RIGHT_EQUAL = 272,
    BOOLEAN_OR = 273,
    BOOLEAN_AND = 274,
    IS_EQUAL = 275,
    IS_NOT_EQUAL = 276,
    IS_SMALLER_OR_EQUAL = 277,
    IS_GREATER_OR_EQUAL = 278,
    SHIFT_LEFT = 279,
    SHIFT_RIGHT = 280,
    INCREMENT = 281,
    DECREMENT = 282,
    INT_CAST = 283,
    DOUBLE_CAST = 284,
    STRING_CAST = 285,
    ARRAY_CAST = 286,
    OBJECT_CAST = 287,
    NEW = 288,
    EXIT = 289,
    IF = 290,
    ELSEIF = 291,
    ELSE = 292,
    ENDIF = 293,
    LNUMBER = 294,
    DNUMBER = 295,
    STRING = 296,
    NUM_STRING = 297,
    INLINE_HTML = 298,
    CHARACTER = 299,
    BAD_CHARACTER = 300,
    ENCAPSED_AND_WHITESPACE = 301,
    PHP_ECHO = 302,
    DO = 303,
    WHILE = 304,
    ENDWHILE = 305,
    FOR = 306,
    ENDFOR = 307,
    SWITCH = 308,
    ENDSWITCH = 309,
    CASE = 310,
    DEFAULT = 311,
    BREAK = 312,
    CONTINUE = 313,
    CONTINUED_WHILE = 314,
    CONTINUED_DOWHILE = 315,
    CONTINUED_FOR = 316,
    OLD_FUNCTION = 317,
    FUNCTION = 318,
    IC_FUNCTION = 319,
    PHP_CONST = 320,
    RETURN = 321,
    INCLUDE = 322,
    REQUIRE = 323,
    HIGHLIGHT_FILE = 324,
    HIGHLIGHT_STRING = 325,
    PHP_GLOBAL = 326,
    PHP_STATIC = 327,
    PHP_UNSET = 328,
    PHP_ISSET = 329,
    PHP_EMPTY = 330,
    CLASS = 331,
    EXTENDS = 332,
    PHP_CLASS_OPERATOR = 333,
    PHP_DOUBLE_ARROW = 334,
    PHP_LIST = 335,
    PHP_ARRAY = 336,
    VAR = 337,
    EVAL = 338,
    DONE_EVAL = 339,
    PHP_LINE = 340,
    PHP_FILE = 341,
    STRING_CONSTANT = 342
  };
#endif
#include "control_structures_inline.h"


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


#line 258 "./language-parser.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "language-parser.tab.h".  */
#ifndef YY_PHP_LANGUAGE_PARSER_TAB_H_INCLUDED
# define YY_PHP_LANGUAGE_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int phpdebug;
#endif

/* Tokens.  */
#define LOGICAL_OR 258
#define LOGICAL_XOR 259
#define LOGICAL_AND 260
#define PHP_PRINT 261
#define PLUS_EQUAL 262
#define MINUS_EQUAL 263
#define MUL_EQUAL 264
#define DIV_EQUAL 265
#define CONCAT_EQUAL 266
#define MOD_EQUAL 267
#define AND_EQUAL 268
#define OR_EQUAL 269
#define XOR_EQUAL 270
#define SHIFT_LEFT_EQUAL 271
#define SHIFT_RIGHT_EQUAL 272
#define BOOLEAN_OR 273
#define BOOLEAN_AND 274
#define IS_EQUAL 275
#define IS_NOT_EQUAL 276
#define IS_SMALLER_OR_EQUAL 277
#define IS_GREATER_OR_EQUAL 278
#define SHIFT_LEFT 279
#define SHIFT_RIGHT 280
#define INCREMENT 281
#define DECREMENT 282
#define INT_CAST 283
#define DOUBLE_CAST 284
#define STRING_CAST 285
#define ARRAY_CAST 286
#define OBJECT_CAST 287
#define NEW 288
#define EXIT 289
#define IF 290
#define ELSEIF 291
#define ELSE 292
#define ENDIF 293
#define LNUMBER 294
#define DNUMBER 295
#define STRING 296
#define NUM_STRING 297
#define INLINE_HTML 298
#define CHARACTER 299
#define BAD_CHARACTER 300
#define ENCAPSED_AND_WHITESPACE 301
#define PHP_ECHO 302
#define DO 303
#define WHILE 304
#define ENDWHILE 305
#define FOR 306
#define ENDFOR 307
#define SWITCH 308
#define ENDSWITCH 309
#define CASE 310
#define DEFAULT 311
#define BREAK 312
#define CONTINUE 313
#define CONTINUED_WHILE 314
#define CONTINUED_DOWHILE 315
#define CONTINUED_FOR 316
#define OLD_FUNCTION 317
#define FUNCTION 318
#define IC_FUNCTION 319
#define PHP_CONST 320
#define RETURN 321
#define INCLUDE 322
#define REQUIRE 323
#define HIGHLIGHT_FILE 324
#define HIGHLIGHT_STRING 325
#define PHP_GLOBAL 326
#define PHP_STATIC 327
#define PHP_UNSET 328
#define PHP_ISSET 329
#define PHP_EMPTY 330
#define CLASS 331
#define EXTENDS 332
#define PHP_CLASS_OPERATOR 333
#define PHP_DOUBLE_ARROW 334
#define PHP_LIST 335
#define PHP_ARRAY 336
#define VAR 337
#define EVAL 338
#define DONE_EVAL 339
#define PHP_LINE 340
#define PHP_FILE 341
#define STRING_CONSTANT 342

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int phpparse (void);

#endif /* !YY_PHP_LANGUAGE_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 482 "./language-parser.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3573

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  117
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  106
/* YYNRULES -- Number of rules.  */
#define YYNRULES  299
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  626

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   342

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    41,   114,     2,   112,    40,    26,   115,
     109,   110,    38,    35,     3,    36,    37,    39,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    21,   111,
      29,     8,    31,    20,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    51,     2,   116,    25,     2,   113,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   107,    24,   108,    42,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     4,     5,
       6,     7,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    22,    23,    27,    28,    30,    32,    33,
      34,    43,    44,    45,    46,    47,    48,    49,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   268,   268,   269,   274,   275,   275,   276,   276,   276,
     277,   277,   277,   278,   278,   278,   278,   279,   280,   281,
     282,   283,   279,   284,   284,   285,   286,   287,   288,   289,
     289,   291,   291,   293,   293,   294,   295,   296,   297,   298,
     298,   299,   299,   300,   301,   302,   303,   303,   304,   305,
     306,   307,   308,   309,   314,   315,   319,   320,   321,   322,
     326,   327,   330,   332,   332,   336,   338,   338,   338,   342,
     344,   345,   346,   347,   344,   352,   354,   355,   355,   354,
     359,   361,   362,   362,   361,   366,   368,   368,   372,   374,
     374,   378,   380,   380,   380,   381,   381,   382,   382,   382,
     383,   383,   388,   388,   389,   394,   395,   396,   397,   398,
     399,   400,   401,   406,   407,   412,   413,   414,   415,   416,
     417,   421,   422,   427,   428,   429,   430,   436,   437,   442,
     443,   448,   449,   449,   451,   451,   458,   459,   460,   461,
     465,   467,   468,   473,   474,   475,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   497,   498,   498,   499,   499,   500,
     500,   501,   502,   503,   504,   505,   506,   507,   508,   509,
     510,   511,   512,   513,   514,   515,   516,   517,   518,   519,
     520,   521,   522,   523,   524,   525,   526,   525,   528,   529,
     528,   531,   532,   533,   532,   535,   536,   537,   538,   536,
     540,   541,   542,   543,   544,   545,   546,   547,   548,   548,
     549,   550,   551,   552,   553,   557,   558,   559,   560,   561,
     562,   563,   564,   568,   569,   570,   574,   580,   581,   586,
     587,   592,   593,   599,   600,   601,   605,   612,   621,   628,
     643,   644,   645,   646,   646,   651,   652,   653,   657,   657,
     662,   663,   664,   664,   665,   665,   670,   671,   675,   676,
     677,   678,   683,   684,   685,   686,   687,   688,   689,   690,
     691,   692,   693,   694,   695,   696,   697,   698,   699,   700,
     701,   702,   703,   704,   709,   710,   711,   715,   717,   718
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "','", "LOGICAL_OR", "LOGICAL_XOR",
  "LOGICAL_AND", "PHP_PRINT", "'='", "PLUS_EQUAL", "MINUS_EQUAL",
  "MUL_EQUAL", "DIV_EQUAL", "CONCAT_EQUAL", "MOD_EQUAL", "AND_EQUAL",
  "OR_EQUAL", "XOR_EQUAL", "SHIFT_LEFT_EQUAL", "SHIFT_RIGHT_EQUAL", "'?'",
  "':'", "BOOLEAN_OR", "BOOLEAN_AND", "'|'", "'^'", "'&'", "IS_EQUAL",
  "IS_NOT_EQUAL", "'<'", "IS_SMALLER_OR_EQUAL", "'>'",
  "IS_GREATER_OR_EQUAL", "SHIFT_LEFT", "SHIFT_RIGHT", "'+'", "'-'", "'.'",
  "'*'", "'/'", "'%'", "'!'", "'~'", "INCREMENT", "DECREMENT", "INT_CAST",
  "DOUBLE_CAST", "STRING_CAST", "ARRAY_CAST", "OBJECT_CAST", "'@'", "'['",
  "NEW", "EXIT", "IF", "ELSEIF", "ELSE", "ENDIF", "LNUMBER", "DNUMBER",
  "STRING", "NUM_STRING", "INLINE_HTML", "CHARACTER", "BAD_CHARACTER",
  "ENCAPSED_AND_WHITESPACE", "PHP_ECHO", "DO", "WHILE", "ENDWHILE", "FOR",
  "ENDFOR", "SWITCH", "ENDSWITCH", "CASE", "DEFAULT", "BREAK", "CONTINUE",
  "CONTINUED_WHILE", "CONTINUED_DOWHILE", "CONTINUED_FOR", "OLD_FUNCTION",
  "FUNCTION", "IC_FUNCTION", "PHP_CONST", "RETURN", "INCLUDE", "REQUIRE",
  "HIGHLIGHT_FILE", "HIGHLIGHT_STRING", "PHP_GLOBAL", "PHP_STATIC",
  "PHP_UNSET", "PHP_ISSET", "PHP_EMPTY", "CLASS", "EXTENDS",
  "PHP_CLASS_OPERATOR", "PHP_DOUBLE_ARROW", "PHP_LIST", "PHP_ARRAY", "VAR",
  "EVAL", "DONE_EVAL", "PHP_LINE", "PHP_FILE", "STRING_CONSTANT", "'{'",
  "'}'", "'('", "')'", "';'", "'$'", "'`'", "'\"'", "'\\''", "']'",
  "$accept", "statement_list", "statement", "$@1", "$@2", "$@3", "$@4",
  "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11", "$@12", "$@13",
  "$@14", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20", "for_statement",
  "switch_case_list", "while_statement", "while_iterations", "$@21",
  "do_while_iterations", "$@22", "$@23", "for_iterations", "$@24", "$@25",
  "$@26", "$@27", "elseif_list", "$@28", "$@29", "$@30", "new_elseif_list",
  "$@31", "@32", "$@33", "else_single", "$@34", "new_else_single", "$@35",
  "case_list", "$@36", "$@37", "$@38", "$@39", "$@40", "$@41",
  "parameter_list", "$@42", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_function_call_parameter_list",
  "global_var_list", "static_var_list", "unambiguous_static_assignment",
  "class_statement_list", "class_statement", "$@43", "$@44",
  "class_variable_declaration", "echo_expr_list", "for_expr",
  "expr_without_variable", "$@45", "$@46", "$@47", "$@48", "$@49", "$@50",
  "$@51", "$@52", "$@53", "$@54", "@55", "$@56", "$@57", "$@58", "scalar",
  "signed_scalar", "varname_scalar", "expr", "unambiguous_variable_name",
  "unambiguous_array_name", "unambiguous_class_name", "assignment_list",
  "assignment_variable_pointer", "$@59", "var", "multi_dimensional_array",
  "$@60", "dimensions", "@61", "@62", "array_pair_list",
  "non_empty_array_pair_list", "encaps_list", "internal_functions_in_yacc",
  "possible_function_call", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,    44,   258,   259,   260,   261,    61,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
      63,    58,   273,   274,   124,    94,    38,   275,   276,    60,
     277,    62,   278,   279,   280,    43,    45,    46,    42,    47,
      37,    33,   126,   281,   282,   283,   284,   285,   286,   287,
      64,    91,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   123,   125,    40,
      41,    59,    36,    96,    34,    39,    93
};
# endif

#define YYPACT_NINF -432

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-432)))

#define YYTABLE_NINF -263

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-263)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -432,   578,  -432,  2278,  2278,  2278,  2278,  2278,   -66,   -66,
    2278,  2278,  2278,  2278,  2278,   377,   -10,   -60,   -57,  -432,
    -432,   -43,  -432,  2278,  -432,   -37,  -432,   -32,  1942,  1968,
      24,    30,    49,  2051,  2278,  -432,  2278,     8,    68,    91,
      17,    95,   101,   110,   102,   109,   111,  -432,  -432,  -432,
    -432,  2278,  -432,   -24,  -432,  -432,  -432,  -432,  -432,  -432,
    -432,  2418,   749,  -432,  -432,  3436,   168,   168,  -432,  -432,
     -24,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  2278,
    -432,   -24,   112,  2077,  2278,    36,  3344,  1853,  2278,   123,
    2278,  -432,  2465,  -432,  2508,  -432,   124,  -432,  -432,  2555,
    2598,  2278,  2645,  2278,   -10,   235,   -10,   236,   -66,   -66,
     -66,   144,   -66,  2278,  2278,   778,   564,  2278,     4,   137,
     197,   153,  -432,   -30,   242,  2455,  2545,  -432,  2278,  -432,
    -432,  -432,  -432,  2278,  2278,  2278,  2278,  2278,  2278,  2278,
    2278,  2278,  2278,  2278,  2278,  2278,  2278,  2278,  2278,  2278,
    -432,  2278,  2278,  2278,  2278,  2278,  2278,  2278,  2278,  2278,
    2278,  2278,  2278,  -432,  -432,   142,  -432,   157,   158,  -432,
     207,  -432,  -432,   150,  -432,  2735,  2778,  2278,  -432,   193,
    2824,  2278,  2867,  -432,  -432,   154,  -432,   156,  -432,  -432,
    2688,  -432,  2329,  -432,   167,   254,   172,   161,   170,   175,
     229,   187,    58,  -432,  3223,   185,   293,  2913,  -432,  -432,
    -432,  3180,  -432,   -24,  -432,  -432,  -432,  -432,  -432,  -432,
    -432,  -432,  -432,   -13,  -432,  -432,  -432,  -432,  2278,  3415,
    2278,  2278,  2278,  2278,   744,  3505,  3519,  3533,  3533,  1490,
    1490,  1490,  1490,   290,   290,   168,   168,   168,  -432,  -432,
    -432,  3436,  3436,  3436,  3436,  3436,  3436,  3436,  3436,  3436,
    3436,  3436,  3436,  2167,   -24,  -432,  -432,   276,  3344,  -432,
    -432,   295,  3344,  -432,   191,    15,   194,   194,  -432,  2278,
     190,   -10,   177,   -10,  -432,  -432,  -432,  -432,  -432,   -66,
     300,  2278,  -432,  2278,   198,   259,   199,     7,  -432,   268,
     212,     9,    -9,  3380,  3436,  3304,  3473,  3490,   -66,   221,
     330,    59,  3344,   435,  2167,  -432,  1853,   225,   867,  2278,
     226,    14,  -432,   230,   231,   278,   338,   234,   237,  2956,
    -432,  -432,   208,   208,  -432,  2278,  -432,  -432,  -432,   337,
     241,   133,  -432,  2278,  3344,  3267,  -432,   301,   243,  2278,
    -432,   244,   -23,   291,   -22,   297,  -432,  -432,  -432,  2197,
     251,  -432,  -432,  2278,  -432,  -432,  -432,  3344,  2278,    -6,
      98,  -432,   956,   278,   278,   354,   140,   256,   257,   255,
    -432,  -432,  3002,   177,  -432,   307,   308,   258,  -432,  -432,
    3436,  2278,   696,  -432,   159,   199,  2167,   260,   261,   311,
    -432,   -16,   312,  -432,   265,  2278,   -25,   -66,    61,  2417,
    -432,  1853,    57,  3045,  1045,  -432,   295,  2278,    -4,    54,
     302,    54,   266,   269,  -432,  -432,   208,   267,   270,   278,
    -432,  -432,  -432,  -432,  -432,   141,  -432,   272,   278,    50,
    3344,  -432,   690,  -432,   301,   273,  -432,  -432,   274,   279,
     280,   325,   284,  -432,  3455,   334,   339,  -432,  -432,   -25,
     121,   288,  -432,  -432,   287,   292,   323,   294,  2375,  -432,
    -432,   331,   296,   303,  -432,  -432,  -432,   278,   278,   400,
    1134,  1223,  -432,   154,  -432,   401,   298,  -432,  -432,  -432,
    -432,   320,   324,   299,  -432,   154,   332,  -432,   333,   395,
    -432,  -432,  1853,  -432,  -432,   346,  2278,  -432,  -432,  -432,
    -432,   322,  -432,  -432,  -432,  -432,   208,  -432,  -432,   347,
     194,  2278,   278,   -25,  -432,  -432,   326,   348,   194,  -432,
    -432,   402,  2278,  -432,  -432,  2278,   295,  -432,  -432,  1853,
    1853,  -432,  -432,  -432,   350,  3344,   450,  -432,  -432,  -432,
     351,  2278,  -432,   352,  3344,   383,  3091,   357,  1853,  1853,
    1312,   361,  2278,  1401,   365,  3344,  1853,  -432,   363,  -432,
    -432,  1497,    54,    54,   353,  -432,  3344,   364,  -432,   370,
    -432,  1853,   867,  -432,  -432,  -432,  -432,  -432,  -432,  1586,
    -432,  1675,   453,  1853,   417,  -432,  1764,  -432,  -432,  -432,
    -432,  -432,  -432,   376,   408,  -432,   386,  -432,  -432,  1853,
    2278,   387,  3134,  2278,   390,   295,  -432,   391,  2278,   295,
     393,  2278,   295,   388,  1497,  -432
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,   225,
     226,   230,    44,   140,    13,     0,    17,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   231,   232,   229,
       3,     0,    53,     0,   293,   293,   293,     2,   238,   221,
     256,     0,   237,   198,   201,   224,   183,   184,   185,   186,
       0,   160,   162,   210,   211,   212,   213,   214,   220,     0,
     236,     0,   205,     0,     0,     0,   142,     0,     0,     0,
       0,    25,     0,    27,     0,    29,     0,    33,    35,     0,
       0,     0,     0,     0,     0,    37,     0,    38,     0,     0,
       0,    39,   249,   266,     0,     0,     0,     0,   256,   250,
       0,     0,   239,   251,     0,     0,     0,   167,     0,   169,
     195,   163,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   159,   161,     0,   250,     0,   251,   219,
     256,   255,   257,     0,   216,     0,     0,     0,    43,     0,
       0,   143,     0,    26,    28,   102,    31,     0,    36,    52,
       0,    48,     0,   122,     0,   125,     0,     0,     0,     0,
       0,     0,     0,   247,   271,     0,   267,     0,     4,   194,
     193,     0,   258,     0,   288,   283,   284,   286,   287,   285,
     292,   290,   291,     0,   223,   289,   227,   228,     0,   171,
       0,     0,     0,     0,   172,   173,   174,   187,   188,   189,
     190,   191,   192,   181,   182,   176,   177,   175,   178,   179,
     180,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   114,     0,   207,   217,     5,   141,    14,
      10,    18,   145,    23,     0,     0,   102,   102,    47,     0,
       0,     0,     0,     0,   294,   295,   296,    41,   130,   248,
       0,     0,   222,     0,     0,   240,   264,   256,   252,     0,
     239,   272,     0,   168,   170,     0,   164,   166,     0,     0,
     113,   238,     0,   237,   114,     7,     0,     0,     0,     0,
       0,     0,     3,     0,     0,     0,   103,     0,     0,     0,
      49,   121,     0,     0,   230,     0,   126,   233,   127,   123,
       0,     0,   246,     0,   270,   269,    51,   259,     0,     0,
     253,     0,     0,     0,     0,     0,   196,   117,   199,     0,
       0,     3,    75,     0,     3,    60,    11,   144,   143,    91,
      91,    24,     0,     0,     0,   105,     0,     0,     0,     0,
     234,   235,     0,     0,   130,     0,     0,     0,    40,   129,
     146,     0,     0,   263,     0,   264,   114,     0,     0,     0,
     276,     0,     0,   277,     0,     0,   297,     0,   238,   237,
     208,    80,    85,     0,     0,    62,    19,     0,     0,    91,
       0,    91,     0,     0,   106,   107,     0,     0,     0,     0,
       3,     3,    50,   128,   124,     0,   132,     0,     0,     0,
     268,   261,     0,   265,   254,     0,   273,   274,     0,     0,
       0,     0,     0,   282,   197,     0,     0,   200,   120,   297,
      88,     0,    86,     6,     0,     0,    12,     0,     0,    95,
     100,     0,     0,     0,    56,    30,   108,     0,     0,   109,
       0,     0,    42,   102,   134,   138,     0,   131,   260,   203,
     275,     0,     0,     0,   281,   102,     0,   209,     0,     0,
       8,    76,     0,    15,    61,     0,   143,    92,    97,     3,
       3,     0,    58,    57,   110,   111,     0,    32,    34,     0,
     102,     0,     0,   297,   278,   279,     0,     0,   102,    81,
      89,     0,     0,    87,    65,     0,    20,     3,     3,    96,
     101,    59,   112,     3,     0,   139,   136,   204,   280,     3,
       0,     0,     3,     0,    77,    16,     0,     0,    93,    98,
       0,     0,     0,     0,     0,    82,    90,     9,     0,    66,
      63,     0,    91,    91,     0,     3,   137,     0,     3,     0,
      78,     0,     0,     3,    54,    21,    94,    99,   133,     0,
     298,     0,     0,     0,     0,    64,     0,    69,   135,   299,
      83,    79,    67,     0,    22,     3,     0,    55,    70,    84,
       0,     0,     0,   143,     0,    71,    68,     0,   143,    72,
       0,   143,    73,     0,     0,    74
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -432,   -44,   -71,  -432,  -432,  -432,  -432,  -432,  -432,  -432,
    -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,
    -432,  -432,  -432,  -127,  -432,   -76,  -432,  -432,  -432,  -432,
    -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,
    -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -356,  -432,
    -432,  -432,  -432,  -432,  -432,  -264,  -432,  -432,  -280,  -432,
    -432,  -432,   126,   128,  -432,  -432,  -432,  -432,  -432,  -344,
    -236,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,  -432,
    -432,  -432,  -432,  -432,  -432,  -259,  -311,   -38,    -3,     1,
    -173,   437,  -432,    10,  -432,   -11,    27,  -432,   118,  -432,
    -432,  -432,  -432,   145,  -432,  -431
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    57,   316,   361,   531,   318,   415,    87,   317,
     534,    89,   320,   467,   557,   597,   321,   185,   276,   187,
     201,   340,   101,   585,   371,   366,   466,   582,   555,   581,
     606,   604,   611,   617,   620,   623,   412,   532,   568,   593,
     460,   551,   579,   605,   463,   502,   500,   552,   420,   537,
     572,   509,   538,   573,   510,   274,   275,   326,   309,   310,
     105,   107,   336,   341,   389,   483,   520,   439,    85,   271,
      58,   232,   233,   228,   230,   231,   405,   165,   406,   351,
     523,   173,   314,   459,    79,    59,   338,    60,    61,   298,
     120,   121,   202,    62,   395,    63,   123,   296,   347,   348,
     349,   205,   206,   124,    64,   457
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,    66,    67,    68,    69,    82,   115,    73,    74,    75,
      76,    77,   327,   328,   422,   118,   179,   469,    71,    72,
      86,   380,   381,   337,   416,    92,    94,   311,   497,   401,
      99,   100,   118,   102,   360,   369,    80,   397,   398,   177,
     299,   323,   122,   170,   449,   450,    70,   301,   116,    83,
      80,   354,    84,   486,   119,  -241,   455,   456,  -241,   122,
     352,   289,  -115,   471,  -118,   473,  -236,  -244,   417,   418,
     122,   166,    88,   337,   337,   402,   169,    90,   311,  -257,
     175,   176,   171,   117,    95,   180,   403,   182,    81,   399,
      96,   299,   547,   193,   302,   195,   451,   168,   190,   324,
     192,  -245,    81,   355,  -243,   419,   353,   470,   172,    97,
     204,   207,   461,   462,   211,   476,   445,   103,   197,   198,
     199,   370,   203,   408,   337,   229,   108,   325,   417,   418,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   250,   178,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     311,   487,   536,   127,   128,   129,   427,   337,   290,  -115,
     111,  -118,   417,   418,   268,   297,   498,   499,   272,   130,
     104,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     125,   126,   300,   106,   109,   542,   147,   148,   149,   421,
     110,   112,   332,   333,   385,   386,   586,   587,   113,   519,
     114,  -206,   385,   386,   428,   303,   297,   304,   305,   306,
     307,   527,   181,   186,   387,    19,    20,   334,   194,   196,
     200,   388,   387,   332,   333,   362,  -255,   365,   212,   482,
     213,   263,   429,   122,   264,  -244,   544,   337,  -241,   265,
     312,   269,   282,  -104,   550,   277,    19,    20,   334,   615,
     331,   284,   339,   313,   619,   443,   329,   622,   372,   281,
     285,    47,    48,    49,   283,   286,   335,   375,   344,   287,
     345,    55,    56,   214,   288,   292,   293,   315,   319,   342,
     322,   330,   215,   216,  -104,   217,   218,   219,   343,   346,
    -242,   312,    47,    48,    49,  -262,   367,   411,   357,   350,
     414,  -202,    55,    56,   313,   144,   145,   146,   147,   148,
     149,   358,   382,   359,   363,   424,   425,   368,    80,   220,
     390,   376,   373,   374,   377,   383,   394,   378,   384,   221,
     222,   400,   392,   396,   223,   224,   312,   404,   225,   393,
     413,   410,   426,   430,   431,   272,   432,   436,   437,   409,
     438,   448,   452,   453,   474,   472,   446,   447,    78,   477,
     475,   484,   478,   489,  -218,   493,   480,   481,   440,   442,
     490,   479,   494,   312,   495,   491,   492,   501,   503,   496,
     485,   505,   454,   504,   511,   506,   313,   512,   516,   521,
     522,   513,  -218,  -218,   468,   526,   530,   458,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,   524,  -218,
    -218,   533,   525,   541,   548,  -218,  -218,  -218,  -116,   514,
     515,   528,   529,   151,   152,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   535,   543,   549,   562,   553,
     561,   564,   569,   567,   588,   539,   540,   571,   575,  -218,
    -218,  -218,   578,   580,   600,   590,  -218,  -218,   163,   164,
     592,  -218,  -218,  -218,   546,   602,  -218,   607,   608,  -218,
    -218,  -218,  -218,   558,   559,   610,   613,   625,   624,   560,
     584,   616,   618,   272,   621,   563,   595,   167,   566,   434,
     594,   365,   435,   444,     0,     0,     0,     0,   545,     0,
       0,     0,   601,     0,     0,     0,     0,     0,     0,   554,
       0,   589,   556,     0,   591,     0,     0,     0,     0,   596,
       0,     0,     0,     0,     0,  -116,     0,     0,   565,     0,
       0,     0,     0,   584,     0,     0,     0,     0,     0,   576,
       0,   609,     0,     0,     0,   209,     0,     0,   127,   128,
     129,     0,     0,     0,     0,     0,     0,     0,     2,     0,
       0,     0,     0,     0,   130,     3,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,     0,     0,   612,     0,     0,
     272,     0,     0,     4,     5,   272,     0,     0,   272,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,     0,
      16,    17,    18,     0,     0,     0,    19,    20,    21,     0,
      22,     0,     0,     0,    23,    24,    25,     0,    26,     0,
      27,     0,     0,     0,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,   210,     0,     0,    44,    45,     0,
      46,     0,    47,    48,    49,    50,     0,    51,     0,    52,
      53,    54,    55,    56,   127,   128,   129,     0,     0,     0,
       0,     0,     0,     3,     0,     0,     0,     0,     0,     0,
     130,     0,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,     4,     5,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    16,    17,
       0,     0,     0,     0,    19,    20,    21,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,     3,     0,     0,    40,    41,
      42,     0,   163,   164,     0,    44,    45,     0,     0,     0,
      47,    48,    49,     0,     0,    51,   488,     0,    53,    54,
      55,    56,   441,     4,     5,     0,     0,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,     0,
      16,    17,    18,     0,     0,     0,    19,    20,    21,     0,
      22,     0,     0,     0,    23,    24,    25,     0,    26,     0,
      27,     0,     0,     0,    28,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,     3,     0,     0,    44,    45,     0,
      46,     0,    47,    48,    49,    50,   208,    51,   364,    52,
      53,    54,    55,    56,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     0,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    16,
      17,    18,     0,     0,     0,    19,    20,    21,     0,    22,
       0,     0,     0,    23,    24,    25,     0,    26,     0,    27,
       0,     0,     0,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,     3,     0,     0,    44,    45,     0,    46,
       0,    47,    48,    49,    50,     0,    51,     0,    52,    53,
      54,    55,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    16,    17,
      18,     0,     0,     0,    19,    20,    21,     0,    22,     0,
       0,     0,    23,    24,    25,     0,    26,     0,    27,     0,
       0,     0,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,     3,     0,     0,    44,    45,     0,    46,     0,
      47,    48,    49,    50,     0,    51,   423,    52,    53,    54,
      55,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     0,     0,     0,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,     0,    16,    17,    18,
       0,     0,     0,    19,    20,    21,     0,    22,     0,     0,
       0,    23,    24,    25,   465,    26,     0,    27,     0,     0,
       0,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     3,     0,     0,    44,    45,     0,    46,     0,    47,
      48,    49,    50,     0,    51,     0,    52,    53,    54,    55,
      56,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     0,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,     0,    16,    17,    18,     0,
       0,     0,    19,    20,    21,     0,    22,     0,     0,     0,
      23,    24,    25,     0,    26,     0,    27,     0,     0,     0,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
       3,     0,     0,    44,    45,     0,    46,     0,    47,    48,
      49,    50,   517,    51,     0,    52,    53,    54,    55,    56,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       0,     0,     0,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,     0,    16,    17,    18,     0,     0,
       0,    19,    20,    21,     0,    22,     0,     0,     0,    23,
      24,    25,     0,    26,     0,    27,     0,     0,     0,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,     3,
       0,     0,    44,    45,     0,    46,     0,    47,    48,    49,
      50,   518,    51,     0,    52,    53,    54,    55,    56,     0,
       0,     0,     0,     0,     0,     0,     0,     4,     5,     0,
       0,     0,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,     0,    16,    17,    18,     0,     0,     0,
      19,    20,    21,     0,    22,     0,     0,     0,    23,    24,
      25,     0,    26,     0,    27,     0,     0,     0,    28,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,     3,     0,
       0,    44,    45,     0,    46,     0,    47,    48,    49,    50,
       0,    51,   574,    52,    53,    54,    55,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     5,     0,     0,
       0,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,     0,    16,    17,    18,     0,     0,     0,    19,
      20,    21,     0,    22,     0,     0,     0,    23,    24,    25,
       0,    26,     0,    27,     0,     0,     0,    28,    29,     0,
       0,     0,    30,    31,    32,     0,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,     0,     0,     0,
      44,    45,     0,    46,     3,    47,    48,    49,    50,     0,
      51,   577,    52,    53,    54,    55,    56,     0,   583,  -263,
    -263,  -263,  -263,   142,   143,   144,   145,   146,   147,   148,
     149,     0,     4,     5,     0,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    16,
      17,    18,     0,     0,     0,    19,    20,    21,     0,    22,
       0,     0,     0,    23,    24,    25,     0,    26,     0,    27,
       0,     0,     0,    28,    29,     0,     0,     0,    30,    31,
      32,     0,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,     3,     0,     0,    44,    45,     0,    46,
       0,    47,    48,    49,    50,     0,    51,     0,    52,    53,
      54,    55,    56,     0,     0,     0,     0,     0,     0,     0,
       0,     4,     5,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    16,    17,
      18,     0,     0,     0,    19,    20,    21,     0,    22,     0,
       0,     0,    23,    24,    25,     0,    26,     0,    27,     0,
       0,     0,    28,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,     3,     0,     0,    44,    45,     0,    46,     0,
      47,    48,    49,    50,   598,    51,     0,    52,    53,    54,
      55,    56,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     0,     0,     0,     0,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,     0,    16,    17,    18,
       0,     0,     0,    19,    20,    21,     0,    22,     0,     0,
       0,    23,    24,    25,     0,    26,     0,    27,     0,     0,
       0,    28,    29,     0,     0,     0,    30,    31,    32,     0,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,     3,     0,     0,    44,    45,     0,    46,     0,    47,
      48,    49,    50,   599,    51,     0,    52,    53,    54,    55,
      56,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       5,     0,     0,     0,     0,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,     0,    16,    17,    18,     0,
       0,     0,    19,    20,    21,     0,    22,     0,     0,     0,
      23,    24,    25,     0,    26,   603,    27,     0,     0,     0,
      28,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
       3,     0,     0,    44,    45,     0,    46,     0,    47,    48,
      49,    50,     0,    51,     0,    52,    53,    54,    55,    56,
       0,     0,     0,     0,     0,     0,     0,     0,     4,     5,
       0,     0,     0,     0,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,     0,    16,    17,    18,     0,     0,
       0,    19,    20,    21,     0,    22,     0,     0,     0,    23,
      24,    25,     0,    26,     0,    27,     0,     0,     0,    28,
      29,     0,     0,     0,    30,    31,    32,     0,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,     3,
       0,     0,    44,    45,     0,    46,     0,    47,    48,    49,
      50,     0,    51,     0,    52,    53,    54,    55,    56,     0,
       0,     0,     0,     0,     0,     3,     0,     4,     5,     0,
       0,     0,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,     0,    16,    17,     0,     0,     0,     0,
      19,    20,    21,     4,     5,     0,     0,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,     0,
      16,    17,     0,     0,     0,     0,    19,    20,    21,     0,
       0,     0,     0,     0,    40,    41,    42,     0,     0,     0,
       0,    44,    45,     0,     0,     0,    47,    48,    49,     0,
       0,    51,     0,    91,    53,    54,    55,    56,     3,     0,
      40,    41,    42,     0,     0,     0,     0,    44,    45,     0,
       0,     0,    47,    48,    49,     0,     0,    51,     0,    93,
      53,    54,    55,    56,     3,     0,     4,     5,     0,     0,
       0,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,     0,    16,    17,     0,     0,     0,     0,    19,
      20,    21,     4,     5,     0,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    16,
      17,     0,     0,     0,     0,    19,    20,    21,     0,     0,
       0,     0,     0,    40,    41,    42,     0,     0,     0,     0,
      44,    45,     0,     0,     0,    47,    48,    49,     0,     0,
      51,     0,    98,    53,    54,    55,    56,     0,     0,    40,
      41,    42,     0,     0,     3,     0,    44,    45,     0,     0,
       0,    47,    48,    49,     0,     0,    51,   174,     0,    53,
      54,    55,    56,   308,     0,     0,     0,     0,     0,     0,
       0,     0,     4,     5,     3,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    16,
      17,     0,     0,   407,     0,    19,    20,    21,     0,     0,
       0,     0,     4,     5,     0,     0,     0,     0,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    16,
      17,     0,     0,     0,     0,    19,    20,    21,     0,    40,
      41,    42,     0,     0,     0,     0,    44,    45,     0,     0,
       0,    47,    48,    49,     0,     0,    51,     0,     0,    53,
      54,    55,    56,     0,     0,     3,     0,     0,     0,    40,
      41,    42,     0,     0,     0,     0,    44,    45,     0,     0,
       0,    47,    48,    49,     0,     0,    51,     0,     0,    53,
      54,    55,    56,     4,     5,     0,     0,     0,     0,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,     0,
      16,    17,   279,   127,   128,   129,    19,    20,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   130,
       0,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
      40,    41,    42,     0,     0,     0,     0,    44,    45,   127,
     128,   129,    47,    48,    49,     0,     0,    51,     0,     0,
      53,    54,    55,    56,     0,   130,   507,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,     0,     0,     0,     0,
    -119,     0,   127,   128,   129,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,     0,   130,   280,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,     0,
     163,   164,     0,     0,     0,     0,     0,     0,     0,   127,
     128,   129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   130,   508,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   214,     0,     0,     0,
       0,     0,   127,   128,   129,   215,   216,     0,   217,   218,
     219,     0,     0,     0,     0,     0,     0,  -119,   130,   150,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,   127,
     128,   129,   221,   222,     0,     0,     0,   223,     0,   226,
       0,   225,     0,     0,     0,   130,   183,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   214,     0,     0,     0,
       0,     0,   127,   128,   129,   215,   216,     0,   217,   218,
     219,     0,     0,     0,     0,     0,     0,     0,   130,   184,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,     0,
       0,     0,   220,     0,     0,     0,     0,     0,     0,   127,
     128,   129,   221,   222,     0,     0,     0,   223,     0,     0,
     227,   225,     0,     0,     0,   130,   188,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,     0,     0,     0,     0,
       0,     0,   127,   128,   129,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   130,   189,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   127,
     128,   129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   130,   191,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,     0,     0,     0,     0,
       0,     0,   127,   128,   129,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   130,   278,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   127,   128,
     129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   130,   266,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,     0,     0,     0,     0,     0,
       0,   127,   128,   129,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   130,   267,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   127,   128,   129,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   130,   270,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,     0,     0,     0,     0,     0,     0,
     127,   128,   129,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   130,   273,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   127,   128,   129,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   130,   294,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,     0,     0,     0,     0,     0,     0,   127,
     128,   129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   130,   379,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   127,   128,   129,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   130,   433,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,   141,   142,   143,   144,   145,   146,   147,
     148,   149,     0,     0,     0,     0,     0,     0,   127,   128,
     129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   130,   464,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   127,   128,   129,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     130,   570,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,     0,     0,     0,     0,     0,     0,   127,   128,   129,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   130,   614,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,     0,     0,     0,     0,     0,     0,
       0,   127,   128,   129,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   130,   295,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   127,   128,
     129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   291,     0,     0,   130,   356,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,     0,     0,     0,   127,   128,
     129,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   130,   391,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   128,   129,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     130,     0,   131,   132,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   129,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   130,     0,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   130,     0,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   133,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   135,   136,   137,   138,   139,   140,   141,   142,   143,
     144,   145,   146,   147,   148,   149,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
    -263,  -263,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149
};

static const yytype_int16 yycheck[] =
{
       3,     4,     5,     6,     7,    16,    50,    10,    11,    12,
      13,    14,   276,   277,   370,    53,    87,    21,     8,     9,
      23,   332,   333,   282,   368,    28,    29,   263,   459,    51,
      33,    34,    70,    36,   314,    21,    60,    60,    61,     3,
     213,    26,    53,    81,    60,    61,   112,    60,    51,   109,
      60,    60,   109,     3,    53,    51,    81,    82,    51,    70,
      51,     3,     3,   419,     3,   421,   109,    97,    74,    75,
      81,    70,   109,   332,   333,    97,    79,   109,   314,   109,
      83,    84,    81,   107,    60,    88,   108,    90,   112,   112,
      60,   264,   523,   104,   107,   106,   112,    70,   101,    84,
     103,    97,   112,   112,    97,   111,    97,   111,    81,    60,
     113,   114,    55,    56,   117,   426,   396,   109,   108,   109,
     110,   107,   112,   359,   383,   128,   109,   112,    74,    75,
     133,   134,   135,   136,   137,   138,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,   111,   151,   152,
     153,   154,   155,   156,   157,   158,   159,   160,   161,   162,
     396,   111,   506,     4,     5,     6,    26,   426,   110,   110,
      60,   110,    74,    75,   177,   213,    55,    56,   181,    20,
     112,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      55,    56,   213,   112,   109,   516,    38,    39,    40,   111,
     109,   109,    35,    36,    81,    82,   572,   573,   109,   483,
     109,   109,    81,    82,    84,   228,   264,   230,   231,   232,
     233,   495,   109,   109,   101,    58,    59,    60,     3,     3,
      96,   108,   101,    35,    36,   316,   109,   318,    51,   108,
      97,   109,   112,   264,    97,    97,   520,   516,    51,   109,
     263,    68,     8,   109,   528,   109,    58,    59,    60,   613,
     281,   110,   283,   263,   618,   116,   279,   621,   322,   112,
     110,   104,   105,   106,   112,   110,   109,   325,   291,    60,
     293,   114,   115,    51,   107,   110,     3,    21,     3,   289,
     109,   111,    60,    61,   110,    63,    64,    65,     8,   111,
      51,   314,   104,   105,   106,   116,   319,   361,   308,    51,
     364,   109,   114,   115,   314,    35,    36,    37,    38,    39,
      40,   110,   335,     3,   109,   373,   374,   111,    60,    97,
     343,     3,   112,   112,   110,     8,   349,   110,   107,   107,
     108,    60,    51,   109,   112,   113,   359,    60,   116,   116,
     363,   110,     8,   107,   107,   368,   111,    60,    60,   359,
     112,    60,    60,   108,   108,    73,   116,   116,     1,   112,
     111,   109,   112,   110,     7,    60,   430,   431,   391,   392,
     116,   429,   108,   396,    60,   116,   116,   109,   111,    60,
     438,    78,   405,   111,    73,   111,   396,   111,     8,     8,
     112,   108,    35,    36,   417,   116,    21,   407,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,   108,    52,
      53,   502,   108,   111,   108,    58,    59,    60,     3,   477,
     478,   109,   109,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,   109,   109,   109,     8,    57,
     110,   110,    79,   111,   111,   509,   510,   110,   107,    92,
      93,    94,   107,   110,    21,   111,    99,   100,    43,    44,
     110,   104,   105,   106,   522,    68,   109,   111,    80,   112,
     113,   114,   115,   537,   538,   109,   109,   624,   110,   543,
     571,   111,   111,   506,   111,   549,   582,    70,   552,   383,
     581,   582,   384,   395,    -1,    -1,    -1,    -1,   521,    -1,
      -1,    -1,   593,    -1,    -1,    -1,    -1,    -1,    -1,   532,
      -1,   575,   535,    -1,   578,    -1,    -1,    -1,    -1,   583,
      -1,    -1,    -1,    -1,    -1,   110,    -1,    -1,   551,    -1,
      -1,    -1,    -1,   624,    -1,    -1,    -1,    -1,    -1,   562,
      -1,   605,    -1,    -1,    -1,     1,    -1,    -1,     4,     5,
       6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     0,    -1,
      -1,    -1,    -1,    -1,    20,     7,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,   610,    -1,    -1,
     613,    -1,    -1,    35,    36,   618,    -1,    -1,   621,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,    54,    -1,    -1,    -1,    58,    59,    60,    -1,
      62,    -1,    -1,    -1,    66,    67,    68,    -1,    70,    -1,
      72,    -1,    -1,    -1,    76,    77,    -1,    -1,    -1,    81,
      82,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   110,    -1,    -1,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,    -1,   109,    -1,   111,
     112,   113,   114,   115,     4,     5,     6,    -1,    -1,    -1,
      -1,    -1,    -1,     7,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    35,    36,    -1,    -1,    -1,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    52,    53,
      -1,    -1,    -1,    -1,    58,    59,    60,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,     7,    -1,    -1,    92,    93,
      94,    -1,    43,    44,    -1,    99,   100,    -1,    -1,    -1,
     104,   105,   106,    -1,    -1,   109,   116,    -1,   112,   113,
     114,   115,   116,    35,    36,    -1,    -1,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,    54,    -1,    -1,    -1,    58,    59,    60,    -1,
      62,    -1,    -1,    -1,    66,    67,    68,    -1,    70,    -1,
      72,    -1,    -1,    -1,    76,    77,    -1,    -1,    -1,    81,
      82,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     7,    -1,    -1,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,    21,   111,
     112,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    62,
      -1,    -1,    -1,    66,    67,    68,    -1,    70,    -1,    72,
      -1,    -1,    -1,    76,    77,    -1,    -1,    -1,    81,    82,
      83,    -1,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     7,    -1,    -1,    99,   100,    -1,   102,
      -1,   104,   105,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    -1,    -1,    -1,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    52,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    -1,    62,    -1,
      -1,    -1,    66,    67,    68,    -1,    70,    -1,    72,    -1,
      -1,    -1,    76,    77,    -1,    -1,    -1,    81,    82,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     7,    -1,    -1,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,    -1,   109,   110,   111,   112,   113,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    -1,    -1,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    52,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    -1,    62,    -1,    -1,
      -1,    66,    67,    68,    69,    70,    -1,    72,    -1,    -1,
      -1,    76,    77,    -1,    -1,    -1,    81,    82,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     7,    -1,    -1,    99,   100,    -1,   102,    -1,   104,
     105,   106,   107,    -1,   109,    -1,   111,   112,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    52,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    -1,    62,    -1,    -1,    -1,
      66,    67,    68,    -1,    70,    -1,    72,    -1,    -1,    -1,
      76,    77,    -1,    -1,    -1,    81,    82,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       7,    -1,    -1,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,    -1,   111,   112,   113,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    52,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    -1,    62,    -1,    -1,    -1,    66,
      67,    68,    -1,    70,    -1,    72,    -1,    -1,    -1,    76,
      77,    -1,    -1,    -1,    81,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     7,
      -1,    -1,    99,   100,    -1,   102,    -1,   104,   105,   106,
     107,   108,   109,    -1,   111,   112,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    52,    53,    54,    -1,    -1,    -1,
      58,    59,    60,    -1,    62,    -1,    -1,    -1,    66,    67,
      68,    -1,    70,    -1,    72,    -1,    -1,    -1,    76,    77,
      -1,    -1,    -1,    81,    82,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,     7,    -1,
      -1,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
      -1,   109,   110,   111,   112,   113,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    35,    36,    -1,    -1,
      -1,    -1,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    52,    53,    54,    -1,    -1,    -1,    58,
      59,    60,    -1,    62,    -1,    -1,    -1,    66,    67,    68,
      -1,    70,    -1,    72,    -1,    -1,    -1,    76,    77,    -1,
      -1,    -1,    81,    82,    83,    -1,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    -1,    -1,    -1,
      99,   100,    -1,   102,     7,   104,   105,   106,   107,    -1,
     109,   110,   111,   112,   113,   114,   115,    -1,    21,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    35,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    54,    -1,    -1,    -1,    58,    59,    60,    -1,    62,
      -1,    -1,    -1,    66,    67,    68,    -1,    70,    -1,    72,
      -1,    -1,    -1,    76,    77,    -1,    -1,    -1,    81,    82,
      83,    -1,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     7,    -1,    -1,    99,   100,    -1,   102,
      -1,   104,   105,   106,   107,    -1,   109,    -1,   111,   112,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    -1,    -1,    -1,    -1,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    52,    53,
      54,    -1,    -1,    -1,    58,    59,    60,    -1,    62,    -1,
      -1,    -1,    66,    67,    68,    -1,    70,    -1,    72,    -1,
      -1,    -1,    76,    77,    -1,    -1,    -1,    81,    82,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     7,    -1,    -1,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,    -1,   111,   112,   113,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    -1,    -1,    -1,    -1,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    52,    53,    54,
      -1,    -1,    -1,    58,    59,    60,    -1,    62,    -1,    -1,
      -1,    66,    67,    68,    -1,    70,    -1,    72,    -1,    -1,
      -1,    76,    77,    -1,    -1,    -1,    81,    82,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     7,    -1,    -1,    99,   100,    -1,   102,    -1,   104,
     105,   106,   107,   108,   109,    -1,   111,   112,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    52,    53,    54,    -1,
      -1,    -1,    58,    59,    60,    -1,    62,    -1,    -1,    -1,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      76,    77,    -1,    -1,    -1,    81,    82,    83,    -1,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       7,    -1,    -1,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,    -1,   109,    -1,   111,   112,   113,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    36,
      -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    52,    53,    54,    -1,    -1,
      -1,    58,    59,    60,    -1,    62,    -1,    -1,    -1,    66,
      67,    68,    -1,    70,    -1,    72,    -1,    -1,    -1,    76,
      77,    -1,    -1,    -1,    81,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     7,
      -1,    -1,    99,   100,    -1,   102,    -1,   104,   105,   106,
     107,    -1,   109,    -1,   111,   112,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,     7,    -1,    35,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    52,    53,    -1,    -1,    -1,    -1,
      58,    59,    60,    35,    36,    -1,    -1,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,    -1,    -1,    -1,    -1,    58,    59,    60,    -1,
      -1,    -1,    -1,    -1,    92,    93,    94,    -1,    -1,    -1,
      -1,    99,   100,    -1,    -1,    -1,   104,   105,   106,    -1,
      -1,   109,    -1,   111,   112,   113,   114,   115,     7,    -1,
      92,    93,    94,    -1,    -1,    -1,    -1,    99,   100,    -1,
      -1,    -1,   104,   105,   106,    -1,    -1,   109,    -1,   111,
     112,   113,   114,   115,     7,    -1,    35,    36,    -1,    -1,
      -1,    -1,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    52,    53,    -1,    -1,    -1,    -1,    58,
      59,    60,    35,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    -1,    -1,    -1,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    -1,    92,    93,    94,    -1,    -1,    -1,    -1,
      99,   100,    -1,    -1,    -1,   104,   105,   106,    -1,    -1,
     109,    -1,   111,   112,   113,   114,   115,    -1,    -1,    92,
      93,    94,    -1,    -1,     7,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,    -1,    -1,   109,   110,    -1,   112,
     113,   114,   115,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    35,    36,     7,    -1,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    -1,    -1,    26,    -1,    58,    59,    60,    -1,    -1,
      -1,    -1,    35,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    -1,    -1,    -1,    -1,    58,    59,    60,    -1,    92,
      93,    94,    -1,    -1,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,    -1,    -1,   109,    -1,    -1,   112,
     113,   114,   115,    -1,    -1,     7,    -1,    -1,    -1,    92,
      93,    94,    -1,    -1,    -1,    -1,    99,   100,    -1,    -1,
      -1,   104,   105,   106,    -1,    -1,   109,    -1,    -1,   112,
     113,   114,   115,    35,    36,    -1,    -1,    -1,    -1,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      52,    53,     3,     4,     5,     6,    58,    59,    60,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,
      -1,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      92,    93,    94,    -1,    -1,    -1,    -1,    99,   100,     4,
       5,     6,   104,   105,   106,    -1,    -1,   109,    -1,    -1,
     112,   113,   114,   115,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,
       3,    -1,     4,     5,     6,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    -1,    20,   110,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      43,    44,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,   111,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    51,    -1,    -1,    -1,
      -1,    -1,     4,     5,     6,    60,    61,    -1,    63,    64,
      65,    -1,    -1,    -1,    -1,    -1,    -1,   110,    20,   111,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,     4,
       5,     6,   107,   108,    -1,    -1,    -1,   112,    -1,   114,
      -1,   116,    -1,    -1,    -1,    20,   111,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    51,    -1,    -1,    -1,
      -1,    -1,     4,     5,     6,    60,    61,    -1,    63,    64,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,   111,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,     4,
       5,     6,   107,   108,    -1,    -1,    -1,   112,    -1,    -1,
     115,   116,    -1,    -1,    -1,    20,   111,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,   111,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,   111,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,   111,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,
       6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,   110,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,   110,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,   110,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
       4,     5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    20,   110,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    20,   110,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,     4,
       5,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,   110,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,     5,     6,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    20,   110,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,
       6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,   110,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     4,     5,     6,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,   110,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    -1,    -1,    -1,    -1,     4,     5,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    20,   110,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    20,   108,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,     4,     5,
       6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,     4,     5,
       6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    20,    98,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,     5,     6,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      20,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    20,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    20,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   118,     0,     7,    35,    36,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    52,    53,    54,    58,
      59,    60,    62,    66,    67,    68,    70,    72,    76,    77,
      81,    82,    83,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    99,   100,   102,   104,   105,   106,
     107,   109,   111,   112,   113,   114,   115,   119,   187,   202,
     204,   205,   210,   212,   221,   205,   205,   205,   205,   205,
     112,   210,   210,   205,   205,   205,   205,   205,     1,   201,
      60,   112,   212,   109,   109,   185,   205,   125,   109,   128,
     109,   111,   205,   111,   205,    60,    60,    60,   111,   205,
     205,   139,   205,   109,   112,   177,   112,   178,   109,   109,
     109,    60,   109,   109,   109,   118,   205,   107,   204,   206,
     207,   208,   212,   213,   220,   220,   220,     4,     5,     6,
      20,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
     111,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    43,    44,   194,   206,   208,   213,   205,
     204,   206,   213,   198,   110,   205,   205,     3,   111,   119,
     205,   109,   205,   111,   111,   134,   109,   136,   111,   111,
     205,   111,   205,   212,     3,   212,     3,   210,   210,   210,
      96,   137,   209,   210,   205,   218,   219,   205,   108,     1,
     110,   205,    51,    97,    51,    60,    61,    63,    64,    65,
      97,   107,   108,   112,   113,   116,   114,   115,   190,   205,
     191,   192,   188,   189,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   205,   205,   205,
     205,   205,   205,   109,    97,   109,   110,   110,   205,    68,
     110,   186,   205,   110,   172,   173,   135,   109,   111,     3,
     110,   112,     8,   112,   110,   110,   110,    60,   107,     3,
     110,    98,   110,     3,   110,   108,   214,   204,   206,   207,
     212,    60,   107,   205,   205,   205,   205,   205,    26,   175,
     176,   187,   205,   210,   199,    21,   120,   126,   123,     3,
     129,   133,   109,    26,    84,   112,   174,   172,   172,   205,
     111,   212,    35,    36,    60,   109,   179,   202,   203,   212,
     138,   180,   210,     8,   205,   205,   111,   215,   216,   217,
      51,   196,    51,    97,    60,   112,    21,   210,   110,     3,
     175,   121,   119,   109,    21,   119,   142,   205,   111,    21,
     107,   141,   118,   112,   112,   204,     3,   110,   110,   110,
     203,   203,   205,     8,   107,    81,    82,   101,   108,   181,
     205,    98,    51,   116,   205,   211,   109,    60,    61,   112,
      60,    51,    97,   108,    60,   193,   195,    26,   187,   210,
     110,   118,   153,   205,   118,   124,   186,    74,    75,   111,
     165,   111,   165,   110,   204,   204,     8,    26,    84,   112,
     107,   107,   111,   110,   179,   180,    60,    60,   112,   184,
     205,   116,   205,   116,   215,   175,   116,   116,    60,    60,
      61,   112,    60,   108,   205,    81,    82,   222,   210,   200,
     157,    55,    56,   161,   110,    69,   143,   130,   205,    21,
     111,   165,    73,   165,   108,   111,   203,   112,   112,   204,
     118,   118,   108,   182,   109,   204,     3,   111,   116,   110,
     116,   116,   116,    60,   108,    60,    60,   222,    55,    56,
     163,   109,   162,   111,   111,    78,   111,    21,   111,   168,
     171,    73,   111,   108,   204,   204,     8,   108,   108,   172,
     183,     8,   112,   197,   108,   108,   116,   172,   109,   109,
      21,   122,   154,   119,   127,   109,   186,   166,   169,   118,
     118,   111,   203,   109,   172,   205,   204,   222,   108,   109,
     172,   158,   164,    57,   205,   145,   205,   131,   118,   118,
     118,   110,     8,   118,   110,   205,   118,   111,   155,    79,
     110,   110,   167,   170,   110,   107,   205,   110,   107,   159,
     110,   146,   144,    21,   119,   140,   165,   165,   111,   118,
     111,   118,   110,   156,   119,   142,   118,   132,   108,   108,
      21,   119,    68,    71,   148,   160,   147,   111,    80,   118,
     109,   149,   205,   109,   110,   186,   111,   150,   111,   186,
     151,   111,   186,   152,   110,   140
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   117,   118,   118,   119,   120,   119,   121,   122,   119,
     123,   124,   119,   125,   126,   127,   119,   128,   129,   130,
     131,   132,   119,   133,   119,   119,   119,   119,   119,   134,
     119,   135,   119,   136,   119,   119,   119,   119,   119,   137,
     119,   138,   119,   119,   119,   119,   139,   119,   119,   119,
     119,   119,   119,   119,   140,   140,   141,   141,   141,   141,
     142,   142,   143,   144,   143,   145,   146,   147,   145,   148,
     149,   150,   151,   152,   148,   153,   154,   155,   156,   153,
     157,   158,   159,   160,   157,   161,   162,   161,   163,   164,
     163,   165,   166,   167,   165,   168,   165,   169,   170,   165,
     171,   165,   173,   172,   172,   174,   174,   174,   174,   174,
     174,   174,   174,   175,   175,   176,   176,   176,   176,   176,
     176,   177,   177,   178,   178,   178,   178,   179,   179,   180,
     180,   181,   182,   181,   183,   181,   184,   184,   184,   184,
     185,   185,   185,   186,   186,   186,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   188,   187,   189,   187,   190,   187,   191,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   187,   187,
     187,   187,   187,   187,   187,   192,   193,   187,   194,   195,
     187,   187,   196,   197,   187,   187,   198,   199,   200,   187,
     187,   187,   187,   187,   187,   187,   187,   187,   201,   187,
     187,   187,   187,   187,   187,   202,   202,   202,   202,   202,
     202,   202,   202,   203,   203,   203,   204,   205,   205,   206,
     206,   207,   207,   208,   208,   208,   209,   209,   209,   209,
     210,   210,   210,   211,   210,   212,   212,   212,   214,   213,
     215,   215,   216,   215,   217,   215,   218,   218,   219,   219,
     219,   219,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   220,   220,   220,   220,   220,   220,   220,
     220,   220,   220,   220,   221,   221,   221,   222,   222,   222
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     3,     0,     8,     0,     0,    12,
       0,     0,     8,     0,     0,     0,    11,     0,     0,     0,
       0,     0,    15,     0,     6,     2,     3,     2,     3,     0,
       8,     0,     9,     0,     9,     2,     3,     2,     2,     0,
       6,     0,     8,     3,     1,     2,     0,     4,     3,     5,
       7,     5,     3,     1,     1,     4,     3,     4,     4,     5,
       1,     4,     0,     0,     7,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    14,     0,     0,     0,     0,     9,
       0,     0,     0,     0,    10,     0,     0,     3,     0,     0,
       4,     0,     0,     0,     7,     0,     4,     0,     0,     7,
       0,     4,     0,     2,     0,     2,     3,     3,     4,     4,
       5,     5,     6,     1,     0,     1,     1,     2,     3,     3,
       4,     4,     2,     4,     6,     2,     4,     1,     3,     2,
       0,     3,     0,     8,     0,     9,     4,     6,     2,     4,
       0,     3,     1,     0,     3,     1,     6,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     0,     4,     0,     4,     0,     4,     0,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     0,     0,     7,     0,     0,
       7,     1,     0,     0,    10,     2,     0,     0,     0,     9,
       2,     2,     2,     2,     2,     1,     3,     4,     0,     3,
       2,     1,     4,     3,     2,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     2,     2,     1,     1,     1,     1,
       3,     1,     3,     3,     1,     1,     3,     1,     2,     0,
       2,     2,     4,     0,     7,     2,     1,     2,     0,     4,
       4,     3,     0,     2,     0,     3,     0,     1,     5,     3,
       3,     1,     3,     6,     6,     7,     5,     5,     8,     8,
       9,     7,     6,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     0,     4,     4,     4,     0,     7,     8
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 275 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_if (&(yyvsp[-1]) _INLINE_TLS); }
#line 2617 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 275 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_if ( _INLINE_TLS_VOID); }
#line 2623 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 276 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_if (&(yyvsp[-2]) _INLINE_TLS); }
#line 2629 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 276 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_if ( _INLINE_TLS_VOID); }
#line 2635 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 277 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_while(&(yyvsp[-3]), &(yyvsp[-1]) _INLINE_TLS); }
#line 2641 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 277 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_while(&(yyvsp[-5]),&yychar _INLINE_TLS); }
#line 2647 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 278 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_do_while(&(yyvsp[0]) _INLINE_TLS); }
#line 2653 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 278 "./language-parser.y" /* yacc.c:1646  */
    { cs_force_eval_do_while( _INLINE_TLS_VOID); }
#line 2659 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 278 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_do_while(&(yyvsp[-8]),&(yyvsp[-2]),&yychar _INLINE_TLS); }
#line 2665 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 279 "./language-parser.y" /* yacc.c:1646  */
    { for_pre_expr1(&(yyvsp[0]) _INLINE_TLS); }
#line 2671 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 280 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) pval_destructor(&(yyvsp[0]) _INLINE_TLS); for_pre_expr2(&(yyvsp[-3]) _INLINE_TLS); }
#line 2677 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 281 "./language-parser.y" /* yacc.c:1646  */
    { for_pre_expr3(&(yyvsp[-6]),&(yyvsp[0]) _INLINE_TLS); }
#line 2683 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 282 "./language-parser.y" /* yacc.c:1646  */
    { for_pre_statement(&(yyvsp[-9]),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 2689 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 283 "./language-parser.y" /* yacc.c:1646  */
    { for_post_statement(&(yyvsp[-12]),&(yyvsp[-7]),&(yyvsp[-4]),&(yyvsp[-1]),&yychar _INLINE_TLS); }
#line 2695 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 284 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_start(&(yyvsp[-3]),&(yyvsp[-1]) _INLINE_TLS); }
#line 2701 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 284 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_end(&(yyvsp[-3]) _INLINE_TLS); }
#line 2707 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 285 "./language-parser.y" /* yacc.c:1646  */
    { DO_OR_DIE(cs_break_continue(NULL,DO_BREAK _INLINE_TLS)) }
#line 2713 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 286 "./language-parser.y" /* yacc.c:1646  */
    { DO_OR_DIE(cs_break_continue(&(yyvsp[-1]),DO_BREAK _INLINE_TLS)) }
#line 2719 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 287 "./language-parser.y" /* yacc.c:1646  */
    { DO_OR_DIE(cs_break_continue(NULL,DO_CONTINUE _INLINE_TLS)) }
#line 2725 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 288 "./language-parser.y" /* yacc.c:1646  */
    { DO_OR_DIE(cs_break_continue(&(yyvsp[-1]),DO_CONTINUE _INLINE_TLS)) }
#line 2731 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 289 "./language-parser.y" /* yacc.c:1646  */
    { start_function_decleration(_INLINE_TLS_VOID); }
#line 2737 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 290 "./language-parser.y" /* yacc.c:1646  */
    { end_function_decleration(&(yyvsp[-7]),&(yyvsp[-6]) _INLINE_TLS); }
#line 2743 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 291 "./language-parser.y" /* yacc.c:1646  */
    { start_function_decleration(_INLINE_TLS_VOID); }
#line 2749 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 292 "./language-parser.y" /* yacc.c:1646  */
    { end_function_decleration(&(yyvsp[-8]),&(yyvsp[-7]) _INLINE_TLS); }
#line 2755 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 293 "./language-parser.y" /* yacc.c:1646  */
    { tc_set_token(&GLOBAL(token_cache_manager), (yyvsp[-1]).offset, FUNCTION); }
#line 2761 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 293 "./language-parser.y" /* yacc.c:1646  */
    { if (!GLOBAL(shutdown_requested)) GLOBAL(shutdown_requested) = TERMINATE_CURRENT_PHPPARSE; }
#line 2767 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 294 "./language-parser.y" /* yacc.c:1646  */
    { cs_return(NULL _INLINE_TLS); }
#line 2773 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 295 "./language-parser.y" /* yacc.c:1646  */
    { cs_return(&(yyvsp[-1]) _INLINE_TLS); }
#line 2779 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 298 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_class_decleration(&(yyvsp[0]),NULL _INLINE_TLS); }
#line 2785 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 298 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_class_decleration( _INLINE_TLS_VOID); }
#line 2791 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 299 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_class_decleration(&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 2797 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 299 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_class_decleration( _INLINE_TLS_VOID); }
#line 2803 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 301 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { if (php3_header()) PUTS((yyvsp[0]).value.str.val); } }
#line 2809 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 302 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) pval_destructor(&(yyvsp[-1]) _INLINE_TLS); }
#line 2815 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 303 "./language-parser.y" /* yacc.c:1646  */
    { php3cs_start_require(&(yyvsp[0]) _INLINE_TLS); }
#line 2821 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 303 "./language-parser.y" /* yacc.c:1646  */
    { php3cs_end_require(&(yyvsp[-3]),&(yyvsp[-1]) _INLINE_TLS); }
#line 2827 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 304 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) cs_show_source(&(yyvsp[-1]) _INLINE_TLS); }
#line 2833 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 305 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) eval_string(&(yyvsp[-2]),NULL,1 _INLINE_TLS); }
#line 2839 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 306 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) eval_string(&(yyvsp[-4]),&(yyvsp[-2]),2 _INLINE_TLS); }
#line 2845 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 307 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) eval_string(&(yyvsp[-2]),&(yyvsp[0]),0 _INLINE_TLS); }
#line 2851 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 308 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) conditional_include_file(&(yyvsp[-1]),&(yyvsp[0]) _INLINE_TLS); }
#line 2857 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 332 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_while(&(yyvsp[-3]), &(yyvsp[-1]) _INLINE_TLS); }
#line 2863 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 332 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_while(&(yyvsp[-5]),&yychar _INLINE_TLS); }
#line 2869 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 338 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_do_while(&(yyvsp[0]) _INLINE_TLS); }
#line 2875 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 338 "./language-parser.y" /* yacc.c:1646  */
    { cs_force_eval_do_while( _INLINE_TLS_VOID); }
#line 2881 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 338 "./language-parser.y" /* yacc.c:1646  */
    { cs_end_do_while(&(yyvsp[-8]),&(yyvsp[-2]),&yychar _INLINE_TLS); }
#line 2887 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 344 "./language-parser.y" /* yacc.c:1646  */
    { for_pre_expr1(&(yyvsp[0]) _INLINE_TLS); }
#line 2893 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 345 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) pval_destructor(&(yyvsp[0]) _INLINE_TLS); for_pre_expr2(&(yyvsp[-3]) _INLINE_TLS); }
#line 2899 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 346 "./language-parser.y" /* yacc.c:1646  */
    { for_pre_expr3(&(yyvsp[-6]),&(yyvsp[0]) _INLINE_TLS); }
#line 2905 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 347 "./language-parser.y" /* yacc.c:1646  */
    { for_pre_statement(&(yyvsp[-9]),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 2911 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 348 "./language-parser.y" /* yacc.c:1646  */
    { for_post_statement(&(yyvsp[-12]),&(yyvsp[-7]),&(yyvsp[-4]),&(yyvsp[-1]),&yychar _INLINE_TLS); }
#line 2917 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 354 "./language-parser.y" /* yacc.c:1646  */
    { cs_elseif_start_evaluate( _INLINE_TLS_VOID); }
#line 2923 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 355 "./language-parser.y" /* yacc.c:1646  */
    { cs_elseif_end_evaluate( _INLINE_TLS_VOID); }
#line 2929 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 355 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_elseif (&(yyvsp[-2]) _INLINE_TLS); }
#line 2935 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 361 "./language-parser.y" /* yacc.c:1646  */
    { cs_elseif_start_evaluate( _INLINE_TLS_VOID); }
#line 2941 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 362 "./language-parser.y" /* yacc.c:1646  */
    { cs_elseif_end_evaluate( _INLINE_TLS_VOID); }
#line 2947 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 362 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_elseif (&(yyvsp[-2]) _INLINE_TLS); }
#line 2953 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 368 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_else( _INLINE_TLS_VOID); }
#line 2959 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 374 "./language-parser.y" /* yacc.c:1646  */
    { cs_start_else( _INLINE_TLS_VOID); }
#line 2965 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 380 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_pre(&(yyvsp[-1]) _INLINE_TLS); }
#line 2971 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 380 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_post( _INLINE_TLS_VOID); }
#line 2977 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 381 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_pre(NULL _INLINE_TLS); }
#line 2983 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 381 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_post( _INLINE_TLS_VOID); }
#line 2989 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 382 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_pre(&(yyvsp[-1]) _INLINE_TLS); }
#line 2995 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 382 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_post( _INLINE_TLS_VOID); }
#line 3001 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 383 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_pre(NULL _INLINE_TLS); }
#line 3007 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 383 "./language-parser.y" /* yacc.c:1646  */
    { cs_switch_case_post( _INLINE_TLS_VOID); }
#line 3013 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 388 "./language-parser.y" /* yacc.c:1646  */
    { GLOBAL(param_index)=0; }
#line 3019 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 394 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[0]), BYREF_NONE, NULL _INLINE_TLS); }
#line 3025 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 395 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[0]), BYREF_FORCE, NULL _INLINE_TLS); }
#line 3031 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 396 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[0]), BYREF_ALLOW, NULL _INLINE_TLS); }
#line 3037 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 397 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[-2]), BYREF_NONE, &(yyvsp[0]) _INLINE_TLS); }
#line 3043 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 398 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[0]), BYREF_NONE, NULL _INLINE_TLS); }
#line 3049 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 399 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[0]), BYREF_FORCE, NULL  _INLINE_TLS); }
#line 3055 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 400 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[0]), BYREF_ALLOW, NULL _INLINE_TLS); }
#line 3061 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 401 "./language-parser.y" /* yacc.c:1646  */
    { get_function_parameter(&(yyvsp[-2]), BYREF_NONE, &(yyvsp[0]) _INLINE_TLS); }
#line 3067 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 412 "./language-parser.y" /* yacc.c:1646  */
    { pass_parameter_by_value(&(yyvsp[0]) _INLINE_TLS); }
#line 3073 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 413 "./language-parser.y" /* yacc.c:1646  */
    { pass_parameter(&(yyvsp[0]),0 _INLINE_TLS); }
#line 3079 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 414 "./language-parser.y" /* yacc.c:1646  */
    { pass_parameter(&(yyvsp[0]),1 _INLINE_TLS); }
#line 3085 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 415 "./language-parser.y" /* yacc.c:1646  */
    { pass_parameter_by_value(&(yyvsp[0]) _INLINE_TLS); }
#line 3091 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 416 "./language-parser.y" /* yacc.c:1646  */
    { pass_parameter(&(yyvsp[0]),0 _INLINE_TLS); }
#line 3097 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 417 "./language-parser.y" /* yacc.c:1646  */
    { pass_parameter(&(yyvsp[0]),1 _INLINE_TLS); }
#line 3103 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 421 "./language-parser.y" /* yacc.c:1646  */
    { cs_global_variable(&(yyvsp[0]) _INLINE_TLS); }
#line 3109 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 422 "./language-parser.y" /* yacc.c:1646  */
    { cs_global_variable(&(yyvsp[0]) _INLINE_TLS); }
#line 3115 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 427 "./language-parser.y" /* yacc.c:1646  */
    { cs_static_variable(&(yyvsp[0]),NULL _INLINE_TLS); }
#line 3121 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 428 "./language-parser.y" /* yacc.c:1646  */
    { cs_static_variable(&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3127 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 429 "./language-parser.y" /* yacc.c:1646  */
    { cs_static_variable(&(yyvsp[0]),NULL _INLINE_TLS); }
#line 3133 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 430 "./language-parser.y" /* yacc.c:1646  */
    { cs_static_variable(&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3139 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 436 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3145 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 437 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3151 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 449 "./language-parser.y" /* yacc.c:1646  */
    { start_function_decleration(_INLINE_TLS_VOID); }
#line 3157 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 450 "./language-parser.y" /* yacc.c:1646  */
    { end_function_decleration(&(yyvsp[-7]),&(yyvsp[-6]) _INLINE_TLS); }
#line 3163 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 451 "./language-parser.y" /* yacc.c:1646  */
    { start_function_decleration(_INLINE_TLS_VOID); }
#line 3169 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 452 "./language-parser.y" /* yacc.c:1646  */
    { end_function_decleration(&(yyvsp[-8]),&(yyvsp[-7]) _INLINE_TLS); }
#line 3175 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 458 "./language-parser.y" /* yacc.c:1646  */
    { declare_class_variable(&(yyvsp[0]),NULL _INLINE_TLS); }
#line 3181 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 459 "./language-parser.y" /* yacc.c:1646  */
    { declare_class_variable(&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3187 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 460 "./language-parser.y" /* yacc.c:1646  */
    { declare_class_variable(&(yyvsp[0]),NULL _INLINE_TLS); }
#line 3193 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 461 "./language-parser.y" /* yacc.c:1646  */
    { declare_class_variable(&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3199 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 467 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { php3i_print_variable(&(yyvsp[0]) _INLINE_TLS); pval_destructor(&(yyvsp[0]) _INLINE_TLS); } }
#line 3205 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 468 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { php3i_print_variable(&(yyvsp[0]) _INLINE_TLS); pval_destructor(&(yyvsp[0]) _INLINE_TLS); } }
#line 3211 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 473 "./language-parser.y" /* yacc.c:1646  */
    { (yyval).value.lval=1;  (yyval).type=IS_LONG; }
#line 3217 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 474 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { (yyval) = (yyvsp[0]); pval_destructor(&(yyvsp[-2]) _INLINE_TLS); } }
#line 3223 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 475 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3229 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 480 "./language-parser.y" /* yacc.c:1646  */
    { assign_to_list(&(yyval), &(yyvsp[-3]), &(yyvsp[0]) _INLINE_TLS);}
#line 3235 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 481 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),NULL _INLINE_TLS); }
#line 3241 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 482 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),add_function _INLINE_TLS); }
#line 3247 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 483 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),sub_function _INLINE_TLS); }
#line 3253 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 484 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),mul_function _INLINE_TLS); }
#line 3259 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 485 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),div_function _INLINE_TLS); }
#line 3265 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 486 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),concat_function_with_free _INLINE_TLS); }
#line 3271 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 487 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),mod_function _INLINE_TLS); }
#line 3277 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 488 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),bitwise_and_function _INLINE_TLS); }
#line 3283 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 489 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),bitwise_or_function _INLINE_TLS); }
#line 3289 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 490 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),bitwise_xor_function _INLINE_TLS); }
#line 3295 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 491 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),shift_left_function _INLINE_TLS); }
#line 3301 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 492 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) assign_to_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),shift_right_function _INLINE_TLS); }
#line 3307 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 493 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) incdec_variable(&(yyval),&(yyvsp[-1]),increment_function,1 _INLINE_TLS); }
#line 3313 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 494 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) incdec_variable(&(yyval),&(yyvsp[0]),increment_function,0 _INLINE_TLS); }
#line 3319 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 495 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) incdec_variable(&(yyval),&(yyvsp[-1]),decrement_function,1 _INLINE_TLS); }
#line 3325 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 496 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) incdec_variable(&(yyval),&(yyvsp[0]),decrement_function,0 _INLINE_TLS); }
#line 3331 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 497 "./language-parser.y" /* yacc.c:1646  */
    { cs_pre_boolean_or(&(yyvsp[-1]) _INLINE_TLS); }
#line 3337 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 497 "./language-parser.y" /* yacc.c:1646  */
    { cs_post_boolean_or(&(yyval),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 3343 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 498 "./language-parser.y" /* yacc.c:1646  */
    { cs_pre_boolean_and(&(yyvsp[-1]) _INLINE_TLS); }
#line 3349 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 498 "./language-parser.y" /* yacc.c:1646  */
    { cs_post_boolean_and(&(yyval),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 3355 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 499 "./language-parser.y" /* yacc.c:1646  */
    { cs_pre_boolean_or(&(yyvsp[-1]) _INLINE_TLS); }
#line 3361 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 499 "./language-parser.y" /* yacc.c:1646  */
    { cs_post_boolean_or(&(yyval),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 3367 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 500 "./language-parser.y" /* yacc.c:1646  */
    { cs_pre_boolean_and(&(yyvsp[-1]) _INLINE_TLS); }
#line 3373 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 500 "./language-parser.y" /* yacc.c:1646  */
    { cs_post_boolean_and(&(yyval),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 3379 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 501 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { boolean_xor_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0])); } }
#line 3385 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 502 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) bitwise_or_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3391 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 503 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) bitwise_xor_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3397 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 504 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) bitwise_and_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3403 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 505 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) concat_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]),1 _INLINE_TLS); }
#line 3409 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 506 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3415 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 507 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) sub_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3421 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 508 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) mul_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3427 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 509 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) div_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3433 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 510 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) mod_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3439 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 511 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) shift_left_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3445 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 512 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) shift_right_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3451 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 513 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { pval tmp;  tmp.value.lval=0;  tmp.type=IS_LONG;  add_function(&(yyval),&tmp,&(yyvsp[0]) _INLINE_TLS); } }
#line 3457 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 514 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { pval tmp;  tmp.value.lval=0;  tmp.type=IS_LONG;  sub_function(&(yyval),&tmp,&(yyvsp[0]) _INLINE_TLS); } }
#line 3463 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 515 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) boolean_not_function(&(yyval),&(yyvsp[0])); }
#line 3469 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 516 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) bitwise_not_function(&(yyval),&(yyvsp[0]) _INLINE_TLS); }
#line 3475 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 517 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) is_equal_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3481 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 518 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) is_not_equal_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3487 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 519 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) is_smaller_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3493 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 520 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) is_smaller_or_equal_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3499 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 521 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) is_greater_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3505 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 522 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) is_greater_or_equal_function(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3511 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 523 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3517 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 524 "./language-parser.y" /* yacc.c:1646  */
    {  php3_error(E_PARSE,"'(' unmatched",GLOBAL(current_lineno)); if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); yyerrok; }
#line 3523 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 525 "./language-parser.y" /* yacc.c:1646  */
    { cs_questionmark_op_pre_expr1(&(yyvsp[-1]) _INLINE_TLS); }
#line 3529 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 526 "./language-parser.y" /* yacc.c:1646  */
    { cs_questionmark_op_pre_expr2(&(yyvsp[-4]) _INLINE_TLS); }
#line 3535 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 527 "./language-parser.y" /* yacc.c:1646  */
    { cs_questionmark_op_post_expr2(&(yyval),&(yyvsp[-6]),&(yyvsp[-3]),&(yyvsp[0]) _INLINE_TLS); }
#line 3541 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 528 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_pre_variable_passing(&(yyvsp[0]),NULL,1 _INLINE_TLS); }
#line 3547 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 529 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_post_variable_passing(&(yyvsp[-4]),&yychar _INLINE_TLS); }
#line 3553 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 530 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_end(&(yyval),&(yyvsp[-6]),&(yyvsp[-2]),&yychar,1 _INLINE_TLS);}
#line 3559 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 532 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_pre_variable_passing(&(yyvsp[0]),&(yyvsp[-2]),1 _INLINE_TLS); }
#line 3565 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 533 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_post_variable_passing(&(yyvsp[-4]),&yychar _INLINE_TLS); }
#line 3571 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 534 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_end(&(yyval),&(yyvsp[-6]),&(yyvsp[-2]),&yychar,1 _INLINE_TLS); }
#line 3577 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 535 "./language-parser.y" /* yacc.c:1646  */
    { assign_new_object(&(yyval),&(yyvsp[0]),1 _INLINE_TLS); }
#line 3583 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 536 "./language-parser.y" /* yacc.c:1646  */
    { assign_new_object(&(yyval),&(yyvsp[0]),0 _INLINE_TLS); }
#line 3589 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 537 "./language-parser.y" /* yacc.c:1646  */
    { if (!GLOBAL(shutdown_requested)) { pval object_pointer; object_pointer.value.varptr.pvalue = &(yyvsp[-1]); cs_functioncall_pre_variable_passing(&(yyvsp[-2]), &object_pointer, 1 _INLINE_TLS); } }
#line 3595 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 538 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_post_variable_passing(&(yyvsp[-5]), &yychar _INLINE_TLS); }
#line 3601 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 539 "./language-parser.y" /* yacc.c:1646  */
    { cs_functioncall_end(&(yyval), &(yyvsp[-7]), &(yyvsp[-2]), &yychar, 1 _INLINE_TLS);  (yyval) = (yyvsp[-6]); }
#line 3607 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 540 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { convert_to_long(&(yyvsp[0])); (yyval) = (yyvsp[0]); } }
#line 3613 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 541 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { convert_to_double(&(yyvsp[0])); (yyval) = (yyvsp[0]); } }
#line 3619 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 542 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { convert_to_string(&(yyvsp[0])); (yyval) = (yyvsp[0]); } }
#line 3625 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 543 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { convert_to_array(&(yyvsp[0])); (yyval) = (yyvsp[0]); } }
#line 3631 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 544 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { convert_to_object(&(yyvsp[0])); (yyval) = (yyvsp[0]); } }
#line 3637 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 545 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { php3_header(); GLOBAL(shutdown_requested)=ABNORMAL_SHUTDOWN; (yyval).type=IS_LONG; (yyval).value.lval=1; } }
#line 3643 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 546 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { php3_header(); GLOBAL(shutdown_requested)=ABNORMAL_SHUTDOWN; (yyval).type=IS_LONG; (yyval).value.lval=1; } }
#line 3649 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 547 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { if (php3_header()) { convert_to_string(&(yyvsp[-1]));  PUTS((yyvsp[-1]).value.str.val); convert_to_long(&(yyvsp[-1])); wanted_exit_status = (yyvsp[-1]).value.lval;  pval_destructor(&(yyvsp[-1]) _INLINE_TLS); } GLOBAL(shutdown_requested)=ABNORMAL_SHUTDOWN; (yyval).type=IS_LONG; (yyval).value.lval=1; } }
#line 3655 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 548 "./language-parser.y" /* yacc.c:1646  */
    { (yyvsp[0]).cs_data.error_reporting=GLOBAL(error_reporting); GLOBAL(error_reporting)=0; }
#line 3661 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 548 "./language-parser.y" /* yacc.c:1646  */
    { GLOBAL(error_reporting)=(yyvsp[-2]).cs_data.error_reporting; (yyval) = (yyvsp[0]); }
#line 3667 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 549 "./language-parser.y" /* yacc.c:1646  */
    { php3_error(E_ERROR,"@ operator may only be used on expressions"); }
#line 3673 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 550 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3679 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 551 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3685 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 552 "./language-parser.y" /* yacc.c:1646  */
    { cs_system(&(yyval),&(yyvsp[-1]) _INLINE_TLS); }
#line 3691 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 553 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { php3i_print_variable(&(yyvsp[0]) _INLINE_TLS);  pval_destructor(&(yyvsp[0]) _INLINE_TLS);  (yyval).value.lval=1; (yyval).type=IS_LONG; } }
#line 3697 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 557 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3703 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 558 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3709 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 559 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3715 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 560 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3721 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 561 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { (yyval) = (yyvsp[0]); COPY_STRING((yyval)); } }
#line 3727 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 562 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { (yyval) = (yyvsp[0]); COPY_STRING((yyval)); php3_error(E_NOTICE,"'%s' is not a valid constant - assumed to be \"%s\"",(yyvsp[0]).value.str.val,(yyvsp[0]).value.str.val); } }
#line 3733 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 563 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { (yyval) = (yyvsp[0]); } }
#line 3739 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 564 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { (yyval) = (yyvsp[0]); COPY_STRING((yyval)); } }
#line 3745 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 568 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3751 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 569 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3757 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 570 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { pval tmp;  tmp.value.lval=0;  tmp.type=IS_LONG;  sub_function(&(yyval),&tmp,&(yyvsp[0]) _INLINE_TLS); } }
#line 3763 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 574 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)){ (yyval) = (yyvsp[0]); }}
#line 3769 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 580 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) read_pointer_value(&(yyval),&(yyvsp[0]) _INLINE_TLS); }
#line 3775 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 581 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3781 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 586 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[0]); }
#line 3787 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 587 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3793 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 592 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) {(yyval) = (yyvsp[0]);COPY_STRING((yyval)); }}
#line 3799 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 593 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval) = (yyvsp[-1]); }
#line 3805 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 599 "./language-parser.y" /* yacc.c:1646  */
    { get_object_symtable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3811 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 600 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { if ((yyvsp[0]).value.varptr.pvalue && ((pval *)(yyvsp[0]).value.varptr.pvalue)->type == IS_OBJECT) { (yyval)=(yyvsp[0]); } else { (yyval).value.varptr.pvalue=NULL; } } }
#line 3817 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 601 "./language-parser.y" /* yacc.c:1646  */
    { get_object_symtable(&(yyval),NULL,&(yyvsp[0]) _INLINE_TLS); }
#line 3823 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 605 "./language-parser.y" /* yacc.c:1646  */
    {

	if (GLOBAL(Execute)) {
		(yyval)=(yyvsp[-2]);
		_php3_hash_next_index_insert((yyval).value.ht,&(yyvsp[0]),sizeof(pval),NULL);
	}
}
#line 3835 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 612 "./language-parser.y" /* yacc.c:1646  */
    {

	if (GLOBAL(Execute)) {
		(yyval).value.ht = (HashTable *) emalloc(sizeof(HashTable));
		_php3_hash_init((yyval).value.ht,0,NULL,NULL,0);
		(yyval).type = IS_ARRAY;
		_php3_hash_next_index_insert((yyval).value.ht,&(yyvsp[0]),sizeof(pval),NULL);
	}
}
#line 3849 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 621 "./language-parser.y" /* yacc.c:1646  */
    {
	if (GLOBAL(Execute)) {
		(yyval)=(yyvsp[-1]);
		(yyvsp[0]).value.varptr.pvalue = NULL; /* $2 is just used as temporary space */
		_php3_hash_next_index_insert((yyval).value.ht,&(yyvsp[0]),sizeof(pval),NULL);
	}
}
#line 3861 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 628 "./language-parser.y" /* yacc.c:1646  */
    {
	if (GLOBAL(Execute)) {
		pval tmp;

		(yyval).value.ht = (HashTable *) emalloc(sizeof(HashTable));
		_php3_hash_init((yyval).value.ht,0,NULL,NULL,0);
		(yyval).type = IS_ARRAY;
		tmp.value.varptr.pvalue = NULL;
		_php3_hash_next_index_insert((yyval).value.ht,&tmp,sizeof(pval),NULL);
	}
}
#line 3877 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 643 "./language-parser.y" /* yacc.c:1646  */
    { get_regular_variable_pointer(&(yyval),&(yyvsp[0]) _INLINE_TLS); }
#line 3883 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 644 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) (yyval)=(yyvsp[0]); }
#line 3889 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 645 "./language-parser.y" /* yacc.c:1646  */
    { get_class_variable_pointer(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 3895 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 646 "./language-parser.y" /* yacc.c:1646  */
    { start_array_parsing(&(yyvsp[-1]),&(yyvsp[-3]) _INLINE_TLS); }
#line 3901 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 646 "./language-parser.y" /* yacc.c:1646  */
    { end_array_parsing(&(yyval),&(yyvsp[0]) _INLINE_TLS); }
#line 3907 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 651 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) get_regular_variable_contents(&(yyval),&(yyvsp[0]),1 _INLINE_TLS); }
#line 3913 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 652 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) {(yyval) = (yyvsp[0]);COPY_STRING((yyval));} }
#line 3919 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 653 "./language-parser.y" /* yacc.c:1646  */
    {  if (GLOBAL(Execute)) read_pointer_value(&(yyval),&(yyvsp[0]) _INLINE_TLS); }
#line 3925 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 657 "./language-parser.y" /* yacc.c:1646  */
    { start_array_parsing(&(yyvsp[-1]),NULL _INLINE_TLS); }
#line 3931 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 657 "./language-parser.y" /* yacc.c:1646  */
    { end_array_parsing(&(yyval),&(yyvsp[0]) _INLINE_TLS); }
#line 3937 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 662 "./language-parser.y" /* yacc.c:1646  */
    { fetch_array_index(&(yyval),&(yyvsp[-1]),&(yyvsp[-3]) _INLINE_TLS); }
#line 3943 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 663 "./language-parser.y" /* yacc.c:1646  */
    { fetch_array_index(&(yyval),NULL,&(yyvsp[-2]) _INLINE_TLS); }
#line 3949 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 664 "./language-parser.y" /* yacc.c:1646  */
    { start_dimensions_parsing(&(yyval) _INLINE_TLS); }
#line 3955 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 664 "./language-parser.y" /* yacc.c:1646  */
    { fetch_array_index(&(yyval),NULL,&(yyval) _INLINE_TLS); }
#line 3961 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 665 "./language-parser.y" /* yacc.c:1646  */
    { start_dimensions_parsing(&(yyval) _INLINE_TLS); }
#line 3967 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 665 "./language-parser.y" /* yacc.c:1646  */
    { fetch_array_index(&(yyval),&(yyvsp[-1]),&(yyval) _INLINE_TLS); }
#line 3973 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 670 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { (yyval).value.ht = (HashTable *) emalloc(sizeof(HashTable));  _php3_hash_init((yyval).value.ht,0,NULL,PVAL_DESTRUCTOR,0); (yyval).type = IS_ARRAY; } }
#line 3979 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 671 "./language-parser.y" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3985 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 675 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) {(yyval)=(yyvsp[-4]); add_array_pair_list(&(yyval), &(yyvsp[-2]), &(yyvsp[0]), 0 _INLINE_TLS);} }
#line 3991 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 676 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) {(yyval)=(yyvsp[-2]); add_array_pair_list(&(yyval), NULL, &(yyvsp[0]), 0 _INLINE_TLS);} }
#line 3997 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 677 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_array_pair_list(&(yyval), &(yyvsp[-2]), &(yyvsp[0]), 1 _INLINE_TLS); }
#line 4003 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 678 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_array_pair_list(&(yyval), NULL, &(yyvsp[0]), 1 _INLINE_TLS); }
#line 4009 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 683 "./language-parser.y" /* yacc.c:1646  */
    { add_regular_encapsed_variable(&(yyval),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 4015 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 684 "./language-parser.y" /* yacc.c:1646  */
    { add_assoc_array_encapsed_variable(&(yyval),&(yyvsp[-5]),&(yyvsp[-3]),&(yyvsp[-1]) _INLINE_TLS); }
#line 4021 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 685 "./language-parser.y" /* yacc.c:1646  */
    { add_regular_array_encapsed_variable(&(yyval),&(yyvsp[-5]),&(yyvsp[-3]),&(yyvsp[-1]) _INLINE_TLS); }
#line 4027 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 686 "./language-parser.y" /* yacc.c:1646  */
    { add_variable_array_encapsed_variable(&(yyval),&(yyvsp[-6]),&(yyvsp[-4]),&(yyvsp[-1]) _INLINE_TLS); }
#line 4033 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 687 "./language-parser.y" /* yacc.c:1646  */
    { add_encapsed_object_property(&(yyval),&(yyvsp[-4]),&(yyvsp[-2]),&(yyvsp[0]) _INLINE_TLS); }
#line 4039 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 688 "./language-parser.y" /* yacc.c:1646  */
    { add_regular_encapsed_variable(&(yyval),&(yyvsp[-4]),&(yyvsp[-1]) _INLINE_TLS); }
#line 4045 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 689 "./language-parser.y" /* yacc.c:1646  */
    { add_assoc_array_encapsed_variable(&(yyval),&(yyvsp[-7]),&(yyvsp[-4]),&(yyvsp[-2]) _INLINE_TLS); }
#line 4051 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 690 "./language-parser.y" /* yacc.c:1646  */
    { add_regular_array_encapsed_variable(&(yyval),&(yyvsp[-7]),&(yyvsp[-4]),&(yyvsp[-2]) _INLINE_TLS); }
#line 4057 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 691 "./language-parser.y" /* yacc.c:1646  */
    { add_variable_array_encapsed_variable(&(yyval),&(yyvsp[-8]),&(yyvsp[-5]),&(yyvsp[-2]) _INLINE_TLS); }
#line 4063 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 692 "./language-parser.y" /* yacc.c:1646  */
    { add_encapsed_object_property(&(yyval),&(yyvsp[-6]),&(yyvsp[-3]),&(yyvsp[-1]) _INLINE_TLS); }
#line 4069 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 693 "./language-parser.y" /* yacc.c:1646  */
    { add_indirect_encapsed_variable(&(yyval),&(yyvsp[-5]),&(yyvsp[-1]) _INLINE_TLS); }
#line 4075 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 694 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { concat_function(&(yyval),&(yyvsp[-1]),&(yyvsp[0]),0 _INLINE_TLS); } }
#line 4081 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 695 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { concat_function(&(yyval),&(yyvsp[-1]),&(yyvsp[0]),0 _INLINE_TLS); } }
#line 4087 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 696 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { concat_function(&(yyval),&(yyvsp[-1]),&(yyvsp[0]),0 _INLINE_TLS); } }
#line 4093 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 697 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_char_to_string(&(yyval),&(yyvsp[-1]),&(yyvsp[0]) _INLINE_TLS); }
#line 4099 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 698 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) { php3_error(E_NOTICE,"Bad escape sequence:  %s",(yyvsp[0]).value.str.val); concat_function(&(yyval),&(yyvsp[-1]),&(yyvsp[0]),0 _INLINE_TLS); } }
#line 4105 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 699 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_char_to_string(&(yyval),&(yyvsp[-1]),&(yyvsp[0]) _INLINE_TLS); }
#line 4111 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 700 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_char_to_string(&(yyval),&(yyvsp[-1]),&(yyvsp[0]) _INLINE_TLS); }
#line 4117 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 701 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_char_to_string(&(yyval),&(yyvsp[-1]),&(yyvsp[0]) _INLINE_TLS); }
#line 4123 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 702 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) add_char_to_string(&(yyval),&(yyvsp[-1]),&(yyvsp[0]) _INLINE_TLS); }
#line 4129 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 703 "./language-parser.y" /* yacc.c:1646  */
    {  if (GLOBAL(Execute)) { pval tmp;  tmp.value.str.val="->"; tmp.value.str.len=2; tmp.type=IS_STRING; concat_function(&(yyval),&(yyvsp[-1]),&tmp,0 _INLINE_TLS); } }
#line 4135 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 704 "./language-parser.y" /* yacc.c:1646  */
    { if (GLOBAL(Execute)) var_reset(&(yyval)); }
#line 4141 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 709 "./language-parser.y" /* yacc.c:1646  */
    { php3_unset(&(yyval), &(yyvsp[-1])); }
#line 4147 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 710 "./language-parser.y" /* yacc.c:1646  */
    { php3_isset(&(yyval), &(yyvsp[-1])); }
#line 4153 "./language-parser.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 711 "./language-parser.y" /* yacc.c:1646  */
    { php3_empty(&(yyval), &(yyvsp[-1])); }
#line 4159 "./language-parser.tab.c" /* yacc.c:1646  */
    break;


#line 4163 "./language-parser.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 721 "./language-parser.y" /* yacc.c:1906  */


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
