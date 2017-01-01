/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdio.h>
#include <math.h>

#include "clm.h"
#include "scan.h"
#include "cat.h"

#include "impala/matrix.h"
#include "impala/compose.h"
#include "impala/edge.h"

#include "impala/io.h"        /* debug purposes */

#include "util/ting.h"
#include "util/io.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/equate.h"
#include "util/err.h"


#ifdef DEBUG
#define DEBUG_DEFINED 1
#else
#define DEBUG 0
#endif


/* fixme: implement */
mclx* clmTop
(  mclv*  dom
)  ;
mclx* clmBottom
(  mclv*  dom
)  ;


mclx* clmProject
(  const mclx*  cl
,  const mclv*  dom
)
   {  mclx* pr =   mclxSub(cl, cl->dom_cols, dom)
   ;  dim overlap=0, missing=0, empty=0
   ;  dim total = clmEnstrict(pr, &overlap, &missing, &empty, ENSTRICT_SPLIT_OVERLAP)

   ;  if (total != empty)
      mcxErr
      (  "clmProject"
      ,  "input clustering on <%lu> elements is not a partition o=%lu m=%lu e=%lu"
      ,  (ulong) N_ROWS(cl)
      ,  (ulong) overlap
      ,  (ulong) missing
      ,  (ulong) empty
      )

   ;  return pr
;  }


double clmLogVariance
(  const mclx*  cl
)
   {  double v = 0.0
   ;  dim d

   ;  if (!N_ROWS(cl))
      return 0.0        /* fixme; some other value ? */

   ;  for (d=0;d<N_COLS(cl);d++)
      {  double n = cl->cols[d].n_ivps
      ;  if (n)
         v += n * log(n)
   ;  }
      return -v
;  }


void clmVIDistance
(  const mclx*  cla
,  const mclx*  clb
,  const mclx*  abmeet
,  double*     abdist
,  double*     badist
)
   {  dim d, e
   ;  double varab   =  0.0
   ;  double vara    =  0.0
   ;  double varb    =  0.0
   ;  double n_elems =  N_ROWS(cla)

   ;  *abdist = 0.0
   ;  *badist = 0.0

   ;  if (!n_elems)
      return

   /* fixme?: test for enstriction ? */
   /* fixme-warning: using reals as ints in abmeet
    * this warning belongs in the caller really,
    * or where abmeet is produced
   */

   ;  if (!mcldEquate(cla->dom_rows, clb->dom_rows, MCLD_EQT_EQUAL))
      {  mcxErr
         (  "clmVIDistance PBD"
         ,  "domains sized (%ld,%ld) differ"
         ,  (long) N_ROWS(cla)
         ,  (long) N_ROWS(clb)
         )
      ;  return
   ;  }

      for (d=0;d<N_COLS(abmeet);d++)
      {  mclv   *vecmeets   =  abmeet->cols+d
      ;  mclv   *bvec       =  NULL
      ;  double n_lft            =  cla->cols[d].n_ivps
      ;  double n_rgt
      ;  double n_meet

      ;  for (e=0;e<vecmeets->n_ivps;e++)
         {  n_meet   =  vecmeets->ivps[e].val
         ;  bvec     =  mclxGetVector
                        (  clb
                        ,  vecmeets->ivps[e].idx
                        ,  EXIT_ON_FAIL
                        ,  bvec
                        )
         ;  n_rgt =  bvec->n_ivps
         ;  if (n_rgt && n_lft)
            varab += n_meet * log (n_meet / (n_lft * n_rgt))
      ;  }
      }

   ;  vara  =  clmLogVariance(cla)
   ;  varb  =  clmLogVariance(clb)
   ;  *badist = (vara - varab) / n_elems
   ;  *abdist = (varb - varab) / n_elems
   ;  if (*badist <= 0.0)  /* for -epsilon case */
      *badist  =  0.0
   ;  if (*abdist <= 0.0)  /* for -epsilon case */
      *abdist  =  0.0
;  }


