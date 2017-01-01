/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "glob.h"
#include "stack.h"
#include "ops.h"       /* maybe opFunc should be moved to mcxshared */
#include "util.h"

#include "util/alloc.h"
#include "util/hash.h"
#include "util/ting.h"
#include "util/err.h"
#include "util/minmax.h"

#include "impala/matrix.h"
#include "impala/compose.h"

/*
 *    Beginning the separation of interface type and storage type.
 *    interface will be denoted by utype, storage by glype.
 *    This is all still very clumsy, but it will show what is needed.
 *    Clumsy because the coupling is still fully there, only the naming
 *    shows the intention of usage.
 *
 *    GLYPE_... GLYPE_... and GLASS_.. are internal storage notions.
 *    UTYPE_... is the interface notion.
 *    
 *    zgNewHandle takes a glype.
 *    zgNew takes a utype.
 *
 *    zgNewHandle does *not* correspond with glob/utype_NEWHDL.
 *    it creates a new handle object for something which is not a
 *    NEWHDL. The NEWHDL type is handled by zgNew.
 *    So this bit of naming shcks.
 *
 *    The glype/glass stuff should be pushed further into its own
 *    envelope. This should probably be done by making types just
 *    integers, and letting the even numbers for example correspond
 *    to handles, and working with arrays for what are now GLASS notions.
 *    On a further note, letting each type have a corresponding handle
 *    type is probably stupid, as it introduces a correspondence that has
 *    no natural place for checking and that has actually no use.
 *    I should make a generic handle type.
*/

typedef struct zgglob_t
{  int               glype       /* glob type */
;  mclMatrix         *mx
;  mcxTing           *ting
;  double            d
;  int               i
;
}  zgglob_t          ;


static mcxHash *hdltable_g    =  NULL;

#define glob__none      0
#define glob__newhdl   (1 << 0)
#define glob__int      (1 << 1)
#define glob__inthdl   (1 << 2)
#define glass__int     (glob__int | glob__inthdl)
#define glob__dbl      (1 << 3)
#define glob__dblhdl   (1 << 4)
#define glass__dbl     (glob__dbl | glob__dblhdl)
#define glob__mx       (1 << 5)
#define glob__mxhdl    (1 << 6)
#define glass__mx      (glob__mx  | glob__mxhdl )
#define glob__str      (1 << 7)
#define glob__strhdl   (1 << 8)
#define glass__str     (glob__str | glob__strhdl)
#define glob__seq      (1 << 9)
#define glob__seqhdl   (1 << 10)
#define glass__seq     (glob__seq | glob__seqhdl)

#define glass__num     (glass__int | glass__dbl)
#define glass__hdl     (  glob__newhdl\
                       |  glob__inthdl\
                       |  glob__dblhdl\
                       |  glob__strhdl\
                       |  glob__seqhdl\
                       |  glob__mxhdl\
                       )

int mapUtype[N_UTYPE]   =  {  glob__none
                           ,  glob__int
                           ,  glob__dbl
                           ,  glob__mx
                           ,  glob__str
                           ,  glob__seq
                           ,  glob__newhdl
                           }  ;
/*  ################## above and below is coupled (ugly indeed) ############ */
int UTYPE_NONE             =  0;
int UTYPE_INT              =  1;
int UTYPE_DBL              =  2;
int UTYPE_MX               =  3;
int UTYPE_STR              =  4;
int UTYPE_SEQ              =  5;
int UTYPE_NEWHDL           =  6;

int UCLASS_NUM[N_UTYPE] =  {  0
                           ,  1
                           ,  1
                           ,  0
                           ,  0
                           ,  0
                           ,  0
                           }  ;

const char* UNAME[N_UTYPE] =
                           {  "none"
                           ,  "int"
                           ,  "dbl"
                           ,  "mtx"
                           ,  "str"
                           ,  "seq"
                           ,  "newhdl"
                           }  ;

/* these are genuine objects: things a glob can encode */
#define glass__ob    (glob__int | glob__dbl | glob__str | glob__seq | glob__mx)

/* these are the objects that are encoded in a ting member */
#define glass__ting  (glob__str | glob__seq | glass__hdl)

int GLYPE_NONE       =  glob__none;
int GLYPE_NEWHDL     =  glob__newhdl;
                  
