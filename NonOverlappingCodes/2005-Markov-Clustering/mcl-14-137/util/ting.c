/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "../config.h"
#include "ting.h"
#include "ding.h"
#include "minmax.h"
#include "alloc.h"
#include "array.h"
#include "hash.h"
#include "types.h"
#include "err.h"
#include "tr.h"



void mcxTingRelease
(  void* tingv
)
   {  if (tingv && ((mcxTing*)tingv)->str)
      mcxFree(((mcxTing*)tingv)->str)
;  }


void mcxTingFree
(  mcxTing            **tingpp
)
   {  mcxTing*  ting   =    *tingpp

   ;  if (ting)
      {  if (ting->str)
         mcxFree(ting->str)
      ;  mcxFree(ting)
      ;  *tingpp = NULL
   ;  }
   }


void mcxTingFree_v
(  void  *tingpp
)
   {  mcxTingFree((mcxTing**) tingpp)
;  }


#if 0
void mcxTingAbandon
(  void  *tingpp
)
   {  mcxTing* ting = *((mcxTing**) tingpp)
   ;  mcxFree(ting)
;  }
#endif


mcxTing* mcxTingShrink
(  mcxTing*    ting
,  ofs         offset
)
   {  if (offset < 0)
      offset = ting->len + offset      /* fixme conversion? */

   ;  if (offset < 0 || (dim) offset > ting->len)
      {  mcxErr
         (  "mcxTingShrink"
         ,  "funny offset <%lu> newlen <%ld> combo"
         ,  (ulong) ting->len
         ,  (long) offset
         )
      ;  return ting
   ;  }
      else
      {  *(ting->str+offset) = '\0'
      ;  ting->len = offset;
   ;  }
      return ting
;  }


mcxTing* mcxTingEnsure
(  mcxTing*    ting
,  dim         len
)  
   {  if (!ting && !(ting = (mcxTing*) mcxTingInit(NULL)))
      return NULL
   
   ;  if (len <= ting->mxl)
   ;  else
      {  char* t = mcxRealloc(ting->str, len+1, RETURN_ON_FAIL)
      ;  if (!t)
         return NULL
      ;  ting->str = t
      ;  ting->mxl = len
      ;  if (1)
         *(ting->str+ting->mxl) =  '\0'

        /* there should be no need to do this;
         * and for large allocations this may fail.
        */
   ;  }

      return ting
;  }

#ifndef HAVE_VA_COPY
#define HAVE_VA_COPY 1
#endif

                                    /* todo nul hardening */
static mcxTing*  mcx_ting_print
(  mcxTing*    dst
,  const char* fmt
,  va_list     *args
)
#define PRINT_BUF_SIZE 512
   {  char buf[PRINT_BUF_SIZE], *src
   ;  mcxTing* txtbuf = NULL
   ;  dim m = PRINT_BUF_SIZE
   ;  int npf

         /* Perhaps hook this #definery with configure options that
          * benefit from autoconf machinery.
         */
#if HAVE_VA_COPY
   ;  va_list  args2
   ;  va_copy(args2, *args)
   ;  npf = vsnprintf(buf, PRINT_BUF_SIZE, fmt, args2)
   ;  va_end(args2)
#else
   ;  npf = vsnprintf(buf, PRINT_BUF_SIZE, fmt, *args)
#endif

         /* from reading standards it seems that npf >= PRINT_BUF_SIZE
          * should be sufficient. However, alpha OSF1 provides
          * a counterexample. And from reading standard annotations
          * it seems that vsnprintf is widely ill-implemented
         */

   ;  if (npf < 0 || npf >= PRINT_BUF_SIZE - 1)
      {  
         m  =  (npf >= 0 && npf >= PRINT_BUF_SIZE - 1) ?  ((dim) npf) + 1 : 2 * m

         /* Suppose m is set to npf+1. Then mcxTingEmpty will malloc npf+2
          * bytes and vnsprintf is given m+1=npf+2 as size argument,
          * which is > PRINT_BUF_SIZE.
         */
      ;  while (1)
         {  if (!(txtbuf = mcxTingEmpty(txtbuf, m)))
            {  mcxTingFree(&txtbuf)
            ;  return NULL
         ;  }
#if HAVE_VA_COPY
            va_copy(args2, *args)
         ;  npf = vsnprintf(txtbuf->str, m+1, fmt, args2)
         ;  va_end(args2)
#else
         ;  npf = vsnprintf(txtbuf->str, m+1, fmt, *args)
#endif

            /* Only the npf < 0 case is expected below.
             * The other check is defensive coding. We gave
             * vsnprintf m+1 as size argument, so the check is
             * against one less than that (alpha OSF1 example).
            */
         ;  if (npf < 0 || (dim) npf >= m)
            m *= 2
         ;  else
            break
      ;  }
         src = txtbuf->str
   ;  }
      else
      src = buf

   ;  dst = mcxTingWrite(dst, src)

   ;  mcxTingFree(&txtbuf)
   ;  return dst
;  }


