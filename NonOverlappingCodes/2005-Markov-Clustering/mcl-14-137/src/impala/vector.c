/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "vector.h"
#include "ivp.h"
#include "iface.h"
#include "pval.h"
#include "io.h"

#include "util/compile.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/array.h"
#include "util/minmax.h"
#include "util/err.h"


/* return smallest ceil to key; can be identical.
 *    !retval || key <= retval
*/

static inline mclp* mclpBsearchCeil
(  const mclp *key
,  const mclp *base
,  dim nmemb
)
   {  dim lft = -1
   ;  dim rgt = nmemb
   ;  dim bar = nmemb / 2

   ;  if (!nmemb || key->idx > base[nmemb-1].idx)
      return NULL

   ;  if (key->idx <= base[0].idx)
      return (mclp*) base

            /* invariant: lft points to a member that is smaller than pivot
             * or it points before the first member.
            */
   ;  while (lft+1 < rgt)
      {  if (key->idx > base[bar].idx)
         lft = bar
      ;  else
         rgt = bar
      ;  bar = rgt - (rgt-lft) / 2;       /* works if lft == -1u */
   ;  }
      return (mclp*) base + bar
;  }



/* return largest floor to key; can be identical
 *    !retval || key >= retval
*/

static inline mclp* mclpBsearchFloor
(  const mclp *key
,  const mclp *base
,  dim nmemb
)
   {  dim lft = 0
   ;  dim rgt = nmemb
   ;  dim bar = nmemb / 2

   ;  if (!nmemb || key->idx < base->idx)
      return NULL

            /* invariant: rgt points to a
             * a member that is larger than pivot or it
             * points beyond the last member.
            */
   ;  while (lft+1 < rgt)
      {  if (key->idx < base[bar].idx)
         rgt = bar
      ;  else
         lft = bar
      ;  bar = lft + (rgt-lft) / 2;
   ;  }
      return (mclp*) base + bar
;  }



                     /* fixme; mclpInit uses 1.0, this uses 0.0.
                      * 0.0 might actually be preferable
                     */
void* mclvInit_v
(  void* vecv
)  
   {  mclv *vec = vecv
   ;  if (!vec && !(vec = mcxAlloc(sizeof(mclVector), ENQUIRE_ON_FAIL)))
      return NULL
   ;  vec->ivps   =  NULL
   ;  vec->n_ivps =   0
   ;  vec->vid    =  -1
   ;  vec->val    =   0.0
   ;  return vec
;  }


mclVector* mclvInit
(  mclVector*              vec
)  
   {  return mclvInit_v(vec)
;  }


/*
 * This routine should always work also for non-conforming vectors
*/

mclVector* mclvInstantiate
(  mclVector*     dst_vec
,  dim            new_n_ivps
,  const mclIvp*  src_ivps
)
   {  mclIvp*     new_ivps
   ;  dim         old_n_ivps

   ;  if (!dst_vec && !(dst_vec = mclvInit(NULL)))    /* create */
      return NULL

   ;  old_n_ivps = dst_vec->n_ivps

                                    /* I've had a suspicion that some reallocs might be too lazy
                                     * to reuse shrunk array space.
                                    */
   ;  if (DIM_MAX / sizeof new_ivps[0] < new_n_ivps)
   /*  DO NOTHING, enter mcxMemDenied below */
   ;  else if (old_n_ivps / 2 > new_n_ivps)
      {  new_ivps = mcxAlloc(new_n_ivps * sizeof new_ivps[0], ENQUIRE_ON_FAIL)
      ;  if (new_ivps && !src_ivps)
         memcpy(new_ivps, dst_vec->ivps, new_n_ivps * sizeof new_ivps[0])
      ;  mcxFree(dst_vec->ivps)
      ;  dst_vec->ivps = new_ivps
   ;  }
      else
      dst_vec->ivps =  mcxRealloc(dst_vec->ivps, new_n_ivps * sizeof new_ivps[0], ENQUIRE_ON_FAIL)

   ;  if 
      (  !dst_vec->ivps
      && new_n_ivps
      )
      {  mcxMemDenied(stderr, "mclvInstantiate", "mclIvp", new_n_ivps)
      ;  return NULL
   ;  }
                                      /*  ^ do not free; *dst_vec could be array element */

      new_ivps = dst_vec->ivps

   ;  if (!src_ivps)                                  /* resize */
      {  dim k = old_n_ivps
      ;  while (k < new_n_ivps)
         {  mclpInit(new_ivps + k)
         ;  k++
      ;  }
      }
      else if (src_ivps && new_n_ivps)                /* copy   */
      memcpy(new_ivps, src_ivps, new_n_ivps * sizeof(mclIvp))

   ;  dst_vec->n_ivps = new_n_ivps
   ;  return dst_vec
;  }


mclVector* mclvNew
(  mclp*    ivps
,  dim      n_ivps
)
   {  return mclvInstantiate(NULL, n_ivps, ivps)
;  }


mclVector* mclvClone
(  const mclVector*  src
)
   {  return mclvCopy(NULL, src)
;  }


mclVector* mclvCopy
(  mclVector*        dst
,  const mclVector*  src
)  
   {  if (!src)
      {  mclvFree(&dst)
      ;  return NULL
   ;  }
      return mclvInstantiate(dst, src->n_ivps, src->ivps)
;  }


void mclvRelease
(  mclVector*        vec
)
   {  if (!vec)
      return
   ;  mcxFree(vec->ivps)
   ;  vec->ivps = NULL
   ;  vec->n_ivps = 0
;  }


void mclvRelease_v
(  void*        vec
)
   {  mclvRelease(vec)
;  }


void mclvFree
(  mclVector**                 vecpp
)  
   {  if (*vecpp)
      {  mcxFree((*vecpp)->ivps)
      ;  mcxFree(*vecpp)
      ;  (*vecpp) = NULL
   ;  }
;  }


void mclvFree_v
(  void*  vecpp
)  
   {  mclvFree(vecpp)
;  }


mclVector* mclvRenew
(  mclv*    dst
,  mclp*    ivps
,  dim      n_ivps
)
   {  return mclvInstantiate(dst, n_ivps, ivps)
;  }


mclVector*  mclvResize
(  mclVector*              vec
,  dim                     n_ivps
)  
   {  return mclvInstantiate(vec, n_ivps, NULL)
;  }


void mclvRuntime
(  const mclVector* vec
,  const char* caller
)
   {  if (!vec)
         mcxErr(caller, "void vector argument")
      ,  mcxExit(1)
;  }


mcxstatus mclvCheck
(  const mclVector*        vec
,  long                    min
,  long                    max
,  mcxbits                 bits
,  mcxOnFail               ON_FAIL
)  
   {  mclIvp*  ivp      =  vec->ivps
   ;  mclIvp*  ivpmax   =  vec->ivps+vec->n_ivps
   ;  long     last     =  -1
   ;  const char* me    =  "mclvCheck"
   ;  mcxbool  ok       =  TRUE

   ;  if (vec->n_ivps && !vec->ivps)
         mcxErr(me, "deadly: NULL ivps and %ld n_ivps", (long) vec->n_ivps)
      ,  ok = FALSE
   ;  else if (vec->n_ivps && min >= 0 && MCLV_MINID(vec) < min)
         mcxErr
         (me, "daemons: MINID %ld less than %ld", (long) MCLV_MINID(vec), min)
      ,  ok = FALSE

   ;  while (ok && ivp<ivpmax)
      {  if (ivp->idx <= last)
         {  mcxErr(me, "deadly: index %s <%ld, %ld> at ivp <%ld>"
            ,  ivp->idx == last ? "repeat" : "descent"
            ,  (long) last
            ,  (long) ivp->idx
            ,  (long) (ivp - vec->ivps)
            )
         ;  ok = FALSE
         ;  break
      ;  }

         if
         (  (bits & MCLV_CHECK_POSITIVE && ivp->val < 0.0)
         || (bits & MCLV_CHECK_NONZERO && ivp->val == 0.0)
         )
         {  mcxErr(me, "error: value <%f> at ivp <%ld>"
            ,  (double) ivp->val
            ,  (long) (ivp - vec->ivps)
            )
         ;  ok = FALSE
         ;  break
      ;  }

         last = ivp->idx
      ;  ivp++
   ;  }

      if (ok && max >= 0 && last > max)
      {  mcxErr
         (  me
         ,  "deadly: index <%ld> tops range <%ld> at ivp <%ld>"
         ,  (long) last
         ,  (long) max
         ,  (long) (ivp - 1 - vec->ivps)
         )
      ;  ok    =  FALSE
   ;  }

      if (!ok && (ON_FAIL == EXIT_ON_FAIL))
      mcxExit(1)

   ;  return ok ? STATUS_OK : STATUS_FAIL
;  }


