/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/*
 * This approach keeps long distances > 2T as well as nw <= T
 * If we have d(new, k) <= T, then long(k) can be removed from todo.
 * If we have d(new, k) > 2T, then short(k) can be removed from todo.
 *
*/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include "impala/io.h"
#include "impala/matrix.h"
#include "impala/tab.h"
#include "impala/stream.h"
#include "impala/ivp.h"
#include "impala/compose.h"
#include "impala/vector.h"
#include "impala/app.h"
#include "impala/iface.h"
#include "impala/pval.h"

#include "util/types.h"
#include "util/ding.h"
#include "util/ting.h"
#include "util/io.h"
#include "util/err.h"
#include "util/equate.h"
#include "util/minmax.h"
#include "util/rand.h"
#include "util/array.h"
#include "util/opt.h"

#include "mcl/proc.h"
#include "mcl/procinit.h"
#include "mcl/alg.h"

#include "clew/clm.h"
#include "clew/cat.h"

const char* usagelines[];

const char* me = "mcxtest";

void usage
(  const char**
)  ;

mclv* reduce_v
(  const mclv* u
)
   {  mclv* v = mclvClone(u)
   ;  dim n = v->n_ivps
   ;  double s = mclvSum(v)
   ;  double sq = mclvPowSum(v, 2.0)
   ;  if (s)
      mclvSelectGqBar(v, 0.25 * sq / s)
;fprintf(stderr, "from %d to %d entries\n", (int) n, (int) v->n_ivps)
   ;  return v
;  }


static dim update_meet_zip
(  mclVector*  v1
,  mclp* ivp1max
,  const mclVector*  v2
)
   {  mclp  *ivp1 = v1->ivps
         ,  *ivp2 = v2->ivps
         ,  *ivp2max = ivp2 + v2->n_ivps

   ;  dim n_zeroed = 0
   ;  while (ivp1 < ivp1max && ivp2 < ivp2max)
      {  if (ivp1->idx < ivp2->idx)
         ivp1++
      ;  else if (ivp1->idx > ivp2->idx)
         ivp2++
      ;  else
            ivp1++->val = 0
         ,  ivp2++
         ,  n_zeroed++
   ;  }
      return n_zeroed
;  }


static dim update_todo
(  mclv* todo
,  mclp* end
,  const mclv* strike
)
   {  dim i, n_zero = 0
   ;  mclp* p = strike->ivps, *pmax = p + strike->n_ivps
   ;  while (p < pmax)
      {  if (todo->ivps[p->idx].val)
         n_zero++
      ;  todo->ivps[p++->idx].val = 0.0
   ;  }
      return n_zero
;  }


static double mclv_inner
(  const mclv* a
,  const mclv* b
,  dim N
)
   {  dim j
   ;  double ip = 0.0

   ;  if (a->n_ivps < N || b->n_ivps < N)
      return mclvIn(a, b)

   ;  for (j=0;j<a->n_ivps;j++)
      ip += a->ivps[j].val * b->ivps[j].val
   ;  return ip
;  }


double pearson
(  const mclv* v1
,  const mclv* v2
,  const mclv* sums
,  const mclv* Nssq
,  double N
)
   {  ofs    c       =  v1->vid
   ;  ofs    d       =  v2->vid 
   ;  double s1      =  sums->ivps[c].val
   ;  double s2      =  sums->ivps[d].val
   ;  double Nsq1    =  Nssq->ivps[c].val
   ;  double Nsq2    =  Nssq->ivps[d].val

   ;  double nom     =  sqrt(Nsq1 - s1 * s1) *  sqrt(Nsq2 - s2 * s2)
   ;  return nom ? (N* mclv_inner(v1, v2, N) - s1*s2) / nom : 0.0
;  }


void add_edge
(  mclx* m
,  ofs c
,  ofs d
,  double v
)
   {  mclp* p1 = mclxInsertIvp(m, c, d)
   ;  mclp* p2 = mclxInsertIvp(m, d, c)
   ;  if (p1)
      p1->val = v
   ;  if (p2)
      p2->val = v
;  }



