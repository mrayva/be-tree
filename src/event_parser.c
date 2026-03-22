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
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         ZZSTYPE
/* Substitute the variable and function names.  */
#define yyparse         zzparse
#define yylex           zzlex
#define yyerror         zzerror
#define yydebug         zzdebug
#define yynerrs         zznerrs

/* First part of user prologue.  */
#line 1 "src/event_parser.y"

    #include <stdint.h>
    #include <stdbool.h>
    #include <stdio.h>
    #include <string.h>
    #include "alloc.h"
    #include "ast.h"
    #include "betree.h"
    #include "event_parser.h"
    #include "tree.h"
    #include "value.h"
    struct betree_event *root;
    extern int zzlex();
    void zzerror(void *scanner, const char *s) { (void)scanner; printf("ERROR: %s\n", s); }
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch-default"
    #pragma GCC diagnostic ignored "-Wshadow"
#endif
#line 28 "src/event_parser.y"

    int event_parse(const char *text, struct betree_event **event);

#line 101 "src/event_parser.c"

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

#include "event_parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_EVENT_LCURLY = 3,               /* EVENT_LCURLY  */
  YYSYMBOL_EVENT_RCURLY = 4,               /* EVENT_RCURLY  */
  YYSYMBOL_EVENT_LSQUARE = 5,              /* EVENT_LSQUARE  */
  YYSYMBOL_EVENT_RSQUARE = 6,              /* EVENT_RSQUARE  */
  YYSYMBOL_EVENT_COMMA = 7,                /* EVENT_COMMA  */
  YYSYMBOL_EVENT_COLON = 8,                /* EVENT_COLON  */
  YYSYMBOL_EVENT_MINUS = 9,                /* EVENT_MINUS  */
  YYSYMBOL_EVENT_NULL = 10,                /* EVENT_NULL  */
  YYSYMBOL_EVENT_TRUE = 11,                /* EVENT_TRUE  */
  YYSYMBOL_EVENT_FALSE = 12,               /* EVENT_FALSE  */
  YYSYMBOL_EVENT_INTEGER = 13,             /* EVENT_INTEGER  */
  YYSYMBOL_EVENT_FLOAT = 14,               /* EVENT_FLOAT  */
  YYSYMBOL_EVENT_STRING = 15,              /* EVENT_STRING  */
  YYSYMBOL_YYACCEPT = 16,                  /* $accept  */
  YYSYMBOL_program = 17,                   /* program  */
  YYSYMBOL_variable_loop = 18,             /* variable_loop  */
  YYSYMBOL_variable = 19,                  /* variable  */
  YYSYMBOL_value = 20,                     /* value  */
  YYSYMBOL_boolean = 21,                   /* boolean  */
  YYSYMBOL_integer = 22,                   /* integer  */
  YYSYMBOL_float = 23,                     /* float  */
  YYSYMBOL_string = 24,                    /* string  */
  YYSYMBOL_empty_list_value = 25,          /* empty_list_value  */
  YYSYMBOL_integer_list_value = 26,        /* integer_list_value  */
  YYSYMBOL_integer_list_loop = 27,         /* integer_list_loop  */
  YYSYMBOL_string_list_value = 28,         /* string_list_value  */
  YYSYMBOL_string_list_loop = 29,          /* string_list_loop  */
  YYSYMBOL_segments_value = 30,            /* segments_value  */
  YYSYMBOL_segments_loop = 31,             /* segments_loop  */
  YYSYMBOL_segment_value = 32,             /* segment_value  */
  YYSYMBOL_frequencies_value = 33,         /* frequencies_value  */
  YYSYMBOL_frequencies_loop = 34,          /* frequencies_loop  */
  YYSYMBOL_frequency_value = 35            /* frequency_value  */
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
typedef yytype_int8 yy_state_t;

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
         || (defined ZZSTYPE_IS_TRIVIAL && ZZSTYPE_IS_TRIVIAL)))

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
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   73

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  16
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  39
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  83

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   270


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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15
};

