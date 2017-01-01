/*   (C) Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007 Stijn van Dongen
 *   (C) Copyright 2008, 2009, 2010, 2011  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/


/*
 *    You probably don't want to look at this code - the reasons are explained
 *    below.  I like the beast though, especially the part that you can specify
 *    callbacks to parse 'external' data.
*/

/*
 * TODO
 *    catch integer overflow.
 *    consider unsigned type. Perhaps implement signed with separate sign.
 *    consider bit operators and unsigned type.
 *    precision.
*/

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include "let.h"
#include "ting.h"
#include "ding.h"
#include "alloc.h"
#include "minmax.h"
#include "err.h"
#include "types.h"
#include "ding.h"
#include "compile.h"


/*  **************************************************************************
 * *
 **            Implementation notes (a few).
 *
* 

   Features
 *    All of C's operators in a revised precedence scheme, with exponentiation
 *    added.  The groups of logical operators, bitwise operators, comparison
 *    operators have equal precedence internally (but changing this is a
 *    matter of editing a single table).  Unsigned integers are not supported
 *    (so bitwise complement behaves funnily).  Ternary operator behaves as
 *    should; evaluates only one of its branches.  Boolean logical operators
 *    do shortcircuit. All mathematical functions from math.h and some
 *    additional ones (e.g. abs, round, sign).  Variables can be parsed and
 *    evaluated using user-supplied functions.  Currently, variables must be
 *    recognizable by a special lead character.

   Todos
 *    I may want to pass raam along to compute and flatten after all.
 *    This brings back in the tokids, which are nice to have.
 *    It also means that global callbacks can be localized.
 *
 *    Right now, inf is not caught. isinf() seems not portable though :(
 *
 *    getatoken could be equipped with more error handling facilities now that
 *    user_parse is inserted.  The current behaviour is that parsing control
 *    is transfered to native parsing if user parsing does not succeed.  This
 *    can be used for overloading the special character, e.g.  setting it to
 *    '!' -> user parsing could require a !<..> sequence; if not found, '!'
 *    would be seen as the negation operator.
 *
 *    user_parse and user_eval need to be global in scope as long
 *    as raam is not passed along in compute and flatten,
 *    but that is actually not a problem (apart from reentrancy).
 *
 *    Hashing of function names.
 *
 *    Audit overflow, exceptions, long/double mixing.
 *
 *    Should min(1,2.0) be 1 rather than 1.0 ?  in that case, need special
 *    behaviour for twoary max and min just like now for oneary abs.
 *
 *    Make int to double overflow promotion a trmInit option.
 *
 *    on STATUS_FAIL for parse, write error message in telraam.
 * 
 *    allow functions with empty arguments (e.g. rand()).
 *
 *    add some of the funny stuff provided by fv, f?
 *
 *    tn toktype is used both in lexing/parsing stage and in reduction stage.
 *    not scalable. Future idea:
 *    Implement an intermediate layer between parsing and evaluation.
 *    E.g. convert the result of trmParse() to a stack.

   Done
 *    Errors cascade back. For parse errors all memory seems to be reclaimed
 *    (for all cases tried so far).
 *    After parsing, no other errors should be possible I believe.
 *
 *    made real type, which could be long double. Tis not however,
 *    because long double math does not seem widespread and/or standard.
 *    made num type, which could be long long. Tis not however,
 *    because long long math seems to be C99 (not widespread etc).
 *
 *    enabled user callbacks for variable and function interpolation.

   Integers/Floats
 *    It is tracked which operations result in integers and which do
 *    not. If integer overflow occurs the result is promoted to double.
 *    The internal logic implementing this behaviour dictates that
 *    as long as (flags & TN_ISINT) it must be true that fval ==~ ival.
 *    TODO: make this customizable.
 *    NOTE: doubles pbb can capture all 32-bit integers, but not so for
 *          64-bit integers. May make subtle difference.

   Apology
 *    It's lame to write your own parser rather than lex and yacc but scriptor
 *    wanted to do it one more time than zero.
 *
 *    The result is pretty ad hoc and not generic, what it's got going for it
 *    is that it works.  Also, trmParse() is not that bad I believe, although
 *    it is not very scalable either.  compute() is the trickiest.  Some
 *    provisions were made to get short-circuiting and the ternary op working.
 *    The same data structure is used for tokenization, parsing, and
 *    evaluation. Ugly!

   Implementation notes
 *  . Parse tree is implicitly stored as a linked list.
 *  . Evaluation is done by compute/flatten; precedence and branching are done
 *    during evaluation, rather than (partly) precomputed (which would much be
 *    cleaner).  precedence is done in flatten; branching in compute.
 *  . tn's (token nodes) are used both by lexer, parser, and interpreter,
 *    which is not the nicest way of doing it.
 *  . Could push and convert all tokens to a stack (format), which would unify
 *    operators and functions to some extent, and separate interpretation from
 *    parsing and braching.

   Errors
 *    Currently we have arithmetic error only.
 *    TODO:
 *    Overflow error -- but isinf does not seem portable :(

   Caveat
 *    tnFree/tnDup should not be applied to a TOKEN_CLOSE node, dupwise
 *    speaking. This is because compute currently has a sanity check for
 *    pointer identity before and after its main loop.
 *
 *    tricky spots, unfinished thoughts, future ideas, and omitted assertions
 *    are marked with the sequence 'mq' (or even 'mqmq'), but not all of them.
 *

   Some reminders
 *    routines that must be checked (malloc dependent)
 *       tnDup
 *       tnNewToken
 *       tnPushToken
 *       tnPushThis
 *    routines that need be checked for other reason
 *       tnUser
 *       flatten
 *       compute
 *       getatom
 *       getexpression
 *       trmParse
*/


typedef double real;          /* but we always use double arithmetic      */
                              /* i.e. never use long double               */

#ifdef LET99
   typedef long long num;
#  define NUM_MIN LLONG_MIN   /* this branch has not been tested!         */
#  define NUM_MAX LLONG_MAX   /* and requires modifiation of trmEval      */
#else               
   typedef long num;
#  define NUM_MIN LONG_MIN
#  define NUM_MAX LONG_MAX
#endif


static int debug_g = 0;
static int (*user_parse_g)(mcxTing* txt, int offset) = NULL;
static mcxenum (*user_eval_g)(const char* token, long *ival, double *fval) = NULL;
static char user_char_g = 0;


typedef struct tn          /* the lex/parse/interpret one stop-shop    */
{  mcxTing*    token
;  i32         toktype

;  i32         optype
;  i32         opid

;  real        fval
;  num         ival
;  struct tn*  prev
;  struct tn*  next

;  i32         flags
;
}  tn;                     /* token node, or whatever */


