/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013, 2014  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#ifdef _GNU_SOURCE
#include <sched.h>
#endif

#include "compose.h"
#include "edge.h"
#include "matrix.h"
#include "vector.h"
#include "iface.h"
#include "io.h"

#include "util/compile.h"
#include "util/array.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/err.h"
#include "util/io.h"
#include "util/tok.h"



/*

mapping:
   mclv* mclxMapVectorPermute
      mcxstatus mclxMapCols
      mcxstatus  mclxMapRows
      mcxbool mclxMapTest

allocation / new / copy / free / release:
   mclx* mclxAllocClone
   mclx* mclxAllocZero
   mclx* mclxCartesian
   mclx* mclxCopy
   static void mclx_release
   mclx* mclxCollectVectors
   void mclxTransplant
   void mclxFree

domain subselection:
   static mcxstatus meet_the_joneses
   mclx*  mclxExtSub
   mclx*  mclxSub
   void mclxAccommodate
   void mclxChangeDomains
   void mclxChangeRDomain
   void mclxChangeCDomain
   mclx*  mclxBlocksC
   mclx*  mclxBlockUnion
   mclx*  mclxBlockUnion2
   void mclxScrub

diagonal
   mclx* mclxConstDiag
   void mclxScaleDiag
   mclx* mclxDiag
   mclx* mclxIdentity

value subselection / transformation:
   double mclxSelectValues
   void mclxUnary
   dim mclxUnaryList
   void mclxMakeStochastic
   void mclxMakeCharacteristic

node degree selection:
   mclv* mclgUnlinkNodes
   mclv* mclgCeilNB
   void mclgKNNdispatch
   void mclgKNN
   dim mclxSelectLower
   dim mclxSelectUpper
   static void* mclx_vector_thread
   mcxstatus mclxVectorDispatch
   mcxstatus mclxVectorDispatchGroup

traits:
   mclv* mclxColSelect
   mclv* mclxColSums
   mclv* mclxColNums
   mclv* mclxDiagValues
   double mclxMass
   dim mclxNrofEntries
   double mclxMaxValue

matrix merge / add / collect:
   void mclxMerge
   void mclxAppendVectors
   void mclxAugment
   mclx* mclxBinary
   mclx* mclxTranspose
   mclx* mclxMakeMap
   mclx* mclxMax
   mclx* mclxMinus
   mclx* mclxAdd
   void mclxAddTranspose
   mclx* mclxHadamard
   mclv* mclgUnionv
   mclv* mclgUnionv2

matrix surgery:
   void  mclxColumnsRealign

get vectors:
   ofs mclxGetVectorOffset
   mclv* mclxGetNextVector
   mclv* mclxGetVector

loops:
   double mclxLoopCBifEmpty
   double mclxLoopCBremove
   double mclxLoopCBsum
   double mclxLoopCBmax
   dim mclxAdjustLoops

*/

/* helper function */

static mcxbool is_identity_map
(  mclx  *map
)
   {  dim d
   ;  for (d=0;d<N_COLS(map);d++)
      {  if (map->cols[d].n_ivps != 1)
         return FALSE
      ;  if (map->cols[d].ivps[0].idx != map->dom_cols->ivps[d].idx)
         return FALSE
   ;  }
      return TRUE
;  }


mclv* mclxMapVectorPermute
(  mclv  *dom
,  mclx  *map
,  mclpAR** ar_dompp
)
   {  mclpAR* ar_dom       =  NULL
   ;  mclv* new_dom_cols   =  NULL
   ;  mcxstatus status     =  STATUS_FAIL
   ;  dim d
   ;  *ar_dompp            =  NULL

   ;  while (1)
      {  long ofs = -1
      ;  ar_dom = mclpARensure(NULL, dom->n_ivps)

      ;  for (d=0;d<dom->n_ivps;d++)
         {  if
            (  (  0
               >  (ofs = mclvGetIvpOffset(map->dom_cols, dom->ivps[d].idx, ofs))
               )
            || map->cols[ofs].n_ivps < 1
            )
            break
         ;  ar_dom->ivps[d].idx = map->cols[ofs].ivps[0].idx
         ;  ar_dom->n_ivps++
      ;  }
         if (d != dom->n_ivps)
         break

      ;  new_dom_cols = mclvFromIvps(NULL, ar_dom->ivps, ar_dom->n_ivps)

      ;  if (new_dom_cols->n_ivps != ar_dom->n_ivps)
         {  mcxErr("mclxMapCheck", "map is not bijective")
         ;  break
      ;  }
         *ar_dompp = ar_dom
      ;  status = STATUS_OK
      ;  break
   ;  }

      if (status)
      {  mclvFree(&new_dom_cols)
      ;  mclpARfree(&ar_dom)
      ;  mcxErr
         (  "mclxMapDomain"
         ,  "error occurred with %lux%lu map matrix"
         ,  (ulong) N_COLS(map)
         ,  (ulong) N_ROWS(map)
         )
   ;  }
      return new_dom_cols
;  }


mcxstatus mclxMapCols
(  mclx  *mx
,  mclx  *map
)
   {  mclv* new_dom_cols = NULL
   ;  mclpAR     *ar_dom = NULL
   ;  dim d

   ;  if (map && is_identity_map(map))
      return STATUS_OK

   ;  if (map)
      {  if (!mcldEquate(mx->dom_cols, map->dom_cols, MCLD_EQT_SUB))
         {  mcxErr("mclxMapCols", "matrix domain not included in map domain")
         ;  return STATUS_FAIL
      ;  }
         if (!(new_dom_cols = mclxMapVectorPermute(mx->dom_cols, map, &ar_dom)))
         return STATUS_FAIL
   ;  }
      else
      new_dom_cols = mclvCanonical(NULL, N_COLS(mx), 1.0)

   ;  for (d=0; d<N_COLS(mx); d++)
      mx->cols[d].vid = ar_dom ? ar_dom->ivps[d].idx : (ofs) d

   ;  if (ar_dom)
      qsort(mx->cols, N_COLS(mx), sizeof(mclv), mclvVidCmp)

   ;  mclvFree(&(mx->dom_cols))
   ;  mx->dom_cols = new_dom_cols
   ;  mclpARfree(&ar_dom)

   ;  return STATUS_OK
;  }


mcxstatus  mclxMapRows
(  mclx  *mx
,  mclx  *map
)
   {  mclv* new_dom_rows
   ;  mclv* vec = mx->cols
   ;  mclpAR* ar_dom = NULL
   ;  mcxbool canonical = mclxRowCanonical(mx)

   ;  if (map && is_identity_map(map))
      return STATUS_OK

   ;  if (map)
      {  if (!mcldEquate(mx->dom_rows, map->dom_cols, MCLD_EQT_SUB))
         {  mcxErr("mclxMapRows", "matrix domain not included in map domain")
         ;  return STATUS_FAIL
      ;  }
         if (!(new_dom_rows = mclxMapVectorPermute(mx->dom_rows, map, &ar_dom)))
         return STATUS_FAIL
   ;  }
      else
      new_dom_rows = mclvCanonical(NULL, N_ROWS(mx), 1.0)

   ;  while (vec < mx->cols + N_COLS(mx))
      {  mclIvp* rowivp    =  vec->ivps
      ;  mclIvp* rowivpmax =  rowivp + vec->n_ivps
      ;  ofs offset = -1
      
      ;  while (rowivp < rowivpmax)
         {  offset  =   canonical
                     ?  rowivp->idx
                     :  mclvGetIvpOffset(mx->dom_rows, rowivp->idx, offset)
         ;  if (offset < 0)
               mcxErr
               (  "mclxMapRows PANIC"
               ,  "index <%lu> not in domain for <%lux%lu> matrix"
               ,  (ulong) rowivp->idx
               ,  (ulong) N_COLS(mx)
               ,  (ulong) N_ROWS(mx)
               )
            ,  mcxExit(1)
         ;  else
            rowivp->idx = ar_dom ? ar_dom->ivps[offset].idx : offset
         ;  rowivp++
      ;  }
         if (ar_dom)
         mclvSort(vec, mclpIdxCmp)
      ;  vec++
   ;  }
      
      mclvFree(&(mx->dom_rows))
   ;  mclpARfree(&ar_dom)
   ;  mx->dom_rows = new_dom_rows
   ;  return STATUS_OK
;  }


mcxbool mclxMapTest
(  mclx* map
)
   {  dim n_edges    =  mclxNrofEntries(map)
   ;  mclv* rowids   =     n_edges == N_COLS(map) && N_COLS(map) == N_ROWS(map)
                        ?  mclgUnionv(map, NULL, NULL, SCRATCH_READY, NULL)
                        :  NULL
   ;  mcxbool ok     =     rowids && rowids->n_ivps == N_COLS(map)
                        ?  TRUE
                        :  FALSE
   ;  if (rowids)
      mclvFree(&rowids)

   ;  return ok
;  }


