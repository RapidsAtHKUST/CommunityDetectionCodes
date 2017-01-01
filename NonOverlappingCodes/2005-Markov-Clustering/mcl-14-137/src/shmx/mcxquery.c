/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
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

#include "mcx.h"

#include "util/types.h"
#include "util/ding.h"
#include "util/ting.h"
#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/array.h"
#include "util/compile.h"
#include "util/minmax.h"
#include "util/inttypes.h"
#include "util/rand.h"

#include "impala/matrix.h"
#include "impala/compose.h"
#include "impala/io.h"
#include "impala/edge.h"

#include "clew/clm.h"
#include "gryphon/path.h"
#include "mcl/transform.h"


int valcmpdesc
(  const void*             i1
,  const void*             i2
)
   {  return *((pval*)i1) < *((pval*)i2) ? 1 : *((pval*)i1) > *((pval*)i2) ? -1 : 0
;  }


int valcmpasc
(  const void*             i1
,  const void*             i2
)
   {  return *((pval*)i1) > *((pval*)i2) ? 1 : *((pval*)i1) < *((pval*)i2) ? -1 : 0
;  }


double pearson
(  const mclv* a
,  const mclv* b
,  dim n
)
   {  double suma = mclvSum(a)
   ;  double sumb = mclvSum(b)
   ;  double sumasq = mclvPowSum(a, 2.0)
   ;  double sumbsq = mclvPowSum(b, 2.0)

   ;  double nom = sqrt( (n*sumasq - suma*suma) * (n*sumbsq - sumb*sumb) )
   ;  double num = n * mclvIn(a, b) - suma * sumb
   ;  return nom ? num / nom : 0.0
;  }


enum
{  MY_OPT_ABC    =   MCX_DISP_UNUSED
,  MY_OPT_IMX
,  MY_OPT_ICL
,  MY_OPT_TAB
,  MY_OPT_DIMENSION
,  MY_OPT_SIZE
,  MY_OPT_NODE
,  MY_OPT_VARY_CORRELATION
,  MY_OPT_VARY_THRESHOLD
,  MY_OPT_VARY_KNN
,  MY_OPT_VARY_N
,  MY_OPT_VARY_CEIL
,  MY_OPT_VALUES
,  MY_OPT_VALUES_SORTED
,  MY_OPT_VALUES_HIST
,  MY_OPT_DEGREES_HIST
,  MY_OPT_DIVIDE
,  MY_OPT_OUTPUT_TABLE
,  MY_OPT_OUTPUT_NOLEGEND
,  MY_OPT_SCALEFREE
,  MY_OPT_TESTCYCLE
,  MY_OPT_TESTMETRIC
,  MY_OPT_TESTCYCLE_N
,  MY_OPT_TRANSFORM
,  MY_OPT_THREAD
,  MY_OPT_CLCF
,  MY_OPT_INFO
,  MY_OPT_CLCF2
,  MY_OPT_INFO2
,  MY_OPT_REDUCE
,  MY_OPT_FOUT
}  ;


mcxOptAnchor qOptions[] =
{  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "specify input matrix/graph"
   }
,  {  "-abc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ABC
   ,  "<fname>"
   ,  "specify input using label pairs"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "specify tab file to be used with matrix input"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "specify input clustering"
   }
,  {  "--node-attr"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_NODE
   ,  NULL
   ,  "output for each node its weight statistics and degree"
   }
,  {  "-t"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_THREAD
   ,  "<num>"
   ,  "number of threads to use (with -knn)"
   }
,  {  "--reduce"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_REDUCE
   ,  NULL
   ,  "do not rebase to input graph, use last reduction"
   }
,  {  "--dim"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DIMENSION
   ,  NULL
   ,  "get matrix dimensions"
   }
,  {  "--size"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_SIZE
   ,  NULL
   ,  "get number of entries"
   }
,  {  "--test-metric"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TESTMETRIC
   ,  NULL
   ,  "test whether graph distance is metric"
   }
,  {  "--test-cycle"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TESTCYCLE
   ,  NULL
   ,  "test whether graph has cycles"
   }
,  {  "-test-cycle"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TESTCYCLE_N
   ,  "<num>"
   ,  "output at most <num> nodes in cycles; 0 for all"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_FOUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "--vary-correlation"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_VARY_CORRELATION
   ,  NULL
   ,  "vary correlation threshold, output graph statistics"
   }
,  {  "-vary-threshold"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_VARY_THRESHOLD
   ,  "start/end/step"
   ,  "vary threshold from start to end by step"
   }
,  {  "-vary-knn"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_VARY_KNN
   ,  "start/end/step"
   ,  "vary knn from end down to start by step"
   }
,  {  "-vary-n"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_VARY_N
   ,  "start/end/step"
   ,  "vary n from end down to start by step"
   }
,  {  "-vary-ceil"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_VARY_CEIL
   ,  "start/end/step"
   ,  "vary ceil from end down to start by step"
   }
,  {  "--values"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_VALUES
   ,  NULL
   ,  "output all entries/weights, unsorted"
   }
,  {  "--values-sorted"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_VALUES_SORTED
   ,  NULL
   ,  "output all entries/weights, sorted"
   }
,  {  "-values-hist"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_VALUES_HIST
   ,  "nbins or start/end/nbins"
   ,  "vary histogram in nbins steps from start to end"
   }
,  {  "-degrees-hist"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DEGREES_HIST
   ,  "s"
   ,  "use step size s"
   }
,  {  "-div"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DIVIDE
   ,  "<num>"
   ,  "divide in sets with property <= num and > num"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to matrix values"
   }
,  {  "--eff"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_INFO
   ,  NULL
   ,  "compute efficiency criterion"
   }
,  {  "--clcf"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_CLCF
   ,  NULL
   ,  "compute clustering coefficient"
   }
,  {  "-eff"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_INFO2
   ,  "mean node degree threshold"
   ,  "compute efficiency criterion"
   }
,  {  "-clcf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CLCF2
   ,  "mean node degree threshold"
   ,  "compute clustering coefficient"
   }
,  {  "--scale-free"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_SCALEFREE
   ,  NULL
   ,  "simple scale-freeness test, correlation of log/log values"
   }
,  {  "--output-table"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_OUTPUT_TABLE
   ,  NULL
   ,  "output tab separated table without key"
   }
,  {  "--no-legend"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_OUTPUT_NOLEGEND
   ,  NULL
   ,  "do not output explanatory key"
   }
,  {  NULL ,  0 ,  0,  NULL, NULL}
}  ;


