/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Stijn van Dongen
 *   (C) Copyright 2008, 2009, 2010  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/*
 * NOTE some of the routines in here expect cluster matrix files
 * to be characteristic, i.e. 0/1-valued.
 * not yet documented; fixme
*/


#ifndef mcl_clm_h
#define mcl_clm_h

#include "impala/matrix.h"
#include "util/types.h"


typedef struct
{  double   efficiency
;  double   massfrac
;  double   areafrac
;
}  clmPerformanceTable   ;


/* 
 *    variance of information
 *    Comparing Clusterings, Marina Meila, Department of Statistics,
 *    University of Washington.
*/
void clmVIDistance
(  const mclMatrix*  cla
,  const mclMatrix*  clb
,  const mclMatrix*  abmeet
,  double* abdist
,  double* badist
)  ;


dim clmSJDistance     /* split join distance */
(  const mclMatrix*  cla
,  const mclMatrix*  clb
,  const mclMatrix*  abmeet
,  const mclMatrix*  bameet
,  dim*        abdist
,  dim*        badist
)  ;


void clmMKDistance     /* Mirkin distance */
(  const mclMatrix*  cla
,  const mclMatrix*  clb
,  const mclMatrix*  abmeet
,  dim*     abdist
,  dim*     badist
)  ;


   /* Does *NOT* check whether row domains match
    * or whether the matrices encode strict partitions.
    * The latter is useful e.g. when interested in
    * the meet of an overlapping clusterings with
    * itself or another clustering.
    *
    * Elements that are missing in either one will
    * be missing in the result.
   */
mclMatrix*  clmMeet
(  const mclMatrix*  c1
,  const mclMatrix*  c2
)  ;


mclMatrix*  clmSeparate
(  const mclMatrix*  cl
)  ;


mclMatrix* clmProject
(  const mclMatrix*  cl
,  const mclVector*  dom
)  ;


mclMatrix*  clmUGraphComponents
(  mclMatrix*  mx                /* mx->dom_rows is used as scratch area */
,  const mclMatrix*  dom
)  ;

mclMatrix*  clmComponents
(  mclMatrix*  mx                /* will be symmetrified */
,  const mclMatrix*  dom
)  ;


mcxstatus clmPerformance
(  const mclMatrix* mx
,  const mclMatrix* cl
,  clmPerformanceTable* pf
)  ;


                  /* used to create stats file for hierarchical clusters */
mcxstatus clmXPerformance
(  const mclx* mx
,  const mclx* clchild
,  const mclx* clparent
,  mcxIO* xf
,  dim   clceil
)  ;


typedef struct
{  dim      n_clusters
;  dim      size_cluster_max
;  double   size_cluster_ctr
;  double   size_cluster_avg
;  dim      size_cluster_min
;  dim      index_cluster_dg
;  dim      index_cluster_tw
;  dim      size_cluster_tw
;  dim      n_singletons
;  dim      n_qrt
;
}  clmGranularityTable   ;


mcxstatus clmGranularity
(  const mclx* cl
,  clmGranularityTable *tbl
)  ;


void clmGranularityPrint
(  FILE* fp
,  const char* info
,  clmGranularityTable *tbl
)  ;


void clmPerformancePrint
(  FILE* fp
,  const char* info
,  clmPerformanceTable* pf
)  ;


#endif