dim clmSJDistance
(  const mclx*  cla
,  const mclx*  clb
,  const mclx*  abmeet
,  const mclx*  bameet
,  dim*        abdistp
,  dim*        badistp
)
   {  dim d, e
   ;  mclx* ab, *ba 
   ;  mcxbool cleanup = !abmeet

   ;  dim abdist = 0
   ;  dim badist = 0

   ;  if (!abmeet)
      {  abmeet = ab = clmContingency(cla, clb)
      ;  bameet = ba = mclxTranspose(abmeet)
   ;  }
                  /* fixme?: test for enstriction ? */
                  /* fixme-warning: using reals as ints. */

      if (!mcldEquate(cla->dom_rows, clb->dom_rows, MCLD_EQT_EQUAL))
      {  mcxErr
         (  "clmSJDistance PBD"
         ,  "domains sized (%ld,%ld) differ"
         ,  (long) N_ROWS(cla)
         ,  (long) N_ROWS(clb)
         )
      ;  return (dim) -1
   ;  }

      for (d=0;d<N_COLS(abmeet);d++)
      {  int         max            =  0
      ;  mclv   *vecmeets      =  abmeet->cols+d

      ;  for (e=0;e<vecmeets->n_ivps;e++)
         if ((int) ((vecmeets->ivps+e)->val+0.25) > max)
         max = (int) ((vecmeets->ivps+e)->val + 0.25)
      ;  abdist += (cla->cols+d)->n_ivps - max
   ;  }

      for (d=0;d<N_COLS(bameet);d++)
      {  int         max         =  0
      ;  mclv   *vecmeets   =  bameet->cols+d

      ;  for (e=0;e<vecmeets->n_ivps;e++)
         if ((int) (vecmeets->ivps+e)->val > max)
         max = (int) ((vecmeets->ivps+e)->val + 0.25)
      ;  badist += (clb->cols+d)->n_ivps - max
   ;  }
      if (cleanup)
         mclxFree(&ab)
      ,  mclxFree(&ba)

   ;  if (abdistp)
      *abdistp = abdist
   ;  if (badistp)
      *badistp = badist

   ;  return abdist + badist
;  }


mclx* clmMeet
(  const mclx*  cla
,  const mclx*  clb
)
   {  dim d, a
   ;  int n_clmeet, i_clmeet
   ;  mclx   *abmeet, *clmeet
   ;  const char* mepanic = "clmMeet panic"

   ;  abmeet      =     clmContingency(cla, clb)
   ;  if (!abmeet)
      return NULL

   ;  i_clmeet    =     0
   ;  n_clmeet    =     mclxNrofEntries(abmeet)
   ;  clmeet      =     mclxAllocZero
                        (  mclvCanonical(NULL, n_clmeet, 1.0)
                        ,  mclvCopy(NULL, cla->dom_rows)
                        )

   ;  for (a=0;a<N_COLS(abmeet);a++)
      {  mclv* vec    =  abmeet->cols+a
      ;  mclv* bvec   =  NULL

      ;  for (d=0;d<vec->n_ivps;d++)
         {  long  b  =  (vec->ivps+d)->idx
         ;  bvec = mclxGetVector(clb, b, RETURN_ON_FAIL, bvec)

         ;  if (!bvec || i_clmeet == n_clmeet)
            {  mcxErr(mepanic, "internal math does not add up")
            ;  continue
           /*  little bit frightning */
         ;  }

            mcldMeet
            (  cla->cols+a
            ,  bvec
            ,  clmeet->cols+i_clmeet
            )
         ;  i_clmeet++
      ;  }
      }

      if (i_clmeet != n_clmeet)
      mcxErr(mepanic, "internal math does not substract")

   ;  mclxFree(&abmeet)
   ;  return clmeet
;  }


