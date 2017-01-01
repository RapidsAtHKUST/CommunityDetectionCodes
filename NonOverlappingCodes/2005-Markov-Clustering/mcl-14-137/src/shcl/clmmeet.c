/*   (C) Copyright 2001, 2002, 2003, 2004, 2005, 2006 Stijn van Dongen
 *   (C) Copyright 2007, 2008, 2009, 2010, 2011  Stijn van Dongen
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
#include "clmmeet.h"

#include "clew/clm.h"
#include "clew/cat.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"


static const char* me = "clmmeet";

enum
{  MY_OPT_OUTPUT = CLM_DISP_UNUSED
}  ;


static mcxOptAnchor meetOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;


static mcxstatus meetInit
(  void
)
   {  xfout = mcxIOnew("-", "w")
   ;  return STATUS_OK
;  }


static mcxstatus meetArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus meetMain
(  int                  argc
,  const char*          argv[]
)
   {  mcxIO          **xfmcs        =  NULL

   ;  mclMatrix      *lft           =  NULL
   ;  mclMatrix      *rgt           =  NULL
   ;  mclMatrix      *dst           =  NULL

   ;  int            a              =  0
   ;  int            n_mx           =  0
   ;  int            j
   ;  dim  o, m, e

   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
   ;  mclx_app_init(stderr)

   ;  xfmcs    =  (mcxIO**) mcxAlloc
                  (  (argc)*sizeof(mcxIO*)
                  ,  EXIT_ON_FAIL
                  )

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  for(j=a;j<argc;j++)
      {  xfmcs[n_mx] = mcxIOnew(argv[j], "r")
      ;  n_mx++
   ;  }

      if (!n_mx)
      mcxDie(1, me, "at least one clustering matrix required")

  /* Fixme: do a decent initialization with lft = clmTop() *before*
   * this loop (removing the need for ugly tmp assignment), but that requires
   * we know the correct domain to pass to it.  For that, we need to peak into
   * the first matrix.
  */

   ;  for (j=0;j<n_mx;j++)
      {  mclMatrix* tmp = mclxRead (xfmcs[j], EXIT_ON_FAIL)

      ;  if (clmEnstrict(tmp, &o, &m, &e, ENSTRICT_SPLIT_OVERLAP))
            report_partition("clmmeet", tmp, xfmcs[j]->fn, o, m, e)
         ,  mcxExit(1)

      ;  if (!lft)
         {  lft = tmp
         ;  continue
      ;  }
         else
         rgt = tmp

      ;  if (!MCLD_EQUAL(lft->dom_rows, rgt->dom_rows))
         mcxDie
         (  1
         ,  me
         ,  "domains not equal (files %s/%s)"
         ,  xfmcs[j-1]->fn->str
         ,  xfmcs[j]->fn->str
         )

      ;  mcxIOclose(xfmcs[j])

      ;  dst   =  clmMeet(lft, rgt)
      ;  lft   =  dst
      ;  mclxFree(&rgt)
   ;  }

      mclxColumnsRealign(lft, mclvSizeRevCmp)
   ;  mclxWrite(lft, xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)

   ;  mclxFree(&lft)
   ;  mcxIOfree(&xfout)
   ;  free(xfmcs)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookMeet
(  void
)
   {  static mcxDispHook meetEntry
   =  {  "meet"
      ,  "meet [options] <cl file>+"
      ,  meetOptions
      ,  sizeof(meetOptions)/sizeof(mcxOptAnchor) - 1
      ,  meetArgHandle
      ,  meetInit
      ,  meetMain
      ,  1
      ,  -1
      ,  MCX_DISP_MANUAL
      }
   ;  return &meetEntry
;  }


/* 
static mcxbool lax_g    =  -1;
   ;  lax_g = FALSE

         case DIST_OPT_MEET
      :  lax_g = TRUE
      ;  break
      ;

   ;  if (lax_g)
      bits ^= MCLX_REQUIRE_DOMSTACK

   ;  if (lax_g)
      {  for (i=0;i<st.n_level;i++)
         {
      ;  }
   ;  }
*/
