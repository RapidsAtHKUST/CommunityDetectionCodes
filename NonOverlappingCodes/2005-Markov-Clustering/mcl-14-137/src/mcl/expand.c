/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* TODO
 *    In 2009 December this file was decluttered and cleaned-up to a large
 *    extent (removed somewhat overengineered logging structure),
 *    but some minor remnants may still exist.

 *    improve naming. rg prefix stands for register, but that's outdated now.
*/

#define DEBUG_SELECTION 0

#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "expand.h"

#include "util/alloc.h"
#include "util/types.h"
#include "util/heap.h"
#include "util/minmax.h"
#include "util/err.h"

#include "impala/compose.h"
#include "impala/ivp.h"
#include "impala/vector.h"
#include "impala/iface.h"


typedef struct
{  double*  bval
;  long*    bidx
;
}  vecbuffer   ;


static vecbuffer* vecbuffer_init
(  dim N
)
   {  dim i
   ;  vecbuffer* vb = mcxAlloc(sizeof vb[0], EXIT_ON_FAIL)
   ;  vb->bval = mcxAlloc(N * sizeof vb->bval[0], EXIT_ON_FAIL)
   ;  vb->bidx = mcxAlloc(N * sizeof vb->bidx[0], EXIT_ON_FAIL)

   ;  for (i=0;i<N;i++)
         vb->bval[i] = 0.0
      ,  vb->bidx[i] = i
   ;  return vb
;  }


static void vecbuffer_free
(  vecbuffer* vb
)
   {  mcxFree(vb->bval)
   ;  mcxFree(vb->bidx)
   ;  mcxFree(vb)
;  }


static void matrix_vector_array
(  const mclx* mx
,  const mclv* src
,  mclv* dst
,  vecbuffer* vecbuf          /* values must be zero; dependent on mclExpandVector */
)
   {  dim i, j, n_copied = 0
   ;  double * d = vecbuf->bval
   ;  long * x = vecbuf->bidx

#if 0
;  for(i=0;i<N_ROWS(mx);i++)
   if (d[i])
   mcxDie(1, "test", "nonzero entry %g\n", d[i])
#endif


/* compute linear combination of columns in mx.
 */
   ;  for (i=0;i<src->n_ivps;i++)
      {  mclv* c = mx->cols + src->ivps[i].idx
      ;  for (j=0;j<c->n_ivps;j++)
         d[c->ivps[j].idx] += c->ivps[j].val * src->ivps[i].val
   ;  }

/* shift nonzero probabilities to front. Set old value to zero if shifting.
*/
   ;  for (i=0;i<N_ROWS(mx);i++)
      {  if (d[i] != 0.0)
         {  x[n_copied] = i
         ;  d[n_copied] = d[i]
         ;  if (n_copied < i)
            d[i] = 0.0
         ;  n_copied++
      ;  }
      }

/* copy to vector, set to zero
*/
      mclvResize(dst, n_copied)
   ;  for (i=0;i<n_copied;i++)
      {  dst->ivps[i].idx = x[i]
      ;  dst->ivps[i].val = d[i]
      ;  d[i] = 0.0
   ;  }
   }



static double mclExpandVector
(  const mclMatrix*  mx
,  const mclVector*  srvec       /* src                         */
,  mclVector*        dstvec      /* dst                         */
,  mclpAR*           ivpbuf      /* backup storage for recovery */
,  vecbuffer*        vecbuf
,  mclxComposeHelper*ch
,  long              col
,  mclExpandParam*   mxp
,  mclExpandStats*   stats
,  dim               thread_id
)  ;


                  /* homg is measured as                                      */
                  /*                                                          */
                  /*  mclvPowSum(src, 2.0) * mclvPowSum(dst, inflation) ** 2  */
                  /*  ------------------------------------------------        */
                  /*             mclvPowSum(dst, 2.0 * inflation)             */
                  /*                                                          */

static double get_homg
(  const mclv* src
,  const mclv* dst
,  double inflation
)
   {  double homg_num   =  mclvPowSum(src, 2.0) * pow(mclvPowSum(dst, inflation), 2.0)
   ;  double homg_nom   =  mclvPowSum(dst, 2.0 * inflation)
   ;  return   homg_nom
            ?  homg_num / homg_nom
            :  1.0
;  }


typedef struct
{  long              id
;  mclExpandParam*   mxp
;  mclExpandStats*   stats
;  const mclx*       mxright
;  double            lap
;  mclx*             mxdst
;  mclv*             chaosVec
;  mclv*             homgVec
;  mclpAR*           ivpbuf
;  vecbuffer*        vecbuf
;  mclxComposeHelper*helper
;
}  mclExpandVectorLine_arg ;


