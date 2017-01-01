/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ivp.h"
#include "pval.h"

#include "util/err.h"
#include "util/alloc.h"
#include "util/array.h"
#include "util/types.h"
#include "util/minmax.h"


mclIvp* mclpInstantiate
(  mclIvp*   ivp
,  long      index
,  double    value
)
   {  if (!ivp)
      ivp = mcxAlloc(sizeof(mclIvp), EXIT_ON_FAIL)

   ;  ivp->idx       =  index
   ;  ivp->val       =  value
   
   ;  return ivp
;  }


mclpAR* mclpARinit
(  mclpAR* mclpar
)
   {  if (!mclpar)
      mclpar =  mcxAlloc(sizeof(mclpAR), EXIT_ON_FAIL)

   ;  if (!mclpar)
      return NULL

   ;  mclpar->ivps      =  NULL
   ;  mclpar->n_ivps    =  0
   ;  mclpar->n_alloc   =  0
   ;  mclpar->sorted    =  MCLPAR_SORTED | MCLPAR_UNIQUE

   ;  return mclpar
;  }


void mclpARreset
(  mclpAR* mclpar
)
   {  mclpar->n_ivps    =  0
   ;  mclpar->sorted    =  MCLPAR_SORTED | MCLPAR_UNIQUE
;  }


void* mclpARinit_v
(  void* mclpar
)
   {  return mclpARinit(mclpar)
;  }


mclpAR* mclpARensure
(  mclpAR* mclpar
,  dim     sz
)
   {  if (!mclpar && !(mclpar =  mclpARinit(NULL)))
      return NULL

   ;  if (sz > mclpar->n_alloc)
      {  if (!( mclpar->ivps
            =  mcxNRealloc
               (  mclpar->ivps
               ,  sz
               ,  mclpar->n_alloc
               ,  sizeof(mclp)
               ,  mclpInit_v
               ,  RETURN_ON_FAIL
               )
            ) )
         return NULL
      ;  mclpar->n_alloc = sz
   ;  }

      return mclpar
;  }


mclpAR* mclpARfromIvps
(  mclpAR*  mclpar
,  mclp*    ivps
,  dim      sz
)
   {  mclpar = mclpARensure(mclpar, sz)
   ;  if (!mclpar)
      return NULL
   ;  if (sz)
      memcpy(mclpar->ivps, ivps, sz * sizeof(mclIvp))
   ;  mclpar->n_ivps = sz
   ;  return mclpar
;  }


void mclpARfree
(  mclpAR**    mclparp
)  
   {  if (*mclparp)
      {  mcxFree(mclparp[0]->ivps)
      ;  mcxFree(*mclparp)   
      ;  *mclparp   =  NULL
   ;  }
   }


mcxbool mclpARbatchCheck
(  mclpAR* ar
,  long range_lo
,  long range_hi
)
   {  long prev = LONG_MIN
   ;  ar->sorted = MCLPAR_SORTED | MCLPAR_UNIQUE
   ;  dim i

   ;  for (i=0; i<ar->n_ivps;i++)
      {  long num = ar->ivps[i].idx
      ;  if (num < range_lo || num >= range_hi)
         break
      ;  if (num < prev)
         BIT_OFF(ar->sorted, MCLPAR_SORTED | MCLPAR_UNIQUE)
      ;  else if (num == prev)
         BIT_OFF(ar->sorted, MCLPAR_UNIQUE)
      ;  prev = num
   ;  }
      if (i != ar->n_ivps)
      {  mcxErr("load", "ivp error { %ld %g }", (long) ar->ivps[i].idx, (double) ar->ivps[i].val)
      ;  return FALSE
   ;  }
      return TRUE
;  }


mcxstatus mclpARextend
(  mclpAR*  ar
,  long     idx
,  double   val
)
   {  mclp* ivp = NULL
   ;  if (ar->n_ivps >= ar->n_alloc)
      {  long n_new_alloc = 4 + 1.22 * ar->n_alloc
      ;  if
         (! (  ar->ivps
            =  mcxNRealloc
               (  ar->ivps
               ,  n_new_alloc
               ,  ar->n_alloc
               ,  sizeof(mclp)
               ,  mclpInit_v
               ,  RETURN_ON_FAIL
         )  )  )
         return STATUS_FAIL
      ;  ar->n_alloc = n_new_alloc
   ;  }

      ivp = ar->ivps + ar->n_ivps
   ;  ivp->val =  val
   ;  ivp->idx =  idx

   ;  if (ar->n_ivps && ivp[-1].idx >= idx)
      {  if (ivp[-1].idx > idx)
         BIT_OFF(ar->sorted, MCLPAR_SORTED | MCLPAR_UNIQUE)
      ;  else
         BIT_OFF(ar->sorted, MCLPAR_UNIQUE)
   ;  }

      ar->n_ivps++
   ;  return STATUS_OK
;  }


