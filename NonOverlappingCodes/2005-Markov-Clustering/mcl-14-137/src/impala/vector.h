/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* TODO
 *    unify various select routines in callback frame.
*/


#ifndef impala_vector_h
#define impala_vector_h

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "pval.h"
#include "ivp.h"
#include "iface.h"

#include "util/types.h"


enum
{  KBAR_SELECT_SMALL =  10000
,  KBAR_SELECT_LARGE
}  ;


enum
{  MCL_VECTOR_COMPLETE = 1
,  MCL_VECTOR_SPARSE
}  ;


typedef struct
{  dim         n_ivps
;  long        vid      /* vector id */
;  double      val      /* for misc purposes */
;  mclIvp*     ivps
;
}  mclVector   ;

#define mclv mclVector

#define MCLV_SIZE(vec)   ((vec)->n_ivps)
#define MCLV_MAXID(vec)  (MCLV_SIZE(vec) ? ((vec)->ivps)[MCLV_SIZE(vec)-1].idx : 0)
#define MCLV_MINID(vec)  (MCLV_SIZE(vec) ? ((vec)->ivps)[0].idx : 0)


#define MCLV_IS_CANONICAL(vec)  (!(vec)->n_ivps || \
      ((vec)->ivps[(vec)->n_ivps-1].idx == (long) ((vec)->n_ivps-1)))

#define PNUM_IN_RANGE(pivot, offset, count)  \
      ((pivot) >= (offset) && (pivot) < (offset) + (count))

#define PNUM_IN_INTERVAL(pivot, left_inclusive, right_inclusive)  \
      ((pivot) >= (left_inclusive) && (pivot) < (right_inclusive))


/* below are used for reading raw input */

#define  MCLV_WARN_REPEAT_ENTRIES  1
#define  MCLV_WARN_REPEAT_VECTORS  2
#define  MCLV_WARN_REPEAT (MCLV_WARN_REPEAT_ENTRIES|MCLV_WARN_REPEAT_VECTORS)


/* INTEGRITY
 *    The idx members of the ivp array need to be nonnegative and ascending.
 *    The val members are usually positive, but this is not mandated.
 *
 *    The setting n_ivps == 0, ivps == NULL is obviously legal.
 *
 * NOTE
 *    Most unary and binary operations remove resulting entries that are zero.
 *
 *    This means it is dangerous to use those operations on vectors
 *    representing value lists (i.e. columns from value tables) that contain
 *    zero-valued entries - one might want to combine two different such
 *    vectors simply by using mclvAdd, but the zero-valued entries will be
 *    removed.
 *
 *    mclvUpdateMeet is an exception to this rule.
*/


#define  MCLV_CHECK_DEFAULT      0
#define  MCLV_CHECK_POSITIVE     1
#define  MCLV_CHECK_NONZERO      2
#define  MCLV_CHECK_NONNEGATIVE  (MCLV_CHECK_POSITIVE | MCLV_CHECK_NONZERO)

mcxstatus mclvCheck
(  const mclVector*  vec
,  long              min         /* inclusive */
,  long              max         /* inclusive */
,  mcxbits           modes
,  mcxOnFail         ON_FAIL
)  ;


mclVector* mclvInit
(  mclVector*     vec
)  ;

void* mclvInit_v
(  void*     vec
)  ;


mclVector* mclvNew
(  mclp*    ivps
,  dim      n_ivps
)  ;


mclVector* mclvRenew
(  mclv*    dst
,  mclp*    ivps
,  dim      n_ivps
)  ;


/* This leaves vec in inconsistent state unless it is shrinking
 * Inconsistent meaning: ivp with idx set to -1.
 *
 * vec argument can be NULL.

 * If the vector is reduced in size, memory is not reclaimed.
 * Conceivably a fixme.
*/

mclVector* mclvResize
(  mclVector*     vec
,  dim            n_ivps
)  ;


/* NOTE
 *    src can be NULL
*/
mclVector* mclvCopy
(  mclVector*        dst
,  const mclVector*  src
)  ;


/* NOTE
 *    src can be NULL
*/
mclVector* mclvClone
(  const mclVector*  src
)  ;


/* NOTE
 *    *vec_p can be NULL
*/
void mclvFree
(  mclVector**    vec_p
)  ;


void mclvFree_v
(  void*          vec_p
)  ;


void mclvRelease
(  mclVector*     vec
)  ;


void mclvRelease_v
(  void*          vec
)  ;