static  const char* me     =  "mcx query";
static  mcxIO*  xfout_g    =   (void*) -1;
static  mcxIO* xfmx_g      =   (void*) -1;
static  mcxIO* xfabc_g     =   (void*) -1;
static  mcxIO* xfcl_g      =   (void*) -1;
static  mcxIO* xftab_g     =   (void*) -1;
static  mclTab* tab_g      =   (void*) -1;

static  int trigger_clcf       =  -1;
static  int trigger_eff        =  -1;

static  unsigned int divide_g=  -1;
static  unsigned mode_vary = -1;


struct vary_threshold
{  double   start
;  double   end
;  double   increment
;  unsigned nbins
;  int      active
;  int      iter
;
}  ;

struct vary_step
{  unsigned starti
;  unsigned endi
;  unsigned stepi
;  int      active
;  int      iter
;
}  ;

struct vary_threshold VT = { 0.0, 0.0, 0.0, 0, 0, 0 };
struct vary_step VS = { 0, 0, 0, 0, 0 };


enum
{  VARY_UNSPECIFIED = 0
,  VARY_CORRELATION
,  VARY_THRESHOLD
,  VARY_CEIL
,  VARY_KNN
,  VARY_N
,  VARY_UNKNOWN
}  ;
static  unsigned mode_dispatch  =     -1;
enum
{  DISPATCH_UNSPECIFIED = 0
,  DISPATCH_VALUES
,  DISPATCH_VALUES_HIST
,  DISPATCH_DEGREES_HIST
,  DISPATCH_VALUES_SORTED
,  DISPATCH_DIMENSION
,  DISPATCH_SIZE
,  DISPATCH_TESTMETRIC
,  DISPATCH_TESTCYCLE
,  DISPATCH_NODEATTR
}  ;
static  unsigned n_limit = 0;
static  mcxbool weefreemen = -1;
static  mcxbool rebase_g = -1;
static  mcxbits output_flags = -1;
static  mcxbool user_imx = -1;
static  mcxbool transpose  = -1;
static  dim n_thread_l = -1;
static  mclgTF* transform  =   (void*) -1;
static  mcxTing* transform_spec = (void*) -1;


static struct level* levels;


static mclv* matrix_vector
(  const mclx* mx
,  const mclv* vec
)
   {  mclv* res = mclvClone(mx->dom_rows)
   ;  dim i, j
   ;  mclvMakeConstant(res, 0.0)
   ;  for (i=0;i<vec->n_ivps;i++)
      {  mclv* c = mx->cols + vec->ivps[i].idx
      ;  for (j=0;j<c->n_ivps;j++)
         res->ivps[c->ivps[j].idx].val += 1.0
   ;  }
      mclvUnary(res, fltxCopy, NULL)
   ;  return res
;  }


static mclv* run_through
(  const mclx* mx
)
   {  mclv* starts = mclvClone(mx->dom_cols)
   ;  dim n_starts_previous = starts->n_ivps + 1
   ;  dim n_steps = 0

   ;  while (n_starts_previous > starts->n_ivps)
      {  mclv* new_starts = matrix_vector(mx, starts)
      ;  mclvMakeCharacteristic(new_starts)
      ;  n_starts_previous = starts->n_ivps
      ;  mclvFree(&starts)
      ;  starts = new_starts
      ;  n_steps++
      ;  fputc('.', stderr)
   ;  }
      if (n_steps)
      fputc('\n', stderr)
   ;  return starts
;  }


static int test_cycle
(  const mclx* mx
,  dim n_limit
)
   {  mclv* starts = run_through(mx), *starts2
   ;  if (starts->n_ivps)
      {  dim i
      ;  if (n_limit)
         {  mclx* mxt = mclxTranspose(mx)
         ;  starts2 = run_through(mxt)
         ;  mclxFree(&mxt)
         ;  mclvBinary(starts, starts2, starts, fltMultiply)

         ;  mcxErr
            (me, "cycles detected (%u nodes)", (unsigned) starts->n_ivps)

         ;  if (starts->n_ivps)
            {  fprintf(stdout, "%lu", (ulong) starts->ivps[0].idx)
            ;  for (i=1; i<MCX_MIN(starts->n_ivps, n_limit); i++)
               fprintf(stdout, " %lu", (ulong) starts->ivps[i].idx)
            ;  fputc('\n', stdout)
         ;  }
            else
            mcxErr(me, "strange, no nodes selected")
      ;  }
         else
         mcxErr(me, "cycles detected")
      ;  return 1
   ;  }

      mcxTell(me, "no cycles detected")
   ;  return 0
;  }


static mcxstatus qInit
(  void
)
   {  mode_dispatch =  DISPATCH_UNSPECIFIED
   ;  n_limit  =  0
   ;  xfout_g  =  mcxIOnew("-", "w")
   ;  xftab_g  =  NULL
   ;  tab_g    =  NULL
   ;  divide_g =  3
   ;  xfmx_g   =  mcxIOnew("-", "r")
   ;  xfabc_g  =  NULL
   ;  xfcl_g   =  NULL
   ;  n_thread_l = 1
   ;  mode_vary = VARY_UNSPECIFIED
   ;  transpose=  FALSE
   ;  rebase_g = TRUE
   ;  weefreemen = FALSE
#define OUTPUT_TABLE 1
#define OUTPUT_KEY 2
   ;  output_flags = OUTPUT_KEY
   ;  user_imx =  FALSE
   ;  transform      =  NULL
   ;  transform_spec =  NULL
   ;  return STATUS_OK
;  }


mcxstatus parse_steps
(  const char* a
,  unsigned *startp
,  unsigned *endp
,  unsigned *stepp
)
   {  dim dash_count = mcxStrCountChar(a, '/', strlen(a))
   ;  int ok = 0
   ;  do
      {  if (dash_count != 2)
         break
      ;  if (3 != sscanf(a, "%u/%u/%u", startp, endp, stepp))
         break
      ;  if (startp[0] > endp[0] || stepp[0] > endp[0]-startp[0] || stepp[0] <= 0)
         break
      ;  ok = 1
   ;  }
      while(0)
   ;  return ok
;  }


mcxstatus parse_threshold
(  const char* a
,  double *startp
,  double *endp
,  unsigned* n
)
   {  dim dash_count = mcxStrCountChar(a, '/', strlen(a))
   ;  int ok = 0
   ;  do
      {  if (dash_count != 0 && dash_count != 2)
         break
      ;  if (dash_count == 2 && 3 != sscanf(a, "%lf/%lf/%d", startp, endp, n))
         break
      ;  if (dash_count == 0 && 1 != sscanf(a, "%d", n))
         break
      ;  if (dash_count == 2 && startp[0] > endp[0])
         break
      ;  if (!n[0])
         break
      ;  ok = 1
   ;  }
      while(0)
   ;  return ok
;  }