mcxbool mclpGivenValGQ
(  mclIvp*        ivp
,  void*          arg
)
   {  double* f = (double*) arg
   ;  if (ivp->val >= *f)
      return TRUE
   ;  return FALSE
;  }


mcxbool mclpGivenValLQ
(  mclIvp*        ivp
,  void*          arg
)
   {  double* f = (double*) arg
   ;  if (ivp->val <= *f)
      return TRUE
   ;  return FALSE
;  }


int mclpIdxGeq
(  const void*             i1
,  const void*             i2
)
   {  return ((mclIvp*)i1)->idx >= ((mclIvp*)i2)->idx
;  }


int mclpIdxCmp
(  const void*             i1
,  const void*             i2
)
   {  long d = ((mclIvp*)i1)->idx - ((mclIvp*)i2)->idx
   ;  return d < 0 ? -1 : d > 0 ? 1 : 0
;  }


int mclpIdxRevCmp
(  const void*             i1
,  const void*             i2
)
   {  long d = ((mclIvp*)i2)->idx - ((mclIvp*)i1)->idx
   ;  return d < 0 ? -1 : d > 0 ? 1 : 0
;  }


int mclpValCmp
(  const void*             i1
,  const void*             i2
)
   {  int     s  =  MCX_SIGN(((mclIvp*)i1)->val - ((mclIvp*)i2)->val)
   ;  return (s ? s : ((mclIvp*)i1)->idx - ((mclIvp*)i2)->idx)
;  }


int mclpValRevCmp
(  const void*             i1
,  const void*             i2
)
   {  int     s  =  MCX_SIGN(((mclIvp*)i2)->val - ((mclIvp*)i1)->val)
   ;  return (s ? s : ((mclIvp*)i1)->idx - ((mclIvp*)i2)->idx)
;  }


   /* a noop apparently works, but this needs more documentation (caller's context) */
void mclpMergeLeft
(  void*                   ivp_unused1
,  const void*             ivp_unused2
)
   {
;  }


void mclpMergeRight
(  void*                   i1
,  const void*             i2
)
   {  ((mclIvp*)i1)->val = ((mclIvp*)i2)->val
;  }


void mclpMergeAdd
(  void*                   i1
,  const void*             i2
)
   {  ((mclIvp*)i1)->val += ((mclIvp*)i2)->val
;  }


void mclpMergeMin
(  void*                   i1
,  const void*             i2
)
   {  ((mclIvp*)i1)->val = MCX_MIN(((mclIvp*)i1)->val,((mclIvp*)i2)->val)
;  }


void mclpMergeMax
(  void*                   i1
,  const void*             i2
)
   {  ((mclIvp*)i1)->val = MCX_MAX(((mclIvp*)i1)->val,((mclIvp*)i2)->val)
;  }


void mclpMergeMul
(  void*                   i1
,  const void*             i2
)
   {  ((mclIvp*)i1)->val *= ((mclIvp*)i2)->val
;  }


mclIvp* mclpInit
(  mclIvp*                 ivp
)  
   {  return mclpInstantiate(ivp, -1, 1.0)
;  }


void* mclpInit_v
(  void*                  ivp
)  
   {  return mclpInstantiate(ivp, -1, 1.0)
;  }


mclIvp* mclpCreate
(  long   idx
,  double   value
)  
   {  return mclpInstantiate(NULL, idx, value)
;  }


double mclpGetDouble
(  const void* ivp
)
   {  return ((mclp*) ivp)->val
;  }


void mclpFree
(  mclIvp**                   p_ivp
)  
   {  if (*p_ivp)
      {  mcxFree(*p_ivp)
      ;  *p_ivp   =  NULL
   ;  }
   }


