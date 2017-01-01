/*   (C) Copyright 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

/* TODO allow empty args ()
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tok.h"
#include "types.h"
#include "ting.h"
#include "ding.h"
#include "err.h"
#include "alloc.h"
#include "compile.h"


#ifndef DEBUG
#  define DEBUG 0
#else
#  define DEBUG_DEFINED
#endif



char* mcxTokSkip
(  const char* offset
,  int (*fbool)(int c)
,  ofs len
)
   {  return mcxStrChrAint(offset, fbool, len)
;  }


/*
 *  Accounts for nesting.
 *  Will do '}', ')', ']', '>', assuming one of several conventions.
 *  Does not handle string arguments with embedded delimiters, e.g. "foo) bar"
*/

mcxstatus mcxTokMatch
(  const char* str_search
,  char**      endptr
,  mcxbits     mode
,  ofs         len            /* considered if >= 0 */
)
   {  const char* offset= str_search
   ;  unsigned char c   = *offset
   ;  mcxstatus status  = STATUS_OK
   ;  mcxTing* stack
   ;  const char* z

   ;  *endptr = NULL

   ;  if (len < 0)
      len = strlen(offset)

   ;  z = offset + len

   ;  switch(c)
      {  case '{' : case '(' : case '['
      :  break
      ;  
         default
      :  mcxErr("mcxTokMatch", "not my coal of char <%c>", (int) c)
      ;  return STATUS_FAIL
   ;  }

      if (!(stack = mcxTingEmpty(NULL, 80)))
      return STATUS_FAIL

   ;  do
      {  unsigned char m = '\0'
      ;  c = *offset
      ;  switch(c)
         {  case '{' : case '(' : case '['
         :  status = mcxTingTackc(stack, c)
         ;  break
         ;

            case ')' : m = '(' ; break
         ;  case '}' : m = '{' ; break
         ;  case ']' : m = '[' ; break
      ;  }

         if (m)
         status = mcxTingTickc(stack, m)

      ;  if (status)
         break

      ;  if (!stack->len)
         break

      ;  offset++
   ;  }
      while (offset < z)

   ;  if (stack->len)
      status = STATUS_FAIL
   ;  else if (!status)
      *endptr = (char* )offset

   ;  if (status)
      mcxErr
      (  "mcxTokMatch"
      ,  "stacklen <%lu>, offset <%ld>, char <%c>"
      ,  (ulong) stack->len
      ,  (long) (offset - str_search)
      ,  (int) *offset
      )

   ;  mcxTingFree(&stack)
   ;  return status
;  }


mcxstatus mcxTokFind
(  const char* offset
,  char*       tok            /* Only tok[0] considered for now! */
,  char**      pos
,  mcxbits     mode  cpl__unused
,  ofs         len            /* considered if >= 0 */
)
   {  mcxstatus status = STATUS_OK
   ;  const char* x = offset, *z
   ;  char *y = NULL

   ;  if (len < 0)
      len = strlen(offset)

   ;  z     =  offset + len
   ;  *pos  =  NULL

   ;  while (x < z)
      {  if (*x == tok[0])
         break
      ;  switch(*x)
         {  case '('
         :  case '{'
         :  case '['
         :
            if (!(status = mcxTokMatch(x, &y, 0, z-x)))
            x = y+1
         ;  break
         ;

            default
         :  x++
         ;  break
      ;  }
         if (status)
         break
   ;  }

      if (!status)
      {  *pos = (char*) x
      ;  return  *x == tok[0] ? STATUS_OK : STATUS_DONE
   ;  }

       return STATUS_FAIL
;  }


void mcxTokFuncFree
(  mcxTokFunc* tf
)
   {  mcxTingFree(&(tf->key))
   ;  mcxListFree(&(tf->args), mcxTingFree_v)
;  }


