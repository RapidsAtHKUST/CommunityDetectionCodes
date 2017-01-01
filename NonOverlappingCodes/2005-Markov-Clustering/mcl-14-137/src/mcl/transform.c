/*   (C) Copyright 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <ctype.h>
#include <string.h>

#include "transform.h"

#include "impala/compose.h"
#include "impala/edge.h"

#include "util/compile.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/err.h"
#include "util/io.h"
#include "util/ting.h"
#include "util/ding.h"

#include "mcl/alg.h"
#include "mcl/proc.h"



struct mclg_transform
{  mclpAR* par_edge
;  mclpAR* par_graph
;
}  ;


enum
{  MCLG_TF_CEILNB = 0
,  MCLG_TF_KNN
,  MCLG_TF_KNNJ
,  MCLG_TF_RANK
,  MCLG_TF_MCL
,  MCLG_TF_SYM_MCL
   ,  MCLG_TF_DUMMY_NOVALUE_START
      ,  MCLG_TF_ILS                /* inverse log-weighted similarity */
      ,  MCLG_TF_ADD
      ,  MCLG_TF_MAX
      ,  MCLG_TF_MIN
      ,  MCLG_TF_MUL
      ,  MCLG_TF_ARCMAX             /* keep the larger of two arcs */
      ,  MCLG_TF_ARCSUB             /* add -M^T */
      ,  MCLG_TF_NORMSELF           /* scale by self, max if not found */
      ,  MCLG_TF_SELFRM
      ,  MCLG_TF_SELFMAX
      ,  MCLG_TF_TRANSPOSE
   ,  MCLG_TF_DUMMY_NOVALUE_END
,  MCLG_TF_TUG
,  MCLG_TF_SSQ
,  MCLG_TF_ARCMAXGQ                 /* keep arc pair if max exceeds value */
,  MCLG_TF_ARCMAXGT
,  MCLG_TF_ARCMAXLQ
,  MCLG_TF_ARCMAXLT
,  MCLG_TF_ARCMINGQ
,  MCLG_TF_ARCMINGT
,  MCLG_TF_ARCMINLQ
,  MCLG_TF_ARCMINLT
,  MCLG_TF_ARCDIFFGQ
,  MCLG_TF_ARCDIFFGT
,  MCLG_TF_ARCDIFFLQ
,  MCLG_TF_ARCDIFFLT
,  MCLG_TF_QT
,  MCLG_TF_SHRUG
,  MCLG_TF_STEP
,  MCLG_TF_THREAD
,  MCLG_TF_SHUFFLE
}  ;