int GLYPE_INT        =  glob__int;
int GLYPE_INTHDL     =  glob__inthdl;
int    GLASS_INT     =  glass__int;
int GLYPE_DBL        =  glob__dbl;
int GLYPE_DBLHDL     =  glob__dblhdl;
int    GLASS_DBL     =  glass__dbl;
int GLYPE_MX         =  glob__mx;
int GLYPE_MXHDL      =  glob__mxhdl;
int    GLASS_MX      =  glass__mx;
int GLYPE_STR        =  glob__str;
int GLYPE_STRHDL     =  glob__strhdl;
int    GLASS_STR     =  glass__str;
int GLYPE_SEQ        =  glob__seq;
int GLYPE_SEQHDL     =  glob__seqhdl;
int    GLASS_SEQ     =  glass__seq;
                  
int    GLASS_NUM     =  glass__num;
int    GLASS_HDL     =  glass__hdl;
                  
int    GLASS_OB      =  glass__ob;
int    GLASS_TING    =  glass__ting;

int zgt(int glype)         ;

static int one_g   =    1  ;
static int zero_g  =    0  ;


zgglob_p zgGetGlobByHandle
(  mcxTing* ting
)  ;
int zgGlypeType
(  zgglob_p   glob
)  ;
int zgGetGlype
(  zgglob_p   glob
)  ;
void* zgGetZgMem
(  zgglob_p glob,  int class
)  ;
int zgPushHandle
(  int         glype
,  void*       object
)  ;

void globInitialize
(  void
)
   {  hdltable_g = mcxHashNew(50,  mcxTingHash, mcxTingCmp)
;  }


void globExit (void)
   {  mcxHashFree(&hdltable_g, mcxTingRelease, NULL)
;  }


int zgUser
(  const char*    token
)
   {  zgglob_p glob =  zsEmpty() ? NULL : zsGetGlob(0)
                     /* ugly hack: glob->i denotes the curly nesting level */
   ;  if (glob && glob->glype == GLYPE_SEQ && glob->i)
      {  if (!strcmp(token, "{"))
         glob->i += 1
      ;  else if (!strcmp(token, "}"))
         glob->i -= 1

      ;  if (glob->i)
         {  mcxTingAppend(glob->ting, " ")
         ;  mcxTingAppend(glob->ting, token)
         ;  zmTell('t', "appending [%s] to sequence", token)
      ;  }
         else
         zmTell('t', "end of sequence")
   ;  }
      else if (!strcmp(token, "{"))
      {  mcxTing* seqting = mcxTingEmpty(NULL, 5)
      ;  zgPush(UTYPE_SEQ, seqting)
      ;  glob    =  zsGetGlob(0)
      ;  glob->i =  1
      ;  zmTell('t', "beginning of sequence")
   ;  }
      else if (!strcmp(token, "}"))
      {  zmTell('e', "closing curly on the loose")
      ;  return 0
   ;  }
      else if (!strncmp(token, ".", 1))
      {  mcxTing* tokenting = mcxTingNew(token+1)
      ;  mcxKV*   kv

      ;  kv = mcxHashSearch(tokenting, hdltable_g, MCX_DATUM_FIND)

      ;  if (kv)
         {  zgglob_p glob = (zgglob_p) kv->val
         ;  zmTell
            (  'd'
            ,  "pushing known handle [%s] glype <%d>"
            ,  tokenting->str
            ,  glob->glype
            )
         ;  return zgPushHandle(glob->glype, tokenting)
      ;  }
         else
         {  zmTell
            (  'd'
            ,  "pushing new handle [%s] utype <%d>"
            ,  tokenting->str
            ,  UTYPE_NEWHDL
            )
         ;  return zgPush(UTYPE_NEWHDL, tokenting)
      ;  }
      }
      else if (!strncmp(token, "/", 1))
      {  mcxTing* tokenting = mcxTingNew(token+1)
      ;  zmTell('t', "pushing string [%s]", token)
      ;  return zgPush(UTYPE_STR, tokenting)
   ;  }
      else if (isdigit(*(token+0)) || *(token+0) == '-')
      {  if (strchr(token, '.'))
         {  double d = atof(token)
         ;  zmTell('t', "pushing double %f", d)
         ;  return zgPush(UTYPE_DBL, &d)
      ;  }
         else
         {  int i = atoi(token)
         ;  zmTell('t', "pushing integer %d", i)
         ;  return zgPush(UTYPE_INT, &i)
      ;  }
      }
      else
      {  mcxTing* tokenting   =  mcxTingNew(token)
      ;  opFunc   func        =  opGetOpByToken(tokenting)
      ;  mcxTingFree(&tokenting)
      ;  if (func)
         return (func())
      ;  else
         {  zmTell
            (  'e'
            ,  "invalid token [%s] (string, number, handle, nor keyword)"
            , token
            )
         ;  return 0
      ;  }
      }
      return 1
;  }