#if ZZDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    94,    94,    95,    98,    99,   102,   103,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   116,   117,   120,
     121,   124,   125,   128,   130,   132,   135,   136,   139,   142,
     143,   146,   149,   150,   154,   157,   160,   161,   165,   167
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if ZZDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "EVENT_LCURLY",
  "EVENT_RCURLY", "EVENT_LSQUARE", "EVENT_RSQUARE", "EVENT_COMMA",
  "EVENT_COLON", "EVENT_MINUS", "EVENT_NULL", "EVENT_TRUE", "EVENT_FALSE",
  "EVENT_INTEGER", "EVENT_FLOAT", "EVENT_STRING", "$accept", "program",
  "variable_loop", "variable", "value", "boolean", "integer", "float",
  "string", "empty_list_value", "integer_list_value", "integer_list_loop",
  "string_list_value", "string_list_loop", "segments_value",
  "segments_loop", "segment_value", "frequencies_value",
  "frequencies_loop", "frequency_value", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-10)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       8,    -3,    29,   -10,    36,     3,   -10,   -10,     4,   -10,
      23,    15,    12,   -10,   -10,   -10,   -10,   -10,   -10,   -10,
     -10,   -10,   -10,   -10,   -10,   -10,   -10,   -10,   -10,   -10,
      18,   -10,    32,   -10,   -10,    -1,    28,    30,   -10,    35,
     -10,   -10,   -10,    31,    42,    43,   -10,    -5,   -10,    37,
     -10,    38,   -10,    46,    47,    -5,    -5,   -10,   -10,    -5,
     -10,    17,   -10,    -5,    49,    51,    52,    37,   -10,    37,
      54,    56,    -5,    58,    59,    -5,    -5,    62,    57,    -5,
     -10,    64,   -10
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     2,     0,     0,     4,     1,     0,     3,
       0,     0,     0,     7,    17,    18,    19,    21,    23,     6,
       8,     9,    10,    11,    12,    13,    14,    15,    16,     5,
       0,    24,     0,    26,    29,     0,     0,     0,    32,     0,
      36,    20,    22,     0,     0,     0,    25,     0,    28,     0,
      31,     0,    35,     0,     0,     0,     0,    27,    30,     0,
      33,     0,    37,     0,     0,     0,     0,     0,    34,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      38,     0,    39
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -10,   -10,   -10,    63,   -10,   -10,    -8,   -10,    -9,   -10,
     -10,   -10,   -10,   -10,   -10,   -10,     2,   -10,   -10,    19
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     5,     6,    19,    20,    45,    22,    23,    24,
      25,    35,    26,    36,    27,    37,    38,    28,    39,    40
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      21,     3,    34,    33,    32,    46,    47,     9,    16,    11,
      10,     1,     4,    12,    13,    14,    15,    16,    17,    18,
      30,    31,    43,    43,    32,    41,    42,    32,    16,     7,
      18,    16,    44,    44,    48,    49,    50,    51,     4,    57,
      58,    52,    53,    59,     8,    41,    54,    64,    65,    55,
      56,    61,    18,    60,    63,    66,    67,    68,    70,    69,
      71,    72,    73,    80,    74,    75,    76,    77,    78,    79,
      82,    81,    62,    29
};

