/*   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef clm_claw_h
#define clm_claw_h

#include <stdlib.h>
#include <stdio.h>

#include "impala/matrix.h"

enum
{  CLM_NODE_SELF    =  1
,  CLM_NODE_INCIDENT
}  ;


/* fixme not documented (at all), mixes dump & readjust functionality
 * It's a quick and dirty prototype.
*/
void clmDumpNodeScores
(  const char* name
,  mclx* mx    /* will be made stochastic; no provision yet for precompute */ 
,  mclx* cl
,  mcxenum mode
)  ;


   /* look at each node invidually, move it to other cluster if the
    * signs are good.
    * Rationale: sparse segments attached to dense segments tend
    * to suck a few nodes out of the dense segment (with mcl).
    * This tries to remedy that.
   */
dim clmAdjust
(  mclx* mx
,  mclx* cl
,  dim cls_size_max
,  mclx** cl_adjustedpp
,  mclv** ls_adjustedpp
,  dim*  sjd_left
,  dim*  sjd_right
)  ;



   /* Look at small clusters of size <= prune_sz and try to have
    * them aborbed by bigger clusters.
    * TODO: perhaps we should do this iteratively similar to Adjust.
   */
dim clmAssimilate
(  mclx* mx
,  mclx* cl
,  dim   prune_sz
,  mclx** cl_prunedpp
,  dim*  sjd_left
,  dim*  sjd_right
)  ;



/* el_on_cl will for each node n always contain the cluster in which
 * it was found, even if the incidence was very small.
 * That incidence is currently always set to 0.01 at least, so
 * mass projections do not add up perfectly to 1.0 in case the self
 * projection value was smaller than 0.01.
*/

void clmCastActors
(  mclx**   mx          /* is made stochastic         */
,  mclx**   cl          /* entries are set to self-value */
,  mclx**   el_to_cl    /* transpose of cl            */
,  mclx**   el_on_cl    /* will be made stochastic    */
,  mclx**   cl_on_cl    /* will be made stochastic    */
,  mclx**   cl_on_el    /* transpose of stochastic el_on_cl */
,  double   frac        /* consider clusters in el_on_cl until frac
                               edge weight is covered */
)  ;


#endif