mclgTF* mclgTFparse
(  mcxLink*    encoding_link
,  mcxTing*    thestring
)
   {  mclgTF* gtf = mcxAlloc(sizeof gtf[0], EXIT_ON_FAIL)
   ;  const char* me = "mclgTFparse"
   ;  const char* a  = thestring->str
   ;  const char* z  = thestring->str + thestring->len
   ;  mcxTing* func  =  mcxTingEmpty(NULL, thestring->len)
   ;  mcxTing* arg   =  mcxTingEmpty(NULL, thestring->len)
   ;  int n = 0

   ;  if (!(gtf->par_edge = mclpARensure(NULL, 10)))
      return NULL     /* +memleak gtf */

   ;  if (!(gtf->par_graph = mclpARensure(NULL, 10)))
      return NULL     /* +memleak gtf, gtf->par_edge */

   ;  if
      (  thestring
      && !mcxStrChrAint(thestring->str, isspace, thestring->len)
      )
      return gtf

   ;  while (a < z)
      {  const char* val, *key
      ;  char* onw = NULL
      ;  int tfe = -1, tfg = -1
      ;  mcxbool nought = FALSE
      ;  unsigned char k0
      ;  double d
      ;  int t

      ;  mcxTingEmpty(arg, z-a)
      ;  mcxTingEmpty(func, z-a)
      ;  n = 0

      ;  if ((t = sscanf(a, " %[a-z_@#-] ( )%n", func->str, &n)) >= 1 && n > 0)
         NOTHING
      ;  else if ((t = sscanf(a, " %[a-z_@#-] ( %[^)_ ] )%n", func->str, arg->str, &n)) >= 2 && n > 0)
         NOTHING
      ;  else
         break

      ;  a += n  
      ;  key=  func->str
      ;  val=  arg->str
      ;  k0 =  key[0]
      ;  d  =  strtod(val, &onw)

      ;  if (!val || !strlen(val))
         nought = TRUE

      ;  else if (val == onw)
         {  mcxErr(me, "failed to parse number <%s>", val)
         ;  break
      ;  }

         if (k0 == '#' || k0 == '@')
         {  if (!strcmp(key+1, "ceilnb"))
            tfg = MCLG_TF_CEILNB
         ;  else if (!strcmp(key+1, "knn"))
            tfg = MCLG_TF_KNN
         ;  else if (!strcmp(key+1, "knnj"))
            tfg = MCLG_TF_KNNJ
         ;  else if (!strcmp(key+1, "rank"))
            tfg = MCLG_TF_RANK
         ;  else if (!strcmp(key+1, "ils"))
            tfg = MCLG_TF_ILS
         ;  else if (!strcmp(key+1, "mcl"))
            tfg = MCLG_TF_MCL
         ;  else if (!strcmp(key+1, "symmcl"))
            tfg = MCLG_TF_SYM_MCL

         ;  else if (!strcmp(key+1, "arcsub"))
            tfg = MCLG_TF_ARCSUB
         ;  else if (!strcmp(key+1, "arcmax"))
            tfg = MCLG_TF_ARCMAX

         ;  else if (!strcmp(key+1, "arcmingq"))
            tfg = MCLG_TF_ARCMINGQ
         ;  else if (!strcmp(key+1, "arcmingt"))
            tfg = MCLG_TF_ARCMINGT
         ;  else if (!strcmp(key+1, "arcmimlq"))
            tfg = MCLG_TF_ARCMINLQ
         ;  else if (!strcmp(key+1, "arcminlt"))
            tfg = MCLG_TF_ARCMINLT

         ;  else if (!strcmp(key+1, "arcdiffgq"))
            tfg = MCLG_TF_ARCDIFFGQ
         ;  else if (!strcmp(key+1, "arcdiffgt"))
            tfg = MCLG_TF_ARCDIFFGT
         ;  else if (!strcmp(key+1, "arcdifflq"))
            tfg = MCLG_TF_ARCDIFFLQ
         ;  else if (!strcmp(key+1, "arcdifflt"))
            tfg = MCLG_TF_ARCDIFFLT

         ;  else if (!strcmp(key+1, "arcmaxgq"))
            tfg = MCLG_TF_ARCMAXGQ
         ;  else if (!strcmp(key+1, "arcmaxgt"))
            tfg = MCLG_TF_ARCMAXGT
         ;  else if (!strcmp(key+1, "arcmaxlq"))
            tfg = MCLG_TF_ARCMAXLQ
         ;  else if (!strcmp(key+1, "arcmaxlt"))
            tfg = MCLG_TF_ARCMAXLT

         ;  else if (!strcmp(key+1, "selfrm"))
            tfg = MCLG_TF_SELFRM
         ;  else if (!strcmp(key+1, "selfmax"))
            tfg = MCLG_TF_SELFMAX
         ;  else if (!strcmp(key+1, "normself"))
            tfg = MCLG_TF_NORMSELF

         ;  else if (!strcmp(key+1, "add"))
            tfg = MCLG_TF_ADD
         ;  else if (!strcmp(key+1, "max"))
            tfg = MCLG_TF_MAX
         ;  else if (!strcmp(key+1, "min"))
            tfg = MCLG_TF_MIN
         ;  else if (!strcmp(key+1, "mul"))
            tfg = MCLG_TF_MUL

         ;  else if (!strcmp(key+1, "tug"))
            tfg = MCLG_TF_TUG
         ;  else if (!strcmp(key+1, "ssq"))
            tfg = MCLG_TF_SSQ
         ;  else if (!strcmp(key+1, "qt"))
            tfg = MCLG_TF_QT
         ;  else if (!strcmp(key+1, "tp") || !strcmp(key+1, "rev"))
            tfg = MCLG_TF_TRANSPOSE
         ;  else if (!strcmp(key+1, "step"))
            tfg = MCLG_TF_STEP
         ;  else if (!strcmp(key+1, "hread"))
            tfg = MCLG_TF_THREAD
         ;  else if (!strcmp(key+1, "shrug"))
            tfg = MCLG_TF_SHRUG
         ;  else if (!strcmp(key+1, "shuffle"))
            tfg = MCLG_TF_SHUFFLE
      ;  }
         else
         {  if (!strcmp(key, "gq"))
            tfe = MCLX_UNARY_GQ
         ;  else if (!strcmp(key, "gt"))
            tfe = MCLX_UNARY_GT
         ;  else if (!strcmp(key, "lt"))
            tfe = MCLX_UNARY_LT
         ;  else if (!strcmp(key, "lq"))
            tfe = MCLX_UNARY_LQ
         ;  else if (!strcmp(key, "rand"))
            tfe = MCLX_UNARY_RAND
         ;  else if (!strcmp(key, "mul"))
            tfe = MCLX_UNARY_MUL
         ;  else if (!strcmp(key, "scale"))
            tfe = MCLX_UNARY_SCALE
         ;  else if (!strcmp(key, "add"))
            tfe = MCLX_UNARY_ADD
         ;  else if (!strcmp(key, "abs"))
            tfe = MCLX_UNARY_ABS
         ;  else if (!strcmp(key, "ceil"))
            tfe = MCLX_UNARY_CEIL
         ;  else if (!strcmp(key, "acos"))
            tfe = MCLX_UNARY_ACOS
         ;  else if (!strcmp(key, "floor"))
            tfe = MCLX_UNARY_FLOOR
         ;  else if (!strcmp(key, "pow"))
            tfe = MCLX_UNARY_POW
         ;  else if (!strcmp(key, "exp"))
            tfe = MCLX_UNARY_EXP
         ;  else if (!strcmp(key, "log"))
            tfe = MCLX_UNARY_LOG
         ;  else if (!strcmp(key, "neglog"))
            tfe = MCLX_UNARY_NEGLOG
      ;  }

         if (tfe < 0 && tfg < 0)
         {  mcxErr(me, "unknown value transform <%s>", key)
         ;  break
      ;  }

         if (tfe >= 0)
         {  if (nought)
            {  if
               (  tfe == MCLX_UNARY_LOG || tfe == MCLX_UNARY_ABS
               || tfe == MCLX_UNARY_EXP || tfe == MCLX_UNARY_NEGLOG
               || tfe == MCLX_UNARY_ACOS
               )
               d = 0.0
            ;  else
               {  mcxErr(me, "transform <%s> needs value", key)
               ;  break
            ;  }
         ;  }
            mclpARextend(gtf->par_edge, tfe, d)
      ;  }
         else if (tfg >= 0)
         {  if (nought)
            {  if
               (  tfg >= MCLG_TF_DUMMY_NOVALUE_START
               && tfg <= MCLG_TF_DUMMY_NOVALUE_END
               )
               d = 0.0
            ;  else if (tfg == MCLG_TF_TUG || tfg == MCLG_TF_SHRUG)
               d = 1000.0
            ;  else if (tfg == MCLG_TF_STEP)
               d = 2.0
            ;  else
               {  mcxErr(me, "transform <%s> needs value", key)
               ;  break
            ;  }
         ;  }
            mclpARextend(gtf->par_edge, MCLX_UNARY_UNUSED, 0.0)
         ;  mclpARextend(gtf->par_graph, tfg, d)
      ;  }

         a = mcxStrChrAint(a, isspace, z-a)
      ;  if (!a || a[0] != ',')
         break
      ;  a++
   ;  }

      if (a)
      {  mcxErr(me, "trailing part <%s> not matched", a)
      ;  mclpARfree(&(gtf->par_edge))
      ;  mcxFree(gtf)
      ;  gtf = NULL
   ;  }

      return gtf
;  }



