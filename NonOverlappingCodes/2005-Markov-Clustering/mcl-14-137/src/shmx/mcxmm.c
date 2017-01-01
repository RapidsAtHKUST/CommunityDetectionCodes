/*   (C) Copyright 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
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
#include "util/minmax.h"
#include "util/ding.h"
#include "util/ting.h"
#include "util/io.h"
#include "util/err.h"
#include "util/equate.h"
#include "util/rand.h"
#include "util/opt.h"

#include "mcl/proc.h"
#include "mcl/procinit.h"
#include "mcl/alg.h"

#include "clew/clm.h"
#include "clew/cat.h"

const char* usagelines[];

const char* me = "mcxmm";
const char* syntax = "Usage: mcxmm { -imx -dag -child-diff-lq -parent-diff-gq }";

void usage
(  const char**
)  ;


enum
{  MY_OPT_IMX
,  MY_OPT_TAB
,  MY_OPT_QUERY
,  MY_OPT_TEST_CYCLE
,  MY_OPT_TEST_CROSS
,  MY_OPT_DAG_ATTR
,  MY_OPT_DAG_REDUCE
,  MY_OPT_DAG_DIFF
,  MY_OPT_PARENT_DIFF_GQ
,  MY_OPT_CHILD_DIFF_LQ
,  MY_OPT_VERSION
,  MY_OPT_HELP
}  ;


mcxOptAnchor options[]
=
{
   {  "-h"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "print this help"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<mx-fname>"
   ,  "input matrix to be converted to dag"
   }
,  {  "-dag-attr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DAG_ATTR
   ,  "<fname>"
   ,  "file to write dag attributes to"
   }
,  {  "--test-cycle"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TEST_CYCLE
   ,  NULL
   ,  "test whether input has cycles"
   }
,  {  "--test-xmin"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TEST_CROSS
   ,  NULL
   ,  "test whether input satisfies cross minimum criterion"
   }
,  {  "-dag-reduce"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DAG_REDUCE
   ,  "<fname>"
   ,  "file to write reduced dag to"
   }
,  {  "-dag-diff"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DAG_DIFF
   ,  "<fname>"
   ,  "create dag from subset-diff matrix"
   }
,  {  "-q"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_QUERY
   ,  "mcl node id"
   ,  "option of unknown function"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<tab-fname>"
   ,  "HMM tab file"
   }
,  {  "-child-diff-lq"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CHILD_DIFF_LQ
   ,  "<frac>"
   ,  "smaller HMM should have at most <frac> of its nodes not in larger"
   }
,  {  "-parent-diff-gq"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PARENT_DIFF_GQ
   ,  "<frac>"
   ,  "larger HMM should have at least <frac> of its nodes not in smaller"
   }
,  {  "--help"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "print this help"
   }
,  {  "--version"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_VERSION
   ,  NULL
   ,  "print version information"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


/* TODO
   starting nodes might be generally nodes that have hight out/in ratio.
   For nodes with high out/in ratio the 'in' part is suspect.
*/


mcxstatus fire_node_next
(  const mclx* mx
,  mclv* seen
,  mclv *todo
,  dim start
)
   {  mclv* next = mclvInit(NULL)
   ;  dim i
   ;  mcxstatus s = STATUS_OK
;if(0)fprintf(stderr, "\tnext layer has %d nodes\n", (int) todo->n_ivps)
   ;  for (i=0; i<todo->n_ivps;i++)
      {  mclv* ls = mclxGetVector(mx, todo->ivps[i].idx, RETURN_ON_FAIL, NULL)
      ;  if (ls)
         {  mcldMerge(next, ls, next)
         ;  if (mclvGetIvp(ls, start, NULL))
            {  s = STATUS_FAIL
            ;  break
         ;  }
         }
      }
      mcldMerge(seen, todo, seen)      /* add todo to seen */
   ;  mcldMinus(next, seen, next)      /* remove seen from next */
   ;  mclvCopy(todo, next)             /* copy next to todo */
   ;  mclvFree(&next)
   ;  return s
;  }