mcxstatus mcxTokExpectFunc
(  mcxTokFunc* tf
,  const char* str
,  dim         str_len
,  char**      z_pp
,  int         n_min
,  int         n_max
,  int        *n_args
)
   {  const char* me    =  "mcxTokExpectFunc"
   ;  const char *z     =  str + str_len
   ;  char *x           =  mcxTokSkip(str, isspace, str_len) /* signed issue */
   ;  char *y

   ;  mcxTing* key      =  mcxTingEmpty(NULL, 20)
   ;  mcxTing* args     =  mcxTingEmpty(NULL, 40)
   ;  mcxstatus status  =  STATUS_FAIL
   ;  mcxLink* src      =  NULL
   ;  int ct            =  0

   ;  *z_pp = NULL

   ;  tf->key  =  NULL
   ;  tf->args =  NULL

   ;  if (n_args)
      *n_args = 0

   ;  do
      {  if (!x)
         {  mcxErr(me, "no identifier at EOS")
         ;  break
      ;  }

         y = mcxStrChrAint(x, isalpha, z-x)
      
      ;  if (y==x)
         {  mcxErr(me, "expect identifier: <%s>", x)
         ;  break
      ;  }
         if (!y)
         {  mcxErr(me, "lost identifier: <%s>", x)
         ;  break
      ;  }

         mcxTingNWrite(key, x, y-x)
      ;  x = mcxTokSkip(y, isspace, z-y)

      ;  if (!x || *x != '(')
         {  mcxErr(me, "expect parenthesis: <%s>", x ? x : "EOS")
         ;  break
      ;  }
         if (mcxTokMatch(x, &y, 0, z-x))
         {  mcxErr(me, "error parsing <%s>", x)
         ;  break
      ;  }

         mcxTingNWrite(args, x+1, y-x-1)

      ;  if (!(src = mcxTokArgs(args->str, args->len, &ct, tf->opts)))
         break

      ;  if
         (  ( n_min >= 0 && ct < n_min )
         || ( n_max >= 0 && ct > n_max )
         )
         {  mcxErr
            (  me
            ,  "for key <%s>, arg count %d conflicts min/max %d/%d"
            ,  key->str
            ,  ct, n_min, n_max
            )
         ;  break
      ;  }

         *z_pp = y+1

      ;  status = STATUS_OK
   ;  }
      while (0)

   ;  mcxTingFree(&args)

   ;  if (status)
      {  mcxTingFree(&key)
      ;  mcxListFree(&src, mcxTingFree_v)
   ;  }
      else
      {  tf->key = key
      ;  tf->args = src
      ;  if (n_args)
         *n_args = ct
   ;  }

      return status
;  }


mcxLink* mcxTokArgs
(  const char* str
,  long        str_len
,  int*        n_args
,  mcxbits     opts
)
   {  mcxLink* src   =  mcxListSource(8, MCX_GRIM_ARITHMETIC)
   ;  mcxLink* lk    =  src
   ;  const char* x  =  str
   ;  char* y        =  NULL
   ;  const char* z  =  str + str_len
   ;  mcxTing* arg   =  NULL
   ;  int ct         =  0
   ;  mcxstatus status = STATUS_FAIL

   ;  while ((status = mcxTokFind(x, ",", &y, 0, z-x)) == STATUS_OK || y)
      {  const char* l = x, *r = y
      ;  if (opts & MCX_TOK_DEL_WS)
         {  l = mcxStrChrAint(x, isspace, y-x)
         ;  r = mcxStrRChrAint(x, isspace, y-x)

         ;  if (l && r && l<=r)
            r++
         ;  else
            r = l = x
      ;  }
         arg = mcxTingNNew(l, r-l)
#if DEBUG
;fprintf(stderr, "adding <%s> [%d/ /%s]\n", arg->str, status, y)
#endif
      ;  lk = mcxLinkAfter(lk, arg)
      ;  x = y+1
      ;  ct++
      ;  if (status)
         break
   ;  }

      if (!y)
      {  mcxErr("mcxTokArgs", "error occurred")
      ;  mcxListFree(&src, mcxTingFree_v)
      ;  return NULL
   ;  }

      *n_args = ct
   ;  return src
;  }


#ifndef DEBUG_DEFINED
#  undef  DEBUG
#else
#  undef  DEBUG_DEFINED
#endif