static void vecMeasure
(  mclVector*        vec
,  double            *maxval
,  double            *center
)  ;


mclExpandParam* mclExpandParamNew
(  void
)  
   {  mclExpandParam *mxp  =  (mclExpandParam*) mcxAlloc
                              (  sizeof(mclExpandParam)
                              ,  EXIT_ON_FAIL
                              )

   ;  mxp->stats           =  NULL

   ;  mxp->n_ethreads      =  0
   ;  mxp->precision       =  0.000666
   ;  mxp->pct             =  0.95

   ;  mxp->num_prune       =  1444u /* should be overridden by scheme or user */
   ;  mxp->num_select      =  1444u /* should be overridden by scheme or user */
   ;  mxp->num_recover     =  1444u /* should be overridden by scheme or user */
   ;  mxp->scheme          =  6
   ;  mxp->implementation  =  0

   ;  mxp->partition_pivot_sort_n = 72

   ;  mxp->vector_progression     =  20

   ;  mxp->warn_factor     =  1000
   ;  mxp->warn_pct        =  0.1

   ;  mxp->verbosity       =  0
   ;  mxp->dimension       =  -1

   ;  mxp->inflation       =  -1.0
   ;  mxp->sparse_trigger  =  10

   ;  return mxp
;  }


void mclExpandParamFree
(  mclExpandParam** epp
)
   {  mclExpandParam *mxp = *epp
   ;  if (mxp->stats)
      mclExpandStatsFree(&(mxp->stats))
   ;  mcxFree(*epp)
   ;  *epp =  NULL
;  }


static void compose_dispatch
(  mclx* mxsrc
,  dim colidx
,  void* data
,  dim thread_id
)
   {  mclExpandVectorLine_arg *a =  ((mclExpandVectorLine_arg*) data) + thread_id
   ;  const mclx*    mxright  =  a->mxright
   ;  mclx*          mxdst    =  a->mxdst
   ;  mclv*          chaosVec =  a->chaosVec
   ;  mclv*          homgVec  =  a->homgVec
   ;  mclExpandParam*  mxp    =  a->mxp
   ;  mclExpandStats* stats   =  a->stats
   ;  clock_t        t1       =  clock(), t2

   ;  mclpAR* ivpbuf          =  a->ivpbuf
   ;  vecbuffer* vecbuf       =  a->vecbuf
   ;  mclxComposeHelper*helper=  a->helper

   ;  double colInhomogeneity
      =  mclExpandVector
         (  mxsrc
         ,  mxright->cols + colidx
         ,  mxdst->cols + colidx
         ,  ivpbuf      /* backup storage for recovery */
         ,  vecbuf
         ,  helper
         ,  colidx
         ,  mxp
         ,  stats       /* important that threads write do different memory locations */
         ,  thread_id
         )

   ;  (homgVec->ivps+colidx)->val
      =  get_homg
         (  mxsrc->cols+colidx
         ,  mxdst->cols+colidx
         ,  2.0
         )
   ;  (chaosVec->ivps+colidx)->val = colInhomogeneity

   ;  t2 = clock()
   ;  a->lap += ((double) (t2 - t1)) / CLOCKS_PER_SEC
;  }


static void warn_pruning
(  long col
,  double maxval
,  long n_ivps
,  long rg_n_current
,  double rg_mass_current
,  long num_select
)
   {  fprintf
      (  stderr,
      "\n"
      "___> Vector with idx [%ld], maxval [%.6f] and [%ld] entries\n"
      " ->  initially reduced to [%ld] entries with combined mass [%.6f].\n"
      " ->  Consider increasing the -P value and %s the -S value.\n"
      ,  (long) col
      ,  (double) maxval
      ,  (long) n_ivps
      ,  (long) rg_n_current
      ,  (double) rg_mass_current
      ,  num_select ? "increasing" : "using"
      )
;  }