double mclvIn
(  const mclVector*        lft
,  const mclVector*        rgt
)
   {  double ip = 0.0
   ;  mclp 
         *ivp1    = lft->ivps
      ,  *ivp2    = rgt->ivps
      ,  *ivp1max = ivp1 + lft->n_ivps
      ,  *ivp2max = ivp2 +rgt->n_ivps

   ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
      {  if (ivp1->idx < ivp2->idx)
         ivp1++
      ;  else if (ivp1->idx > ivp2->idx)
         ivp2++
      ;  else
         ip += (ivp1++)->val * (ivp2++)->val
   ;  }
      return ip
;  }


void mclvSortUniq
(  mclVector*  vec
)
   {  mclvSort(vec, mclpIdxCmp)
   ;  mclvUniqIdx(vec, mclpMergeLeft)
;  }


void mclvSortDescVal
(  mclVector*              vec
)
   {  mclvSort(vec, mclpValRevCmp)
;  }


void mclvSortAscVal
(  mclVector*              vec
)
   {  mclvSort(vec, mclpValCmp)
;  }


void mclvSort
(  mclVector*   vec
,  int         (*cmp)(const void*, const void*)
)  
   {  if (!cmp)
      cmp = mclpIdxCmp

   ;  if (vec->n_ivps)
      qsort(vec->ivps, vec->n_ivps, sizeof(mclIvp), cmp)
;  }


dim mclvUniqIdx
(  mclVector*              vec
,  void (*merge)(void* ivp1, const void* ivp2)
)  
   {  dim n = 0, diff = 0

   ;  if (vec->n_ivps)
      n = mcxDedup
      (  vec->ivps
      ,  vec->n_ivps
      ,  sizeof(mclIvp)
      ,  mclpIdxCmp
      ,  merge
      )
   ;  diff = vec->n_ivps - n
   ;  vec->n_ivps = n                  /* noteme; one could realloc */
   ;  return diff
;  }


double mclvKBar
(  mclVector   *vec
,  dim         k
,  double      ignore            /*    ignore elements relative to this  */
,  int         mode
)  
   {  int      have_even   =  (k+1) % 2
   ;  dim      n_inserted  =  0
   ;  double   ans         =  0.0
   ;  mclIvp * vecivp      =  vec->ivps
   ;  mclIvp*  vecmaxivp   =  vecivp + vec->n_ivps
   ;  pval *   heap
                                 /* can select everything */
   ;  if (k >= vec->n_ivps)
      return mode == KBAR_SELECT_LARGE ? -FLT_MAX : FLT_MAX

                                 /* let's select nothing, it might even help */
   ;  if (!(heap =  mcxAlloc ((k+have_even)*sizeof(pval), RETURN_ON_FAIL)))
      return mode == KBAR_SELECT_LARGE ? FLT_MAX : -FLT_MAX

   ;  if (mode == KBAR_SELECT_LARGE)
      {  if (have_even)
         *(heap+k) =  PVAL_MAX

      ;  while(vecivp < vecmaxivp)
         {  pval val =  vecivp->val

         ;  if (val >= ignore)
            NOTHING
         ;  else if (n_inserted < k)
            {  dim d =  n_inserted

            ;  while (d != 0 && *(heap+(d-1)/2) > val)
               { *(heap+d) =  *(heap+(d-1)/2)
               ;  d = (d-1)/2
            ;  }
               *(heap+d) =  val
            ;  n_inserted++
         ;  }
            else if (val > *heap)
            {  dim root  =  0
            ;  dim d

            ;  while((d = 2*root+1) < k)
               {  if (*(heap+d) > *(heap+d+1))
                  d++
               ;  if (val > *(heap+d))
                  {  *(heap+root) = *(heap+d)
                  ;  root = d
               ;  }
                  else break
            ;  }
               *(heap+root) = val
         ;  }
            vecivp++
      ;  }
      }

      else if (mode == KBAR_SELECT_SMALL)
      {  if (have_even)
         *(heap+k) = -PVAL_MAX

      ;  while(vecivp < vecmaxivp)
         {  pval val = vecivp->val

         ;  if (val < ignore)
            NOTHING
         ;  else if (n_inserted < k)
            {  dim d = n_inserted

            ;  while (d != 0 && *(heap+(d-1)/2) < val)
               { *(heap+d) = *(heap+(d-1)/2)
               ;  d = (d-1)/2
            ;  }
               *(heap+d) = val
            ;  n_inserted++
         ;  }
            else if (val < *heap)
            {  dim root = 0
            ;  dim d

            ;  while((d = 2*root+1) < k)
               {  if (*(heap+d) < *(heap+d+1))
                  d++
               ;  if (val < *(heap+d))
                  {  *(heap+root) = *(heap+d)
                  ;  root = d
               ;  }
                  else break
            ;  }
               *(heap+root) = val
         ;  }
            vecivp++
      ;  }
      }
      else
      {  mcxErr("mclvKBar PBD", "invalid mode")
      ;  mcxExit(1)
   ;  }

      ans = *heap
   ;  mcxFree(heap)
   ;  return ans
;  }


double mclvSelectGqBar
(  mclVector* vec
,  double     fbar
)
   {  mclIvp *writeivp, *readivp, *maxivp
   ;  double mass = 0.0

   ;  writeivp =  vec->ivps
   ;  readivp  =  vec->ivps
   ;  maxivp   =  vec->ivps+vec->n_ivps

   ;  while (readivp < maxivp)
      {  if (readivp->val >= fbar)
         {  mass += readivp->val
         ;  *writeivp = *readivp
         ;  writeivp++
      ;  }
         readivp++
   ;  }

      mclvResize(vec, writeivp - (vec->ivps))
   ;  return mass
;  }


dim mclvCountGiven
(  mclVector*     src
,  mcxbool       (*operation)(mclIvp* ivp, void* arg)
,  void*          arg
)  
   {  dim   n_src =  src->n_ivps
   ;  mclp *ivp   =  src->ivps
   ;  dim   n_hits=  0
   ;  while (n_src-- > 0)     /* careful with unsignedness */
      {  if (operation(ivp, arg))
         n_hits++
      ;  ivp++
   ;  }
      return n_hits
;  }


                              /* hierverder: src/dst conflict with mclvCopyGiven src/dst
                               * but only callers of mclvSelectIdcs I know have src == dst.
                              */
dim mclvSelectIdcs
(  mclv        *src
,  long        *lft
,  long        *rgt
,  mcxbits     equate
,  mclv        *dst
)
   {  mclpIRange range
   ;  range.lft = lft
   ;  range.rgt = rgt
   ;  range.equate = equate

   ;  mclvCopyGiven(src, dst, mclpSelectIdcs, &range, 0)
   ;  return dst->n_ivps
;  }


double mclvSelectGtBar
(  mclVector* vec
,  double     fbar
)
   {  return mclvSelectValues(vec, &fbar, NULL, MCLX_EQT_GT, vec)
;  }


