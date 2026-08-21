/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         XXSTYPE
/* Substitute the variable and function names.  */
#define yyparse         xxparse
#define yylex           xxlex
#define yyerror         xxerror
#define yydebug         xxdebug
#define yynerrs         xxnerrs

/* First part of user prologue.  */
#line 1 "src/parser.y"

    #include <stdint.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include "alloc.h"
    #include "ast.h"
    #include "betree.h"
    #include "parser.h"
    #include "value.h"
    void xxerror(void *scanner, struct ast_node** node, const char *s) { (void)node; (void)scanner; printf("ERROR: %s\n", s); }
#ifdef NIF
    #include "erl_nif.h"
    #define YYMALLOC enif_alloc
    #define YYFREE enif_free
#endif
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-default"
    #pragma GCC diagnostic ignored "-Wshadow"
#endif
#line 30 "src/parser.y"

    int parse(const char *text, struct ast_node **node);

#line 103 "src/parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TMINUS = 3,                     /* TMINUS  */
  YYSYMBOL_TCEQ = 4,                       /* TCEQ  */
  YYSYMBOL_TCNE = 5,                       /* TCNE  */
  YYSYMBOL_TCGT = 6,                       /* TCGT  */
  YYSYMBOL_TCGE = 7,                       /* TCGE  */
  YYSYMBOL_TCLT = 8,                       /* TCLT  */
  YYSYMBOL_TCLE = 9,                       /* TCLE  */
  YYSYMBOL_TLPAREN = 10,                   /* TLPAREN  */
  YYSYMBOL_TRPAREN = 11,                   /* TRPAREN  */
  YYSYMBOL_TCOMMA = 12,                    /* TCOMMA  */
  YYSYMBOL_TNOTIN = 13,                    /* TNOTIN  */
  YYSYMBOL_TIN = 14,                       /* TIN  */
  YYSYMBOL_TONEOF = 15,                    /* TONEOF  */
  YYSYMBOL_TNONEOF = 16,                   /* TNONEOF  */
  YYSYMBOL_TALLOF = 17,                    /* TALLOF  */
  YYSYMBOL_TAND = 18,                      /* TAND  */
  YYSYMBOL_TOR = 19,                       /* TOR  */
  YYSYMBOL_TNOT = 20,                      /* TNOT  */
  YYSYMBOL_TWITHINFREQUENCYCAP = 21,       /* TWITHINFREQUENCYCAP  */
  YYSYMBOL_TSEGMENTWITHIN = 22,            /* TSEGMENTWITHIN  */
  YYSYMBOL_TSEGMENTBEFORE = 23,            /* TSEGMENTBEFORE  */
  YYSYMBOL_TGEOWITHINRADIUS = 24,          /* TGEOWITHINRADIUS  */
  YYSYMBOL_TCONTAINS = 25,                 /* TCONTAINS  */
  YYSYMBOL_TSTARTSWITH = 26,               /* TSTARTSWITH  */
  YYSYMBOL_TENDSWITH = 27,                 /* TENDSWITH  */
  YYSYMBOL_TISNOTNULL = 28,                /* TISNOTNULL  */
  YYSYMBOL_TISNULL = 29,                   /* TISNULL  */
  YYSYMBOL_TISEMPTY = 30,                  /* TISEMPTY  */
  YYSYMBOL_TTRUE = 31,                     /* TTRUE  */
  YYSYMBOL_TFALSE = 32,                    /* TFALSE  */
  YYSYMBOL_TINVALID = 33,                  /* TINVALID  */
  YYSYMBOL_TSTRING = 34,                   /* TSTRING  */
  YYSYMBOL_TIDENTIFIER = 35,               /* TIDENTIFIER  */
  YYSYMBOL_TANNOTATION = 36,               /* TANNOTATION  */
  YYSYMBOL_TINTEGER = 37,                  /* TINTEGER  */
  YYSYMBOL_TFLOAT = 38,                    /* TFLOAT  */
  YYSYMBOL_39_ = 39,                       /* '{'  */
  YYSYMBOL_40_ = 40,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 41,                  /* $accept  */
  YYSYMBOL_program = 42,                   /* program  */
  YYSYMBOL_ident = 43,                     /* ident  */
  YYSYMBOL_integer = 44,                   /* integer  */
  YYSYMBOL_float = 45,                     /* float  */
  YYSYMBOL_string = 46,                    /* string  */
  YYSYMBOL_integer_list_value = 47,        /* integer_list_value  */
  YYSYMBOL_integer_list_loop = 48,         /* integer_list_loop  */
  YYSYMBOL_string_list_value = 49,         /* string_list_value  */
  YYSYMBOL_string_list_loop = 50,          /* string_list_loop  */
  YYSYMBOL_expr = 51,                      /* expr  */
  YYSYMBOL_is_null_expr = 52,              /* is_null_expr  */
  YYSYMBOL_num_comp_value = 53,            /* num_comp_value  */
  YYSYMBOL_num_comp_expr = 54,             /* num_comp_expr  */
  YYSYMBOL_eq_value = 55,                  /* eq_value  */
  YYSYMBOL_eq_expr = 56,                   /* eq_expr  */
  YYSYMBOL_variable_value = 57,            /* variable_value  */
  YYSYMBOL_set_left_value = 58,            /* set_left_value  */
  YYSYMBOL_set_right_value = 59,           /* set_right_value  */
  YYSYMBOL_set_expr = 60,                  /* set_expr  */
  YYSYMBOL_list_value = 61,                /* list_value  */
  YYSYMBOL_list_expr = 62,                 /* list_expr  */
  YYSYMBOL_bool_expr = 63,                 /* bool_expr  */
  YYSYMBOL_special_expr = 64,              /* special_expr  */
  YYSYMBOL_s_frequency_expr = 65,          /* s_frequency_expr  */
  YYSYMBOL_s_segment_expr = 66,            /* s_segment_expr  */
  YYSYMBOL_s_geo_expr = 67,                /* s_geo_expr  */
  YYSYMBOL_s_string_expr = 68              /* s_string_expr  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined XXSTYPE_IS_TRIVIAL && XXSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  51
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   193

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  28
/* YYNRULES -- Number of rules.  */
#define YYNRULES  77
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  172

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   293


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,    39,     2,    40,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38
};

