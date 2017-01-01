/*   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include "claw.h"

#include "util/types.h"
#include "util/err.h"
#include "util/minmax.h"
#include "util/compile.h"
#include "util/ting.h"

#include "scan.h"
#include "clm.h"
#include "cat.h"

#include "impala/matrix.h"
#include "impala/compose.h"
#include "impala/pval.h"
#include "impala/io.h"        /* likely just for debugging purposes  */

static const char* us = "claw";


static void clm_dump_line
(  const char* name
,  clmVScore*  sc
,  long        nid
,  long        cid
,  dim         ndsize
,  dim         clsize         /* for defensive check */
,  int         alien
)
   {  double sum = sc->sum_i + sc->sum_o
   ;  double eff, eff_max
   ;  mcxTing* ti = NULL, *to = NULL

   ;  clmVScoreCoverage(sc, &eff, &eff_max)
   ;  if (clsize != sc->n_ddif + sc->n_meet)
      mcxErr
      (  "clmDumpNodeScores panic"
      ,  "for cluster %ld in %s diff <%ld> meet <%ld> do not add to <%lu>"
      ,  (long) cid
      ,  name
      ,  (long) sc->n_ddif
      ,  (long) sc->n_meet
      ,  (ulong) clsize
      )

   ;  if (sum && sc->max_i > -FLT_MAX)
      ti = mcxTingPrint(NULL, "%.5f", sc->max_i / sum)

   ;  if (sum && sc->max_o > -FLT_MAX)
      to = mcxTingPrint(NULL, "%.5f", sc->max_o / sum)

   ;  fprintf
      (  stdout
      ,  "nm=%s ni=%ld ci=%ld nn=%lu nc=%lu"
         " ef=%.5f em=%.5f mf=%.5f ma=%.5f"
         " xn=%lu xc=%lu ns=%lu"
         " ti=%s to=%s"
         " al=%d"
         "\n"

      ,  name,  nid,  cid,  (ulong) ndsize, (ulong) clsize
      ,  eff,  eff_max,  sum ? sc->sum_i / sum : 0.0f,  sc->sum_e
      ,  (ulong) sc->n_vdif, (ulong) sc->n_ddif,  (ulong) sc->n_meet
      ,  (ti ? ti->str : "na"),  (to ? to->str : "na")
      ,  alien
      )
;  }


/* fixme pbb need enstricted clustering;
 * document and check or enforce
*/

void clmDumpNodeScores
(  const char* name
,  mclx* mx
,  mclx* cl
,  mcxenum  mode
)
   {  dim d, e
   ;  clmVScore sc

   ;  if (mode == CLM_NODE_SELF)
      {  for (d=0;d<N_COLS(cl);d++)
         {  ofs o = -1
         ;  dim clsize = cl->cols[d].n_ivps
         ;  for (e=0;e<clsize;e++)
            {  long idx = cl->cols[d].ivps[e].idx
            ;  o = mclxGetVectorOffset(mx, idx, EXIT_ON_FAIL, o)
            ;  mx->cols[o].val = mclvSum(mx->cols+o)              /* fixme stupid dependency. */
            ;  clmVScanDomain(mx->cols+o, cl->cols+d, &sc)
            ;  clm_dump_line
               (name, &sc, idx, cl->cols[d].vid, mx->cols[o].n_ivps, clsize, 0)
         ;  }
         }
         /* fixme: sum_e not set, pbb due to missing clmCastActors */
      }
      else if (mode == CLM_NODE_INCIDENT)
      {  mclx *el_to_cl = NULL
      ;  mclx *el_on_cl = NULL
      ;  mclx *cl_on_cl = NULL
      ;  mclx *cl_on_el = NULL
      ;  clmCastActors
         (&mx, &cl, &el_to_cl, &el_on_cl, &cl_on_cl, &cl_on_el, 0.95)
      ;  mclxFree(&cl_on_cl)
      ;  mclxFree(&cl_on_el)

      ;  for (d=0;d<N_COLS(mx);d++)
         {  long nid  = mx->cols[d].vid
         ;  long nsize = mx->cols[d].n_ivps
         ;  mclv* clidvec = mclxGetVector(el_on_cl, nid, RETURN_ON_FAIL, NULL)
         ;  mclv* clself = mclxGetVector(el_to_cl, nid, RETURN_ON_FAIL, NULL)
         ;  dim f

         ;  if (!clself)
            mcxErr
            ("clmDumpNodeScores panic", "node <%ld> does not belong", nid)

         ;  for (f=0;f<clidvec->n_ivps;f++)
            {  long cid = clidvec->ivps[f].idx
            ;  mclv* clvec = mclxGetVector(cl, cid, RETURN_ON_FAIL, NULL)
                          /* ^ overdoing: cid == clvec->vid */
            ;  int alien
            ;  if (!clvec)
               {  mcxErr
                  (  "clmDumpNodeScores panic"
                  ,  "cluster <%ld> node <%ld> mishap"
                  ,  cid
                  ,  nid
                  )
               ;  continue
            ;  }
               clmVScanDomain(mx->cols+d, clvec, &sc)
            ;  alien = clself && clvec->vid == clself->ivps[0].idx ? 0 : 1
            ;  clm_dump_line
               (name, &sc, nid, clvec->vid, nsize, clvec->n_ivps, alien)
         ;  }
         }
         mclxFree(&el_on_cl)
      ;  mclxFree(&el_to_cl)
   ;  }
   }