ofs fire_node
(  const mclx* mx
,  dim i
,  mclv** seenpp
)
   {  mclv* v = mx->cols+i
   ;  mclv* seen = mclvInsertIdx(NULL, v->vid, 1.0)
   ;  mclv* todo = mclvClone(v)
   ;  mcxstatus s = STATUS_OK
   ;  dim level = 0
;if(0)fprintf(stderr, "node %d\n", (int) i)
   ;  while(todo->n_ivps && !s)
         s = fire_node_next(mx, seen, todo, i)
      ,  level++
   ;  mclvFree(&todo)

   ;  if (seenpp)
      seenpp[0] = seen
   ;  else
      mclvFree(&seen)

   ;  return s ? -1 : level
;  }


void test_cross_ratio
(  mclx* mx
)
   {  dim i, j, n = 0
   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  double selfv = mclvSelf(v)
      ;  for (j=0;j<v->n_ivps;j++)
         {  mclv* w = mclxGetVector(mx, v->ivps[j].idx, EXIT_ON_FAIL, NULL)
         ;  double arc  =  v->ivps[j].val
         ;  double selfw=  mclvSelf(w)
         ;  double cra  =  mclvIdxVal(w, v->vid, NULL)
         ;  double s    =  MCX_MIN(selfv, selfw)
         ;  if (s > arc || s > cra)
            fprintf
            (  stdout
            ,  "%u\t%u\t%g\t%g\t%g\t%g\n"
            ,  (unsigned) v->vid
            ,  (unsigned) w->vid
            ,  arc
            ,  cra
            ,  selfv
            ,  selfw
            )
         ;  n++
      ;  }
      }
      fprintf(stderr, "tested %u entries\n", (unsigned) n)
;  }


void test_for_cycles
(  mclx* mx
)
   {  mclx* tp = mclxTranspose(mx)
   ;  mclv* fwd = mclxColSizes(mx, MCL_VECTOR_COMPLETE)
   ;  mclv* bwd = mclxColSizes(tp, MCL_VECTOR_COMPLETE)
   ;  dim i, n_cycle = 0

   ;  for (i=0;i<bwd->n_ivps;i++)
      {  ofs level_up = fire_node(mx, i, NULL)
      ;  ofs level_dn = fire_node(tp, i, NULL)
      ;  if (level_up < 0 || level_dn < 0)
            fprintf(stderr, " [%lu cycle]", (ulong) i)
         ,  n_cycle++
   ;  }

      if (n_cycle)
      fputc('\n', stderr)
   ;  mclvFree(&bwd)
   ;  mclvFree(&fwd)
   ;  mclxFree(&tp)
   ;  fprintf
      (  stderr
      ,  "file with %lu edges has %d cycles\n"
      ,  (ulong) mclxNrofEntries(mx)
      ,  (int) n_cycle
      )
   ;  exit(n_cycle ? 1 : 0)
;  }