static mcxstatus qArgHandle
(  int optid
,  const char* val
)
   {  int n_modes = 0
   ;  switch(optid)
      {  case MY_OPT_IMX
      :  mcxIOnewName(xfmx_g, val)
      ;  user_imx = TRUE
      ;  break
      ;

         case MY_OPT_ABC
      :  xfabc_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_TAB
      :  xftab_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_ICL
      :  xfcl_g =  mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_TRANSFORM
      :  transform_spec = mcxTingNew(val)
      ;  break
      ;

         case MY_OPT_SIZE
      :  mode_dispatch = DISPATCH_SIZE
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_DIMENSION
      :  mode_dispatch = DISPATCH_DIMENSION
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_TESTMETRIC
      :  mode_dispatch = DISPATCH_TESTMETRIC
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_TESTCYCLE
      :  mode_dispatch = DISPATCH_TESTCYCLE
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_TESTCYCLE_N
      :  n_limit = atoi(val)
      ;  mode_dispatch = DISPATCH_TESTCYCLE
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_INFO2
      :  trigger_eff = atoi(val)
      ;  break
      ;
         case MY_OPT_CLCF2
      :  trigger_clcf = atoi(val)
      ;  break
      ;

         case MY_OPT_INFO
      :  trigger_eff = 0
      ;  break
      ;
         case MY_OPT_CLCF
      :  trigger_clcf = 0
      ;  break
      ;

         case MY_OPT_REDUCE
      :  rebase_g = FALSE
      ;  break
      ;

         case MY_OPT_THREAD
      :  n_thread_l = atoi(val)
      ;  break
      ;

         case MY_OPT_NODE
      :  mode_dispatch = DISPATCH_NODEATTR
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_FOUT
      :  mcxIOnewName(xfout_g, val)
      ;  break
      ;

         case MY_OPT_OUTPUT_NOLEGEND
      :  output_flags ^= OUTPUT_KEY
      ;  break
      ;

         case MY_OPT_OUTPUT_TABLE
      :  output_flags |= OUTPUT_TABLE
      ;  break
      ;

         case MY_OPT_SCALEFREE
      :  weefreemen = TRUE
      ;  break
      ;

         case MY_OPT_VALUES
      :  mode_dispatch = DISPATCH_VALUES
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_DEGREES_HIST                  /* mq check end condition */
      :  mode_dispatch = DISPATCH_DEGREES_HIST
      ;  n_modes++
      ;  VS.stepi = atoi(val)
      ;  VS.active = 1
      ;  break
      ;

         case MY_OPT_VALUES_HIST
      :  mode_dispatch = DISPATCH_VALUES_HIST
      ;  n_modes++
      ;  if (!parse_threshold(val, &VT.start, &VT.end, &VT.nbins))
         mcxDie(1, me, "failed to parse argument as #bins or start/end/#bins")
      ;  VT.active =1
      ;  break
      ;

         case MY_OPT_VALUES_SORTED
      :  mode_dispatch = DISPATCH_VALUES_SORTED
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_DIVIDE
      :  divide_g = atoi(val)
      ;  break
      ;

         case MY_OPT_VARY_CORRELATION
      :  VT.start = 0.2
      ;  VT.end   = 1.0
      ;  VT.nbins = 20
      ;  VT.active = 1
      ;  mode_vary = VARY_CORRELATION
      ;  n_modes++
      ;  break
      ;

         case MY_OPT_VARY_THRESHOLD
      :  mode_vary = VARY_THRESHOLD
      ;  n_modes++
      ;  if (!parse_threshold(val, &VT.start, &VT.end, &VT.nbins))
         mcxDie(1, me, "failed to parse argument as #bins or start/end/#bins")
      ;  VT.active = 1
      ;  break
      ;

         case MY_OPT_VARY_CEIL
      :  case MY_OPT_VARY_KNN
      :  case MY_OPT_VARY_N
      :  if (!parse_steps(val, &VS.starti, &VS.endi, &VS.stepi))
         mcxDie(1, me, "failed to parse argument as start/end/stepsize")
      ;  mode_vary
         =     optid == MY_OPT_VARY_KNN
            ?  VARY_KNN
            :  optid == MY_OPT_VARY_CEIL
            ?  VARY_CEIL
            :  optid == MY_OPT_VARY_N
            ?  VARY_N
            :  VARY_UNKNOWN
      ;  VS.active = 1
      ;  n_modes++
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
   ;  }

      if
      (  mode_vary
      && VS.stepi
      && 1000 * VS.stepi < VS.endi - VS.starti
      )
      mcxDie(1, me, "argument leads to more than one thousand steps")

   ;  if (n_modes > 1 || (VS.active && VT.active))
      mcxDie(1, me, "too many modes specified")

   ;  return STATUS_OK
;  }


struct level
{  double   threshold                   /* edges below this cut out    */

;  double   degree_cor                  /* power-law for node degrees? */

;  double   sim_median
;  double   sim_mean
;  double   sim_iqr

;  ulong    nb_median
;  double   nb_mean
;  double   nb_sum
;  double   nb_iqr

;  ulong    bigsize
;  double   cc_exp
;  double   clcf
;  ulong    n_single
;  ulong    n_edge
;  ulong    n_lq
;
}  ;


static double pval_get_double
(  const void* v
)
   {  return *((pval*) v)
;  }

static double ivp_get_double
(  const void* v
)
   {  return ((mclp*) v)->val
;  }


dim get_n_sort_allvals
(  const mclx* mx
,  pval* allvals
,  dim noe
,  double* sum_vals
,  mcxbool asc
)
   {  dim n_allvals = 0
   ;  double s = 0.0
   ;  dim j
   ;  for (j=0;j<N_COLS(mx);j++)
      {  mclv* vec = mx->cols+j
      ;  dim k
      ;  if (n_allvals + vec->n_ivps > noe)
         mcxDie(1, me, "panic/impossible: not enough memory")
      ;  for (k=0;k<vec->n_ivps;k++)
         allvals[n_allvals+k] = vec->ivps[k].val
      ;  n_allvals += vec->n_ivps
      ;  s += mclvSum(vec)
   ;  }
      if (asc)
      qsort(allvals, n_allvals, sizeof(pval), valcmpasc)
   ;  else
      qsort(allvals, n_allvals, sizeof(pval), valcmpdesc)
   ;  sum_vals[0] = s
   ;  return n_allvals
;  }