mcxTing*  mcxTingPrintSplice
(  mcxTing*    dst
,  ofs         offset
,  ofs         delete
,  const char* fmt
,  ...
)
   {  va_list  args
   ;  mcxTing *infix = NULL

   ;  va_start(args, fmt)
   ;  infix = mcx_ting_print(NULL, fmt, &args)
   ;  va_end(args)

   ;  if (!infix)
      return NULL

   ;  if (!dst)
      return infix

   ;  if
      (  mcxTingSplice(dst, infix->str, offset, delete, infix->len)
      != STATUS_OK
      )
      {  mcxTingFree(&infix)
      ;  return NULL
   ;  }

      mcxTingFree(&infix)
   ;  return dst
;  }


mcxTing*  mcxTingPrint
(  mcxTing*    dst
,  const char* fmt
,  ...
)
   {  va_list  args

   ;  va_start(args, fmt)
   ;  dst = mcx_ting_print(dst, fmt, &args)
   ;  va_end(args)
   ;  return dst
;  }


mcxTing*  mcxTingPrintAfter
(  mcxTing*    dst
,  const char* fmt
,  ...
)
   {  va_list  args
   ;  mcxTing *affix = NULL

   ;  va_start(args, fmt)
   ;  affix = mcx_ting_print(affix, fmt, &args)
   ;  va_end(args)

   ;  if (!dst)
      return affix

   ;  if (!affix)    /* presumably malloc failure */
      return NULL

   ;  if (!mcxTingAppend(dst, affix->str))
      {  mcxTingFree(&affix)
      ;  return NULL
   ;  }

      mcxTingFree(&affix)
   ;  return dst
;  }


static const char* roman[40] =
{  "" , "i" , "ii", "iii", "iv"  , "v" , "vi", "vii", "viii", "ix"
,  "" , "x" , "xx", "xxx", "xl"  , "l" , "lx", "lxx", "lxxx", "xc"
,  "" , "c" , "cc", "ccc", "cd"  , "d" , "dc", "dcc", "dccc", "cm"
,  "" , "m" , "mm", "mmm", "mmmm", ""  ,  "" ,    "",     "", ""
}  ;


mcxTing*  mcxTingRoman
(  mcxTing*  dst
,  long      a
,  mcxbool   ucase
)
   {  long i, x, c, m
   ;  char* p

   ;  if (a >= 5000 || a <= 0)
      return mcxTingWrite(dst, "-")

   ;  i  =  a % 10
   ;  a /=  10
   ;  x  =  a % 10
   ;  a /=  10
   ;  c  =  a % 10
   ;  a /=  10
   ;  m  =  a

   ;  dst = mcxTingPrint
      (  dst
      ,  "%s%s%s%s"
      ,  roman[30+m]
      ,  roman[20+c]
      ,  roman[10+x]
      ,  roman[ 0+i]
      )
   ;  if (dst && ucase)
      for (p=dst->str;p<dst->str+dst->len;p++)
      *p += 'A' - 'a'

   ;  return dst
;  }


mcxTing*  mcxTingDouble
(  mcxTing* dst
,  double   x
,  int      decimals
)
   {  char num[500]
   ;  char* p
   ;  int len = snprintf(num, 500, "%.*f", decimals, x)

   ;  if (decimals < 0)
      {  mcxErr("mcxTingDouble PBD", "negative decimals arg")
      ;  decimals = 6
   ;  }

      if (len < 0 || len >= 500)
      return mcxTingWrite(dst, "[]")

   ;  p = num+len-1
                              /* GNU libcx bug workaround         */
                              /* it returns 2 for length of 'inf' */ 
   ;  if (decimals && strcmp(num, "inf"))
      {  while (*p == '0')
         p--
      ;  if (*p == '.')
         *++p = '0'
      ;  *++p = '\0'
   ;  }

      return mcxTingWrite(dst, num)
;  }


mcxTing*  mcxTingInteger
(  mcxTing*  dst
,  long     x
)
   {  char num[128]
   ;  int len = snprintf(num, 128, "%ld", x)
   ;  if (len < 0 || len >= 128)
      return mcxTingWrite(dst, "[]")
   ;  return mcxTingWrite(dst, num)
;  }