void mclxInflate
(  mclx*   mx
,  double       power
)
   {  mclv*     vecPtr          =     mx->cols
   ;  mclv*     vecPtrMax       =     vecPtr + N_COLS(mx)

   ;  while (vecPtr < vecPtrMax)
      {  mclvInflate(vecPtr, power)
      ;  vecPtr++
   ;  }
   }


mclx* mclxAllocClone
(  const mclx* mx
)
   {  mclv* dom_cols, *dom_rows
   ;  if (!mx)
      {  mcxErr("mclxAllocClone PBD", "void matrix argument")
      ;  return NULL
   ;  }

      dom_cols = mclvClone(mx->dom_cols)
   ;  dom_rows = mclvClone(mx->dom_rows)

   ;  if (!dom_cols || !dom_rows)
      return NULL

   ;  return mclxAllocZero(dom_cols, dom_rows)
;  }



mclx* mclxAllocZero
(  mclv * dom_cols
,  mclv * dom_rows
)
   {  dim d, n_cols
   ;  mclx *dst
   ;  const char* me = "mclxAllocZero"

   ;  if (!dom_cols || !dom_rows)
      {  mcxErr(me, "got NULL arguments (allocation error?)")
      ;  return NULL
   ;  }

      n_cols  = dom_cols->n_ivps
   
   ;  dst = mcxAlloc(sizeof(mclx), EXIT_ON_FAIL)
   ;  dst->cols = mcxAlloc (n_cols * sizeof(mclv), EXIT_ON_FAIL)

   ;  dst->dom_cols  =  dom_cols
   ;  dst->dom_rows  =  dom_rows

   ;  for (d=0; d<n_cols; d++)
      {  mclv* col   =  dst->cols+d
      ;  col->vid    =  dom_cols->ivps[d].idx
      ;  col->ivps   =  NULL
      ;  col->val    =  0.0
      ;  col->n_ivps =  0
   ;  }

      return dst
;  }


mclx* mclxCartesian
(  mclv*     dom_cols
,  mclv*     dom_rows
,  double         val
)
   {  dim d
   ;  mclx*  rect  =  mclxAllocZero(dom_cols, dom_rows)

   ;  for(d=0;d<N_COLS(rect);d++)
      {  mclvCopy(rect->cols+d, dom_rows)
      ;  mclvMakeConstant(rect->cols+d, val)
   ;  }
      return rect
;  }


/* If row_segment == NULL columns will be empty.
 * If row_segment subsumes src->dom_rows columns are copied.
*/

static mcxstatus meet_the_joneses
(  mclx* dst
,  const mclx* src
,  const mclv* col_segment    /* pick these columns from src */
,  const mclv* row_segment    /* and these rows              */
)
   {  const mclv* col_select =  col_segment ? col_segment : src->dom_cols
   ;  mclp* selivp      =  col_select->ivps
   ;  mclp* selivpmax   =  selivp + col_select->n_ivps
   ;  mclv* dstvec      =  NULL, *srcvec = NULL

   ;  mcxbool copyrow   =  row_segment && MCLD_SUPER(row_segment, src->dom_rows)

   ;  while (selivp<selivpmax)
      {  dstvec  =  mclxGetVector(dst, selivp->idx, RETURN_ON_FAIL, dstvec)
      ;  srcvec  =  mclxGetVector(src, selivp->idx, RETURN_ON_FAIL, srcvec)

      ;  if (!dstvec)
         {  mcxErr
            (  "mclxSelect panic"
            ,  "corruption in submatrix - vector %u not found among %u entries"
            ,  (unsigned) selivp->idx
            ,  (unsigned) col_select->n_ivps
            )
         ;  return STATUS_FAIL
      ;  }

         if (srcvec)
         {  if (copyrow)
            mclvCopy(dstvec, srcvec)
         ;  else if (row_segment)
            mcldMeet(srcvec, row_segment, dstvec)
         ;  srcvec++
      ;  }
         selivp++
      ;  dstvec++
   ;  }
      return STATUS_OK
;  }


mclx*  mclxExtSub
(  const mclx*  mx
,  const mclv*  col_select
,  const mclv*  row_select
)
   {  mclv *new_dom_cols, *new_dom_rows, *colc_select = NULL
   ;  mcxstatus status = STATUS_FAIL
   ;  mclx* sub = NULL

   ;  if (!col_select)
      col_select = mx->dom_cols
   ;  if (!row_select)
      row_select = mx->dom_rows

   ;  colc_select = mcldMinus(mx->dom_cols, col_select, NULL)

   ;  new_dom_cols = mclvClone(mx->dom_cols)
   ;  new_dom_rows = mclvClone(mx->dom_rows)

   ;  if (!(sub = mclxAllocZero(new_dom_cols, new_dom_rows)))
      return NULL

   ;  while (1)
      {  if (meet_the_joneses(sub, mx, colc_select, row_select))
         break

      ;  if (meet_the_joneses(sub, mx, col_select, new_dom_rows))
         break

      ;  status = STATUS_OK
      ;  break
   ;  }

      mclvFree(&colc_select)

   ;  if (status)
      mclxFree(&sub)

   ;  return sub
;  }


void mclxReduce
(  mclx*  mx
,  const mclv*  col_select
,  const mclv*  row_select
)
   {  dim i
   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  mcxbool keep_column = !col_select || mclvGetIvp(col_select, v->vid, NULL)
      ;  if (!keep_column)
         mclvResize(v, 0)
      ;  else if (row_select)
         mcldMeet(v, row_select, v)
   ;  }
;  }


mclx*  mclxSub
(  const mclx*  mx
,  const mclv*  col_select
,  const mclv*  row_select
)
   {  mclv *new_dom_cols, *new_dom_rows
   ;  mclx*  sub = NULL

   ;  new_dom_cols = col_select ? mclvClone(col_select) : mclvInit(NULL)
   ;  new_dom_rows = row_select ? mclvClone(row_select) : mclvInit(NULL)

   ;  if (!(sub = mclxAllocZero(new_dom_cols, new_dom_rows)))
      return NULL

   ;  if (meet_the_joneses(sub, mx, new_dom_cols, new_dom_rows))
      mclxFree(&sub)
                  /* noteme dangersign recentchange; 
                   * previously we called 
               meet_the_joneses(sub, mx, col_select, row_select)
                   * but if !col_select the sub argument would have
                   * new_dom_cols equal to empty vector;
                   * meet_the_joneses would interpret the NULL col_select
                   * as the full domain, and hence croak
                  */
   ;  return sub
;  }


dim mclxSelectLower
(  mclx*  mx
)
   {  dim d, n_entries = 0
   ;  for (d=0;d<N_COLS(mx);d++)
      n_entries
      += mclvSelectIdcs
         (mx->cols+d, NULL, &(mx->cols[d].vid), MCLX_EQT_LT, mx->cols+d)
   ;  return n_entries
;  }


dim mclxSelectUpper
(  mclx*  mx
)
   {  dim d, n_entries = 0
   ;  for (d=0;d<N_COLS(mx);d++)
      n_entries
      +=    mclvSelectIdcs
            (mx->cols+d, &(mx->cols[d].vid), NULL, MCLX_EQT_GT, mx->cols+d)
   ;  return n_entries
;  }


double mclxSelectValues
(  mclx*  mx
,  double*     lft
,  double*     rgt
,  mcxbits     equate
)
   {  dim d
   ;  double sum = 0.0
   ;  for (d=0;d<N_COLS(mx);d++)
      sum += mclvSelectValues(mx->cols+d, lft, rgt, equate, mx->cols+d)
   ;  return sum
;  }


mclx* mclxConstDiag
(  mclv* vec
,  double c
)
   {  mclx*  m = mclxDiag(vec)
   ;  mclxUnary(m, fltxConst, &c)
   ;  return m
;  }


void mclxScaleDiag
(  mclx* mx
,  double fac
)
   {  dim d
   ;  for(d=0;d<N_COLS(mx);d++)
      {  mclv* vec = mx->cols+d
      ;  mclp* self = mclvGetIvp(vec, vec->vid, NULL)
      ;  if (self)
         self->val *= fac
   ;  }
   }


mclx* mclxDiag
(  mclv* vec
)
   {  mclx* mx = mclxAllocZero(vec, mclvCopy(NULL, vec))
   ;  dim d

   ;  if (!mx)
      return NULL

   ;  for(d=0;d<N_COLS(mx);d++)
      mclvInsertIdx(mx->cols+d, vec->ivps[d].idx, vec->ivps[d].val)
      /* fixme; this might fail */
   ;  return mx
;  }