static void do_vary_threshold
(  mclx*  mx
,  FILE*  fp
,  unsigned mode
)
   {  dim cor_i = 0, j

   ;  mclx* mx_start = mclxCopy(mx)
   ;  unsigned long noe = 0, n_edges_level =0
   ;  pval*  allvals
   ;  dim  n_allvals = 0
   ;  double sum_vals = 0.0

   ;  noe = mclxNrofEntries(mx)
   ;  allvals = mcxAlloc(noe * sizeof allvals[0], EXIT_ON_FAIL)
   ;  n_allvals = get_n_sort_allvals(mx, allvals, noe, &sum_vals, FALSE)
            /* ^ sort descending */

   ;  if (!VT.start && !VT.end && VT.nbins)
      {  double start =  n_allvals ? allvals[n_allvals-1] : 0.0 
      ;  double end   =  n_allvals ? allvals[0]           : 1.0
      ;  double width =  (end-start)
      ;  VT.start     =  start - width/(VT.nbins*10)
      ;  VT.increment =  width / VT.nbins
      ;  VT.end       = end
;fprintf(stderr, "%f %f %f %f\n", VT.start, VT.end, width, VT.increment)
   ;  }
      else
      VT.increment = VT.start < VT.end && VT.nbins ? (VT.end - VT.start) / VT.nbins : 1.0

   ;  if (mode == VARY_CORRELATION)
      {  double smallest = n_allvals ? allvals[n_allvals-1] : 0.0
      ;  double increment = (VT.end - VT.start) / VT.nbins

      ;  if (smallest < 0)
         VT.start = -1.0

      ;  if (VT.start < smallest)
         {  while (VT.start < smallest && VT.nbins)
               VT.start += increment
            ,  VT.nbins--
         ;  VT.start -= increment
         ;  VT.nbins++
      ;  }
         mcxTell
         (  me
         ,  "smallest correlation is %.2f, using starting point %.2f"
         ,  smallest
         ,  VT.start
         )
      ;  VT.increment = increment
   ;  }

      if (output_flags & OUTPUT_TABLE)
      {  fprintf(fp, "L\tD\tR\tS\tE\tcce\tEWmean\tEWmed\tEWiqr\tNDmean\tNDmed\tNDiqr\tCCF\teff\t%s\n", mode == VARY_KNN ? "kNN" : mode == VARY_N ? "N" : "Cutoff")
;     }
      else
      {  if (output_flags & OUTPUT_KEY)
         {
;fprintf(fp, "-------------------------------------------------------------------------------\n")
;fprintf(fp, " L       Percentage of nodes in the largest component\n")
;fprintf(fp, " D       Percentage of nodes in components of size at most %d [-div option]\n", (int) divide_g)
;fprintf(fp, " R       Percentage of nodes not in L or D: 100 - L -D\n")
;fprintf(fp, " S       Percentage of nodes that are singletons\n")
;fprintf(fp, " E       Fraction of edges retained (input graph has %lu)\n", (ulong) noe)
;fprintf(fp, " cce     Expected size of component, nodewise [ sum(sz^2) / sum^2(sz) ]\n")
;fprintf(fp, " EW      Edge weight traits (mean, median and IQR)\n")
;fprintf(fp, " ND      Node degree traits [mean, median and IQR]\n")
;fprintf(fp, " CCF     Clustering coefficient (scale 1-100)\n")
;fprintf(fp, " eff     Induced component efficiency relative to start graph (scale 1-1000)\n")

;if (mode == VARY_CORRELATION)
 fprintf(fp, "Cutoff   The threshold used.\n")
;else if (mode == VARY_KNN)
 fprintf(fp, "k-NN     The knn parameter\n")
;else if (mode == VARY_N)
 fprintf(fp, "N        The knn parameter (merge mode)\n")
;else if (mode == VARY_CEIL)
 fprintf(fp, "ceil     The ceil parameter\n")
;fprintf(fp, "Total number of nodes: %lu\n", (ulong) N_COLS(mx))
;        }
 fprintf(fp, "----------------------------------------------------------------------------------------------\n")
;fprintf(fp, "  L   D   R   S     E     cce  EWmean   EWmed   EWiqr  NDmean   NDmed  NDiqr CCF  eff %7s \n", mode == VARY_KNN ? "k-NN" : mode == VARY_N ? "N" : mode == VARY_CEIL ? "Ceil" : "Cutoff")
;fprintf(fp, "----------------------------------------------------------------------------------------------\n")
;     }

VT.iter = 0;
      while (1)
      {  double cutoff = 0.0
      ;  double eff = -1.0
      ;  mclv* nnodes = mclvCanonical(NULL, N_COLS(mx), 0.0)
      ;  mclv* degree = mclvCanonical(NULL, N_COLS(mx), 0.0)
      ;  dim i, n_sample = 0
      ;  double cor, y_prev, iqr = 0.0
      ;  mclx* cc = NULL, *res = NULL
      ;  mclv* sz, *ccsz = NULL
      ;  int step = 0, step2 = 0

      ;  if (VT.active)
         {  cutoff = VT.start + VT.iter * VT.increment
         ;  if (++VT.iter > VT.nbins)
            break
      ;  }
         else if (VS.active)
         {  step = VS.starti + VS.iter * VS.stepi
         ;  if (step > VS.endi)
            break
         ;  step2 = (int) (VS.endi + VS.starti) - step
         ;  VS.iter++
      ;  }
         else
         break
      ;

      ;  sum_vals = 0.0
      
      ;  if (mode == VARY_THRESHOLD || mode == VARY_CORRELATION)
            mclxSelectValues(mx, &cutoff, NULL, MCLX_EQT_GQ)
         ,  res = mx
      ;  else if (mode == VARY_KNN)
         {  res = rebase_g ? mclxCopy(mx) : mx
         ;  mclgKNNdispatch(res, step2, n_thread_l, 1)
      ;  }
         else if (mode == VARY_N)
         {  res = mx
         ;  mclgKNNdispatch(res, step2, n_thread_l, 0)
      ;  }
         else if (mode == VARY_CEIL)
         {  res = rebase_g ? mclxCopy(mx) : mx
         ;  mclv* cv = mclgCeilNB(res, step2, NULL, NULL, NULL)
         ;  mclvFree(&cv)
      ;  }

         sz = mclxColSizes(res, MCL_VECTOR_COMPLETE)
      ;  n_edges_level = mclvSum(sz)
      ;  mclvSortDescVal(sz)
      ;  levels[cor_i].nb_mean   =  mclvSum(sz) / N_COLS(res)

      ;  cc = clmUGraphComponents(res, NULL)     /* fixme: user has to specify -tf '#max()' if graph is directed */
      ;  if (cc)
         {  ccsz = mclxColSizes(cc, MCL_VECTOR_COMPLETE)
         ;  if (!trigger_eff || levels[cor_i].nb_mean <= trigger_eff)
            {  clmPerformanceTable pftable
            ;  clmPerformance(mx_start, cc, &pftable)
            ;  eff = pftable.efficiency
         ;  }
         }

         if (mode == VARY_THRESHOLD || mode == VARY_CORRELATION)
         {  for
            (
            ;  n_allvals > 0 && allvals[n_allvals-1] < cutoff
            ;  n_allvals--
            )
         ;  sum_vals = 0.0
         ;  for (i=0;i<n_allvals;i++)
            sum_vals += allvals[i]
      ;  }
         else if (mode == VARY_KNN || mode == VARY_CEIL || mode == VARY_N)
         {  n_allvals = get_n_sort_allvals(res, allvals, noe, &sum_vals, FALSE)
      ;  }

         levels[cor_i].sim_median=  mcxMedian(allvals, n_allvals, sizeof allvals[0], pval_get_double, &iqr)
      ;  levels[cor_i].sim_iqr   =  iqr
      ;  levels[cor_i].sim_mean  =  n_allvals ? sum_vals / n_allvals : 0.0

      ;  levels[cor_i].nb_median =  mcxMedian(sz->ivps, sz->n_ivps, sizeof sz->ivps[0], ivp_get_double, &iqr)
      ;  levels[cor_i].nb_iqr    =  iqr
      ;  levels[cor_i].cc_exp    =  cc ? mclvPowSum(ccsz, 2.0) / N_COLS(res) : 0
      ;  levels[cor_i].nb_sum    =  mclxNrofEntries(res)

      ;  if (!trigger_clcf || levels[cor_i].nb_mean <= trigger_clcf)
         {  mclv* clcf = mclgCLCFdispatch(res, n_thread_l)
         ;  levels[cor_i].clcf   =  mclvSum(clcf) / N_COLS(mx)
         ;  mclvFree(&clcf)
      ;  }
         else
         levels[cor_i].clcf = -1.0

      ;  levels[cor_i].threshold =  mode == VARY_KNN || mode == VARY_N || mode == VARY_CEIL ? step2 : cutoff
      ;  levels[cor_i].bigsize   =  cc ? cc->cols[0].n_ivps : 0
      ;  levels[cor_i].n_single  =  0
      ;  levels[cor_i].n_edge    =  n_allvals
      ;  levels[cor_i].n_lq      =  0

      ;  if (cc)
         for (i=0;i<N_COLS(cc);i++)
         {  dim n = cc->cols[N_COLS(cc)-1-i].n_ivps
         ;  if (n == 1)
            levels[cor_i].n_single++
         ;  if (n <= divide_g)
            levels[cor_i].n_lq += n
         ;  else
            break
      ;  }

         if (levels[cor_i].bigsize <= divide_g)
         levels[cor_i].bigsize = 0

      ;  y_prev = sz->ivps[0].val

                  /* wiki says:
                     A scale-free network is a network whose degree distribution follows a power
                     law, at least asymptotically. That is, the fraction P(k) of nodes in the
                     network having k connections to other nodes goes for large values of k as P(k)
                     ~ k^−g where g is a constant whose value is typically in the range 2<g<3,
                     although occasionally it may lie outside these bounds.
                 */
      ;  for (i=1;i<sz->n_ivps;i++)
         {  double y = sz->ivps[i].val
         ;  if (y > y_prev - 0.5)
            continue                                              /* same as node degree seen last */
         ;  nnodes->ivps[n_sample].val = log( (i*1.0) / (1.0*N_COLS(res)))    /* x = #nodes >= k, as fraction   */
         ;  degree->ivps[n_sample].val = log(y_prev ? y_prev : 1)            /* y = k = degree of node         */
         ;  n_sample++
;if(0)fprintf(stderr, "k=%.0f\tn=%d\t%.3f\t%.3f\n", (double) y_prev, (int) i, (double) nnodes->ivps[n_sample-1].val, (double) degree->ivps[n_sample-1].val)
         ;  y_prev = y
      ;  }
         nnodes->ivps[n_sample].val = 0
      ;  nnodes->ivps[n_sample++].val = log(y_prev ? y_prev : 1)
;if(0){fprintf(stderr, "k=%.0f\tn=%d\t%.3f\t%.3f\n", (double) sz->ivps[sz->n_ivps-1].val, (int) N_COLS(res), (double) nnodes->ivps[n_sample-1].val, (double) degree->ivps[n_sample-1].val)
;}

      ;  mclvResize(nnodes, n_sample)
      ;  mclvResize(degree, n_sample)
      ;  cor = pearson(nnodes, degree, n_sample)

      ;  levels[cor_i].degree_cor =  cor * cor

;if(0)fprintf(stdout, "cor at cutoff %.2f %.3f\n\n", cutoff, levels[cor_i-1].degree_cor)
      ;  mclvFree(&nnodes)
      ;  mclvFree(&degree)
      ;  mclvFree(&sz)
      ;  mclvFree(&ccsz)
      ;  mclxFree(&cc)

;  if(output_flags & OUTPUT_TABLE)
   {  fprintf
      (  fp
      ,  "%lu\t%lu\t%lu\t%lu"
         "\t%g\t%lu"
         "\t%6g\t%6g\t%6g"
         "\t%6g\t%lu\t%6g"

      ,  (ulong) levels[cor_i].bigsize
      ,  (ulong) levels[cor_i].n_lq
      ,  (ulong) N_COLS(mx) - levels[cor_i].bigsize - levels[cor_i].n_lq
      ,  (ulong) levels[cor_i].n_single
      ,  (double) (noe ? (n_edges_level * 1.0 / noe) : 1.0)

      ,  (ulong) levels[cor_i].cc_exp

      ,  (double) levels[cor_i].sim_mean
      ,  (double) levels[cor_i].sim_median
      ,  (double) levels[cor_i].sim_iqr

      ,  (double) levels[cor_i].nb_mean
      ,  (ulong) levels[cor_i].nb_median
      ,  (double) levels[cor_i].nb_iqr
      )

   ;  if (levels[cor_i].clcf >= 0.0)
      fprintf(fp, "\t%6g", levels[cor_i].clcf)
   ;  else
      fputs("\tNA", fp)

   ;  if (eff >= 0.0)
      fprintf(fp, "\t%4g", eff)
   ;  else
      fputs("\tNA", fp)

   ;  fprintf(fp, "\t%6g", (double) levels[cor_i].threshold)
   ;  fputc('\n', fp)
;  }
   else
   {  fprintf
      (  fp
      ,  "%3d %3d %3d %3d %5.3f %7d"
         " %7.2f %7.2f %7.2f"
         " %7.1f %7.1f %6.1f"

      ,  0 ? 1 : (int) (0.5 + (100.0 * levels[cor_i].bigsize) / N_COLS(mx))
      ,  0 ? 1 : (int) (0.5 + (100.0 * levels[cor_i].n_lq) / N_COLS(mx))
      ,  0 ? 1 : (int) (0.5 + (100.0 * (N_COLS(mx) - levels[cor_i].bigsize - levels[cor_i].n_lq)) / N_COLS(mx))
      ,  0 ? 1 : (int) (0.5 + (100.0 * levels[cor_i].n_single) / N_COLS(mx))
      ,  0 ? 1 : (noe ? (n_edges_level * 1.0 / noe) : 1.0)
      ,  0 ? 1 : (int) (0.5 + levels[cor_i].cc_exp)

      ,  0 ? 1.0 : (double) (levels[cor_i].sim_mean                )
      ,  0 ? 1.0 : (double) (levels[cor_i].sim_median              )
      ,  0 ? 1.0 : (double) (levels[cor_i].sim_iqr                 )

      ,  0 ? 1.0 : (double) (levels[cor_i].nb_mean                 )
      ,  0 ? 1.0 : (double) (levels[cor_i].nb_median + 0.5         )
      ,  0 ? 1.0 : (double) (levels[cor_i].nb_iqr + 0.5            )
      )

   ;  if (levels[cor_i].clcf >= 0)
      fprintf(fp, " %3d", 0 ? 1 : (int) (0.5 + (100.0 * levels[cor_i].clcf)))
   ;  else
      fputs("   -", fp)

   ;  if (eff >= 0.0)
      fprintf(fp, " %4d", (int) (0.5 + 1000 * eff))
   ;  else
      fputs("    -", fp)

   ;  if (mode == VARY_CORRELATION)
      fprintf(fp, " %8.3f", (double) levels[cor_i].threshold)
   ;  else if (mode == VARY_THRESHOLD)
      fprintf(fp, " %8.2f", (double) levels[cor_i].threshold)
   ;  else if (mode == VARY_KNN || mode == VARY_CEIL || mode == VARY_N)
      fprintf(fp, " %8.0f", (double) levels[cor_i].threshold)

#if 0
/* fixme experimental */
   ;  if (levels[cor_i].clcf >= 0.0 && levels[cor_i].nb_mean > 0)
      fprintf(fp, " %5.1f", levels[cor_i].clcf * 100.0 / levels[cor_i].nb_mean)
   ;  else
      fprintf(fp, " %5s", "-")
/* emxif */
#endif

   ;  fputc('\n', fp)
 ; }

      ;  cor_i++
      ;  if (res != mx)
         mclxFree(&res)
   ;  }

   if (!(output_flags & OUTPUT_TABLE))
   {  if (weefreemen)
      {
fprintf(fp, "-------------------------------------------------------------------------------\n")
;fprintf(fp, "The graph below plots the R^2 squared value for the fit of a log-log plot of\n")
;fprintf(fp, "<node degree k> versus <#nodes with degree >= k>, for the network resulting\n")
;fprintf(fp, "from applying a particular %s cutoff.\n", mode == VARY_CORRELATION ? "correlation" : "similarity")
;fprintf(fp, "-------------------------------------------------------------------------------\n")
   ;  for (j=0;j<cor_i;j++)
      {  dim jj
      ;  for (jj=30;jj<=100;jj++)
         {  char c = ' '
         ;  if (jj * 0.01 < levels[j].degree_cor && (jj+1.0) * 0.01 > levels[j].degree_cor)
            c = 'X'
         ;  else if (jj % 5 == 0)
            c = '|'
         ;  fputc(c, fp)
      ;  }
         if (mode == VARY_CORRELATION)
         fprintf(fp, "%8.2f\n", (double) levels[j].threshold)
      ;  else
         fprintf(fp, "%8.0f\n", (double) levels[j].threshold)
   ;  }

 fprintf(fp, "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|--------\n")
;fprintf(fp, "| R^2   0.4       0.5       0.6       0.7       0.8       0.9    |  1.0    -o)\n")
;fprintf(fp, "+----+----+----+----+----+---------+----+----+----+----+----+----+----+    /\\\\\n")
;fprintf(fp, "| 2 4 6 8   2 4 6 8 | 2 4 6 8 | 2 4 6 8 | 2 4 6 8 | 2 4 6 8 | 2 4 6 8 |   _\\_/\n")
;fprintf(fp, "+----+----|----+----|----+----|----+----|----+----|----+----|----+----+--------\n")
;     }
      else
      fprintf(fp, "---------------------------------------------------------------------------------------------\n")
;  }

      mcxFree(allvals)
;  }



