/*   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#ifndef impala_edge_h
#define impala_edge_h

#include <stdio.h>
#include <stdarg.h>

#include "ivp.h"
#include "vector.h"
#include "pval.h"
#include "matrix.h"

#include "util/types.h"
#include "util/ting.h"


enum
{  SCRATCH_READY
,  SCRATCH_BUSY
,  SCRATCH_UPDATE
,  SCRATCH_DIRTY
}  ;

/* return union of columns with vid in dom.
 *
 * SCRATCH_READY:    mx->dom_rows is characteristic, will be used and reset.
 * SCRATCH_BUSY:     do not use scratch.
 * SCRATCH_DIRTY:    reset and use scratch, then leave it dirty.
 * SCRATCH_UPDATE:   ignore nodes in scratch, add newly encountered nodes, do not reset.
 *
 *
 * NOTE --- relevant for flood code that uses SCRATCH_UPDATE ---
 *    mclgUnionv does not update scratch for the indices in dom_cols.
 *    An example is clew/clm.c/clmComponents. The initial annotation,
 *    if needed, is provided by mclgUnionvInitNode or mclgUnionvInitList
 *
 * NOTE --- added mclgUnionv2 variants, that pass in a scratch area
 *    of their own (so that mclgUnionv2 can be used in a thread-safe way).
 *    This means the interface looks a bit kludgy by virtue of duplication. 
 *    This observation may lead to more changes, possibly mclgUnionv2
 *    will take over entirely (at the moment, mclgUnionv dispatches
 *    to mclgUnionv2).
 *
 * Fixme:
 *    improve documentation and/or interface.
 *    When is mclgUnionvInitNode used?
*/

#define MCLG_UNIONV_SENTINEL 1.5

#define  mclgUnionvInitNode(mx, node) \
         mclvInsertIdx(mx->dom_rows, node, MCLG_UNIONV_SENTINEL)

#define  mclgUnionvInitList(mx, vec) \
         mclvUpdateMeet(mx->dom_rows, vec, flt1p5)

#define  mclgUnionvResetList(mx, vec) \
         mclvUpdateMeet(mx->dom_rows, vec, flt1p0)

#define  mclgUnionvReset(mx) \
         mclvMakeCharacteristic(mx->dom_rows)

mclv* mclgUnionv
(  mclx* mx                   /*  mx->dom_rows used as scratch area     */
,  const mclv* dom_cols       /*  take union over these columns in mx   */
,  const mclv* restrict       /*  only consider row entries in restrict */
,  mcxenum SCRATCH_STATUS     /*  if SCRATCH_READY also left SCRATCH_READY */
,  mclv* dst
)  ;


#define  mclgUnionvInitNode2(vec, node) \
         mclvInsertIdx(vec, node, MCLG_UNIONV_SENTINEL)

#define  mclgUnionvInitList2(vec, list) \
         mclvUpdateMeet(vec, list, flt1p5)

#define  mclgUnionvResetList2(vec, list) \
         mclvUpdateMeet(vec, list, flt1p0)

#define  mclgUnionvReset2(vec) \
         mclvMakeCharacteristic(vec)

mclv* mclgUnionv2             /*  This one has a const matrix argument, additional scratch */
(  const mclx* mx
,  const mclv* dom_cols
,  const mclv* restrict
,  mcxenum SCRATCH_STATUS
,  mclv* dst
,  mclv* scratch
)  ;






   /* This will present two arcs at a time (if present)
    * If the graph is directed, it may miss arcs entirely,
    * as it only considers pairs with index (i,j) where i >= j.
   */
typedef struct
{  mclx* mx
;  mclv* src
;  mclv* dst
;  ofs   src_i
;  ofs   dst_i
;
}  mclgEdgeIter   ;

mcxstatus mclgEdgeIterInit
(  mclgEdgeIter *ei
,  mclx* mx
)  ;

mcxstatus mclgEdgeInc
(  mclgEdgeIter * ei
)  ;


enum
{  KNN_JOIN
,  KNN_INTERSECT
,  KNN_SELECT_ONLY
}  ;

void mclgKNNdispatch
(  mclx* mx
,  dim knn
,  dim n_thread
,  mcxenum mode           /* take intersect if reduce == 1, otherwise merge */
)  ;


void mclgKNN
(  mclx* mx
,  dim knn
)  ;


#endif

