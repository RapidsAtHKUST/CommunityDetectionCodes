/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#include <math.h>
#include <pthread.h>

#include "compose.h"

#include "util/compile.h"
#include "util/types.h"
#include "util/alloc.h"
#include "util/types.h"
#include "util/err.h"

struct mclIOV
{  long     index
;  int      ref
;  double   value
;
}  ;


void*  mclIOVinit_v
(  void*   iovp
)
   {  mclIOV* iov = (mclIOV*) iovp
   ;  iov->index  = -1
   ;  iov->ref    = -1
   ;  iov->value  = -1.0
   ;  return iov
;  }


struct mclxComposeHelper
{  mclIOV** iovs
;  int      n_iovs
;  int      n_jobs         /* 1 or more; n_threads can be 0 or more */
;
}  ;


void* mclxComposeThreadData
(  mclxComposeHelper* ch
,  int i_thread
)
   {  if (i_thread > ch->n_jobs)
      mcxDie(1, "compose", "fatal: thread ID error (%d asked, %d max)", (int) i_thread, (int) ch->n_jobs)
   ;  return ch->iovs[i_thread]
;  }


      /* In the future this could look at #cores conceivably */
int mclxComposeSetThreadCount
(  int n
)
   {  if (n < 0)
      n = 0
   ;  if (n > MCLX_COMPOSE_NTHREAD_MAX_SUGGESTED)
      n = MCLX_COMPOSE_NTHREAD_MAX_SUGGESTED
   ;  return n
;  }


/* fixme: callers of mclxcomposeprepare need to use n_jobs */

mclxComposeHelper* mclxComposePrepare
(  const mclMatrix*  mx1
,  const mclMatrix*  mx2_unused  cpl__unused
,  int n_threads
)
   {  int i
   ;  mclxComposeHelper* ch
                  =     mcxRealloc
                        (  NULL, sizeof(mclxComposeHelper), EXIT_ON_FAIL)

   ;  if (n_threads <= 0)
      n_threads = 1
   ;  ch->n_jobs = n_threads

   ;  ch->iovs = mcxAlloc(ch->n_jobs * sizeof ch->iovs[0], EXIT_ON_FAIL)

   ;  for (i=0; i<ch->n_jobs; i++)
      ch->iovs[i] =  mcxNAlloc
                     (  N_ROWS(mx1) + 1
                     ,  sizeof(mclIOV)
                     ,  mclIOVinit_v
                     ,  EXIT_ON_FAIL
                     )
   ;  return ch
;  }


void mclxComposeRelease
(  mclxComposeHelper **chpp
)
   {  mclxComposeHelper* ch = *chpp
   ;  if (ch)
      {  int i
      ;  for (i=0; i< ch->n_jobs; i++)
         mcxFree(ch->iovs[i])
      ;  mcxFree(ch->iovs)
      ;  mcxFree(ch)
      ;  *chpp = NULL
   ;  }
   }