double fltxCos
(  pval     v
,  void*    not_used
)
   {  return cos(v)
;  }


int main
(  int                  argc
,  const char*          argv[]
)  
   {  mclx* tbl, *LONG, *nw, *buffer_nw, *buffer_long
   ;  mclv* Nssqs, *sums, *scratch_nw, *scratch_long, *todo, *order
   ;  mcxIO* xfdata
   ;  dim i, c, d
   ;  dim N, n_data, n_skip = 0, n_total = 0, n_skip_batch = 0, n_total_batch = 0
   ;  mcxIO* xfout = mcxIOnew("tst", "w")
   ;  mclv themclv
   ;  mclp themclp

   ;  dim n_class1 = 0, n_class2 = 0, n_class3 = 0

   ;  double THRESHOLD = acos(0.7)

   ;  if (argc != 3) mcxDie(1, me, "need <matrix> <pearson threshold>")

   ;  xfdata = mcxIOnew(argv[1], "r")
   ;  tbl = mclxRead(xfdata, EXIT_ON_FAIL)

   ;  for (i=0;i<N_COLS(tbl);i++)
      mclvCanonicalEmbed(tbl->cols+i, tbl->cols+i, N_ROWS(tbl), 0.0)

   ;  THRESHOLD = acos(atof(argv[2]))

   ;  n_data = N_COLS(tbl)
   ;  N = N_ROWS(tbl)

   ;  srandom(mcxSeed(8910111213))

   ;  if (0)
      {  srandom(mcxSeed(8910111213))
      ;  mcxShuffle(tbl->cols, N_COLS(tbl), sizeof themclv, (void*) &themclv)
      ;  for (i=0;i<N_COLS(tbl);i++)
         tbl->cols[i].vid = i
   ;  }

      LONG  = mclxAllocZero(mclvClone(tbl->dom_cols), mclvClone(tbl->dom_cols))
   ;  nw    = mclxAllocZero(mclvClone(tbl->dom_cols), mclvClone(tbl->dom_cols))

   ;  Nssqs    =  mclvCopy(NULL, tbl->dom_cols)
   ;  sums     =  mclvCopy(NULL, tbl->dom_cols)
   ;  scratch_nw  =  mclvCopy(NULL, tbl->dom_cols)
   ;  scratch_long  =  mclvCopy(NULL, tbl->dom_cols)
   ;  todo     =  mclvCopy(NULL, tbl->dom_cols)
   ;  order    =  mclvCopy(NULL, tbl->dom_cols)

   ;  buffer_nw =  mclxAllocZero(mclvClone(tbl->dom_cols), mclvClone(tbl->dom_cols))
   ;  buffer_long= mclxAllocZero(mclvClone(tbl->dom_cols), mclvClone(tbl->dom_cols))

#define NW_BUF_SIZE 20

   ;  for (c=0;c<N_COLS(tbl);c++)
      {  Nssqs->ivps[c].val = N * mclvPowSum(tbl->cols+c, 2.0)
      ;  sums->ivps[c].val = mclvSum(tbl->cols+c)
      ;  mclvResize(buffer_nw->cols+c, NW_BUF_SIZE)
      ;  mclvResize(buffer_long->cols+c, NW_BUF_SIZE)
      ;  buffer_nw->cols[c].n_ivps = 0             /* dangersign, hugely dangerous hack. fix */
      ;  buffer_long->cols[c].n_ivps = 0          /* dangersign, hugely dangerous hack. fix */
   ;  }

      for (i=0; i<N_COLS(nw); i++)
      {  dim new = i, j
      ;  dim n_pass_nw = 0, n_pass_long = 0, n_skip_local = 0
      ;  dim n_update_todo =0

      ;  for (j=0;j<i;j++)
         todo->ivps[j].val = 1.0
      ;  todo->n_ivps = i              /* dangersign, ugly hack. requires no resizing of todo */
;  if (0) mcxShuffle(order->ivps, i, sizeof themclp, (void*) &themclp)

      ;  for (j=0;j<i;j++)
         {  double p, arc
         ;  n_total++
         ;  n_total_batch++

         ;  dim k = i - j - 1
         ;  k = j
         ;  k = order->ivps[j].idx
         ;  k = i - j - 1

         ;  if (!todo->ivps[k].val)
            {  n_skip_local++
            ;  n_skip_batch++
            ;  continue
         ;  }
            p = pearson(tbl->cols+k, tbl->cols+new, sums, Nssqs, N * 1.0)
         ;  if (p > 1.0)
            p = 1.0
         ;  arc = acos(p)

         ;  if (arc <= THRESHOLD)
            {  scratch_nw->ivps[n_pass_nw].idx = k
            ;  scratch_nw->ivps[n_pass_nw++].val = arc
            ;  n_class1++
            ;  if (LONG->cols[k].n_ivps > 0)
               n_update_todo += update_todo(todo, todo->ivps+k, LONG->cols+k)
         ;  }

            else if (0 && arc > 3 * THRESHOLD)
            {  n_class3++
            ;  if (n_pass_long < 0.9 * todo->n_ivps)     /* magic */
               {  dim l
               ;  for (l=0;l<nw->cols[k].n_ivps;l++)
                  {  ofs id = nw->cols[k].ivps[l].idx
                  ;  n_update_todo += update_todo(todo, todo->ivps+k, nw->cols+id)
               ;  }
               }
            }

            else if (arc > 2 * THRESHOLD)
            {  n_class2++
            ;  if (n_pass_long < 0.9 * todo->n_ivps)     /* magic */
               {  scratch_long->ivps[n_pass_long].idx = k
               ;  scratch_long->ivps[n_pass_long++].val = arc
               ;  if (nw->cols[k].n_ivps > 0)
                  n_update_todo += update_todo(todo, todo->ivps+k, nw->cols+k)
            ;  }
            }
         }

         n_skip += n_skip_local
      ;  mclvFromIvps(nw->cols+new, scratch_nw->ivps, n_pass_nw)
      ;  mclvFromIvps(LONG->cols+new, scratch_long->ivps, n_pass_long)
      ;

#define BUFFER_LONG_EDGES 0

#if 0
      ;  if (BUFFER_LONG_EDGES)
         for (j=0;j<n_pass_long;j++)
         {  ofs idx = scratch_long->ivps[j].idx
         ;  if (LONG->cols[idx].n_ivps < 1000)                          /* magic */
            {  dim nbuf = buffer_long->cols[idx].n_ivps
            ;  buffer_long->cols[idx].ivps[nbuf].idx = new
            ;  buffer_long->cols[idx].ivps[nbuf].val = scratch_long->ivps[j].val
            ;  buffer_long->cols[idx].n_ivps++
            ;  if (buffer_long->cols[idx].n_ivps == NW_BUF_SIZE)
               {  mclvSort(buffer_long->cols+idx, NULL)
               ;  mclvAdd(LONG->cols+idx, buffer_long->cols+idx, LONG->cols+idx)
               ;  buffer_long->cols[idx].n_ivps = 0
            ;  }
            }
      ;  }
         else
         for (j=0;j<n_pass_long;j++)
         {  if (random() * 1.0 / RAND_MAX < 0.5)
            mclvInsertIdx(LONG->cols+scratch_long->ivps[j].idx, new, scratch_long->ivps[j].val)
      ;  }
#endif
                           /* insert the short edges into the network; this is costly. */
         if (0)
         for (j=0;j<n_pass_nw;j++)
         mclvInsertIdx(nw->cols+scratch_nw->ivps[j].idx, new, scratch_nw->ivps[j].val)

      ;  if ((i+1) % 200 == 0)
            fprintf(stderr, "%.2f percent skipped\n", 100.0 * n_skip / n_total)
         ,  n_skip_batch = 0
         ,  n_total_batch =0
   ;  }

      if (1)
      mclxMergeTranspose(nw, fltMax, 0.0)

   ;  if (0)
      for (i=0;i<N_COLS(buffer_long);i++)
      {  if (buffer_long->cols[i].n_ivps)
         {  mclvSort(buffer_long->cols+i, NULL)
         ;  mclvAdd(nw->cols+i, buffer_long->cols+i, LONG->cols+i)
      ;  }
   ;  }

      if (0)
      for (i=0;i<N_COLS(LONG);i++)
      fprintf(stdout, "%d %d %d\n", (int) i, (int) nw->cols[i].n_ivps, (int) LONG->cols[i].n_ivps)

   ;  fprintf(stderr, "%.2f percent skipped (%.2f for batch)\n", 100.0 * n_skip / n_total, n_skip_batch * 100.0 / n_total_batch)
   ;  fprintf(stderr, "skipped %d, nw has %d, LONG has %d (%d per node)\n", (int) n_skip, (int) mclxNrofEntries(nw), (int) mclxNrofEntries(LONG), (int) (mclxNrofEntries(LONG) * 1.0 / N_COLS(LONG)))
   ;  fprintf(stderr, "class 1,2,3  %d  %d  %d\n", (int) n_class1,  (int) n_class2,  (int) n_class3)
   ;  mclxUnary(nw, fltxCos, NULL)
   ;  mclxWrite(nw, xfout, 8, RETURN_ON_FAIL)
   ;  return 0
;  }