double mclvSelectValues
(  mclv        *src
,  double      *lft
,  double      *rgt
,  mcxbits     equate
,  mclv        *dst
)
   {  mclpVRange range
   ;  range.lft = lft
   ;  range.rgt = rgt
   ;  range.equate = equate

   ;  mclvCopyGiven(src, dst, mclpSelectValues, &range, 0)
   ;  return mclvSum(dst)
;  }


mclVector* mclvCopyGiven
(  mclVector*     dst
,  mclVector*     src
,  mcxbool        (*operation)(mclIvp* ivp, void* arg)
,  void*          arg
,  dim            sup
)  
   {  dim         n_src
   ;  mclIvp      *src_ivp, *dst_ivp

                        /* dst allowed to be NULL */
   ;  if (dst != src)
      dst = mclvInstantiate(dst, sup ? sup : src->n_ivps, NULL)
   ; /*
      * else we must not destroy src before it is copied
     */

      n_src       =  src->n_ivps
   ;  src_ivp     =  src->ivps
   ;  dst_ivp     =  dst->ivps

                        /* BEWARE: this routine must work if dst==src */
                        /* n_src--: careful with unsignedness */

   ;  while (n_src-- > 0 && dst_ivp < dst->ivps + dst->n_ivps)
      {  if (operation(src_ivp, arg))
         {  dst_ivp->idx =  src_ivp->idx
         ;  dst_ivp->val =  src_ivp->val
         ;  dst_ivp++
      ;  }
         src_ivp++
   ;  }

      mclvResize(dst, dst_ivp - dst->ivps)
   ;  return dst
;  }


void mclvUnary
(  mclVector*  vec
,  double     (*operation)(pval val, void* arg)
,  void*       arg
)  
   {  dim      n_ivps
   ;  mclIvp   *src_ivp, *dst_ivp

   ;  n_ivps   =  vec->n_ivps
   ;  src_ivp  =  vec->ivps
   ;  dst_ivp  =  vec->ivps
   
   ;  while (n_ivps-- > 0)    /* careful with unsignedness */
      {  double val =  operation(src_ivp->val, arg)

      ;  if (val != 0.0)
         {  dst_ivp->idx =  src_ivp->idx
         ;  dst_ivp->val =  val
         ;  dst_ivp++
      ;  }
         src_ivp++
   ;  }
      mclvResize(vec, dst_ivp - vec->ivps)
;  }


/* NOTE caller ensures that both vectors have > 0 ivps
*/

static dim update_meet_small_large
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)  
   {  mcxbool update_small = FALSE
   ;  const mclv* s, *l
   ;  const mclp* l_ofs, *s_ofs, *s_max
   ;  dim n_zeroed = 0

;nu_meet_sl++

   ;  if (v1->n_ivps < v2->n_ivps)
      {  l = v2
      ;  s = v1
      ;  update_small = TRUE    /* updates have to be made in s, not l */
   ;  }
      else
      {  l = v1
      ;  s = v2
   ;  }

            /* search for largest applicable start in l; possible if
             * l->ivps[0] <= s->ivps[0] then k is maximal s.t.
             * l_ofs->ivps[k].idx <= s->ivps[0].idx
             *           _
             *    l    3 4     10 17 30 50
             *    s       5 8 9  11
            */
      if (l->ivps[0].idx <= s->ivps[0].idx)
         l_ofs = mclvGetIvpFloor(l, s->ivps[0].idx, NULL)
      ,  s_ofs = s->ivps+0
   ;  else
            /*                __       __
             *    l           19 20 21 22 30 39 40 44 45
             *    s     4 5 13         22
             *    In the example we first find small 22 on large 19;
             *    then we have to find large 22 on small 22.
            */
      {  if (!(s_ofs = mclvGetIvpCeil(s, l->ivps[0].idx, NULL)))
         return 0
      ;  l_ofs = mclvGetIvpFloor(l, s_ofs->idx, NULL)
   ;  }

      s_max = s->ivps + s->n_ivps

            /*
             * pre-condition: s_ofs < s_max.
             * invariant: l_ofs->idx <= s_ofs->idx
            */
   ;  while (l_ofs)
      {  if (s_ofs->idx == l_ofs->idx)
         {  if
            (  update_small
            && !(((mclp*) s_ofs)->val = op(s_ofs->val, l_ofs->val))
            )
            n_zeroed++
         ;  else if ( !(((mclp*) l_ofs)->val = op(l_ofs->val, s_ofs->val)))
            n_zeroed++
      ;  }
                  /* 
                   * alternative to ++s_ofs is mclvGetIvpCeil again,
                   * but s is small so we choose not to.
                  */
         if (++s_ofs >= s_max)
         break
      ;  l_ofs = mclvGetIvpFloor(l, s_ofs->idx, l_ofs)
   ;  }

      return n_zeroed
;  }


static dim update_meet_zip
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)
   {  mclp* ivp1 = v1->ivps, *ivp2 = v2->ivps
         ,  *ivp1max = ivp1 + v1->n_ivps
         ,  *ivp2max = ivp2 + v2->n_ivps

   ;  dim n_zeroed = 0

;nu_meet_zip++

   ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
      {  if (ivp1->idx < ivp2->idx)
         ivp1++
      ;  else if (ivp1->idx > ivp2->idx)
         ivp2++
      ;  else if (ivp1->val = op(ivp1->val, ivp2++->val), !ivp1++->val)
         n_zeroed++
   ;  }
      return n_zeroed
;  }


static dim update_meet_canonical
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)
   {  dim d = 0
   ;  dim n_zeroed = 0
   ;  long maxidx = v1->n_ivps-1

;nu_meet_can++

   ;  for (d=0;d<v2->n_ivps;d++)
      {  long idx = v2->ivps[d].idx
      ;  if (idx > maxidx)
         break
      ;  v1->ivps[idx].val = op(v1->ivps[idx].val, v2->ivps[d].val)
      ;  if (!v1->ivps[idx].val)
         n_zeroed++
   ;  }
      return n_zeroed
;  }


dim mclvUpdateMeet
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)  
   {  if (!v1->n_ivps || !v2->n_ivps)
      return 0

   ;  if ((int) MCLV_IS_CANONICAL(v1))
      return update_meet_canonical(v1, v2, op)
   ;  else if
      (  v2->n_ivps * nu_magic * log(v1->n_ivps) < v1->n_ivps
      || v1->n_ivps * nu_magic * log(v2->n_ivps) < v2->n_ivps
      )
      return update_meet_small_large(v1, v2, op)
   ;  else
      return update_meet_zip(v1, v2, op)
;  }


static dim update_diff_zip
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)
   {  mclp* p1 = v1->ivps, *p2 = v2->ivps
         ,  *p1max = p1 + v1->n_ivps
         ,  *p2max = p2 + v2->n_ivps

   ;  dim n_zeroed = 0

;nu_diff_zip++

   ;  while (p1 < p1max && p2 < p2max)
      {  if (p1->idx < p2->idx)
         {  if (!(p1->val = op(p1->val, 0)))
            n_zeroed++
         ;  p1++
      ;  }
         else if (p1->idx == (p2++)->idx)
         p1++
   ;  }

      while (p1 < p1max)
      {  if (!(p1->val = op(p1->val, 0)))
         n_zeroed++
      ;  p1++
   ;  }

      return n_zeroed
;  }


   /* update everything in  v1/v2 with op(v1,0)
   */
static dim update_diff_canonical
(  mclVector*  v1
,  const mclVector*  v2
,  double (*op)(pval mval, pval nval)
)
   {  dim d = 0
   ;  dim n_zeroed   =  0
   ;  long maxidx    =  v1->n_ivps-1
   ;  long prev_idx  =  -1, t

;nu_diff_can++

   ;  for (d=0;d<v2->n_ivps;d++)
      {  long idx = v2->ivps[d].idx
      ;  if (idx > maxidx)
         break

      ;  for (t=prev_idx+1; t<idx; t++)
         if (!(v1->ivps[t].val = op(v1->ivps[t].val, 0)))
         n_zeroed++
      ;  prev_idx = idx
   ;  }

      for (t=prev_idx+1; t<=maxidx;t++)
      if (!(v1->ivps[t].val = op(v1->ivps[t].val, 0)))
      n_zeroed++

   ;  return n_zeroed
;  }