mclx* mclxCopy
(  const mclx*     src
)
                               /* pbb sufficiently efficient */
   {  return mclxSub(src, src->dom_cols, src->dom_rows)
;  }


static void mclx_release
(  mclx* mx
)
   {  mclv* vec =  mx->cols
   ;  dim n_cols = N_COLS(mx)

   ;  while (n_cols-- > 0)    /* careful with unsignedness */
      {  mcxFree(vec->ivps)
      ;  vec++
   ;  }
      mclvFree(&(mx->dom_rows))
   ;  mclvFree(&(mx->dom_cols))
   ;  mcxFree(mx->cols)
;  }


void mclxTransplant
(  mclx* dst
,  mclx** src      /* will be freed */
)
   {  mclx_release(dst)
   ;  dst->dom_rows = src[0]->dom_rows
   ;  dst->dom_cols = src[0]->dom_cols
   ;  dst->cols = src[0]->cols
   ;  mcxFree(*src)
   ;  *src = NULL
;  }


void mclxFree
(  mclx**             mxpp
)
   {  mclx* mx = *mxpp
   ;  if (mx)
      {  mclx_release(mx)
      ;  mcxFree(mx)
      ;  *mxpp = NULL
   ;  }
   }


void mclxMakeStochastic
(  mclx* mx
)  
   {  mclv* vecPtr    =  mx->cols
   ;  mclv* vecPtrMax =  vecPtr + N_COLS(mx)

   ;  while (vecPtr < vecPtrMax)
         mclvNormalize(vecPtr)
      ,  vecPtr++
;  }



mclv* mclxColSelect
(  const mclx*  m
,  double          (*f_cb)(const mclv*, void*)
,  void*             arg_cb
)
   {  mclv*  sel = mclvClone(m->dom_cols)
   ;  dim i =  0
   
   ;  while (i < N_COLS(m))
      {  sel->ivps[i].val = f_cb(m->cols + i, arg_cb)
      ;  i++
   ;  }

      mclvUnary(sel, fltxCopy, NULL)
   ;  return sel
;  }


struct sparse_sel
{  dim sel_gq
;  dim sel_lq
;
}  ;


double sparse_sel_cb
(  const mclv* vec
,  void* data
)
   {  struct sparse_sel* sel = data
   ;  return
      (  (sel->sel_gq && vec->n_ivps < sel->sel_gq)
      || (sel->sel_lq && vec->n_ivps > sel->sel_lq)
      )
   ?  0.0 : 1
;  }


mclv* mclgUnlinkNodes
(  mclx* m
,  dim        sel_gq
,  dim        sel_lq
)
   {  struct sparse_sel values
   ;  mclv* sel   = NULL
   ;  mclp* p = NULL
   ;  dim d

   ;  values.sel_gq = sel_gq
   ;  values.sel_lq = sel_lq

   ;  sel = mclxColSelect(m, sparse_sel_cb, &values)

   ;  for (d=0;d<N_COLS(m);d++)
      {  if (!(p = mclvGetIvp(sel, m->cols[d].vid, p)))
         mclvResize(m->cols+d, 0)
      ;  else
         mcldMeet(m->cols+d, sel, m->cols+d)
   ;  }
      return sel
;  }


mclv* mclgCeilNB
(  mclx* mx
,  dim max_neighbours
,  dim* n_nodes_hub
,  dim* n_edges_in
,  dim* n_edges_out
)
   {  dim i, j, n_sel = 0, n_out = 0, n_in = 0, n_hub = 0
   ;  mclv* sizes = mclxColSizes(mx, MCL_VECTOR_SPARSE)
   ;  mclvSelectGtBar(sizes, (double) max_neighbours + 0.5)
   ;  mclvSortDescVal(sizes)           /* dangersign no longer in canonical ordering required for most operations */
   ;  for (i=0;i<sizes->n_ivps;i++)
      {  long idx1 = sizes->ivps[i].idx, hub_size = 0
      ;  mclv* hub = mclxGetVector(mx, idx1, RETURN_ON_FAIL, NULL)
      ;  mclv* discarded = mclvCopy(NULL, hub)

      ;  if (!hub)      /* fixme this SNHappen. */
         break

      ;  n_hub++
      ;  hub_size = hub->n_ivps

      ;  mclvSelectHighest(hub, max_neighbours)
      ;  n_sel++

      ;  mcldMinus(discarded, hub, discarded)
      ;  n_out += discarded->n_ivps

      ;  sizes->ivps[i].val = discarded->n_ivps + 0.5    /* n_ivps could be 0 */

      ;  for (j=0;j<discarded->n_ivps;j++)
         {  long idx2 = discarded->ivps[j].idx
         ;  mclv* tgt = mclxGetVector(mx, idx2, RETURN_ON_FAIL, NULL)
         ;  if (tgt)
               mclvRemoveIdx(tgt, idx1)
            ,  n_in++
      ;  }
         if (discarded->n_ivps)
         mcxLog
         (  MCX_LOG_CELL
         ,  "mclg"
         ,  "trample hub %lu size %lu removed %lu"
         ,  (ulong) idx1
         ,  (ulong) hub_size
         ,  (ulong) discarded->n_ivps
         )
      ;  mclvFree(&discarded)
   ;  }
      if (n_edges_out)
      n_edges_out[0] = n_out
   ;  if (n_edges_in)
      n_edges_in[0] = n_in
   ;  if (n_nodes_hub)
      n_nodes_hub[0] = n_hub
   ;  mclvSort(sizes, NULL)
   ;  return sizes
;  }


void mclxUnary
(  mclx*  src
,  double (*op)(pval, void*)
,  void* arg
)
   {  dim         n_cols =  N_COLS(src)
   ;  mclv*  vec    =  src->cols

   ;  while (n_cols-- > 0)    /* careful with unsignedness */
         mclvUnary(vec, op, arg)
      ,  vec++
;  }


void mclxAccommodate
(  mclx* mx
,  const mclv* dom_cols
,  const mclv* dom_rows
)
   {  if (dom_cols && !mcldEquate(mx->dom_cols, dom_cols, MCLD_EQT_SUPER))
      mclxChangeCDomain(mx, mcldMerge(mx->dom_cols, dom_cols, NULL))
   ;  if (dom_rows && !mcldEquate(mx->dom_rows, dom_rows, MCLD_EQT_SUPER))
      mclxChangeRDomain(mx, mcldMerge(mx->dom_rows, dom_rows, NULL))
;  }



void mclxChangeDomains
(  mclx* mx
,  mclv* dom_cols
,  mclv* dom_rows
)
   {  if (dom_cols)
      mclxChangeCDomain(mx, dom_cols)
   ;  if (dom_rows)
      mclxChangeRDomain(mx, dom_rows)
;  }


void mclxChangeRDomain
(  mclx* mx
,  mclv* domain
)
   {  dim d

   ;  if (mcldEquate(mx->dom_rows, domain, MCLD_EQT_LDIFF))
      {  for (d=0;d<N_COLS(mx);d++)
         mcldMeet(mx->cols+d, domain, mx->cols+d)
   ;  }
      mclvFree(&(mx->dom_rows))
   ;  mx->dom_rows = domain
;  }


void mclxChangeCDomain
(  mclx* mx
,  mclv* domain
)
   {  dim d
   ;  mclv* new_cols
   ;  mclv* cvec = mx->cols

   ;  if (mcldEquate(mx->dom_cols, domain, MCLD_EQT_EQUAL))
      {  mclvFree(&domain)
      ;  return
   ;  }

      new_cols =  mcxAlloc
                  (  domain->n_ivps * sizeof(mclv)
                  ,  EXIT_ON_FAIL
                  )

   ;  for (d=0;d<domain->n_ivps;d++)
      {  mclv* newcol=  new_cols+d
      ;  long vid    =  domain->ivps[d].idx
      ;  cvec        =  mclxGetVector(mx, vid, RETURN_ON_FAIL, cvec)

      ;  newcol->vid = vid
      ;  newcol->val = 0.0

      ;  if (cvec)
         {  newcol->ivps   =  cvec->ivps
         ;  newcol->n_ivps =  cvec->n_ivps
         ;  newcol->val    =  cvec->val
         ;  cvec->ivps     =  NULL
         ;  cvec->n_ivps   =  0
         ;  cvec++
      ;  }
         else
         {  newcol->ivps   =  NULL
         ;  newcol->n_ivps =  0
      ;  }
      }

   ;  for (d=0;d<N_COLS(mx);d++)
      mclvRelease(mx->cols+d)

   ;  mcxFree(mx->cols)
   ;  mx->cols = new_cols

   ;  mclvFree(&(mx->dom_cols))
   ;  mx->dom_cols = domain
;  }


