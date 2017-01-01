/*   (C) Copyright 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include "shadow.h"

#include "impala/ivp.h"
#include "impala/io.h"
#include "impala/edge.h"

#include "util/minmax.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/err.h"


double mclvMedian
(  const mclv* vecx
)
   {  mclv* vec = mclvClone(vecx)
   ;  dim N
   ;  double m
   ;  if (!vec || !vec->n_ivps)
      return -PVAL_MAX
   ;  mclvSort(vec, mclpValCmp)

   ;  N = vec->n_ivps  
   ;  m = (vec->ivps[(N-1)/2].val + vec->ivps[N/2].val) / 2.0
   ;  mclvFree(&vec)
   ;  return m
;  }


double mclvAvg
(  const mclv* vec
)
   {  if (vec->n_ivps)
      return mclvSum(vec) / vec->n_ivps
   ;  return 0.0
;  }


mclv* mcl_shadow_matrix
(  mclx* mx
,  const mclv* factors
)
   {  dim i_master, N = N_COLS(mx)
   ;  dim n_affected = 0
   ;  mclv* dom_master
   ;  const char* ep
   ;  double boost = 0.0

   ;  if ((ep = getenv("MCL_LOOP_BOOST")))
      boost = atof(ep)

   ;  if (!mclxDomCanonical(mx))
      mcxDie(1, "shadow", "shadowing only works with canonical domains")

   ;  mcxLog(MCX_LOG_MODULE, "mcl", "creating shadow loops")

   ;  dom_master = mclvClone(mx->dom_cols)

   ;  {  mclv* dom_shadow = mclvCanonical(NULL, 2*N, 1.0)
      ;  mclxAccommodate(mx, dom_shadow, dom_shadow)
      ;  mclvFree(&dom_shadow)
   ;  }

      for (i_master=0;i_master<N;i_master++)
      {  mclp ivps_shadow[2]
      ;  dim i_shadow         =  i_master+N
      ;  mclv* v_master       =  mx->cols+i_master
      ;  mclv* v_shadow       =  mx->cols+i_shadow
      ;  double master_max    =  mclvMaxValue(v_master)
      ;  double fac           =  mclvIdxVal(factors, i_master, NULL) 

      ;  if (master_max < 0)
         master_max = 1

      ;  if (boost)
         master_max *= boost

      ;  ivps_shadow[0].idx   =  i_master
      ;  ivps_shadow[0].val   =  master_max

      ;  ivps_shadow[1].idx   =  i_shadow
      ;  ivps_shadow[1].val   =  master_max * fac     /* shadow self-value */
         
      ;  mclvFromIvps(v_shadow, ivps_shadow, 2)

      ;  mclvInsertIdx(v_master, (long) i_master, master_max)
      ;  mclvInsertIdx(v_master, (long) i_shadow, master_max * fac)  /* master-to-shadow value */

      ;  if (fac)
         n_affected++
   ;  }

      mcxLog(MCX_LOG_MODULE, "mcl", "done (%lu)", (ulong) n_affected)
   ;  return dom_master
;  }


