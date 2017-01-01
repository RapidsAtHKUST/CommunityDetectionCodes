/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef mcl_expand_h
#define mcl_expand_h

#include <stdio.h>
#include <pthread.h>

#include "util/ting.h"
#include "util/heap.h"
#include "util/types.h"

#include "impala/matrix.h"

#define  MCL_PRUNING_RIGID   1
#define  MCL_PRUNING_ADAPT  2

#define  MCL_EXPAND_DENSE  1           
#define  MCL_EXPAND_SPARSE 2    

extern int mclDefaultWindowSizes[];
extern dim mcl_n_windows;


typedef struct
{  double            chaosMax
;  double            chaosAvg
;  double            homgMax
;  double            homgMin
;  double            homgAvg
;  mclv*             homgVec
;  double            lap

;  int               n_cols

;  float*            bob_low        /* initial pruning */
;  float*            bob_final      /* final result    */
;  dim*              bob_expand     /* size after expansion */
;  volatile dim      bob_sparse
;
}  mclExpandStats    ;


typedef struct
{  mclExpandStats*   stats
;  int               n_ethreads

;  double            precision
;  double            pct

;  dim               num_prune
;  dim               num_select
;  dim               num_recover
;  dim               partition_pivot_sort_n
;  int               scheme

#define MCL_USE_PARTITION_SELECTION 1 << 0
#define MCL_USE_RPRUNE              1 << 1

;  mcxbits           implementation

#define  XPNVB(mxp, bit)   (mxp->verbosity & bit)

#define  XPNVB_PRUNING     1 << 0
#define  XPNVB_EXPLAIN     1 << 1
#define  XPNVB_CLUSTERS    1 << 2

;  mcxbits           verbosity
;  int               vector_progression

;  int               warn_factor
;  double            warn_pct
;  dim               sparse_trigger

;  int               dimension
;  double            inflation      /* for computing homg vector     */

;
}  mclExpandParam    ;


mclMatrix* mclExpand
(  const mclMatrix*  mx
,  const mclMatrix*  mxright
,  mclExpandParam*   mxp
)  ;


mclExpandStats* mclExpandStatsNew
(  dim   n_cols
)  ;  


void mclExpandParamDim
(  mclExpandParam*  mxp
,  const mclMatrix* mx
)  ;


void mclExpandStatsReset
(  mclExpandStats* stats
)  ;


void mclExpandStatsFree
(  mclExpandStats** statspp
)  ;


void mclExpandStatsPrint
(  mclExpandStats*  stats
,  FILE*             fp
)  ;

void mclExpandAppendLog
(  mcxTing* Log
,  mclExpandStats *s
,  int n_ite
)  ;

void mclExpandInitLog
(  mcxTing* Log
,  mclExpandParam* mxp  
)  ;

void mclExpandStatsHeader
(  FILE* vbfp
,  mclExpandStats* stats
,  mclExpandParam*   mxp
)  ;


mclExpandParam* mclExpandParamNew
(  void
)  ;

void mclExpandParamFree
(  mclExpandParam** epp
)  ;


#endif