mclx*  mclxBlocksC
(  const mclx*     mx
,  const mclx*     domain
)
   {  dim d
   ;  mclx* blocksc  =  mclxAllocClone(mx)

   ;  for (d=0;d<N_COLS(domain);d++)
      {  mclv* dom  =   domain->cols+d
      ;  ofs offset =  -1
      ;  dim e
      ;  for (e=0;e<dom->n_ivps;e++)
         {  long idx =  dom->ivps[e].idx
         ;  mclv* universe
         ;  offset = mclvGetIvpOffset(mx->dom_cols, idx, offset)
         ;  if (offset < 0)
            continue
         ;  universe =     blocksc->cols[offset].n_ivps
                        ?  blocksc->cols+offset
                        :  mx->cols+offset
         ;  mcldMinus(universe, dom, blocksc->cols+offset)
      ;  }
      }
   ;  return blocksc
;  }



/* implementation note:
 *    This may suggest mclvTernary(v1, v2, v3, f, g)
 *    Write f(v1[x], v2[x]) if g(v2[x], v3[x]).
 *    That would get rid of the temporary vector in the inner loop.
 *
 *    How useful would mclvTernary be otherwise?
*/

mclx*  mclxBlockUnion
(  const mclx*     mx       /* fixme; check domain equality ? */
,  const mclx*     domain
)
   {  dim d
   ;  mclv* meet     =  mclvInit(NULL)
   ;  mclx* blocks   =  mclxAllocClone(mx)

   ;  for (d=0;d<N_COLS(domain);d++)
      {  mclv* dom  =   domain->cols+d
      ;  ofs offset =  -1
      ;  dim e
      ;  for (e=0;e<dom->n_ivps;e++)
         {  long idx =  dom->ivps[e].idx
         ;  offset = mclvGetIvpOffset(mx->dom_cols, idx, offset)
         ;  if (offset < 0)
            continue
         ;  mcldMeet(mx->cols+offset, dom, meet)
         ;  mclvBinary(blocks->cols+offset, meet, blocks->cols+offset, fltLoR)
      ;  }
      }
      mclvFree(&meet)
   ;  return blocks
;  }


mclx*  mclxBlockPartition
(  const mclx*     mx
,  const mclx*     domain
,  int             quantile   /* only median supported for now */
)
   {  dim d
   ;  mclv* meet     =  mclvInit(NULL), *remove = mclvInit(NULL)
   ;  mclx* blocks   =  mclxAllocClone(mx)

   ;  for (d=0;d<N_COLS(domain);d++)
      {  mclv* dom  =   domain->cols+d
      ;  ofs offset =  -1
      ;  dim e

      ;  for (e=0;e<dom->n_ivps;e++)
         {  long idx =  dom->ivps[e].idx
         ;  double med = 0.0
         ;  long n = 0

         ;  offset = mclvGetIvpOffset(mx->dom_cols, idx, offset)
         ;  if (offset < 0)
            continue
         ;  mcldMeet(mx->cols+offset, dom, meet)

         ;  if (quantile)
            {  mcldMinus(mx->cols+offset, dom, remove)
            ;  if (remove->n_ivps)
               {  mclvSortAscVal(remove)
               ;  med
                  =  mcxMedian
                     (  remove->ivps
                     ,  remove->n_ivps
                     ,  sizeof remove->ivps[0]
                     ,  mclpGetDouble
                     ,  NULL
                     )
            ;  }
            }
            n = meet->n_ivps
         ;  if (med && meet->n_ivps && med < mclvMaxValue(meet))
            mclvSelectGtBar(meet, med)
;if(0)fprintf(stderr, "%d meet from %d to %d (rm %d)\n", (int) offset, (int) n, (int) meet->n_ivps, (int) remove->n_ivps)
         ;  mclvBinary(blocks->cols+offset, meet, blocks->cols+offset, fltLoR)
      ;  }
      }
      mclxMergeTranspose(blocks, fltMin, 0.5)
   ;  mclvFree(&meet)
   ;  mclvFree(&remove)
   ;  return blocks
;  }


mclx*  mclxBlockUnion2
(  const mclx*     mx
,  const mclx*     domain
)
   {  dim d
   ;  mclx* blocks  =   mclxAllocClone(mx)

   ;  for (d=0;d<N_COLS(domain);d++)
      {  mclv* dom = domain->cols+d
      ;  if (dom->n_ivps)
         {  mclx* sub = mclxSub(mx, dom, dom)
         ;  mclxMerge(blocks, sub, fltLoR)
         ;  mclxFree(&sub)
      ;  }
      }
      return blocks
;  }



/* TODO: allow m1 = NULL
 *    equate can be inefficient for block selection (mclxblock).
 * WARNING:
 *    does not check domains. use mclxAccommodate if necessary.
*/

void mclxMerge
(  mclx* m1
,  const mclx* m2
,  double  (*op)(pval, pval)
)
   {  mclv *m1vec = m1->cols
   ;  dim d, rdif = 0

   ;  if (mcldCountParts(m1->dom_rows, m2->dom_rows, NULL, NULL, &rdif))
      {  mcxErr
         (  "mclxMerge PBD"
         ,  "left domain (ct %ld) does not subsume right domain (ct %ld)"
         ,  (long) N_COLS(m2)
         ,  (long) N_COLS(m1)
         )
      ;  return
   ;  }

      for (d=0;d<N_COLS(m2);d++)
      {  mclv *m2vec = m2->cols+d

      ;  if (!(m1vec = mclxGetVector(m1, m2vec->vid, RETURN_ON_FAIL, m1vec)))
         continue

      ;  if (!mclvBinary(m1vec, m2vec, m1vec, op))
         break    /* fixme; should err, pbb not free */
   ;  }
   }


   /* Note: caller has to make sure vid is suitable. No checks are possible here.
   */
static mclx* mclx_collect_vectors
(  mclv* domain       /* allowed to be NULL; otherwise taken as row domain */
,  dim   vid          /* starting vid in new matrix */
,  va_list *ap
)
#define CAT_ACCEPT 16
   {  mclx* mx = mclxAllocZero(mclvCanonical(NULL, CAT_ACCEPT, 1.0), domain ? domain : mclvInit(NULL))
   ;  dim n_done = 0

   ;  mclvMap(mx->dom_cols, 0, vid, mx->dom_cols)     /* starts at vid now */
   ;  while (1)
      {  mclVector* vec
      ;  if (!(vec = va_arg(*ap, mclVector*)))
         break
      ;  if (n_done == CAT_ACCEPT)
         {  mcxErr("mclxCatVectors", "accepting %d vectors, ignoring the rest", (int) CAT_ACCEPT)
         ;  break
      ;  }
         if (domain)
         mcldMeet(vec, domain, mx->cols+n_done)
      ;  else
         {  mclvCopy(mx->cols+n_done, vec)
         ;  mcldMerge(mx->dom_rows, vec, mx->dom_rows)
      ;  }
         mx->cols[n_done].vid = vid++
      ;  n_done++
   ;  }
      mclvResize(mx->dom_cols, n_done)
   ;  mclvMakeCharacteristic(mx->dom_rows)
   ;  return mx
;  }


mclx* mclxCollectVectors
(  mclv* domain       /* allowed to be NULL; otherwise taken as row domain */
,  dim   vid          /* starting vid in new matrix */
,  ...                /* pointers to vectors to be copied */
)
   {  mclx* mx
   ;  va_list ap
   ;  va_start(ap, vid)
   ;  mx = mclx_collect_vectors(domain, vid, &ap)
   ;  va_end(ap)
   ;  return mx
;  }


void mclxAppendVectors
(  mclx* dst
,  ...
)
   {  mclx *mxadd
   ;  mclv* domain = mclvCopy(NULL, dst->dom_rows)
   ;  va_list ap

   ;  va_start(ap, dst)
   ;  mxadd = mclx_collect_vectors(domain, mclvHighestIdx(dst->dom_cols)+1, &ap)
   ;  va_end(ap)

   ;  mclxAugment(dst, mxadd, fltAdd)
   ;  mclxFree(&mxadd)
;  }