static void mclg_tf_step
(  mclx* mx
,  dim i
)
   {  dim j
   ;  mclx* input = mx
   ;  for (j=0;j<i;j++)
      {  mclx* st = mclxCompose(mx, mx, 0, 0)
      ;  if (j)
         mclxFree(&mx)
      ;  mx = st
   ;  }
      if (i)
      mclxTransplant(input, &mx)
;  }


static void tf_ssq
(  mclx* mx
,  double val
)
   {  dim i
   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  double ssq = mclvPowSum(v, 2.0)
      ;  double sum = mclvSum(v)
      ;  double self = mclvSelf(v)
      ;  if (sum-self)
         mclvSelectGtBar(v, val * (ssq - self*self) / (sum - self))
   ;  }
;  }


static void tf_do_mcl
(  mclx* mx
,  double infl
,  mcxbool add_transpose
)
   {  mclx* mx2 = NULL, *mx3 = NULL, *cl = NULL
   ;  mclAlgParam* mlp = NULL
   ;  char* argv2[] =  { NULL }
   ;  mcxstatus s

   ;  if (add_transpose)
      {  mx2 = mclxCopy(mx)
      ;  mclxAddTranspose(mx2, 0.0)
   ;  }

      s  =  mclAlgInterface
            (  &mlp
            ,  argv2
            ,  0
            ,  NULL
            ,  add_transpose ? mx2 : mx
            ,  ALG_CACHE_INPUT 
            )
   ;  do
      {  if (s)
         {  mcxErr("tf-mcl", "unexpected failure")
         ;  break
      ;  }
         mlp->mpp->mainInflation = infl   
      ;  if (mclAlgorithm(mlp) == STATUS_FAIL)
         break
      ;  if (!(cl = mclAlgParamRelease(mlp, mlp->cl_result)))
         break
      ;  mclAlgParamRelease(mlp, mlp->mx_input)    /* now we own it again, either mx2 or mx */
      ;  mx3 = mclxBlockUnion2(mx, cl)
      ;  mclxTransplant(mx, &mx3)      /* this frees mx3 */
   ;  }
      while (0)
   ;  mclxFree(&cl)
   ;  mclxFree(&mx2)
   ;  mclAlgParamFree(&mlp, TRUE)
;  }