void clmMKDistance
(  const mclx*    cla
,  const mclx*    clb
,  const mclx*    abmeet
,  dim*           abdist
,  dim*           badist
)
   {  dim d, e
   ;  double sosqa  = 0.0
   ;  double sosqb  = 0.0
   ;  double sosqm  = 0.0

   ;  if (!mcldEquate(cla->dom_rows, clb->dom_rows, MCLD_EQT_EQUAL))
      {  mcxErr
         (  "clmMKDistance PBD"
         ,  "domains sized (%ld,%ld) differ"
         ,  (long) N_ROWS(cla)
         ,  (long) N_ROWS(clb)
         )
      ;  return
   ;  }

      for (d=0;d<N_COLS(abmeet);d++)
      {  mclv  *vecmeets  =  abmeet->cols+d
      ;  for (e=0;e<vecmeets->n_ivps;e++)
         sosqm +=  pow((vecmeets->ivps+e)->val, 2)
   ;  }

      for (d=0;d<N_COLS(cla);d++)
      sosqa +=  pow(cla->cols[d].n_ivps, 2)
   ;  for (d=0;d<N_COLS(clb);d++)
      sosqb +=  pow(clb->cols[d].n_ivps, 2)

   ;  *abdist = (dim) (sosqa - sosqm + 0.5)
   ;  *badist = (dim) (sosqb - sosqm + 0.5)
;  }



mcxstatus clmGranularity
(  const mclx* cl
,  clmGranularityTable *tbl
)
   {  clmVScore vscore
   ;  mclv* szs   =  mclxColSizes(cl, MCL_VECTOR_COMPLETE)
   ;  dim d       =  szs->n_ivps
   ;  dim d1      =  0
   ;  dim d2      =  0
   ;  dim d2_size =  0
   ;  dim n_sgl   =  0
   ;  dim david   =  0
   ;  dim n_qrt   =  0         /* quartletons */

   ;  clmVScan(szs, &vscore)
   ;  mclvSort(szs, mclpValRevCmp)

   ;  if (N_COLS(cl))
      while (d-- > 0)      /* careful with unsignedness */
      {  david += szs->ivps[d].val
      ;  if (szs->ivps[d].val <= 1.0)
         n_sgl = szs->n_ivps-d
      ;  if (szs->ivps[d].val <= 4.0)
         n_qrt = szs->n_ivps-d
      ;  if (!d1 && david >= vscore.max_i)
         d1 = szs->n_ivps-d
      ;  if (!d2 && david >= ((N_ROWS(cl)+1)/2))
         {  d2 = d
         ;  d2_size = szs->ivps[d].val
      ;  }
   ;  }

      tbl->n_clusters         =  N_COLS(cl)
   ;  tbl->size_cluster_max   =  vscore.max_i
   ;  tbl->size_cluster_ctr   =  vscore.sum_i ? vscore.ssq_i / vscore.sum_i : -1.0
   ;  tbl->size_cluster_avg   =  N_COLS(cl) ?  N_ROWS(cl) / (double) N_COLS(cl) : -1.0
   ;  tbl->size_cluster_min   =  vscore.min_i
   ;  tbl->index_cluster_dg   =  d1
   ;  tbl->index_cluster_tw   =  d2
   ;  tbl->size_cluster_tw    =  d2_size
   ;  tbl->n_singletons       =  n_sgl
   ;  tbl->n_qrt              =  n_qrt

   ;  mclvFree(&szs)
   ;  return STATUS_OK
;  }


void clmGranularityPrint
(  FILE* fp
,  const char* info
,  clmGranularityTable *tbl
)
   {  fprintf
      (  fp
      ,  "clusters=%lu max=%lu %s"
         " ctr=%.1f avg=%.1f min=%lu DGI=%lu TWI=%lu TWL=%lu"
         " sgl=%lu qrt=%lu"

      ,  (ulong) tbl->n_clusters
      ,  (ulong) tbl->size_cluster_max
      ,  info ? info : ""
      ,  tbl->size_cluster_ctr
      ,  tbl->size_cluster_avg
      ,  (ulong) tbl->size_cluster_min
      ,  (ulong) tbl->index_cluster_dg
      ,  (ulong) tbl->index_cluster_tw
      ,  (ulong) tbl->size_cluster_tw
      ,  (ulong) tbl->n_singletons
      ,  (ulong) tbl->n_qrt
      )
;  }


void clmPerformancePrint
(  FILE* fp
,  const char* info
,  clmPerformanceTable* pf
)
   {  fprintf
      (  fp
      ,  "efficiency=%.5f massfrac=%.5f areafrac=%.5f %s"
      ,  pf->efficiency
      ,  pf->massfrac
      ,  pf->areafrac
      ,  info ? info : ""
      )
;  }