const char* usagelines[] =
{  NULL
}  ;



#if 0


0) in below, is it possible to improve to the point
   where for z we do not even need to constrct V(SHORT), but could just
   navigate into appropriate parts of a graph or tree structure? Probably tough
   ... either todo[] or avoid[] is bound to become large, but one of them is
   needed.  A tree structure with provable separation properties (allowing us
   to skip large parts of search space) is orthogonal to this approach, a
   hypothetical alien concept with no hooks to fit.

   Parallelisation
      a) immediately by partitioning into disjoint subgraphs (requiring 2)
      b) 3) below is paralleziable by node sets.
      c) 2)

   Is 1) threadable? Perhaps if y is instead taken in bunches of y[t];


1) compute a delta graph.
   How much could below gain us?

      {  V(SHORT) <- ()
         V(LONG)  <- ()

         TODO     <- V()

         while (TODO) {

            next z from TODO

            todo <- V(SHORT)                    # search all that came before: initialise to complete vector.
            V(SHORT) += z

                                 # note: probably best to randomise TODO.
            while (todo) {

               THREAD y over todo[todo != 0] {

                  compute distance(x,y)

                  if d(y,z) < delta then   
                     add (y,z) to SHORT            # update y-column - thread-safe, one y per thread.
                     add (z,y) to SHORT            # threads all pin on z; stick in thread-local ivp array.
                     rm LONG(y,*) from todo        #
                              # http://stackoverflow.com/questions/8315931/does-writing-the-same-value-to-the-same-memory-location-cause-a-data-race
                              # setting to zero should work ...
                                                   #     LONG(y*,*) and LONG(y**,*) may intersect

                  if d(y,z) > 2 delta then
                     add (y,z) to LONG             # update y-column - thread-safe, one y per thread.
                     add (z,y) to LONG             # threads all pin on z; stick in thread-local ivparray.
                     rm SHORT(y,*) from todo       #
                                                   # see above; setting to 0.0 should work.

               ?  if (y,z) > 3 delta then
                     add (SHORT(y,*),z) to LONG
               }

               # add thread-local SHORT(z,) and LONG(z,) arrays to column z;
               # combine in single ivp array vectorFromIvps.
            }
         }
      }


2)    Compute the union of two disjoint delta graphs.

      {  V(SHORT) <- (V_1,)
         V(LONG)  <- (V_1,)

         TODO     <- V_2()

         while (TODO) {

            next z from TODO

            todo <- left(SHORT)

            while (todo) {
               next y from todo

               if d(y,z) < delta then   
                  rm LONG(y,*) from todo
                  add (y,z) to SHORT

               if (y,z) > 2 delta then
                  add (y,z) to LONG
                  rm  SHORT(y,*) from todo
            }
         }
      }


      Or is there any use in using 



3)    Extend a delta graph to a delta + epsilon graph.
      assume epsilon < delta. Can we utilise the epsilon graph?
      Yes, given a pair x,y with d(x,y) > delta, we need only
      consider
         epsilon(delta(x,*), *) / delta(x,*)





understand slink!

#endif