/*    The entries in cl are set to the self-projection value.
 *    fixme: don't exit, propagate.
*/

static void set_cl_to_projection
(  mclMatrix* cl
,  mclMatrix* el_on_cl
)
   {  dim i, j
   ;  for (i=0;i<N_COLS(cl);i++)
      {  mclv* clvec = cl->cols+i
      ;  long  clid  = clvec->vid
      ;  mclv* elclvec = NULL
      ;  mclp* valivp = NULL
      ;  for (j=0;j<clvec->n_ivps;j++)
         {  long elid = clvec->ivps[j].idx
         ;  elclvec = mclxGetVector(el_on_cl, elid, EXIT_ON_FAIL, elclvec)
         ;  valivp = mclvGetIvp(elclvec, clid, NULL)
         ;  if (!valivp && clvec->n_ivps > 1)
            mcxErr("clmCastActors", "match error: el %ld cl %ld", elid, clid)
         ;  clvec->ivps[j].val = valivp ? MCX_MAX(0.01, valivp->val) : 0.01
      ;  }
      }
   }


static void prune_el_on_cl
(  mclMatrix* el_to_cl  /* must be conforming */
,  mclMatrix* el_on_cl  /* this one will be pruned */
,  double pct
,  int max
)
   {  dim i
   ;  for (i=0;i<N_COLS(el_on_cl);i++)
      {  mclv*  elclvec =  el_on_cl->cols+i
      ;  long   clid    =  el_to_cl->cols[i].ivps[0].idx
      ;  double sum     =  0.0
      ;  int n_others   =  0
      ;  dim k          =  0
      ;  mcxbool selfok =  FALSE
      ;  mclvSort(elclvec, mclpValRevCmp)

      ;  while (k++ < elclvec->n_ivps && sum < pct && n_others < max)
         {  long y = elclvec->ivps[k-1].idx
         ;  if (y == clid)
            selfok = TRUE
         ;  sum += elclvec->ivps[k-1].val
         ;  n_others++
      ;  }

         mclvResize(elclvec, k-1)        /* careful recentchange */
      ;  mclvSort(elclvec, mclpIdxCmp)
      ;  if (!selfok)
         mclvInsertIdx(elclvec, clid, 0.01)
   ;  }
   }