mcxstatus clmXPerformance
(  const mclx* mx
,  const mclx* clchild
,  const mclx* clparent
,  mcxIO* xf
,  dim   clceil
)
   {  clmXScore xscore
   ;  dim i

   ;  if (clparent)
      {  mclx* cting = clmContingency(clparent, clchild)    /* from parent to child */

      ;  if (N_COLS(cting) != N_COLS(clparent))
         mcxDie
         (  1
         ,  "clmXPerformance"
         ,  "pathetic %ld vs %ld"
         ,  (long) N_COLS(cting)
         ,  (long) N_COLS(clparent)
         )

      ;  for (i=0;i<N_COLS(cting);i++)
         {  mclv *cllist = cting->cols+i
         ;  mclv* cl = NULL
         ;  mclx* sub = NULL
         ;  dim j

                     /* A parser of this output has to deduce COV from the parent,
                      * which is printed later. mlmhihohum does exactly this.
                      * Note that the no-parent/root case (other branch)
                      * ensures we always output a non-trivial parent for any
                      * cluster.
                     */
         ;  if (cllist->n_ivps == 1)
            {  cl = mclxGetVector(clchild, cllist->ivps[0].idx, EXIT_ON_FAIL, cl)
            ;  fprintf
               (  xf->fp
               ,  "%-10ld TRIVIAL[sz=%ld] PARENT[%ld]\n"
               ,  (long) cllist->ivps[0].idx
               ,  (long) cl->n_ivps
               ,  (long) clparent->cols[i].vid
               )
            ;  continue
         ;  }

            sub = mclxSub(mx, clparent->cols+i, clparent->cols+i)

         ;  for (j=0;j<cllist->n_ivps;j++) 
            {  dim rdif, n_elem_ii
            ;  mcxTing* num = mcxTingEmpty(NULL, 40)
            ;  cl = mclxGetVector(clchild, cllist->ivps[j].idx, EXIT_ON_FAIL, cl)

            ;  if (mcldCountParts(clparent->cols+i, cl, NULL, NULL, &rdif))
               mcxDie(1, "clmXPerformance", "pathetic II")

            ;  fprintf(xf->fp, "%-10ld", (long) cl->vid)

            ;  clmXScanInit(&xscore)

            ;  if (!clceil || cl->n_ivps <= clceil)
               clmXScanDomain(mx, cl, &xscore)
            ;  else if (clceil)
                  xscore.cov = 0
               ,  xscore.covmax = 0
               ,  xscore.n_hits = cl->n_ivps

                  /* NOTE
                   * n_elem_ii is the number of edges internally,
                   * deduplicated.
                   *
                   * for n_elem_o we do not need to do deduplication
                   * as everything is counted just once.
                  */
            ;  n_elem_ii = (xscore.n_elem_i - xscore.n_self) / 2

            ;  if (xscore.max_o > -FLT_MAX)
               mcxTingPrint(num, "%.4f", xscore.max_o)
            ;  else
               mcxTingWrite(num, "0")

;if (xscore.n_hits != cl->n_ivps)
fprintf
(  stderr
,  "mismatch! cluster %ld in parent with %ld clusters\n"
,  (long) cl->n_ivps
,  (long) N_COLS(clparent)
);
                                    /* fixme divide-by-zero */
            ;  fprintf
               (  xf->fp
               ,  " GLOBAL[sz=%lu cov=%.4f covmax=%.4f int=%.4f ext=%.4f i=%ld e=%ld"
               ,  (ulong) cl->n_ivps
               ,  (double) xscore.cov / xscore.n_hits
               ,  (double) xscore.covmax / xscore.n_hits
               ,     cl->n_ivps == 1
                  ?  1.0f
                  :  (2.0f * n_elem_ii) / (cl->n_ivps * (cl->n_ivps-1))
               ,     n_elem_ii + xscore.n_elem_o
                  ?  (1.0f * n_elem_ii) / (n_elem_ii + xscore.n_elem_o)
                  :  0.0f
               ,  (long) n_elem_ii
               ,  (long) xscore.n_elem_o
               )

            ;  if (0)               /* todo interface */
               fprintf
               (  xf->fp
               ,  " maxi=%.4f avgi=%.4f maxo=%s avgo=%.4f"
               ,  xscore.max_i
               ,     xscore.n_elem_i
                  ?  (double) (xscore.sum_i / xscore.n_elem_i)
                  :  0.0
               ,  num->str
               ,     xscore.n_elem_o
                  ?  (double) (xscore.sum_o / xscore.n_elem_o)
                  :  0.0
               )
            ;  fputs("] ", xf->fp)

               /* implement local scoring, i.e. in the context of
                * the subgraph induced by the parent cluster.
                * I am not sure whether the stuff below
                * does exactly that. Reinstate with care.
               */
#if 0
            ;  clmXScanInit(&xscore)

            ;  if (!clceil || cl->n_ivps <= clceil)
               clmXScanDomain(mx, cl, &xscore)
            ;  else if (clceil)
                  xscore.cov = 0
               ,  xscore.covmax = 0
               ,  xscore.n_hits = cl->n_ivps

            ;  if (xscore.max_o > -FLT_MAX)
               mcxTingPrint(num, "%.4f", xscore.max_o)
            ;  else
               mcxTingWrite(num, "0")

            ;  if (xscore.n_hits)
               fprintf
               (  xf->fp
               ,  " LOCAL[sz=%lu cov=%.4f covmax=%.4f"
               ,  (ulong) clparent->cols[i].n_ivps
               ,  (double) xscore.cov / xscore.n_hits
               ,  (double) xscore.covmax / xscore.n_hits
               )

            ;  if (0)
               fprintf
               (  xf->fp
               ,  " maxi=%.4f avgi=%.4f maxo=%s avgo=%.4f] "
               ,  xscore.max_i
               ,     xscore.n_elem_i
                  ?  (double) (xscore.sum_i / xscore.n_elem_i)
                  :  0.0
               ,  num->str
               ,     xscore.n_elem_o
                  ?  (double) (xscore.sum_o / xscore.n_elem_o)
                  :  0.0
               )
            ;  fputs("] ", xf->fp)
#endif

            ;  fprintf(xf->fp, " PARENT[%ld]", (long) clparent->cols[i].vid)

            ;  if (0)
               {  dim i2
               ;  fputs(" CHILDREN[", xf->fp)
               ;  for (i2=0;i2<cl->n_ivps-1;i2++)
                  fprintf(xf->fp, "%ld, ", (long) cl->ivps[i2].idx)
               ;  if (cl->n_ivps)
                  fprintf(xf->fp, "%ld", (long) cl->ivps[i2].idx)
               ;  fputc(']', xf->fp)
            ;  }

               fputc('\n', xf->fp)
         ;  }
            mclxFree(&sub)
      ;  }
         mclxFree(&cting)
   ;  }
      else
      {  mclv *cl = clchild->cols, *clmax = cl + N_COLS(clchild)
      ;  while (cl < clmax)
         {  double cov
         ;  if (!clceil || cl->n_ivps <= clceil)
            {  clmXScanInit(&xscore)
            ;  clmXScanDomain(mx, cl, &xscore)
            ;  cov = xscore.n_hits ? xscore.cov / xscore.n_hits : -1
         ;  }
            else
            cov = 0.0
         ;  fprintf(xf->fp, "%ld %.4f\n", cl->vid, cov)
         ;  cl++
      ;  }
      }
      return STATUS_OK
;  }



