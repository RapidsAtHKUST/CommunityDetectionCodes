/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* NOTE:
 * with groups --list should be implied ...
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

#include "clm.h"

#include "util/types.h"
#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/compile.h"

#include "impala/io.h"
#include "impala/matrix.h"
#include "impala/tab.h"
#include "impala/stream.h"
#include "impala/app.h"

#include "mcl/interpret.h"
#include "mcl/transform.h"

#include "clew/claw.h"
#include "clew/scan.h"
#include "clew/clm.h"
#include "clew/cat.h"

#include "gryphon/path.h"


enum
{  MY_OPT_OUTPUT = CLM_DISP_UNUSED
,  MY_OPT_LIST
,  MY_OPT_PI
,  MY_OPT_TF
,  MY_OPT_THREAD
,  MY_OPT_G
,  MY_OPT_littleG
,  MY_OPT_CLTREE
,  MY_OPT_CLCEIL
,  MY_OPT_NCLMAX
}  ;

static const char* me  =  "clminfo";

static mcxOptAnchor info2Options[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "--list"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LIST
   ,  NULL
   ,  "list efficiency for all nodes"
   }
,  {  "-t"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_THREAD
   ,  "<num>"
   ,  "number of threads to use"
   }
,  {  "-J"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_G
   ,  "<int>"
   ,  "number of compute jobs overall"
   }
,  {  "-j"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_littleG
   ,  "<int>"
   ,  "index of this compute job"
   }
,  {  "-pi"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PI
   ,  "<num>"
   ,  "apply inflation with parameter <num> beforehand"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TF
   ,  "<tf-spec>"
   ,  "first apply tf-spec to matrix"
   }
,  {  "-cl-tree"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CLTREE
   ,  "<fname>"
   ,  "assume mclcm-type hierarchical clusterings (stack format)"
   }
,  {  "-cat-max"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NCLMAX
   ,  "<num>"
   ,  "do at most <num> levels of the hierarchy"
   }
,  {  "-cl-ceil"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CLCEIL
   ,  "<num>"
   ,  "skip clusters with <num> or more members"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO* xfstack_g    =  (void*) -1;
static  dim progress_g     =  0;
static  dim n_thread_l     =  -1;
static dim n_group_G       =  -1;
static dim i_group_g       =  -1;
static const char* out_g   =  (void*) -1;

static dim     clceil_g    =  -1;
static double inflation_g  =  -1;
static mcxbool list_g      =  -1;
static mcxIO*  xfout_g     =  (void*) -1;
static mcxTing* tfting_g   =  (void*) -1;
static dim     n_cl_max_g  =  -1;


static mcxstatus info2Init
(  void
)
   {  progress_g  =  0
   ;  xfstack_g   =  NULL
   ;  out_g       =  "-"
   ;  n_thread_l  =  1
   ;  n_group_G   =  1
   ;  i_group_g     =  0
   ;  clceil_g    =  100000
   ;  inflation_g =  -1.0
   ;  tfting_g         =  NULL
   ;  list_g    =  FALSE
   ;  xfout_g     =  mcxIOnew("-", "w")
   ;  n_cl_max_g       =  0
   ;  return STATUS_OK
;  }


static mcxstatus info2ArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_CLTREE
      :  xfstack_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  mcxIOrenew(xfout_g, val, "w")
      ;  break
      ;

         case MY_OPT_THREAD
      :  n_thread_l = atoi(val)
      ;  break
      ;

         case MY_OPT_littleG
      :  i_group_g =  atoi(val)
      ;  break
      ;

         case MY_OPT_G
      :  n_group_G =  atoi(val)
      ;  break
      ;

         case MY_OPT_TF
      :  tfting_g = mcxTingNew(val)
      ;  break
      ;

         case MY_OPT_CLCEIL
      :  clceil_g = atoi(val)
      ;  break
      ;

         case MY_OPT_LIST
      :  list_g = TRUE
      ;  break
      ;

         case MY_OPT_NCLMAX
      :  n_cl_max_g = atoi(val)
      ;  break
      ;

         case MY_OPT_PI
      :  inflation_g = atof (val)
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
      }

   ;  return STATUS_OK
;  }



struct info_bundle
{  mclv* tabulator
;  mclx* cl
;  mclx* cltp
;  dim   clmaxsize
;
}  ;


double info_do
(  const mclx* mx
,  const mclv* cl
,  dim idx
)
   {  double eff, eff_max
   ;  clmVScore sc
   ;  clmVScanDomain(mx->cols+idx, cl, &sc)
   ;  clmVScoreCoverage(&sc, &eff, &eff_max)
   ;  return eff
;  }


#define INCLUDE_NODE(thecls, ib)  (thecls->n_ivps > 0 && ib.cl->cols[thecls->ivps[0].idx].n_ivps <= ib.clmaxsize)