mclv* mclvFromIvps
(  mclv* dst
,  mclp* ivps
,  dim n_ivps
)  ;


mclv* mclvFromPAR
(  mclv* dst
,  mclpAR* par
,  mcxbits warnbits
,  void (*ivpmerge)(void* ivp1, const void* ivp2)
,  double (*fltbinary)(pval val1, pval val2)
)  ;


/*
 *    If mul > 0, it will map indices i to mul*i + shift. 
 *    If mul < 0, it will map indices i to (i+shift)/mul.
 *    Be careful to check the validity of the result (mul < 0)!
 *    shift may assume all values.
*/

mclVector* mclvMap
(  mclVector*     dst
,  long           mul
,  long           shift
,  mclVector*     src
)  ;


mclVector* mclvCanonical
(  mclVector*     dst
,  dim            n_ivps
,  double         val
)  ;


   /* returns number of elements in src not found in dst
    * dst and src must be different.
   */
dim mclvEmbed
(  mclv*       dst
,  const mclv* src
,  double      val
)  ;


   /* Initialize values to those in dst; if the index
    * is not present then set it to val
   */
mclVector* mclvCanonicalEmbed
(  mclv*       dst
,  const mclv* src
,  dim         nr
,  double      val
)  ;  

   /* Further extend a canonical vector.
    * dst == NULL yields a new vector.
    * dst->n_ivps > N is an acceptable no-op.
    * dst not canonical will proceed (with the top index)
    *    and issue an error
   */
mclVector* mclvCanonicalExtend
(  mclv*       dst
,  dim         N
,  double      val
)  ;


mclVector* mclvRange
(  mclVector*     dst
,  dim            n_ivps
,  dim            offset
,  double         val
)  ;


void mclvSort
(  mclVector*     vec
,  int           (*mclpCmp)(const void*, const void*)
)  ;


void mclvSortAscVal
(  mclVector*     vec
)  ;


void mclvSortDescVal
(  mclVector*     vec
)  ;


dim mclvUniqIdx
(  mclVector*     vec
,  void (*merge)(void* ivp1, const void* ivp2)
)  ;


/* sorts vectors and discards duplicates (based on idx) */

void mclvSortUniq
(  mclVector*  vec
)  ;


      /* this one uses GQ if possible, otherwise GQ threshold * (1+epsilon) */
void mclvSelectHighest
(  mclVector*     vec
,  dim            max_n_ivps
)  ;

      /* always uses GQ */
void mclvSelectHighestGQ
(  mclVector*     vec
,  dim            max_n_ivps
)  ;

      /* always uses GT */
void mclvSelectHighestGT
(  mclVector*     vec
,  dim            max_n_ivps
)  ;


/*
 * ignore:
 *    when searching k large elements, consider only those elements that are <
 *    ignore. case mode = KBAR_SELECT_LARGE
 *
 *    when searching k small elements,
 *    consider only those elements that are >= ignore. case mode =
 *    KBAR_SELECT_SMALL
*/

double mclvKBar
(  mclVector      *vec
,  dim            max_n_ivps
,  double         ignore
,  int            mode
)  ;



double mclvSelectValues
(  mclv*          src
,  double         *lft        /* NULL for turning of lft comparison  */
,  double         *rgt        /* NULL for turning of rgt comparison  */
,  mcxbits        equate      /*  0,1,or 2 of { MCLX_EQT_GQ,  MCLX_EQT_LQ }  */
,  mclv*          dst
)  ;


dim mclvSelectIdcs
(  mclv        *src
,  long        *lft
,  long        *rgt
,  mcxbits     equate
,  mclv        *dst
)  ;


/* this one should be a wee bit more efficient than
 * mclSelectValues - it should be measured sometime though.
 * mclvSelectGqBar is often called by mcl - hence the
 * special purpose routine. No siblings for lt, lq,
 * you have to call mclvSelectValues for those.
*/

double mclvSelectGqBar
(  mclVector*     vec
,  double         bar
)  ;

double mclvSelectGtBar
(  mclVector* vec
,  double     fbar
)  ;


   /* If the vector is reduced in size, memory is not reclaimed.
    * Conceivably a fixme.
   */
void mclvUnary
(  mclVector*     vec
,  double        (*operation)(pval val, void* argument)
,  void*          argument
)  ;


mclVector* mclvBinary
(  const mclVector*  src1
,  const mclVector*  src2
,  mclVector*        dst
,  double           (*operation)(pval val1, pval val2)
)  ;