static ofs get_cls_id
(  long idx
,  const mclx* cl
,  const mclx* cltp
,  long* cls_size
)
   {  mclv* tocl = mclxGetVector(cltp, idx, EXIT_ON_FAIL, NULL), *cls = NULL
   ;  ofs clid = -1
   ;  cls_size[0] = 0
   ;  if (tocl)
      {  clid = tocl->ivps[0].idx
      ;  cls  = mclxGetVector(cl, clid, EXIT_ON_FAIL, NULL)
      ;  cls_size[0] = cls->n_ivps
   ;  }
      return clid
;  }


int do_attr
(  const mclx* mx
,  const mclx* cl
,  const mclx* cltp
)
   {  dim i

   ;  if (cl && !MCLD_EQUAL(cl->dom_rows, mx->dom_cols))
      mcxDie(1, "query", "cluster row domain and matrix column domains differ")

   ;  fputs("node\tdegree\tmean\tmin\tmax\tmedian\tiqr", xfout_g->fp)
   ;  if (cl)
      fputs("\tclsize\tclid", xfout_g->fp)

   ;  fputc('\n', xfout_g->fp)

   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mclvClone(mx->cols+i)
      ;  double iqr = 0, med = 0, avg = 0, max = -DBL_MAX, min = DBL_MAX

      ;  mclvSortAscVal(v)

      ;  if (v->n_ivps)
         {  med = mcxMedian(v->ivps, v->n_ivps, sizeof v->ivps[0], ivp_get_double, &iqr)
         ;  avg = mclvSum(v) / v->n_ivps
         ;  max = v->ivps[v->n_ivps-1].val
         ;  min = v->ivps[0].val
      ;  }

         mcx_dump_node(xfout_g->fp, tab_g, mx->cols[i].vid)

      ;  fprintf
         (  xfout_g->fp
         ,  "\t%lu\t%g\t%g\t%g\t%g\t%g"
         ,  (ulong) v->n_ivps
         ,  avg
         ,  min
         ,  max
         ,  med
         ,  iqr
         )
      ;  if (cl)
         {  ofs clid, clsize
         ;  clid = get_cls_id(mx->cols[i].vid, cl, cltp, &clsize)
         ;  fprintf(xfout_g->fp, "\t%ld\t%ld", (long) clsize, (long) clid)
      ;  }
         fputc('\n', xfout_g->fp)
      ;  mclvFree(&v)
   ;  }
      return 0