void mclxAugment
(  mclx* m1
,  const mclx* m2
,  double (*fltop)(pval, pval)
)
   {  mclv *m1vec
   ;  dim d, rdiff = 0

   ;  mclv* join_col = NULL
   ;  mclv* join_row = NULL

   ;  if (mcldCountParts(m1->dom_rows, m2->dom_rows, NULL, NULL, &rdiff))
      join_row = mcldMerge(m1->dom_rows, m2->dom_rows, NULL)
   ;  if (mcldCountParts(m1->dom_cols, m2->dom_cols, NULL, NULL, &rdiff))
      join_col = mcldMerge(m1->dom_cols, m2->dom_cols, NULL)

   ;  mclxAccommodate(m1, join_col, join_row)
   ;  m1vec = m1->cols        /* note, mclxAccommodate may change m1->cols */

   ;  for (d=0;d<N_COLS(m2);d++)
      {  mclv *m2vec = m2->cols+d

      ;  if
         (  !m2vec->n_ivps
         || !(m1vec = mclxGetVector(m1, m2vec->vid, RETURN_ON_FAIL, m1vec))
         )
         continue

      ;  if (mcldCountParts(m1vec, m2vec, NULL, NULL, &rdiff))
         mclvBinary(m1vec, m2vec, m1vec, fltop)
      ;  else
         mclvUpdateMeet(m1vec, m2vec, fltop)
   ;  }

      if (join_col) mclvFree(&join_col)
   ;  if (join_row) mclvFree(&join_row)
;  }


mclx* mclxBinary
(  const mclx* m1
,  const mclx* m2
,  double  (*op)(pval, pval)
)
   {  mclv *dom_rows     =  mcldMerge
                                 (  m1->dom_rows
                                 ,  m2->dom_rows
                                 ,  NULL
                                 )
   ;  mclv *dom_cols     =  mcldMerge
                                 (  m1->dom_cols
                                 ,  m2->dom_cols
                                 ,  NULL
                                 )
   ;  mclx*  m3          =  mclxAllocZero(dom_cols, dom_rows)
   ;  mclv  *dstvec      =  m3->cols 
   ;  mclv  *m1vec       =  m1->cols
   ;  mclv  *m2vec       =  m2->cols
   ;  mclv  empvec

   ;  mclvInit(&empvec)

   ;  while (dstvec < m3->cols + N_COLS(m3))
      {  m1vec = mclxGetVector(m1, dstvec->vid, RETURN_ON_FAIL, m1vec)
      ;  m2vec = mclxGetVector(m2, dstvec->vid, RETURN_ON_FAIL, m2vec)

      ;  if
         (  !mclvBinary
            (  m1vec ? m1vec : &empvec
            ,  m2vec ? m2vec : &empvec
            ,  dstvec
            ,  op
            )
         )
         {  mclxFree(&m3)
         ;  break
      ;  }
         dstvec++
      ;  if (m1vec)
         m1vec++
      ;  if (m2vec)
         m2vec++
   ;  }

      return m3
;  }


ofs mclxGetVectorOffset
(  const mclx* mx
,  long  vid
,  mcxOnFail ON_FAIL
,  ofs  offset
)
   {  mclv* vec =  mclxGetVector
                        (  mx
                        ,  vid
                        ,  ON_FAIL
                        ,  offset > 0 ? mx->cols+offset : NULL
                        )
   ;  return vec ? vec - mx->cols : -1
;  }


mclv* mclxGetNextVector
(  const mclx* mx
,  long   vid
,  mcxOnFail ON_FAIL
,  const mclv* offset
)
   {  const mclv* max =  mx->cols + N_COLS(mx)

   ;  if (!offset)
      offset = mx->cols

   ;  while (offset < max)
      {  if (offset->vid >= vid)
         break
      ;  else
         offset++
   ;  }
      if (offset >= max || offset->vid > vid)
      {  if (ON_FAIL == RETURN_ON_FAIL)
         return NULL
      ;  else
            mcxErr
            (  "mclxGetNextVector PBD"
            ,  "did not find vector <%ld> in <%lu,%lu> matrix"
            ,  (long) vid
            ,  (ulong) N_COLS(mx)
            ,  (ulong) N_ROWS(mx)
            )
         ,  mcxExit(1)
   ;  }
      else
      return (mclv*) offset
   ;  return NULL
;  }


inline mclp* mclxInsertIvp
(  mclx* mx
,  long   vidc
,  long   vidr
)
   {  mclp* p = NULL
   ;  mclv* v = mclxGetVector(mx, vidc, RETURN_ON_FAIL, NULL)
   ;  if (v)
      mclvInsertIvp(v, vidr, &p)
   ;  return p
;  }


inline mclp* mclgArcAddto
(  mclx* mx
,  long   vidc
,  long   vidr
,  double val
)
   {  mclp* p = mclxInsertIvp(mx, vidc, vidr)
   ;  if (p)
      p->val += val
   ;  return p
;  }


inline mclp* mclgArcAdd
(  mclx* mx
,  long   vidc
,  long   vidr
,  double val
)
   {  mclp* p = mclxInsertIvp(mx, vidc, vidr)
   ;  if (p)
      p->val = val
   ;  return p
;  }


inline ofs mclgEdgeAddto
(  mclx*    mx
,  long     a
,  long     b
,  double   val
)
   {  if (!mclgArcAddto(mx, a, b, val))
      return a
   ;  if (!mclgArcAddto(mx, b, a, val))
      return b
   ;  return -1
;  }


inline ofs mclgEdgeAdd
(  mclx* mx
,  long  a
,  long  b
,  double val
)
   {  if (!mclgArcAdd(mx, a, b, val))
      return a
   ;  if (!mclgArcAdd(mx, b, a, val))
      return b
   ;  return -1
;  }



mclv* mclxGetVector
(  const mclx* mx
,  long   vid
,  mcxOnFail ON_FAIL
,  const mclv* offset
)
   {  dim n_cols  =  N_COLS(mx)
   ;  mclv* found =  NULL

   ;  if
      (  !N_COLS(mx)
      || vid < 0
      || vid > mx->cols[n_cols-1].vid
      )
      found = NULL
   ;  else if (mx->cols[0].vid == 0 && mx->cols[n_cols-1].vid == (ofs) (n_cols-1))
      {  if (mx->cols[vid].vid == vid)
         found = mx->cols+vid
      ;  else
         found = NULL
   ;  }
      else if (offset && offset - mx->cols + 1 < N_COLS(mx) && offset[1].vid == vid)
      found = (mclv*) (offset+1)  /* const riddance */
   ;  else if (offset && offset->vid == vid)
      found = (mclv*) offset      /* const riddance */
   ;  else
      {  mclv keyvec
      ;  mclvInit(&keyvec)
      ;  keyvec.vid = vid

      ;  if (!offset)
         offset = mx->cols

      ;  n_cols -= (offset - mx->cols)
      ;  found =  bsearch
                  (  &keyvec
                  ,  offset
                  ,  n_cols
                  ,  sizeof(mclv)
                  ,  mclvVidCmp
                  )
   ;  }

      if (!found && ON_FAIL == EXIT_ON_FAIL)
         mcxErr
         (  "mclxGetVector PBD"
         ,  "did not find vector <%ld> in <%lu,%lu> matrix"
         ,  (long) vid
         ,  (ulong) N_COLS(mx)
         ,  (ulong) N_ROWS(mx)
         )
      ,  mcxExit(1)

   ;  return found
;  }


mclx* mclxMakeMap
(  mclv*  dom_cols
,  mclv*  new_dom_cols
)
   {  mclx* mx
   ;  dim d

   ;  if (dom_cols->n_ivps != new_dom_cols->n_ivps)
      return NULL

   ;  mx = mclxAllocZero(dom_cols, new_dom_cols)

   ;  for (d=0;d<N_COLS(mx);d++)
      mclvInsertIdx(mx->cols+d, new_dom_cols->ivps[d].idx, 1.0)

   ;  return mx
;  }


mclx* mclxTranspose2
(  const mclx*  m
,  int withzeroes
)
   {  mclx*   tp  =  mclxAllocZero
                     (  mclvCopy(NULL, m->dom_rows)
                     ,  mclvCopy(NULL, m->dom_cols)
                     )
   ;  const mclv*  mvec =  m->cols
   ;  mclv* tvec
   ;  dim i = N_COLS(m)
   ;

                           /*
                            * Pre-calculate sizes of destination columns
                            * fixme; if canonical domains do away with mclxGetVector.
                           */
      while (i-- > 0)            /* careful with unsignedness */
      {  dim   n_ivps  =  mvec->n_ivps
      ;  mclIvp*  ivp  =  mvec->ivps
      ;  tvec          =  tp->cols

      ;  while (n_ivps-- > 0)    /* careful with unsignedness */
         {  if (ivp->val || withzeroes)
            {  tvec = mclxGetVector(tp, ivp->idx, EXIT_ON_FAIL, tvec)
            ;  tvec->n_ivps++
         ;  }
            ivp++
         ;  tvec++               /* with luck we get immediate hit */
      ;  }
         mvec++
   ;  }

                           /* Allocate */
      tvec  =  tp->cols
   ;  i     =  N_COLS(tp)
   ;  while (i-- > 0)            /* careful with unsignedness */
      {  if (!mclvResize(tvec, tvec->n_ivps))
         {  mclxFree(&tp)
         ;  return 0
      ;  }
         tvec->n_ivps = 0        /* dirty: start over for write */
      ;  tvec++
   ;  }

                           /* Write */
      mvec     =  m->cols
   ;  while (mvec < m->cols+N_COLS(m))
      {  dim   n_ivps  =  mvec->n_ivps
      ;  mclIvp* ivp   =  mvec->ivps
      ;  tvec           =  tp->cols

      ;  while (n_ivps-- > 0)   /* careful with unsignedness */
         {  if (ivp->val || withzeroes)
            {  tvec = mclxGetVector(tp, ivp->idx, EXIT_ON_FAIL, tvec)
            ;  tvec->ivps[tvec->n_ivps].idx = mvec->vid
            ;  tvec->ivps[tvec->n_ivps].val = ivp->val
            ;  tvec->n_ivps++
         ;  }
            tvec++
         ;  ivp++
      ;  }
         mvec++
   ;  }

      return tp
;  }


