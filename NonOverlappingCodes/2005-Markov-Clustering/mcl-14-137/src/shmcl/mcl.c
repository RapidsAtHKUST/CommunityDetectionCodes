/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#include "../../config.h"

/*  **************************************************************************
 * *
 **            Implementation notes (a few).
 *
 *    MCL front-end. Lots of options, many more have come and gone.
 *    Most options are inessential (and you dont see em here anyway).
 *
 *    The design is this:
 *
 * o     the routine mclProcess is provided by the library.
 *    It takes a stochastic matrix as input, and starts clustering using the
 *    supplied inflation parameter(s) and not much more.  Additionally, the
 *    library provides extensive support for different pruning strategies, and
 *    for tracking those (verbosity and progress options).  mclProcess belongs
 *    to the 'lib' or 'library' part.
 *
 * o     mclAlgorithm provides some handles for manipulating the input graph
 *    (most importantly provided by the -tf option), it provides some handles
 *    for output and re-chunking the mcl process (--expand-only and
 *    --inflate-first), and it provides handles (one?) for postprocessing like
 *    removing overlap.  mclAlgorithm belongs to the 'main' part.
 *
 *    We do library initialization first, because matrix reading is part of
 *    algorithm initialization. If the two switch order it is painful if errors
 *    are only detected after reading in the matrix (which takes time if it is
 *    huge).
 *
 *    There are some subtleties in command line parsing, as the lib and main
 *    interfaces are unified for the parsing part, but after that each does the
 *    interpretation of the arguments it accepts all by itself. Additionally,
 *    we check whether any info flag appears at the position of the file
 *    argument.
 *
 * o     These days we do streaming input as well. It is largely encapsulated
 *    but adds to the input/output/tab logic.
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
#include "impala/stream.h"
#include "impala/ivp.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "util/types.h"
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


static void helpful_reminder
(  void
)
   {
#ifdef MCL_HELPFUL_REMINDER
if (mcxLogLevel & MCX_LOG_UNIVERSE) {
fprintf(stderr, "\nPlease cite:\n");
fprintf(stderr, "    Stijn van Dongen, Graph Clustering by Flow Simulation.  PhD thesis,\n");
fprintf(stderr, "    University of Utrecht, May 2000.\n");
fprintf(stderr, "       (  http://www.library.uu.nl/digiarchief/dip/diss/1895620/full.pdf\n");
fprintf(stderr, "       or  http://micans.org/mcl/lit/svdthesis.pdf.gz)\n");
fprintf(stderr, "OR\n");
fprintf(stderr, "    Stijn van Dongen, A cluster algorithm for graphs. Technical\n");
fprintf(stderr, "    Report INS-R0010, National Research Institute for Mathematics\n");
fprintf(stderr, "    and Computer Science in the Netherlands, Amsterdam, May 2000.\n");
fprintf(stderr, "       (  http://www.cwi.nl/ftp/CWIreports/INS/INS-R0010.ps.Z\n");
fprintf(stderr, "       or  http://micans.org/mcl/lit/INS-R0010.ps.Z)\n\n");
}
#endif
   }



int main
(  int               argc
,  const char*       argv[]
)
   {  const char* fn_input    =  argc > 1 ? argv[1] : ""
   ;  mclAlgParam* mlp        =  NULL
   ;  const char* me          =  "mcl"
   ;  mcxstatus status        =  STATUS_OK

   ;  srandom(mcxSeed(315))
   ;  signal(SIGALRM, mclSigCatch)
   ;  if (signal(SIGUSR1, mcxLogSig) == SIG_ERR)
      mcxErr(me, "cannot catch SIGUSR1!")

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclx_app_init(stderr)

   ;  if (argc < 2)
      {  mcxTell
         (  me
         ,  "usage: mcl <-|file name> [options],"
            " do 'mcl -h' or 'man mcl' for help"
         )
      ;  mcxExit(0)
   ;  }

      status
      =  mclAlgInterface
         (&mlp, (char**) (argv+2), argc-2, fn_input, NULL, ALG_DO_IO)

   ;  if (status == ALG_INIT_DONE)
      return 0
   ;  else if (status)
      mcxDie(STATUS_FAIL, me, "no tango")

   ;  if ((status = mclAlgorithm(mlp)) == STATUS_FAIL)
      mcxDie(STATUS_FAIL, me, "failed")

   ;  if (mlp->n_assimilated)
      mcxLog(MCX_LOG_MODULE, me, "%lu nodes will assimilate", (ulong) mlp->n_assimilated)

   ;  if (mlp->mx_start)
      mcxLog(MCX_LOG_MODULE, me, "cached matrix with %lu columns", (ulong) N_COLS(mlp->mx_start))

   ;  mclAlgParamFree(&mlp, TRUE)
   ;  helpful_reminder()
   ;  return STATUS_OK
;  }