;  }


int do_attr_clsonly
(  const mclx* cl
,  const mclx* cltp
)
   {  dim i
   ;  ofs clid, clsize
   ;  for (i=0;i<cl->dom_rows->n_ivps;i++)
      {  mclp* p = cl->dom_rows->ivps+i
      ;  clid = get_cls_id(p->idx, cl, cltp, &clsize)
      ;  mcx_dump_node(xfout_g->fp, tab_g, p->idx)
      ;  fprintf(xfout_g->fp, "\t%ld\t%ld\n", (long) clsize, (long) clid)
   ;  }
      return 0
;  }


int do_testmetric
(  const mclx* mx
)
   {  dim N = N_COLS(mx), i
   ;  dim n_offending = 0, n_ok = 0
   ;  double max_diff = 0.0, total_diff = 0.0
   ;  double min_skew = -DBL_MAX

   ;  for (i=0;i<N;i++)
      {  mclv* va = mx->cols+i, *vb = NULL
      ;  pnum a = va->vid
      ;  dim j
      ;  for (j=0;j<va->n_ivps && va->ivps[j].idx < a;j++)
         {  pnum b = va->ivps[j].idx
         ;  double distab = va->ivps[j].val
         ;  mclp* ia = NULL
         ;  dim k
         ;  vb = mclxGetVector(mx, b, RETURN_ON_FAIL, vb)
         ;  if (!vb)
            {  mcxErr(me, "strange miss for %ld\n", (long) b)
            ;  continue
         ;  }
            for (k=0;k<vb->n_ivps;k++)
            {  pnum c = vb->ivps[k].idx
            ;  double distbc = vb->ivps[k].val
            ;  ia = mclvGetIvp(va, c, ia)
            ;  if (ia)
               {  double distac = ia->val
               ;  double diff = distac - distab - distbc
               ;  if (diff > 0)
                  {  n_offending++
                  ;  total_diff += diff
                  ;  if (max_diff < diff)
                        max_diff = diff
                     ,  mcxErr
                        (  me
                        ,  "a=%ld b=%ld c=%ld ab=%.6g bc=%.6g ac=%.6g diff=%.6g"
                        ,  (long) a, (long) b, (long) c
                        ,  distab,  distbc,  distac
                        ,  diff
                        )
               ;  }
                  else
                  {  n_ok++
                  ;  if (diff > min_skew)
                     min_skew = diff
               ;  }
               }
            }
         }
      }
      fprintf
      (  stdout
      ,  "fail=%lu ok=%lu frac=%.6f maxdiff=%.6f totaldiff=%.6f minskew=%.6f\n"
      ,  (ulong) n_offending
      ,  (ulong) n_ok
      ,  (double) ((1.0 * n_offending) / (1.0 * n_offending + n_ok))
      ,  max_diff
      ,  total_diff / (N ? N * 0.5 * N : 1.0)
      ,  -min_skew
      )
   ;  return n_offending ? 11 : 0
;  }