mclVector* mclxVectorCompose
(  const mclMatrix*        mx
,  const mclVector*        vecs
,  mclVector*              vecd
,  mclIOV*                 iovs
)
   {  mclIvp*  facivp      =  vecs->ivps - 1
   ;  mclIvp*  facivpmax   =  vecs->ivps + vecs->n_ivps
   ;  int      n_entries   =  0
   ;  int      cleanup     =  0
   ;  mcxbool  canonical   =  mclxColCanonical(mx)
   ;  mclVector* vecprev   =  NULL
   ;  int n_cols           =  N_COLS(mx)

   ;  if (!iovs)
      {  iovs = mcxNAlloc(N_ROWS(mx) + 1, sizeof(mclIOV), mclIOVinit_v, EXIT_ON_FAIL)
      ;  cleanup = 1
   ;  }

      iovs[0].ref    =  -1
   ;  iovs[0].index  =  -1
   ;  iovs[0].value  =  -1.0        /* reset values; docme why not all? */

   ;  while (++facivp < facivpmax)
      {  mclVector*  mxvec
         =  canonical
            ?  (  facivp->idx < n_cols
                  ?  mx->cols+(facivp->idx)
                  :  NULL
               )
            :  mclxGetVector
               (  mx
               ,  facivp->idx
               ,  RETURN_ON_FAIL
               ,  vecprev
               )
      ;  int      i_iov    =  0
      ;  mclIvp*  colivp   =  mxvec ? mxvec->ivps + mxvec->n_ivps : NULL
      ;  double   facval   =  facivp->val

      ;  vecprev  = mxvec ? mxvec + 1 : NULL

      ;  if (!mxvec || !mxvec->n_ivps)
         continue

      ;  while (--colivp >= mxvec->ivps)
         {  long dstidx  =  colivp->idx

         ;  while(dstidx < iovs[i_iov].index)
            i_iov = iovs[i_iov].ref

           /*  now   dstidx >= iovs[i_iov].index
            *  fixme do I need to check sth here ?
            *  hum, the threshold value is -1, so should be ok.
           */

         ;  if (iovs[i_iov].index != dstidx)
            {  n_entries++
            ;  iovs[n_entries]   =  iovs[i_iov]
            ;  iovs[i_iov].index =  dstidx
            ;  iovs[i_iov].ref   =  n_entries
            ;  iovs[i_iov].value =  0.0
         ;  }

            iovs[i_iov].value   +=  facval * colivp->val
         ;  i_iov = iovs[i_iov].ref
         ;
         }
      }
      vecd = mclvResize(vecd, n_entries)

   ;  if (n_entries)
      {  int i_iov = 0
      ;  int i_ivp = n_entries

      ;  while (--i_ivp, iovs[i_iov].index >= 0)
         {  vecd->ivps[i_ivp].idx = iovs[i_iov].index
         ;  vecd->ivps[i_ivp].val = iovs[i_iov].value
         ;  i_iov = iovs[i_iov].ref
      ;  }
      }

      if (cleanup)
      mcxFree(iovs)
   ;  return vecd
;  }



struct compose_data
{  long id
;  const mclx* m1
;  const mclx* dest
;  int maxdensity
;  mclIOV*  iov
;
}  ;



void compose_thread
(  mclx* m2
,  dim colidx
,  void* data
,  dim thread_id
)
   {  struct compose_data* cd = ((struct compose_data*) data + thread_id)
   ;  mclxVectorCompose(cd->m1, m2->cols+colidx, cd->dest->cols+colidx, cd->iov)
   ;  if (cd->maxdensity)
      mclvSelectHighestGT(cd->dest->cols+colidx, cd->maxdensity)
;  }



mclMatrix* mclxCompose
(  const mclMatrix*  m1
,  const mclMatrix*  m2
,  int               maxDensity
,  int               n_threads
)
   {  int            n_m2_cols   =  N_COLS(m2)
   ;  mclMatrix*     pr          =  0
   ;  const char* valp           =  getenv("MCLEDGE_NCPUS")
   ;  mclxComposeHelper *ch

   ;  if (!n_threads && valp)
         n_threads = atoi(valp)
,fprintf(stderr, "threads now %d\n", (int) n_threads)

   ;  ch  =    mclxComposePrepare(m1, m2, n_threads)

   ;  pr  =    mclxAllocZero
               (  mclvCopy(NULL, m2->dom_cols)
               ,  mclvCopy(NULL, m1->dom_rows)
               )

   ;  if (pr)
      {  if (ch->n_jobs == 1)
         {  while (--n_m2_cols >= 0)
            {  mclxVectorCompose
               (  m1
               ,  m2->cols + n_m2_cols
               ,  pr->cols + n_m2_cols
               ,  ch->iovs[0]
               )
            ;  if (maxDensity)
               mclvSelectHighestGT
               (  pr->cols + n_m2_cols
               ,  maxDensity
               )
         ;  }
         }
         else
         {  int i
         ;  struct compose_data* cds = mcxAlloc(ch->n_jobs * sizeof cds[0], EXIT_ON_FAIL)
         ;  for (i=0; i<ch->n_jobs; i++)
            {  struct compose_data *cd = cds+i
            ;  cd->id = i
            ;  cd->m1 = m1
            ;  cd->dest = pr
            ;  cd->maxdensity = maxDensity
            ;  cd->iov = ch->iovs[i]
         ;  }
            mclxVectorDispatch((mclx*) m2, cds, ch->n_jobs, compose_thread, NULL)
         ;  mcxFree(cds)
      ;  }
      }
      mclxComposeRelease(&ch)
   ;  return pr
;  }



