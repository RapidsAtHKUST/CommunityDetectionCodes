/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <time.h>
#include <signal.h>
#include <string.h>

#include "proc.h"
#include "dpsd.h"
#include "expand.h"
#include "inflate.h"
#include "procinit.h"

#include "clew/clm.h"
#include "clew/cat.h"

#include "impala/io.h"
#include "impala/matrix.h"

#include "util/ting.h"
#include "util/equate.h"
#include "util/err.h"
#include "util/io.h"
#include "util/types.h"
#include "util/alloc.h"
#include "util/minmax.h"
#include "util/compile.h"

#define ITERATION_INITIAL  1
#define ITERATION_MAIN     2


static volatile sig_atomic_t abort_loop = 0;

static int mclVerbosityStart    =  0;        /* mq static */


void  mclDumpVector
(  mclProcParam*  mpp
,  const char*    affix
,  const char*    pfix
,  int            n
,  mcxbool        print_value
)  ;


int doIteration
(  const mclx*    mxstart
,  mclx**         mxin
,  mclx**         mxout
,  mclProcParam*  mpp
,  int            type
)  ;


void mclSigCatch
(  int sig
)
   {  if (sig == SIGALRM)
      abort_loop = 1
;  }


mclProcParam* mclProcParamNew
(  void
)  
   {  mclProcParam* mpp    =  (mclProcParam*)
                              mcxAlloc(sizeof(mclProcParam), EXIT_ON_FAIL)
   ;  int i

   ;  mpp->mxp             =  mclExpandParamNew()
   ;  mpp->ipp             =  mclInterpretParamNew()

   ;  mpp->n_ithreads      =  0
   ;  mpp->fname_expanded  =  NULL

   ;  for (i=0;i<5;i++)
      mpp->marks[i]        =  100

   ;  mpp->devel           =  0

   ;  mpp->dumping         =  0
   ;  mpp->dump_modulo     =  1
   ;  mpp->dump_offset     =  0
   ;  mpp->dump_bound      =  5
   ;  mpp->dump_stem       =  mcxTingNew("")
   ;  mpp->dump_tab        =  NULL

   ;  mpp->chaosLimit      =  0.0001
   ;  mpp->lap             =  0.0
   ;  mpp->n_ite           =  0

   ;  mpp->vec_attr        =  NULL

   ;  mpp->mainInflation   =  2
   ;  mpp->mainLoopLength  =  10000
   ;  mpp->initInflation   =  2
   ;  mpp->initLoopLength  =  0

   ;  mpp->printDigits     =  3
   ;  mpp->printMatrix     =  0
   ;  mpp->expansionVariant=  0
   ;  mpp->n_entries       =  0

   ;  mpp->dimension       =  0
   ;  return mpp
;  }


void mclProcParamFree
(  mclProcParam** ppp
)
   {  mclProcParam* mpp = *ppp
   ;  mclExpandParamFree(&(mpp->mxp))
   ;  mclInterpretParamFree(&(mpp->ipp))
   ;  mcxTingFree(&(mpp->dump_stem))
   ;  mcxFree(mpp)
   ;  *ppp = NULL
;  }