static double mclExpandVector1
(  const mclMatrix*  mx
,  const mclVector*  srcvec
,  mclVector*        dstvec
,  mclpAR*           ivpbuf
,  vecbuffer*        vecbuf
,  mclxComposeHelper*ch
,  long              col
,  mclExpandParam*   mxp
,  mclExpandStats*   stats
,  dim               thread_id
)
   {  dim            rg_n_expand    =  0
   ;  dim            rg_n_prune     =  0
   ;  dim            v_offset       =  col   /* srcvec - mx->cols */

   ;  mcxbool        rg_b_recover   =  FALSE
   ;  mcxbool        rg_b_select    =  FALSE

   ;  double         rg_mass_prune  =  0.0   /* used for heap, must be float */
   ;  double         rg_mass_final  =  1.0   /* same */

   ;  double         rg_sbar        =  -1.0     /*    select bar           */
   ;  double         rg_rbar        =  -1.0     /*    recovery bar         */

   ;  double         cut            =  0.0
   ;  mcxbool        mesg           =  FALSE

   ;  double         maxval         =  0.0
   ;  double         center         =  0.0
   ;  double         colInhomogeneity =  0.0

   ;  mcxbool        progress       =  mcxLogGet(MCX_LOG_GAUGE)
   ;  mcxbool        have_canonical =  MCLV_IS_CANONICAL(mx->dom_rows)
   ;  dim            n_entries      =  0, i

   ;  if (have_canonical)
      for (i=0;i<srcvec->n_ivps;i++)
      n_entries += mx->cols[srcvec->ivps[i].idx].n_ivps

   ;  if
      (  have_canonical
      && mxp->sparse_trigger
      && n_entries * mxp->sparse_trigger >= N_COLS(mx)
      )
         matrix_vector_array(mx, srcvec, dstvec, vecbuf)
      ,  stats->bob_sparse++     /* not an atomic update, but we do not care */
   ;  else
      mclxVectorCompose(mx, srcvec, dstvec, mclxComposeThreadData(ch, thread_id))

   ;  rg_n_expand = dstvec->n_ivps ? dstvec->n_ivps : 1

   ;  if (mxp->num_recover)
      {  memcpy(ivpbuf->ivps, dstvec->ivps, dstvec->n_ivps * sizeof(mclIvp))
      ;  ivpbuf->n_ivps = dstvec->n_ivps
   ;  }

      {  vecMeasure(dstvec, &maxval, &center)
      ;  if (mxp->implementation & MCL_USE_RPRUNE)
         {  cut            =  maxval / mxp->num_prune
         ;  rg_mass_prune  =  mclvSelectGqBar (dstvec, cut)
      ;  }
         else if (mxp->precision)
         {  cut            =  mxp->precision
         ;  rg_mass_prune  =  mclvSelectGqBar (dstvec, cut)
      ;  }
         else
         rg_mass_prune  =  mclvSum(dstvec)

      ;  rg_n_prune        =  dstvec->n_ivps
      ;  rg_mass_final     =  rg_mass_prune
;if(DEBUG_SELECTION)fprintf(stdout, "%d pru %d ", (int) dstvec->vid, (int) dstvec->n_ivps)
   ;  }

      if
      (  mxp->warn_factor
      && (     mxp->warn_factor * MCX_MAX(dstvec->n_ivps,mxp->num_select)
            <  rg_n_expand
         && rg_mass_prune < mxp->warn_pct
         )
      )
         mesg = TRUE
      ,  warn_pruning(col, maxval, rg_n_expand, dstvec->n_ivps, rg_mass_prune, mxp->num_select)

   ;  if (!mxp->num_recover && !dstvec->n_ivps)
      {  mclvResize(dstvec, 1)
      ;  (dstvec->ivps+0)->idx = col
      ;  (dstvec->ivps+0)->val = 1.0
      ;  rg_mass_prune  =  1.0
      ;  rg_n_prune     =  1
      ;  if (mxp->warn_factor)
         fprintf(stderr, " ->  Emergency measure: added loop to node\n")
   ;  }

      if
      (  mxp->num_recover
      && (  dstvec->n_ivps <  mxp->num_recover)
      && (  rg_mass_prune  <  mxp->pct)
      )
      {  dim recnum     =  mxp->num_recover
      ;  rg_b_recover   =  TRUE
      ;  mclvRenew(dstvec, ivpbuf->ivps, ivpbuf->n_ivps)

      ;  if (dstvec->n_ivps > recnum)        /* use cut previously      */
         rg_rbar                             /* computed.               */
         =  mclvKBar                         /* we should check         */
         (  dstvec                           /* whether it is any use   */
         ,  recnum - rg_n_prune              /* (but we don't)          */
         ,  cut
         ,  KBAR_SELECT_LARGE
         )
      ;  else
         rg_rbar = 0.0

      ;  rg_mass_final     =  mclvSelectGqBar(dstvec, rg_rbar)
;if(DEBUG_SELECTION)fprintf(stdout, "rec1 %d ", (int) dstvec->n_ivps)
   ;  }

      else if (mxp->num_select && dstvec->n_ivps > mxp->num_select)
      {  double mass_select
      ;  int n_select
      ;  rg_b_select       =  TRUE

      ;  if (mxp->num_recover)                  /* recovers to post prune vector */
         {  memcpy(ivpbuf->ivps, dstvec->ivps, dstvec->n_ivps * sizeof(mclIvp))
         ;  ivpbuf->n_ivps = dstvec->n_ivps
      ;  }

         if (dstvec->n_ivps >= 2*mxp->num_select)
         rg_sbar
         =  mclvKBar
            (  dstvec
            ,  mxp->num_select
            ,  FLT_MAX
            ,  KBAR_SELECT_LARGE
            )
      ;  else
         rg_sbar
         =  mclvKBar
            (  dstvec
            ,  dstvec->n_ivps - mxp->num_select + 1
            ,  -FLT_MAX                         /* values < cut are already removed */
            ,  KBAR_SELECT_SMALL
            )

      ;  mass_select       =  mclvSelectGqBar(dstvec, rg_sbar)
;if(DEBUG_SELECTION)fprintf(stdout, "sel %d [%.16f %.9f] ", (int) dstvec->n_ivps, rg_sbar, mass_select)
      ;  rg_mass_final     =  mass_select
      ;  n_select          =  dstvec->n_ivps

      ;  if
         (  mxp->num_recover
         && (  dstvec->n_ivps  <  mxp->num_recover)
         && (  mass_select    <  mxp->pct)
         )
         {  dim recnum     =  mxp->num_recover
         ;  rg_b_recover   =  TRUE
         ;  mclvRenew(dstvec, ivpbuf->ivps, ivpbuf->n_ivps)

         ;  if (dstvec->n_ivps > recnum)        /* use cut previously   */
            rg_rbar                             /* computed.            */
            =  mclvKBar                         /* we should check      */
            (  dstvec                           /* whether it is any use*/
            ,  recnum - n_select                /* (but we don't)       */
            ,  rg_sbar
            ,  KBAR_SELECT_LARGE
            )
         ;  else
            rg_rbar = 0.0

         ;  rg_mass_final  =  mclvSelectGqBar(dstvec, rg_rbar)
;if(DEBUG_SELECTION)fprintf(stdout, "rec2 %d ", (int) dstvec->n_ivps)
      ;  }
      }

      if (mesg)
      fprintf
      (  stderr
      ,  " ->  (before rescaling) Finished with [%ld] entries and [%f] mass.\n"
      ,  (long) dstvec->n_ivps
      ,  (double) rg_mass_final
      )

      /* fixme at this stage I could have 0.0 mass and 0 entries
       * The code seems to work in that extreme boundary case (sometimes
       * achievable e.g. by '-I 30', but some of the adding loop block above
       * could be used over here. Before doing anything, a careful look
       * at the selection and recovery logic is needed though.
      */

   ;  if (rg_mass_final)
      mclvScale(dstvec, rg_mass_final)

   ;  colInhomogeneity  =  (maxval-center) * dstvec->n_ivps
   ;


     /*
      *  expansion threads only have read & write access to stats
      *  in the block below and nowhere else.
     */

      {  stats->bob_low[v_offset]   =  rg_mass_prune
      ;  stats->bob_final[v_offset] =  rg_mass_final
      ;  stats->bob_expand[v_offset]=  rg_n_expand

      ;  if (progress && !mxp->n_ethreads)         /* fixme: change to thread-specific data. */
         {  stats->n_cols++
         ;  if (stats->n_cols % mxp->vector_progression == 0)
            fwrite(".", sizeof(char), 1, stderr)
      ;  }
      }

      return colInhomogeneity
;  }