mclx* mclxTranspose
(  const mclx*  m
)
   {  return mclxTranspose2(m, 0)
;  }


mclv* mclxRowSizes
(  const mclx* m
,  mcxenum mode  
)
   {  mclv* res = mclvClone(m->dom_rows)
   ;  dim i, j, n_err = 0
   ;  mclvMakeConstant(res, 0.0)
   ;  for (i=0;i<N_COLS(m);i++)
      {  mclv* c = m->cols+i
      ;  mclp* p = res->ivps+0
      ;  for (j=0;j<c->n_ivps;j++)
         {  ofs idx = c->ivps[j].idx
         ;  p = mclvGetIvp(res, idx, p)
         ;  if (p)
            p->val += 1.0
         ;  else if (!n_err)
               mcxErr("mclxRowSizes", "panic - %ld not found in result", (long) idx)
            ,  n_err++
      ;  }
      }

      if (mode == MCL_VECTOR_SPARSE)
      mclvUnary(res, fltxCopy, NULL)
   ;  return res
;  }


mclv* mclxColNums
(  const mclx*  m
,  double           (*f_cb)(const mclv * vec)
,  mcxenum           mode
)
   {  mclv*  nums = mclvClone(m->dom_cols)
   ;  dim i =  0
   
   ;  if (nums)
      {  while (i < N_COLS(m))
         {  nums->ivps[i].val = f_cb(m->cols + i)
         ;  i++
      ;  }
      }
      if (mode == MCL_VECTOR_SPARSE)
      mclvUnary(nums, fltxCopy, NULL)

   ;  return nums
;  }


mclv* mclxDiagValues
(  const mclx*  m
,  mcxenum     mode  
)
   {  return mclxColNums(m, mclvSelf, mode)
;  }

mclv* mclxColSums
(  const mclx*  m
,  mcxenum     mode  
)
   {  return mclxColNums(m, mclvSum, mode)
;  }

mclv* mclxPowColSums
(  const mclx*  m
,  unsigned    exp
,  mcxenum     mode  
)
   {  mclv* cs = mclxColNums(m, mclvSum, mode)
   ;  mclv* v  = mclvClone(cs)
   ;  dim i, e
   ;  for (e=1; e<exp; e++)   
      {  for (i=0;i<v->n_ivps;i++)
         {  mclv* xv = mclxGetVector(m, v->ivps[i].idx, RETURN_ON_FAIL, NULL)
         ;  v->ivps[i].val = xv ? mclvIn(xv, cs) : 0.0
      ;  }
         {  mclv* tmp = v ;  v = cs ;  cs = tmp ;  }
      }
      mclvFree(&v)
   ;  return cs
;  }


double mclxMass
(  const mclx*     m
)
   {  dim d
   ;  double  mass  =  0
   ;  for (d=0;d<N_COLS(m);d++)
      mass += mclvSum(m->cols+d)
   ;  return mass
;  }


dim mclxNrofEntries
(  const mclx*     m
)
   {  dim d
   ;  dim nr =  0
   ;  for (d=0;d<N_COLS(m);d++)
      nr += (m->cols+d)->n_ivps
   ;  return nr
;  }


void  mclxColumnsRealign
(  mclx* m
,  int (*cmp)(const void* vec1, const void* vec2)
)
   {  dim d
   ;  qsort(m->cols, N_COLS(m), sizeof(mclv), cmp)
   ;  for (d=0;d<m->dom_cols->n_ivps;d++)
      m->cols[d].vid = m->dom_cols->ivps[d].idx
;  }


double mclxMaxValue
(  const mclx*        mx
) 
   {  double max_val  =  0.0
   ;  mclxUnary((mclx*)mx, fltxPropagateMax, &max_val)
   ;  return max_val
;  }


mclx* mclxIdentity
(  mclv* vec
)  
   {  return mclxConstDiag(vec, 1.0)
;  }


void mclxZeroValues
(  mclx*          m
)
   {  int i
   ;  for (i=0;i<N_COLS(m);i++)
      mclvZeroValues(m->cols+i)
;  }


void mclxMakeCharacteristic
(  mclx*              mx
)  
   {  double one  =  1.0
   ;  mclxUnary(mx, fltxConst, &one)
;  }


mclx* mclxMax
(  const mclx*        m1
,  const mclx*        m2
)  
   {  return mclxBinary(m1, m2, fltMax)
;  }


mclx* mclxMinus
(  const mclx*        m1
,  const mclx*        m2
)  
   {  return mclxBinary(m1, m2, fltSubtract)
;  }


mclx* mclxAdd
(  const mclx*        m1
,  const mclx*        m2
)  
   {  return mclxBinary(m1, m2, fltAdd)
;  }


void mclxMergeColumn
(  mclx* mx
,  const mclv* vec
,  double (*op)(pval arg1, pval arg2)
)
   {  long vid = vec->vid
   ;  mclv* dest
   ;  if (vid < 0)
      vid = 1 + mclvHighestIdx(mx->dom_cols)

   ;  if (!mclxGetVector(mx, vid, RETURN_ON_FAIL, NULL))
      {  mclv* dom = mclvCopy(NULL, mx->dom_cols)
      ;  mclvInsertIdx(dom, vid, 1.0)
      ;  mclxAccommodate(mx, dom, NULL)
      ;  mclvFree(&dom)
   ;  }

      mclxAccommodate(mx, NULL, vec)
   ;  if ((dest = mclxGetVector(mx, vid, RETURN_ON_FAIL, NULL)))
      mclvBinary(dest, vec, dest, op)
;  }



/* TODO: what would be an efficient mechanism for optionally
 *    squeezing in a diagonal element around the mclvAdd call?  Perhaps the
 *    overalloc-one-ivp thing. But it adds significant complexity and ideally
 *    one would want to hide it in the allocator. Perhaps write a special
 *    purpose allocator for ivps.
*/

void mclxMergeTranspose
(  mclx* mx
,  double (*op)(pval arg1, pval arg2)
,  double diagweight
)
   {  dim d
   ;  mclx* mxt = mclxTranspose(mx)
   ;  mclv* mvec = NULL

   ;  mclxChangeDomains
      (  mx
      ,  mcldMerge(mx->dom_cols, mxt->dom_cols, NULL)
      ,  mcldMerge(mx->dom_rows, mxt->dom_rows, NULL)
      )

   ;  for (d=0;d<N_COLS(mxt);d++)
      {  long vid = mxt->dom_cols->ivps[d].idx
      ;  mvec = mclxGetVector(mx, vid, RETURN_ON_FAIL, mvec)
      ;  if (!mvec)
         {  mcxErr("mclxMergeTranspose panic", "no vector %ld in matrix", vid)
         ;  continue
      ;  }
         mclvBinary(mvec, mxt->cols+d, mvec, op)
      ;  mclvRelease(mxt->cols+d)
   ;  }

      if (diagweight != 1.0)
      mclxScaleDiag(mx, diagweight)
   ;  mclxFree(&mxt)
;  }



void mclxMergeTranspose3
(  mclx* mx
,  double (*op)(pval arg1, pval arg2, pval arg3)
,  double diagweight
,  double arg3
)
   {  dim d
   ;  mclx* mxt = mclxTranspose(mx)
   ;  mclv* mvec = NULL

   ;  mclxChangeDomains
      (  mx
      ,  mcldMerge(mx->dom_cols, mxt->dom_cols, NULL)
      ,  mcldMerge(mx->dom_rows, mxt->dom_rows, NULL)
      )

   ;  for (d=0;d<N_COLS(mxt);d++)
      {  long vid = mxt->dom_cols->ivps[d].idx
      ;  mvec = mclxGetVector(mx, vid, RETURN_ON_FAIL, mvec)
      ;  if (!mvec)
         {  mcxErr("mclxMergeTranspose panic", "no vector %ld in matrix", vid)
         ;  continue
      ;  }
         mclvBinaryx(mvec, mxt->cols+d, mvec, op, arg3)
      ;  mclvRelease(mxt->cols+d)
   ;  }

      if (diagweight != 1.0)
      mclxScaleDiag(mx, diagweight)
   ;  mclxFree(&mxt)
;  }