void clmCastActors
(  mclx**   mxpp        /* is made stochastic and has loops added */
,  mclx**   clpp        /* entries are set to self-value */
,  mclx**   el_to_clpp  /* transpose of cl            */
,  mclx**   el_on_clpp  /* will be made stochastic    */
,  mclx**   cl_on_clpp  /* will be made stochastic    */
,  mclx**   cl_on_elpp  /* transpose of stochastic el_on_cl */
,  double   frac        /* consider clusters in el_on_cl until frac
                               edge weight is covered */
)
   {  mclxAdjustLoops(*mxpp, mclxLoopCBmax, NULL)
   ;  mclxMakeStochastic(*mxpp)
   ;  *el_to_clpp =  mclxTranspose(*clpp)

                              /* el_to_cl not stochastic? */
   ;  *el_on_clpp =  mclxCompose(*el_to_clpp, *mxpp, 0, 1)
   ;  mclxMakeStochastic(*el_on_clpp)

   ;  *cl_on_clpp =  mclxCompose(*el_on_clpp, *clpp, 0, 1)
   ;  mclxMakeStochastic(*cl_on_clpp)

   ;  set_cl_to_projection(*clpp, *el_on_clpp)
   ;  prune_el_on_cl(*el_to_clpp, *el_on_clpp, frac, 10)
   ;  *cl_on_elpp =  mclxTranspose(*el_on_clpp)
;  }