struct telRaam
{  mcxTing  *text
;  mcxTing  *token         /* current token */
;  char*    p
;  mcxbool  buffered       /* should use buffer (pushed back token)? */
;  tn*      node           /* document symantics */
;  tn*      start
;  real     fval
;  num      ival
;  i32      flags
;  i32      toktype
;  i32      depth
;  tn*      stack          /* when converting parsed expression to stack */
;  int      stack_size
;
}  ;


typedef enum
{  TOKEN_EXH   = -1
,  TOKEN_START =  0     /* special start symbol                         */
,  TOKEN_UNIOP =  1     /* unary, 1                                     */
,  TOKEN_BINOP =  2     /* binary, 2                                    */
,  TOKEN_FUN   =  69    /* can be fun, but I mean 6 = (  9 = )          */
,  TOKEN_TRIOP =  3333  /* ternary, let's stress the fact               */
,  TOKEN_TRICATCH =  6667  /* complement of a number wrt another number */
,  TOKEN_CMP   =  12321 /* hum, dunnow really                           */
,  TOKEN_OR    =  11    /* ||                                           */
,  TOKEN_AND   =  88    /* &&                                           */
,  TOKEN_OPEN  =  6     /* like fun, 6 = (                              */
,  TOKEN_CLOSE =  9     /* like fun, 9 = )                              */
,  TOKEN_COMMA =  13579 /* gaps                                         */
,  TOKEN_CONST =  31415 /* PI                                           */
,  TOKEN_USER  =  981   /* G, variable                                  */
}  tokentype   ;


#define  OP_UNI_NEG     1 <<  0                             /*    -     */
#define  OP_UNI_NOT     1 <<  1                             /*    !     */
#define  OP_UNI_COMPL   1 <<  2                             /*    ~     */
                                                           
#define  OP_EXP_EXP     1 <<  3                             /*    **    */
                                                           
#define  OP_MUL_MUL     1 <<  4                             /*    *     */
#define  OP_MUL_FRAC    1 <<  5                             /*    /     */
#define  OP_MUL_DIV     1 <<  6                             /*    //    */
#define  OP_MUL_MOD     1 <<  7                             /*    %     */
                                                           
#define  OP_ADD_ADD     1 <<  8                             /*    +     */
#define  OP_ADD_SUB     1 <<  9                             /*    -     */
                                                           
#define  OP_BIT_LSHIFT  1 << 10                             /*    <<    */
#define  OP_BIT_RSHIFT  1 << 11                             /*    >>    */
#define  OP_BIT_AND     1 << 12                             /*    &     */
#define  OP_BIT_OR      1 << 13                             /*    |     */
#define  OP_BIT_XOR     1 << 14                             /*    ^     */
                                                           
#define  OP_CMP_LT      1 << 15                             /*    <     */
#define  OP_CMP_LQ      1 << 16                             /*    <=    */
#define  OP_CMP_GQ      1 << 17                             /*    >=    */
#define  OP_CMP_GT      1 << 18                             /*    >     */
#define  OP_CMP_EQ      1 << 19                             /*    ==    */
#define  OP_CMP_NE      1 << 20                             /*    !=    */
                                                           
#define  OP_TRI_START   1 << 21                             /*    !     */


#define  OPTYPE_UNI     (OP_UNI_NEG | OP_UNI_NOT | OP_UNI_COMPL)
#define  OPTYPE_EXP     OP_EXP_EXP
#define  OPTYPE_MUL     (OP_MUL_MUL | OP_MUL_FRAC | OP_MUL_DIV | OP_MUL_MOD)
#define  OPTYPE_ADD     (OP_ADD_ADD | OP_ADD_SUB)
#define  OPTYPE_BIT     (OP_BIT_LSHIFT | OP_BIT_RSHIFT \
                      | OP_BIT_AND | OP_BIT_OR | OP_BIT_XOR)
#define  OPTYPE_CMP     (OP_CMP_LT | OP_CMP_LQ | OP_CMP_GT | OP_CMP_GQ \
                      | OP_CMP_EQ | OP_CMP_NE)
#define  OPTYPE_TRI     OP_TRI_START


typedef struct opHook
{  char*    opname
;  i32      opid
;  i32      optype
;  
}  opHook   ;


double sign
(double f
)
   {  return f > 0 ? 1.0 : f < 0 ? -1.0 : 0.0
;  }


double letround
(double f
)
   { return f > 0 ? floor(f+0.5) : ceil(f-0.5)
;  }


double letlog2
(double f) { return f > 0 ? log(f) / log(2.0) : 0.0 ;  }


typedef struct fun1Hook
{  char*    funname
;  double   (*funcd)(double a)
;  i32      funflags
;
}  fun1Hook  ;

#define  FUN_SPECIAL     1
#define  FUN_INTRESULT   2
#define  FUN_OVERLOADED  4

double show_bits(double a) {
      return a;
}

static fun1Hook fun1HookDir[] =
{
   {  "sin",   sin   ,  0    }
,  {  "cos",   cos   ,  0    }
,  {  "tan",   tan   ,  0    }
,  {  "exp",   exp   ,  0    }
,  {  "log",   log   ,  0    }
,  {  "log10", log10 ,  0    }
,  {  "log2",  letlog2  ,  0    }
,  {  "asin",  asin  ,  0    }
,  {  "acos",  acos  ,  0    }
,  {  "atan",  atan  ,  0    }
,  {  "sqrt",  sqrt  ,  0    }
,  {  "abs",   fabs  ,  FUN_SPECIAL     }
,  {  "floor", floor ,  FUN_INTRESULT   }
,  {  "ceil",  ceil  ,  FUN_INTRESULT   }
,  {  "round", letround ,  FUN_INTRESULT   }
,  {  "int",   letround ,  FUN_INTRESULT   }
,  {  "sign",  sign  ,  FUN_INTRESULT   }
,  {  "bits",  show_bits   ,  FUN_SPECIAL }
,  {   NULL,   NULL  ,  0    }
}  ;


double max ( double a, double b ) { return a > b ? a  : b ; }
double min ( double a, double b ) { return a < b ? a  : b ; }
num maxl  ( num a, num b ) { return a > b ? a  : b ; }
num minl  ( num a, num b ) { return a < b ? a  : b ; }


typedef struct fun2Hook
{  char*    funname
;  double   (*funcd)(double a, double b)
;  num      (*funcl)(num a, num b)
;  i32      funflags
;
}  fun2Hook  ;