static dim update_diff_small_large
(  mclVector*  a
,  const mclVector*  b
,  double  (*op)(pval mval, pval nval)
)  
   {  mclp *b_ofs = b->ivps+0
         , *a_ofs = a->ivps+0
         , *a_max = a_ofs + a->n_ivps
   ;  dim n_zeroed = 0

               /* no s vs l distinction as in update_meet_small_large,
                * because meet is symmetrical, diff not.
               */
;  nu_diff_sl++

   ;  if (b_ofs)
      while (a_ofs < a_max)
      {  if
         (  b_ofs->idx < a_ofs->idx
         && (!(b_ofs = mclvGetIvpCeil(b, a_ofs->idx, b_ofs)))
         )
         break
            /* nothing in (remainder of) b is >= current a */

      ;  if
         (  a_ofs->idx < b_ofs->idx
         && (!(a_ofs->val = op(a_ofs->val, 0.0)))
         )
         n_zeroed++
            /* current a is not in b; compute op */

      ;  a_ofs++
   ;  }

            /* all these are not in b either */
      while (a_ofs < a_max)            
      {  if (!(a_ofs->val = op(a_ofs->val, 0.0)))
         n_zeroed++
      ;  a_ofs++
   ;  }

      return n_zeroed
;  }



dim mclvUpdateDiff
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)  
   {  if (!v1->n_ivps)
      return 0

   ;  if (MCLV_IS_CANONICAL(v1))
      return update_diff_canonical(v1, v2, op)
   ;  else if
      (  (v1->n_ivps * nu_magic * log(v2->n_ivps) < v2->n_ivps)
      || (v2->n_ivps * nu_magic * log(v1->n_ivps) < v1->n_ivps)
      )
      return update_diff_small_large(v1, v2, op)
   ;  else
      return update_diff_zip(v1, v2, op)
;  }



mclVector* mclvBinaryx
(  const mclVector*  vec1
,  const mclVector*  vec2
,  mclVector*        dst
,  double           (*op)(pval arg1, pval arg2, pval arg3)
,  double            arg3
)  
   {  mclIvp  *ivp1, *ivp2, *ivp1max, *ivp2max, *ivpk, *ivpl
   ;  long n1n2 = vec1->n_ivps+vec2->n_ivps

   ;  if (vec1->n_ivps + vec2->n_ivps == 0)
      return mclvInstantiate(dst, 0, NULL)

   ;  ivpl  =  ivpk 
            =  mcxAlloc
               (  n1n2 * sizeof(mclIvp)
               ,  RETURN_ON_FAIL
               )
   ;  if (!ivpk)
      {  mcxMemDenied(stderr, "mclvBinary", "mclIvp", n1n2)
      ;  return NULL
   ;  }

      ivp1     =  vec1->ivps
   ;  ivp2     =  vec2->ivps

   ;  ivp1max  =  ivp1 + vec1->n_ivps
   ;  ivp2max  =  ivp2 + vec2->n_ivps
   ;
      
      {  double rval

      ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
         {  pval val1 =  0.0
         ;  pval val2 =  0.0
         ;  long idx

         ;  if (ivp1->idx < ivp2->idx)
            {  idx   =  ivp1->idx
            ;  val1  =  (ivp1++)->val
         ;  }
            else if (ivp1->idx > ivp2->idx)
            {  idx   =  ivp2->idx
            ;  val2  =  (ivp2++)->val
         ;  }
            else
            {  idx   =  ivp1->idx
            ;  val1  =  (ivp1++)->val
            ;  val2  =  (ivp2++)->val
         ;  }

            if ((rval = op(val1, val2, arg3)) != 0.0)
            {  ivpl->idx      =  idx
            ;  (ivpl++)->val  =  rval
         ;  }
         }

         while (ivp1 < ivp1max)
         {  if ((rval = op(ivp1->val, 0.0, arg3)) != 0.0)
            {  ivpl->idx      =  ivp1->idx
            ;  (ivpl++)->val  =  rval
         ;  }
            ivp1++
      ;  }

         while (ivp2 < ivp2max)
         {  if ((rval = op(0.0, ivp2->val, arg3)) != 0.0)
            {  ivpl->idx      =  ivp2->idx
            ;  (ivpl++)->val  =  rval
         ;  }
            ivp2++
      ;  }
      }

      dst = mclvInstantiate(dst, ivpl-ivpk, ivpk)
   ;  mcxFree(ivpk)
   ;  return dst
;  }



mclVector* mclvBinary
(  const mclVector*  vec1
,  const mclVector*  vec2
,  mclVector*        dst
,  double           (*op)(pval arg1, pval arg2)
)  
   {  mclIvp  *ivp1, *ivp2, *ivp1max, *ivp2max, *ivpk, *ivpl
   ;  long n1n2 = vec1->n_ivps+vec2->n_ivps

   ;  if (vec1->n_ivps + vec2->n_ivps == 0)
      return mclvInstantiate(dst, 0, NULL)

   ;  ivpl  =  ivpk 
            =  mcxAlloc
               (  n1n2 * sizeof(mclIvp)
               ,  RETURN_ON_FAIL
               )
   ;  if (!ivpk)
      {  mcxMemDenied(stderr, "mclvBinary", "mclIvp", n1n2)
      ;  return NULL
   ;  }

      ivp1     =  vec1->ivps
   ;  ivp2     =  vec2->ivps

   ;  ivp1max  =  ivp1 + vec1->n_ivps
   ;  ivp2max  =  ivp2 + vec2->n_ivps
   ;
      
      {  double rval

      ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
         {  pval val1 =  0.0
         ;  pval val2 =  0.0
         ;  long idx

         ;  if (ivp1->idx < ivp2->idx)
            {  idx   =  ivp1->idx
            ;  val1  =  (ivp1++)->val
         ;  }
            else if (ivp1->idx > ivp2->idx)
            {  idx   =  ivp2->idx
            ;  val2  =  (ivp2++)->val
         ;  }
            else
            {  idx   =  ivp1->idx
            ;  val1  =  (ivp1++)->val
            ;  val2  =  (ivp2++)->val
         ;  }

            if ((rval = op(val1, val2)) != 0.0)
            {  ivpl->idx      =  idx
            ;  (ivpl++)->val  =  rval
         ;  }
         }

         while (ivp1 < ivp1max)
         {  if ((rval = op(ivp1->val, 0.0)) != 0.0)
            {  ivpl->idx      =  ivp1->idx
            ;  (ivpl++)->val  =  rval
         ;  }
            ivp1++
      ;  }

         while (ivp2 < ivp2max)
         {  if ((rval = op(0.0, ivp2->val)) != 0.0)
            {  ivpl->idx      =  ivp2->idx
            ;  (ivpl++)->val  =  rval
         ;  }
            ivp2++
      ;  }
      }

      dst = mclvInstantiate(dst, ivpl-ivpk, ivpk)
   ;  mcxFree(ivpk)
   ;  return dst
;  }


mclVector* mclvMap
(  mclVector*  dst
,  long        mul
,  long        shift
,  mclVector*  src
)
   {  mclIvp*  ivp, *ivpmax

  /*  fixme: add error checking, overflow, sign */

   ;  if (!dst)
      dst = mclvCopy(NULL, src)
   ;  else if (src != dst)
      mclvInstantiate(dst, src->n_ivps, src->ivps)

   ;  ivp = dst->ivps
   ;  ivpmax = ivp + dst->n_ivps

   ;  while (ivp < ivpmax)
      {  ivp->idx = mul*ivp->idx + shift
      ;  ivp++
   ;  }

      return dst         
;  }