mclMatrix* mclExpand
(  const mclMatrix*        mx
,  const mclMatrix*        mxright
,  mclExpandParam*         mxp
)
   {  mclMatrix*        sq
   ;  mclVector*        chaosVec, * homgVec
   ;  long              col      =  0
   ;  mclExpandStats*   stats    =  mxp->stats
   ;  clock_t           t1       =  clock(), t2
   ;  long              n_cols   =  N_COLS(mx)

   ;  if (mxp->dimension < 0 || !stats)
         mcxErr("mclExpand", "pbd: not correctly initialized")
         /* mclExpandParamDim probably not called */
      ,  mcxExit(1)

   ;  if (!mcldEquate(mx->dom_cols, mx->dom_rows, MCLD_EQT_EQUAL))
         mcxErr("mclExpand", "pbd: matrix not square")
      ,  mcxExit(1)

   ;  sq       =  mclxAllocZero
                  (  mclvCopy(NULL, mx->dom_rows)
                  ,  mclvCopy(NULL, mx->dom_cols)
                  )
   ;  chaosVec =  mclvCanonical(NULL, n_cols, 1.0)
   ;  homgVec  =  mclvCanonical(NULL, n_cols, 1.0)

   ;  mclExpandStatsReset(stats)

   ;  if (mxp->n_ethreads)
      {  int i
      ;  mclExpandVectorLine_arg *data = mcxAlloc(mxp->n_ethreads * sizeof data[0], EXIT_ON_FAIL)
      ;  mclxComposeHelper *ch = mclxComposePrepare(mx, NULL, mxp->n_ethreads)
      ;  struct mclx_thread_map map

      ;  for (i=0;i<mxp->n_ethreads;i++)
         {  mclExpandVectorLine_arg* a = data+i

         ;  a->id          =  i
         ;  a->mxdst       =  sq
         ;  a->lap         =  0.0
         ;  a->mxp         =  mxp
         ;  a->stats       =  stats
         ;  a->chaosVec    =  chaosVec
         ;  a->homgVec     =  homgVec

         ;  a->mxright     =  mxright
         ;  a->ivpbuf      =  mclpARensure(NULL, N_ROWS(mx))
         ;  a->vecbuf      =  vecbuffer_init(N_ROWS(mx))
         ;  a->helper      =  ch
      ;  }

         mclxVectorDispatch((mclx*) mx, data, mxp->n_ethreads, compose_dispatch, NULL)

      ;  for (i=0;i<mxp->n_ethreads;i++)
         {  mclExpandVectorLine_arg* a = data+i
         ;  mclpARfree(&(a->ivpbuf))
         ;  vecbuffer_free(a->vecbuf)
         ;  stats->lap = MCX_MAX(stats->lap, data[i].lap)
      ;  }

         mclxComposeRelease(&ch)
      ;  mcxFree(data)
   ;  }

      else
      {  mclpAR* ivpbuf    =  mclpARensure(NULL, N_ROWS(mx))  
      ;  vecbuffer* vecbuf =  vecbuffer_init(N_ROWS(mx))
      ;  mclxComposeHelper *ch = mclxComposePrepare(mx, NULL, 1)

      ;  for (col=0;col<n_cols;col++)
         {  double colInhomogeneity
            =  mclExpandVector
               (  mx
               ,  mxright->cols+col
               ,  sq->cols+col
               ,  ivpbuf
               ,  vecbuf
               ,  ch
               ,  col
               ,  mxp
               ,  stats
               ,  0        /* thread id, indexes structure in ch */
               )
         ;  (chaosVec->ivps+col)->val = colInhomogeneity
         ;  (homgVec->ivps+col)->val
            =  get_homg
               (  mx->cols+col
               ,  sq->cols+col
               ,  2.0
               )

         ;  if (!((col+1) % 10))
            {  t2 = clock()
            ;  stats->lap += ((double) (t2 - t1)) / CLOCKS_PER_SEC
            ;  t1 = t2
         ;  }
         }

         mclpARfree(&ivpbuf)
      ;  mclxComposeRelease(&ch)
      ;  vecbuffer_free(vecbuf)
   ;  }

      if (chaosVec->n_ivps)
      {  stats->chaosMax =  mclvMaxValue(chaosVec)
      ;  stats->chaosAvg =  mclvSum(chaosVec) / chaosVec->n_ivps
      ;  stats->homgAvg  =  mclvSum(homgVec) / homgVec->n_ivps
      ;  stats->homgMax  =  mclvMaxValue(homgVec)
      ;  stats->homgMin  =  mclvMinValue(homgVec)
   ;  }

      mclvFree(&chaosVec)
   ;  stats->homgVec = homgVec
   ;  return sq
;  }


