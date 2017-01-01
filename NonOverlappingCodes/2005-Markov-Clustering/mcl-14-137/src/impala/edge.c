/*   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#include <stdio.h>
#include <stdarg.h>

#include "util/err.h"
#include "edge.h"


mclv* mclgUnionv
(  mclx* mx
,  const mclv* coldom
,  const mclv* confine
,  mcxenum scratch_STATUS
,  mclv* dst
)
   {  return mclgUnionv2(mx, coldom, confine, scratch_STATUS, dst, mx->dom_rows)
;  }


mclv* mclgUnionv2
(  const mclx* mx
,  const mclv* coldom
,  const mclv* confine
,  mcxenum scratch_STATUS
,  mclv* dst
,  mclv* scratch
)
   {  const mclv* dvec = NULL
   ;  mclv *row_scratch = NULL
   ;  mcxbool canonical = mclxRowCanonical(mx)
   ;  mclpAR* par = mclpARensure(NULL, 256)
   ;  dim d

   ;  if (!dst)
      dst = mclvInit(dst)
   ;  else
      mclvResize(dst, 0)

   ;  if (!coldom)
      coldom = mx->dom_cols

   ;  if (scratch_STATUS == SCRATCH_BUSY)
      row_scratch = mclvClone(scratch)
   ;  else
      row_scratch = scratch
      
   ;  if (scratch_STATUS != SCRATCH_READY && scratch_STATUS != SCRATCH_UPDATE)
      mclvMakeCharacteristic(row_scratch)

   ;  for (d=0;d<coldom->n_ivps;d++)
      {  long idx = coldom->ivps[d].idx

      ;  if ((dvec = mclxGetVector(mx, idx, RETURN_ON_FAIL, dvec)))
         {  long o_scratch = -1, o_confine = -1
         ;  dim t
         ;  for (t=0; t<dvec->n_ivps; t++)
            {  long idx = dvec->ivps[t].idx
            ;  if
               (  0
               >  (  o_scratch
                  =     canonical
                     ?  idx
                     :  mclvGetIvpOffset(row_scratch, idx, o_scratch)
                  )
               )
               continue               /* SNH if coldom is subset of mx->dom_cols */
            ;  if
               (  confine
               && 0 > (o_confine = mclvGetIvpOffset(confine, idx, o_confine))
               )
               continue               /* not found in restriction domain */

            ;  if (row_scratch->ivps[o_scratch].val < MCLG_UNIONV_SENTINEL)
                  row_scratch->ivps[o_scratch].val = MCLG_UNIONV_SENTINEL + 0.5
               ,  mclpARextend(par, idx, 1.0)
         ;  }
         }
      }

   ;  mclvFromPAR
      (  dst
      ,  par
      ,  0
      ,  mclpMergeLeft
      ,  NULL
      )
   ;  mclpARfree(&par)

   ;  if (scratch_STATUS == SCRATCH_READY)
      {  long o = -1
      ;  for(d=0;d<dst->n_ivps;d++)
         {  o =   canonical
               ?  dst->ivps[d].idx
               :  mclvGetIvpOffset(mx->dom_rows, dst->ivps[d].idx, o)
         ;  row_scratch->ivps[o].val = 1.0
      ;  }
      }

      if (scratch_STATUS == SCRATCH_BUSY)
      mclvFree(&row_scratch)

   ;  return dst
;  }



mcxstatus mclgEdgeInc
(  mclgEdgeIter * ei
)
   {  mclp* src_ivp

   ;  if (ei->src >= ei->mx->cols + N_COLS(ei->mx))
      return STATUS_DONE

   ;  ei->src_i++
   ;  src_ivp = ei->src->ivps + ei->src_i

   ;  if (ei->src_i >= ei->src->n_ivps || ei->src->vid < src_ivp->idx)
      {  ei->src++
      ;  ei->src_i = -1
      ;  return mclgEdgeInc(ei)
   ;  }
      ei->dst = mclxGetVector(ei->mx, src_ivp->idx, RETURN_ON_FAIL, NULL)
   ;  ei->dst_i = ei->dst ? mclvGetIvpOffset(ei->dst, ei->src->vid, -1) : -1
   ;  return STATUS_OK
;  }


mcxstatus mclgEdgeIterInit
(  mclgEdgeIter *ei
,  mclx* mx
)
   {  ei->mx   =  mx
   ;  ei->src  =  mx->cols+0
   ;  ei->src_i  =  -1
   ;  return STATUS_OK
;  }


static void select_highest_dispatch
(  mclx* mx
,  dim i
,  void* data
,  dim thread_id     /* not needed here */
)
   {  mclvSelectHighest(mx->cols+i, ((dim*) data)[0])
;  }


void mclgKNNdispatch
(  mclx* mx
,  dim knn
,  dim n_thread
,  mcxenum mode
)
   {  if (!mclxIsGraph(mx) && mode != KNN_SELECT_ONLY)
      {  mcxErr
         (  "mclgKNNdispatch"
         ,  "knn-%lu request on matrix with %lu/%lu cols/rows, refused"
         ,  (ulong) knn,  (ulong) N_COLS(mx), N_ROWS(mx)
         )
      ;  return
   ;  }

      if (n_thread <= 1)
      {  dim i
      ;  for (i=0;i<N_COLS(mx);i++)
         mclvSelectHighest(mx->cols+i, knn)
      ;  if (mode == KNN_INTERSECT)
         mclxSymReduce(mx)
   ;  }
      else
      {  mclxVectorDispatch(mx, &knn, n_thread, select_highest_dispatch, NULL)
      ;  if (mode == KNN_INTERSECT)
         mclxSymReduceDispatch(mx, n_thread)
   ;  }
      if (mode == KNN_JOIN)         /* the union then .. */
      mclxMergeTranspose(mx, fltMax, 1.0)
;  }



void mclgKNN
(  mclx* mx
,  dim knn
)
   {  mclgKNNdispatch(mx, knn, 1, 1)
;  }



void mclgKNNmerge
(  mclx* mx
,  dim knn
)
   {  mclgKNNdispatch(mx, knn, 1, 0)
;  }