mcxstatus clmPerformance
(  const mclx* mx
,  const mclx* cl
,  clmPerformanceTable* pf
)
   {  double mxArea = N_COLS(mx) * (N_COLS(mx) -1)
   ;  clmXScore xscore
   ;  double clArea  =  0.0
   ;  dim c
   ;  clmXScanInit(&xscore)

   ;  for (c=0;c<N_COLS(cl);c++)
      {  mclv *cvec   =  cl->cols+c
      ;  clmXScanDomain(mx, cvec, &xscore)
      ;  clArea     +=  cvec->n_ivps * (cvec->n_ivps -1)
   ;  }

      if (!mxArea)
      mxArea = -1.0
   ;  if (!clArea)
      clArea = -1.0

        /* massFraction, overlap: counting edges in overlap doubly (or more) */

   ;  pf->massfrac   =  xscore.n_hits ? xscore.sum_i / xscore.n_hits : -1.0
   ;  pf->efficiency =  xscore.n_hits ? xscore.cov / xscore.n_hits : -1.0
   ;  pf->areafrac   =  mxArea ? clArea / mxArea : -1.0

   ;  return STATUS_OK
;  }


mclx* clmComponents
(  mclx* mxin
,  const mclx* dom
)
   {  mclx* mx = mclxCopy(mxin), *cc
   ;  mclxAddTranspose(mx, 0.0)
   ;  cc = clmUGraphComponents(mx, dom)
   ;  mclxFree(&mx)
   ;  return cc
;  }

