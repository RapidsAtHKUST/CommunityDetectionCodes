/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <string.h>
#include <stdio.h>

#include "clm.h"
#include "report.h"
#include "clmadjust.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "mcl/interpret.h"

#include "clew/claw.h"
#include "clew/clm.h"
#include "clew/cat.h"


enum
{  MY_OPT_IMX     =  CLM_DISP_UNUSED
,  MY_OPT_ICL
,  MY_OPT_OUTPUT
,  MY_OPT_LINT_K
,  MY_OPT_LINT_L
,  MY_OPT_FORCE_CONNECTED
}  ;


static mcxOptAnchor adjustOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "read clustering matrix from file"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "read graph matrix from file"
   }
,  {  "-lint-k"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_LINT_K
   ,  "<num>"
   ,  "try to assimilate clusters of size up to <num> in other clusters"
   }
,  {  "-lint-l"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_LINT_L
   ,  "<num>"
   ,  "try to rebalance  clusters of size up to <num>"
   }
,  {  "--force-connected"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_FORCE_CONNECTED
   ,  NULL
   ,  "ensure that clusters induce connected subgraphs"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
static mcxIO*  xfcl     =  (void*) -1;
static mcxIO*  xfmx     =  (void*) -1;
static dim     lintl    =  -1;
static dim     lintk    =  -1;
static mcxbool fc       =  -1;


static mcxstatus adjustInit
(  void
)
   {  xfout =  mcxIOnew("-", "w")
   ;  xfcl  =  NULL
   ;  xfmx  =  NULL
   ;  lintl =  0
   ;  lintk =  0
   ;  fc    =  FALSE
   ;  return STATUS_OK
;  }


static mcxstatus adjustArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
;fprintf(stderr, "new name %s\n", xfout->fn->str)
      ;  break
      ;

         case MY_OPT_IMX
      :  xfmx = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_ICL
      :  xfcl = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_LINT_L
      :  lintl = atoi(val)
      ;  break
      ;

         case MY_OPT_FORCE_CONNECTED
      :  fc = TRUE
      ;  break
      ;

         case MY_OPT_LINT_K
      :  lintk = atoi(val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus adjustMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclMatrix   *cl         =  NULL
   ;  mclMatrix   *mx         =  NULL
   ;  mclMatrix   *cladj      =  NULL

   ;  const char* me          =  "clm adjust"

   ;  if (!xfcl || !xfmx)
      mcxDie(1, me, "need matrix and cluster files")

   ;  cl =  mclxRead(xfcl, EXIT_ON_FAIL)
   ;  mx =  mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)

   ;  if (fc)
      {  mclMatrix* cm = clmUGraphComponents(mx, cl)
      ;  if (N_COLS(cl) != N_COLS(cm))
         {  mclx* ct  = clmContingency(cl, cm)
         ;  dim i
         ;  for (i=0;i<N_COLS(ct);i++)
            {  if (ct->cols[i].n_ivps > 1)
               {  dim j
               ;  fprintf(stderr, "cluster %d split as clusters", (int) ct->cols[i].vid)
               ;  for (j=0;j<ct->cols[i].n_ivps;j++)
                  fprintf(stderr, " %d", (int) ct->cols[i].ivps[j].idx)
               ;  fputc('\n', stderr)
            ;  }
            }
         }
         mcxFree(&cl)
      ;  cl = cm
         /* mclxWrite(cm, xfout, MCLXIO_VALUE_NONE, RETURN_ON_FAIL) */
      ;  return 0
   ;  }

      if (lintl > 0)
      {  dim sjd_left, sjd_right, n_adjusted
      ;  mclx* cladj
      ;  mclv* lsadj

      ;  mcxLog
         (  MCX_LOG_FUNC
         ,  me
         ,  "rebalance (pre -lint-l) have %ld clusters"
         ,  (long) N_COLS(cl)
         )

      ;  if
         (  (  n_adjusted
            =  clmAdjust
               (mx, cl, lintl, &cladj, &lsadj, &sjd_left, &sjd_right)
            )
         )
         {  mcxLog
            (  MCX_LOG_FUNC
            ,  me
            ,  "have rebalanced clustering at distance [%lu,%lu]"
            ,  (ulong) sjd_left
            ,  (ulong) sjd_right
            )
         ;  mclvFree(&lsadj)
         ;  mclxFree(&cl)
         ;  cl = cladj
      ;  }
         else
         mcxLog(MCX_LOG_FUNC, me, "none reallocated")
   ;  }

      if (lintk > 0)
      {  dim sjd_left, sjd_right, n_adjusted

      ;  mcxLog
         (  MCX_LOG_FUNC
         ,  me
         ,  "assimilate (before -lint-k) have %ld clusters"
         ,  (long) N_COLS(cl)
         )

      ;  if
         (  (  n_adjusted
            =  clmAssimilate
               (mx, cl, lintk, &cladj, &sjd_left, &sjd_right)
            )
         )
         {  mcxLog(MCX_LOG_FUNC, me, "projected %lu nodes", (ulong) n_adjusted)
         ;  mclxFree(&cl)
         ;  cl = cladj
      ;  }
         else
         mcxLog(MCX_LOG_FUNC, me, "nothing to project")
   ;  }

      mclxWrite(cl, xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookAdjust
(  void
)
   {  static mcxDispHook adjustEntry
   =  {  "adjust"
      ,  "adjust [options] -icl <cl-file> -imx <mx-file>"
      ,  adjustOptions
      ,  sizeof(adjustOptions) / sizeof(mcxOptAnchor) - 1
      ,  adjustArgHandle
      ,  adjustInit
      ,  adjustMain
      ,  0
      ,  0 
      ,  MCX_DISP_DEFAULT
      }
   ;  return &adjustEntry
;  }


