/*   (C) Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007 Stijn van Dongen
 *   (C) Copyright 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* TODO
 *    Perhaps pre-read clusterings in one go, both for stack and
 *    herd (the default) cases.
 *
 *    create a sane output format.
 *    Either s-expression based or line based.
*/


#include <string.h>
#include <stdio.h>

#include "clm.h"
#include "report.h"
#include "clminfo.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/app.h"
#include "impala/iface.h"
#include "impala/compose.h"

#include "clew/claw.h"
#include "clew/scan.h"
#include "clew/clm.h"
#include "clew/cat.h"

#include "mcl/interpret.h"
#include "mcl/transform.h"

#include "util/io.h"
#include "util/err.h"
#include "util/types.h"
#include "util/opt.h"
#include "util/minmax.h"

static const char* me  =  "clminfo";


enum
{  MY_OPT_OUTPUT = CLM_DISP_UNUSED
,  MY_OPT_CLTREE
,  MY_OPT_CLCEIL
,  MY_OPT_NCLMAX
,  MY_OPT_ADAPT
,  MY_OPT_PI
,  MY_OPT_TF
,  MY_OPT_PERNODE
,  MY_OPT_PERPAIR
,  MY_OPT_LAX
}  ;


static mcxOptAnchor infoOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "--node-self-measures"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_PERNODE
   ,  NULL
   ,  "dump node-wise criteria for native cluster"
   }
