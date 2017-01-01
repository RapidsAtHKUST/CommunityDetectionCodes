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

#include "impala/io.h"
#include "impala/tab.h"
#include "impala/stream.h"
#include "impala/ivp.h"
#include "impala/compose.h"
#include "impala/vector.h"
#include "impala/app.h"
#include "impala/iface.h"

#include "util/types.h"
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

const char* usagelines[];

const char* me = "mcxminusmeet";

void usage
(  const char**
)  ;


#define  MMM_OVERWRITE  1
#define  MMM_RIGHT      2
#define  MMM_CHECK      4
#define  MMM_BINARY     8
#define  MMM_DUMPMX     16
#define  MMM_MEET2      32


const char* usagelines[] =
{  "mcxminusmeet <modes> <matrix>"
,  "  modes:   1  new target rather than self"
,  "           2  right rather than left"
,  "           4  perform checks"
,  "           8  use binary"
,  "          16  dump final matrix (should be identical)"
,  "          32  use dispatching meet implementation"
,  NULL
}  ;


mclv* mcldMinus1
(  const mclv* lft
,  const mclv* rgt
,  mclv* dst
)
   {  return mclvBinary(lft, rgt, dst, fltLaNR)
;  }


void pairwise_setops
(  mclx* mx1
,  mclx* mx2
,  mcxbits modes
)
   {  dim t, u, n_tst = 0
   ;  mclv* cache   = mclvInit(NULL)
   ;  mclv* meet    = mclvInit(NULL)
   ;  mclv* join    = mclvInit(NULL)
   ;  mclv* diff    = mclvInit(NULL)
   ;  mcxbool overwrite = modes & MMM_OVERWRITE
   ;  dim n_zero_meet = 0, n_plus_meet = 0

   ;  mclv* (*fn_meet)(const mclv* lft, const mclv* rgt, mclv* dst)  =  mcldMeet
   ;  mclv* (*fn_minus)(const mclv* lft, const mclv* rgt, mclv* dst) =  mcldMinus1

   ;  if (modes & MMM_MEET2)
         fn_meet = mcldMeet2
      ,  fn_minus = mcldMinus

                                                      /* the point of overwrite is to have
                                                       * a lft == dst or rgt == dst pattern.
                                                      */
   ;  for (t=0;t<N_COLS(mx1);t++)
      {  for (u=0;u<N_COLS(mx2);u++)
         {  mclv* dst = overwrite ? (modes & MMM_RIGHT ? mx1->cols+u : mx2->cols+t) : diff
         ;  if (overwrite)
            mclvCopy(cache, dst)                      /* cache column, reinstate later */

         ;  if (modes & MMM_BINARY)
            mclvBinary(mx1->cols+t, mx2->cols+u, dst, fltLaNR)
         ;  else
            fn_minus(mx1->cols+t, mx2->cols+u, dst)  /* compute t / u */

         ;  if (overwrite)
               mclvCopy(diff, dst)
            ,  mclvCopy(dst, cache)                   /* reinstate column */
                                                      /* diff contains t / u */

         ;  dst = overwrite ? dst : meet              /* cache column, same as above */

         ;  if (modes & MMM_BINARY)
            mclvBinary(mx1->cols+t, mx2->cols+u, dst, fltLaR)
         ;  else
            fn_meet(mx1->cols+t, mx2->cols+u, dst)

         ;  if (overwrite)
               mclvCopy(meet, dst)
            ,  mclvCopy(dst, cache)                   /* meet contains t /\ u */

         ;  mcldMerge(diff, meet, join)               /* join should be identical to column t */

         ;  if (meet->n_ivps)
            n_plus_meet++
         ;  else
            n_zero_meet++

         ;  if (modes & MMM_CHECK)
            {  mclv* dediff = mclvClone(mx1->cols+t)
            ;  mclv* demeet = mclvClone(mx1->cols+t)
               
            ;  dim nd = mclvUpdateMeet(dediff, diff, fltSubtract)
            ;  dim nm = mclvUpdateMeet(demeet, meet, fltSubtract)

            ;  if
               (  diff->n_ivps + meet->n_ivps != mx1->cols[t].n_ivps
               || !mcldEquate(join, mx1->cols+t, MCLD_EQT_EQUAL)
               || diff->n_ivps != nd
               || meet->n_ivps != nm
               )
               {  mclvaDump(mx1->cols+t, stdout, -1, " ", MCLVA_DUMP_HEADER_ON)
               ;  mclvaDump(mx2->cols+u, stdout, -1, " ", MCLVA_DUMP_HEADER_ON)
               ;  mclvaDump(meet, stdout, -1, " ", MCLVA_DUMP_HEADER_ON)
               ;  mclvaDump(diff, stdout, -1, " ", MCLVA_DUMP_HEADER_ON)
               ;  mcxDie(1, me, "rats")
            ;  }

               mclvFree(&dediff)
            ;  mclvFree(&demeet)
         ;  }

            n_tst++
      ;  }
      }

      fprintf
      (  stdout
      ,  "meet was nonempty %.2f\n"
      ,  (double) (n_plus_meet * 1.0f / n_tst)
      )

   ;  fprintf
      (  stdout
      ,  "%d successful tests in %s%s %s mode (checked: %s)\n"
      ,  (int) n_tst
      ,  overwrite ? "overwrite" : "create"
      ,  overwrite ? ( modes & MMM_RIGHT ? "-right" : "-left" ) : ""
      ,     modes & MMM_BINARY
         ?  "generic"
         :  "update"
      ,  (modes & MMM_CHECK ? "yes" : "no")
      )
  ;   fprintf
      (  stdout
      ,  "meet-can: %10lu\n"
         "meet-zip: %10lu\n"
         "meet-s/l: %10lu\n"
         "diff-can: %10lu\n"
         "diff-zip: %10lu\n"
         "diff-s/l: %10lu\n"
      ,  (ulong) nu_meet_can
      ,  (ulong) nu_meet_zip
      ,  (ulong) nu_meet_sl
      ,  (ulong) nu_diff_can
      ,  (ulong) nu_diff_zip
      ,  (ulong) nu_diff_sl
      )

   ;  mclvFree(&cache)
   ;  mclvFree(&meet)
   ;  mclvFree(&join)
   ;  mclvFree(&diff)
;  }


int main
(  int                  argc
,  const char*          argv[]
)  
   {  mcxIO* xf1, *xf2
   ;  mclx* mx1, *mx2
   ;  mcxbits modes = 0

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclx_app_init(stdout)

   ;  if (argc < 4)
         mcxUsage(stdout, me, usagelines)
      ,  mcxExit(0)

   ;  modes =  atoi(argv[1])
   ;  xf1   =  mcxIOnew(argv[2], "r")
   ;  xf2   =  mcxIOnew(argv[3], "r")

   ;  mx1   =  mclxRead(xf1, EXIT_ON_FAIL)
   ;  mx2   =  mclxRead(xf2, EXIT_ON_FAIL)

   ;  pairwise_setops(mx1, mx2, modes)

   ;  if (modes & MMM_DUMPMX)
      {  mcxIO* xo = mcxIOnew("out.mmm", "w")
      ;  mclxWrite(mx1, xo, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  }

      mclxFree(&mx1)
   ;  mclxFree(&mx2)
   ;  mcxIOfree(&xf1)
   ;  mcxIOfree(&xf2)
   ;  return 0
;  }