void* mcxTingInit
(  void *  tingv
)
   {  mcxTing *ting = tingv

   ;  if (!ting)
      {  if (!(ting =  mcxAlloc(sizeof(mcxTing), RETURN_ON_FAIL)))
         return NULL
   ;  }

      if (!(ting->str  =  (char*) mcxAlloc(sizeof(char), RETURN_ON_FAIL)))
      return NULL

   ;  *(ting->str+0) =  '\0'
   ;  ting->len      =  0
   ;  ting->mxl      =  0

   ;  return  ting
;  }


/*
 *    Take string into an existing ting or into a new ting
*/

mcxTing* mcxTingInstantiate
(  mcxTing*          ting
,  const char*       string
)
                                    /* strnotice strlen */
   {  dim length = string ? strlen(string) : 0

                     /* ensure handles ting==NULL and/or length==0  cases */

   ;  if (!(ting = mcxTingEnsure(ting, length)))
      return NULL
                                    /* strnotice strncpy */
	;  if (string)
      {  strncpy(ting->str, string, length)
      ;  *(ting->str+length)  =  '\0'
   ;  }

      ting->len = length
	;  return ting
;  }


int mcxTingRevCmp
(  const void* t1
,  const void* t2
)
   {  return (strcmp(((mcxTing*)t2)->str, ((mcxTing*)t1)->str))
;  }


int mcxTingCmp
(  const void* t1
,  const void* t2
)
   {  return (strcmp(((mcxTing*)t1)->str, ((mcxTing*)t2)->str))
;  }


int mcxTingPCmp
(  const void* t1
,  const void* t2
)
   {  return (strcmp(((mcxTing**)t1)[0]->str, ((mcxTing**)t2)[0]->str))
;  }


int mcxTingPRevCmp
(  const void* t1
,  const void* t2
)
   {  return (strcmp(((mcxTing**)t2)[0]->str, ((mcxTing**)t1)[0]->str))
;  }


int mcxPKeyTingCmp
(  const void* k1
,  const void* k2
)
   {  return
      strcmp
      (  ((mcxTing*) ((mcxKV**)k1)[0]->key)->str
      ,  ((mcxTing*) ((mcxKV**)k2)[0]->key)->str
      )
;  }


int mcxPKeyTingRevCmp
(  const void* k1
,  const void* k2
)
   {  return
      strcmp
      (  ((mcxTing*) ((mcxKV**)k2)[0]->key)->str
      ,  ((mcxTing*) ((mcxKV**)k1)[0]->key)->str
      )
;  }


mcxstatus mcxTingSplice
(  mcxTing*       ting
,  const char*    pstr
,  ofs            offset
,  ofs            n_delete
,  dim            n_copy
)
   {  dim newlen

   ;  if (!ting)
      {  mcxErr("mcxTingSplice PBD", "void ting argument")
      ;  return STATUS_FAIL
   ;  }

              /* offset -1 denotes appending at the end,
               * offset -2 denotes inserting at the last-but-one position.
               * etc
              */
      if (offset < 0)
      {  if ((dim) -offset > ting->len + 1)
         offset = 0
      ;  else
         offset = ting->len + offset + 1
   ;  }
      else if ((dim) offset > ting->len)
      offset = ting->len

   ;  if (n_delete == TING_INS_CENTER)
      {  n_delete = MCX_MIN(ting->len, n_copy)
      ;  offset = (ting->len - n_delete) / 2    /* n_delete <= ting->len */
   ;  }

      else if (n_delete == TING_INS_OVERWRITE)
      n_delete = MCX_MIN(ting->len - offset, n_copy)  /* offset <= ting->len */

   ;  else if (n_delete == TING_INS_OVERRUN || n_delete < 0)
      n_delete = ting->len - offset               /* offset <= ting->len */

   ;  else if (n_delete < 0)
      {  mcxErr
         (  "mcxTingSplice PBD"
         ,  "unsupported delete mode %ld"
         ,  (long) n_delete
         )
      ;  return STATUS_FAIL
   ;  }

      else if ((dim) (offset + n_delete) > ting->len)
      n_delete = ting->len - offset

   ;  if (ting->len + n_copy < (dim) n_delete)  /* now spurious? */
      {  mcxErr("mcxTingSplice PBD", "arguments result in negative length")
      ;  return STATUS_FAIL
   ;  }
           /*
            *  essential: mcxSplice does not know to allocate room for '\0'
           */
      newlen = ting->len - n_delete + n_copy
   ;  if (!mcxTingEnsure(ting, newlen))
      return STATUS_FAIL

   ;  if
      (  mcxSplice
         (  &(ting->str)
         ,  pstr
         ,  sizeof(char)
         ,  &(ting->len)
         ,  &(ting->mxl)
         ,  offset
         ,  (dim) n_delete
         ,  n_copy
         )
      != STATUS_OK
      )
      return STATUS_FAIL

  /*
   *  essential: mcxSplice might have realloced, so has to be done afterwards.
  */
   ;  *(ting->str+newlen)  =  '\0'

   ;  if (ting->len != newlen)
      {  mcxErr("mcxTingSplice panic", "mcxSplice gives unexpected result")
      ;  return STATUS_FAIL
   ;  }
      return STATUS_OK
;  }