void get_attr
(  mclx* mx
,  mclTab* tab
,  mcxIO* xfattr
)
   {  mclx* tp = mclxTranspose(mx)
   ;  mclx* G  = mclxAdd(mx, tp)
   ;  mclv* fwd = mclxColSizes(mx, MCL_VECTOR_COMPLETE)
   ;  mclv* bwd = mclxColSizes(tp, MCL_VECTOR_COMPLETE)
   ;  mclx* cc = clmComponents(G, NULL)
   ;  mclx* node2cc = mclxTranspose(cc)
   ;  dim i, n_cycle = 0

   ;  fprintf(xfattr->fp, "node\tup\tdown\tnparent\tnchild\tndag\n")

   ;  for (i=0;i<bwd->n_ivps;i++)
      {  mclv* seenpp1 = NULL, *seenpp2 = NULL

      ;  ofs level_up = fire_node(mx, i, &seenpp1)
      ;  ofs level_dn = fire_node(tp, i, &seenpp2)
      ;  ofs ccidx = node2cc->cols[i].ivps[0].idx
      ;  dim ccsize = cc->cols[ccidx].n_ivps

      ;  mclvFree(&seenpp1)
      ;  mclvFree(&seenpp2)

      ;  if ((i+1) % 500 == 0)
         fputc('.', stderr)
      ;  if (tab)
         {  const char* label = mclTabGet(tab, mx->cols[i].vid, NULL)
         ;  fputs(label, xfattr->fp)
         ;  fputc('\t', xfattr->fp)
      ;  }
         else
         fprintf
         (  xfattr->fp
         ,  "%lu\t"
         ,  (ulong) mx->cols[i].vid
         )

      ;  fprintf
         (  xfattr->fp
         ,  "%ld\t%ld\t%lu\t%lu\t%lu\n"
         ,  (long) level_up
         ,  (long) level_dn
         ,  (ulong) fwd->ivps[i].val
         ,  (ulong) bwd->ivps[i].val
         ,  (ulong) ccsize
         )

      ;  if (level_up < 0 || level_dn < 0)
            fputc('.', stderr)
         ,  n_cycle++
   ;  }

      if (n_cycle)
      fputc('\n', stderr)
   ;  mclvFree(&bwd)
   ;  mclvFree(&fwd)
   ;  mclxFree(&tp)
;  }


#if 0
void dump_label
(  mclTab* tab
,  dim vid
,  int level
)
   {  const char* label = mclTabGet(tab, vid, NULL)
   ;  dim i
   ;  for (i=0;i<level;i++)
      fputs("   ", stdout)
   ;  fprintf("|  %s\n", label ? label : "????")
;  }


/* TODO: lots of duplication in tree-walking DAG output.
   perhaps measure first (how many steps does it take)?
*/

void walk_dag
(  mclx* mx
,  mclv* v
,  int level
)
   {  dim i
   ;  v->val = 2.0
   ;  for (i=0;i<v->n_ivps;i++)
      {  mclv* v = mclxGetVector(mx, v->ivps[i].idx, EXIT_ON_FAIL, NULL)
      ;  v->val = 2.0
      ;  dump_label(tab, v, level)
      ;  
      }
;  }


/* TODO: hmm.content.subset2 vid values can denote #proteins hitting */

void dump_dag
(  mclx* mx
,  mclTab* tab
)
   {  mclx* mx = mclxTranspose(mx)
   ;  dim i
   ;  for (i=0;i<N_COLS(mx);i++)
      {  mclv* v = mx->cols+i
      ;  if (v->val < 1.5)
         {  dump_label(tab, v->vid, 0)
         ;  walk_dag(mx, v, 1)
      ;  }
      }
   }

#endif