static dim clm_clm_adjust
(  mclx* mx
,  mclx* cl
,  dim cls_size_max
,  mclx** cl_adjustedpp
,  mclv** cid_affectedpp
,  mclv** nid_affectedpp
)
   {  dim i, j, n_adjusted = 0
   ;  mclx* cl_adj = mclxCopy(cl)

   ;  mclv* cid_affected = mclvClone(cl->dom_cols)
   ;  mclv* nid_affected = mclvClone(mx->dom_cols)
   ;  double bar_affected = 1.5

   ;  const char* e1 = getenv("MCL_ADJ_FMAX")
   ;  const char* e2 = getenv("MCL_ADJ_EMASS")
   
   ;  double f1 = e1 ? atof(e1) : 2
   ;  double f2 = e2 ? atof(e2) : 3

   ;  mcxbool loggit = mcxLogGet( MCX_LOG_CELL | MCX_LOG_INFO )

   ;  clmVScore sc

   ;  mclx *el_to_cl = NULL
   ;  mclx *el_on_cl = NULL
   ;  mclx *cl_on_cl = NULL
   ;  mclx *cl_on_el = NULL

   ;  *cl_adjustedpp = NULL
   ;  *cid_affectedpp = NULL
   ;  *nid_affectedpp = NULL

   ;  clmCastActors
      (&mx, &cl, &el_to_cl, &el_on_cl, &cl_on_cl, &cl_on_el, 0.95)

   ;  mclxFree(&cl_on_cl)
   ;  mclxFree(&cl_on_el)

   ;  mclvMakeConstant(cid_affected, 1.0)
   ;  mclvMakeConstant(nid_affected, 1.0)


   ;  for (i=0;i<N_COLS(cl_adj);i++)
      cl_adj->cols[i].val = 0.5

                     /*    Proceed with smallest clusters first.
                      *    Caller has to take care of mclxColumnsRealign
                     */
   ;  for (i=0;i<N_COLS(cl);i++)
      {  mclv* clself = cl->cols+i

                     /*    Only consider nodes in clusters of
                      *    size <= cls_size_max
                     */
      ;  if (cls_size_max && clself->n_ivps > cls_size_max)
         break
                     /*    Clusters that have been marked for inclusion
                      *    cannot play.
                     */
      ;  if (cl_adj->cols[i].val > 1)
         continue

      ;  for (j=0;j<clself->n_ivps;j++)
         {  long nid  = clself->ivps[j].idx
         ;  long nos  = mclvGetIvpOffset(mx->dom_cols, nid, -1)
         ;  mclv* clidvec  =  mclxGetVector(el_on_cl, nid, RETURN_ON_FAIL, NULL)

         ;  double eff_alien_bsf = 0.0, eff_alien_max_bsf = 0.0 /* best so far*/
         ;  double eff_self = 0.0, eff_self_max = 0.0
         ;  long cid_alien  = -1, cid_self = -1
         ;  clmVScore sc_self = { 0 }, sc_alien = { 0 }
         ;  dim f

         ;  if (nos < 0 || !clidvec)
            {  mcxErr
               ("clmDumpNodeScores panic", "node <%ld> does not belong", nid)
            ;  continue
         ;  }

            clmVScanDomain(mx->cols+nos, clself, &sc)
         ;  clmVScoreCoverage(&sc, &eff_self, &eff_self_max)
         ;  cid_self = clself->vid
         ;  sc_self  = sc

         ;  if (loggit)
            mcxLog2
            (  us
            ,  "node %ld in cluster %ld eff %.3f,%.3f sum %.3f"
            ,  nid
            ,  cid_self
            ,  eff_self
            ,  eff_self_max
            ,  sc.sum_i
            )

         ;  for (f=0;f<clidvec->n_ivps;f++)
            {  long cid = clidvec->ivps[f].idx
            ;  mclv* clvec = mclxGetVector(cl, cid, RETURN_ON_FAIL, NULL)
                          /* ^ overdoing: cid == clvec->vid */
            ;  double eff, eff_max
            ;  if (!clvec)
               {  mcxErr
                  (  "clmAdjust panic"
                  ,  "cluster <%ld> node <%ld> mishap"
                  ,  cid
                  ,  nid
                  )
               ;  continue
            ;  }


                        /* fixme: document or remove first condition
                         *
                        */
               if ((0 && clvec->n_ivps <= clself->n_ivps) || clvec->vid == cid_self)
               continue

            ;  clmVScanDomain(mx->cols+nos, clvec, &sc)
            ;  clmVScoreCoverage(&sc, &eff, &eff_max)

#if 0
#  define PIVOT eff > eff_alien_bsf
#else
#  define PIVOT eff_max > eff_alien_max_bsf
#endif

            ;  if
               (  PIVOT
               || sc.sum_i >= 0.5
               )
                  eff_alien_bsf = eff
               ,  eff_alien_max_bsf = eff_max
               ,  cid_alien = clvec->vid
               ,  sc_alien = sc

            ;  if (sc.sum_i >= 0.5)
               break
         ;  }

            if (loggit)
            mcxLog2
            (  us
            ,  " -> best alien %ld eff %.3f,%.3f sum %.3f"
            ,  cid_alien
            ,  eff_alien_bsf
            ,  eff_alien_max_bsf
            ,  sc_alien.sum_i
            )

                  /* below: use sum_i as mass fraction
                   * (clmAdjust framework uses stochastic * matrix)
                  */
         ;  if
            (  cid_alien >= 0
            && cid_self >= 0
            && f1 * sc_alien.max_i >= sc_self.max_i
            && (  (  eff_alien_bsf > eff_self
                  && sc_alien.sum_i > sc_self.sum_i
                  )
               || (  pow(sc_alien.sum_i, f2) >= sc_self.sum_i
                  && pow(eff_self, f2) <= eff_alien_bsf
                  )
               )
                  /* So, if max is reasonable
                   * and efficiency is better and mass is better
                   * or if mass is ridiculously better -> move
                   * Somewhat intricate and contrived, yes.
                  */
            )
            {  mclv* acceptor
               =  mclxGetVector(cl_adj, cid_alien, RETURN_ON_FAIL, NULL)
            ;  mclv* donor
               =  mclxGetVector(cl_adj, cid_self,  RETURN_ON_FAIL, NULL)
            ;  if (!donor || !acceptor || acceptor == donor)
               continue

            ;  mclvInsertIdx(donor, nid, 0.0)
            ;  mclvInsertIdx(acceptor, nid, 1.0)
            ;  acceptor->val = 1.5

            ;  if (mcxLogGet(MCX_LOG_LIST))
               {  mclv* nb = mx->cols+nos
               ;  double mxv = mclvMaxValue(nb)
               ;  double avg = nb->n_ivps ? mclvSum(nb) / nb->n_ivps : -1.0
               ;  mcxLog
                  (  MCX_LOG_LIST
                  ,  us
                  ,  "mov %ld (%ld %.2f %.2f)"
                     " %ld (cv=%.2f cm=%.2f s=%.2f m=%.2f #=%lu)"
                     " to %ld (cv=%.2f cm=%.2f s=%.2f m=%.2f #=%lu)"
                  ,  nid
                  ,     (long) nb->n_ivps, mxv, avg
                  ,  cid_self
                  ,     eff_self, eff_self_max, sc_self.sum_i, sc_self.max_i
                  ,              (ulong) (sc_self.n_meet + sc_self.n_ddif)
                  ,  cid_alien
                  ,     eff_alien_bsf, eff_alien_max_bsf, sc_alien.sum_i, sc_alien.max_i
                  ,              (ulong) (sc_alien.n_meet + sc_alien.n_ddif)
                  )
            ;  }

               n_adjusted++                  
            ;  mclvInsertIdx(cid_affected, cid_alien, 2.0)
            ;  mclvInsertIdx(cid_affected, cid_self, 2.0)
            ;  mclvInsertIdx(nid_affected, nid, 2.0)
         ;  }
         }
      }
      mclxFree(&el_on_cl)
   ;  mclxFree(&el_to_cl)

   ;  for (i=0;i<N_COLS(cl_adj);i++)
      cl_adj->cols[i].val = 0.0

   ;  mclxMakeCharacteristic(cl)

   ;  if (!n_adjusted)
      {  mclxFree(&cl_adj)
      ;  mclvFree(&cid_affected)
      ;  mclvFree(&nid_affected)
      ;  return 0
   ;  }

      mclxUnary(cl_adj, fltxCopy, NULL)
   ;  mclxMakeCharacteristic(cl_adj)   
                     /* FIRST REMOVE ENTRIES set to zero (sssst now .. */
                     /* ...) and THEN make it characteristic again     */

   ;  mclvUnary(cid_affected, fltxGT, &bar_affected)
   ;  mclvUnary(nid_affected, fltxGT, &bar_affected)

   ;  *cl_adjustedpp  =  cl_adj
   ;  *cid_affectedpp =  cid_affected
   ;  *nid_affectedpp =  nid_affected

   ;  return n_adjusted
;  }