int do_values
(  const mclx* mx
,  unsigned mode_dispatch
)
   {  unsigned long noe, n_allvals, i, j
   ;  noe = mclxNrofEntries(mx)
   ;  if (mode_dispatch == DISPATCH_VALUES_SORTED || mode_dispatch == DISPATCH_VALUES_HIST)
      {  double sum_vals = 0.0
      ;  pval*  allvals = mcxAlloc(noe * sizeof allvals[0], EXIT_ON_FAIL)
      ;  n_allvals = get_n_sort_allvals(mx, allvals, noe, &sum_vals, TRUE) /* ascending */
      ;  if (mode_dispatch == DISPATCH_VALUES_SORTED)
         {  for (i=0;i<n_allvals;i++)
            fprintf(xfout_g->fp, "%g\n", (double) allvals[i])
      ;  }
         else if (mode_dispatch == DISPATCH_VALUES_HIST)
         {  ulong ofs = 0, cur = 0

         ;  if (!VT.start && !VT.end && VT.nbins)
            {  double start =  n_allvals ? allvals[0] : 0.0 
            ;  double end   =  n_allvals ? allvals[n_allvals-1] : 1.0
            ;  VT.start = start
            ;  VT.end   = end
         ;  }
            for (i=1;i<=VT.nbins;i++)
            {  double cutoff = VT.start + (VT.end - VT.start) * i * 1.0 / VT.nbins
            ;  if (i == VT.nbins)
               cutoff = VT.end      /* above could have rounding error issues */
            ;  while (cur < n_allvals && allvals[cur] <= cutoff)
               cur++
            ;  fprintf(xfout_g->fp, "%g\t%lu\n", cutoff, (ulong) (cur - ofs))
            ;  ofs = cur
         ;  }
         }
   ;  }
      else
      {  for (i=0;i<N_COLS(mx);i++)
         {  mclv* v = mx->cols+i
         ;  for (j=0;j<v->n_ivps;j++)
            fprintf(xfout_g->fp, "%g\n", (double) v->ivps[j].val)
      ;  }
      }
      return 0
;  }


int do_degrees_hist
(  const mclx* mx
)
   {  long step
   ;  ulong ofs = 0, cur = 0
   ;  mclv* dg

   ;  dg = mclxColSizes(mx, MCL_VECTOR_COMPLETE)
   ;  mclvSortAscVal(dg)

   ;  for (step = 0; ofs < dg->n_ivps; step += VS.stepi)
      {  double cutoff = step * 1.0 + 0.5
      ;  while (cur < dg->n_ivps && dg->ivps[cur].val <= cutoff)
         cur++
      ;  fprintf(xfout_g->fp, "%g\t%lu\n", (double) ((int) cutoff), (ulong) (cur - ofs))
      ;  ofs = cur
   ;  }
      return 0
;  }