#if XXDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   116,   116,   118,   120,   121,   124,   125,   128,   130,
     132,   133,   136,   138,   139,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   153,   154,   155,   158,   159,   162,
     163,   164,   165,   166,   167,   168,   169,   172,   173,   174,
     177,   178,   179,   180,   183,   185,   186,   187,   190,   191,
     192,   195,   196,   199,   200,   203,   204,   205,   208,   209,
     210,   211,   212,   213,   216,   217,   218,   219,   222,   226,
     228,   230,   232,   236,   238,   242,   244,   246
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if XXDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TMINUS", "TCEQ",
  "TCNE", "TCGT", "TCGE", "TCLT", "TCLE", "TLPAREN", "TRPAREN", "TCOMMA",
  "TNOTIN", "TIN", "TONEOF", "TNONEOF", "TALLOF", "TAND", "TOR", "TNOT",
  "TWITHINFREQUENCYCAP", "TSEGMENTWITHIN", "TSEGMENTBEFORE",
  "TGEOWITHINRADIUS", "TCONTAINS", "TSTARTSWITH", "TENDSWITH",
  "TISNOTNULL", "TISNULL", "TISEMPTY", "TTRUE", "TFALSE", "TINVALID",
  "TSTRING", "TIDENTIFIER", "TANNOTATION", "TINTEGER", "TFLOAT", "'{'",
  "'}'", "$accept", "program", "ident", "integer", "float", "string",
  "integer_list_value", "integer_list_loop", "string_list_value",
  "string_list_loop", "expr", "is_null_expr", "num_comp_value",
  "num_comp_expr", "eq_value", "eq_expr", "variable_value",
  "set_left_value", "set_right_value", "set_expr", "list_value",
  "list_expr", "bool_expr", "special_expr", "s_frequency_expr",
  "s_segment_expr", "s_geo_expr", "s_string_expr", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-45)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-47)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      92,    -8,    92,    92,     8,    22,    35,    49,    70,    81,
      90,   -45,   -45,   -45,   -45,   -12,   -45,   -45,    48,   133,
      33,    39,    50,    64,   -45,    61,   -45,    80,   -45,   -45,
      73,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,
     -45,    38,   -45,    69,    23,    23,     4,     0,     0,     0,
      92,   -45,     2,     2,     4,     4,     4,     4,    96,    96,
      96,   -45,   -45,   -45,    92,    92,     0,     0,     0,     0,
       0,     0,    -4,    -4,   -45,   113,    74,   121,   152,   157,
     158,   159,   160,   161,   162,   163,    15,   -45,   -45,   -45,
     -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,    16,   -45,
     -45,   -45,   -45,   -45,   -45,   102,   -45,   -45,   -45,   -45,
     -45,   -45,   -45,   -45,   -45,   -45,   -45,   -45,   111,    25,
      25,    25,    25,    25,    14,   111,   111,   111,   -45,   -45,
     -45,    82,    85,   164,   165,    97,   166,   168,   169,   142,
     170,   172,   173,   174,   -45,    25,   -45,   111,    25,    25,
     -45,    25,   -45,    25,    14,   -45,   -45,   -45,   -45,   -45,
     175,   177,   178,   179,   180,    25,   -45,   -45,   -45,   -45,
     181,   -45
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    63,     8,     3,     0,     4,     6,     0,    61,
      27,    28,    39,     2,    23,     0,    17,     0,    18,    47,
       0,    19,    20,    21,    22,    64,    65,    66,    67,     5,
       7,     0,    60,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    25,    24,    26,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    37,    38,    39,
      40,    41,    27,    28,    29,    30,    31,    32,     0,    53,
      54,    55,    56,    57,    58,    59,    35,    36,    33,    34,
      42,    43,    44,    48,    49,    50,    51,    52,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    16,    10,
      13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     9,     0,    12,     0,     0,     0,
      69,     0,    71,     0,     0,    75,    76,    77,    11,    14,
       0,     0,     0,     0,     0,     0,    70,    72,    73,    74,
       0,    68
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -45,   -45,    87,   -44,   -32,   -37,    26,   -45,    71,   -45,
       1,   -45,    17,   -45,    99,   -45,    93,   -45,   120,   -45,
     108,   -45,   -45,   -45,   -45,   -45,   -45,   -45
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    18,    19,    20,    21,    22,    99,   131,   100,   132,
      23,    24,    25,    26,    27,    28,    29,    30,   116,    31,
     101,    32,    33,    34,    35,    36,    37,    38
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      78,    80,    81,    41,    42,     1,    98,     1,    87,    87,
      92,    92,    92,    92,    82,    89,    89,   139,    43,    76,
      88,    88,    93,    93,    93,    93,    76,    50,    76,    39,
      40,    14,    44,    64,    65,    14,    13,   -37,   -37,    16,
      17,    16,    17,   -38,   -38,    45,   -45,   -45,    51,    74,
      13,    86,    17,    16,   129,   128,    64,    65,    14,    46,
      16,   130,    16,   -46,   -46,   104,   105,    66,    67,    68,
      69,    94,    95,    96,    97,   134,   135,   136,   137,   138,
      47,   133,    64,    65,    70,    71,    72,    73,   141,   142,
     143,    48,   140,   144,   145,     1,   146,   147,   113,   113,
      49,   158,     2,    75,   160,   161,    98,   162,   150,   163,
     159,    39,     3,     4,     5,     6,     7,     8,     9,    10,
      64,   170,   164,    11,    12,   118,    13,    14,    15,    16,
      17,    77,    79,   119,    83,    84,    85,    52,    53,    54,
      55,    56,    57,   114,   114,    13,   -44,   -44,    58,    59,
      60,    90,    91,   106,   107,   108,   109,   110,   111,   112,
     112,    61,    62,    63,   120,   115,   115,   102,   103,   121,
     122,   123,   124,   125,   126,   127,   148,   149,   151,   152,
      40,   153,   154,   155,   156,   157,     0,   165,   166,   167,
     168,   169,   171,   117
};