dim clmAdjust
(  mclx* mx
,  mclx* cl
,  dim cls_size_max
,  mclx** cl_adjustedpp
,  mclv** ls_adjustedpp    /* nodes that moved around */
,  dim*  sjd_left
,  dim*  sjd_right
)
   {  dim sum_adjusted = 0, n_ite = 0
   ;  dim dist_curr_adj = 0, dist_adj_curr = 0
   ;  mclx* cl_adj = NULL
   ;  mclx* cl_curr = cl
   ;  mclv* ls_adjusted = mclvInit(NULL)
   ;  clmXScore score_curr, score_adj
   ;  const char* me = "clmAdjust"

   ;  *cl_adjustedpp = NULL
   ;  *ls_adjustedpp = NULL

   ;  while (1)
      {  dim n_adjusted
      ;  double cov_curr, cov_adj, frac_curr = 0.0, frac_adj = 0.0
      ;  mclv* cid_affected = NULL, *nid_affected = NULL
      ;  dim o, m, e

      ;  if (n_ite++ >= 100)
         break

      ;  mclxColumnsRealign(cl_curr, mclvSizeCmp)

      ;  if
         ( !(n_adjusted
         =   clm_clm_adjust
             (mx, cl_curr, cls_size_max, &cl_adj, &cid_affected, &nid_affected)
            )
         )
         break

      ;  mcxTell
         (  me
         ,  "assembled %lu nodes with %lu clusters affected"
         ,  (ulong) n_adjusted
         ,  (ulong) cid_affected->n_ivps
         )

      ;  clmXScanInit(&score_curr)
      ;  clmXScanInit(&score_adj)

      ;  clmXScanDomainSet(mx, cl_curr,cid_affected, &score_curr)
      ;  clmXScanDomainSet(mx, cl_adj, cid_affected, &score_adj)

      ;  clmXScoreCoverage(&score_curr, &cov_curr, NULL)
      ;  clmXScoreCoverage(&score_adj , &cov_adj , NULL)

      ;  if (score_curr.n_hits && score_adj.n_hits)
            frac_curr = score_curr.sum_i / score_curr.n_hits
         ,  frac_adj  = score_adj.sum_i  / score_adj.n_hits

      ;  mcxLog
         (  MCX_LOG_LIST
         ,  me
         ,  "consider (%.5f|%.5f|%lu) vs (%.5f|%.5f|%lu)"
         ,  cov_adj, frac_adj, (ulong) score_adj.n_hits
         ,  cov_curr, frac_curr, (ulong) score_curr.n_hits
         )
                           /* experience tells us that mcl's funneling
                            * worsens frac
                           */
      ;  if (frac_adj <=  frac_curr)
         {  mclvFree(&cid_affected)
         ;  mclvFree(&nid_affected)
         ;  break
      ;  }
         
         clmEnstrict(cl_adj, &o, &m, &e, 0)
      ;  clmSJDistance(cl_curr, cl_adj, NULL, NULL, &dist_curr_adj, &dist_adj_curr)

      ;  mcxLog
         (  MCX_LOG_AGGR
         ,  me
         ,  "distance %lu|%lu"
         ,  (ulong) dist_curr_adj, (ulong) dist_adj_curr
         )

      ;  mclvAdd(ls_adjusted, nid_affected, ls_adjusted)

      ;  if (cl_curr != cl)
         mclxFree(&cl_curr)

      ;  cl_curr = cl_adj
      ;  sum_adjusted += n_adjusted

      ;  mclvFree(&cid_affected)
      ;  mclvFree(&nid_affected)
   ;  }

      if (cl_curr != cl)            /* fixme free logic */
      {  mclxColumnsRealign(cl_curr, mclvSizeRevCmp)
      ;  *cl_adjustedpp = cl_curr
      ;  *ls_adjustedpp = ls_adjusted
      ;  clmSJDistance
         (cl, cl_curr, NULL, NULL, &dist_curr_adj, &dist_adj_curr)
      ;  if (sjd_left)
            *sjd_left  = dist_curr_adj
         ,  *sjd_right = dist_adj_curr
   ;  }
      else
      {  if (sjd_left)
            *sjd_left = 0
         ,  *sjd_right = 0
      ;  mclvFree(&ls_adjusted)
   ;  }

      mcxLog
      (  MCX_LOG_AGGR
      ,  me
      ,  "total adjusted %lu, final distance %lu|%lu"
      ,  (ulong) sum_adjusted 
      ,  (ulong) dist_curr_adj
      ,  (ulong) dist_adj_curr
      )

   ;  mclxColumnsRealign(cl, mclvSizeRevCmp)
   ;  return sum_adjusted
;  }