void mclgTFgraph
(  mclx* mx
,  pnum  mode
,  pval  val
)
   {  switch(mode)
      {        case MCLG_TF_MAX:       mclxMergeTranspose(mx, fltMax, 1.0)
   ;  break ;  case MCLG_TF_MIN:       mclxMergeTranspose(mx, fltMin, 1.0)
   ;  break ;  case MCLG_TF_ADD:       mclxMergeTranspose(mx, fltAdd, 1.0)
   ;  break ;  case MCLG_TF_SELFRM:    mclxAdjustLoops(mx, mclxLoopCBremove, NULL)
   ;  break ;  case MCLG_TF_SELFMAX:   mclxAdjustLoops(mx, mclxLoopCBmax, NULL)
   ;  break ;  case MCLG_TF_NORMSELF:  mclxNormSelf(mx)
   ;  break ;  case MCLG_TF_MUL:       mclxMergeTranspose(mx, fltMultiply, 1.0)
   ;  break ;  case MCLG_TF_ARCMAX:    mclxMergeTranspose(mx, fltArcMax, 1.0)

   ;  break ;  case MCLG_TF_ARCMAXGQ:  mclxMergeTranspose3(mx, fltArcMaxGQ, 1.0, val)
   ;  break ;  case MCLG_TF_ARCMAXGT:  mclxMergeTranspose3(mx, fltArcMaxGT, 1.0, val)
   ;  break ;  case MCLG_TF_ARCMAXLQ:  mclxMergeTranspose3(mx, fltArcMaxLQ, 1.0, val)
   ;  break ;  case MCLG_TF_ARCMAXLT:  mclxMergeTranspose3(mx, fltArcMaxLT, 1.0, val)

   ;  break ;  case MCLG_TF_ARCMINGQ:  mclxMergeTranspose3(mx, fltArcMinGQ, 1.0, val)
   ;  break ;  case MCLG_TF_ARCMINGT:  mclxMergeTranspose3(mx, fltArcMinGT, 1.0, val)
   ;  break ;  case MCLG_TF_ARCMINLQ:  mclxMergeTranspose3(mx, fltArcMinLQ, 1.0, val)
   ;  break ;  case MCLG_TF_ARCMINLT:  mclxMergeTranspose3(mx, fltArcMinLT, 1.0, val)

   ;  break ;  case MCLG_TF_ARCDIFFGQ: mclxMergeTranspose3(mx, fltArcDiffGQ, 1.0, val)
   ;  break ;  case MCLG_TF_ARCDIFFGT: mclxMergeTranspose3(mx, fltArcDiffGT, 1.0, val)
   ;  break ;  case MCLG_TF_ARCDIFFLQ: mclxMergeTranspose3(mx, fltArcDiffLQ, 1.0, val)
   ;  break ;  case MCLG_TF_ARCDIFFLT: mclxMergeTranspose3(mx, fltArcDiffLT, 1.0, val)

   ;  break ;  case MCLG_TF_ARCSUB:    mclxMergeTranspose(mx, fltSubtract, 1.0)
   ;  break ;  case MCLG_TF_TUG:       mclxPerturb(mx, val, MCLX_PERTURB_SYMMETRIC)
   ;  break ;  case MCLG_TF_TRANSPOSE: { mclx* tp = mclxTranspose(mx); mclxTransplant(mx, &tp); }
   ;  break ;  case MCLG_TF_SHRUG:     mclxPerturb(mx, val, MCLX_PERTURB_SYMMETRIC | MCLX_PERTURB_RAND)
   ;  break ;  case MCLG_TF_ILS:       mclxILS(mx)
   ;  break ;  case MCLG_TF_KNNJ:      if (val) mclgKNNdispatch(mx, val+0.5, mclx_n_thread_g, KNN_JOIN)
   ;  break ;  case MCLG_TF_KNN:       if (val) mclgKNNdispatch(mx, val+0.5, mclx_n_thread_g, KNN_INTERSECT)
   ;  break ;  case MCLG_TF_RANK:      if (val) mclgKNNdispatch(mx, val+0.5, mclx_n_thread_g, KNN_SELECT_ONLY)
   ;  break ;  case MCLG_TF_MCL:       tf_do_mcl(mx, val, FALSE)
   ;  break ;  case MCLG_TF_SYM_MCL:   tf_do_mcl(mx, val, TRUE)
   ;  break ;  case MCLG_TF_THREAD:    mclx_n_thread_g = val + 0.5
   ;  break ;  case MCLG_TF_CEILNB:    { mclv* cv = mclgCeilNB(mx, val+0.5, NULL, NULL, NULL); mclvFree(&cv); }
   ;  break ;  case MCLG_TF_STEP:      mclg_tf_step(mx, val+0.5)
   ;  break ;  case MCLG_TF_QT:        mclxQuantiles(mx, val)
   ;  break ;  case MCLG_TF_SSQ:       tf_ssq(mx, val)
   ;  break ;  case MCLG_TF_SHUFFLE:   mcxErr("mclgTFgraph", "shuffle not yet done (lift from mcxrand)")
   ;  break ;  default:                mcxErr("mclgTFgraph", "unknown mode")
   ;  break
   ;  }
   }