void mclxAddTranspose
(  mclx* mx
,  double diagweight
)
   {  mclxMergeTranspose(mx, fltAdd, diagweight)
;  }



mclx* mclxHadamard
(  const mclx*        m1
,  const mclx*        m2
)
   {  return mclxBinary(m1, m2, fltMultiply)
;  }


double mclxLoopCBset
(  mclv  *vec
,  long r_unused           cpl__unused
,  void* data
)
   {  return ((double*) data)[0]
;  }


double mclxLoopCBifEmpty
(  mclv  *vec
,  long r_unused           cpl__unused
,  void*data_unused        cpl__unused
)
   {  return vec->n_ivps ? 0.0 : 1.0
;  }


double mclxLoopCBremove
(  mclv  *  vec            cpl__unused
,  long     r_unused       cpl__unused
,  void  *  data_unused    cpl__unused
)
   {  return 0.0
;  }


double mclxLoopCBsum
(  mclv  *vec
,  long r_unused           cpl__unused
,  void*data_unused        cpl__unused
)
   {  double sum = mclvSum(vec)
   ;  if (vec->n_ivps && sum)
      return sum
   ;  return 1.0
;  }


double mclxLoopCBmax
(  mclv  *vec
,  long r_unused           cpl__unused
,  void*data_unused        cpl__unused
)
   {  double max = mclvMaxValue(vec)
   ;  if (vec->n_ivps && max)
      return max
   ;  return 1.0
;  }


dim mclxAdjustLoops
(  mclx*    mx
,  double  (*op)(mclv* vec, long r, void* data)
,  void*    data
)
   {  dim d, n_void = 0
   ;  for (d=0;d<N_COLS(mx);d++)
      {  mclv*    vec   =  mx->cols+d
      ;  mclp*    ivp   =  mclvGetIvp(vec, vec->vid, NULL)
      ;  double   val

      ;  if (ivp)
         ivp->val = 0.0

      ;  val = op(vec, vec->vid, data)

      ;  if (!vec->n_ivps)
         n_void++

      ;  if (ivp && !val)
            ivp->val = 0.0
         ,  mclvUnary(vec, fltxCopy, NULL)
      ;  else if (ivp && val)
         ivp->val = val
      ;  else if (!ivp && val)
         mclvInsertIdx(vec, vec->vid, val)
   ;  }
      return n_void
;  }


void mclxScrub
(  mclx* mx
,  mcxbits bits
)
   {  mclv* colselect
      =     bits & (MCLX_SCRUB_COLS | MCLX_SCRUB_GRAPH)
         ?  mclxColNums(mx, mclvSize, MCL_VECTOR_SPARSE)
         :  NULL

   ;  mclv* rowselect                     /* fixme, cheaper way? */
      =     bits & (MCLX_SCRUB_ROWS | MCLX_SCRUB_GRAPH)
         ?  mclgUnionv(mx, NULL, NULL, SCRATCH_DIRTY, NULL)
         :  NULL

   ;  if (bits & MCLX_SCRUB_GRAPH)
      {  mcldMerge(colselect, rowselect, colselect)
      ;  mclvCopy(rowselect, colselect)
   ;  }
      mclxChangeDomains(mx, colselect, rowselect)
;  }


dim mclxUnaryList
(  mclx*    mx
,  mclpAR*  ar       /* idx: MCLX_UNARY_mode, val: arg */
)
   {  dim n_cols  =  N_COLS(mx)
   ;  mclv* vec   =  mx->cols
   ;  dim n_entries_kept = 0

   ;  while (n_cols-- > 0)    /* careful with unsignedness */
         n_entries_kept += mclvUnaryList(vec, ar)
      ,  vec++
   ;  return n_entries_kept
;  }


static void sym_reduce_dispatch
(  mclx* mx
,  dim i
,  void* data        /* not needed here */
,  dim thread_id     /* not needed here */
)
   {  mclv* v = mx->cols+i
   ;  mclv* vrev = mx->cols
   ;  dim j
   ;  for (j=0; j<v->n_ivps;j++)
      {  vrev = mclxGetVector(mx, v->ivps[j].idx, RETURN_ON_FAIL, vrev)
      ;  if (!vrev || !mclvGetIvp(vrev, v->vid, NULL))
         v->ivps[j].val = 0.0
   ;  }
   }


void mclxSymReduceDispatch
(  mclx* mx
,  dim n_thread
)
   {  dim i

   ;  if (n_thread <= 1)
      for (i=0;i<N_COLS(mx);i++)
      sym_reduce_dispatch(mx, i, NULL, 0)

   ;  else
      mclxVectorDispatch(mx, NULL, n_thread, sym_reduce_dispatch, NULL)

   ;  mclxUnary(mx, fltxCopy, NULL)
;  }


void mclxSymReduce
(  mclx* mx
)
   {  mclxSymReduceDispatch(mx, 1)
;  }


               /* fixme; only works on pairs of connected nodes */
void mclxILS
(  mclx* mx
)
   {  dim i, j, k
   ;  if (!mclxGraphCanonical(mx))
      mcxErr("mclxILS", "input is not a graph or not in canonical format")
   ;  mclxAdjustLoops(mx, mclxLoopCBremove, NULL)

   ;  for (i=0; i< N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  long vidi = v->vid
      ;  for (j=0; j<v->n_ivps && v->ivps[j].idx <= vidi ;j++)
         {  mclv* vj = mx->cols+(v->ivps[j]).idx
         ;  double ils = 0
         ;  mclv* meet = mcldMeet(v, vj, NULL)
         ;  for (k=0; k<meet->n_ivps;k++)
            {  mclv* nb = mx->cols+(meet->ivps[k].idx)
            ;  if (nb->n_ivps > 1)
               ils += log(2) / log(nb->n_ivps)
         ;  }
            v->ivps[j].val = ils
         ;  mclvFree(&meet)               /* fixme inline iterator better */
      ;  }
      }
      mclxMergeTranspose(mx, fltMax, 0.0)
;  }



struct generic_arg
{  mclx* mx
;  dim   n_thread
;  dim   thread_id
;  dim   n_group
;  dim   group_id
;  const struct mclx_thread_map* map
;  void (*cb)(mclx* mx, dim i, void*data, dim thread_id)
;  void* data
;
}  ;


static void* mclx_vector_thread
(  void* arg
)
   {  struct generic_arg* garg = arg
   ;  mclx* mx = garg->mx
   ;  dim ti=garg->thread_id, nt = garg->n_thread, ng=garg->n_group, gi = garg->group_id, i
   ;  const char* policy = getenv("MCLX_THREAD_POLICY")
   ;  if (!policy)
      policy  = "spread"

      /* not implemented. Idea is to have caller specify explicit thread-node mapping */
   ;  if (garg->map)
      {
   ;  }
      
      else if (!strcmp(policy, "compact"))
      {  unsigned njobs = nt * ng
      ;  unsigned jobsize = N_COLS(mx) / njobs + (N_COLS(mx) % njobs != 0)
      ;  unsigned start = (gi * nt + ti) * jobsize
      ;  unsigned end   = start + jobsize
      ;  if (end > N_COLS(mx))             /* It may happen that start also >= N_COLS(mx) - that's fine */
         end = N_COLS(mx)
;if(0)fprintf(stderr, "@@ %d %d jobsize %d\n", (int) start, (int) end, (int) jobsize)
      ;  for (i=start; i<end; i++)
         {  dim thei = i
         ;  garg->cb(mx, thei, garg->data, ti)
      ;  }
      }


   /* Spread example:    nt=10 (threads)  ng=4 (groups/machines)
    * gi=0
    *     ti:  0     1     2     3     4     5     6     7     8     9
    * ----------------------------------------------------------------
    *          0     1     2     3     4     5     6     7     8     9
    *         40    41    42    43    44    45    46    47    48    49
    *         80    81    82    83    84    85    86    87    88    89
    *        120   121   122   123   124   125   126   127   128   129
    *        ...   ...   ...   ...   ...   ...   ...   ...   ...   ...
    *
    * gi=1
    *     ti:  0     1     2     3     4     5     6     7     8     9
    * ----------------------------------------------------------------
    *         10    11    12 |  13 |  14    15    16    17    18    19
    *         50    51    52 |  53 |  54    55    56    57    58    59
    *         90    91    92 |  93 |  94    95    96    97    98    99
    *        130   131   132 | 133 | 134   135   136   137   138   139
    *        ...   ...   ... | ... | ...   ...   ...   ...   ...   ...
    *
    * gi=2
    *     ti:  0     1     2     3     4     5     6     7     8     9
    * ----------------------------------------------------------------
    *         20    21    22    23    24    25    26    27    28    29
    *         60    61    62    63    64    65    66    67    68    69
    *        100   101   102   103   104   105   116   117   118   119
    *        140   141   142   143   144   145   146   147   148   149
    *        ...   ...   ...   ...   ...   ...   ...   ...   ...   ...
   */

      else
      for (i = gi * nt + ti; i < N_COLS(mx); i += ng * nt)
      {  dim thei = i
      ;  garg->cb(mx, thei, garg->data, ti)
;if(0)fprintf(stderr, "g i=%d thei=%d ngnt=%d ti=%d\n", (int) i, (int) thei, (int)  (ng * nt), (int) ti)
   ;  }

      return NULL
;  }