mclVector* mclvBinaryx
(  const mclVector*  src1
,  const mclVector*  src2
,  mclVector*        dst
,  double           (*operation)(pval val1, pval val2, pval val3)
,  double            arg3
)  ;


/*    <!!!!>   Experimental.
 *
 *    <!!!>    Returns number of entries in src1 set to zero.
 * This one updates src1 in the meet with src2
 *
 * Be aware that result (src1) could contain zero-valued entries. Perhaps you
 * know it cannot e.g. because fltAdd was used on vectors with only positive
 * entries, otherwise consider the possibility and what needs to be done.
 * Rationale: it can be used for efficient bookkeeping, with a starting vector
 * full of zeroes.
*/

dim mclvUpdateMeet
(  mclVector*  src1
,  const mclVector*  src2
,  double           (*operation)(pval val1, pval val2)
)  ;

dim mclvUpdateDiff
(  mclVector*  v1
,  const mclVector*  v2
,  double  (*op)(pval mval, pval nval)
)  ;

long mclvUnaryList
(  mclv*    mx
,  mclpAR*  ar       /* idx: MCLX_UNARY_mode, val: arg */
)  ;

dim mclvCountGiven
(  mclVector*     src
,  mcxbool       (*operation)(mclIvp* ivp, void* arg)
,  void*          arg
)  ;


/*
 *    src may equal dst.
 *    all ivps are copied/retained for which operation
 *    returns true.
*/

mclVector* mclvCopyGiven
(  mclVector*     dst
,  mclVector*     src
,  mcxbool        (*operation)(mclIvp* ivp, void* arg)
,  void*          arg
,  dim            size_upper_bound  /* 0 for I-don't-know */
)  ;


mclVector* mclvAdd
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*  dst
)  ;  


void mclvScale
(  mclVector*     vec
,  double         fac
)  ;


double mclvNormalize
(  mclVector*     vec
)  ;


/* Returns powsum^(1/(power-1)) */

double mclvInflate
(  mclVector*     vec
,  double         power
)  ;

void mclvMakeConstant
(  mclVector*     vec
,  double         val
)  ;

void mclvZeroValues
(  mclVector*  vec
)  ;  

void mclvMakeCharacteristic
(  mclVector*     vec
)  ;


void mclvHdp
(  mclVector*     vec
,  double         pow
)  ;


long mclvHighestIdx
(  mclVector*     vec
)  ;


void mclvRemoveIdx
(  mclVector*     vec
,  long           idx
)  ;


      /* returns new vector with NULL vec argument */
mclVector* mclvInsertIdx
(  mclVector*     vec
,  long           idx
,  double         val
)  ;

mclVector* mclvInsertIvp
(  mclVector*  vec
,  long        idx
,  mclp**      ivpp
)  ;  


/* Efficient replacement of an index.
 * The index to be removed is specified by its ofset and the index
 * to be inserted must not be present already, lest STATUS_FAIL
 * is returned.
 * The vector is reordered to be in canonical order.
 * If !vec or ofs >= vec->n_ivps STATUS_FAIL is returned.
*/

mcxstatus mclvReplaceIdx
(  mclVector*     vec
,  long           ofs
,  long           idx
,  double         val
)  ;


/* inner product */

double mclvIn
(  const mclVector*        lft
,  const mclVector*        rgt
)  ;


mclVector* mcldMinus
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*        dst      /* value from lft (naturally) */
)  ;


mclVector* mcldMerge
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*        dst      /* values in lft prefered over rgt */
)  ;


   /* simple generic implementation */
mclVector* mcldMeet
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*        dst      /* values in dst are from lft */
)  ;

   /* this optimizes various scenarios at cost of increased code complexity */
mclVector* mcldMeet2
(  const mclVector*  lft
,  const mclVector*  rgt
,  mclVector*        dst      /* values in dst are from lft */
)  ;


mcxbool mcldIsCanonical
(  mclVector* vec
)  ;

               /*    l: size of left difference
                *    r: size of right difference
                *    m: size of meet.
               */
enum
{  MCLD_EQT_SUPER           /*    r == 0                     */
,  MCLD_EQT_SUB             /*    l == 0                     */
,  MCLD_EQT_EQUAL           /*    r == 0 && l == 0           */
,  MCLD_EQT_DISJOINT        /*    m == 0                     */
,  MCLD_EQT_MEET            /*    m != 0                     */
,  MCLD_EQT_TRISPHERE       /*    l != 0 && m != 0 && r != 0 */ 
,  MCLD_EQT_LDIFF           /*    l != 0                     */
,  MCLD_EQT_RDIFF           /*    r != 0                     */
}  ;