static const yytype_int16 yycheck[] =
{
      44,    45,    46,     2,     3,     3,    10,     3,    52,    53,
      54,    55,    56,    57,    46,    52,    53,     3,    10,     3,
      52,    53,    54,    55,    56,    57,     3,    39,     3,    37,
      38,    35,    10,    18,    19,    35,    34,     4,     5,    37,
      38,    37,    38,     4,     5,    10,    13,    14,     0,    11,
      34,    50,    38,    37,    98,    40,    18,    19,    35,    10,
      37,    98,    37,    13,    14,    64,    65,     6,     7,     8,
       9,    54,    55,    56,    57,   119,   120,   121,   122,   123,
      10,   118,    18,    19,     4,     5,    13,    14,   125,   126,
     127,    10,   124,    11,    12,     3,    11,    12,    72,    73,
      10,   145,    10,    34,   148,   149,    10,   151,    11,   153,
     147,    37,    20,    21,    22,    23,    24,    25,    26,    27,
      18,   165,   154,    31,    32,    12,    34,    35,    36,    37,
      38,    44,    45,    12,    47,    48,    49,     4,     5,     6,
       7,     8,     9,    72,    73,    34,    13,    14,    15,    16,
      17,    52,    53,    66,    67,    68,    69,    70,    71,    72,
      73,    28,    29,    30,    12,    72,    73,    59,    60,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    12,    11,
      38,    12,    12,    11,    11,    11,    -1,    12,    11,    11,
      11,    11,    11,    73
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,    10,    20,    21,    22,    23,    24,    25,    26,
      27,    31,    32,    34,    35,    36,    37,    38,    42,    43,
      44,    45,    46,    51,    52,    53,    54,    55,    56,    57,
      58,    60,    62,    63,    64,    65,    66,    67,    68,    37,
      38,    51,    51,    10,    10,    10,    10,    10,    10,    10,
      39,     0,     4,     5,     6,     7,     8,     9,    15,    16,
      17,    28,    29,    30,    18,    19,     6,     7,     8,     9,
       4,     5,    13,    14,    11,    34,     3,    43,    44,    43,
      44,    44,    45,    43,    43,    43,    51,    44,    45,    46,
      55,    55,    44,    45,    53,    53,    53,    53,    10,    47,
      49,    61,    61,    61,    51,    51,    43,    43,    43,    43,
      43,    43,    43,    47,    49,    57,    59,    59,    12,    12,
      12,    12,    12,    12,    12,    12,    12,    12,    40,    44,
      46,    48,    50,    46,    44,    44,    44,    44,    44,     3,
      45,    46,    46,    46,    11,    12,    11,    12,    12,    12,
      11,    12,    11,    12,    12,    11,    11,    11,    44,    46,
      44,    44,    44,    44,    45,    12,    11,    11,    11,    11,
      44,    11
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    41,    42,    43,    44,    44,    45,    45,    46,    47,
      48,    48,    49,    50,    50,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    52,    52,    52,    53,    53,    54,
      54,    54,    54,    54,    54,    54,    54,    55,    55,    55,
      56,    56,    56,    56,    57,    58,    58,    58,    59,    59,
      59,    60,    60,    61,    61,    62,    62,    62,    63,    63,
      63,    63,    63,    63,    64,    64,    64,    64,    65,    66,
      66,    66,    66,    67,    67,    68,    68,    68
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     2,     1,     3,
       1,     3,     3,     1,     3,     3,     4,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       3,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       2,     1,     1,     1,     1,     1,     1,     1,    10,     6,
       8,     6,     8,     8,     8,     6,     6,     6
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = XXEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == XXEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, root, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use XXerror or XXUNDEF. */
#define YYERRCODE XXUNDEF


/* Enable debugging if requested.  */
#if XXDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scanner, root); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner, struct ast_node** root)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  YY_USE (root);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner, struct ast_node** root)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner, root);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scanner, struct ast_node** root)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner, root);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner, root); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !XXDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !XXDEBUG */


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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner, struct ast_node** root)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  YY_USE (root);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_TSTRING: /* TSTRING  */
#line 91 "src/parser.y"
            { bfree(((*yyvaluep).string)); }