static fun2Hook fun2HookDir[] =
{
   {  "max",   max   ,  maxl,  0    }
,  {  "min",   min   ,  minl,  0    }
,  {   NULL,   NULL  ,  NULL,  0    }
}  ;

/* mq
if (tn_isint(lft) && tn_isint(rgt) && hook->funcl)
*/


static opHook opHookDir[] =
{
   {  "-",     OP_UNI_NEG,    OPTYPE_UNI }
,  {  "!",     OP_UNI_NOT,    OPTYPE_UNI }
,  {  "~",     OP_UNI_COMPL,  OPTYPE_UNI }

,  {  "**",    OP_EXP_EXP,    OPTYPE_EXP }

,  {  "*",     OP_MUL_MUL,    OPTYPE_MUL }
,  {  "/",     OP_MUL_FRAC,   OPTYPE_MUL }
,  {  "//",    OP_MUL_DIV,    OPTYPE_MUL }
,  {  "%",     OP_MUL_MOD,    OPTYPE_MUL }

,  {  "+",     OP_ADD_ADD,    OPTYPE_ADD }
,  {  "-",     OP_ADD_SUB,    OPTYPE_ADD }

,  {  "<<",    OP_BIT_LSHIFT, OPTYPE_BIT }
,  {  ">>",    OP_BIT_RSHIFT, OPTYPE_BIT }
,  {  "&",     OP_BIT_AND,    OPTYPE_BIT }
,  {  "|",     OP_BIT_OR,     OPTYPE_BIT }
,  {  "^",     OP_BIT_XOR,    OPTYPE_BIT }

,  {  "<",     OP_CMP_LT,     OPTYPE_CMP }
,  {  "<=",    OP_CMP_LQ,     OPTYPE_CMP }
,  {  ">=",    OP_CMP_GQ,     OPTYPE_CMP }
,  {  ">",     OP_CMP_GT,     OPTYPE_CMP }
,  {  "==",    OP_CMP_EQ,     OPTYPE_CMP }
,  {  "!=",    OP_CMP_NE,     OPTYPE_CMP }

,  {  "?",     OP_TRI_START,  OPTYPE_TRI }

,  {  NULL,    0,             0,         }
}  ;


enum
{  EXPECT_ANY  = 1
,  EXPECT_ATOM = 2
}  ;


#define TN_ISINT   1    /* tn token node, object used everywhere */
#define TN_NOINT   2
#define TN_ISNAN   4
#define TN_ISINF   8

#define tn_isint(a)  (a->flags & TN_ISINT)

mcxbool trmIsNan
(  int  flags
)
   {  return flags & TN_ISNAN 
;  }

mcxbool trmError
(  int  flags
)
   {  return flags & (TN_ISNAN | TN_ISINF)
;  }

mcxbool trmIsInf
(  int  flags
)
   {  return flags & TN_ISINF 
;  }

mcxbool trmIsNum
(  int  flags
)
   {  return flags & TN_ISINT
;  }

mcxbool trmIsReal
(  int  flags
)
   {  return !(flags & (TN_ISINT | TN_ISNAN | TN_ISINF))
;  }


mcxstatus getexpression
(  telRaam *raam
)  ;

mcxstatus getatom
(  telRaam* raam
)  ;


void dump
(  tn* node
,  i32  times
,  const char* msg
)  ;


void trmDump
(  telRaam* raam
,  const char* msg
)
   {  dump(raam->start, 0, msg)
;  }


tn* tnNewToken
(  const char* token 
,  i32         toktype
,  real        fval
,  num         ival
)
   {  tn* node = mcxAlloc(sizeof(tn), RETURN_ON_FAIL)

   ;  if (!node)
      return NULL

   ;  if (!(node->token = mcxTingNew(token ? token : "_<>_")))
      {  mcxFree(node)
      ;  return NULL
   ;  }

      node->toktype  =  toktype
   ;  node->optype   =  0
   ;  node->opid     =  0

   ;  node->ival     =  ival
   ;  node->fval     =  fval

   ;  node->next     =  NULL
   ;  node->prev     =  NULL

   ;  node->flags    =  0

   ;  if (debug_g)
      dump(node, 1, "new node")
   ;  return node
;  }


tn* tnDup
(  tn*   this
,  const char* str
)
   {  tn* new =
      tnNewToken
      (  str
      ,  this->toktype
      ,  this->fval
      ,  this->ival
      )
   ;  if (!new)
      return NULL
      
   ;  new->optype = this->optype
   ;  new->next   =  this->next
   ;  new->prev   =  this->prev
   ;  new->flags  =  this->flags

   ;  return new
;  }


mcxstatus tnFree
(  tn*   lft
,  tn*   rgt
)
   {  tn* cur = lft, *next

   ;  while (cur)
      {  
         mcxTingFree(&(cur->token))

      ;  if (debug_g)
         fprintf(stderr, "___ [telraam] freeing node <%p>\n", (void*) cur)

      ;  if (cur == rgt)
         {  mcxFree(cur)
         ;  break
      ;  }
         if (cur->next && cur->next->prev != cur)
         {  mcxErr("tnFree", "free encountered spaghetti")
         ;  return STATUS_FAIL
      ;  }

         next = cur->next
      ;  mcxFree(cur)
      ;  cur = next
   ;  }
      return STATUS_OK
;  }


void tnLink2
(  tn*   one
,  tn*   two
)
   {  if (one)
      one->next = two
   ;  if (two)
      two->prev = one
;  }


void tnLink3
(  tn*   one
,  tn*   two
,  tn*   three
)
   {  if (one)
      one->next = two
   ;  if (three)
      three->prev = two
   
   ;  two->prev = one
   ;  two->next = three
;  }


mcxstatus tnPushToken
(  telRaam* raam
)
   {  i32  toktype = raam->toktype
   ;  tn* new = tnNewToken(raam->token->str, toktype, 0.0, 0)

   ;  if (!new)
      return STATUS_FAIL

   ;  if (toktype == TOKEN_CONST)
         new->fval   =  raam->fval
      ,  new->ival   =  raam->ival
      ,  new->flags  =  raam->flags

   ;  else if
      (  toktype == TOKEN_BINOP
      || toktype == TOKEN_UNIOP
      )
      {  opHook* oh = raam->toktype == TOKEN_BINOP ? opHookDir+3 : opHookDir+0
                              /* bigg phat ugly hack */
                              /* (need to overcome '-' uni/bin ambiguity */
      ;  while (oh->opname)
         {  if (!strcmp(oh->opname, raam->token->str))
            {  new->optype = oh->optype
            ;  new->opid   = oh->opid
            ;  break
         ;  }
            oh++
      ;  }
         if (!oh->opname)
         {  mcxErr("tnPushToken", "no such operator: <%s>", raam->token->str)
         ;  tnFree(new, NULL)
         ;  return STATUS_FAIL
      ;  }
      }
      else if (raam->toktype == TOKEN_FUN)
      {  /* mq: move name resolution to here ? mm, needs arity */
   ;  }
      tnLink3(raam->node, new, NULL)
   ;  raam->node = new
   ;  return STATUS_OK
;  }