mclMatrix*  mclProcess
(  mclMatrix** mxstart
,  mclProcParam* mpp
,  mcxbool constmx            /* if true do not free *mxstart */
,  mclx**  cachexp            /* if !NULL cache expanded */
,  mclx**  limit
)
   {  mclMatrix*        mxIn        =  *mxstart
   ;  mclMatrix*        mxOut       =  NULL
   ;  mclMatrix*        mxCluster   =  NULL
   ;  int               digits      =  mpp->printDigits
   ;  mclExpandParam    *mxp        =  mpp->mxp
   ;  int               i           =  0
   ;  clock_t           t1          =  clock()
   ;  const char* me                =  "mclProcess"
   ;  FILE*             fplog       =  mcxLogGetFILE()

   ;  if (cachexp)
      *cachexp =  NULL
   ;  if (limit)
      *limit   =  NULL
                                       /* mq check memleak for param and stats
                                        * structs and members
                                       */

   ;  if (!mxp->stats)                 /* size dependent init stuff */
      mclExpandParamDim(mxp, mxIn)

   ;  mpp->n_entries = mclxNrofEntries(mxstart[0])

   ;  if (mpp->printMatrix)
      mclFlowPrettyPrint
      (  mxIn
      ,  stdout
      ,  digits
      ,  "1 After centering (if) and normalization"
      )

   ;  if (MCPVB(mpp, MCPVB_ITE))
      mclDumpMatrix(mxIn, mpp, "ite", "", 0, TRUE)

               /* see below, mainLoopLength, for discussion of parameters */
   ;  for (i=0;i<mpp->initLoopLength;i++)
      {  doIteration 
         (  mxstart[0]
         ,  &mxIn
         ,  &mxOut
         ,  mpp
         ,  ITERATION_INITIAL
         )

      ;  if
         (  (i == 0 && !constmx && !mpp->expansionVariant)
         || (i == 1 && !cachexp)
         ||  i > 1
         )
         mclxFree(&mxIn)
      ;  else if (i == 1 && cachexp)
         *cachexp = mxIn

      ;  mpp->n_ite++
      ;  mxIn  =  mxOut
   ;  }

      if (mpp->initLoopLength)
      mcxLog
      (  MCX_LOG_MODULE
      ,  me
      ,  "====== Changing from initial to main inflation now ======"
      )

            /* initially &mxOut[0] == NULL, and &mxIn[0] == mxstart[0]
             * doIteration writes in mxOut.
             * On later occassions &mxIn will be an intermediate mcl iterand.
             * we can free mxIn provided it is not the start matrix when it needs caching,
             * and it is not the expanded matrix that needs caching.
            */
   ;  for (i=0;i<mpp->mainLoopLength;i++)
      {  int convergence
         =  doIteration
            (  mxstart[0]
            ,  &mxIn
            ,  &mxOut
            ,  mpp
            ,  ITERATION_MAIN
            )

      ;  if
         (  mpp->initLoopLength
         || (i == 0 && !constmx && !mpp->expansionVariant)
         || (i == 1 && !cachexp)
         ||  i > 1
         )
         mclxFree(&mxIn)
      ;  else if (i == 1 && cachexp)
         *cachexp = mxIn

      ;  mpp->n_ite++
      ;  mxIn  =  mxOut

      ;  if (abort_loop || convergence)
         break
   ;  }

      if (cachexp && ! *cachexp)
      *cachexp = mxOut

   ;  mpp->lap = ((double) (clock() - t1)) / CLOCKS_PER_SEC

   ;  *limit = mxIn

   ;  {  mclx* dag = mclDag(mxIn, mpp->ipp)
      ;  if (1)  /* hum ho fixme docme. is-this-really-necessary-or-is-it-a-debug-remnant ? */
         {  dim j
         ;  mclxMakeStochastic(dag)
         ;  for (j=0;j<N_COLS(dag);j++)
            mclvSelectGqBar(dag->cols+j, 1.0 / (dag->cols[j].n_ivps + 1))
      ;  }
         mxCluster = mclInterpret(dag)
      ;  mclxFree(&dag)
   ;  }

      return mxCluster
;  }


void mclInflate
(  mclx*    mx
,  double   power
,  mclv*    homgVec
)
   {  mcxbool   local   cpl__unused = getenv("MCL_AUTO_LOCAL") ? TRUE : FALSE
   ;  mcxbool   smooth  cpl__unused = getenv("MCL_AUTO_SMOOTH") ? TRUE : FALSE  

   ;  double    infl   =   power
   ;  mclv*     vec_infl = NULL
   ;  dim k

   ;  for (k=0;k<N_COLS(mx);k++)
      mclvInflate(mx->cols+k, vec_infl ? vec_infl->ivps[k].val : infl)

   ;  mclvFree(&vec_infl)
;  }



