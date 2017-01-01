/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
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

#include "mcx.h"

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

#include "gryphon/path.h"


enum
{  MY_OPT_ABC = MCX_DISP_UNUSED
,  MY_OPT_IMX
,  MY_OPT_TAB
,  MY_OPT_OUT
,  MY_OPT_SUMMARY
,  MY_OPT_THREAD
,  MY_OPT_G
,  MY_OPT_littleG
,  MY_OPT_INCLUDE_ENDS
,  MY_OPT_TEST_LOOPS
}  ;


mcxOptAnchor clcfOptions[] =
{  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "specify input matrix"
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
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "write to file fname"
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
,  {  "--summary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SUMMARY
   ,  NULL
   ,  "return average for all nodes"
   }
,  {  "--test-loops"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_TEST_LOOPS
   ,  NULL
   ,  "test loop compensation code"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


static  dim progress_g  =  0;
static  mcxbool list_g  =  FALSE;
static  mcxbool test_loop_g = FALSE;
static  mcxIO* xfmx_g   =  (void*) -1;
static  mcxIO* xfabc_g  =  (void*) -1;
static  mcxIO* xftab_g  =  (void*) -1;
static  mclTab* tab_g   =  (void*) -1;
static  dim n_thread_l  =  -1;
static dim n_group_G    =  -1;
static dim i_group      =  -1;
static const char* out_g=  (void*) -1;


static mcxstatus clcfInit
(  void
)
   {  progress_g  =  0
   ;  list_g      =  TRUE
   ;  test_loop_g =  FALSE
   ;  xfmx_g      =  mcxIOnew("-", "r")
   ;  xfabc_g     =  NULL
   ;  out_g       =  "-"
   ;  xftab_g     =  NULL
   ;  tab_g       =  NULL
   ;  n_thread_l  =  1
   ;  n_group_G   =  1
   ;  i_group     =  0
   ;  return STATUS_OK
;  }


static mcxstatus clcfArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_IMX
      :  mcxIOnewName(xfmx_g, val)
      ;  break
      ;

         case MY_OPT_ABC
      :  xfabc_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_THREAD
      :  n_thread_l = atoi(val)
      ;  break
      ;

         case MY_OPT_littleG
      :  i_group =  atoi(val)
      ;  break
      ;

         case MY_OPT_G
      :  n_group_G =  atoi(val)
      ;  break
      ;

         case MY_OPT_OUT
      :  out_g = val
      ;  break
      ;

         case MY_OPT_TAB
      :  xftab_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_SUMMARY
      :  list_g = FALSE
      ;  break
      ;

         case MY_OPT_TEST_LOOPS
      :  test_loop_g = atoi(val) ? TRUE : FALSE
      ;  mcxTell("mcx clcf", "--test-loops kurrently kalltgestelt")
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
      }
   ;  return STATUS_OK
;  }



static void clcf_dispatch
(  mclx* mx
,  dim i
,  void* data
,  dim thread_id
)
   {  mclv* tabulator = data
   ;  tabulator->ivps[i].val  = mclnCLCF(mx, mx->cols+i, NULL)
;  }


static mcxstatus clcfMain
(  int          argc_unused      cpl__unused
,  const char*  argv_unused[]    cpl__unused
)
   {  mclx* mx
   ;  mclv* res = NULL

   ;  double clcf = 0.0, ccmax = 0.0
   ;  mcxIO* xfout =  mcxIOnew(out_g, "w")

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  mx = mcx_get_graph("mcx clcf", xfmx_g, xfabc_g, xftab_g, &tab_g, NULL, MCLX_REQUIRE_GRAPH)

   ;  progress_g  =  mcx_progress_g

   ;  if (!test_loop_g)
      mclxAdjustLoops(mx, mclxLoopCBremove, NULL)

   ;  if (list_g)
      fprintf(xfout->fp, "node\tclcf\n")

   ;  n_thread_l = mclx_set_threads_or_die("mcx clcf", n_thread_l, i_group, n_group_G)

   ;  if (n_thread_l * n_group_G > 1)           /* bit of a rickety interface */
      {  res = mclvClone(mx->dom_cols)
      ;  mclvMakeConstant(res, 0.0)
      ;  mclxVectorDispatchGroup(mx, res, n_thread_l, clcf_dispatch, n_group_G, i_group, NULL)
   ;  }
      else
      res = mclgCLCFdispatch(mx, 1)

   ;  {  dim i
      ;  for (i=0;i<N_COLS(mx);i++)
         {  double cc = res->ivps[i].val
         ;  long vid  = res->ivps[i].idx
         ;  clcf += cc
         ;  if (list_g)
            {  if (tab_g)
               {  const char* label = mclTabGet(tab_g, vid, NULL)
               ;  if (!label) mcxDie(1, "mcx clcf", "panic label %ld not found", vid)
               ;  fprintf(xfout->fp, "%s\t%.4f\n", label, cc)
            ;  }
               else
               fprintf(xfout->fp, "%ld\t%.4f\n", vid, cc)
         ;  }
            if (progress_g && cc > ccmax)
               fprintf(stderr, "new max vec %u clcf %.4f\n", (unsigned) i, cc)
            ,  ccmax = cc
         ;  if (progress_g && !((i+1) % progress_g))
            fprintf
            (  stderr
            ,  "%u average %.3f\n"
            ,  (unsigned) (i+1)
            ,  (double) (clcf/(i+1))
            )
      ;  }
         mclvFree(&res)
   ;  }

      if (N_COLS(mx))
      clcf /= N_COLS(mx)

   ;  if (!list_g)
      fprintf(xfout->fp, "%.3f\n", clcf)

   ;  return 0
;  }


mcxDispHook* mcxDispHookClcf
(  void
)
   {  static mcxDispHook clcfEntry
   =  {  "clcf"
      ,  "clcf [options]"
      ,  clcfOptions
      ,  sizeof(clcfOptions)/sizeof(mcxOptAnchor) - 1

      ,  clcfArgHandle
      ,  clcfInit
      ,  clcfMain

      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &clcfEntry
;  }


