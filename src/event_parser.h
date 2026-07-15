/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_ZZ_SRC_EVENT_PARSER_H_INCLUDED
# define YY_ZZ_SRC_EVENT_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef ZZDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ZZDEBUG 1
#  else
#   define ZZDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ZZDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ZZDEBUG */
#if ZZDEBUG
extern int zzdebug;
#endif

/* Token kinds.  */
#ifndef ZZTOKENTYPE
# define ZZTOKENTYPE
  enum zztokentype
  {
    ZZEMPTY = -2,
    ZZEOF = 0,                     /* "end of file"  */
    ZZerror = 256,                 /* error  */
    ZZUNDEF = 257,                 /* "invalid token"  */
    EVENT_LCURLY = 258,            /* EVENT_LCURLY  */
    EVENT_RCURLY = 259,            /* EVENT_RCURLY  */
    EVENT_LSQUARE = 260,           /* EVENT_LSQUARE  */
    EVENT_RSQUARE = 261,           /* EVENT_RSQUARE  */
    EVENT_COMMA = 262,             /* EVENT_COMMA  */
    EVENT_COLON = 263,             /* EVENT_COLON  */
    EVENT_MINUS = 264,             /* EVENT_MINUS  */
    EVENT_NULL = 265,              /* EVENT_NULL  */
    EVENT_TRUE = 266,              /* EVENT_TRUE  */
    EVENT_FALSE = 267,             /* EVENT_FALSE  */
    EVENT_INTEGER = 268,           /* EVENT_INTEGER  */
    EVENT_FLOAT = 269,             /* EVENT_FLOAT  */
    EVENT_STRING = 270             /* EVENT_STRING  */
  };
  typedef enum zztokentype zztoken_kind_t;
#endif

/* Value type.  */
#if ! defined ZZSTYPE && ! defined ZZSTYPE_IS_DECLARED
union ZZSTYPE
{
#line 32 "src/event_parser.y"

    int token;
    char *string;

    bool boolean_value;
    int64_t integer_value;
    double float_value;
    struct string_value string_value;
    struct betree_integer_list* integer_list_value;
    struct betree_string_list* string_list_value;
    struct betree_segments* segments_list_value;
    struct betree_segment* segment_value;
    struct betree_frequency_caps* frequencies_value;
    struct betree_frequency_cap* frequency_value;

    struct value value;
    struct betree_variable* variable;

    struct betree_event* event;

#line 108 "src/event_parser.h"

};
typedef union ZZSTYPE ZZSTYPE;
# define ZZSTYPE_IS_TRIVIAL 1
# define ZZSTYPE_IS_DECLARED 1
#endif




int zzparse (void *scanner);


#endif /* !YY_ZZ_SRC_EVENT_PARSER_H_INCLUDED  */