mcxstatus tnPushThis
(  telRaam* raam
,  const char* token 
,  i32       toktype
)
   {  tn* new = tnNewToken(token, toktype, 0.0, 0)

   ;  if (!new)
      return STATUS_FAIL

   ;  tnLink3(raam->node, new, NULL)
   ;  raam->node = new
   ;  return STATUS_OK
;  }


void trmDebug
(  void
)
   {  debug_g = 1
;  }


telRaam* trmInit
(  const char* str
)
   {  telRaam* raam=     mcxAlloc(sizeof(telRaam), RETURN_ON_FAIL)

   ;  if (!raam)
      return NULL

   ;  raam->text   =     mcxTingNew(str)
   ;  raam->token  =     mcxTingEmpty(NULL, 30)
   ;  raam->p      =     raam->text->str
   ;  raam->buffered =   FALSE
   ;  raam->node   =     tnNewToken("_start_", TOKEN_START, 0.0, 0)
   ;  raam->start  =     raam->node
   ;  raam->fval   =     0.0
   ;  raam->ival   =     0
   ;  raam->flags  =     0
   ;  raam->depth  =     1
   ;  raam->toktype=     0
   ;  raam->stack  =     NULL

   ;  if (!raam->text || !raam->token || !raam->node)
         mcxFree(raam)
      ,  raam = NULL

   ;  return raam
;  }


mcxstatus trmExit
(  telRaam*  raam
)
   {  if (tnFree(raam->start, raam->node))
      return STATUS_FAIL
   ;  mcxTingFree(&(raam->text))
   ;  mcxTingFree(&(raam->token))
   ;  mcxFree(raam)
   ;  return STATUS_OK
;  }


void untoken
(  telRaam* raam
)
   {  raam->buffered = TRUE
;  }


void dump
(  tn* node
,  i32  times
,  const char* msg
)
   {  tn* prev = NULL
   ;  printf("______ %s\n", msg ? msg : "dumping dumping dumping")
   ;  printf
("%8s"     "%10s"   "%10s"    "%10s"  "%12s" "%10s" "%6s\n"
,"toktype","optype","opclass","token","fval","ival","flags"
)
   ;  while (node)
      {  printf
("%8d"     "%10d"   "%10d"    "%10s"  "%12.4f""%10ld""%6d\n"
         ,  node->toktype
         ,  node->opid
         ,  node->optype
         ,  node->token ? node->token->str : "<>"
         ,  node->fval
         ,  (long) node->ival
         ,  node->flags
         )
      ;  prev = node
      ;  node = node->next
      ;  if (node && (node->prev->next != node || node->prev != prev))
         fprintf
         (  stderr
         ,  "_____ [telraam] PANICK incorrect linking"
            " <%p> n<%p> np<%p> npn<%p>\n"
         ,  (void*) prev
         ,  (void*) node
         ,  (void*) node->prev
         ,  (void*) node->prev->next
         )
      ;  if (!--times)
         break
   ;  }
   }


int getatoken
(  telRaam* raam
,  i32    mode
)
   {  char* p = raam->p
   ;  i32  toktype = 0
   ;  int len
   ;  while (isspace((unsigned char) *p))
      p++
   ;  raam->p = p

   ;  if (!*p)
      {  mcxTingWrite(raam->token, "EOF")
      ;  return TOKEN_EXH
   ;  }

      else if
      (  mode == EXPECT_ATOM
      && (  *p == '-'
         || *p == '!'
         || *p == '~'
         )
      )
      {  toktype = TOKEN_UNIOP
      ;  p = p+1
   ;  }
      else if (*p == ':')
      {  toktype = TOKEN_TRICATCH
      ;  p = p+1
   ;  }
      else if (*p == '&' && *(p+1) == '&')
      {  toktype = TOKEN_AND
      ;  p = p+2
   ;  }
      else if (*p == '|' && *(p+1) == '|')
      {  toktype = TOKEN_OR
      ;  p = p+2
   ;  }
      else if (*p == '?')
      {  toktype = TOKEN_TRIOP
      ;  p = p+1
   ;  }
      else if (*p == ',')
      {  toktype = TOKEN_COMMA
      ;  p = p+1
   ;  }
      else if (isdigit((unsigned char) *p))
      {  int l
      ;  double f
      ;  sscanf(p, "%lf%n", &f, &l)       /* mq need error checking */
      ;  toktype = TOKEN_CONST
      ;  raam->fval = f
      ;  raam->ival = 0
      ;  if (raam->fval < NUM_MIN || raam->fval > NUM_MAX)
         raam->flags = TN_NOINT
      ;  else
         {  raam->flags = mcxStrChrAint(p, isdigit, l) ? 0 : TN_ISINT
         ;  raam->ival = f > 0 ? f + 0.5 : f - 0.5
      ;  }
         p = p+l
   ;  }
      else if (isalpha((unsigned char) *p) || *p == '_')
      {  char* q = p
      ;  while(isalpha((unsigned char) *q) || *q == '_' || isdigit((unsigned char) *q))
         q++
      ;  p = q
      ;  toktype = TOKEN_FUN
   ;  }
      else if (*p == '(' || *p == ')')
      {  toktype = *p == '(' ? TOKEN_OPEN : TOKEN_CLOSE
      ;  p = p+1
   ;  }
      else if
      (  user_char_g == *p
      && (len = user_parse_g(raam->text, p-raam->text->str)) > 0
      )                               /* ^truncintok */
      {  p += len
      ;  toktype = TOKEN_USER
   ;  }
      else
      {  char* q = p
      ;  while (*q == *p || *q == '=')   /* hack */
         q++
      ;  toktype = TOKEN_BINOP
      ;  p = q
   ;  }
      mcxTingNWrite(raam->token, raam->p, (dim) (p-raam->p))
   ;  raam->p = p
   ;  return toktype
;  }