mclVector* mclvCanonicalExtend
(  mclv*       dst
,  dim         N
,  double      val
)
   {  dim j, N_old
   ;  ofs idx
   ;  if (!dst)
      return mclvCanonical(NULL, N, val)

   ;  N_old = dst->n_ivps
   ;  if (N < N_old)          /* fixme: err? */
      return dst

   ;  if (N_old)
      {  idx = dst->ivps[N_old-1].idx+1
      ;  if ((dim) idx != N_old)
         mcxErr("mclvCanonicalExtend", "argument not canonical (proceeding)")
   ;  }
      else
      idx = 0

   ;  mclvResize(dst, N)
   ;  for (j=N_old; j<N; j++)
         dst->ivps[j].idx = idx++ 
      ,  dst->ivps[j].val = val
   ;  return dst
;  }


   /* returns number of elements in src not found in dst */
dim mclvEmbed
(  mclv*       dst
,  const mclv* src
,  double      val
)  
   {  mclIvp* ivp
   ;  dim d =  0
   ;  dim n_notfound = 0
                           /* set everything to val */
   ;  ivp = dst->ivps
   ;  while (ivp < dst->ivps+dst->n_ivps)
      (ivp++)->val =  val

                           /* insert src values */
                           /* fixme: use better implementation,
                            * preferably with a callback
                           */
   ;  ivp = dst->ivps
   ;  for (d=0;d<src->n_ivps;d++)
      {  ivp = mclvGetIvp(dst, src->ivps[d].idx, ivp)
      ;  if (ivp)
         ivp->val = src->ivps[d].val
      ;  else
         n_notfound++
   ;  }
      return n_notfound
;  }


mclVector* mclvCanonicalEmbed
(  mclv*       dst
,  const mclv* src
,  dim         nr
,  double      val
)  
   {  mclIvp* ivp
   ;  dim d =  0
   ;  mclv* src_clone = NULL

   ;  if (dst == src)
         src_clone = mclvClone(src)
      ,  src = src_clone

   ;  dst = mclvResize(dst, nr) 

                           /* set everything to val */
   ;  ivp = dst->ivps
   ;  while (ivp < dst->ivps+dst->n_ivps)
      {  ivp->idx = d++
      ;  (ivp++)->val =  val
   ;  }

                           /* insert src values */
                           /* fixme: use better implementation,
                            * preferably with a callback
                           */
      ivp = dst->ivps
   ;  for (d=0;d<src->n_ivps;d++)
      {  ivp = mclvGetIvp(dst, src->ivps[d].idx, ivp)
      ;  if (ivp)
         ivp->val = src->ivps[d].val
   ;  }

      if (src_clone)
      mclvFree(&src_clone)
   ;  return dst
;  }


mclVector* mclvRange
(  mclVector*     dst
,  dim            n_ivps
,  dim            offset
,  double         val
)
   {  dim i
   ;  dst = mclvCanonical(dst, n_ivps, val)
   ;  for (i=0;i<n_ivps;i++)
      dst->ivps[i].idx = offset+i
   ;  return dst
;  }


mclVector* mclvCanonical
(  mclVector* dst
,  dim        nr
,  double     val
)  
   {  mclIvp* ivp
   ;  dim d  =  0
   ;  dst  =  mclvResize(dst, nr) 

   ;  ivp = dst->ivps

   ;  while (ivp < dst->ivps+dst->n_ivps)
      {  ivp->idx =  d++
      ;  (ivp++)->val =  val
   ;  }
      return dst
;  }


void mclvScale
(  mclVector*  vec
,  double      fac
)  
   {  dim      n_ivps   =  vec->n_ivps
   ;  mclIvp*  ivps     =  vec->ivps

   ;  if (!fac)
      mcxErr("mclvScale PBD", "zero")

   ;  while (n_ivps-- > 0)
      (ivps++)->val /= fac
;  }


double mclvNormalize
(  mclVector*  vec
)  
   {  dim      vecsize  = vec->n_ivps
   ;  mclIvp*  vecivps  = vec->ivps
   ;  double   sum      = mclvSum(vec)

   ;  vec->val =  sum

   ;  if (vec->n_ivps && sum == 0.0)
      {  mcxErr
         (  "mclvNormalize"
         ,  "warning: zero sum <%f> for vector <%ld>"
         ,  (double) sum
         ,  (long) vec->vid
         )
      ;  return 0.0
   ;  }
      else if (sum < 0.0)
      mcxErr("mclvNormalize", "warning: negative sum <%f>", (double) sum)

   ;  while (vecsize-- > 0)      /* careful with unsignedness */
      (vecivps++)->val /= sum
   ;  return sum
;  }


double mclvInflate
(  mclVector*  vec
,  double      power
)  
   {  mclIvp*  vecivps
   ;  dim      vecsize
   ;  double   powsum   =  0.0

   ;  if (!vec->n_ivps)
      return 0.0

   ;  vecivps  =  vec->ivps
   ;  vecsize  =  vec->n_ivps

   ;  while (vecsize-- > 0)
      {  (vecivps)->val = pow((double) (vecivps)->val, power)
      ;  powsum += (vecivps++)->val
   ;  }

     /* fixme static interface */
      if (powsum <= 0.0)
      {  mcxErr
         (  "mclvInflate"
         ,  "warning: nonpositive sum <%f> for vector %ld"
         ,  (double) powsum
         ,  (long) vec->vid
         )
      ;  mclvResize(vec, 0)
      ;  return 0.0
   ;  }

      vecivps = vec->ivps
   ;  vecsize = vec->n_ivps
   ;  while (vecsize-- > 0)
      (vecivps++)->val /= powsum

   ;  return pow((double) powsum, power > 1.0 ? 1/(power-1) : 1.0)
;  }


double mclvVal
(  const mclVector*   vec
)
   {  return vec ? vec->val : 0.0
;  }


double mclvSize
(  const mclVector*   vec
)
   {  return vec ? vec->n_ivps : 0.0
;  }


double mclvSelf
(  const mclVector* vec
)  
   {  return vec ? mclvIdxVal(vec, vec->vid, NULL) :  0.0
;  }


double mclvSum
(  const mclVector* vec
)  
   {  double   sum      =  0.0
   ;  mclIvp*  vecivps  =  vec->ivps
   ;  dim      vecsize  =  vec->n_ivps

   ;  while (vecsize-- > 0)      /* careful with unsignedness */
      {  sum += vecivps->val
      ;  vecivps++  
   ;  }
      return sum
;  }


double mclvPowSum
(  const mclVector* vec
,  double power
)  
   {  mclIvp* vecivps = vec->ivps
   ;  dim     vecsize = vec->n_ivps
   ;  double  powsum  = 0.0

   ;  while (vecsize-- > 0)      /* careful with unsignedness */
      powsum += (float) pow((double) (vecivps++)->val, power)
   ;  return powsum
;  }


double mclvNorm
(  const mclVector*        vec
,  double                  power
)  
   {  if(power > 0.0)
         mcxErr("mclvNorm", "pbd: negative power argument <%f>", (double) power)
      ,  mcxExit(1)

   ;  return pow((double) mclvPowSum(vec, power), 1.0 / power)
;  }


ofs mclvGetIvpOffset
(  const mclv* vec
,  long        idx
,  ofs         offset
)
   {  mclIvp*  match    =  mclvGetIvp
                           (  vec
                           ,  idx
                           ,  offset >= 0
                              ?  vec->ivps + offset
                              :  vec->ivps
                           )
   ;  return match ? match - vec->ivps : -1
;  }


mclIvp* mclvGetIvpCeil
(  const mclVector*  vec
,  long              idx
,  const mclIvp*     offset
)
   {  mclIvp sought
   ;  const mclp *base  =  offset ? offset : vec->ivps
   ;  dim n_ivps        =  vec->n_ivps - (base - vec->ivps)
   ;  sought.idx = idx
   ;  sought.val = 1.0

   ;  return
      mclpBsearchCeil(&sought, base, n_ivps)
;  }