void dag_diff_select 
(  mclx* mx
,  mclTab* tab
,  mcxIO* xfdiff
,  double child_diff_lq
,  double parent_diff_gq
)
   {  dim i
   ;  mclx* dag = mclxAllocClone(mx)
   ;  for (i=0;i<N_COLS(mx); i++)
      {  mclv* v = mx->cols+i
      ;  dim j
      ;  for (j=0;j<v->n_ivps;j++)
         {  dim idx     = v->ivps[j].idx
         ;  double valv = v->ivps[j].val
         ;  mclv* t = mclxGetVector(mx, idx, EXIT_ON_FAIL, NULL)
         ;  mclp* p = mclvGetIvp(t, v->vid, NULL)
         ;  double valt = p ? p->val : 0.0
         ;  double delta = valv - valt
         ;  double lg = valv, sm = valt
         ;  double child_diff, parent_diff
         ;  int v_is_child = 0

         ;  if (delta < 0)
               delta = -delta
            ,  lg=valt, sm=valv
            ,  v_is_child = 1

         ;  child_diff = sm
         ;  parent_diff = lg

;if(0 && i==111)
fprintf(stderr, "nb %d delta %g\n", (int) idx, delta)
         ;  if (child_diff > child_diff_lq || parent_diff < parent_diff_gq)
            NOTHING
         ;  else
            {  if (v_is_child)
               mclvInsertIdx(dag->cols+i, idx, delta)
            ;  else
               mclvInsertIdx(dag->cols+(t-mx->cols), v->vid, delta)
         ;  }
         }
      }
   ;  mclxWrite(dag, xfdiff, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
   ;  mclxFree(&dag)
;  }


int main
(  int                  argc
,  const char*          argv[]
)  
   {  mcxIO* xfdagreduce = NULL, *xfattr = NULL, *xfdiff = NULL
   ;  double child_diff_lq = 0.2
   ;  double parent_diff_gq = 0.4
   ;  mcxIO* xfimx = mcxIOnew("-", "r"), *xfdag = NULL, *xftab = NULL
   ;  mclTab* tab = NULL
   ;  int q = -1
   ;  mclx* mx
   ;  unsigned char test_mode = 0

   ;  mcxstatus parseStatus = STATUS_OK
   ;  mcxOption* opts, *opt
   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)
   
   ;  if
      (!(opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)))
      exit(0)

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
   ;  mclx_app_init(stderr)


   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch

      ;  switch(anch->id)
         {  case MY_OPT_HELP
         :  mcxOptApropos(stdout, me, syntax, 0, 0, options)
         ;  return 0
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  return 0
         ;

            case MY_OPT_TEST_CYCLE
         :  test_mode = 'c'
         ;  break
         ;

            case MY_OPT_TEST_CROSS
         :  test_mode = 'x'
         ;  break
         ;

            case MY_OPT_DAG_ATTR
         :  xfattr = mcxIOnew(opt->val, "w")
         ;  mcxIOopen(xfattr, EXIT_ON_FAIL)
         ;  break
         ;

            case MY_OPT_DAG_DIFF
         :  xfdiff = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_DAG_REDUCE
         :  xfdagreduce = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_CHILD_DIFF_LQ
         :  child_diff_lq = atof(opt->val)
         ;  break
         ;

            case MY_OPT_PARENT_DIFF_GQ
         :  parent_diff_gq = atof(opt->val)
         ;  break
         ;

            case MY_OPT_QUERY
         :  q = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_TAB
         :  xftab = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_IMX
         :  mcxIOnewName(xfimx, opt->val)
         ;  break
         ;
         }
      }

   ;  if (xfimx)
      mx = mclxRead(xfimx, EXIT_ON_FAIL)
   ;  else
      mcxDie(1, me, "need -imx")

   ;  if (xftab)
      tab = mclTabRead(xftab, mx->dom_cols, EXIT_ON_FAIL)

   ;  if (test_mode == 'c')
      test_for_cycles(mx)

   ;  else if (test_mode == 'x')
      test_cross_ratio(mx)

   ;  else if (xfattr)
      get_attr(mx, tab, xfattr)

   ;  else if (xfdagreduce)
      {  mclxComposeHelper* ch = mclxComposePrepare(mx, mx, 0)
      ;  dim i
      ;  for (i=0;i<N_COLS(mx);i++)
         {  mclv* in = mx->cols+i
         ;  mclv* out = mclxVectorCompose(mx, in, NULL, mclxComposeThreadData(ch, 0))
         ;  mcldMinus(in, out, in)
         ;  mclvFree(&out)
      ;  }
         mclxWrite(mx, xfdagreduce, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
      ;  mclxComposeRelease(&ch)
   ;  }

      else if (xfdiff)
      dag_diff_select(mx, tab, xfdiff, child_diff_lq, parent_diff_gq)

   ;  mclxFree(&mx)
   ;  mcxIOfree(&xfimx)
   ;  mcxIOfree(&xfdag)
   ;  mcxIOfree(&xfattr)
   ;  mcxIOfree(&xfdagreduce)
   ;  return 0
;  }


const char* usagelines[] =
{  NULL
}  ;