mclExpandStats* mclExpandStatsNew
(  dim   n_cols
)  
   {  mclExpandStats* stats   =  (mclExpandStats*) mcxAlloc
                                 (  sizeof(mclExpandStats)
                                 ,  EXIT_ON_FAIL
                                 )
   ;  stats->bob_low          =  mcxAlloc(n_cols * sizeof stats->bob_low[0], EXIT_ON_FAIL)
   ;  stats->bob_final        =  mcxAlloc(n_cols * sizeof stats->bob_final[0], EXIT_ON_FAIL)
   ;  stats->bob_expand       =  mcxAlloc(n_cols * sizeof stats->bob_expand[0], EXIT_ON_FAIL)
   ;  stats->bob_sparse       =  0

   ;  stats->homgVec          =  NULL

   ;  mclExpandStatsReset(stats)       /* this initializes several members */
   ;  return stats
;  }


void mclExpandStatsReset
(  mclExpandStats* stats
)  
   {  stats->chaosMax         =  0.0
   ;  stats->chaosAvg         =  0.0
   ;  stats->homgMax          =  0.0
   ;  stats->homgMin          =  PVAL_MAX
   ;  stats->homgAvg          =  0.0
   ;  stats->n_cols           =  0
   ;  stats->lap              =  0.0
   ;  stats->bob_sparse       =  0

   ;  mclvFree(&(stats->homgVec))
;  }