int doIteration
(  const mclx*          mxstart
,  mclx**               mxin
,  mclx**               mxout
,  mclProcParam*        mpp
,  int                  type
)
   {  int               digits         =  mpp->printDigits
   ;  mclExpandParam*   mxp            =  mpp->mxp  
   ;  mclExpandStats*   stats          =  mxp->stats
   ;  int               bInitial       =  (type == ITERATION_INITIAL)
   ;  const char        *when          =  bInitial ? "initial" : "main"
   ;  dim               n_ite          =  mpp->n_ite
   ;  char              msg[80]
   ;  FILE*             fplog          =  mcxLogGetFILE()
   ;  double            inflation      =  bInitial
                                          ?  mpp->initInflation
                                          :  mpp->mainInflation
   ;  mcxbool           log_gauge      =  mcxLogGet(MCX_LOG_GAUGE) 
   ;  mcxbool           log_stats      =  XPNVB(mxp, XPNVB_CLUSTERS)
   ;  double            homgAvg
   ;  mclv*             homgVec
   ;  int               n_cols         =  N_COLS(*mxin)
   ;  dim               n_expand_entries = 0
   ;  dim               n_graph_entries = mclxNrofEntries(mxin[0])
   ;  dim               n_new_entries  =  0
   ;  dim i

   ;  mxp->inflation = inflation

   ;  if (mclVerbosityStart == 0)
      {  if (log_gauge)
         {  fprintf(fplog, " ite ")
         ;  if (!mxp->n_ethreads)
            for (i=0;i<n_cols/mxp->vector_progression;i++)
            fputc('-', fplog)
         ;  fputs("  chaos  time hom(avg,lo,hi) m-ie m-ex i-ex fmv", fplog)
         ;  if (log_stats)
            fputs("   E/V  dd    cls   olap avg", fplog)
         ;  fputc('\n', fplog)
      ;  }
         mclVerbosityStart = 1
   ;  }

      if (log_gauge)
      fprintf(fplog, "%3d  ", (int) n_ite+1)

;if(0)mclxDebug("-", mxin[0], 3, "mxin")
;if(0)mclxDebug("-", mpp->expansionVariant ? mxstart : mxin[0], 3, "mxstart")
   ;  *mxout  =   mclExpand(*mxin, mpp->expansionVariant ? mxstart : *mxin,  mxp)
;if(0)fprintf(stdout, "------\n")
   ;  homgAvg =   mxp->stats->homgAvg

   ;  n_new_entries = mclxNrofEntries(mxout[0])

   ;  homgVec = mxp->stats->homgVec
   ;  mxp->stats->homgVec = NULL       /* fixme ugly ownership */

   ;  for (i=0;i<N_COLS(mxout[0]);i++)
      n_expand_entries += mxp->stats->bob_expand[i]

   ;  if (n_ite < 5)
      {  dim z
      ;  mcxHeap* h  =  mcxHeapNew(NULL, n_cols ? MCX_MIN(1000, n_cols) : 1, sizeof(float), fltCmp)
      ;  float*   f  =  h->base
      ;  double mean =  0.0

      ;  for (z=0;z<n_cols;z++)
         mcxHeapInsert(h, mxp->stats->bob_final+z)

      ;  for (z=0;z<h->n_inserted;z++)
         mean += f[z]

      ;  if (h->n_inserted)
         mpp->marks[n_ite] = mean * 100.0001 / h->n_inserted
      ;  mcxHeapFree(&h)
   ;  }

      if (log_gauge)
      fprintf
      (  fplog
      ,  " %6.2f %5.2f %.2f/%.2f/%.2f %.2f %.2f %.2f %3d"
      ,  (double) stats->chaosMax
      ,  (double) stats->lap
      ,  (double) stats->homgAvg
      ,  (double) stats->homgMin
      ,  (double) stats->homgMax
      /*
      ,  (double) n_graph_entries
      ,  (double) n_expand_entries
      ,  (double) n_new_entries
      */
      ,  (double) ((1.0 * n_expand_entries) / (n_graph_entries+1))
      ,  (double) ((1.0 * n_new_entries) / (n_graph_entries+1))
      ,  (double) ((1.0 * n_new_entries) / (mpp->n_entries+1))
      ,  (int) ((100.0 * stats->bob_sparse) / N_COLS(mxout[0]))
      )

   ;  if (log_stats || MCPVB(mpp, (MCPVB_CLUSTERS | MCPVB_DAG)))
      {  dim o, m, e
      ;  mclMatrix* dag  = mclDag(*mxout, mpp->ipp)
      ;  mclMatrix* clus = mclInterpret(dag)
      ;  dim clm_stats[N_CLM_STATS]
      ;  int dagdepth = mclDagTest(dag)

      ;  clmStats(clus, clm_stats)

#if 0
;mcxIO *xfstdout = mcxIOnew("-", "w")
#endif
      ;  clmEnstrict(clus, &o, &m, &e, ENSTRICT_KEEP_OVERLAP)
      ;  if (log_stats)
         fprintf
         (  fplog
         ,  "%6.0f %2d %7lu %6lu %3.1f"
         ,     N_COLS(*mxout)
            ? (double) mclxNrofEntries(*mxout) / N_COLS(*mxout)
            :  0.0
         ,  dagdepth
         ,  (ulong) clm_stats[CLM_STAT_CLUSTERS]
         ,  (ulong) clm_stats[CLM_STAT_NODES_OVERLAP]
         ,  (double)
               (  clm_stats[CLM_STAT_NODES_OVERLAP]
               ?  clm_stats[CLM_STAT_SUM_OVERLAP] * 1.0 / clm_stats[CLM_STAT_NODES_OVERLAP]
               :  0.0
               )
         )
      ;  if (m+e)
         fprintf(fplog, " [!m=%lu e=%lu]", (ulong) m, (ulong) e)
#if 0
;if (o)
mclxWrite(*mxout, xfstdout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
#endif
      ;  if (MCPVB(mpp, MCPVB_CLUSTERS))
         mclDumpMatrix(clus, mpp, "cls", "", n_ite+1, FALSE)
      ;  if (MCPVB(mpp, MCPVB_DAG))
         mclDumpMatrix(dag, mpp, "dag", "", n_ite+1, TRUE)
      ;  mclxFree(&dag)
      ;  mclxFree(&clus)
   ;  }

      if (log_gauge)
      fputc('\n', fplog)

   ;  if (mpp->printMatrix)
      {  snprintf
         (  msg, sizeof msg, "%d%s%s%s"
         ,  (int) (2*n_ite+1), " After expansion (", when, ")"
         )
      ;  if (log_gauge)
         fprintf(stdout, "\n")
      ;  mclFlowPrettyPrint(*mxout, stdout, digits, msg)
   ;  }

      if (mpp->n_ite == 0 && mpp->fname_expanded)
      {  mcxIO* xftmp = mcxIOnew(mpp->fname_expanded->str, "w")
      ;  mclxWrite(*mxout, xftmp, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
      ;  mcxIOfree(&xftmp)
   ;  }

      mclInflate(*mxout, inflation, homgVec)

   ;  mclvFree(&homgVec)

   ;  if (mpp->printMatrix)
      {  snprintf
         (  msg,  sizeof msg, "%d%s%s%s"
         ,  (int) (2*n_ite+2), " After inflation (", when, ")"
         )
      ;  if (log_gauge)
         fprintf(stdout, "\n")
      ;  mclFlowPrettyPrint(*mxout, stdout, digits, msg)
   ;  }

   ;  if (MCPVB(mpp, MCPVB_ITE))
      mclDumpMatrix(*mxout, mpp, "ite", "", n_ite+1, TRUE)

   ;  if (stats->chaosMax < mpp->chaosLimit)
      return 1
   ;  else
      return 0
;  }


void mclDumpMatrix
(  mclMatrix*     mx
,  mclProcParam*  mpp
,  const char*    affix
,  const char*    postfix
,  int            n
,  mcxbool        print_value
)
   {  mcxIO*   xfdump
   ;  mcxTing*  fname
   ;  mcxbool   lines   =  MCPVB(mpp, MCPVB_LINES)
   ;  mcxbool   cat     =  MCPVB(mpp, MCPVB_CAT)

   ;  mcxbits   dump_modes =     !strcmp(affix, "result")
                              ?  (MCLX_DUMP_LINES | MCLX_DUMP_NOLEAD)
                              :  (MCLX_DUMP_PAIRS | MCLX_DUMP_VALUES)

   ;  if (!strcmp(affix, "result"))

   ;  else if (  (  mpp->dump_offset
            && (n<mpp->dump_offset)
            )
         || (  mpp->dump_bound
            && (n >= mpp->dump_bound)
            )
         || (  ((n-mpp->dump_offset) % mpp->dump_modulo) != 0)
         )
         return

   ;  if (cat)
      fname = mcxTingNew(mpp->dump_stem->str)
   ;  else
      fname
      =  mcxTingPrint
         (NULL, "%s-%d.%s%s", affix, (int) n, mpp->dump_stem->str, postfix)

   ;  xfdump = mcxIOnew(fname->str, cat ? "a" : "w")

   ;  if (mcxIOopen(xfdump, RETURN_ON_FAIL) != STATUS_OK)
      {  mcxErr
         ("mclDumpMatrix", "cannot open stream <%s>, ignoring", xfdump->fn->str)
      ;  return
   ;  }
      else if (lines)
      {  mclxIOdumper dumper
      ;  mclxIOdumpSet(&dumper, dump_modes, NULL, NULL, NULL)
      ;  dumper.threshold = 0.00001
      ;  if (cat)
         fprintf(xfdump->fp, "(mcldump %s %d\n", affix, (int) n)
      ;  mclxIOdump
         (  mx
         ,  xfdump
         ,  &dumper
         ,  mpp->dump_tab
         ,  mpp->dump_tab
         ,  MCLXIO_VALUE_GETENV
         ,  RETURN_ON_FAIL
         )
      ;  if (cat)
         fprintf(xfdump->fp, ")\n")
   ;  }
      else
      {  mcxenum printMode = print_value?MCLXIO_VALUE_GETENV:MCLXIO_VALUE_NONE
      ;  mclxWrite(mx, xfdump, printMode, RETURN_ON_FAIL)
   ;  }

      mcxIOfree(&xfdump)
   ;  mcxTingFree(&fname)
;  }


void  mclDumpVector
(  mclProcParam*  mpp
,  const char*    affix
,  const char*    postfix
,  int            n
,  mcxbool        print_value
)
   {  mcxIO*   xf
   ;  mcxTing*  fname

   ;  if (  (  mpp->dump_offset
            && (n<mpp->dump_offset)
            )
         || (  mpp->dump_bound
            && (n >= mpp->dump_bound)
            )
         )
         return

   ;  fname = mcxTingNew(mpp->dump_stem->str)
   ;  mcxTingAppend(fname, affix)

   ;  mcxTingPrintAfter(fname, "%d", (int) n)
   ;  mcxTingAppend(fname, postfix)

   ;  xf =  mcxIOnew(fname->str, "w")
   ;  if (mcxIOopen(xf, RETURN_ON_FAIL) == STATUS_FAIL)
      {  mcxTingFree(&fname)
      ;  mcxIOfree(&xf)
      ;  return
   ;  }

      mclvaWrite(mpp->vec_attr, xf->fp, print_value ? 8 : MCLXIO_VALUE_NONE)
   ;  mcxIOfree(&xf)
   ;  mcxTingFree(&fname)
;  }


void mclProcPrintInfo
(  FILE* fp
,  mclProcParam* mpp  
)
   {  mclShowSettings(fp, mpp, FALSE)
;  }