i32  gettoken
(  telRaam* raam
,  i32  mode
)
   {  if (raam->buffered)
      raam->buffered = FALSE
   ;  else
      raam->toktype = getatoken(raam, mode)

   ;  return raam->toktype
;  }


tn* findop
(  tn* end
)
   {  tn* node = end->prev, *max = NULL
   ;  while (node->toktype != TOKEN_OPEN)
      {  if
         (  node->toktype == TOKEN_UNIOP
         || node->toktype == TOKEN_BINOP
         || node->toktype == TOKEN_TRIOP
         )
         {  if
            (  !max
            || node->optype <= max->optype
            )
            max = node
      ;  }
         node = node->prev
   ;  }
      return max
;  }


tn* finduser
(  tn* start
)
   {  tn* node = start->next

   ;  while (node->toktype != TOKEN_CLOSE)
      {  if (node->toktype == TOKEN_USER)
         return node
      ;  node = node->next
   ;  }
      return NULL
;  }


mcxstatus tnUser
(  tn*   usr
)
   {  mcxenum stat = user_eval_g(usr->token->str, &usr->ival, &usr->fval)
   ;  if (stat == TRM_ISNUM)
      {  usr->flags = TN_ISINT
      ;  usr->fval = usr->ival
   ;  }
      else if (stat == TRM_ISREAL)
      usr->flags = 0
   ;  else if (stat == TRM_ISNAN)
      {  usr->flags = TN_ISNAN
      ;  return STATUS_FAIL
   ;  }
      else if (stat == TRM_FAIL)
      {  usr->flags = TN_ISNAN
      ;  return STATUS_FAIL
   ;  }

      usr->toktype = TOKEN_CONST
   ;  return STATUS_OK 
;  }

/*
 *    Flattens a bunch of leafs interspersed with operators.
 *    Leaves received start and corresponding end alone.
*/

mcxstatus flatten
(  tn* start
,  tn* end
)
   {  tn* new, *op, *usr
   ;  real  fval = 0.0
   ;  num   ival = 0
   ;  const char* me = "flatten"

   ;  if
      (  start->toktype != TOKEN_OPEN
      || end->toktype != TOKEN_CLOSE
      )
      {  mcxErr
         (  me
         ,  "wrong toktype - ids (%p, %p)"
         ,  (void*) start, (void*) end
         )
      ;  dump(start, 0, NULL)
      ;  return STATUS_FAIL
   ;  }

      new = start->next

   ;  while ((usr = finduser(start)))
      {  if (tnUser(usr))
         return STATUS_FAIL
   ;  }

      while ((op = findop(end)))
      {
         tn* lft = op->prev
      ;  tn* rgt = op->next   /* ugly in case of UNIOP */
      ;  int err = 0
      ;  i32  flags = 0

      ;  if (op->toktype == TOKEN_UNIOP)
         {  
            real frgt = rgt->fval
         ;  num irgt = rgt->ival

         ;  switch(op->opid)
            {  case OP_UNI_NOT
            :  ival = (tn_isint(rgt) && irgt) ? 0 : frgt ? 0 : 1
            ;  fval = ival  
            ;  flags |= TN_ISINT
         ;  break

            ;  case OP_UNI_NEG
            :  fval = -frgt
            ;  ival = -irgt
         ;  break
                  
            ;  case OP_UNI_COMPL
            :  ival = ~irgt
            ;  fval =  ival
            ;  flags |= TN_ISINT
         ;  break

            ;  default
            :  err = 1
         ;  }
            lft = op
         ;  flags |=  tn_isint(rgt)
            /* mq fval=ival assignment ugly, need overflow check as well :) */
      ;  }

         else if (op->toktype == TOKEN_BINOP)
         {
            real  flft = lft->fval  
         ;  real  frgt = rgt->fval  
         ;  num   ilft = lft->ival  
         ;  num   irgt = rgt->ival  

         ;  if (op->opid & OPTYPE_BIT)
            {  if (!tn_isint(lft))
               ilft = lft->fval        /* fixme: why the reassign? */
            ;  if (!tn_isint(rgt))
               irgt = rgt->fval        /* fixme: why the reassign? */
            ;  if (!tn_isint(rgt) || !tn_isint(lft))
               mcxErr
               (  "let"
               ,  "[flatten][bitop %s] forcing real operands to number"
               ,  op->token->str
               )
         ;  }

            if (lft->toktype != TOKEN_CONST || rgt->toktype != TOKEN_CONST)
            {  mcxErr(me, "this bifoo is not the right foo")
            ;  dump(start, 0, NULL)
            ;  return STATUS_FAIL
         ;  }

            switch(op->opid)
            {
               case OP_MUL_MUL
            :  fval = flft * frgt
            ;  ival = ilft * irgt
            ;  break

            ;  case OP_EXP_EXP
            :  if (flft < 0 && !(rgt->flags & TN_ISINT))
                  fval = 0.0
               ,  flags |= TN_ISNAN
            ;  else
               fval = pow(flft,frgt)
            ;  ival = letround(fval)
            ;  break

            ;  case OP_ADD_ADD
            :  fval = flft + frgt
            ;  ival = ilft + irgt
            ;  break

            ;  case OP_MUL_FRAC
            :  fval = frgt ? (flft / frgt) : 0.0
            ;  ival = irgt ? (ilft / irgt) : 0
            ;  if (tn_isint(lft) && tn_isint(rgt) && ival * irgt == ilft)
               flags |= TN_ISINT
            ;  else
               flags |= TN_NOINT
            ;  if (!frgt)
               flags |= TN_ISNAN
            ;  break

            ;  case OP_MUL_DIV
            :  fval = frgt ? floor(flft/frgt) : 0.0
            ;  ival = irgt ? (ilft / irgt) : 0
            ;  if (!frgt)
               flags |= TN_ISNAN
            ;  break

            ;  case OP_MUL_MOD
            :  fval = frgt ? frgt * (flft/frgt-floor(flft/frgt)) : 0.0
            ;  ival = irgt ? (ilft % irgt) : 0.0
            ;  if (!frgt)
               flags |= TN_ISNAN
            ;  break

            ;  case OP_ADD_SUB
            :  fval = flft - frgt
            ;  ival = ilft - irgt
            ;  break

            ;  case OP_CMP_LT
            :  ival =   tn_isint(lft) && tn_isint(rgt) && (ilft < irgt)
                        ?  1
                        :  flft < frgt
                           ?  1
                           :  0
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_CMP_LQ
            :  ival =   tn_isint(lft) && tn_isint(rgt) && (ilft <= irgt)
                        ?  1
                        :  flft <= frgt
                           ?  1
                           :  0
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_CMP_GQ
            :  ival =   tn_isint(lft) && tn_isint(rgt) && (ilft >= irgt)
                        ?  1
                        :  flft >= frgt
                           ?  1
                           :  0
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_CMP_GT
            :  ival =   tn_isint(lft) && tn_isint(rgt) && (ilft > irgt)
                        ?  1
                        :  flft > frgt
                           ?  1
                           :  0
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_CMP_EQ
            :  ival =   tn_isint(lft) && tn_isint(rgt) && (ilft == irgt)
                        ?  1
                        :  flft == frgt
                           ?  1
                           :  0
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_CMP_NE
            :  ival =   tn_isint(lft) && tn_isint(rgt) && (ilft != irgt)
                        ?  1
                        :  flft != frgt
                           ?  1
                           :  0
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_BIT_LSHIFT
            :  ival = ilft << irgt
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_BIT_RSHIFT
            :  ival = ilft >> irgt
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_BIT_AND
            :  ival = ilft & irgt
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_BIT_OR
            :  ival = ilft | irgt
            ;  flags |= TN_ISINT
            ;  break

            ;  case OP_BIT_XOR
            :  ival = ilft ^ irgt
            ;  flags |= TN_ISINT
            ;  break

            ;  default
            :  err = 1
         ;  }

               /* this rule implements implicit behaviour with overruling:
                * two integers result in an integer unless overruled
                * with the TN_NOINT attribute.
               */

            if (!(flags & TN_NOINT))
            flags |=  tn_isint(lft) & tn_isint(rgt)

               /* next we check whether overflow occurred. If so, discard the
                * integer attribute.  This depends on i) fval follows ival as
                * long as the integer attribute is set and ii) in that pursuit,
                * fval is computed to be similar to ival.
               */

         ;  if ((fval > NUM_MAX || fval < NUM_MIN) && (flags & TN_ISINT))
            flags ^= TN_ISINT

               /* make fval follow ival, otherwise, give ival special
                * value. *Never* should float->int conversion happen
                * in this code; it should be user-enforced.
                * Setting ival to 0 may help show any such behaviour as a bug.
               */

         ;  if (flags & TN_ISINT)
            fval = ival
         ;  else
            ival = 0
      ;  }
         else
         {  mcxErr(me, "panicking at toktype <%ld>", (long) op->toktype)
         ;  return STATUS_FAIL
      ;  }

         if (err)
         {  mcxErr
            (  me
            ,  "op <%s> id <%ld> class <%ld> not yet supported"
            ,  op->token->str
            ,  (long) op->opid
            ,  (long) op->optype
            )
         ;  return STATUS_FAIL
      ;  }

         if (flags & TN_ISNAN)
         {  mcxErr(me, "arithmetic exception for op <%s>", op->token->str)
         ;  return STATUS_FAIL
      ;  }

         if (!(new = tnNewToken("_eval_", TOKEN_CONST, fval, ival)))
         return STATUS_FAIL

      ;  new->flags = flags
                     /* mq need overflow check, nan check etc */
      ;  tnLink3(lft->prev, new, rgt->next)
      ;  if (tnFree(lft, rgt))
         return STATUS_FAIL
   ;  }

      return STATUS_OK
;  }


