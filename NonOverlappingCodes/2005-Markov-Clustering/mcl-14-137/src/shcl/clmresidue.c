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
#include "clmresidue.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/tab.h"
#include "impala/app.h"

#include "mcl/interpret.h"

#include "clew/clm.h"
#include "clew/cat.h"

/*
 *    Ideal scenario: only read in the vectors that are needed, i.e. with a mask.
 *    The mask can be obtained from header information.
 *    (The needed vectors are those for which the vids are in the residue,
 *    which contains the vectors not covered by the clustering).
 *    This has not yet been implemented (and is for performance only).
*/


enum
{  MY_OPT_IMX     =  CLM_DISP_UNUSED
,  MY_OPT_ICL
,  MY_OPT_OUTPUT
,  MY_OPT_RPM
,  MY_OPT_TAB
,  MY_OPT_DUP
}  ;


static mcxOptAnchor residueOptions[] =
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
,  {  "-rpm"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RPM
   ,  "<fname>"
   ,  "residue projection matrix, node/cluster weight distribution"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
static mcxIO*  xfcl     =  (void*) -1;
static mcxIO*  xfmx     =  (void*) -1;
static mcxIO*  xfrpm    =  (void*) -1;
static mcxIO*  xftab    =  (void*) -1;
static mcxIO*  xfdup    =  (void*) -1;


static mcxstatus residueInit
(  void
)
   {  xfout =  mcxIOnew("-", "w")
   ;  xfcl  =  NULL
   ;  xfmx  =  NULL
   ;  xfrpm =  NULL
   ;  return STATUS_OK
;  }


static mcxstatus residueArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
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

         case MY_OPT_RPM
      :  xfrpm = mcxIOnew(val, "w")
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus residueMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclMatrix   *cl         =  NULL
   ;  mclMatrix   *cl2el      =  NULL
   ;  mclMatrix   *mx         =  NULL
   ;  mclMatrix   *mxres      =  NULL
   ;  mclMatrix   *clmxres    =  NULL

   ;  mclVector   *meet       =  NULL
   ;  mclVector   *residue    =  NULL

   ;  const char* me          =  "clmresidue"
   ;  dim i
   ;  dim o, m, e

   ;  if (!xfcl || !xfmx)
      mcxDie(1, me, "need matrix and cluster files")

   ;  cl =  mclxRead(xfcl, EXIT_ON_FAIL)
   ;  mx =  mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)

   ;  meet = mcldMeet(cl->dom_rows, mx->dom_cols, NULL)

   ;  if (meet->n_ivps != N_ROWS(cl))
      report_exit(me, SHCL_ERROR_DOMAIN)

   ;  residue = mcldMinus(mx->dom_cols, meet, NULL)

     /* fixme; no dummy cluster added -  breaks general behaviour */
   ;  if (!residue->n_ivps)
      {  mcxErr(me, "Ranges are identical - no residue to work with!")
      ;  mcxErr(me, "You still get a <%ld>x<0> matrix", (long) N_COLS(cl))
      ;  clmxres = mclxAllocZero(mclvInit(NULL), mclvCopy(NULL, cl->dom_cols))
      ;  mclxWrite(clmxres, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
      ;  mcxExit(0)
   ;  }
      else
      mcxTell(me, "Residue has <%ld> nodes", (long) residue->n_ivps)

     /* fixme: this only in adapt mode */
   ;  if (clmEnstrict(cl, &o, &m, &e, ENSTRICT_REPORT_ONLY))
      mcxDie(1, me, "not a partition")

     /* add residue nodes as separate cluster */
   ;  {  long newvid     =  mclvHighestIdx(cl->dom_cols) + 1
      ;  mclVector* clus
      ;  int n_cols_new = N_COLS(cl) + 1

      ;  cl->cols       =  (mclVector*) mcxRealloc
                           (  cl->cols
                           ,  n_cols_new * sizeof(mclVector)
                           ,  EXIT_ON_FAIL
                           )

     /* fixme: this code also in mcl/clm.c: factor out? */
     /* is mclxAccommodate useful here? */
      ;  clus = cl->cols + n_cols_new -1
      ;  mclvInit(clus)                            /* make consistent state */
      ;  clus->vid = newvid                        /* give vector identity  */
      ;  mclvRenew(clus, residue->ivps, residue->n_ivps)    /* fill   */
      ;  mclvInsertIdx(cl->dom_cols, newvid, 1.0)  /* update column domain  */
      ;  mcldMerge(cl->dom_rows, residue, cl->dom_rows)  /* add res to dom  */
     /*  NOW N_COLS(cl) has changed (by mclvInsertIdx) */

      ;  mcxTell(me, "Added dummy cluster <%ld> for residue nodes", (long) newvid)
   ;  }

      cl2el   =   mclxTranspose(cl)
   ;  mxres   =   mclxSub(mx, residue, mx->dom_rows)
   ;  clmxres =   mclxCompose(cl2el, mxres, 0, 0)        /* fixme: thread interface */

   ;  mcxIOfree(&xfcl)
   ;  mcxIOfree(&xfmx)

   ;  if (xfrpm)
      mclxWrite(clmxres, xfrpm, 6, EXIT_ON_FAIL)
      /* fixme: digits option */

     /* now empty cluster and allot all nodes */
   ;  mclvResize(cl->cols+N_COLS(cl)-1, 0)

   ;  for (i=0;i<N_COLS(clmxres);i++)
      {  long vid = clmxres->cols[i].vid, cid
      ;  mclVector* clus
         /* what if projection vector has zero entries [possible]?
          * let's continue then.
         */ 
      ;  if (!clmxres->cols[i].n_ivps)
         continue
      ;  mclvSortDescVal(clmxres->cols+i)
     /* DANGER: safer to sort copy */
      ;  cid = clmxres->cols[i].ivps[0].idx
      ;  clus = mclxGetVector(cl, cid, EXIT_ON_FAIL, NULL)
      ;  mclvInsertIdx(clus, vid, 1.0)
   ;  }
      mclxFree(&clmxres)
   /* clmxres was corrupted in loop above. */

   ;  clmEnstrict(cl, &o, &m, &e, ENSTRICT_SPLIT_OVERLAP)
   /* perhaps the extra dummy cluster was totally emptied */

   ;  mclxWrite(cl, xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)

   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookResidue
(  void
)
   {  static mcxDispHook residueEntry
   =  {  "residue"
      ,  "residue [options] -icl <cl-file> -imx <mx-file>"
      ,  residueOptions
      ,  sizeof(residueOptions)/sizeof(mcxOptAnchor) - 1
      ,  residueArgHandle
      ,  residueInit
      ,  residueMain
      ,  0
      ,  -1
      ,  MCX_DISP_MANUAL
      }
   ;  return &residueEntry
;  }


static mcxOptAnchor enstrictOptions[] =
{  {  "-icl"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "read clustering matrix from file"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxstatus enstrictArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;  
         case MY_OPT_ICL
      :  mcxIOnewName(xfcl  , val)
      ;  break
      ;  
         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus enstrictInit
(  void
)
   {  xfout    =  mcxIOnew("-", "w")
   ;  xfcl     =  mcxIOnew("-", "r")
   ;  return STATUS_OK
;  }


static mcxstatus enstrictMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclx* cl
   ;  dim o, m, e
   ;  cl  =  mclxRead(xfcl, EXIT_ON_FAIL)
   ;  mcxIOfree(&xfcl)
   ;  clmEnstrict(cl, &o, &m, &e, ENSTRICT_PARTITION)
   ;  mclxWrite(cl, xfout, -1, EXIT_ON_FAIL)
   ;  mcxIOfree(&xfout)
   ;  mclxFree(&cl)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookEnstrict
(  void
)
   {  static mcxDispHook enstrictEntry
   =  {  "enstrict"
      ,  "enstrict [-o <cl-file>] <cl-file>"
      ,  enstrictOptions
      ,  sizeof(enstrictOptions)/sizeof(mcxOptAnchor) - 1
      ,  enstrictArgHandle
      ,  enstrictInit
      ,  enstrictMain
      ,  0
      ,  0
      ,  MCX_DISP_DEFAULT
      }
   ;  return &enstrictEntry
;  }


static mcxOptAnchor foldOptions[] =
{  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "read clustering matrix from file"
   }
,  {  "-dup"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DUP
   ,  "<fname>"
   ,  "matrix file specifying duplicate nodes"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "tab file, graph will be folded on identical labels"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxstatus foldArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;  
         case MY_OPT_DUP
      :  xfdup = mcxIOnew(val, "r")
      ;  break
      ;  
         case MY_OPT_TAB
      :  xftab = mcxIOnew(val, "r")
      ;  break
      ;  
         case MY_OPT_IMX
      :  xfmx = mcxIOnew(val, "r")
      ;  break
      ;
         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus foldInit
(  void
)
   {  xfout =  mcxIOnew("-", "w")
   ;  xftab =  NULL
   ;  xfdup =  NULL
   ;  xfmx  =  NULL
   ;  return STATUS_OK
;  }


static mcxstatus foldMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclMatrix   *mx         =  NULL
   ;  mclMatrix   *dup        =  NULL
   ;  mclTab      *tab        =  NULL

   ;  const char* me          =  "clmfold"

   ;  if ((!xftab && !xfdup)  || !xfmx)
      mcxDie(1, me, "need tab and matrix files - identical entries in tab will be folded")

   ;  mx  =  mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)
   ;  if (xfdup)
      dup = mclxRead(xfdup, EXIT_ON_FAIL)
   ;  else
         tab = mclTabRead(xftab, mx->dom_cols, EXIT_ON_FAIL)
      ,  dup = mclTabDuplicated(tab, NULL)

   ;  mclxFold(mx, dup)
   ;  mcxTell(me, "found %ld duplicates", (long) mclxNrofEntries(dup))
   ;  mclxWrite(mx, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
;if(0) mclxWrite(dup, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)

   ;  mcxIOclose(xfout)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookFold
(  void
)
   {  static mcxDispHook foldEntry
   =  {  "fold"
      ,  "fold [options] -tab <tab-file> -imx <mx-file>"
      ,  foldOptions
      ,  sizeof(foldOptions)/sizeof(mcxOptAnchor) - 1
      ,  foldArgHandle
      ,  foldInit
      ,  foldMain
      ,  0
      ,  0
      ,  MCX_DISP_DEFAULT
      }
   ;  return &foldEntry
;  }

