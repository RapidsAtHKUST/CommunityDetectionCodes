/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
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
#include <limits.h>

#include "scan.h"

#include "impala/io.h"

#include "util/types.h"
#include "util/err.h"
#include "util/minmax.h"


void clmVScan
(  const mclVector*  vec
,  clmVScore* score
)
   {  clmVScanDomain(vec, NULL, score)
;  }


void clmVScanDomain
(  const mclVector*  vec
,  const mclVector*  dom
,  clmVScore* score
)
#define STAT_NONE 0
#define STAT_MEET 1
#define STAT_VDIF 2
#define STAT_DDIF 3
   {  dim      n_meet   =  0
   ;  dim      n_vdif   =  0
   ;  dim      n_ddif   =  0
   ;  double   max_i    = -FLT_MAX
   ;  double   max_o    = -FLT_MAX
   ;  double   min_i    =  FLT_MAX
   ;  double   min_o    =  FLT_MAX
   ;  double   sum_i    =  0.0
   ;  double   sum_o    =  0.0
   ;  double   ssq_i    =  0.0
   ;  double   ssq_o    =  0.0
   ;  mclIvp*  ivp1     =  vec->ivps
   ;  mclIvp*  ivp1max  =  ivp1 + vec->n_ivps
   ;  mclIvp*  ivp2, *ivp2max, *ivpself
   ;  double   powah    =  getenv("MCL_SCAN_R") ? atof(getenv("MCL_SCAN_R")) : 0.0
                                                   /* fixmefixmefixme hackhackhack */

   ;  if (!dom)
      dom = vec

   ;  ivp2     =  dom->ivps
   ;  ivp2max  =  ivp2 + dom->n_ivps

   ;  if (vec->vid >= 0 && (ivpself = mclvGetIvp(vec, vec->vid, NULL)))
         score->n_self = 1
      ,  score->wgt_s = ivpself->val
   ;  else 
         score->n_self = 0
      ,  score->wgt_s = 0.0

   ;  while (ivp1 < ivp1max)
      {  double val  =  0.0
      ;  long i1     =  ivp1->idx
      ;  long i2     =  ivp2 == ivp2max ? -1 : ivp2->idx  
      ;  int mode    =  STAT_NONE

                                                /* node edge el not in domain */
      ;  if (ivp2 == ivp2max || i1 < i2)
            mode  =  STAT_VDIF
         ,  val   = (ivp1++)->val
                                                /* domain el not in edge list */
                                                /* this case is ignored below */
      ;  else if (i1 > i2)
            mode  =  STAT_DDIF
         ,  ivp2++
                                                /* i1==i2, i1 in domain */
      ;  else
            mode  =  STAT_MEET
         ,  val   =  (ivp1++)->val
         ,  ivp2++


      ;  if (mode == STAT_VDIF)
         {  min_o  =  MCX_MIN(val, min_o)
         ;  max_o  =  MCX_MAX(val, max_o)
         ;  sum_o +=  val
         ;  ssq_o +=  powah ? pow(val, powah) : val * val
         ;  n_vdif++
      ;  }
         else if (mode == STAT_MEET)
         {  min_i  =  MCX_MIN(val, min_i)
         ;  max_i  =  MCX_MAX(val, max_i)
         ;  sum_i +=  val
         ;  ssq_i +=  powah ? pow(val, powah) : val * val
         ;  n_meet++
      ;  }
         else
         n_ddif++
   ;  }

      score->n_meet   =  n_meet
   ;  score->n_vdif   =  n_vdif
   ;  score->n_ddif   =  n_ddif + (ivp2max - ivp2)
   ;  score->max_i    =  max_i
   ;  score->min_i    =  min_i
   ;  score->sum_i    =  sum_i
   ;  score->ssq_i    =  ssq_i
   ;  score->max_o    =  max_o
   ;  score->min_o    =  min_o
   ;  score->sum_o    =  sum_o
   ;  score->ssq_o    =  ssq_o
   ;  score->sum_e    =  mclvSum(vec)  /* vec->val  fixme; clmCastActors should have set this; but it is not always called */
#undef STAT_MEET
#undef STAT_VDIF
#undef STAT_VDIF
;  }


/* given a finished score computation (by clmVScan)
 * compute the coverage
*/

void clmVScoreCoverage
(  clmVScore* score
,  double*   cov
,  double*   covmax
)
   {  double sum = score->sum_i + score->sum_o
   ;  double ssq = score->ssq_i + score->ssq_o
   ;  double div = score->sum_i - score->sum_o
   ;  double max = MCX_MAX(score->max_i, score->max_o)
   ;  double powah = getenv("MCL_SCAN_R") ? atof(getenv("MCL_SCAN_R")) : 0.0

   ;  *cov = 0.0
   ;  *covmax = 0.0

   ;  if (sum * sum)
      {  double ctr  =  ssq / (powah ? pow(sum, powah) : (sum * sum))
      ;  dim n_join  =  score->n_vdif+score->n_meet+score->n_ddif
      ;  dim n_dom   =  (dim) (n_join - score->n_vdif)

      ;  if (powah)
         ctr = pow(ctr, (1.0/(powah-1.0)))

      ;  max /=  sum
      ;  div /=  sum

      ;  if (ctr && n_join)
         *cov     =  1.0 - ((n_dom - (div/ctr)) /  n_join)
      ;  if (max && n_join)
         *covmax  =  1.0 - ((n_dom - (div/max)) /  n_join)
   ;  }
   }