mcxbool mcldEquate
(  const mclVector* dom1
,  const mclVector* dom2
,  mcxenum    mode
)  ;

#define MCLD_EQUAL(d1, d2) mcldEquate((d1), (d2), MCLD_EQT_EQUAL)
#define MCLD_SUPER(d1, d2) mcldEquate((d1), (d2), MCLD_EQT_SUPER)
#define MCLD_SUB(d1, d2) mcldEquate((d1), (d2), MCLD_EQT_SUB)
#define MCLD_DISJOINT(d1, d2) mcldEquate((d1), (d2), MCLD_EQT_DISJOINT)


dim mcldCountParts
(  const mclVector* dom1
,  const mclVector* dom2
,  dim*       ldif
,  dim*       meet
,  dim*       rdif
)  ;


#define MCLD_CT_LDIFF 1
#define MCLD_CT_MEET  2
#define MCLD_CT_RDIFF 4
#define MCLD_CT_JOIN  7

dim mcldCountSet
(  const mclVector*  dom1
,  const mclVector*  dom2
,  mcxbits     parts
)  ;


double mclvSum
(  const mclVector*   vec
)  ;


void mclvMean
(  const mclv*    vec
,  dim            N           /* vec does/might not store zeroes */
,  double        *mean
,  double        *stddev
)  ;


void mclvAffine
(  mclv* vec
,  double mean
,  double stddev
)  ;


double mclvSelf
(  const mclVector*   vec
)  ;


double mclvSize
(  const mclVector*   vec
)  ;


double mclvVal
(  const mclVector*   vec
)  ;


double mclvSelf
(  const mclVector* vec
)  ;  


double mclvPowSum
(  const mclVector*   vec
,  double             power
)  ;


double mclvNorm
(  const mclVector*   vec
,  double             power
)  ;


double mclvHasLoop
(  const mclVector*   vec
)  ;


double mclvMaxValue
(  const mclVector*   vec
)  ;


double mclvMinValue
(  const mclVector*   vec
)  ;


/**********************************************************************
 * *
 **      Some get routines.
*/

mclIvp* mclvGetIvp
(  const mclv*    vec
,  long           idx
,  const mclp*    offset
)  ;

ofs mclvGetIvpOffset
(  const mclv*    vec
,  long           idx
,  ofs            offset
)  ;

double mclvIdxVal
(  const mclv*    vec
,  long           idx
,  ofs*           p_offset
)  ;

   /*
    * ceil is smallest ivp for which ivp.idx >= idx
    * NULL if for all ivp in vec ivp.idx < idx
    * (result is minimal ceil to idx)
   */
mclIvp* mclvGetIvpCeil
(  const mclVector*  vec
,  long              idx
,  const mclIvp*     offset
)  ;

   /*
    * floor is largest ivp for which ivp.idx <= idx,
    * NULL if for all ivp in vec ivp.idx > idx
    * (result is maximal floor to idx)
   */
mclIvp* mclvGetIvpFloor
(  const mclVector*  vec
,  long              idx
,  const mclIvp*     offset
)  ;



/**********************************************************************
 * *
 **      Some cmp routines.
*/

/* looks first at size, then at lexicographic ordering, ignores vid. */

int mclvSizeCmp
(  const void*  p1
,  const void*  p2
)  ;

int mclvSizeRevCmp
(  const void*  p1
,  const void*  p2
)  ;


/* looks at lexicographic ordering, ignores vid. */

int mclvLexCmp
(  const void*  p1
,  const void*  p2
)  ;


/* looks at vid only. */

int mclvVidCmp
(  const void*  p1
,  const void*  p2
)  ;

int mclvValCmp
(  const void*  p1
,  const void*  p2
)  ;

int mclvValRevCmp
(  const void*  p1
,  const void*  p2
)  ;

int mclvSumCmp
(  const void*  p1
,  const void*  p2
)  ;


int mclvSumRevCmp
(  const void*  p1
,  const void*  p2
)  ;


   /* modes:
    *    1     vid
    *    2     values
    *    4     closing '$'
   */
void mclvSprintf
(  mcxTing* scr
,  mclv* vec
,  int valdigits
,  mcxbits modes
)  ;


#endif