#line 1018 "src/parser.c"
        break;

    case YYSYMBOL_TIDENTIFIER: /* TIDENTIFIER  */
#line 91 "src/parser.y"
            { bfree(((*yyvaluep).string)); }
#line 1024 "src/parser.c"
        break;

    case YYSYMBOL_TANNOTATION: /* TANNOTATION  */
#line 91 "src/parser.y"
            { bfree(((*yyvaluep).string)); }
#line 1030 "src/parser.c"
        break;

    case YYSYMBOL_ident: /* ident  */
#line 91 "src/parser.y"
            { bfree(((*yyvaluep).string)); }
#line 1036 "src/parser.c"
        break;

    case YYSYMBOL_integer_list_value: /* integer_list_value  */
#line 93 "src/parser.y"
            { if(((*yyvaluep).integer_list_value) != NULL) free_integer_list(((*yyvaluep).integer_list_value)); }
#line 1042 "src/parser.c"
        break;

    case YYSYMBOL_integer_list_loop: /* integer_list_loop  */
#line 93 "src/parser.y"
            { if(((*yyvaluep).integer_list_value) != NULL) free_integer_list(((*yyvaluep).integer_list_value)); }
#line 1048 "src/parser.c"
        break;

    case YYSYMBOL_string_list_value: /* string_list_value  */
#line 94 "src/parser.y"
            { if(((*yyvaluep).string_list_value) != NULL) free_string_list(((*yyvaluep).string_list_value)); }
#line 1054 "src/parser.c"
        break;

    case YYSYMBOL_string_list_loop: /* string_list_loop  */
#line 94 "src/parser.y"
            { if(((*yyvaluep).string_list_value) != NULL) free_string_list(((*yyvaluep).string_list_value)); }
#line 1060 "src/parser.c"
        break;

    case YYSYMBOL_expr: /* expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1066 "src/parser.c"
        break;

    case YYSYMBOL_is_null_expr: /* is_null_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1072 "src/parser.c"
        break;

    case YYSYMBOL_num_comp_expr: /* num_comp_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1078 "src/parser.c"
        break;

    case YYSYMBOL_eq_expr: /* eq_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1084 "src/parser.c"
        break;

    case YYSYMBOL_set_expr: /* set_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1090 "src/parser.c"
        break;

    case YYSYMBOL_list_expr: /* list_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1096 "src/parser.c"
        break;

    case YYSYMBOL_bool_expr: /* bool_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1102 "src/parser.c"
        break;

    case YYSYMBOL_special_expr: /* special_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1108 "src/parser.c"
        break;

    case YYSYMBOL_s_frequency_expr: /* s_frequency_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1114 "src/parser.c"
        break;

    case YYSYMBOL_s_segment_expr: /* s_segment_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1120 "src/parser.c"
        break;

    case YYSYMBOL_s_geo_expr: /* s_geo_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1126 "src/parser.c"
        break;

    case YYSYMBOL_s_string_expr: /* s_string_expr  */
#line 92 "src/parser.y"
            { if(((*yyvaluep).node) != NULL) free_ast_node(((*yyvaluep).node)); }
