/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* modes of operation.
 *    --center    vary between center and max (dag only)
 *       default: vary between self and max (works for both ite and dag)
*/

#include <string.h>

#include "clm.h"
#include "report.h"
#include "clmimac.h"

#include "util/types.h"
#include "util/err.h"
#include "util/ting.h"
#include "util/minmax.h"
#include "util/opt.h"
#include "util/array.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/ivp.h"
#include "impala/io.h"
#include "impala/app.h"

#include "clew/clm.h"
#include "clew/cat.h"

#include "mcl/interpret.h"


static const char* me = "clmmate";


enum
{  MY_OPT_OUTPUT = CLM_DISP_UNUSED
,  MY_OPT_IMX
,  MY_OPT_DAG
,  MY_OPT_STRICT
,  MY_OPT_OVERLAP
,  MY_OPT_SORT
}  ;


static mcxOptAnchor imacOptions[] =
{  {  "-overlap"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OVERLAP   
   ,  "<cut|keep|split>"
   ,  "cut, keep, or split overlap (default keep)"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output cluster file"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "input matrix file, presumably dumped mcl iterand or dag"
   }
,  {  "-sort"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SORT
   ,  "<str>"
   ,  "sort mode for clusters, str one of <revsize|size|lex>"
   }
,  {  "-dag"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DAG
   ,  "<fname>"
   ,  "output DAG associated with matrix"
   }
,  {  "-strict"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STRICT
   ,  "<num>"
   ,  "in (0..1)"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
mcxbits overlap_mode    =  -1;
mcxIO* xfdag            =  (void*) -1;
mcxIO* xfmx             =  (void*) -1;
const char* sortmode    =  (void*) -1;
static double strict    =  -1;


static mcxstatus imacInit
(  void
)
   {  xfout          =  mcxIOnew("-", "w")
   ;  xfmx           =  NULL
   ;  overlap_mode   =  ENSTRICT_KEEP_OVERLAP
   ;  xfdag          =  NULL
   ;  sortmode       =  "revsize"
   ;  strict         =  0.001
   ;  return STATUS_OK
;  }

static mcxstatus imacArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OVERLAP
      :  overlap_mode
         =     !strcmp(val, "cut")
            ?  ENSTRICT_CUT_OVERLAP
            :     !strcmp(val, "split")
               ?  ENSTRICT_SPLIT_OVERLAP
               :  ENSTRICT_KEEP_OVERLAP

      ;  break
      ;

         case MY_OPT_IMX
      :  xfmx = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         case MY_OPT_STRICT
      :  strict = atof(val)
      ;  if (strict <= 0)
         strict = 0.00001
      ;  if (strict > 1)
         strict = 1
      ;  break
      ;

         case MY_OPT_DAG
      :  xfdag = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_SORT
      :  sortmode = val
      ;  break
      ;

         default
      :  return STATUS_FAIL
   ;  }
      return STATUS_OK
;  }




static mcxstatus imacMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclInterpretParam* ipp  =  mclInterpretParamNew()

   ;  dim o = 0, m = 0, e = 0

   ;  mclx *mx = NULL, *dag = NULL, *cl = NULL

   ;  ipp->w_maxval  =   strict;
   ;  ipp->w_selfval = 1.0 - strict;

   ;  if (!xfmx)
      mcxDie(1, me, "-imx option is required")

   ;  mx = mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)
   ;  mclxMakeStochastic(mx)
   ;  mcxIOfree(&xfmx)

   ;  dag = mclDag(mx, ipp)

   ;  if (1)
      {  dim j
      ;  mclxMakeStochastic(dag)
      ;  for (j=0;j<N_COLS(dag);j++)
         mclvSelectGqBar(dag->cols+j, 1.0 / (dag->cols[j].n_ivps + 1))
   ;  }
      mclxFree(&mx)
   ;  cl    =  mclInterpret(dag)

;  {  mclx* cltp = mclxTranspose(cl)
   ;  dim x
   ;  for (x=0;x<N_COLS(cltp);x++)
      if (!cltp->cols[x].n_ivps)
      fprintf(stderr, "hey %lu\n", (ulong) x)
;  }

   ;  mclDagTest(dag)

   ;  if (xfdag)
      mclxWrite(dag, xfdag, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  mclxFree(&dag)

   ;  {  const char* did
         =     (overlap_mode & ENSTRICT_CUT_OVERLAP)
            ?  "cut"
            :     (overlap_mode & ENSTRICT_SPLIT_OVERLAP)
               ?  "split"
               :  "kept"

      ;  mcxIOopen(xfout, EXIT_ON_FAIL)

      ;  clmEnstrict(cl, &o, &m, &e, overlap_mode)
      ;  if (o)
         mcxTell(me, "%s <%lu> instances of overlap", did, (ulong) o)
      ;  if (m)
         mcxTell(me, "collected <%lu> missing nodes", (ulong) m)
      ;  if (e)
         mcxTell(me, "removed <%lu> empty clusters", (ulong) e)

      ;  if (!strcmp(sortmode, "size"))
         mclxColumnsRealign(cl, mclvSizeCmp)
      ;  else if (!strcmp(sortmode, "revsize"))
         mclxColumnsRealign(cl, mclvSizeRevCmp)
      ;  else if (!strcmp(sortmode, "lex"))
         mclxColumnsRealign(cl, mclvLexCmp)
      ;  else if (!strcmp(sortmode, "none"))
        /* ok */
      ;  else
         mcxErr(me, "ignoring unknown sort mode <%s>", sortmode)
                                                                                                     
      ;  mclxWrite(cl, xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)
      ;  mcxIOfree(&xfout)
   ;  }

      mclxFree(&cl)
   ;  mclInterpretParamFree(&ipp)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookImac
(  void
)
   {  static mcxDispHook imacEntry
   =  {  "imac"
      ,  "imac [options] -imx <mx file>"
      ,  imacOptions
      ,  sizeof(imacOptions)/sizeof(mcxOptAnchor) - 1
      ,  imacArgHandle
      ,  imacInit
      ,  imacMain
      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &imacEntry
;  }