zgglob_p zgNewHandle
(  int         glype
,  void*       object
)
   {  zgglob_p   new    =  mcxAlloc(sizeof(zgglob_t), EXIT_ON_FAIL)
   ;  new->mx           =  NULL
   ;  new->ting         =  (mcxTing*) object
   ;  new->glype        =  glype << 1
   ;  new->i            =  0
   ;  new->d            =  0.0

   ;  return new
;  }


zgglob_p zgNew
(  int         utype
,  void*       object
)
   {  zgglob_p   new    =  mcxAlloc(sizeof(zgglob_t), EXIT_ON_FAIL)
   ;
      if (utype < 0 || utype >= N_UTYPE)
      {  zmTell('e', "(internal) zgNew unsupported utype <%d>",  utype)
      ;  return NULL
   ;  }

      new->glype        =  mapUtype[utype]
   ;  new->mx           =  NULL
   ;  new->ting         =  NULL
   ;  new->i            =  0
   ;  new->d            =  0.0
   ;
      switch(new->glype)
      {
         case glob__mx
      :  new->mx   = (mclMatrix*) object
      ;  break
      ;
         case glob__str
      :  new->ting = (mcxTing*) object
      ;  break
      ;
         case glob__int
      :  new->i    = ((int*) object)[0]
      ;  break
      ;
         case glob__dbl
      :  new->d    = ((double*) object)[0]
      ;  break
      ;
         case glob__seq
      :  new->ting = (mcxTing*) object
      ;  break
      ;
         case glob__newhdl        /* should be a new handle, not linked */
      :  new->ting = (mcxTing*) object
      ;  break
   ;  }
      return new
;  }


zgglob_p zgCopyObject
(  zgglob_p glob
)
   {  int glype =  glob->glype

   ;  if (glype == GLYPE_NEWHDL)
      {  zmNotSupported1(TOKEN_COPY, zgt(glype))
      ;  return NULL
   ;  }
   ;  if (glype == GLYPE_MXHDL)
      {  mclMatrix* x =  zgGetOb(glob, UTYPE_MX)
      ;  mclMatrix* y
      ;  if (!x) return NULL
      ;  y = mclxCopy(x)
      ;  return zgNew(UTYPE_MX, y)
   ;  }
      else if (glype == GLYPE_STRHDL)
      {  mcxTing* x = zgGetOb(glob, UTYPE_STR)
      ;  mcxTing* y
      ;  if (!x) return NULL
      ;  y = mcxTingNew(x->str)
      ;  return zgNew(UTYPE_STR, y)
   ;  }
      else
      {  return zgDupObject(glob)
   ;  }
   ;  return NULL
;  }


zgglob_p zgDupObject
(  zgglob_p glob
)
   {  int glype =  zgGetGlype(glob)

   ;  if (glype == GLYPE_NEWHDL)
      {  zmNotSupported1(TOKEN_DUP, zgt(glype))
      ;  return NULL
   ;  }
      else if (glype & GLASS_HDL)
      {  mcxTing *x  =  zgGetZgMem(glob, GLASS_HDL)
      ;  mcxTing *y
      ;  if (!x) return NULL
      ;  y = mcxTingNew(x->str)
      ;  return zgNewHandle(glype >> 1, y)      /* hierverder. */
   ;  }
      else if (glype == GLYPE_MX)
      {  mclMatrix* x =  zgGetOb(glob, UTYPE_MX)
      ;  mclMatrix* y
      ;  if (!x) return NULL
      ;  y = mclxCopy(x)
      ;  return zgNew(UTYPE_MX, y)
   ;  }
      else if (glype == GLYPE_INT)
      {  int* x = zgGetOb(glob, UTYPE_INT)
      ;  if (!x) return NULL
      ;  return zgNew(UTYPE_INT, x)
   ;  }
      else if (glype == GLYPE_DBL)
      {  double* x = zgGetOb(glob, UTYPE_DBL)
      ;  if (!x) return NULL
      ;  return zgNew(UTYPE_DBL, x)
   ;  }
      else if (glype == GLYPE_STR)
      {  mcxTing* x = zgGetOb(glob, UTYPE_STR)
      ;  if (!x) return NULL
      ;  return zgNew(UTYPE_STR, x)
   ;  }
      else
      {  zgSupportError("zgDupObject", glob)
      ;  return NULL
   ;  }
      return NULL
;  }


int zgMDup
(  int n
)
   {  int N = n

   ;  while(n--)
      {
         zgglob_p glob  = zsGetGlob(N-1)
      ;  zgglob_p new = glob ? zgDupObject(glob) : NULL

      ;  if (new)
         zsPush(new)
      ;  else
         return 0
   ;  }

   ;  return 1
;  }