#line 1132 "src/parser.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, struct ast_node** root)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = XXEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == XXEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= XXEOF)
    {
      yychar = XXEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == XXerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = XXUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = XXEMPTY;
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
| yyreduce -- do a reduction.  |
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
  case 2: /* program: expr  */
#line 116 "src/parser.y"
                                                            { *root = (yyvsp[0].node); }
#line 1408 "src/parser.c"
    break;

  case 3: /* ident: TIDENTIFIER  */
#line 118 "src/parser.y"
                                                            { (yyval.string) = (yyvsp[0].string); }
#line 1414 "src/parser.c"
    break;

  case 4: /* integer: TINTEGER  */
#line 120 "src/parser.y"
                                                            { (yyval.integer_value) = (yyvsp[0].integer_value); }
#line 1420 "src/parser.c"
    break;

  case 5: /* integer: TMINUS TINTEGER  */
#line 121 "src/parser.y"
                                                            { if ((yyvsp[0].integer_value) < 0) { YYERROR; } (yyval.integer_value) = - (yyvsp[0].integer_value); }
#line 1426 "src/parser.c"
    break;

  case 6: /* float: TFLOAT  */
#line 124 "src/parser.y"
                                                            { (yyval.float_value) = (yyvsp[0].float_value); }
#line 1432 "src/parser.c"
    break;

  case 7: /* float: TMINUS TFLOAT  */
#line 125 "src/parser.y"
                                                            { if ((yyvsp[0].float_value) < 0) { YYERROR; } (yyval.float_value) = - (yyvsp[0].float_value); }
#line 1438 "src/parser.c"
    break;

  case 8: /* string: TSTRING  */
#line 128 "src/parser.y"
                                                            { (yyval.string_value).string = bstrdup((yyvsp[0].string)); (yyval.string_value).str = INVALID_STR; bfree((yyvsp[0].string)); }
#line 1444 "src/parser.c"
    break;

  case 9: /* integer_list_value: TLPAREN integer_list_loop TRPAREN  */
#line 130 "src/parser.y"
                                                            { (yyval.integer_list_value) = (yyvsp[-1].integer_list_value); }
#line 1450 "src/parser.c"
    break;

  case 10: /* integer_list_loop: integer  */
#line 132 "src/parser.y"
                                                            { (yyval.integer_list_value) = make_integer_list(); add_integer_list_value((yyvsp[0].integer_value), (yyval.integer_list_value)); }
#line 1456 "src/parser.c"
    break;

  case 11: /* integer_list_loop: integer_list_loop TCOMMA integer  */
#line 133 "src/parser.y"
                                                            { add_integer_list_value((yyvsp[0].integer_value), (yyvsp[-2].integer_list_value)); (yyval.integer_list_value) = (yyvsp[-2].integer_list_value); }
#line 1462 "src/parser.c"
    break;

  case 12: /* string_list_value: TLPAREN string_list_loop TRPAREN  */
#line 136 "src/parser.y"
                                                            { (yyval.string_list_value) = (yyvsp[-1].string_list_value); }
#line 1468 "src/parser.c"
    break;

  case 13: /* string_list_loop: string  */
#line 138 "src/parser.y"
                                                            { (yyval.string_list_value) = make_string_list(); add_string_list_value((yyvsp[0].string_value), (yyval.string_list_value)); }
#line 1474 "src/parser.c"
    break;

  case 14: /* string_list_loop: string_list_loop TCOMMA string  */
#line 139 "src/parser.y"
                                                            { add_string_list_value((yyvsp[0].string_value), (yyvsp[-2].string_list_value)); (yyval.string_list_value) = (yyvsp[-2].string_list_value); }
#line 1480 "src/parser.c"
    break;

  case 15: /* expr: TLPAREN expr TRPAREN  */
#line 142 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[-1].node); }
#line 1486 "src/parser.c"
    break;

  case 16: /* expr: TANNOTATION '{' expr '}'  */
#line 143 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[-1].node); bfree((yyvsp[-3].string)); }
#line 1492 "src/parser.c"
    break;

  case 17: /* expr: num_comp_expr  */
#line 144 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1498 "src/parser.c"
    break;

  case 18: /* expr: eq_expr  */
#line 145 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1504 "src/parser.c"
    break;

  case 19: /* expr: set_expr  */
#line 146 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1510 "src/parser.c"
    break;

  case 20: /* expr: list_expr  */
#line 147 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1516 "src/parser.c"
    break;

  case 21: /* expr: bool_expr  */
#line 148 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1522 "src/parser.c"
    break;

  case 22: /* expr: special_expr  */
#line 149 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1528 "src/parser.c"
    break;

  case 23: /* expr: is_null_expr  */
#line 150 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1534 "src/parser.c"
    break;

  case 24: /* is_null_expr: ident TISNULL  */
#line 153 "src/parser.y"
                                                            { (yyval.node) = ast_is_null_expr_create(AST_IS_NULL, (yyvsp[-1].string)); bfree((yyvsp[-1].string)); }
#line 1540 "src/parser.c"
    break;

  case 25: /* is_null_expr: ident TISNOTNULL  */
#line 154 "src/parser.y"
                                                            { (yyval.node) = ast_is_null_expr_create(AST_IS_NOT_NULL, (yyvsp[-1].string)); bfree((yyvsp[-1].string)); }
#line 1546 "src/parser.c"
    break;

  case 26: /* is_null_expr: ident TISEMPTY  */
#line 155 "src/parser.y"
                                                            { (yyval.node) = ast_is_null_expr_create(AST_IS_EMPTY, (yyvsp[-1].string)); bfree((yyvsp[-1].string)); }
#line 1552 "src/parser.c"
    break;

  case 27: /* num_comp_value: integer  */
#line 158 "src/parser.y"
                                                            { (yyval.compare_value).value_type = AST_COMPARE_VALUE_INTEGER; (yyval.compare_value).integer_value = (yyvsp[0].integer_value); }
#line 1558 "src/parser.c"
    break;

  case 28: /* num_comp_value: float  */
#line 159 "src/parser.y"
                                                            { (yyval.compare_value).value_type = AST_COMPARE_VALUE_FLOAT; (yyval.compare_value).float_value = (yyvsp[0].float_value); }
#line 1564 "src/parser.c"
    break;

  case 29: /* num_comp_expr: ident TCGT num_comp_value  */
#line 162 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_GT, (yyvsp[-2].string), (yyvsp[0].compare_value)); bfree((yyvsp[-2].string)); }
#line 1570 "src/parser.c"
    break;

  case 30: /* num_comp_expr: ident TCGE num_comp_value  */
#line 163 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_GE, (yyvsp[-2].string), (yyvsp[0].compare_value)); bfree((yyvsp[-2].string)); }
#line 1576 "src/parser.c"
    break;

  case 31: /* num_comp_expr: ident TCLT num_comp_value  */
#line 164 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_LT, (yyvsp[-2].string), (yyvsp[0].compare_value)); bfree((yyvsp[-2].string)); }
#line 1582 "src/parser.c"
    break;

  case 32: /* num_comp_expr: ident TCLE num_comp_value  */
#line 165 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_LE, (yyvsp[-2].string), (yyvsp[0].compare_value)); bfree((yyvsp[-2].string)); }
#line 1588 "src/parser.c"
    break;

  case 33: /* num_comp_expr: num_comp_value TCLT ident  */
#line 166 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_GT, (yyvsp[0].string), (yyvsp[-2].compare_value)); bfree((yyvsp[0].string)); }
#line 1594 "src/parser.c"
    break;

  case 34: /* num_comp_expr: num_comp_value TCLE ident  */
#line 167 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_GE, (yyvsp[0].string), (yyvsp[-2].compare_value)); bfree((yyvsp[0].string)); }
#line 1600 "src/parser.c"
    break;

  case 35: /* num_comp_expr: num_comp_value TCGT ident  */
#line 168 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_LT, (yyvsp[0].string), (yyvsp[-2].compare_value)); bfree((yyvsp[0].string)); }
#line 1606 "src/parser.c"
    break;

  case 36: /* num_comp_expr: num_comp_value TCGE ident  */
#line 169 "src/parser.y"
                                                            { (yyval.node) = ast_compare_expr_create(AST_COMPARE_LE, (yyvsp[0].string), (yyvsp[-2].compare_value)); bfree((yyvsp[0].string)); }
#line 1612 "src/parser.c"
    break;

  case 37: /* eq_value: integer  */
#line 172 "src/parser.y"
                                                            { (yyval.equality_value).value_type = AST_EQUALITY_VALUE_INTEGER; (yyval.equality_value).integer_value = (yyvsp[0].integer_value); }
#line 1618 "src/parser.c"
    break;

  case 38: /* eq_value: float  */
#line 173 "src/parser.y"
                                                            { (yyval.equality_value).value_type = AST_EQUALITY_VALUE_FLOAT; (yyval.equality_value).float_value = (yyvsp[0].float_value); }
#line 1624 "src/parser.c"
    break;

  case 39: /* eq_value: string  */
#line 174 "src/parser.y"
                                                            { (yyval.equality_value).value_type = AST_EQUALITY_VALUE_STRING; (yyval.equality_value).string_value = (yyvsp[0].string_value); }
#line 1630 "src/parser.c"
    break;

  case 40: /* eq_expr: ident TCEQ eq_value  */
#line 177 "src/parser.y"
                                                            { (yyval.node) = ast_equality_expr_create(AST_EQUALITY_EQ, (yyvsp[-2].string), (yyvsp[0].equality_value)); bfree((yyvsp[-2].string)); }
#line 1636 "src/parser.c"
    break;

  case 41: /* eq_expr: ident TCNE eq_value  */
#line 178 "src/parser.y"
                                                            { (yyval.node) = ast_equality_expr_create(AST_EQUALITY_NE, (yyvsp[-2].string), (yyvsp[0].equality_value)); bfree((yyvsp[-2].string)); }
#line 1642 "src/parser.c"
    break;

  case 42: /* eq_expr: eq_value TCEQ ident  */
#line 179 "src/parser.y"
                                                            { (yyval.node) = ast_equality_expr_create(AST_EQUALITY_EQ, (yyvsp[0].string), (yyvsp[-2].equality_value)); bfree((yyvsp[0].string)); }
#line 1648 "src/parser.c"
    break;

  case 43: /* eq_expr: eq_value TCNE ident  */
#line 180 "src/parser.y"
                                                            { (yyval.node) = ast_equality_expr_create(AST_EQUALITY_NE, (yyvsp[0].string), (yyvsp[-2].equality_value)); bfree((yyvsp[0].string)); }
#line 1654 "src/parser.c"
    break;

  case 44: /* variable_value: ident  */
#line 183 "src/parser.y"
                                                            { (yyval.variable_value) = make_attr_var((yyvsp[0].string), NULL); bfree((yyvsp[0].string)); }
#line 1660 "src/parser.c"
    break;

  case 45: /* set_left_value: integer  */
#line 185 "src/parser.y"
                                                            { (yyval.set_left_value).value_type = AST_SET_LEFT_VALUE_INTEGER; (yyval.set_left_value).integer_value = (yyvsp[0].integer_value); }
#line 1666 "src/parser.c"
    break;

  case 46: /* set_left_value: string  */
#line 186 "src/parser.y"
                                                            { (yyval.set_left_value).value_type = AST_SET_LEFT_VALUE_STRING; (yyval.set_left_value).string_value = (yyvsp[0].string_value); }
#line 1672 "src/parser.c"
    break;

  case 47: /* set_left_value: variable_value  */
#line 187 "src/parser.y"
                                                            { (yyval.set_left_value).value_type = AST_SET_LEFT_VALUE_VARIABLE; (yyval.set_left_value).variable_value = (yyvsp[0].variable_value); }
#line 1678 "src/parser.c"
    break;

  case 48: /* set_right_value: integer_list_value  */
#line 190 "src/parser.y"
                                                            { (yyval.set_right_value).value_type = AST_SET_RIGHT_VALUE_INTEGER_LIST; (yyval.set_right_value).integer_list_value = (yyvsp[0].integer_list_value); }
#line 1684 "src/parser.c"
    break;

  case 49: /* set_right_value: string_list_value  */
#line 191 "src/parser.y"
                                                            { (yyval.set_right_value).value_type = AST_SET_RIGHT_VALUE_STRING_LIST; (yyval.set_right_value).string_list_value = (yyvsp[0].string_list_value); }
#line 1690 "src/parser.c"
    break;

  case 50: /* set_right_value: variable_value  */
#line 192 "src/parser.y"
                                                            { (yyval.set_right_value).value_type = AST_SET_RIGHT_VALUE_VARIABLE; (yyval.set_right_value).variable_value = (yyvsp[0].variable_value); }
#line 1696 "src/parser.c"
    break;

  case 51: /* set_expr: set_left_value TNOTIN set_right_value  */
#line 195 "src/parser.y"
                                                            { (yyval.node) = ast_set_expr_create(AST_SET_NOT_IN, (yyvsp[-2].set_left_value), (yyvsp[0].set_right_value)); }
#line 1702 "src/parser.c"
    break;

  case 52: /* set_expr: set_left_value TIN set_right_value  */
#line 196 "src/parser.y"
                                                            { (yyval.node) = ast_set_expr_create(AST_SET_IN, (yyvsp[-2].set_left_value), (yyvsp[0].set_right_value)); }
#line 1708 "src/parser.c"
    break;

  case 53: /* list_value: integer_list_value  */
#line 199 "src/parser.y"
                                                            { (yyval.list_value).value_type = AST_LIST_VALUE_INTEGER_LIST; (yyval.list_value).integer_list_value = (yyvsp[0].integer_list_value); }
#line 1714 "src/parser.c"
    break;

  case 54: /* list_value: string_list_value  */
#line 200 "src/parser.y"
                                                            { (yyval.list_value).value_type = AST_LIST_VALUE_STRING_LIST; (yyval.list_value).string_list_value = (yyvsp[0].string_list_value); }
#line 1720 "src/parser.c"
    break;

  case 55: /* list_expr: ident TONEOF list_value  */
#line 203 "src/parser.y"
                                                            { (yyval.node) = ast_list_expr_create(AST_LIST_ONE_OF, (yyvsp[-2].string), (yyvsp[0].list_value)); bfree((yyvsp[-2].string));}
#line 1726 "src/parser.c"
    break;

  case 56: /* list_expr: ident TNONEOF list_value  */
#line 204 "src/parser.y"
                                                            { (yyval.node) = ast_list_expr_create(AST_LIST_NONE_OF, (yyvsp[-2].string), (yyvsp[0].list_value)); bfree((yyvsp[-2].string));}
#line 1732 "src/parser.c"
    break;

  case 57: /* list_expr: ident TALLOF list_value  */
#line 205 "src/parser.y"
                                                            { (yyval.node) = ast_list_expr_create(AST_LIST_ALL_OF, (yyvsp[-2].string), (yyvsp[0].list_value)); bfree((yyvsp[-2].string));}
#line 1738 "src/parser.c"
    break;

  case 58: /* bool_expr: expr TAND expr  */
#line 208 "src/parser.y"
                                                            { (yyval.node) = ast_bool_expr_binary_create(AST_BOOL_AND, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1744 "src/parser.c"
    break;

  case 59: /* bool_expr: expr TOR expr  */
#line 209 "src/parser.y"
                                                            { (yyval.node) = ast_bool_expr_binary_create(AST_BOOL_OR, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 1750 "src/parser.c"
    break;

  case 60: /* bool_expr: TNOT expr  */
#line 210 "src/parser.y"
                                                            { (yyval.node) = ast_bool_expr_unary_create((yyvsp[0].node)); }
#line 1756 "src/parser.c"
    break;

  case 61: /* bool_expr: ident  */
#line 211 "src/parser.y"
                                                            { (yyval.node) = ast_bool_expr_variable_create((yyvsp[0].string)); bfree((yyvsp[0].string)); }
#line 1762 "src/parser.c"
    break;

  case 62: /* bool_expr: TTRUE  */
#line 212 "src/parser.y"
                                                            { (yyval.node) = ast_bool_expr_literal_create(true); }
#line 1768 "src/parser.c"
    break;

  case 63: /* bool_expr: TFALSE  */
#line 213 "src/parser.y"
                                                            { (yyval.node) = ast_bool_expr_literal_create(false); }
#line 1774 "src/parser.c"
    break;

  case 64: /* special_expr: s_frequency_expr  */
#line 216 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1780 "src/parser.c"
    break;

  case 65: /* special_expr: s_segment_expr  */
#line 217 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1786 "src/parser.c"
    break;

  case 66: /* special_expr: s_geo_expr  */
#line 218 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1792 "src/parser.c"
    break;

  case 67: /* special_expr: s_string_expr  */
#line 219 "src/parser.y"
                                                            { (yyval.node) = (yyvsp[0].node); }
#line 1798 "src/parser.c"
    break;

  case 68: /* s_frequency_expr: TWITHINFREQUENCYCAP TLPAREN TSTRING TCOMMA string TCOMMA integer TCOMMA integer TRPAREN  */
#line 223 "src/parser.y"
                                                            { (yyval.node) = ast_special_frequency_create(AST_SPECIAL_WITHINFREQUENCYCAP, (yyvsp[-7].string), (yyvsp[-5].string_value), (yyvsp[-3].integer_value), (yyvsp[-1].integer_value)); bfree((yyvsp[-7].string)); }
#line 1804 "src/parser.c"
    break;

  case 69: /* s_segment_expr: TSEGMENTWITHIN TLPAREN integer TCOMMA integer TRPAREN  */
#line 227 "src/parser.y"
                                                            { (yyval.node) = ast_special_segment_create(AST_SPECIAL_SEGMENTWITHIN, NULL, (yyvsp[-3].integer_value), (yyvsp[-1].integer_value)); }
#line 1810 "src/parser.c"
    break;

  case 70: /* s_segment_expr: TSEGMENTWITHIN TLPAREN ident TCOMMA integer TCOMMA integer TRPAREN  */
#line 229 "src/parser.y"
                                                            { (yyval.node) = ast_special_segment_create(AST_SPECIAL_SEGMENTWITHIN, (yyvsp[-5].string), (yyvsp[-3].integer_value), (yyvsp[-1].integer_value)); bfree((yyvsp[-5].string)); }
#line 1816 "src/parser.c"
    break;

  case 71: /* s_segment_expr: TSEGMENTBEFORE TLPAREN integer TCOMMA integer TRPAREN  */
#line 231 "src/parser.y"
                                                            { (yyval.node) = ast_special_segment_create(AST_SPECIAL_SEGMENTBEFORE, NULL, (yyvsp[-3].integer_value), (yyvsp[-1].integer_value)); }
#line 1822 "src/parser.c"
    break;

  case 72: /* s_segment_expr: TSEGMENTBEFORE TLPAREN ident TCOMMA integer TCOMMA integer TRPAREN  */
#line 233 "src/parser.y"
                                                            { (yyval.node) = ast_special_segment_create(AST_SPECIAL_SEGMENTBEFORE, (yyvsp[-5].string), (yyvsp[-3].integer_value), (yyvsp[-1].integer_value)); bfree((yyvsp[-5].string)); }
#line 1828 "src/parser.c"
    break;

  case 73: /* s_geo_expr: TGEOWITHINRADIUS TLPAREN integer TCOMMA integer TCOMMA integer TRPAREN  */
#line 237 "src/parser.y"
                                                            { (yyval.node) = ast_special_geo_create(AST_SPECIAL_GEOWITHINRADIUS, (double)(yyvsp[-5].integer_value), (double)(yyvsp[-3].integer_value), true, (double)(yyvsp[-1].integer_value)); }
#line 1834 "src/parser.c"
    break;

  case 74: /* s_geo_expr: TGEOWITHINRADIUS TLPAREN float TCOMMA float TCOMMA float TRPAREN  */
#line 239 "src/parser.y"
                                                            { (yyval.node) = ast_special_geo_create(AST_SPECIAL_GEOWITHINRADIUS, (yyvsp[-5].float_value), (yyvsp[-3].float_value), true, (yyvsp[-1].float_value)); }
#line 1840 "src/parser.c"
    break;

  case 75: /* s_string_expr: TCONTAINS TLPAREN ident TCOMMA string TRPAREN  */
#line 243 "src/parser.y"
                                                            { (yyval.node) = ast_special_string_create(AST_SPECIAL_CONTAINS, (yyvsp[-3].string), (yyvsp[-1].string_value).string); bfree((yyvsp[-3].string)); bfree((char*)(yyvsp[-1].string_value).string); }
#line 1846 "src/parser.c"
    break;

  case 76: /* s_string_expr: TSTARTSWITH TLPAREN ident TCOMMA string TRPAREN  */
#line 245 "src/parser.y"
                                                            { (yyval.node) = ast_special_string_create(AST_SPECIAL_STARTSWITH, (yyvsp[-3].string), (yyvsp[-1].string_value).string); bfree((yyvsp[-3].string)); bfree((char*)(yyvsp[-1].string_value).string); }
#line 1852 "src/parser.c"
    break;

  case 77: /* s_string_expr: TENDSWITH TLPAREN ident TCOMMA string TRPAREN  */
#line 247 "src/parser.y"
                                                            { (yyval.node) = ast_special_string_create(AST_SPECIAL_ENDSWITH, (yyvsp[-3].string), (yyvsp[-1].string_value).string); bfree((yyvsp[-3].string)); bfree((char*)(yyvsp[-1].string_value).string); }
#line 1858 "src/parser.c"
    break;


#line 1862 "src/parser.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == XXEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, root, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= XXEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == XXEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner, root);
          yychar = XXEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner, root);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, root, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != XXEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner, root);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner, root);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 250 "src/parser.y"


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "lexer.h"

int parse(const char *text, struct ast_node **node)
{
    // xxdebug = 1;

    // program's only action (`program : expr { *root = $1; }`, above) is
    // the sole place *node ever gets written - it runs as soon as a
    // complete expr is reduced, *before* bison has seen whether anything
    // else follows. So a trailing-garbage input like "flag !!!" still
    // populates *node with a real, fully-built tree even though the
    // overall parse then fails once bison chokes on "!!!" - xxparse's
    // stack-unwind-on-error path doesn't know program's value duplicates
    // *node's, so it silently drops it (program has no %type/%destructor
    // of its own - see below) rather than freeing it. Callers rely on
    // *node being NULL whenever there's nothing valid to free, so init it
    // here rather than leaving it uninitialized: a parse that fails
    // before ever completing a top-level expr (e.g. "country = ") never
    // touches it, and a parse that fails on trailing garbage after a
    // complete one leaves a real, orphaned tree for the caller to free.
    *node = NULL;

    // Parse using Bison.
    yyscan_t scanner;
    xxlex_init(&scanner);
    YY_BUFFER_STATE buffer = xx_scan_string(text, scanner);
    int rc = xxparse(scanner, node);
    xx_delete_buffer(buffer, scanner);
    xxlex_destroy(scanner);
    
    if(rc == 0) {
        return 0;
    }
    else {
        return -1;
    }
}