mcxTing* mcxTingNew
(  const char* str
)
   {  return mcxTingInstantiate(NULL, str)
;  }


mcxTing* mcxTingNNew
(  const char* str
,  dim         sz
)
   {  mcxTing* ting = mcxTingEnsure(NULL, sz)

   ;  if (!ting)
      return NULL
                                    /* strnotice memcpy */
   ;  if (str && sz)
      memcpy(ting->str, str, sz)
   ;  *(ting->str+sz) = '\0'
   ;  ting->len = sz
   ;  return ting
;  }


mcxTing* mcxTingEmpty
(  mcxTing*    ting
,  dim         len
)
   {  if (!(ting = mcxTingEnsure(ting, len)))
      return NULL

   ;  *(ting->str+0)     =  '\0'
   ,  ting->len          =  0
   ;  return ting
;  }


mcxTing* mcxTingWrite
(  mcxTing*          ting
,  const char*       str
)
   {  return mcxTingInstantiate(ting, str)
;  }


mcxTing* mcxTingNWrite
(  mcxTing*          ting
,  const char*       str
,  dim               sz
)
   {  if (!(ting = mcxTingEnsure(ting, sz)))
      return NULL
                                    /* strnotice strncpy */
   ;  memcpy(ting->str, str, sz)

   ;  *(ting->str+sz) = '\0'
   ;  ting->len = sz

   ;  return ting
;  }


char* mcxTingSubStr
(  const mcxTing*    ting
,  ofs               offset  
,  ofs               length
)
   {  char* str

   ;  if (offset < 0 || (dim) offset > ting->len)
      offset = ting->len

   ;  if (length < 0 || (dim) (offset+length) > ting->len)
      length = ting->len - offset
      
   ;  if (!(str = mcxAlloc((dim) (length+1), RETURN_ON_FAIL)))
      return NULL

   ;  if (length)
      memcpy(str, ting->str+offset, (dim) length)

   ;  *(str+length) = '\0'
   ;  return str
;  }


mcxTing* mcxTingify
(  char* str
)
   {  mcxTing* ting = mcxAlloc(sizeof(mcxTing), RETURN_ON_FAIL)
   ;  ting->str = str
                                    /* strnotice strlen; ? tingify2 */
   ;  ting->len = strlen(str)
   ;  return ting
;  }


char* mcxTinguish
(  mcxTing*    ting
)
   {  char* str = ting->str
   ;  mcxFree(ting)
   ;  return str
;  }


char* mcxTingStr
(  const mcxTing*       ting
)
   {  ofs slen = ting->len
   ;  return mcxTingSubStr(ting, 0, slen)
;  }



mcxTing* mcxTingAppend
(  mcxTing*             ting
,  const char*          str
)
   {  if (!ting)
      return mcxTingNew(str)
                                          /* strnotice strlen */
   ;  if
      (  mcxTingSplice
         (  ting
         ,  str
         , -1                             /*    splice offset     */
         ,  0                             /*    delete nothing    */
         ,  str? strlen(str) : 0          /*    string length     */
         )
      != STATUS_OK
      )
      return NULL

   ;  return ting
;  }


mcxTing* mcxTingKAppend
(  mcxTing*    ting
,  const char* str
,  dim         sz
)
   {  dim len = strlen(str)

   ;  if (!ting && !sz)
      return mcxTingEmpty(NULL, 0)

   ;  if (!sz)
      return ting

   ;  while (sz-- > 0)                    /* careful with unsignedness */
      if (!(ting = mcxTingNAppend(ting, str, len)))
      return NULL

   ;  return ting
;  }


mcxTing* mcxTingNAppend
(  mcxTing*      ting
,  const char*   str
,  dim           sz
)
   {  if (!ting)
      return mcxTingNWrite(NULL, str, sz)

   ;  if
      (  mcxTingSplice
         (  ting
         ,  str
         , -1                             /*    splice offset     */
         ,  0                             /*    delete nothing    */
         ,  sz
         )
      != STATUS_OK
      )
      return NULL

   ;  return ting
;  }