int do_size
(  mcxIO* xf
)
   {  const char* fmt
   ;  unsigned int format

   ;  mclx* mx = mclxReadSkeleton(xf, NO_BITS_SET, FALSE)
   ;  if (!mx)
      mcxDie(1, me, "reading %s failed", xf->fn->str)

   ;  format = mclxIOformat(xf)

   ;  fmt = format == 'b' ? "binary" : format == 'a' ? "interchange" : "?"
   ;  fprintf
      (  xfout_g->fp
      ,  "%s format,  row x col dimensions are %ld x %ld\n"
      ,  fmt
      ,  (long) N_ROWS(mx)
      ,  (long) N_COLS(mx)
      )

   ;  if (format == 'b')
      {  int szl =  sizeof(long)
      ;  dim* oa 
      ;  oa = mcxAlloc((1+N_COLS(mx))*szl, EXIT_ON_FAIL)
      ;  if ((1+N_COLS(mx)) != fread(oa, szl, 1+N_COLS(mx), xf->fp))
         mcxDie(1, me, "reading %s failed (offsets)", xf->fn->str)
      ;  fprintf
         (  xfout_g->fp
         ,  "%ld entries %ld rows %ld columns %s format\n"
         ,  (long) (oa[N_COLS(mx)] - N_COLS(mx) * (2 * szl + sizeof(double)))/sizeof(mclp)
         ,  (long) N_ROWS(mx)
         ,  (long) N_COLS(mx)
         ,  "binary"
         )
   ;  }

      return 0
;  }


int do_dimension
(  mcxIO* xf
)
   {  const char* fmt
   ;  unsigned int format
   ;  long n_cols, n_rows

   ;  if (mclxReadDimensions(xf, &n_cols, &n_rows))
      mcxDie(1, me, "reading %s failed", xf->fn->str)
   ;  format = mclxIOformat(xf)
   ;  mcxIOclose(xf)

   ;  fmt = format == 'b' ? "binary" : format == 'a' ? "interchange" : "?"
   ;  fprintf
      (  xfout_g->fp
      ,  "%s format,  row x col dimensions are %ld x %ld\n"
      ,  fmt
      ,  n_rows
      ,  n_cols
      )
   ;  return 0
;  }


static mcxstatus qMain
(  int          argc_unused      cpl__unused
,  const char*  argv_unused[]    cpl__unused
)
   {  mclx* cl = NULL, *cltp = NULL

   ;  srandom(mcxSeed(135313531))         /* for tf arguments, #tug, #rug */

   ;  mcxIOopen(xfout_g, EXIT_ON_FAIL)
   ;  levels = mcxAlloc(1001 * sizeof levels[0], EXIT_ON_FAIL)

   ;  if (mode_vary == VARY_UNSPECIFIED && mode_dispatch == DISPATCH_UNSPECIFIED)
      mode_dispatch = DISPATCH_NODEATTR

   ;  if
      (  transform_spec
      && !(transform = mclgTFparse(NULL, transform_spec))
      )
      mcxDie(1, me, "input -tf spec does not parse")

   ;  if (xftab_g && mode_dispatch != DISPATCH_NODEATTR)    /* weird, docme */
      tab_g = mclTabRead(xftab_g, NULL, EXIT_ON_FAIL)

   ;  if (xfcl_g)
      {  cl = mclxRead(xfcl_g, EXIT_ON_FAIL)
      ;  cltp = mclxTranspose(cl)
      ;  mcxIOclose(xfcl_g)
      ;  if (mode_dispatch == DISPATCH_NODEATTR && !user_imx)
         return do_attr_clsonly(cl, cltp)
   ;  }

      if (mode_vary != VARY_UNSPECIFIED)
      {  mclx* mx
         =  mcx_get_graph("mcx query", xfmx_g, xfabc_g, xftab_g, &tab_g, transform, MCL_READX_REMOVE_LOOPS)
      ;  do_vary_threshold(mx, xfout_g->fp, mode_vary)
   ;  }

      else if (mode_dispatch != DISPATCH_UNSPECIFIED)
      {  if (mode_dispatch == DISPATCH_DIMENSION)
         return do_dimension(xfmx_g)

      ;  else if (mode_dispatch == DISPATCH_SIZE)
         return do_size(xfmx_g)

      ;  else if (mode_dispatch == DISPATCH_TESTMETRIC)
         {  mclx* mx
            =  mcx_get_graph("mcx query", xfmx_g, xfabc_g, xftab_g, &tab_g, transform, MCL_READX_REMOVE_LOOPS)
         ;  return do_testmetric(mx)
      ;  }
                                                            /* fixme could be useful for things that are not graphs */
         else if (mode_dispatch == DISPATCH_DEGREES_HIST)
         {  mclx* mx
            =  mcx_get_graph("mcx query", xfmx_g, xfabc_g, xftab_g, &tab_g, transform, MCL_READX_REMOVE_LOOPS)
         ;  return do_degrees_hist(mx)
      ;  }
         else if (mode_dispatch == DISPATCH_NODEATTR)
         {  mclx* mx
            =  mcx_get_graph("mcx query", xfmx_g, xfabc_g, xftab_g, &tab_g, transform, MCL_READX_REMOVE_LOOPS)
         ;  return do_attr(mx, cl, cltp)
      ;  }
         else if (mode_dispatch == DISPATCH_TESTCYCLE)
         {  mclx* mx
            =  mcx_get_graph
               (  "mcx query", xfmx_g, xfabc_g, xftab_g, &tab_g, transform,
                  MCL_READX_REMOVE_LOOPS | MCLX_REQUIRE_GRAPH | MCLX_REQUIRE_CANONICAL )
         ;  return test_cycle(mx, n_limit)
      ;  }
         else if
         (  mode_dispatch == DISPATCH_VALUES_SORTED
         || mode_dispatch == DISPATCH_VALUES
         || mode_dispatch == DISPATCH_VALUES_HIST
         )
         {  mclx* mx
            =  mcx_get_graph("mcx query", xfmx_g, xfabc_g, xftab_g, &tab_g, transform, MCL_READX_REMOVE_LOOPS)
         ;  return do_values(mx, mode_dispatch)
      ;  }
      }

      mcxIOclose(xfout_g)
   ;  mcxIOfree(&xfout_g)
   ;  mcxIOfree(&xfmx_g)
   ;  mcxFree(levels)
   ;  return 0
;  }


mcxDispHook* mcxDispHookquery
(  void
)
   {  static mcxDispHook qEntry
   =  {  "query"
      ,  "query [options]"
      ,  qOptions
      ,  sizeof(qOptions)/sizeof(mcxOptAnchor) - 1

      ,  qArgHandle
      ,  qInit
      ,  qMain

      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &qEntry
;  }


/*
-------------------------------------------------------------------------------
  L   D   R   S     cce  EWmean   EWmed  EWiqr NDmean  NDmed  NDiqr      Cutoff 
-------------------------------------------------------------------------------
100   0   0   0   11142     140      69     11 1444.3   1034   2076        0.20
100   0   0   0   11142     210      69     11 1444.3   1034   2076        0.25
100   0   0   0   11142     280      69     11 1444.3   1034   2076        0.30
*/