fun1Hook* getfun1id
(  tn*   start
)
   {  fun1Hook *fh = fun1HookDir+0
   ;  while (fh->funname && strcmp(fh->funname, start->token->str))
      fh++
   ;  return fh->funname ? fh : NULL
;  }


fun2Hook* getfun2id
(  tn*   start
)
   {  fun2Hook *fh = fun2HookDir+0
   ;  while (fh->funname && strcmp(fh->funname, start->token->str))
      fh++
   ;  return fh->funname ? fh : NULL
;  }


tn* funcx
(  tn* start
,  tn* end
)
   {  tn *new, *arg  = end->prev
   ;  real  fval     =  0.0
   ;  num   ival     =  0
   ;  i32   flags    =  0
   ;  int n_args = arg->toktype == TOKEN_CONST
   ;  int err        =  0
   ;  const char* me = "funcx"
   ;  const char* fn = "_init_"

   ;  if
      (  start->toktype != TOKEN_FUN
      || start->next->toktype != TOKEN_OPEN
      || end->toktype != TOKEN_CLOSE
      )
      {  mcxErr(me, "wrong toktype - ids (%p, %p)", (void*) start, (void*) end)
      ;  dump(start, 0, NULL)
      ;  return NULL
   ;  }

      while
      (  arg->toktype == TOKEN_CONST
      && arg->prev->toktype == TOKEN_COMMA
      )
         arg = arg->prev->prev
      ,  n_args++

   ;  if (arg->prev != start->next)
      {  mcxErr(me, "this function foo is not the right foo")
      ;  dump(start, 0, NULL)
      ;  return NULL
   ;  }

      if (n_args == 1)
      {  tn* op1 = arg
      ;  fun1Hook* fh = getfun1id(start)
      ;  if (fh)
         {  fn    =  fh->funname
         ;  if (fh->funflags & FUN_SPECIAL)
            {  if (!strcmp(fn, "abs"))
               {  if (tn_isint(op1))
                  {  ival = op1->ival > 0 ? op1->ival : -op1->ival
                  ;  flags |= TN_ISINT
               ;  }
                  else
                  fval = op1->fval > 0 ? op1->fval : -op1->fval
            ;  }
               else if (!strcmp(fn, "bits"))
               {  if (tn_isint(op1))
                  {  ival = op1->ival
                  ;  flags |= TN_ISINT
                     /* mq show the damn bits */
               ;  }
                  else
                  fval = op1->fval
                     /* mq show the damn bits */
            ;  }
               else
               err = 1
         ;  }
            else
            {  fval  =  (fh->funcd)(op1->fval)
            ;  if
               (  fh->funflags & FUN_INTRESULT
               && fval <= NUM_MAX
               && fval >= NUM_MIN
               )
               {  flags |= TN_ISINT
               ;  ival = (num) fval > 0 ? fval+0.5 : fval - 0.5
            ;  }
            }
         }
         else
         err = 1
   ;  }
      else if (n_args == 2)
      {  
         tn* op1 = arg, *op2 = op1->next->next
      ;  fun2Hook* fh = getfun2id(start)

      ;  if (fh)
         {  fn    =  fh->funname
         ;  if (tn_isint(op1) && tn_isint(op2) && fh->funcl)
            {  ival = (fh->funcl)(op1->ival, op2->ival)
            ;  flags |= TN_ISINT
         ;  }
            else
            fval = (fh->funcd)(op1->fval, op2->fval)
      ;  }
         else
         err = 1
   ;  }
      else
      err = 1

   ;  if (err)
      {  mcxErr
         (  me
         ,  "<%s> [%d] not supported"
         ,  start->token->str
         ,  n_args
         )
      ;  return NULL
   ;  }
      else
      {  if (!(new = tnNewToken(fn, TOKEN_CONST, fval, ival)))
         return NULL
      ;  new->flags = flags
   ;  }

      return new
;  }


