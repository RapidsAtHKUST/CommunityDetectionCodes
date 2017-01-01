/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#define DEBUG 1

#include "interpret.h"

#include <math.h>
#include <float.h>
#include <limits.h>

#include "clew/clm.h"

#include "util/types.h"
#include "util/alloc.h"
#include "util/io.h"
#include "util/err.h"

#include "impala/io.h"
#include "impala/edge.h"

#include "clew/scan.h"

#define  IS_ATTRACTIVE_RECURBOUND     20


#define  is_attractive      0x01
#define  is_unattractive    0x02
#define  is_classified      0x04


/* Keep only highest weight edges for each tail node,
 * using some weighting factors.
*/

mclMatrix* mclDag
(  const mclMatrix*  A
,  const mclInterpretParam* ipp
)
   {  dim d

   ;  double w_selfval= ipp ? ipp->w_selfval: 0.999
   ;  double w_maxval = ipp ? ipp->w_maxval : 0.001
   ;  double delta    = ipp ? ipp->delta    : 0.01

   ;  mclMatrix* M = mclxAllocZero
                     (  mclvCopy(NULL, A->dom_cols)
                     ,  mclvCopy(NULL, A->dom_rows)
                     )
   ;  for (d=0;d<N_COLS(A);d++)           /* thorough clean-up */
      {  mclVector*  vec      =  A->cols+d
      ;  mclVector*  dst      =  M->cols+d
      ;  double      selfval  =  mclvIdxVal(vec, vec->vid, NULL)
      ;  double      maxval   =  mclvMaxValue(vec)
      ;  double      bar      =  selfval < maxval
                                 ?  (  (w_selfval * selfval)
                                    +  (w_maxval * maxval)
                                    )
                                 :     delta
                                    ?  selfval / (1 + delta)
                                    :  selfval
      ;  int n_bar =  mclvCountGiven(vec, mclpGivenValGQ, &bar)
      ;  mclvCopyGiven(dst, vec, mclpGivenValGQ, &bar, n_bar)
   ;  }
if (0)
{ dim ne = mclxNrofEntries(M)
;fprintf(stderr, "nroff entries %u\n", (unsigned) ne)
;
}
      return M
;  }



/* m_transient *should* be a nilpotent matrix; the dag-depth is the
   number of multiplications it takes to arrive at an all-zero matrix.
   In actuality we may have loops, so we need to do some kind of marking.

   Transpose the matrix; only compute waves for nodes with no incoming edges.
   May merit general purpose routine.

   TODO: loop detection, handle on dag.
*/

static int calc_depth
(  mclx* m_transient
)
   {  mclx* m_inverse = mclxTranspose(m_transient)
   ;  dim c, depth = 0

;if(0)puts("")

   ;  for (c=0;c<N_COLS(m_inverse);c++)
      {  dim this_depth = 0
      ;  if (!m_inverse->cols[c].n_ivps)    /* no incoming nodes */
         {  mclv* next = mclxGetVector(m_transient, m_inverse->cols[c].vid, RETURN_ON_FAIL, NULL)
         ;  if (!next)
            continue
         ;  mclgUnionvInitList(m_transient, next)
         ;  do
            {  mclv* next2 = mclgUnionv(m_transient, next, NULL, SCRATCH_UPDATE, NULL)
            ;  if (0 && next->ivps)
                  fprintf(stdout, "chain %d ->\n", (int) m_inverse->cols[c].vid)
               ,  mclvaDump(next, stdout, -1, " ", 0)
            ;  if (this_depth)   /* otherwise starting vector in matrix */
               mclvFree(&next)
            ;  next = next2
            ;  this_depth++
         ;  }
            while (next->n_ivps)
         ;  mclvFree(&next)      /* did loop at least once, so not the starting vector */
         ;  mclgUnionvReset(m_transient)
      ;  }
         if (this_depth > depth)
         depth = this_depth
   ;  }

      mclxFree(&m_inverse)
   ;  return depth
;  }