mcxstatus mclxVectorDispatch
(  mclx* mx
,  void* data
,  dim n_thread
,  void (*cb)(mclx* mx, dim i, void* data, dim thread_id)
,  const struct mclx_thread_map* map
)
   {  return mclxVectorDispatchGroup(mx, data, n_thread, cb, 1, 0, map)
;  }


mcxstatus mclxVectorDispatchGroup
(  mclx* mx
,  void* data           /* might be logically partitioned in n_thread elements */
,  dim n_thread
,  void (*cb)(mclx* mx, dim i, void* data, dim thread_id)
,  dim n_group
,  dim group_id
,  const struct mclx_thread_map* map
)
   {  pthread_t *yarn = mcxAlloc(n_thread * sizeof yarn[0], EXIT_ON_FAIL)
   ;  struct generic_arg* garg = mcxAlloc(n_thread * sizeof garg[0], EXIT_ON_FAIL)
   ;  pthread_attr_t  t_attr
   ;  dim thread_id = 0, t_spun = 0

#ifdef _GNU_SOURCE
   ;  cpu_set_t cpuset[512] = { 0 }
#endif

   ;  if (n_group == 0 || group_id >= n_group)
      {  mcxErr("mclxVectorDispatchGroup PBD", "wrong parameters")
      ;  return STATUS_FAIL
   ;  }

      if (!yarn || !garg)     /* memleak if yarn && !garg */
      return STATUS_FAIL

   ;  pthread_attr_init(&t_attr)
   ;

#ifdef _GNU_SOURCE
   ;  CPU_ZERO_S(512, cpuset)

   ;  if (pthread_attr_getaffinity_np(&t_attr, 512, cpuset))
      mcxErr("___", "not good")
   ;  else
      {  int i
      ;  mcxErr("___", "ok")
      ;  for (i=0;i<512;i++)
         if (CPU_ISSET_S(i, 512, cpuset))
         fprintf(stderr, "cpu %d is set\n", i)
   ;  }
#endif

      while (thread_id < n_thread)
      {  struct generic_arg* g = garg+thread_id
      ;  g->mx    =  mx
      ;  g->data  =  data
      ;  g->cb    =  cb
      ;  g->n_thread = n_thread
      ;  g->thread_id= thread_id
      ;  g->n_group  = n_group
      ;  g->map   =  map
      ;  g->group_id = group_id
      ;  if (pthread_create(yarn+thread_id, &t_attr, mclx_vector_thread, g))
         {  mcxErr("mclxVectorDispatchGroup", "error creating thread %d", (int) thread_id)
         ;  break
      ;  }
         thread_id++
   ;  }

      if ((t_spun = thread_id) == n_thread)
      for (thread_id=0; thread_id < n_thread; thread_id++)
      pthread_join(yarn[thread_id], NULL)
#if 0
,fprintf(stderr, "dispatch %d\n", (int) thread_id)
#endif

   ;  mcxFree(yarn)
   ;  mcxFree(garg)

   ;  return t_spun == n_thread ? STATUS_OK : STATUS_FAIL
;  }


static double mclv_sosq
(  const mclv* v
)
   {  return mclvPowSum(v, 2.0)
;  }


void  mclxPerturb
(  mclx* mx
,  double radius
,  mcxbits  modes
)
   {  dim i, j
   ;  mcxbool symmetric = modes & MCLX_PERTURB_SYMMETRIC
   ;  mcxbool randific = modes & MCLX_PERTURB_RAND
   ;  mcxbool corr = !randific || (modes & MCLX_PERTURB_CORR)
   ;  mclv* sosq   = corr ? mclxColNums(mx, mclv_sosq, MCL_VECTOR_COMPLETE) : NULL

   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  for (j=0;j<v->n_ivps;j++)
         {  mclp* p = v->ivps+j
         ;  double perturb, fraction = 0.0
         ;  mclv* w = NULL
         
         ;  if (symmetric && p->idx >= v->vid)
            break
         ;  w = mclxGetVector(mx, p->idx, RETURN_ON_FAIL, NULL)

         ;  if (randific)
            fraction = 2.0 * (0.5 - (rand() * 1.0 / RAND_MAX))
         ;  else if (corr && w)
            {  double nom = sqrt(sosq->ivps[w - mx->cols].val * sosq->ivps[v - mx->cols].val)
            ;  fraction = nom ? (mclvIn(w, v) / nom) : 0.0
         ;  }

            perturb =   1.0 +  PVAL_EPSILON *  radius * fraction
;if(0)fprintf(stderr, "perturb %d %d %.20g\n", (int) v->vid, (int) w->vid, (double) perturb)
         ;  p->val *= perturb
         ;  if (symmetric && w)
            mclvInsertIdx(w, v->vid, p->val)
      ;  }
      }
      if (sosq)
      mclvFree(&sosq)
;  }



dim mclxQuantiles
(  mclx* mx
,  double q          /* should be between 0.0 and 1.0 */
)
   {  dim i
   ;  if (q < 0)
      q = 0.0
   ;  else if (q > 1.0)
      q = 1.0
   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclvSelectHighest(mx->cols+i, (ulong) (q * mx->cols[i].n_ivps + 0.5))
   ;  }
      return 0
;  }


void mclxNormSelf
(  mclx* mx
)
   {  dim i
   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  if (v->n_ivps)
         {  mclp* p = mclvGetIvp(v, v->vid, NULL)
         ;  double m = p ? p->val : mclvMaxValue(v)
         ;  mclvScale(v, m)
      ;  }
      }
   }


void mclxFold
(  mclx* mx
,  mclx* dup
)
   {  dim i, n_meet
   ;  mclv* uniondup = mclgUnionv(dup, NULL, NULL, SCRATCH_READY, NULL)
   
   ;  if (!mclxIsGraph(mx))
      {  mcxErr("mclxFold", "not folding, domains not equal")
      ;  return
   ;  }
   
      for (i=0;i<N_COLS(dup);i++)
      {  mclv* ls = dup->cols+i
      ;  mclv* dst = mclxGetVector(mx, ls->vid, RETURN_ON_FAIL, NULL)
      ;  dim j
      ;  if (!dst)
         {  mcxErr("mclxFold", "vector %ld not found", (long) ls->vid)
         ;  continue
      ;  }
         for (j=0;j<ls->n_ivps;j++)
         {  mclv* src = mclxGetVector(mx, ls->ivps[j].idx, RETURN_ON_FAIL, NULL)
         ;  if (src)
            {  mclvBinary(dst, src, dst, fltMax)
            ,  mclvResize(src, 0)
         ;  }
      ;  }
      }

      for (i=0;i<N_COLS(mx);i++)
      if (mcldCountParts(mx->cols+i, uniondup, NULL, &n_meet, NULL))
      mcldMinus(mx->cols+i, uniondup, mx->cols+i)

   ;  mclxMergeTranspose(mx, fltMax, 1.0)
   ;  mclvFree(&uniondup)
;  }


ofs mclxGetClusterID
(  const mclx*    cltp
,  long           nid
,  mcxbool        always_succeed
)
   {  mcxbool found = FALSE
   ;  mclv* cls  = NULL

   ;  do
      {  if (nid < 0 || nid >= N_COLS(cltp))
         break

      ;  if (!(cls = mclxGetVector(cltp, nid, RETURN_ON_FAIL, NULL)))
         break

      ;  if (cls->n_ivps == 0)
         break

      ;  if (cls->ivps[0].idx < 0 || cls->ivps[0].idx >= N_ROWS(cltp))
         break

      ;  found = TRUE
   ;  }
      while(0)

   ;  return found ? cls->ivps[0].idx : always_succeed ? 0 : -1
;  }