tn* match
(  tn*   start
)
   {  int depth = 1
      
   ;  if (start->toktype != TOKEN_OPEN)
      {  mcxErr("match", "node <%p> has wrong toktype", (void*) start)
      ;  return NULL
   ;  }

      while (start->next)
      {
         start = start->next
      ;  if (start->toktype == TOKEN_OPEN)
         depth++
      ;  else if (start->toktype == TOKEN_CLOSE)
         {  depth--
         ;  if (!depth)
            break
      ;  }
      }
      return depth ? NULL : start
;  }


/*
 *    must leave received start and corresponding end alone
*/

mcxstatus compute
(  tn*   start
)
   {  tn* ptr, *new, *end
   ;  const char* me = "compute"  

   ;  if (start->toktype != TOKEN_OPEN)
      {  mcxErr(me, "node <%p> has wrong toktype", (void*) start)
      ;  return(STATUS_FAIL)
   ;  }

      if (!(end =  match(start)))
      {  mcxErr(me, "node <%p> has no match", (void*) start)
      ;  return(STATUS_FAIL)
   ;  }

      ptr   =  start->next

   ;  while (ptr)                         /* ok by the naming police? */
      {
         tn* eosc, *val

      ;  if (ptr->toktype == TOKEN_FUN)
         {
            if (compute(ptr->next))
            return STATUS_FAIL            /* now:: LPT op CM [op CM]* RPT */
         ;  eosc = match(ptr->next)
         ;  if (!eosc || !(val = funcx(ptr, eosc)))
            return STATUS_FAIL            /* now:: fun LPT val RPT       */
         ;  tnLink3(ptr->prev, val, eosc->next)
         ;  if (tnFree(ptr, eosc))
            return STATUS_FAIL
         ;  ptr = val->next
      ;  }

         else if (ptr->toktype == TOKEN_OPEN)
         {
            if (compute(ptr))
            return STATUS_FAIL

         ;  if (!(eosc = match(ptr)))      /* should check singularity */
            return STATUS_FAIL

         ;  if (!(val = tnDup(eosc->prev, "_scope_")))
            return STATUS_FAIL

         ;  tnLink3(ptr->prev, val, eosc->next)

         ;  if (tnFree(ptr, eosc))
            return STATUS_FAIL

         ;  ptr  =  val->next
      ;  }

                                          /* should check presence TRICATCH */
         else if (ptr->toktype == TOKEN_TRIOP)
         {  tn* br1 = ptr->next, *br2, *eobr1, *eobr2    /* branches */
         ;  if (!(eobr1 = match(br1)))
            return STATUS_FAIL

         ;  if (!(br2 = eobr1->next->next))
            return STATUS_FAIL

         ;  eobr2 = match(br2)

         ;  if (ptr->prev->fval)          /* mqmq! logic by fval */
            {  if (compute(br1))
               return STATUS_FAIL

            ;  if (!(new = tnDup(br1->next, "triop1")))
               return STATUS_FAIL

            ;  tnLink3(ptr->prev->prev, new, eobr2->next)

            ;  if (tnFree(ptr->prev, eobr2))
               return STATUS_FAIL
         ;  }
            else
            {  if (compute(br2))
               return STATUS_FAIL

            ;  if (!(new = tnDup(br2->next, "triop2")))
               return STATUS_FAIL

            ;  tnLink3(ptr->prev->prev, new, eobr2->next)
            ;  if (tnFree(ptr->prev, eobr2))
               return STATUS_FAIL
         ;  }
            ptr = new->next
      ;  }
         else if (ptr->toktype == TOKEN_AND)    /* now:: val AND LPT any RPT */
         {  tn* pivot = ptr->prev, *clause=ptr->next, *after
         ;  if (pivot->fval)
            {  if (compute(clause))
               return STATUS_FAIL
           /* should check singularity of result   */
           /*          lpt      val  rpt    ?      */
            ;  after = clause->next->next->next     /* oops, ugly dugly    */
            ;  pivot->fval = clause->next->fval     /* mqmq! logic by fval */
            ;  if (tnFree(pivot->next, after->prev))
               return STATUS_FAIL
            ;  tnLink2(pivot, after)
            ;  ptr = after
         ;  }
            else
            {  tn* eoclause = match(clause)
            ;  tn* any = eoclause ? eoclause->next : NULL
            ;  if (!eoclause || tnFree(pivot->next, eoclause))
               return STATUS_FAIL
            ;  tnLink2(pivot, any)
            ;  ptr = any
         ;  }
            pivot->ival = pivot->fval ? 1 : 0
         ;  pivot->flags |= TN_ISINT
      ;  }
         else if (ptr->toktype == TOKEN_OR)
         {  tn* pivot = ptr->prev, *clause=ptr->next, *after
         ;  if (pivot->fval)
            {  tn* eoclause = match(clause)
            ;  tn* any = eoclause ? eoclause->next : NULL
            ;  if (!eoclause || tnFree(pivot->next, eoclause))
               return STATUS_FAIL
            ;  tnLink2(pivot, any)
            ;  ptr = any
         ;  }
            else
            {  if (compute(clause))
               return STATUS_FAIL
           /* should check singularity of result    */
            ;  after = clause->next->next->next    /* oops, ugly dugly */
            ;  pivot->fval = clause->next->fval    /* mqmq! logic by fval */
            ;  if (tnFree(pivot->next, after->prev))
               return STATUS_FAIL
            ;  tnLink2(pivot, after)
            ;  ptr = after
         ;  }
            pivot->ival = pivot->fval ? 1 : 0
         ;  pivot->flags |= TN_ISINT
      ;  }
         else if (ptr->toktype == TOKEN_CLOSE)
         break
      ;  else
         ptr  = ptr->next
   ;  }

                  /* NOTE: by design we should always have ptr != NULL
                   * that makes the clause to the while (ptr) { } loop
                   * above a bit dodgy.
                  */
      if (ptr != end || ptr->toktype != TOKEN_CLOSE)
      {  mcxErr(me, "ptr does not close")
      ;  dump(ptr->prev, 0, NULL)
      ;  return STATUS_FAIL
   ;  }

      if (flatten(start, ptr))
      return STATUS_FAIL

   ;  return STATUS_OK
;  }