/* mq fewer mallocs still possible? */

mclx* clmUGraphComponents
(  mclx* mxin
,  const mclx* dom
)
   {  dim d, c, n_cls = 0
   ;  mcxbool project = dom ? TRUE : FALSE
   ;  mclv* wave1 = NULL
   ;  mclx* coco                                /* connected components */

   ;  if (!mxin || !mclxIsGraph(mxin))
      return NULL

                        /* construct a single-column matrix */
   ;  if (!project)
      {  dom = mclxAllocZero
               (mclvInsertIdx(NULL, 0, 1.0), mclvCopy(NULL, mxin->dom_rows))
      ;  mclvCopy(dom->cols+0, mxin->dom_rows)
   ;  }

      coco  =  mclxAllocZero
               (mclvCanonical(NULL, N_COLS(mxin), 1.0), mclvCopy(NULL, mxin->dom_rows))

   ;  mclgUnionvReset(mxin)

   ;  for (c=0;c<N_COLS(dom);c++)
      {  mclv* domvec = mclvClone(dom->cols+c)
      ;  mclv* wave2  = NULL
      ;  mclvMakeCharacteristic(domvec)         /* important, we use values as sentinels   */

      ;  for (d=0;d<domvec->n_ivps;d++)
         {  long cidx = domvec->ivps[d].idx     /* consider all neighbours of list c node */

         ;  if (domvec->ivps[d].val > 1.5)
            {  if (0) fprintf(stdout, "skip %d\n", (int) cidx)
            ;  continue
         ;  }

                                                /* fixme finish computation, escalate error */
            if (n_cls == N_COLS(coco))
            mcxDie(1, "mclcComponents", "ran out of space, fix me")

         ;  mclvInsertIdx(coco->cols+n_cls, cidx, 1.0)
         ;  mclgUnionvInitNode(mxin, cidx)

         ;  wave1 = mclvCopy(wave1, coco->cols+n_cls)

         ;  while (wave1->n_ivps)
            {  wave2 = mclgUnionv(mxin, wave1, domvec, SCRATCH_UPDATE, NULL)
            ;  mcldMerge(wave2, coco->cols+n_cls, coco->cols+n_cls)
                                                /* fixme; consider getting rid of merge */
            ;  mclvFree(&wave1)
            ;  wave1 = wave2
         ;  }
            mclvUpdateMeet(domvec, coco->cols+n_cls, fltAdd)
         ;  n_cls++
      ;  }
         mclvFree(&domvec)
   ;  }

      if (!project)
      mclxFree((mclx**) &dom)   /* we did allocate it ourselves */

   ;  mclvResize(coco->dom_cols, n_cls)
   ;  coco->cols = mcxRealloc(coco->cols, n_cls * sizeof(mclv), RETURN_ON_FAIL)
   ;  mclxColumnsRealign(coco, mclvSizeRevCmp)
   ;  mclvFree(&wave1)

   ;  mclgUnionvReset(mxin)
   ;  return coco
;  }


#ifdef DEBUG_DEFINED
#undef DEBUG_DEFINED
#else
#undef DEBUG
#endif