void mclExpandStatsFree
(  mclExpandStats** statspp
)  
   {  mclExpandStats* stats = *statspp

   ;  mcxFree(stats->bob_low)
   ;  mcxFree(stats->bob_final)
   ;  mcxFree(stats->bob_expand)

   ;  mclvFree(&(stats->homgVec))
   ;  mcxFree(stats)

   ;  *statspp = NULL
;  }


static void vecMeasure
(  mclVector*  vec
,  double      *maxval
,  double      *center
)  
   {  mclIvp*  ivp      =  vec->ivps
   ;  int      n_ivps   =  vec->n_ivps
   ;  double   m        =  0.0
   ;  double   c        =  0.0

   ;  while (--n_ivps >= 0)
      {  double val      =  (ivp++)->val
      ;  c += val * val
      ;  if (val > m)
         m = val
   ;  }

      *maxval           =  m
   ;  *center           =  c
;  }


/* do the init stuff that depends on the dimension of the input graph */

void mclExpandParamDim
(  mclExpandParam*  mxp
,  const mclMatrix *mx
)
   {  mxp->stats  =  mclExpandStatsNew(N_COLS(mx))
   ;  mxp->dimension = N_COLS(mx)
;  }


#if 0
static int cmp_pval
(  void* p1
,  void* p2
)
   {  return *((pval*) p1) < *((pval*) p2) ? -1 : *((pval*) p1) > *((pval*) p2) ? 1 : 0
;  }
#endif


pval partition_select
(  pval* d
,  dim  N
,  dim  K
,  double* mass
,  dim* nd
,  dim* ns
,  mclExpandParam* mxp
)
#define SWAP(a,b)  e = d[a], d[a] = d[b], d[b] = e
   {  dim left = 0, right = N -1
   ;  dim i
   ;  dim n_delta = 0, n_swap = 0
   ;  mclv* sample = mclvCanonical(NULL, 7, 1.0)
   ;  pval e

   ;  if (!N || !K)
      return 0.0

   ;  while (left < right)
      {  dim delta = right - left, storeid = left
      ;  ofs pid = -1
      ;  pval piv = -1.0
      ;  if (delta > mxp->partition_pivot_sort_n)
         {  dim fac = (delta - (delta % mxp->partition_pivot_sort_n)) / mxp->partition_pivot_sort_n
         ;  for (i=0;i<7;i++)
            {  sample->ivps[i].val = d[left + 1 + 2*i*fac]
            ;  sample->ivps[i].idx = left + 1 + 2*i*fac
         ;  }
            mclvSortAscVal(sample)
         ;  pid =  sample->ivps[(6 * (right - (K-1))) / delta].idx     /* if right-K large, pick a large pivot */
         ;  piv =  sample->ivps[(6 * (right - (K-1))) / delta].val
      ;  }
         else
            pid = left + (delta >> 1)
        ,   piv = d[pid]

      ;  n_delta += delta

      ;  SWAP(right, pid)

      ;  for (i=left;i<right;i++)
         {  if (d[i] >= piv)
            {  SWAP(storeid, i)
            ;  n_swap++
            ;  storeid++
         ;  }
         }

         SWAP(right, storeid)
      ;  if (storeid > K-1)
         right = storeid - 1
      ;  else if (storeid <= K-1)
         left = storeid + 1
   ;  }

      if (ns) ns[0] = n_swap
   ;  if (nd) nd[0] = n_delta

   ;  if (mass)
      {  double m = 0.0
      ;  for (i=0;i<K;i++)
         m += d[i]
      ;  mass[0] = m
   ;  }

      mclvFree(&sample)
   ;  return d[K-1]
;  }


   /* See the note at mclvExpandVector2 below for the fine details
    * of MCL_USE_PARTITION_TIES.
   */

pval selectk
(  pval* d
,  dim  N
,  dim  K
,  dim  *K_adjusted     /* to account for ties */
,  double* mass
,  dim* nd
,  dim* ns
,  mclExpandParam *mxp
)
   {  dim i = 0, k = K
   ;  pval bar = partition_select(d, N, K, mass, nd, ns, mxp)

   ;  if (1)
      {  while (k < N && d[k] >= bar)
            mass[0] += d[k]
         ,  k++
   ;  }
      else                    /* test from scratch */
      {  double m = 0.0
      ;  k = 0
      ;  for (i=0;i<N;i++)
         {  if (d[i] >= bar)
               k++
            ,  m += d[i]
      ;  }
         mass[0] = m
   ;  }

      K_adjusted[0] = k
   ;  return bar
;  }


   /* fixme. variables + naming to keep track of #entries
    * and their combined mass is kludgy.
   */