mcxTing* mcxTingInsert
(  mcxTing*             ting
,  const char*          str
,  ofs                  offset
)
   {  if (!ting)
      return mcxTingNew(str)

                                          /* strnotice strlen */
   ;  if
      (  mcxTingSplice
         (  ting
         ,  str
         ,  offset                        /*    splice offset     */
         ,  0                             /*    delete nothing    */
         ,  str ? strlen(str) : 0         /*    string length     */
         )
      != STATUS_OK
      )
      return NULL

   ;  return ting
;  }


mcxTing* mcxTingNInsert
(  mcxTing*          ting
,  const char*       str
,  ofs               offset
,  dim               length
)
   {  if (!ting)
      return mcxTingNWrite(NULL, str, length)

   ;  if
      (  mcxTingSplice
         (  ting
         ,  str
         ,  offset                        /*    splice offset     */
         ,  0                             /*    delete nothing    */
         ,  length                        /*    length of str     */
         )
      != STATUS_OK
      )
      return NULL

   ;  return ting
;  }


mcxTing* mcxTingDelete
(  mcxTing*          ting
,  ofs               offset
,  dim               width
)
   {  ofs swidth =   width
   ;  if (!ting)
      return NULL

   ;  if
      (  mcxTingSplice
         (  ting
         ,  NULL
         ,  offset                        /*    splice offset     */
         ,  swidth                        /*    delete width      */
         ,  0                             /*    string length     */
         )
      != STATUS_OK
      )
      return NULL

   ;  return ting
;  }


u32 mcxTingDJBhash
(  const void* ting
)
   {  return(mcxBJhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingBDBhash
(  const void* ting
)
   {  return(mcxBDBhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingBJhash
(  const void* ting
)
   {  return(mcxBJhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingCThash
(  const void* ting
)
   {  return(mcxCThash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingDPhash
(  const void* ting
)
   {  return(mcxDPhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingGEhash
(  const void* ting
)
   {  return(mcxGEhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingOAThash
(  const void* ting
)
   {  return(mcxOAThash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingELFhash
(  const void* ting
)
   {  return(mcxELFhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingFNVhash
(  const void* ting
)
   {  return(mcxFNVhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 (*mcxTingHFieByName(const char* id))(const void* ting)
   {  if (!strcmp(id, "dp"))
      return mcxTingDPhash
   ;  else if (!strcmp(id, "bj"))
      return mcxTingBJhash
   ;  else if (!strcmp(id, "elf"))
      return mcxTingELFhash
   ;  else if (!strcmp(id, "djb"))
      return mcxTingDJBhash
   ;  else if (!strcmp(id, "bdb"))
      return mcxTingBDBhash
   ;  else if (!strcmp(id, "ge"))
      return mcxTingGEhash
   ;  else if (!strcmp(id, "oat"))
      return mcxTingOAThash
   ;  else if (!strcmp(id, "svd"))
      return mcxTingSvDhash
   ;  else if (!strcmp(id, "svd2"))
      return mcxTingSvD2hash
   ;  else if (!strcmp(id, "svd1"))
      return mcxTingSvD1hash
   ;  else if (!strcmp(id, "ct"))
      return mcxTingCThash
   ;  else if (!strcmp(id, "fnv"))
      return mcxTingFNVhash
   ;  else
      return NULL
;  }


u32 mcxTingSvDhash
(  const void* ting
)
   {  return(mcxSvDhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingSvD2hash
(  const void* ting
)
   {  return(mcxSvD2hash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingSvD1hash
(  const void* ting
)
   {  return(mcxSvD1hash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }


u32 mcxTingHash
(  const void* ting
)
   {  return(mcxDPhash(((mcxTing*) ting)->str, ((mcxTing*) ting)->len))
;  }



/* Fails only for memory reason */

mcxstatus mcxTingTackc
(  mcxTing*  ting
,  unsigned char c
)
   {  if (!ting || !mcxTingEnsure(ting, ting->len+1))
      return STATUS_FAIL
   ;  ting->str[ting->len] = c
   ;  ting->str[ting->len+1] = '\0'
   ;  ting->len += 1
   ;  return STATUS_OK
;  }


/* Fails if last char is not the same as c */

mcxstatus mcxTingTickc
(  mcxTing*  ting
,  unsigned char c
)
   {  if (!ting || !ting->len)
      return STATUS_FAIL
   ;  if ((unsigned char) ting->str[ting->len-1] == c)
      {  mcxTingShrink(ting, -1)
      ;  return STATUS_OK
   ;  }
      return STATUS_FAIL
;  }