zgglob_p zgPow
(  zgglob_p o1
,  zgglob_p o2
)
   {  int glypex  = o1->glype
   ;  int glypey  = o2->glype

   ;  if (glypex & GLASS_NUM && glypey & GLASS_NUM)
      {  
         if (glypex & GLASS_DBL || glypey & GLASS_DBL)
         {  double *x, *y, z
         ;  x = zgGetOb(o1, UTYPE_DBL)
         ;  y = zgGetOb(o2, UTYPE_DBL)
         ;  if (!x || !y) return NULL
         ;  z  = pow(*x,*y)
         ;  return zgNew(UTYPE_DBL, &z)
      ;  }
         else
         {  int *x, *y, i, p
         ;  x = zgGetOb(o1, UTYPE_INT)
         ;  y = zgGetOb(o2, UTYPE_INT)
         ;  if (!x || !y) return NULL
         ;  p = *x, i = *y
         ;  while (--i)
            p *= *x
         ;  return zgNew(UTYPE_INT, &p)
      ;  }
   ;  }
      else if (glypex & GLASS_MX && glypey & GLASS_INT)
      {  mclMatrix *mx = zgGetOb(o1, UTYPE_MX)
      ;  mclMatrix *mxp
      ;  int *ip = zgGetOb(o2, UTYPE_INT)
      ;  int i = ip ? *ip : 0
      ;  if (!mx || !ip) return NULL
      ;  mxp = mclxCopy(mx)
      ;  while (--i)
         {  mclMatrix* mxt = mclxCompose(mxp, mx, 0, n_threads_g)
         ;  mclxFree(&mxp)
         ;  mxp = mxt
      ;  }
      ;  return zgNew(UTYPE_MX, mxp)
   ;  }
      else
      {  zmNotSupported2(TOKEN_POW, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
      return NULL
;  }


zgglob_p zgMul
(  zgglob_p o1
,  zgglob_p o2
)
   {  int glypex  = o1->glype
   ;  int glypey  = o2->glype

   ;  if (glypex & GLASS_NUM && glypey & GLASS_NUM)
      {  
         if (glypex & GLASS_DBL || glypey & GLASS_DBL)
         {  double *x, *y, z
         ;  x = zgGetOb(o1, UTYPE_DBL)
         ;  y = zgGetOb(o2, UTYPE_DBL)
         ;  if (!x || !y) return NULL
         ;  z  = *x * *y
         ;  return zgNew(UTYPE_DBL, &z)
      ;  }
         else
         {  int *x, *y, z
         ;  x = zgGetOb(o1, UTYPE_INT)
         ;  y = zgGetOb(o2, UTYPE_INT)
         ;  if (!x || !y) return NULL
         ;  z = *x * *y
         ;  return zgNew(UTYPE_INT, &z)
      ;  }
   ;  }
      else if (glypex & GLASS_MX && glypey & GLASS_MX)
      {  mclMatrix *x, *y, *z
      ;  x = zgGetOb(o1, UTYPE_MX)
      ;  y = zgGetOb(o2, UTYPE_MX)
      ;  if (!x || !y) return NULL
      ;  if (!mcldEquate(x->dom_cols, y->dom_rows, MCLD_EQT_EQUAL))
         zmTell
         (  'e'
         , "left col domain (dim %ld) and right row domain (dim %ld) do not match"
         ,  (long) N_COLS(x)
         ,  (long) N_ROWS(y)
         )
      ;  z = mclxCompose(x, y, 0, n_threads_g)
      ;  return zgNew(UTYPE_MX, z)
   ;  }
      else if (glypex & GLASS_MX && glypey & GLASS_NUM)
      {  mclMatrix *mx = zgGetOb(o1, UTYPE_MX)
      ;  double *dp = zgGetOb(o2, UTYPE_DBL)
      ;  if (!mx || !dp) return NULL
      ;  mclxUnary(mx, fltxMul, dp)
      ;  return NULL
   ;  }
      else if (glypex & GLASS_NUM && glypey & GLASS_MX)
      {  zmNotSupported2(TOKEN_MUL, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
      else
      {  zmNotSupported2(TOKEN_MUL, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
      return NULL
;  }


enum
{  BINARY_ADD = 0
,  BINARY_MIN
,  BINARY_MAX
}  ;


static zgglob_p zgBinary
(  zgglob_p o1
,  zgglob_p o2
,  mcxmode mode
)
   {  int glypex  = o1->glype
   ;  int glypey  = o2->glype
   ;  zgglob_p o3 = NULL
   ;  const char* tok = TOKEN_ADD

   ;  if (mode == BINARY_MAX)
      tok = TOKEN_MAX
   ;  else if (mode == BINARY_MIN)
      tok = TOKEN_MIN

   ;  if (glypex & GLASS_NUM && glypey & GLASS_NUM)
      {  if (glypex & GLASS_DBL || glypey & GLASS_DBL)
         {  double *x, *y, z
         ;  x = zgGetOb(o1, UTYPE_DBL)
         ;  y = zgGetOb(o2, UTYPE_DBL)
         ;  if (!x || !y) return NULL
         ;  switch(mode)
            {  case BINARY_ADD : z = *x + *y; break
            ;  case BINARY_MIN : z = MCX_MIN(*x , *y); break
            ;  case BINARY_MAX : z = MCX_MAX(*x , *y); break
         ;  }
            o3 = zgNew(UTYPE_DBL, &z)
      ;  }
         else
         {  int *x, *y, z
         ;  x = zgGetOb(o1, UTYPE_INT)
         ;  y = zgGetOb(o2, UTYPE_INT)
         ;  if (!x || !y) return NULL
         ;  switch(mode)
            {  case BINARY_ADD : z = *x + *y; break
            ;  case BINARY_MIN : z = MCX_MIN(*x , *y); break
            ;  case BINARY_MAX : z = MCX_MAX(*x , *y); break
         ;  }
         ;  o3 = zgNew(UTYPE_INT, &z)
      ;  }
      }
      else if (glypex & GLASS_STR && glypey & GLASS_STR)
      {  mcxTing *x, *y, *z
      ;  if (mode != BINARY_ADD)
         {  zmNotSupported2(tok, zgt(glypex), zgt(glypey))
         ;  return NULL
      ;  }
         x =  zgGetOb(o1, UTYPE_STR)
      ;  y =  zgGetOb(o2, UTYPE_STR)
      ;  if (!x || !y) return NULL
      ;  z = mcxTingNew(x->str)
      ;  mcxTingAppend(z, y->str)
      ;  o3 = zgNew(UTYPE_STR, z)
   ;  }
      else if (glypex & GLASS_MX && glypey & GLASS_MX)
      {  mclMatrix *x, *y, *z = NULL
      ;  x = zgGetOb(o1, UTYPE_MX)
      ;  y = zgGetOb(o2, UTYPE_MX)

      ;  if (!x || !y)
         return NULL

      ;  switch(mode)
         {  case BINARY_ADD : z = mclxAdd(x, y); break
         ;  case BINARY_MIN : z = mclxBinary(x, y, fltMin); break
         ;  case BINARY_MAX : z = mclxBinary(x, y, fltMax); break
      ;  }
         o3 = zgNew(UTYPE_MX, z)
   ;  }
      else
      {  zmNotSupported2(TOKEN_ADD, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
      return o3
;  }


zgglob_p zgAdd
(  zgglob_p o1
,  zgglob_p o2
)
   {  return zgBinary(o1, o2, BINARY_ADD)
;  }


zgglob_p zgMin
(  zgglob_p o1
,  zgglob_p o2
)
   {  return zgBinary(o1, o2, BINARY_MIN)
;  }


zgglob_p zgMax
(  zgglob_p o1
,  zgglob_p o2
)
   {  return zgBinary(o1, o2, BINARY_MAX)
;  }


zgglob_p zgLt
(  zgglob_p o1
,  zgglob_p o2
)
   {  int glypex = o1->glype
   ;  int glypey = o2->glype

   ;  if (glypex & GLASS_NUM && glypey & GLASS_NUM)
      {  double  *x =  zgGetOb(o1, UTYPE_DBL)
      ;  double  *y =  zgGetOb(o2, UTYPE_DBL)
      ;  if (!x || !y) return NULL
      ;  return zgNew(UTYPE_INT, *x < *y ? &one_g : &zero_g)
   ;  }
      else
      {  zmNotSupported2(TOKEN_LT, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
   ;  return NULL
;  }



zgglob_p zgEq
(  zgglob_p o1
,  zgglob_p o2
)
   {  int glypex = o1->glype
   ;  int glypey = o2->glype

   ;  if (glypex == GLYPE_INT && glypey == GLYPE_INT)
      {  int  *x =  zgGetOb(o1, UTYPE_INT)
      ;  int  *y =  zgGetOb(o2, UTYPE_INT)
      ;  if (!x || !y) return NULL
      ;  return zgNew(UTYPE_INT, *x == *y ? &one_g : &zero_g)
   ;  }
      else if (glypex & GLASS_STR && glypey & GLASS_STR)
      {  mcxTing *x =  zgGetOb(o1, UTYPE_STR)
      ;  mcxTing *y =  zgGetOb(o2, UTYPE_STR)
      ;  if (!x || !y) return NULL
      ;  return zgNew(UTYPE_INT, !strcmp(x->str, y->str) ? &one_g : &zero_g)
   ;  }
      else
      {  zmNotSupported2(TOKEN_EQ, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
      return NULL
;  }



zgglob_p zgLq
(  zgglob_p o1
,  zgglob_p o2
)
   {  int glypex = o1->glype
   ;  int glypey = o2->glype

   ;  if (glypex & GLASS_NUM && glypey & GLASS_NUM)
      {  double  *x =  zgGetOb(o1, UTYPE_DBL)
      ;  double  *y =  zgGetOb(o2, UTYPE_DBL)
      ;  if (!x || !y) return NULL
      ;  return zgNew(UTYPE_INT, *x <= *y ? &one_g : &zero_g)
   ;  }
      else
      {  zmNotSupported2(TOKEN_LQ, zgt(glypex), zgt(glypey))
      ;  return NULL
   ;  }
   ;  return NULL
;  }


int zgPushHandle
(  int         glype
,  void*       object
)
   {  zgglob_p   glob    =  zgNewHandle(glype, object)

   ;  if (!glob)
      return 0

   ;  zsPush(glob)
   ;  return 1
;  }


int zgPush
(  int         utype
,  void*       object
)
   {  zgglob_p   glob    =  zgNew(utype, object)

   ;  if (!glob)
      return 0

   ;  zsPush(glob)
   ;  return 1
;  }


void zgFree
(  zgglob_p*   globpp
)
   {  zgglob_p glob = *globpp
   ;  if (glob)
      {
         if (glob->glype == GLYPE_MX)
         {
            if (glob->mx)
            {  mclxFree(&(glob->mx))
         ;  }
      ;  }
         if (glob->ting)
         mcxTingFree(&(glob->ting))
   ;  }
      mcxFree(glob)
   ;  *globpp = NULL
;  }


               /* rip glob, which may then Rest In Peace, Requiescem In Pace */
zgglob_p zgRip
(  zgglob_p   glob
)
   {  zgglob_p   new  =  glob ? mcxAlloc(sizeof(zgglob_t), EXIT_ON_FAIL) : NULL

   ;  if (!new)
      return NULL

   ;  new->mx        =  glob->mx
   ;  new->ting      =  glob->ting
   ;  new->glype     =  glob->glype

   ;  glob->mx       =  NULL
   ;  glob->ting     =  NULL
   ;  glob->glype    =  GLYPE_NONE

   ;  new->i         =  glob->i
   ;  new->d         =  glob->d
   ;  return new
;  }


int zsGetType
(  int         depth
)
   {  zgglob_p   glob    =  zsGetGlob(depth)
   ;  return glob ? zgGetType(glob) : UTYPE_NONE
;  }


void* zsGetMem
(  int   depth
,  int   class
)
   {  zgglob_p glob = zsGetGlob(depth)
   ;  return glob ? zgGetZgMem(glob, class) : NULL
;  }

/* what is zsGetMem doing in glob, should that not be in stack?
 * same for zsgettype.
 * zgGetZgMem does not issue warning, zgGetGlob does indeed.
 * Who does, who does not?
*/

void* zgGetZgMem
(  zgglob_p glob
,  int     class
)
   {  if (!glob)
      return NULL

   ;  if (class == GLASS_HDL)
      {  if (glob->glype & GLASS_HDL)
         return glob->ting
   ;  }
   ;  return NULL
;  }


void* zsGetOb
(  int   depth
,  int   utype
)
   {  zgglob_p glob = zsGetGlob(depth)
   ;  return glob ? zgGetOb(glob, utype) : NULL
;  }


void* zgGetOb
(  zgglob_p glob
,  int     utype
)
   {  mcxTing* handle

   ;  if (!glob)
      return NULL

   ;  handle = glob->glype & GLASS_HDL ? glob->ting : NULL  

   ;  if (glob->glype != GLYPE_NEWHDL && glob->glype & GLASS_HDL)
      {
         zgglob_p obx = zgGetGlobByHandle(glob->ting)
      ;  if (!obx)
         {  zmTell
            (  'e'
            ,  "attempted <%s> dereference of STALE HANDLE [%s]"
            ,  zgGetTypeName(utype)
            ,  handle->str
            )
         ;  return NULL
      ;  }
         else if (!(obx->glype & GLASS_OB))
         {  zmTell
            (  'e'
            ,  "attempted <%s> dereference of handle [%s] to non-object <%s>"
            ,  zgGetTypeName(utype)
            ,  handle->str
            ,  zgGetTypeName(zgt(obx->glype))
            )
         ;  return NULL
      ;  }
         glob = obx
   ;  }

   ;  if (glob->glype != mapUtype[utype])
      {  if (glob->glype == GLYPE_INT && utype == UTYPE_DBL)
         {  glob->d = (double) glob->i
      ;  }
         else
         {  zmTell
            (  'e'
            ,  "attempted <%s> access of <%s> object"
            ,  zgGetTypeName(utype)
            ,  zgGetTypeName(zgt(glob->glype))
            )
         ;  return NULL
      ;  }
   ;  }

      if (utype == UTYPE_INT)
      return &(glob->i)
   ;  else if (utype == UTYPE_DBL)
      return &(glob->d)
   ;  else if (utype == UTYPE_MX)
      return glob->mx
   ;  else if (utype == UTYPE_STR)
      return glob->ting
   ;  else if (utype == UTYPE_NEWHDL)
      return glob->ting
   ;  else if (utype == UTYPE_SEQ)
      return glob->ting

   ;  zmTell('e', "[zgGetOb] this should not happen")
   ;  return NULL
;  }


const char* zgGetTypeName
(  int type
)
   {  if (type < 0 || type > N_UTYPE)
         fprintf(stderr, "(internal) type range error, deadly\n")
      ,  mcxExit(1)

   ;  return UNAME[type]
;  }


int zgt
(  int glype
)
   {  if (glype & GLASS_INT)
      return UTYPE_INT
   ;  else if (glype & GLASS_DBL)
      return UTYPE_DBL
   ;  else if (glype & GLASS_MX)
      return UTYPE_MX
   ;  else if (glype & GLASS_STR)
      return UTYPE_STR
   ;  else if (glype & GLASS_SEQ)
      return UTYPE_SEQ
   ;  else if (glype == GLYPE_NEWHDL)
      return UTYPE_NEWHDL
   ;  else
      return UTYPE_NONE
;  }


int zgGetType
(  zgglob_p   glob
)
   {  return zgt(glob->glype)
;  }


int zgGetGlype
(  zgglob_p   glob
)
   {  return glob->glype
;  }


void zgSupportError
(  const char* what
,  const zgglob_p glob
)
   {  if (glob)
      zmTell
      ('e', "<%s> [%s] not supported", zgGetTypeName(zgt(glob->glype)), what)
   ;  else
      zmTell('e', "stale handle passed to '%s'", what)
;  }


void zgAccessError
(  const char* what
,  const zgglob_p glob
)
   {  if (glob)
      zmTell
      ('e', "invalid <%s> acces of <%s>", what, zgGetTypeName(zgt(glob->glype)))
   ;  else
      zmTell('e', "stale handle referenced as %s", what)
;  }


zgglob_p zgGetGlobByHandle
(  mcxTing* ting
)
   {  mcxKV* kv = mcxHashSearch(ting, hdltable_g, MCX_DATUM_FIND)
   ;  return kv ? (zgglob_p) kv->val : NULL
;  }


int zgUnlink
(  void
)
   {  mcxTing  *handle  =  zsGetMem(0, GLASS_HDL)
   ;  mcxKV*   kv

   ;  if (!handle) return 0

   ;  if ((kv = mcxHashSearch(handle, hdltable_g, MCX_DATUM_DELETE)))
      {
         zgglob_p glob = (zgglob_p) kv->val
      ;  mcxTing *hashhandle = (mcxTing*) kv->key

      ;  zmTell
         (  'd'
         ,  "[%s] removing handle [%s], pushing object glyped <%d>"
         ,  TOKEN_UNLINK
         ,  hashhandle->str
         ,  glob->glype
         )

      ;  zsPop()
      ;  zsPush(glob)
      ;  mcxTingFree(&hashhandle)
   ;  }
      else
      {  zmTell('e', "handle [%s] is not bound", handle->str)
      ;  return 0
   ;  }
      return 1
;  }


/* fixme can use mcxhashkeys now */

int zgVars
(  void
)
   {  mcxHashWalk* hw = mcxHashWalkInit(hdltable_g)
   ;  mcxKV* kv

   ;  while ((kv = mcxHashWalkStep(hw, NULL)))
      {  zgglob_p  glob = (zgglob_p) kv->val
      ;  mcxTing* hdl  = (mcxTing*) kv->key
      ;  const char* what
      ;  switch(glob->glype)
         {  case glob__mx
         :  what = "matrix"
         ;  break

         ;  case glob__seq
         :  what = "sequence"
         ;  break

         ;  case glob__int
         :  what = "integer"
         ;  break

         ;  case glob__dbl
         :  what = "double"
         ;  break

         ;  case glob__str
         :  what = "string"
         ;  break

         ;  default
         :  zmTell
            (  'e'
            ,  "[" TOKEN_VARS "] " "(internal) unexpected object glype <%ld>\n"
            ,  (long) glob->glype
            )
         ;  what = "???"
      ;  }
      ;  zmTell('m', "%8s  ==  handle to %s", hdl->str, what)
   ;  }

      mcxHashWalkFree(&hw)
   ;  return 1
;  }


int zgDef
(  void
)
   {  zgglob_p obh     =  zsGetGlob(0)
   ;  zgglob_p obx     =  zsGetGlob(1)
   ;  mcxTing   *handle  =  obh ? zgGetZgMem(obh, GLASS_HDL) : NULL

   ;  mcxTing   *hashhandle
   ;  zgglob_p oby
   ;  mcxKV     *kv

   ;  if (!obh || !obx)
      return 0

   ;  if (!handle)
      {  zmNotSupported2(TOKEN_DEF, zgGetType(obx), zgGetType(obh))
      ;  return 0
   ;  }
   
   ;  if (!(obx->glype << 1 & GLASS_HDL))
      {  zmTell('e', "object glyped <%d> cannot be handled", obx->glype)
      ;  return 0
   ;  }

      hashhandle  =  mcxTingNew(handle->str)
   ;  kv          =  mcxHashSearch(hashhandle, hdltable_g, MCX_DATUM_INSERT)

   ;  if (kv->key != hashhandle)
      {  zgglob_p obz = (zgglob_p) kv->val

      ;  zmTell('d', "freeing object in handle [%s]", hashhandle->str)
      ;  mcxTingFree(&hashhandle)
      ;  zgFree(&obz)
   ;  }

      oby         =  zgRip(obx)           /* now obx is no longer meaningful */
   ;  obh->glype  =  (oby->glype) << 1    /* one big fat ugly hack */
   ;  kv->val     =  oby

   ;  zmTell
      ('d', "bound handle [%s] to object glyped <%d>", handle->str, oby->glype)
   ;  return (zsPop() && zsPop())
;  }


void zgInfo
(  zgglob_p glob
)
   {  zgglob_p obx = NULL

   ;  if (glob->glype & GLASS_HDL && glob->glype != GLYPE_NEWHDL)
      {  obx = zgGetGlobByHandle(glob->ting)
      ;  if (!obx)
         {  zmTell('v', "STALE HANDLE [%s]", glob->ting->str)
         ;  return
      ;  }
      ;  if ((glob->glype >> 1) != (obx->glype))
         {  zmTell('v', "HAYWIRE HANDLE [%s]", glob->ting->str)
         ;  return
      ;  }
      }

      switch(glob->glype)
      {  case glob__int
      :  zmTell('v', "int %d", glob->i)
      ;  break

      ;  case glob__inthdl
      :  zmTell('v', "[%s] int %d", glob->ting->str, obx->i)
      ;  break

      ;  case glob__dbl
      :  zmTell('v', "dbl %f", glob->d)
      ;  break

      ;  case glob__dblhdl
      :  zmTell('v', "[%s] dbl %f", glob->ting->str, obx->d)
      ;  break

      ;  case glob__mx
      :  zmTell('v', "mtx %ld x %ld", (long) N_ROWS(glob->mx), (long) N_COLS(glob->mx))
      ;  break

      ;  case glob__mxhdl
      :  zmTell
         (  'v'
         ,  "[%s] mtx %ld x %ld"
         ,  glob->ting->str
         ,  (long) N_ROWS(obx->mx)
         ,  (long) N_COLS(obx->mx)
         )
      ;  break

      ;  case glob__str
      :  zmTell('v', "str %s", glob->ting->str)
      ;  break

      ;  case glob__strhdl
      :  zmTell('v', "[%s] str %s", glob->ting->str, obx->ting->str)
      ;  break

      ;  case glob__seq
      :  zmTell('v', "seq {%s }", glob->ting->str)
      ;  break

      ;  case glob__seqhdl
      :  zmTell('v', "[%s] seq {%s }", glob->ting->str, obx->ting->str)
      ;  break

      ;  case glob__newhdl
      :  zmTell('v', "[%s] fresh handle", glob->ting->str)
      ;  break

      ;  default
      :  break
   ;  }
   }