static double mclExpandVector2
(  const mclMatrix*  mx
,  const mclVector*  srcvec
,  mclVector*        dstvec
,  mclpAR*           ivpbuf
,  vecbuffer*        vecbuf
,  mclxComposeHelper*ch
,  long              col
,  mclExpandParam*   mxp
,  mclExpandStats*   stats
,  dim               thread_id
)
   {  dim            v_offset       =  srcvec - mx->cols
   ;  dim            rg_n_expand    =  0

   ;  double         rg_mass_current=  0.0
   ;  double         rg_mass_prune  =  0.0
   ;  double         rg_mass_precision  =  0.0
   ;  dim            rg_n_current   =  0
   ;  dim            rg_n_prune     =  0
   ;  double         rg_rbar        =  -1.0     /*    recovery bar         */

   ;  mcxbool        mesg           =  FALSE

   ;  double         maxval         =  0.0
   ;  double         center         =  0.0
   ;  double         colInhomogeneity =  0.0

   ;  mcxbool        progress       =  mcxLogGet(MCX_LOG_GAUGE)

   ;  pval*          values         =  NULL
   ;  dim            i, n_values    =  0
   ;  dim            n_delta, n_swap, n_obtained
   ;  mcxbool        have_canonical =  MCLV_IS_CANONICAL(mx->dom_rows)
   ;  dim            n_entries      =  0

   ;  if (have_canonical)
      for (i=0;i<srcvec->n_ivps;i++)
      n_entries += mx->cols[srcvec->ivps[i].idx].n_ivps

   ;  if
      (  have_canonical
      && mxp->sparse_trigger
      && n_entries * mxp->sparse_trigger >= N_COLS(mx)
      )
         matrix_vector_array(mx, srcvec, dstvec, vecbuf)
      ,  stats->bob_sparse++     /* not an atomic update, but we do not care */
   ;  else
      mclxVectorCompose(mx, srcvec, dstvec, mclxComposeThreadData(ch, thread_id))

   ;  rg_n_expand = dstvec->n_ivps ? dstvec->n_ivps : 1

   ;  if (mxp->implementation & MCL_USE_RPRUNE)
         maxval   =  mclvMaxValue(dstvec)
      ,  rg_rbar  =  maxval / mxp->num_prune
   ;  else if (mxp->precision)
      rg_rbar = mxp->precision
   ;  else
      rg_rbar = 0.0

   ;  n_values =  dstvec->n_ivps
   ;  values   =  mcxAlloc(n_values * sizeof values[0], EXIT_ON_FAIL)

                     /* fill values, elements >= rg_rbar on left, < rg_rbar on right */
   ;  if (n_values)
      {  dim i_left = 0, i_right = n_values -1
      ;  for (i=0;i<dstvec->n_ivps;i++)
         {  pval pv = dstvec->ivps[i].val
         ;  if (pv >= rg_rbar)
               values[i_left++] = pv
            ,  rg_mass_current += pv
         ;  else
            values[i_right--] = pv
      ;  }
         if (i_left != i_right + 1)
         mcxDie(1, "mclExpandVector2", "copying error")
      ;  rg_n_prune = rg_n_current = i_left
      ;  rg_mass_precision = rg_mass_prune = rg_mass_current
;if(DEBUG_SELECTION)fprintf(stdout, "%d pru %d ", (int) dstvec->vid, (int) rg_n_current)
   ;  }

      if
      (  mxp->warn_factor
      && (     mxp->warn_factor * MCX_MAX(dstvec->n_ivps,mxp->num_select)
            <  dstvec->n_ivps
         && rg_mass_current < mxp->warn_pct
         )
      )
         mesg = TRUE
      ,  warn_pruning(col, maxval, dstvec->n_ivps, rg_n_current, rg_mass_current, mxp->num_select)

   ;  if (!mxp->num_recover && !dstvec->n_ivps)
      {  mclvResize(dstvec, 1)
      ;  (dstvec->ivps+0)->idx = col
      ;  (dstvec->ivps+0)->val = 1.0
      ;  rg_mass_current   =  1.0
      ;  rg_n_current      =  1
      ;  if (mxp->warn_factor)
         fprintf(stderr, " ->  Emergency measure: added loop to node\n")
   ;  }

                                    /* Case: overdid it, threshold too small */
      else if
      (  mxp->num_recover
      && (  rg_n_current      <  mxp->num_recover)
      && (  rg_mass_current   <  mxp->pct)
      )
      {  double mass_recover = 0.0
      ;  dim n_request = mxp->num_recover - rg_n_current

      ;  if (n_request > n_values - rg_n_current)
         n_request = n_values - rg_n_current

      ;  rg_rbar
         =  selectk                 /* recover to mxp->num_recover entries;   */
            (  values + rg_n_current 
            ,  n_values - rg_n_current
            ,  n_request
            ,  &n_obtained
            ,  &mass_recover
            ,  &n_delta
            ,  &n_swap
            ,  mxp
            )
      ;  rg_mass_current += mass_recover
      ;  rg_n_current += n_obtained
;if(DEBUG_SELECTION)fprintf(stdout, "rec1 %d ", (int) rg_n_current)
   ;  }

                                    /* Case: threshold too large, reduce further */
      else if (mxp->num_select && rg_n_current > mxp->num_select)
      {  rg_rbar
         =  selectk
            (  values
            ,  rg_n_current
            ,  mxp->num_select  
            ,  &n_obtained
            ,  &rg_mass_current
            ,  &n_delta
            ,  &n_swap
            ,  mxp
            )

      ;  rg_mass_prune = rg_mass_current
      ;  rg_n_current  = n_obtained
;if(DEBUG_SELECTION)fprintf(stdout, "sel %d [%.16f %.9f] ", (int) rg_n_current, (double) rg_rbar, rg_mass_current)

      ;  if
         (  mxp->num_recover
         && ( rg_n_current     < mxp->num_recover)
         && ( rg_mass_current  <  mxp->pct)
         )
         {  dim n_request  =  mxp->num_recover - rg_n_current, n_obtained = 0
         ;  double mass_recover = 0.0
         ;  if (n_request > n_values - rg_n_current)
            n_request = n_values - rg_n_current

         ;  if (mxp->num_recover < rg_n_prune)
            {  rg_rbar
               =  selectk
                  (  values + rg_n_current
                  ,  n_values - rg_n_current
                  ,  n_request
                  ,  &n_obtained
                  ,  &mass_recover
                  ,  &n_delta
                  ,  &n_swap
                  ,  mxp
                  )
               ;  rg_mass_current += mass_recover
               ;  rg_n_current += n_obtained 
         ;  }
            else
            {  rg_rbar = mxp->precision
            ;  rg_n_current = rg_n_prune
            ;  rg_mass_current = rg_mass_precision
         ;  }

;if(DEBUG_SELECTION)fprintf(stdout, "rec2 %d ", (int) rg_n_current)
      ;  }
      }

      if (mesg)
      fprintf
      (  stderr
      ,  " ->  (before rescaling) Finished with [%ld] entries and [%f] mass.\n"
      ,  (long) rg_n_current
      ,  (double) rg_mass_current
      )

   ;  mclvSelectGqBar(dstvec, rg_rbar)
   ;  mclvNormalize(dstvec)
   ;  vecMeasure(dstvec, &maxval, &center)
   ;  colInhomogeneity  =  (maxval-center) * dstvec->n_ivps
   ;

     /*
      *  expansion threads only have read & write access to stats
      *  in the block below and nowhere else.
     */

      {  stats->bob_low[v_offset]   =  rg_mass_prune
      ;  stats->bob_final[v_offset] =  rg_mass_current
      ;  stats->bob_expand[v_offset]=  rg_n_expand

      ;  if (progress && !mxp->n_ethreads)         /* fixme: change to thread-specific data. */
         {  stats->n_cols++
         ;  if (stats->n_cols % mxp->vector_progression == 0)
            fwrite(".", sizeof(char), 1, stderr)
      ;  }
      }

      mcxFree(values)
   ;  return colInhomogeneity
;  }



static double mclExpandVector
(  const mclMatrix*  mx
,  const mclVector*  srcvec      /* src                         */
,  mclVector*        dstvec      /* dst                         */
,  mclpAR*           ivpbuf      /* backup storage for recovery */
,  vecbuffer*        vecbuf      /* storage for full mx/vec computation */
,  mclxComposeHelper*ch
,  long              col
,  mclExpandParam*   mxp
,  mclExpandStats*   stats
,  dim               thread_id
)
   {  double val =
         (mxp->implementation & MCL_USE_PARTITION_SELECTION)
      ?  mclExpandVector2(mx, srcvec, dstvec, ivpbuf, vecbuf, ch, col, mxp, stats, thread_id)
      :  mclExpandVector1(mx, srcvec, dstvec, ivpbuf, vecbuf, ch, col, mxp, stats, thread_id)
;if(DEBUG_SELECTION)fputc('\n', stdout)
   ;  return val
;  }