static void info_dispatch
(  mclx* mx
,  dim nid
,  void* data
,  dim thread_id
)
   {  struct info_bundle* ib = data
   ;  const mclv* thecls = ib->cltp->cols+nid
   ;  if (INCLUDE_NODE(thecls, ib[0]))
      ib->tabulator->ivps[nid].val  = info_do(mx, ib->cl->cols+thecls->ivps[0].idx, nid)
   ;  else
      ib->tabulator->ivps[nid].val  = 0.0
;  }


static mcxstatus info2Main
(  int          argc
,  const char*  argv[]
)
   {  mclx* mx
   ;  struct info_bundle ib = { NULL, NULL }
   ;  int a =  0

   ;  mcxIO* xfmx = mcxIOnew(argv[a++], "r")
   ;  mx = mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)
   ;  mcxIOfree(&xfmx)
   ;  mclxAdjustLoops(mx, mclxLoopCBmax, NULL)
   ;  if (inflation_g >= 0)
      mclxInflate(mx, inflation_g)
   ;  if (tfting_g)
      {  mclgTF* tfar = mclgTFparse(NULL, tfting_g)
      ;  if (!tfar)
         mcxDie(1, me, "errors in tf-spec")
      ;  mclgTFexec(mx, tfar)
   ;  }

      ib.tabulator = mclvCopy(NULL, mx->dom_cols)

   ;  if (!clceil_g)
      clceil_g = N_COLS(mx)

   ;  mcxLogLevel = MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_GAUGE | MCX_LOG_WARN

   ;  mclx_app_init(stderr)
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)

   ;  mcxIOopen(xfout_g, EXIT_ON_FAIL)

   ;  n_thread_l = mclx_set_threads_or_die("clm info2", n_thread_l, i_group_g, n_group_G)

   ;  if (n_group_G > 1)
      list_g = TRUE

   ;  while(a < argc)
      {  mcxIO *xfcl
      ;  dim j
      ;  mclxCat st
      ;  mcxstatus status

      ;  if (!strcmp(argv[a], "--"))
         {  a++
         ;  fputc('\n', xfout_g->fp)
         ;  continue
      ;  }

         xfcl =  mcxIOnew(argv[a], "r")

      ;  mclxCatInit(&st)

      ;  if
         (( status
         =  mclxCatRead
            (  xfcl, &st, n_cl_max_g, NULL, mx->dom_rows
            ,  MCLX_PRODUCE_DOMSTACK | MCLX_REQUIRE_PARTITION
            )
         ))
         mcxDie(1, me, "error reading stack")

      ;  for (j=0;j<st.n_level;j++)
         {  ib.cl = st.level[j].mx
         ;  ib.cltp = mclxTranspose(ib.cl)
         ;  ib.clmaxsize = clceil_g
         ;  double sum = 0.0
         ;  dim n_included = 0
         ;  int i

         ;  mclvMakeConstant(ib.tabulator, 0.0)

         ;  if (n_thread_l * n_group_G > 1)
            mclxVectorDispatchGroup(mx, &ib, n_thread_l, info_dispatch, n_group_G, i_group_g, NULL)
         ;  else
            {  for (i=0;i<N_COLS(ib.cltp);i++)           /* fixme: this requires canonical domains */
               {  mclv* thecls = ib.cltp->cols+i
               ;  if (INCLUDE_NODE(thecls, ib))
                  ib.tabulator->ivps[i].val = info_do(mx, ib.cl->cols+thecls->ivps[0].idx, i)
            ;  }
            }

            for (i=0;i<N_COLS(ib.cltp);i++)
            {  double val = ib.tabulator->ivps[i].val
            ;  mclv* thecls = ib.cltp->cols+i
            ;  if (INCLUDE_NODE(thecls, ib))
               n_included++
            ;  if (list_g)
               {  if (i) fputc('\t', xfout_g->fp)
               ;  fprintf(xfout_g->fp, "%.4f", val)
            ;  }
               sum += val
         ;  }
            if (!list_g)
            fprintf(xfout_g->fp, "%.4f", n_included ? sum * 1.0 / n_included : 0.0)
         ;  fputc('\n', xfout_g->fp)
         ;  mclxFree(&ib.cltp)
      ;  }

         mcxIOfree(&xfcl)         /* todo: free stack */
      ;  a++
   ;  }

      mclxFree(&mx)
   ;  return STATUS_OK
;  }



mcxDispHook* mcxDispHookInfo2
(  void
)
   {  static mcxDispHook info2Entry
   =  {  "info2"
      ,  "info2 [options] <mx-file> <cl-file>+"
      ,  info2Options
      ,  sizeof(info2Options)/sizeof(mcxOptAnchor) - 1

      ,  info2ArgHandle
      ,  info2Init
      ,  info2Main

      ,  1
      ,  -1
      ,  MCX_DISP_DEFAULT
      }
   ;  return &info2Entry
;  }


