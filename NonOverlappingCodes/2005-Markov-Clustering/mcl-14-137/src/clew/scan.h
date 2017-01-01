/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/*
 *  For a stochastic vector u and a cluster P, the coverage cov(P,u) equals
 *
 *                  1       ( ---            ---            ) 
 *       |P|  -  -------- * ( \       u_i  - \          u_i )
 *                ctr(u)    ( /__i[=P        /__i[=P^C      )
 *  1 -  ------------------------------------------------------
 *                         | P V S |
 *
 *
 *                  1       ( ---            ---            ) 
 *      |S/P| +  -------- * ( \       u_i  - \          u_i )
 *                ctr(u)    ( /__i[=P        /__i[=P^C      )
 *       ------------------------------------------------------
 *                         | P V S |
 *
 *  Where S is the set of nonzero entries in u (and |S/P| is
 *  the size of S with elements of P removed).
 *  So for arbitrary nonngative v we need to compute:
 *
 *  div, normalized by sum(v)  (the large term on the upper right)
 *  ctr, sum of squares normalized by sum(v)^2
 *
*/


/*    TODO:
 *    support simultaneous support in clmVScore for complement and meet ?
 *    The needed data structure would feel unwieldy somehow.
 *    e.g.  max_intern
 *          min_intern
 *          max_extern
 *          min_extern
 *    etc
 *    and how about the index for which max/minima are achieved ?
 *
 *    The current setup is still very experimental and bound to be rewritten
 *    some day.
 *
 *    fixme:!
 *    clmXScanDomain will implicitly act on meet if domain is not a subdomain.
 *    this is not so nice; strict interface is better.
 *    not nice because e.g. average coverage is returned, but if subdomain
 *    is not required than the number over which is averaged is not clear.
*/


#ifndef clm_scan_h
#define clm_scan_h


#include "impala/matrix.h"
#include "util/types.h"


typedef struct
{  dim         n_vdif   /* vector diff */
;  dim         n_meet
;  dim         n_ddif   /* domain diff */
;  dim         n_self   /* 1 if vec->vid >= 0 && vid in vec->ivps, 0 otherwise */
;  double      wgt_s    /* self weight */
;  double      sum_e    /* sum of edge weights */
;  double      max_i
;  double      min_i
;  double      sum_i
;  double      ssq_i    /* sum of squares, but we do other powers as well */
;  double      max_o
;  double      min_o
;  double      sum_o
;  double      ssq_o
;
}  clmVScore   ;


typedef struct
{  dim         n_elem_i       /* edges, presumably */
;  dim         n_elem_o       /* edges, presumably */
;  dim         n_hits
;  dim         n_self   /* number of loops present */
;  double      sum_s    /* sum of loops            */
;  double      max_i
;  double      min_i
;  double      sum_i
;  double      ssq_i
;  double      max_o
;  double      min_o
;  double      sum_o
;  double      ssq_o
;  double      cov      /* this is the true coverage measure */
;  double      covmax   /* this is the true coverage measure */
;
}  clmXScore   ;


void clmXScanInit
(  clmXScore* xscore
)  ;


void clmVScan
(  const mclVector*  vec
,  clmVScore* score
)  ;


/* NOTE.
 *    sum_i and sum_o and ssq_i and ssq_o are computed on vector
 *    without stochastification.
 *
 * The 'cov' number is the sum of mass of entries in vec that
 * are in dom, minus the sum of mass of entries in vec that are
 * not in dom. It can be used to compute the 'coverage' of dom wrt vec.
*/

void clmVScanDomain
(  const mclVector*  vec
,  const mclVector*  dom
,  clmVScore* score
)  ;


/* compute coverage measures from a score struct */

void clmVScoreCoverage
(  clmVScore* score
,  double*   cov
,  double*   covmax
)  ;


void clmXScoreCoverage
(  clmXScore* xscore
,  double*   cov
,  double*   covmax
)  ;


double clmCoverage
(  const mclx* mx
,  const mclx* cl
,  double* covmaxp
)  ;


/* NOTE.
 * -  sum_i sum_o ssq_i ssq_o are computed as if mx
 *    were first made stochastic.
 * -  max_i max_o min_i min_o are computed on mx
 *    itself.
 *
 * This may change in the future, as {max,min}_{io}
 * are currently nowhere used.
 *
 * NOTE
 *    you have to call clmXScanInit to initialize xscore.
*/

void clmXScanDomain
(  const mclMatrix*  mx
,  const mclVector*  cl
,  clmXScore* xscore
)  ;


/*
 * NOTE
 *    you have to call clmXScanInit to initialize xscore.
*/

void clmXScanDomainSet
(  const mclMatrix*  mx
,  const mclMatrix*  cl
,  const mclv* cl_select
,  clmXScore* xscore
)  ;



#endif