mclv* mcl_get_shadow_turtle_factors
(  const mclx* mx
,  long shadow_mode
,  double shadow_s
)
   {  dim i_master, N = N_COLS(mx)
   ;  mcxbool dump = getenv("MCL_DUMP_SHADOW") ? TRUE : FALSE
   ;  mclv* v_factors
   ;  dim n_affected = 0
   ;  double gl_gg
   ;  double gl_sz_avg
   ;  dim ne

   ;  mclv* v_size, *v_sum, *v_gauge

   ;  if (!mclxDomCanonical(mx))
      mcxDie(1, "shadow", "shadowing only works with canonical domains")

   ;  mcxLog
      (  MCX_LOG_MODULE
      ,  "mcl"
      ,  "computing turtle shadow factors (mode %ld)"
      ,  shadow_mode
      )

   ;  v_gauge  =  mclxColNums(mx, mclvAvg, MCL_VECTOR_COMPLETE)
   ;  v_size   =  mclxColSizes(mx, MCL_VECTOR_COMPLETE)
   ;  v_sum    =  mclxColNums(mx, mclvSum, MCL_VECTOR_COMPLETE)

   ;  ne = mclxNrofEntries(mx)
   ;  gl_gg = mclvSum(v_sum)
   ;  if (ne)
      gl_gg /= ne
   ;  gl_sz_avg = ne / N_COLS(mx)          /* fixme, unchecked */

   ;  v_factors = mclvCanonical(NULL, N_COLS(mx), 0.0)

   ;  for (i_master=0;i_master<N;i_master++)
      {  mclv* v_master = mx->cols+i_master

      ;  double master_sz     =  v_master->n_ivps
      ;  double lc_sz_avg     =  0.0
      ;  double lc_gg         =  0.0         /* local gauge */
      ;  double fac_sz        =  1.0
      ;  double fac_final     =  1.0
      ;  double fac_gg        =  1.0
      ;  double master_gg     =  0.0
      ;  mclv* nbhood = NULL

      ;  dim j

      ;  do
         {  if (!master_sz)
            break

         ;  nbhood = mclvClone(v_master)
         ;  mclvNormalize(nbhood)

         ;  master_gg = mclvAvg(v_master)

               /* compute weighted averages of
                * node edge value averages and node edge counts
                * in the neighbour(hood) list.

                * nbhood    : the list of neighbours for v_master
                * nb        : one in the list of neighbours
                * p         : ivp of the reverse arc from nb to v_master, if present
                * lc_sz_avg : sum of  { outgoing weight * #neighbours-of-nb }
                * lc_gg     : sum of  { outgoing weight * avg(outgoing-weight-of-nb) }
               */
         ;  for (j=0;j<nbhood->n_ivps;j++)
            {  long nb    =   nbhood->ivps[j].idx
                                 /* fixme: canonical dependency */
            ;  mclp* p    =   mclvGetIvp(mx->cols+nb, i_master, NULL)
            ;  lc_sz_avg +=   nbhood->ivps[j].val * v_size->ivps[nb].val
            ;  if
               ( (shadow_mode & (MCL_SHADOW_SELF))
               || !p
               || v_size->ivps[nb].val < 1.5
               )
               lc_gg
               += nbhood->ivps[j].val * v_gauge->ivps[nb].val
            ;  else
               lc_gg
               +=    nbhood->ivps[j].val
                  *  (  (v_gauge->ivps[nb].val * v_size->ivps[nb].val)
                      - p->val
                     )
                  /  (v_size->ivps[nb].val - 1)
         ;  }

            if (lc_gg <= 0 || lc_sz_avg <= 0 || master_gg <= 0)
            break

         ;  if
            (  shadow_mode & MCL_SHADOW_E_HIGH
            && lc_sz_avg < master_sz
            )
            fac_sz = master_sz / lc_sz_avg
         ;  else if
            (  shadow_mode & MCL_SHADOW_E_LOW
            && lc_sz_avg > master_sz
            )
            fac_sz = lc_sz_avg / master_sz
         ;  else
            fac_sz = 1.0


         ;  if
            (  shadow_mode & MCL_SHADOW_V_HIGH
            && lc_gg < master_gg
            )
            fac_gg = master_gg / lc_gg
         ;  else if
            (  shadow_mode & MCL_SHADOW_V_LOW
            && lc_gg > master_gg
            )
            fac_gg = lc_gg / master_gg
         ;  else
            fac_gg = 1.0

;if(0)
fprintf
(stderr
,"node %d gauge[%.2f] size[%.2f] mssize[%d]\n"
,(int) i_master
,fac_gg
,fac_sz
,(int) v_master->n_ivps
)
                                 /* fixme more control
                                  * -> no directionality yet in size
                                 */
         ;  fac_final =    (shadow_mode & MCL_SHADOW_MULTIPLY)
                        ?  fac_sz * fac_gg
                        :  MCX_MAX(fac_sz,fac_gg)
         ;  if (fac_final < 1.0)
            fac_final = 1.0

         ;  fac_final = pow(fac_final, shadow_s)

         ;  fac_final -= 1
;if(dump)
fprintf
(stdout
," [%d->%.2f]"
,(int) i_master
,fac_final
)

         ;  mclvInsertIdx(v_factors, i_master, fac_final)
         ;  n_affected++
      ;  }
         while (0)

      ;  mclvFree(&nbhood)
   ;  }

      mclvFree(&v_size)
   ;  mclvFree(&v_sum)
   ;  mclvFree(&v_gauge)
   ;  return v_factors
;  }


mclv* mcl_density_adjust
(  mclx* mx
,  const char* da
)
   {  mclgEdgeIter ei = { 0 }
   ;  double da_inflate = 1.0
   ;  int da_exp = 1
   ;  mclv* qv

   ;  sscanf(da, "%d,%lf", &da_exp, &da_inflate)
;fprintf(stderr, "values %d %g\n", da_exp, da_inflate)
   ;  mclxAdjustLoops(mx, mclxLoopCBmax, NULL)
   ;  qv = mclxPowColSums(mx, da_exp, MCL_VECTOR_COMPLETE)

   ;  mclgEdgeIterInit(&ei, mx)
   ;  while (!mclgEdgeInc(&ei))
      {  double q1, q2, factor = 0.0
      ;  if (ei.dst_i < 0)
         continue
      ;  q1 = qv->ivps[ei.src->vid].val
      ;  q2 = qv->ivps[ei.dst->vid].val
;fprintf(stdout, "src %d (%g) dst %d (%g)", (int) ei.src->vid, (double) q1, (int) ei.dst->vid, (double) q2)
      ;  if (q1 && q2)
         factor = q1 < q2 ? q1 / q2 : q2 / q1
      ;  ei.src->ivps[ei.src_i].val *= pow(factor, da_inflate)
      ;  ei.dst->ivps[ei.dst_i].val *= pow(factor, da_inflate)
;fprintf(stdout, " %f\n", pow(factor, da_inflate))
   ;  }
      {  double qvmax = mclvMaxValue(qv)
      ;  int i
      ;  for (i=0;i<qv->n_ivps;i++)
         {  if (qv->ivps[i].val)
            qv->ivps[i].val /= qvmax
      ;  }
      }
      return qv
;  }