mclIvp* mclvGetIvpFloor
(  const mclVector*  vec
,  long              idx
,  const mclIvp*     offset
)
   {  mclIvp   sought
   ;  const mclp *base  =  offset ? offset : vec->ivps
   ;  dim n_ivps        =  vec->n_ivps - (base - vec->ivps)
   ;  sought.idx = idx
   ;  sought.val = 1.0
   ;  return
      mclpBsearchFloor(&sought, base, n_ivps)
;  }


mclIvp* mclvGetIvp
(  const mclVector*  vec
,  long              idx
,  const mclIvp*     offset
)  
   {  mclIvp   sought
   ;  const mclp *base  =  offset ? offset : vec->ivps
   ;  dim n_ivps        =  vec->n_ivps - (base - vec->ivps)

   ;  mclpInstantiate(&sought, idx, 1.0)

   ;  return
         (vec->n_ivps)
      ?  bsearch(&sought, base, n_ivps, sizeof(mclIvp), mclpIdxCmp)
      :  NULL
;  }


double mclvIdxVal
(  const mclVector*  vec
,  long        idx
,  ofs*        p_offset
)  
   {  ofs     offset   =  mclvGetIvpOffset(vec, idx, -1)
   ;  double  value    =  0.0

   ;  if (p_offset)
      *p_offset = offset
      
   ;  if (offset >= 0)
      value = (vec->ivps+offset)->val

   ;  return value
;  }


mcxstatus mclvReplaceIdx
(  mclVector*     vec
,  long           ofs
,  long           idx
,  double         val
)
   {  mclp piv, *dst

   ;  if (!vec || ofs < 0 || vec->n_ivps <= (dim) ofs)
      return STATUS_FAIL

   ;  if (mclvGetIvp(vec, idx, NULL))
      return STATUS_FAIL

   ;  piv.idx = idx
   ;  piv.val = val

                     /*       a  b  c  d  _  f  g  h i
                      * e will go;
                      * idx needs to be somewhere in the f-i range. If i needs
                      * to go then we have
                      *       a  b  c  d  e  f  g  h _
                      * then (vec->ivps+ofs+1) == dst == vec->ivps+vec->n_ivps
                      * and the size to copy is zero.
                     */
   ;  if (vec->ivps[ofs].idx < idx)
      {  if (!(dst = mclpBsearchCeil(&piv, vec->ivps, vec->n_ivps)))
         dst = vec->ivps+vec->n_ivps
      ;  memmove
         (  vec->ivps+ofs           /* destination */
         ,  vec->ivps+ofs+1         /* source      */
         ,  sizeof piv * ((dst-(vec->ivps+ofs))-1)
         )
      ;  dst[-1] = piv
   ;  }
                     /*       a  b  c  d  _  f  g  h i
                      * e will go;
                      * idx needs to be somewhere in the a-d range. If a
                      * needs to go then we have
                      *       _  b  c  d  e  f  g  h i
                      * then ofs == vec->ivps and dst == vec->ivps
                      * and the size to copy is zero.
                     */
      else if (vec->ivps[ofs].idx > idx)
      {  if (!(dst = mclpBsearchFloor(&piv, vec->ivps, vec->n_ivps)))
         dst = vec->ivps
      ;  else
         dst++       
      ;  memmove
         (  dst+1    /* destination */
         ,  dst      /* source */
         ,  sizeof piv * (vec->ivps+ofs-dst)
         )
      ;  dst[0] = piv
   ;  }
      return STATUS_OK
;  }


mclVector* mclvInsertIvp
(  mclVector*  vec
,  long        idx
,  mclp**      ivpp
)  
   {  ofs offset = 0             /* initialises first clause below */

   ;  if (!vec)
         vec = mclvInstantiate(NULL, 1, NULL)
      ,  vec->ivps[0].val = 0.0

   ;  else if ((offset =  mclvGetIvpOffset(vec, idx, -1)) >= 0)
      /* THAT'S ALL, offset correct */

   ;  else
      {  dim d = vec->n_ivps
      ;  mclvResize(vec, d+1)
      ;  while (d && vec->ivps[d-1].idx > idx)
            vec->ivps[d] = vec->ivps[d-1]
         ,  d--
      ;  offset = d
      ;  vec->ivps[offset].val = 0.0
   ;  }

      vec->ivps[offset].idx = idx
   ;  ivpp[0] = vec->ivps+offset
   ;  return vec
;  }


inline mclVector* mclvInsertIdx
(  mclVector*  vec
,  long        idx
,  double      val
)  
   {  mclp* p
   ;  mclv* v = mclvInsertIvp(vec, idx, &p)
   ;  p->val = val
   ;  return v
;  }


inline mclVector* mclvAddtoIdx
(  mclVector*  vec
,  long        idx
,  double      val
)  
   {  mclp* p
   ;  mclv* v = mclvInsertIvp(vec, idx, &p)
   ;  p->val += val
   ;  return v
;  }


void mclvRemoveIdx
(  mclVector*  vec
,  long        idx
)  
   {  ofs offset = mclvGetIvpOffset(vec, idx, -1)
                     /* check for nonnull vector is done in mclvIdxVal */
   ;  if (offset >= 0)
      {  memmove
         (  vec->ivps + offset
         ,  vec->ivps + offset + 1
         ,  (vec->n_ivps - offset - 1) * sizeof(mclIvp)
         )
      ;  mclvResize(vec, vec->n_ivps - 1)
   ;  }
   }


int mclvVidCmp
(  const void*  p1
,  const void*  p2
)
   {  long diff = ((mclVector*) p1)->vid - ((mclVector*)p2)->vid
   ;  return MCX_SIGN(diff)
;  }


int mclvValCmp
(  const void*  p1
,  const void*  p2
)
   {  double diff = ((mclVector*) p1)->val - ((mclVector*)p2)->val
   ;  return MCX_SIGN(diff)
;  }


int mclvValRevCmp
(  const void*  p1
,  const void*  p2
)
   {  double diff = ((mclVector*) p2)->val - ((mclVector*)p1)->val
   ;  return MCX_SIGN(diff)
;  }


int mclvSizeRevCmp
(  const void*  p1
,  const void*  p2
)
   {  long diff  = ((mclVector*)p2)->n_ivps - ((mclVector*)p1)->n_ivps
   ;  if (diff)
      return MCX_SIGN(diff)
   ;  else
      return mclvLexCmp(p1, p2)
;  }


int mclvSizeCmp
(  const void*  p1
,  const void*  p2
)
   {  return mclvSizeRevCmp(p2, p1)
;  }


int mclvLexCmp
(  const void*  p1
,  const void*  p2
)
   {  mclIvp*   ivp1    =  ((mclVector*)p1)->ivps
   ;  mclIvp*   ivp2    =  ((mclVector*)p2)->ivps
   ;  long      diff
   ;  dim       n_ivps  =  MCX_MIN
                           (  ((mclVector*)p1)->n_ivps
                           ,  ((mclVector*)p2)->n_ivps
                           )
  /*
   *  Vectors with low numbers first
  */
   ;  while (n_ivps-- > 0)       /* careful with unsignedness */
      if ((diff = (ivp1++)->idx - (ivp2++)->idx))
      return MCX_SIGN(diff)

   ;  diff = ((mclVector*)p1)->n_ivps - ((mclVector*)p2)->n_ivps
   ;  return MCX_SIGN(diff)
;  }


int mclvSumCmp
(  const void* p1
,  const void* p2
)  
   {  double diff = mclvSum((mclVector*) p1) - mclvSum((mclVector*) p2)
   ;  return MCX_SIGN(diff)
;  }


int mclvSumRevCmp
(  const void* p1
,  const void* p2
)  
   {  return mclvSumCmp(p2, p1)
;  }