dim mclgTFexecx
(  mclx*    mx
,  mclgTF*  tf
,  mcxbool  allow_graph_ops
)
   {  dim offset = 0, k = 0
   ;  mclpAR* par_edge = tf->par_edge
   ;  mclpAR* par_graph = tf->par_graph
   ;  while (offset < par_edge->n_ivps || k < par_graph->n_ivps)
      {  dim top = offset
      ;  while (top < par_edge->n_ivps && par_edge->ivps[top].idx != MCLX_UNARY_UNUSED)
         top++
      ;  if (top > offset)
         {  mclpAR* par = mclpARfromIvps(NULL, par_edge->ivps+offset, top-offset)
/* fprintf(stderr, "unary exec %d %d\n", (int) offset, (int) top) */
         ;  mclxUnaryList(mx, par)
         ;  mclpARfree(&par)
      ;  }

         if (par_edge->ivps[top].idx == MCLX_UNARY_UNUSED)
         {  if (k >= par_graph->n_ivps)
            {  mcxErr("mclgTFexec", "off the rails")
            ;  break
         ;  }
            if (allow_graph_ops)
            mclgTFgraph(mx, par_graph->ivps[k].idx, par_graph->ivps[k].val)
         ;  k++
/* fprintf(stderr, "graph %s top=%d k=%d\n", allow_graph_ops ? "exec" : "skip", (int) offset, (int) k) */
      ;  }
         offset = top+1
   ;  }
      return mclxNrofEntries(mx)
;  }


dim mclgTFexec
(  mclx*    mx
,  mclgTF*  tf
)
   {  return mclgTFexecx(mx, tf, TRUE)
;  }


void mclgTFfree
(  mclgTF** tfpp
)
   {  mclgTF* tf = tfpp[0]
   ;  if (tf)
      {  mclpARfree(&(tf->par_edge))
      ;  mcxFree(tf)
      ;  tfpp[0] = NULL
   ;  }
   }

mclpAR* mclgTFgetEdgePar
(  mclgTF* tf
)
   {  return tf ? tf->par_edge : NULL
;  }