,  {  "--node-all-measures"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_PERPAIR
   ,  NULL
   ,  "dump node-wise criteria for all incident clusters"
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
,  {  "-cl-ceil"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CLCEIL
   ,  "<num>"
   ,  "skip clusters with <num> or more members (with --cl-stack)"
   }
,  {  "-cat-max"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NCLMAX
   ,  "<num>"
   ,  "do at most <num> levels of the hierarchy"
   }
,  {  "--lax"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_LAX
   ,  NULL
   ,  "allow arbitrary sets to be read in, drop partition requirement"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout        =  (void*) -1;
static mcxIO *xfmx          =  (void*) -1;
static mcxIO* xfstack       =  (void*) -1;
static mclx *mx             =  (void*) -1;
static dim     clceil       =  -1;
static dim     n_cl_max     =  -1;
static double inflation     =  -1;
static mcxbool perpair      =  -1;
static mcxbool pernode      =  -1;
static mcxbool cone         =  -1;
static mcxbool lax          =  -1;
static mcxTing* tfting      = (void*) -1;


static mcxstatus infoInit
(  void
)
   {  xfout          =  mcxIOnew("-", "w")
   ;  xfmx           =  NULL
   ;  xfstack        =  NULL
   ;  mx             =  NULL
   ;  clceil         =  0
   ;  n_cl_max       =  0
   ;  inflation      =  0.0
   ;  tfting         =  NULL
   ;  perpair        =  FALSE
   ;  pernode        =  FALSE
   ;  cone           =  FALSE
   ;  lax            =  FALSE
   ;  return STATUS_OK
;  }


static mcxstatus infoArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOrenew(xfout, val, "w")
      ;  break
      ;

         case MY_OPT_NCLMAX
      :  n_cl_max = atoi(val)
      ;  break
      ;

#if 0
               /* this leads to crashes; clew/claw.c requires nodes
                * to be in a cluster.
               */
         case MY_OPT_LAX
      :  lax = TRUE
      ;  break
      ;
#endif

         case MY_OPT_CLCEIL
      :  clceil = atoi(val)
      ;  break
      ;

         case MY_OPT_CLTREE
      :  xfstack = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_PERPAIR
      :  perpair = TRUE
      ;  break
      ;

         case MY_OPT_TF
      :  tfting = mcxTingNew(val)
      ;  break
      ;

         case MY_OPT_PERNODE
      :  pernode = TRUE
      ;  break
      ;

         case MY_OPT_PI
      :  inflation = atof (val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


void do_stack
(  mclx* mx
,  mcxIO* xfstack
,  mcxIO* xfout
,  dim clceil
,  dim n_cl_max
)
   {  mclxCat st
   ;  dim i
   ;  mcxstatus status

   ;  mclxCatInit(&st)

   ;  status = mclxCatRead
         (  xfstack, &st, n_cl_max, NULL, mx->dom_rows
         ,  MCLX_PRODUCE_DOMSTACK | MCLX_ENSURE_ROOT | MCLX_REQUIRE_PARTITION
         )

   ;  if (status)
      mcxDie(1, me, "error reading stack")
   ;  else if (!st.level)
      mcxExit(0)

   ;  for (i=1;i<st.n_level;i++)
      {  mclx* cl = st.level[i].mx
      ;  mclx* clchild = st.level[i-1].mx

      ;  clmXPerformance(mx, clchild, cl, xfout, clceil)
      ;  fprintf(xfout->fp, "===\n")
   ;  }
      clmXPerformance(mx, st.level[st.n_level-1].mx, NULL, xfout, clceil)
;  }



static mcxstatus infoMain
(  int                  argc
,  const char*          argv[]
)
   {  int a =  0
   ;  mcxTing* ginfo = mcxTingEmpty(NULL, 40)

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_GAUGE | MCX_LOG_WARN

   ;  if(1)
      mclx_app_init(stderr)

   ;  if(1)
      mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)

   ;  if
      (  (xfstack && 1 != argc)
      || (!xfstack && 1 >= argc)
      )
      mcxDie
      (  1
      ,  me
      ,  "need %smatrix file%s"
      ,  xfstack ? "(just a) " : ""
      ,  xfstack ? "" : " and cluster file(s)"
      )

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  xfmx  =  mcxIOnew(argv[a++], "r")
   ;  mx    =  mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)

   ;  if (tfting)
      {  mclgTF* tfar = mclgTFparse(NULL, tfting)
      ;  if (!tfar)
         mcxDie(1, me, "errors in tf-spec")
      ;  mclgTFexec(mx, tfar)
   ;  }

      if (inflation)
      {  mclxInflate(mx, inflation)
      ;  mcxTingPrintAfter(ginfo, "inflation=%.2f\n", (double) inflation)
   ;  }

      mclxAdjustLoops(mx, mclxLoopCBmax, NULL)

   ;  if (xfstack)
         do_stack(mx, xfstack, xfout, clceil, n_cl_max)
      ,  mcxExit(0)

   ;  mcxIOfree(&xfmx)

   ;  while(a < argc)
      {  mcxIO *xfcl
      ;  mclx *cl
      ;  dim j
      ;  mclxCat st
      ;  mcxstatus status

      ;  if (!strcmp(argv[a], "--"))
         {  a++
         ;  fputc('\n', xfout->fp)
         ;  continue
      ;  }

         xfcl =  mcxIOnew(argv[a], "r")

      ;  mclxCatInit(&st)

      ;  if
         (( status
         =  mclxCatRead
            (  xfcl, &st, n_cl_max, NULL, mx->dom_rows
            ,  MCLX_PRODUCE_DOMSTACK | (lax ? 0 : MCLX_REQUIRE_PARTITION)
            )
         ))
         mcxDie(1, me, "error reading stack")

      ;  for (j=0;j<st.n_level;j++)
         {  cl = st.level[j].mx

         ;  if (pernode)
            clmDumpNodeScores(xfcl->fn->str, mx, cl, CLM_NODE_SELF)

         ;  else if (perpair)
            clmDumpNodeScores(xfcl->fn->str, mx, cl, CLM_NODE_INCIDENT)

         ;  else
            {  clmGranularityTable tbl
            ;  clmPerformanceTable pftable
            ;  mcxTing* linfo = mcxTingNew(ginfo->str)
            ;  mcxTingPrintAfter(linfo, " source=%s", xfcl->fn->str)
            ;  if (st.n_level > 1)
               mcxTingPrintAfter(linfo, ":%03d", (int) (j+1))

            ;  clmPerformance(mx, cl, &pftable)
            ;  clmPerformancePrint(xfout->fp, linfo->str, &pftable)

            ;  fputc(' ', xfout->fp)
            ;  clmGranularity(cl, &tbl)
            ;  clmGranularityPrint(xfout->fp, NULL, &tbl)
            ;  fputc('\n', xfout->fp)
         ;  }
            if (a < argc-1 || j <st.n_level-1)
            fprintf(xfout->fp, "===\n")
      ;  }

         mcxIOfree(&xfcl)
      ;  /* todo: free stack */
      ;  a++
   ;  }

      mclxFree(&mx)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookInfo
(  void
)
   {  static mcxDispHook infoEntry
   =  {  "info"
      ,  "info [options] <mx file> <cl file>+"
      ,  infoOptions
      ,  sizeof(infoOptions)/sizeof(mcxOptAnchor) - 1
      ,  infoArgHandle
      ,  infoInit
      ,  infoMain
      ,  1
      ,  -1
      ,  MCX_DISP_MANUAL
      }
   ;  return &infoEntry
;  }