void mclvSelectHighest
(  mclVector*  vec
,  dim         max_n_ivps
)  
   {  double f
   ;  if (vec->n_ivps <= max_n_ivps)
      return

   ;  f =   vec->n_ivps >= 2 * max_n_ivps
            ?  mclvKBar
               (vec, max_n_ivps, PVAL_MAX, KBAR_SELECT_LARGE)
            :  mclvKBar
               (vec, vec->n_ivps - max_n_ivps + 1, -PVAL_MAX, KBAR_SELECT_SMALL)

   ;  mclvSelectGqBar(vec, f)
   ;  if (vec->n_ivps > max_n_ivps)
      mclvSelectGqBar(vec, f * (1.0 + PVAL_EPSILON))
;  }


void mclvSelectHighestGQ
(  mclVector*  vec
,  dim         max_n_ivps
)  
   {  double f
   ;  if (vec->n_ivps <= max_n_ivps)
      return

   ;  f =   vec->n_ivps >= 2 * max_n_ivps
            ?  mclvKBar
               (vec, max_n_ivps, PVAL_MAX, KBAR_SELECT_LARGE)
            :  mclvKBar
               (vec, vec->n_ivps - max_n_ivps + 1, -PVAL_MAX, KBAR_SELECT_SMALL)

   ;  mclvSelectGqBar(vec, f)
;  }


void mclvSelectHighestGT
(  mclVector*  vec
,  dim         max_n_ivps
)  
   {  double f
   ;  if (vec->n_ivps <= max_n_ivps)
      return

   ;  f =   vec->n_ivps >= 2 * max_n_ivps
            ?  mclvKBar
               (vec, max_n_ivps, PVAL_MAX, KBAR_SELECT_LARGE)
            :  mclvKBar
               (vec, vec->n_ivps - max_n_ivps + 1, -PVAL_MAX, KBAR_SELECT_SMALL)

   ;  mclvSelectGtBar(vec, f)
;  }


void mclvZeroValues
(  mclVector*  vec
)  
   {  mclIvp*  ivp   =  vec->ivps
   ;  mclIvp*  maxivp=  ivp + vec->n_ivps

   ;  while (ivp<maxivp)
      (ivp++)->val = 0.0
;  }


void mclvMakeConstant
(  mclVector*  vec
,  double      val
)  
   {  if (val == 0.0) 
      mclvZeroValues(vec)        /* cannot use mclvUnary, it removes zeroes */
   ;  else
      mclvUnary(vec, fltxConst, &val)
;  }


void mclvMakeCharacteristic
(  mclVector* vec
)  
   {  mclp* ivp = vec->ivps, *ivpmax = ivp+vec->n_ivps
   ;  while (ivp<ivpmax)
      (ivp++)->val = 1.0
;  }


void mclvHdp
(  mclVector* vec
,  double power
)  
   {  mclvUnary(vec, fltxPower, &power)
;  }


mclVector* mclvAdd
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*  dst
)  
   {  return mclvBinary(lft, rgt, dst, fltAdd)
;  }


mclVector* mcldMerge
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*  dst
)
   {  return mclvBinary(lft, rgt, dst, fltLoR)
;  }


/*
 * Remember that the dst values have to come from lft
*/

mclVector* mcldMinus
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*  dst
)  
   {  if (rgt == dst)         /* interesting src/dst pattern */
      return mclvBinary(lft, rgt, dst, fltLaNR)
   ;  else
      {  if (lft != dst)
         dst = mclvCopy(dst, lft)
            /* if lft is large and rgt as well then copying may introduce
             * unnecessary work. But we don't really have a way of knowing in
             * advance, and the solution would probably involve headaches
             * similar to the ones in mcldMeet.
            */
      ;  if (mclvUpdateMeet(dst, rgt, flt0p0))
         mclvUnary(dst, fltxCopy, NULL)
   ;  }
      return dst
;  }


/*
 * Remember that the dst values have to come from lft
*/

mclVector* mcldMeet
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*  dst
)
   {  return mclvBinary(lft, rgt, dst, fltLaR)
;  }

                      /* this code is hilarious hiledeous ..
                       * but it is an honest attempt at optimizing different scenarios.
                      */
mclVector* mcldMeet2
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*  dst
)
   {  if (lft == rgt)
      return dst == lft ? dst : mclvCopy(dst, lft)

   ;  if
      (  dst != lft
      && (  nu_magic * log(lft->n_ivps+1) * rgt->n_ivps < lft->n_ivps    /* this means rgt is small */
         || dst == rgt
         )
      )
      {  if (dst != rgt)                /* now (small) dst has only rgt values (and lft != rgt) */
         dst = mclvCopy(dst, rgt)
      ;  if (mclvUpdateDiff(dst, lft, flt0p0))  /* rgt meet values remain */
         mclvUnary(dst, fltxCopy, NULL)
      ;  mclvUpdateMeet(dst, lft, fltRight)     /* set them to left again */ 
   ;  }
                     /*       dst == lft
                      * or    (     size comparison ok
                      *       and   dst != rgt
                      *       )
                     */
      else
      {  if (dst == lft)
         NOTHING
      ;  else        /*  dst != rgt */
         dst = mclvCopy(dst, lft)

      ;  if (mclvUpdateDiff(dst, rgt, flt0p0))
         mclvUnary(dst, fltxCopy, NULL)
   ;  }

      return dst
;  }


mcxbool mcldEquate
(  const mclVector* dom1
,  const mclVector* dom2
,  mcxenum    mode
)
   {  dim meet, ldif, rdif
   ;  mcldCountParts(dom1, dom2, &ldif, &meet, &rdif)

   ;  switch(mode)
      {  case MCLD_EQT_SUPER
      :  return rdif == 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_SUB
      :  return ldif == 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_EQUAL
      :  return ldif + rdif == 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_DISJOINT
      :  return meet == 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_TRISPHERE
      :  return ldif != 0 && rdif != 0 && meet != 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_LDIFF
      :  return ldif != 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_MEET
      :  return meet != 0 ? TRUE : FALSE
      ;

         case MCLD_EQT_RDIFF
      :  return rdif != 0 ? TRUE : FALSE
      ;

         default
      :  mcxErr("mcldEquate PBD", "unknown mode <%d>", (int) mode)
      ;  break
   ;  }
      return TRUE
;  }


dim mcldCountSet
(  const mclVector*  dom1
,  const mclVector*  dom2
,  mcxbits           parts
)
   {  dim meet, ldif, rdif, count = 0
   ;  mcldCountParts(dom1, dom2, &ldif, &meet, &rdif)
   ;  if (parts & MCLD_CT_LDIFF)
      count += ldif
   ;  if (parts & MCLD_CT_MEET)
      count += meet
   ;  if (parts & MCLD_CT_RDIFF)
      count += rdif
   ;  return count
;  }


dim mcldCountParts
(  const mclVector* dom1
,  const mclVector* dom2
,  dim*       ld
,  dim*       mt
,  dim*       rd
)
   {  mclIvp   *ivp1, *ivp2, *ivp1max, *ivp2max
   ;  dim meet = 0, ldif = 0, rdif = 0, ret = 0

   ;  ivp1     =  dom1->ivps
   ;  ivp2     =  dom2->ivps

   ;  ivp1max  =  ivp1 + dom1->n_ivps
   ;  ivp2max  =  ivp2 + dom2->n_ivps

   ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
      {  if (ivp1->idx < ivp2->idx)
            ldif++
         ,  ivp1++
      ;  else if (ivp1->idx > ivp2->idx)
            rdif++
         ,  ivp2++
      ;  else
            meet++
         ,  ivp1++
         ,  ivp2++
   ;  }

      ldif += ivp1max - ivp1
   ;  rdif += ivp2max - ivp2
   ;  if (ld)
         *ld = ldif
      ,  ret += ldif
   ;  if (rd)
         *rd = rdif
      ,  ret += rdif
   ;  if (mt)
         *mt = meet
      ,  ret += meet
   ;  return ret
;  }