static const yytype_int8 yycheck[] =
{
       8,     4,    11,    11,     9,     6,     7,     4,    13,     5,
       7,     3,    15,     9,    10,    11,    12,    13,    14,    15,
       5,     6,     5,     5,     9,    13,    14,     9,    13,     0,
      15,    13,    15,    15,     6,     7,     6,     7,    15,    47,
      49,     6,     7,     5,     8,    13,    15,    55,    56,     7,
       7,     5,    15,    51,     7,    63,     7,     6,    67,     7,
      69,     7,     6,     6,    72,     7,     7,    75,    76,     7,
       6,    79,    53,    10
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,    17,     4,    15,    18,    19,     0,     8,     4,
       7,     5,     9,    10,    11,    12,    13,    14,    15,    20,
      21,    22,    23,    24,    25,    26,    28,    30,    33,    19,
       5,     6,     9,    22,    24,    27,    29,    31,    32,    34,
      35,    13,    14,     5,    15,    22,     6,     7,     6,     7,
       6,     7,     6,     7,    15,     7,     7,    22,    24,     5,
      32,     5,    35,     7,    22,    22,    22,     7,     6,     7,
      24,    24,     7,     6,    22,     7,     7,    22,    22,     7,
       6,    22,     6
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    16,    17,    17,    18,    18,    19,    19,    20,    20,
      20,    20,    20,    20,    20,    20,    20,    21,    21,    22,
      22,    23,    23,    24,    25,    26,    27,    27,    28,    29,
      29,    30,    31,    31,    32,    33,    34,    34,    35,    35
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     3,     1,     3,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     2,     1,     2,     3,     1,     3,     3,     1,
       3,     3,     1,     3,     5,     3,     1,     3,    11,    13
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = ZZEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == ZZEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use ZZerror or ZZUNDEF. */
#define YYERRCODE ZZUNDEF


/* Enable debugging if requested.  */
#if ZZDEBUG

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
                  Kind, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_EVENT_INTEGER: /* EVENT_INTEGER  */
#line 83 "src/event_parser.y"
         { fprintf(yyoutput, "%lld", ((*yyvaluep).integer_value)); }
#line 788 "src/event_parser.c"
        break;

    case YYSYMBOL_EVENT_FLOAT: /* EVENT_FLOAT  */
#line 84 "src/event_parser.y"
         { fprintf(yyoutput, "%.2f", ((*yyvaluep).float_value)); }
#line 794 "src/event_parser.c"
        break;

    case YYSYMBOL_EVENT_STRING: /* EVENT_STRING  */
#line 85 "src/event_parser.y"
         { fprintf(yyoutput, "%s", ((*yyvaluep).string)); }
#line 800 "src/event_parser.c"
        break;

    case YYSYMBOL_integer: /* integer  */
#line 83 "src/event_parser.y"
         { fprintf(yyoutput, "%lld", ((*yyvaluep).integer_value)); }
#line 806 "src/event_parser.c"
        break;

    case YYSYMBOL_float: /* float  */
#line 84 "src/event_parser.y"
         { fprintf(yyoutput, "%.2f", ((*yyvaluep).float_value)); }
#line 812 "src/event_parser.c"
        break;

    case YYSYMBOL_string: /* string  */
#line 86 "src/event_parser.y"
         { fprintf(yyoutput, "%s", ((*yyvaluep).string_value).string); }
#line 818 "src/event_parser.c"
        break;

    case YYSYMBOL_empty_list_value: /* empty_list_value  */
#line 87 "src/event_parser.y"
         { fprintf(yyoutput, "%zu integers", ((*yyvaluep).integer_list_value).count); }
#line 824 "src/event_parser.c"
        break;

    case YYSYMBOL_integer_list_value: /* integer_list_value  */
#line 87 "src/event_parser.y"
         { fprintf(yyoutput, "%zu integers", ((*yyvaluep).integer_list_value).count); }
#line 830 "src/event_parser.c"
        break;

    case YYSYMBOL_integer_list_loop: /* integer_list_loop  */
#line 87 "src/event_parser.y"
         { fprintf(yyoutput, "%zu integers", ((*yyvaluep).integer_list_value).count); }
#line 836 "src/event_parser.c"
        break;

    case YYSYMBOL_string_list_value: /* string_list_value  */
#line 88 "src/event_parser.y"
         { fprintf(yyoutput, "%zu strings", ((*yyvaluep).string_list_value).count); }
#line 842 "src/event_parser.c"
        break;

    case YYSYMBOL_string_list_loop: /* string_list_loop  */
#line 88 "src/event_parser.y"
         { fprintf(yyoutput, "%zu strings", ((*yyvaluep).string_list_value).count); }
#line 848 "src/event_parser.c"
        break;

    case YYSYMBOL_segments_value: /* segments_value  */
#line 89 "src/event_parser.y"
         { fprintf(yyoutput, "%zu segments", ((*yyvaluep).segments_list_value).size); }
#line 854 "src/event_parser.c"
        break;

    case YYSYMBOL_segments_loop: /* segments_loop  */
#line 89 "src/event_parser.y"
         { fprintf(yyoutput, "%zu segments", ((*yyvaluep).segments_list_value).size); }
#line 860 "src/event_parser.c"
        break;

    case YYSYMBOL_frequencies_value: /* frequencies_value  */
#line 90 "src/event_parser.y"
         { fprintf(yyoutput, "%zu caps", ((*yyvaluep).frequencies_value).size); }
#line 866 "src/event_parser.c"
        break;

    case YYSYMBOL_frequencies_loop: /* frequencies_loop  */
#line 90 "src/event_parser.y"
         { fprintf(yyoutput, "%zu caps", ((*yyvaluep).frequencies_value).size); }
#line 872 "src/event_parser.c"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner);
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
                 int yyrule, void *scanner)
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
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !ZZDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ZZDEBUG */


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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
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

  yychar = ZZEMPTY; /* Cause a token to be read.  */

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
  if (yychar == ZZEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= ZZEOF)
    {
      yychar = ZZEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == ZZerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = ZZUNDEF;
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
  yychar = ZZEMPTY;
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
  case 2: /* program: EVENT_LCURLY EVENT_RCURLY  */
#line 94 "src/event_parser.y"
                                                                { root = make_empty_event(); }
#line 1269 "src/event_parser.c"
    break;

  case 3: /* program: EVENT_LCURLY variable_loop EVENT_RCURLY  */
#line 95 "src/event_parser.y"
                                                                { root = (yyvsp[-1].event); }
#line 1275 "src/event_parser.c"
    break;

  case 4: /* variable_loop: variable  */
#line 98 "src/event_parser.y"
                                                            { (yyval.event) = make_empty_event(); add_variable((yyvsp[0].variable), (yyval.event)); }
#line 1281 "src/event_parser.c"
    break;

  case 5: /* variable_loop: variable_loop EVENT_COMMA variable  */
#line 99 "src/event_parser.y"
                                                            { add_variable((yyvsp[0].variable), (yyvsp[-2].event)); (yyval.event) = (yyvsp[-2].event); }
#line 1287 "src/event_parser.c"
    break;

  case 6: /* variable: EVENT_STRING EVENT_COLON value  */
#line 102 "src/event_parser.y"
                                                            { (yyval.variable) = make_pred((yyvsp[-2].string), INVALID_VAR, (yyvsp[0].value)); bfree((yyvsp[-2].string)); }
#line 1293 "src/event_parser.c"
    break;

  case 7: /* variable: EVENT_STRING EVENT_COLON EVENT_NULL  */
#line 103 "src/event_parser.y"
                                                            { (yyval.variable) = NULL; bfree((yyvsp[-2].string)); }
#line 1299 "src/event_parser.c"
    break;

  case 8: /* value: boolean  */
#line 106 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_BOOLEAN; (yyval.value).boolean_value = (yyvsp[0].boolean_value); }
#line 1305 "src/event_parser.c"
    break;

  case 9: /* value: integer  */
#line 107 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_INTEGER; (yyval.value).integer_value = (yyvsp[0].integer_value); }
#line 1311 "src/event_parser.c"
    break;

  case 10: /* value: float  */
#line 108 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_FLOAT; (yyval.value).float_value = (yyvsp[0].float_value); }
#line 1317 "src/event_parser.c"
    break;

  case 11: /* value: string  */
#line 109 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_STRING; (yyval.value).string_value = (yyvsp[0].string_value); }
#line 1323 "src/event_parser.c"
    break;

  case 12: /* value: empty_list_value  */
#line 110 "src/event_parser.y"
                                                            {
                                                                (yyval.value).value_type = BETREE_INTEGER_LIST;
                                                                (yyval.value).integer_list_value = (yyvsp[0].integer_list_value);
                                                                (yyval.value).string_list_value = (struct betree_string_list*)(yyvsp[0].integer_list_value);
                                                                (yyval.value).segments_value = (struct betree_segments*)(yyvsp[0].integer_list_value);
                                                                (yyval.value).frequency_caps_value = (struct betree_frequency_caps*)(yyvsp[0].integer_list_value);
                                                            }
#line 1329 "src/event_parser.c"
    break;

  case 13: /* value: integer_list_value  */
#line 111 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_INTEGER_LIST; (yyval.value).integer_list_value = (yyvsp[0].integer_list_value); }
#line 1335 "src/event_parser.c"
    break;

  case 14: /* value: string_list_value  */
#line 112 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_STRING_LIST; (yyval.value).string_list_value = (yyvsp[0].string_list_value); }
#line 1341 "src/event_parser.c"
    break;

  case 15: /* value: segments_value  */
#line 113 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_SEGMENTS; (yyval.value).segments_value = (yyvsp[0].segments_list_value); }
#line 1347 "src/event_parser.c"
    break;

  case 16: /* value: frequencies_value  */
#line 114 "src/event_parser.y"
                                                            { (yyval.value).value_type = BETREE_FREQUENCY_CAPS; (yyval.value).frequency_caps_value = (yyvsp[0].frequencies_value); }
#line 1353 "src/event_parser.c"
    break;

  case 17: /* boolean: EVENT_TRUE  */
#line 116 "src/event_parser.y"
                                                            { (yyval.boolean_value) = true; }
#line 1359 "src/event_parser.c"
    break;

  case 18: /* boolean: EVENT_FALSE  */
#line 117 "src/event_parser.y"
                                                            { (yyval.boolean_value) = false; }
#line 1365 "src/event_parser.c"
    break;

  case 19: /* integer: EVENT_INTEGER  */
#line 120 "src/event_parser.y"
                                                            { (yyval.integer_value) = (yyvsp[0].integer_value); }
#line 1371 "src/event_parser.c"
    break;

  case 20: /* integer: EVENT_MINUS EVENT_INTEGER  */
#line 121 "src/event_parser.y"
                                                            { (yyval.integer_value) = - (yyvsp[0].integer_value); }
#line 1377 "src/event_parser.c"
    break;

  case 21: /* float: EVENT_FLOAT  */
#line 124 "src/event_parser.y"
                                                            { (yyval.float_value) = (yyvsp[0].float_value); }
#line 1383 "src/event_parser.c"
    break;

  case 22: /* float: EVENT_MINUS EVENT_FLOAT  */
#line 125 "src/event_parser.y"
                                                            { (yyval.float_value) = - (yyvsp[0].float_value); }
#line 1389 "src/event_parser.c"
    break;

  case 23: /* string: EVENT_STRING  */
#line 128 "src/event_parser.y"
                                                            { (yyval.string_value).string = bstrdup((yyvsp[0].string)); (yyval.string_value).str = INVALID_STR; bfree((yyvsp[0].string)); }
#line 1395 "src/event_parser.c"
    break;

  case 24: /* empty_list_value: EVENT_LSQUARE EVENT_RSQUARE  */
#line 130 "src/event_parser.y"
                                                            { (yyval.integer_list_value) = make_integer_list(); }
#line 1401 "src/event_parser.c"
    break;

  case 25: /* integer_list_value: EVENT_LSQUARE integer_list_loop EVENT_RSQUARE  */
#line 133 "src/event_parser.y"
                                                            { (yyval.integer_list_value) = (yyvsp[-1].integer_list_value); }
#line 1407 "src/event_parser.c"
    break;

  case 26: /* integer_list_loop: integer  */
#line 135 "src/event_parser.y"
                                                            { (yyval.integer_list_value) = make_integer_list(); add_integer_list_value((yyvsp[0].integer_value), (yyval.integer_list_value)); }
#line 1413 "src/event_parser.c"
    break;

  case 27: /* integer_list_loop: integer_list_loop EVENT_COMMA integer  */
#line 136 "src/event_parser.y"
                                                            { add_integer_list_value((yyvsp[0].integer_value), (yyvsp[-2].integer_list_value)); (yyval.integer_list_value) = (yyvsp[-2].integer_list_value); }
#line 1419 "src/event_parser.c"
    break;

  case 28: /* string_list_value: EVENT_LSQUARE string_list_loop EVENT_RSQUARE  */
#line 140 "src/event_parser.y"
                                                            { (yyval.string_list_value) = (yyvsp[-1].string_list_value); }
#line 1425 "src/event_parser.c"
    break;

  case 29: /* string_list_loop: string  */
#line 142 "src/event_parser.y"
                                                            { (yyval.string_list_value) = make_string_list(); add_string_list_value((yyvsp[0].string_value), (yyval.string_list_value)); }
#line 1431 "src/event_parser.c"
    break;

  case 30: /* string_list_loop: string_list_loop EVENT_COMMA string  */
#line 143 "src/event_parser.y"
                                                            { add_string_list_value((yyvsp[0].string_value), (yyvsp[-2].string_list_value)); (yyval.string_list_value) = (yyvsp[-2].string_list_value); }
#line 1437 "src/event_parser.c"
    break;

  case 31: /* segments_value: EVENT_LSQUARE segments_loop EVENT_RSQUARE  */
#line 147 "src/event_parser.y"
                                                            { (yyval.segments_list_value) = (yyvsp[-1].segments_list_value); }
#line 1443 "src/event_parser.c"
    break;

  case 32: /* segments_loop: segment_value  */
#line 149 "src/event_parser.y"
                                                            { (yyval.segments_list_value) = make_segments(); add_segment((yyvsp[0].segment_value), (yyval.segments_list_value)); }
#line 1449 "src/event_parser.c"
    break;

  case 33: /* segments_loop: segments_loop EVENT_COMMA segment_value  */
#line 151 "src/event_parser.y"
                                                            { add_segment((yyvsp[0].segment_value), (yyvsp[-2].segments_list_value)); (yyval.segments_list_value) = (yyvsp[-2].segments_list_value); }
#line 1455 "src/event_parser.c"
    break;

  case 34: /* segment_value: EVENT_LSQUARE integer EVENT_COMMA integer EVENT_RSQUARE  */
#line 155 "src/event_parser.y"
                                                            { (yyval.segment_value) = make_segment((yyvsp[-3].integer_value), (yyvsp[-1].integer_value)); }
#line 1461 "src/event_parser.c"
    break;

  case 35: /* frequencies_value: EVENT_LSQUARE frequencies_loop EVENT_RSQUARE  */
#line 158 "src/event_parser.y"
                                                            { (yyval.frequencies_value) = (yyvsp[-1].frequencies_value); }
#line 1467 "src/event_parser.c"
    break;

  case 36: /* frequencies_loop: frequency_value  */
#line 160 "src/event_parser.y"
                                                            { (yyval.frequencies_value) = make_frequency_caps(); add_frequency((yyvsp[0].frequency_value), (yyval.frequencies_value)); }
#line 1473 "src/event_parser.c"
    break;

  case 37: /* frequencies_loop: frequencies_loop EVENT_COMMA frequency_value  */
#line 162 "src/event_parser.y"
                                                            { add_frequency((yyvsp[0].frequency_value), (yyvsp[-2].frequencies_value)); (yyval.frequencies_value) = (yyvsp[-2].frequencies_value); }
#line 1479 "src/event_parser.c"
    break;

  case 38: /* frequency_value: EVENT_LSQUARE EVENT_STRING EVENT_COMMA integer EVENT_COMMA string EVENT_COMMA integer EVENT_COMMA integer EVENT_RSQUARE  */
#line 166 "src/event_parser.y"
                                                            { (yyval.frequency_value) = make_frequency_cap((yyvsp[-9].string), (yyvsp[-7].integer_value), (yyvsp[-5].string_value), true, (yyvsp[-1].integer_value), (yyvsp[-3].integer_value)); bfree((yyvsp[-9].string)); }
#line 1485 "src/event_parser.c"
    break;

  case 39: /* frequency_value: EVENT_LSQUARE EVENT_LSQUARE EVENT_STRING EVENT_COMMA integer EVENT_COMMA string EVENT_RSQUARE EVENT_COMMA integer EVENT_COMMA integer EVENT_RSQUARE  */
#line 168 "src/event_parser.y"
                                                            { (yyval.frequency_value) = make_frequency_cap((yyvsp[-10].string), (yyvsp[-8].integer_value), (yyvsp[-6].string_value), true, (yyvsp[-1].integer_value), (yyvsp[-3].integer_value)); bfree((yyvsp[-10].string)); }
#line 1491 "src/event_parser.c"
    break;


#line 1495 "src/event_parser.c"

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
  yytoken = yychar == ZZEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= ZZEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == ZZEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, scanner);
          yychar = ZZEMPTY;
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner);
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
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != ZZEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 171 "src/event_parser.y"


#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "event_lexer.h"

int event_parse(const char *text, struct betree_event** event)
{
    // zzdebug = 1;
    
    // Parse using Bison.
    yyscan_t scanner;
    zzlex_init(&scanner);
    YY_BUFFER_STATE buffer = zz_scan_string(text, scanner);
    int rc = zzparse(scanner);
    zz_delete_buffer(buffer, scanner);
    zzlex_destroy(scanner);
    
    if(rc == 0) {
        *event = root;
        return 0;
    }
    else {
        return -1;
    }
}