mcxstatus getatom
(  telRaam* raam
)
   {  i32  toktype = gettoken(raam, EXPECT_ATOM)
   ;  const char* me = "getatom"

   ;  if (toktype < 0)
      {  mcxErr(me, "unexpected token <%s>", raam->token->str)
      ;  return STATUS_FAIL
   ;  }

   ;  if (toktype == TOKEN_UNIOP)
      {  if (tnPushToken(raam))
         return STATUS_FAIL
      ;  if (getatom(raam))
         return STATUS_FAIL
   ;  }
      else if (toktype == TOKEN_OPEN)
      {  if (getexpression(raam))
         return STATUS_FAIL

      ;  if ((toktype = gettoken(raam, EXPECT_ANY)) != TOKEN_CLOSE)
         {  mcxErr(me, "no close (token <%ld>)", (long) toktype)
         ;  return STATUS_FAIL
      ;  }
         if (raam->depth < 0)
         {  mcxErr(me, "spurious rpth (atom I)")
         ;  return STATUS_FAIL
      ;  }
      }
      else if (toktype == TOKEN_FUN)
      {  
         if (tnPushToken(raam))
         return STATUS_FAIL

      ;  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
         return STATUS_FAIL

      ;  if ((toktype = gettoken(raam, EXPECT_ANY)) != TOKEN_OPEN)
         {  mcxErr(me, "expect '(' after function symbol")
         ;  return STATUS_FAIL
      ;  }

         while(1)
         {  if (getexpression(raam))
            return STATUS_FAIL

         ;  if (gettoken(raam, EXPECT_ANY) == TOKEN_COMMA)
            {  if (tnPushToken(raam))
               return STATUS_FAIL
         ;  }
            else
            {  untoken(raam)
            ;  break
         ;  }
         }
         if ((toktype = gettoken(raam, EXPECT_ANY)) != TOKEN_CLOSE)
         {  mcxErr(me, "expect ')' closing function symbol")
         ;  return STATUS_FAIL
      ;  }
         if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
         return STATUS_FAIL
   ;  }
      else if (toktype == TOKEN_CONST)
      {  if (tnPushToken(raam))
         return STATUS_FAIL
   ;  }
      else if (toktype == TOKEN_CLOSE)
      {  mcxErr(me, "empty group not allowed")
      ;  return STATUS_FAIL
   ;  }
      else if (toktype == TOKEN_USER)
      {  if (tnPushToken(raam))
         return STATUS_FAIL
   ;  }
      else
      {  mcxErr(me, "unexpected token <%s> (atom)", raam->token->str)
      ;  return STATUS_FAIL
   ;  }

      return STATUS_OK
;  }


mcxstatus getexpression
(  telRaam* raam
)
   {  i32  toktype
   ;  const char* me = "getexpression"
   ;  raam->depth++

   ;  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
      return STATUS_FAIL

   ;  while (1)
      {  
         if (getatom(raam))
         return STATUS_FAIL

      ;  toktype = gettoken(raam, EXPECT_ANY)

      ;  if (toktype == TOKEN_BINOP)
         {  if (tnPushToken(raam))
            return STATUS_FAIL
      ;  }
         else if (toktype == TOKEN_AND)
         {  if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
            return STATUS_FAIL
         ;  if (tnPushToken(raam))
            return STATUS_FAIL
         ;  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
            return STATUS_FAIL
      ;  }
         else if (toktype == TOKEN_OR)
         {  if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
            return STATUS_FAIL
         ;  if (tnPushToken(raam))
            return STATUS_FAIL
         ;  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
            return STATUS_FAIL
      ;  }
         else if (toktype == TOKEN_TRIOP)
         {  if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
            return STATUS_FAIL

         ;  if (tnPushToken(raam))
            return STATUS_FAIL

         ;  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
            return STATUS_FAIL

         ;  if (getexpression(raam))
            return STATUS_FAIL

         ;  if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
            return STATUS_FAIL

         ;  toktype = gettoken(raam, EXPECT_ANY)

         ;  if (toktype != TOKEN_TRICATCH)
            {  mcxErr
               (  me
               ,  "unexpected token <%s> (expression)"
               ,  raam->token->str
               )
            ;  return STATUS_FAIL
         ;  }
            if (tnPushToken(raam))
            return STATUS_FAIL

         ;  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
            return STATUS_FAIL
      ;  }
         else if
         (  toktype == TOKEN_COMMA
         || toktype == TOKEN_CLOSE
         || toktype == TOKEN_EXH
         || toktype == TOKEN_TRICATCH
         )
         {  untoken(raam)
         ;  break
      ;  }
         else
         {  mcxErr
            (  me
            ,  "unexpected token <%s> <%ld> (expression)"
            ,  raam->token->str
            ,  (long) toktype
            )
         ;  return STATUS_FAIL
      ;  }
      }
   
      if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
      return STATUS_FAIL

   ;  raam->depth--
   ;  return STATUS_OK
;  }


mcxstatus trmParse
(  telRaam* raam
)
   {  if (tnPushThis(raam, "_open_", TOKEN_OPEN))
      return STATUS_FAIL

   ;  if (getexpression(raam))
      return STATUS_FAIL

   ;  if (tnPushThis(raam, "_close_", TOKEN_CLOSE))
      return STATUS_FAIL

   ;  if (gettoken(raam, EXPECT_ANY) != TOKEN_EXH)
      {  mcxErr("trmParse", "spurious token <%s>", raam->token->str)
      ;  return STATUS_FAIL
   ;  }

      return STATUS_OK
;  }


void trmRegister
(  telRaam* raam  cpl__unused
,  int      (user_parse)(mcxTing* txt, int offset)
,  mcxenum  (user_eval)(const char* token, long *ival, double *fval)
,  char     user_char
)
   {  user_parse_g = user_parse
   ;  user_eval_g  = user_eval
   ;  user_char_g  = user_char
;  }


int trmEval
(  telRaam* raam
,  long* lp
,  double* fp
)
   {  tn* result
   ;  mcxstatus stat = compute(raam->start->next)
   ;  result = stat ? NULL : (raam->start->next->next)

   ;  if (result)
      {  *lp = result->ival
      ;  *fp = result->fval
      ;  return result->flags
   ;  }

      return -1
;  }



int trmStack
(  telRaam* raam
)
   {  tn* result
   ;  mcxstatus stat = compute(raam->start->next)
   ;  return stat
;  }