mclv* mclvFromIvps
(  mclv* dst
,  mclp* ivps
,  dim   n_ivps
)
   {  mclpAR ar
   ;  ar.ivps    =  ivps
   ;  ar.n_ivps  =  n_ivps
   ;  ar.n_alloc =  n_ivps
   ;  ar.sorted  =  0
   ;  return mclvFromPAR(dst, &ar, 0, mclpMergeLeft, NULL )
;  }


/* current dst content is thrown away if fltbinary not used */
mclv* mclvFromPAR
(  mclv*      dst
,  mclpAR*    par  
,  mcxbits    warnbits
,  void     (*ivpmerge)(void* ivp1, const void* ivp2)
,  double   (*fltbinary)(pval val1, pval val2)
)
   {  mcxbool  warn_re   =  warnbits & MCLV_WARN_REPEAT_ENTRIES
   ;  mcxbool  warn_rv   =  warnbits & MCLV_WARN_REPEAT_VECTORS
   ;  mclp*    ivps      =  par->ivps
   ;  dim      n_ivps    =  par->n_ivps
   ;  mcxbits  sortbits  =  par->sorted
   ;  dim      n_old     =  dst ? dst->n_ivps : 0
   ;  const char* me     =  "mclvFromPAR"
   ;  dim n_re = 0, n_rv = 0
   ;  if (!dst)
      dst = mclvInit(NULL)

   ;  if (n_ivps)
      {  if (dst->n_ivps && fltbinary)
         {  mclVector* tmpvec = mclvNew(ivps, n_ivps)

         ;  if (!(sortbits & MCLPAR_SORTED))
            mclvSort(tmpvec, NULL)

         ;  if (!(sortbits & MCLPAR_UNIQUE))
            n_re = mclvUniqIdx(tmpvec, ivpmerge)

         ;  n_rv += tmpvec->n_ivps
         ;  n_rv += dst->n_ivps
         ;  mclvBinary(dst, tmpvec, dst, fltbinary)
         ;  n_rv -= dst->n_ivps

         ;  mclvFree(&tmpvec)
      ;  }
         else
         {  if (dst->ivps == ivps)
            mcxErr(me, "DANGER dst->ivps == ivps (dst vid %d)", (int) dst->vid)

         ;  mclvRenew(dst, ivps, n_ivps)

         ;  if (!(sortbits & MCLPAR_SORTED))
            mclvSort(dst, NULL)

         ;  if (!(sortbits & MCLPAR_UNIQUE))
            n_re += mclvUniqIdx(dst, ivpmerge)
      ;  }
      }

      if (warn_re && n_re)
      mcxErr
      (  me
      ,  "<%ld> found <%ld> repeated entries within %svector"
      ,  (long) dst->vid
      ,  (long) n_re
      ,  n_rv ? "repeated " : ""
      )

   ;  if (warn_rv && n_rv)
      mcxErr
      (  me
      ,  "<%ld> new vector has <%ld> overlap with previous amalgam"
      ,  (long) dst->vid
      ,  (long) n_rv
      )

   ;  if (warnbits && n_re + n_rv)
      mcxErr
      (  me
      ,  "<%ld> vector went from <%ld> to <%ld> entries"
      ,  (long) dst->vid
      ,  (long) n_old
      ,  (long) dst->n_ivps
      )
   ;  return dst
;  }


mcxbool mcldIsCanonical
(  mclVector* vec
)
   {  dim n = vec->n_ivps
   ;  if (!n)
      return TRUE
   ;  if (vec->ivps[0].idx == 0 && vec->ivps[n-1].idx == (long) (n-1))
      return TRUE
   ;  return FALSE
;  }


long mclvHighestIdx
(  mclVector*  vec
)
   {  dim n = vec->n_ivps
   ;  if (!n)
      return -1
   ;  return vec->ivps[n-1].idx
;  }


double mclvHasLoop
(  const mclVector*   vec
)
   {  mclp* ivp = mclvGetIvp(vec, vec->vid, NULL)
   ;  return ivp ? 1.0 : 0.0
;  }


double mclvMinValue
(  const mclVector*           vec
)  
   {  double min_val = FLT_MAX
   ;  mclIvp* ivp    = vec->ivps  
   ;  mclIvp* ivpmax = ivp + vec->n_ivps
   ;  while (ivp<ivpmax)
      {  if (ivp->val < min_val)
         min_val = ivp->val
      ;  ivp++
   ;  }
      return  min_val
;  }


double mclvMaxValue
(  const mclVector*           vec
)  
   {  double max_val = -FLT_MAX
   ;  mclIvp* ivp    = vec->ivps  
   ;  mclIvp* ivpmax = ivp + vec->n_ivps
   ;  while (ivp<ivpmax)
      {  if (ivp->val > max_val)
         max_val = ivp->val
      ;  ivp++
   ;  }
      return  max_val
;  }


long mclvUnaryList
(  mclv*    vec
,  mclpAR*  ar       /* idx: MCLX_UNARY_mode, val: arg */
)
   {  dim      n_ivps
   ;  mclIvp   *src_ivp, *dst_ivp

   ;  n_ivps   =  vec->n_ivps
   ;  src_ivp  =  vec->ivps
   ;  dst_ivp  =  vec->ivps
   
   ;  while (n_ivps-- > 0)    /* careful with unsignedness */
      {  double val =  mclpUnary(src_ivp, ar)

      ;  if (val != 0.0)
         {  dst_ivp->idx =  src_ivp->idx
         ;  dst_ivp->val =  val
         ;  dst_ivp++
      ;  }
         src_ivp++
   ;  }

      mclvResize(vec, dst_ivp - vec->ivps)
   ;  return vec->n_ivps
;  }



void mclvMean
(  const mclv*    vec
,  dim      N           /* vec does/might not store zeroes */
,  double   *meanp
,  double   *stdp
)
   {  dim d
   ;  double sum, mean, std = 0.0

   ;  *meanp = 0.0
   ;  *stdp  = 0.0

   ;  if (!N && !(N = vec->n_ivps))
      return
   ;  else if (N < vec->n_ivps)
      mcxErr("mclvMean PBD", "N < vec->n_ivps (vid %ld)", (long) vec->vid)

   ;  sum  = mclvSum(vec)
   ;  mean = sum/N

   ;  for (d=0;d<vec->n_ivps;d++)
      {  double dif = vec->ivps[d].val - mean
      ;  std += dif * dif
   ;  }

      if (N > vec->n_ivps)
      std += (N-vec->n_ivps) * mean * mean

   ;  *stdp = sqrt(std/N)
   ;  *meanp = mean
;  }


void mclvAffine
(  mclv* vec
,  double mean
,  double std
)
   {  dim d

   ;  if (std)
      for (d=0;d<vec->n_ivps;d++)
      vec->ivps[d].val = (vec->ivps[d].val - mean) / std
   ;  else
      for (d=0;d<vec->n_ivps;d++)
      vec->ivps[d].val -= mean
;  }


void mclvSprintf
(  mcxTing* scr
,  mclv* vec
,  int valdigits
,  mcxbits modes
)
   {  dim j
   ;  mcxTingEmpty(scr, 0)
   ;  valdigits = get_interchange_digits(valdigits)

   ;  if (modes & 1)
      {  mcxTingPrintAfter(scr, "%ld", (long) vec->vid)
      ;  if (modes & 2)
         mcxTingPrintAfter(scr, ":%.*g", valdigits, (double) vec->val)
   ;  }
         
      for (j=0;j<vec->n_ivps;j++)
      {  mcxTingPrintAfter(scr, " %ld", (long) vec->ivps[j].idx)
      ;  if (modes & 2)
         mcxTingPrintAfter(scr, ":%.*g", valdigits, (double) vec->ivps[j].val)
   ;  }

      if (modes & 4)
      mcxTingAppend(scr, " $")
;  }


