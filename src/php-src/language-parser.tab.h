/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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

#ifndef YY_PHP_LANGUAGE_PARSER_TAB_H_INCLUDED
# define YY_PHP_LANGUAGE_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int phpdebug;
#endif

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