int mclDagTest
(  const mclMatrix* dag
)
   {  mclv* v_transient =  mclvCopy(NULL, dag->dom_cols)
   ;  mclx* m_transient =  NULL
   ;  int maxdepth      =  0
   ;  dim d

   ;  mclvMakeCharacteristic(v_transient)
   ;  for (d=0;d<N_COLS(dag);d++)
      {  mclv* col = dag->cols+d
      ;  if (mclvGetIvp(col, col->vid, NULL))   /* deemed attractor */
         mclvInsertIdx(v_transient, col->vid, 0.25)
   ;  }

      mclvSelectGqBar(v_transient, 0.5)

   ;  m_transient = mclxSub(dag, v_transient, v_transient)
;if(0)mclxDebug("-", m_transient, 3, "transient")
   ;  maxdepth = calc_depth(m_transient)

   ;  mclxFree(&m_transient)
   ;  mclvFree(&v_transient)
   ;  return maxdepth
;  }


   /* fixme this should pbb be a generic utility routine
      fixme wave1 copy seems too much. simpler design should be possible
   */

static mclv* get_closure
(  mclx*  mx               /* caller must have invoked mclgUnionvReset before */
,  const mclv* nbls
)
   {  mclv* nbls_closure = mclvCopy(NULL, nbls), *wave1 = mclvCopy(NULL, nbls_closure), *wave2 = NULL
   ;  mclgUnionvInitList(mx, nbls_closure)

   ;  while (wave1->n_ivps)
      {  wave2 = mclgUnionv(mx, wave1, NULL, SCRATCH_UPDATE, NULL)
      ;  mcldMerge(nbls_closure, wave2, nbls_closure)
      ;  mclvFree(&wave1)
      ;  wave1 = wave2
   ;  }
      mclgUnionvResetList(mx, nbls_closure)
   ;  mclvFree(&wave1)
   ;  return nbls_closure
;  }


mclMatrix* mclInterpret
(  mclMatrix* dag
)
   {  mclv* v_attr = mclvCopy(NULL, dag->dom_cols)
   ;  mclx* m_attr = NULL, *m_cls = NULL, *m_clst = NULL
   ;  dim d

   ;  mclvMakeCharacteristic(v_attr)

   ;  for (d=0;d<N_COLS(dag);d++)
      {  mclv* col = dag->cols+d
      ;  if (mclvGetIvp(col, col->vid, NULL))   /* deemed attractor */
         mclvInsertIdx(v_attr, col->vid, 2.0)
   ;  }

      mclvSelectGqBar(v_attr, 1.5)

   ;  m_attr = mclxSub(dag, v_attr, v_attr)
   ;  mclxAddTranspose(m_attr, 1.0)

   ;  m_cls = clmUGraphComponents(m_attr, NULL) /* attractor systems as clusters */
   ;  mclvCopy(m_cls->dom_rows, dag->dom_cols)  /* add all nodes to this cluster matrix */
   ;  m_clst = mclxTranspose(m_cls)             /* nodes(columns) with zero neighbours need to be classified */
   ;  mclgUnionvReset(dag)                      /* make mx->dom-rows characteristic */
   ;  mclxFree(&m_cls)

   ;  for (d=0;d<N_COLS(dag);d++)
      {  mclv* closure, *clsids
      ;  if (mclvGetIvp(v_attr, dag->cols[d].vid, NULL))
         continue                               /* attractor already classified */

      ;  closure =   get_closure(dag, dag->cols+d)  /* take all [neighbours of [neighbours of [..]]] */
      ;  clsids  =   mclgUnionv(m_clst, closure, NULL, SCRATCH_READY, NULL)

      ;  mclvAdd(m_clst->cols+d, clsids, m_clst->cols+d)
      ;  mclvFree(&clsids)
      ;  mclvFree(&closure)
   ;  }

      m_cls = mclxTranspose(m_clst)
   ;  mclxFree(&m_attr)
   ;  mclxFree(&m_clst)
   ;  mclvFree(&v_attr)
   ;  return m_cls
;  }



