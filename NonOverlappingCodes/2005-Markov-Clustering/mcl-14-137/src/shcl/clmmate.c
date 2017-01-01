/*   (C) Copyright 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* TODO
 * compute when inner loop can be broken; all of xproj has been accounted for.
 *    (sum meetsize)
 * ? adapt or warn on mismatched domains.
*/


#include <string.h>
#include <stdio.h>

#include "clm.h"
#include "clmmate.h"

#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/compose.h"
#include "impala/ivp.h"
#include "impala/app.h"

#include "clew/clm.h"

static const char* me = "clmmate";

enum
{  MY_OPT_OUTPUT = CLM_DISP_UNUSED
,  MY_OPT_BATCH
,  MY_OPT_1TM
}  ;


static mcxOptAnchor mateOptions[] =
{  {  "--one-to-many"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_1TM
   ,  NULL
   ,  "only output pairs with multiple incidences in left <cl file>"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-b"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_BATCH
   ,  NULL
   ,  "batch mode, omit leading headers"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxbool legend   =  -1;
static mcxIO*  xfout    =  (void*) -1;
static mcxbool one2many =  -1;


static mcxstatus mateInit
(  void
)
   {  xfout = mcxIOnew("-", "w")
   ;  legend = TRUE
   ;  one2many = FALSE
   ;  return STATUS_OK
;  }


static mcxstatus mateArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_1TM
      :  one2many = TRUE
      ;  break
      ;

         case MY_OPT_BATCH
      :  legend = FALSE
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus mateMain
(  int         argc_unused    cpl__unused
,  const char* argv[]
)
   {  mcxIO* xfx, *xfy
   ;  mclx* mx, *my, *meet, *teem, *myt
   ;  dim x, y

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  xfx =  mcxIOnew(argv[0], "r")
   ;  mx  =  mclxRead(xfx, EXIT_ON_FAIL)
   ;  mcxIOclose(xfx)
   ;  xfy =  mcxIOnew(argv[1], "r")
   ;  my  =  mclxRead(xfy, EXIT_ON_FAIL)
   ;  myt =  mclxTranspose(my)

   ;  if (!MCLD_EQUAL(mx->dom_rows, my->dom_rows))
      mcxDie(1, me, "domains are not equal")

   ;  meet=  mclxCompose(myt, mx, 0, 0)      /* fixme thread interface */
   ;  teem=  mclxTranspose(meet)

   ;  if (legend)
      fprintf
      (  xfout->fp
      ,  "%-10s %6s %6s %6s %6s %6s %6s %6s\n"
      ,  "overlap"
      ,  "x-idx"
      ,  "y-idx"
      ,  "meet"
      ,  "xdiff"
      ,  "ydiff"
      ,  "x-size"
      ,  "y-size"
      )

   ;  for (x=0;x<N_COLS(meet);x++)
      {  mclv* xvec = meet->cols+x
      ;  long X = xvec->vid
      ;  long xsize = mx->cols[x].n_ivps

      ;  if (one2many && xvec->n_ivps < 2)
         continue

      ;  for (y=0;y<N_COLS(teem);y++)
         {  mclv* yvec = teem->cols+y
         ;  long Y = yvec->vid
         ;  long ysize = my->cols[y].n_ivps
         ;  double twinfac
         ;  long meetsize
         ;  mclp* ivp = mclvGetIvp(yvec, X, NULL)
         ;  if (!ivp)
            continue

         /*
          * meet size, left diff, right diff, right size.
         */

         ;  meetsize = ivp->val

         ;  if (!xsize && !ysize)         /* paranoia */
            continue

         ;  twinfac = 2 * meetsize / ( (double) (xsize + ysize) )

         ;  if (xfout)
            fprintf
            (  xfout->fp
            ,  "%-10.3f %6ld %6ld %6ld %6ld %6ld %6ld %6ld\n"
            ,  twinfac
            ,  X
            ,  Y
            ,  meetsize
            ,  xsize - meetsize
            ,  ysize - meetsize
            ,  xsize
            ,  ysize
            )
      ;  }
      }
      return STATUS_OK
;  }


mcxDispHook* mcxDispHookMate
(  void
)
   {  static mcxDispHook mateEntry
   =  {  "mate"
      ,  "mate [options] <cl file> <cl file>"
      ,  mateOptions
      ,  sizeof(mateOptions)/sizeof(mcxOptAnchor) - 1
      ,  mateArgHandle
      ,  mateInit
      ,  mateMain
      ,  2
      ,  2
      ,  MCX_DISP_MANUAL
      }
   ;  return &mateEntry
;  }