void clmXScanInit
(  clmXScore* xscore
)
   {  xscore->max_i     = -FLT_MAX
   ;  xscore->min_i     =  FLT_MAX
   ;  xscore->sum_i     =  0.0
   ;  xscore->ssq_i     =  0.0
   ;  xscore->sum_s     =  0.0

   ;  xscore->max_o     = -FLT_MAX
   ;  xscore->min_o     =  FLT_MAX
   ;  xscore->sum_o     =  0.0
   ;  xscore->ssq_o     =  0.0

   ;  xscore->cov       =  0.0
   ;  xscore->covmax    =  0.0
   ;  xscore->n_elem_i  =  0
   ;  xscore->n_elem_o  =  0
   ;  xscore->n_self    =  0
   ;  xscore->n_hits    =  0
;  }


void clmXScanDomain
(  const mclMatrix*  mx
,  const mclVector*  domain
,  clmXScore* xscore
)
   {  mclVector* col = NULL
   ;  clmVScore vscore
   ;  dim i

   ;  if (!domain)
      domain = mx->dom_cols

   ;  for (i=0;i<domain->n_ivps;i++)
      {  long vid = domain->ivps[i].idx
      ;  double vsum = 0.0, vssq = 0.0

      ;  if ((col = mclxGetVector(mx, vid, RETURN_ON_FAIL, col)))
         {  clmVScanDomain(col, domain, &vscore)
         ;  vsum = vscore.sum_i + vscore.sum_o
         ;  vssq = vscore.ssq_i + vscore.ssq_o

         ;  xscore->n_self += vscore.n_self
         ;  xscore->sum_s  += vscore.wgt_s

         ;  xscore->max_i =  MCX_MAX(vscore.max_i, xscore->max_i)
         ;  xscore->min_i =  MCX_MIN(vscore.min_i, xscore->min_i)
                                          /* ^^ fixme; normalize ? */

         ;  if (vsum)
            {  xscore->sum_i += vscore.sum_i / vsum
            ;  xscore->sum_o += vscore.sum_o / vsum
            ;  if (vssq)
                  xscore->ssq_i += vscore.ssq_i / (vssq * vssq)
               ,  xscore->ssq_o += vscore.ssq_o / (vssq * vssq)
         ;  }

            xscore->max_o =  MCX_MAX(vscore.max_o, xscore->max_o)
         ;  xscore->min_o =  MCX_MIN(vscore.min_o, xscore->min_o)

         ;  xscore->n_elem_i += vscore.n_meet
         ;  xscore->n_elem_o += vscore.n_vdif

         ;  xscore->n_hits++

         ;  if (domain->n_ivps)
            {  double cov, covmax
            ;  clmVScoreCoverage(&vscore, &cov, &covmax)
;if(0)fprintf(stderr, "node %d add %.3f domain size %d nb size %d\n", (int) col->vid, cov, (int) domain->n_ivps, (int) col->n_ivps)
            ;  xscore->cov += cov
            ;  xscore->covmax += covmax
         ;  }
            col++
      ;  }
         else
         mcxErr("clmXScanDomain PANIC", "cannot find col <%ld>", vid)
   ;  }
   }


void clmXScanDomainSet
(  const mclMatrix*  mx
,  const mclMatrix*  cl
,  const mclv* cl_select
,  clmXScore* xscore
)
   {  dim d
   ;  const mclv* clvec = NULL
   ;  for (d=0;d<cl_select->n_ivps;d++)
      {  if
         (  (clvec
         =  mclxGetVector(cl, cl_select->ivps[d].idx, RETURN_ON_FAIL, clvec)
         )  )
         clmXScanDomain(mx, clvec, xscore)
   ;  }
   }


void clmXScoreCoverage
(  clmXScore* xscore
,  double*   cov
,  double*   covmax
)
   {  if (xscore->n_hits)
      {  *cov = xscore->cov / xscore->n_hits
      ;  if (covmax)
         *covmax = xscore->covmax / xscore->n_hits
   ;  }            
      else
      {  *cov = 0.0
      ;  if (covmax)
         *covmax = 0.0
   ;  }
   }


double clmCoverage
(  const mclx* mx
,  const mclx* cl
,  double* covmaxp
)
   {  double cov, covmax
   ;  clmXScore xscore
   ;  clmXScanInit(&xscore)
   ;  clmXScanDomainSet(mx, cl, cl->dom_cols, &xscore)
   ;  clmXScoreCoverage(&xscore, &cov, &covmax)
   ;  if (covmaxp)
      *covmaxp = covmax
   ;  return cov
;  }