static dim clm_clm_prune
(  mclx*    mx
,  mclx*    cl
,  dim      prune_sz
,  mclx**   cl_adjustedpp
,  dim*     n_sink
,  dim*     n_source
)
   {  dim d, n_adjusted = 0
   ;  mclx* cl_adj = mclxCopy(cl)
   ;  mclv* cid_affected = mclvClone(cl->dom_cols)
   ;  const char* me = "clmAssimilate"

   ;  double bar_affected = 1.5

   ;  mclx *el_to_cl = NULL
   ;  mclx *el_on_cl = NULL
   ;  mclx *cl_on_cl = NULL
   ;  mclx *cl_on_el = NULL

   ;  *n_sink = 0
   ;  *n_source = 0

   ;  mclvMakeConstant(cid_affected, 1.0)
   ;  mclxColumnsRealign(cl_adj, mclvSizeCmp)

   ;  *cl_adjustedpp = NULL

   ;  clmCastActors
      (&mx, &cl_adj, &el_to_cl, &el_on_cl, &cl_on_cl, &cl_on_el, 0.95)
   ;  mclxFree(&cl_on_el)

   ;  for (d=0;d<N_COLS(cl_on_cl);d++)
      {  mclv* clthis   =  cl_adj->cols+d
      ;  mclv* cllist   =  cl_on_cl->cols+d
      ;  mclp* pself    =  mclvGetIvp(cllist, clthis->vid, NULL)
      ;  double self_val = -1.0
      
      ;  if (pself)
            self_val = pself->val
         ,  pself->val *= 1.001  /* to push it up in case of equal weights */

;if(0)fprintf(stderr, "test size %d\n", (int) clthis->n_ivps)
      ;  if (prune_sz && clthis->n_ivps > prune_sz)
         continue

      ;  while (1)
         {  mclv* clthat
         ;  dim e
         ;  if (cllist->n_ivps < 2)
            break
         ;  mclvSort(cllist, mclpValRevCmp)

                     /* now get biggest mass provided that cluster
                      * ranks higher (has at least as many entries)
                      *
                      * fixme/todo: we probably have a slight order
                      * dependency for some fringe cases. If provable
                      * then either solve or document it.
                     */
         ;  for (e=0;e<cllist->n_ivps;e++)
            if (cllist->ivps[e].idx >= clthis->vid)
            break

                     /* found none or itself */
         ;  if (e == cllist->n_ivps || cllist->ivps[e].idx == clthis->vid)
            break

         ;  if       /* Should Not Happen */
            (!(clthat
            =  mclxGetVector(cl_adj, cllist->ivps[e].idx, RETURN_ON_FAIL, NULL)
            ) )
            break

                     /*    works for special case prune_sz == 0               */
                     /*    if (clthat->n_ivps + clthis->n_ivps > prune_sz)    */
                     /*    ^iced. inconsistent behaviour as k grows.          */
         ;  {  mcxLog
               (  MCX_LOG_LIST
               ,  me
               ,  "source %ld|%lu|%.3f absorbed by %ld|%lu|%.3f"
               ,  clthis->vid, (ulong) clthis->n_ivps, self_val
               ,  clthat->vid, (ulong) clthat->n_ivps, cllist->ivps[0].val
               )
            ;  n_adjusted += clthis->n_ivps
            ;  (*n_sink)++
                     /* note: we could from our precomputed cl_on_cl
                      * obtain that A is absorbed in B, B is absorbed in C.
                      * below we see that A will be merged with B,
                      * and the result will then be merged with C.
                      * This depends on the fact that cl_adj is ordered
                      * on increasing cluster size.
                     */
            ;  mcldMerge(cl_adj->cols+d, clthat, clthat)
            ;  mclvResize(cl_adj->cols+d, 0)
            ;  mclvInsertIdx(cid_affected, clthat->vid, 2.0)
         ;  }
            break
      ;  }
         mclvSort(cllist, mclpIdxCmp)
   ;  }

      mclxFree(&cl_on_cl)
   ;  mclxFree(&el_on_cl)
   ;  mclxFree(&el_to_cl)

   ;  mclxMakeCharacteristic(cl)

   ;  mclvUnary(cid_affected, fltxGT, &bar_affected)
   ;  *n_source = cid_affected->n_ivps
   ;  mclvFree(&cid_affected)

   ;  mclxColumnsRealign(cl_adj, mclvSizeRevCmp)

   ;  if (!n_adjusted)
      {  mclxFree(&cl_adj)
      ;  return 0
   ;  }

      mclxUnary(cl_adj, fltxCopy, NULL)
   ;  mclxMakeCharacteristic(cl_adj)   

   ;  *cl_adjustedpp  =  cl_adj
   ;  return n_adjusted
;  }