void  clusterMeasure
(  const mclMatrix*  clus
,  FILE*          fp
)
   {  int         clsize   =  N_COLS(clus)
   ;  int         clrange  =  N_ROWS(clus)
   ;  double      ctr      =  0.0
   ;  dim         d

   ;  for (d=0;d<N_COLS(clus);d++)
      ctr += pow((clus->cols+d)->n_ivps, 2.0)

   ;  if (clsize && clrange)
      ctr /= clrange

   ;  fprintf
      (  fp
      ,  " %-d"
      ,  (int) clsize
      )
;  }


            /* Keep it around as annotation of the idea
             * it encodes. Not terribly exciting or useful.
             * Reconsider at the next census.
            */
#if 0
void mcxDiagnosticsAttractor
(  const char*          ffn_attr
,  const mclMatrix*     clus2elem
,  const mcxDumpParam   dumpParam
)
   {  int         n_nodes     =  clus2elem->n_range
   ;  int         n_written   =  dumpParam->n_written
   ;  mclMatrix*  mtx_Ascore  =  mclxAllocZero(n_written, n_nodes)
   ;  mcxIO*      xfOut       =  mcxIOnew(ffn_atr, "w")
   ;  dim         d           =  0

   ;  if (mcxIOopen(xfOut, RETURN_ON_FAIL) == STATUS_FAIL)
      {  mclxFree(&mtx_Ascore)
      ;  mcxIOfree(&xfOut)
      ;  return
   ;  }

   ;  for(d=0; d<n_written; d++)
      {  mclMatrix*  iterand     =  *(dumpParam->iterands+d)
      ;  mclVector*  vec_Ascore  =  NULL

      ;  if (iterands->n_cols != n_nodes || iterand->n_range != n_nodes)
         {  fprintf(stderr, "mcxDiagnosticsAttractor: dimension error\n")
         ;  mcxExit(1)
      ;  }

         vec_Ascore  =  mcxAttractivityScale(iterand)
      ;  mclvRenew((mtx_Ascore->cols+d), vec_Ascore->ivps, vec_Ascore->n_ivps)
      ;  mclvFree(&vec_Ascore)
   ;  }

      mclxbWrite(mtx_Ascore, xfOut, RETURN_ON_FAIL)
   ;  mclxFree(mtx_Ascore)
;  }


void mcxDiagnosticsPeriphery
(  const char*     ffn_peri
,  const mclMatrix*  clustering
,  const mcxDumpParam   dumpParam
)  {
;  }
#endif


mclVector*  mcxAttractivityScale
(  const mclMatrix*           M
)
   {  dim n_cols = N_COLS(M)
   ;  dim d

   ;  mclVector* vec_values = mclvResize(NULL, n_cols)

   ;  for (d=0;d<n_cols;d++)
      {  mclVector*  vec   =  M->cols+d
      ;  double   selfval  =  mclvIdxVal(vec, d, NULL)
      ;  double   maxval   =  mclvMaxValue(vec)
      ;  if (maxval <= 0.0)
         {  mcxErr
            (  "mcxAttractivityScale"
            ,  "encountered nonpositive maximum value"
            )
         ;  maxval = 1.0  
      ;  }
         (vec_values->ivps+d)->idx = d
      ;  (vec_values->ivps+d)->val = selfval / maxval
   ;  }
      return vec_values
;  }


void mclInterpretParamFree
(  mclInterpretParam **ipp
)
   {  if (*ipp)
      mcxFree(*ipp)
   ;  *ipp = NULL
;  }



mclInterpretParam* mclInterpretParamNew
(  void
)
   {  mclInterpretParam* ipp =  mcxAlloc
                                (  sizeof(mclInterpretParam)
                                ,  EXIT_ON_FAIL
                                )
   ;  ipp->w_selfval    =  0.999
   ;  ipp->w_maxval     =  0.001
   ;  ipp->delta        =  0.01

   ;  return ipp
;  }