#if 0
enum
{  MCLX_UNARY_LT  =  0
,  MCLX_UNARY_LQ
,  MCLX_UNARY_GQ
,  MCLX_UNARY_GT
,  MCLX_UNARY_RAND
,  MCLX_UNARY_MUL
,  MCLX_UNARY_SCALE
,  MCLX_UNARY_ADD 
,  MCLX_UNARY_CEIL
,  MCLX_UNARY_FLOOR 
,  MCLX_UNARY_ACOS
,  MCLX_UNARY_POW
,  MCLX_UNARY_EXP
,  MCLX_UNARY_LOG
,  MCLX_UNARY_NEGLOG
,  MCLX_UNARY_ABS
,  MCLX_UNARY_COPY
,  MCLX_UNARY_UNUSED
}  ;
#endif


double (*mclp_unary_tab[])(pval, void*)
=
{  fltxLT
,  fltxLQ
,  fltxGQ
,  fltxGT
,  fltxRand
,  fltxMul
,  fltxScale
,  fltxAdd 
,  fltxCeil
,  fltxFloor 
,  fltxAcos
,  fltxPower
,  fltxExp
,  fltxLog
,  fltxNeglog
,  fltxAbs
,  fltxCopy
,  NULL           /* (double (f*)(pval* flt, void*arg)) NULL */
}  ;


double mclpUnary
(  mclp*    ivp
,  mclpAR*  ar       /* idx: MCLX_UNARY_mode, val: arg */
)
   {  dim i
   ;  double val = ivp->val
   ;  for (i=0;i<ar->n_ivps;i++)
      {  int mode = ar->ivps[i].idx
      ;  double arg = ar->ivps[i].val
      ;  if (mode == MCLX_UNARY_UNUSED)   /* sentinel used by callers to implement #knn(5) etc */
         continue
      ;  else if (mode < 0 || mode > MCLX_UNARY_UNUSED)
         {  mcxErr("mclpUnary", "not a mode: %d", mode)
         ;  break
      ;  }
         val = mclp_unary_tab[mode](val, &arg)
      ;  if (!val)
         switch(mode)            /* edges have disappeared, do not process further */
         {  case MCLX_UNARY_LT
         :  case MCLX_UNARY_LQ
         :  case MCLX_UNARY_GQ
         :  case MCLX_UNARY_GT
         :  return val
      ;  }
      }
      return val
;  }



mcxbool mclpSelectIdcs
(  mclp     *ivp
,  void     *range
)
   {  mclpIRange* rng = range
   ;  long idx = ivp->idx
   ;  long* lft = rng->lft
   ;  long* rgt = rng->rgt
   ;  if
      (  (  rgt
         && 
            (  idx > rgt[0]
            || (rng->equate & MCLX_EQT_LT && idx >= rgt[0])
            )
         )
      || (  lft
         && 
            (  idx < lft[0]
            || (rng->equate & MCLX_EQT_GT && idx <= lft[0])
            )
         )
      )
      return FALSE
   ;  return TRUE
;  }



mcxbool mclpSelectValues
(  mclp     *ivp
,  void     *range
)
   {  mclpVRange* rng = range
   ;  double val = ivp->val
   ;  double* lft = rng->lft
   ;  double* rgt = rng->rgt
   ;  if
      (  (  rgt
         && 
            (  val > rgt[0]
            || ((rng->equate & MCLX_EQT_LT) && val >= rgt[0])
            )
         )
      || (  lft
         && 
            (  val < lft[0]
            || ((rng->equate & MCLX_EQT_GT) && val <= lft[0])
            )
         )
      )
      return FALSE
   ;  return TRUE
;  }


int mcleCmp(const void* a, const void* b)
   {  const mcle* e1 = a, *e2 = b
   ;  int d = e1->src < e2->src ? -1 : e1->src > e2->src ? 1 : 0
   ;  return d ? d : e1->dst < e2->dst ? -1 : e1->dst > e2->dst ? 1 : 0
;  }

int mcleSrcCmp(const void* a, const void* b)
   {  const mcle* e1 = a, *e2 = b
   ;  return e1->src < e2->src ? -1 : e1->src > e2->src ? 1 : 0
;  }

int mcleDstCmp(const void* a, const void* b)
   {  const mcle* e1 = a, *e2 = b
   ;  return e1->dst < e2->dst ? -1 : e1->dst > e2->dst ? 1 : 0
;  }