dim clmAssimilate
(  mclx* mx
,  mclx* cl
,  dim   prune_sz
,  mclx** cl_prunedpp
,  dim*  sjd_left
,  dim*  sjd_right
)
   {  dim dist_this_pru = 0, dist_pru_this = 0
   ;  mclx* cl_pru = NULL
   ;  const char* me = "clmAssimilate"

   ;  dim o, m, e, n_source, n_sink

   ;  dim sum_pruned = clm_clm_prune
      (mx, cl, prune_sz, &cl_pru, &n_source, &n_sink)

   ;  *cl_prunedpp = NULL

   ;  if (sum_pruned)
      {  mcxLog
         (  MCX_LOG_AGGR
         ,  me
         ,  "funneling %lu nodes from %lu sources into %lu targets"
         ,  (ulong) sum_pruned
         ,  (ulong) n_source
         ,  (ulong) n_sink
         )

      ;  clmEnstrict(cl_pru, &o, &m, &e, 0)
      ;  *cl_prunedpp = cl_pru

      ;  clmSJDistance
         (cl, cl_pru, NULL, NULL, &dist_this_pru, &dist_pru_this)
      ;  if (sjd_left)
            *sjd_left  = dist_this_pru
         ,  *sjd_right = dist_pru_this
   ;  }
      else if (sjd_left)
         *sjd_left = 0
      ,  *sjd_right = 0

   ;  mcxLog
      (  MCX_LOG_AGGR
      ,  me
      ,  "dim %lu pruned %lu distance %lu|%lu"
      ,  (ulong) N_COLS(mx)
      ,  (ulong) sum_pruned 
      ,  (ulong) dist_this_pru
      ,  (ulong) dist_pru_this
      )

   ;  return sum_pruned
;  }


