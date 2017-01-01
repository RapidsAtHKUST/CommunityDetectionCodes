/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/*
 * TODO
 *
 * Clean up, modularize, compactify.
 * Different modes are now messy:
 *    vanilla/contract
 *    subcluster
 *    integrate
 *    dispatch
 *
 * Component part still is a bit tricky.
 *
 * ? optionally add layer of singleton nodes?
 * ? are the loops set here redone in mclAlgorithm?
 *
 * If non-ground-up projection is used, need to keep track of
 * contracted matrices and chain of backprojected clusterings (NYI)
 *
 * The top level code is a bit stretched. especially -c, -b1, -b2
 * functionality (initialization).
 *
 * [?] where does vec->val come from with --write-graphx + --shadow
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

#include "../../config.h"        /* needed for MCL_HELPFUL_REMINDER */

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/stream.h"
#include "impala/ivp.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "clew/clm.h"
#include "clew/cat.h"

#include "util/types.h"
#include "util/ding.h"
#include "util/alloc.h"
#include "util/ting.h"
#include "util/io.h"
#include "util/err.h"
#include "util/equate.h"
#include "util/rand.h"
#include "util/opt.h"

#include "mcl/proc.h"
#include "mcl/procinit.h"
#include "mcl/alg.h"
#include "mcl/transform.h"

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


enum
{  MY_OPT_SHARED
,  MY_OPT_TRANSFORM
,  MY_OPT_Z
,  MY_OPT_BASE
,  MY_OPT_B1
,  MY_OPT_B2
,  MY_OPT_ADDTP
,  MY_OPT_HDP
,  MY_OPT_SHADOW
,  MY_OPT_ANNOT
,  MY_OPT_IAF
,  MY_OPT_CL
,  MY_OPT_CONTROL
,  MY_OPT_CONTRACT
,  MY_OPT_SUBCLUSTER
,  MY_OPT_SUBCLUSTERX
,  MY_OPT_INTEGRATE
,  MY_OPT_DISPATCH
,  MY_OPT_NMAX
,  MY_OPT_ROOT
,  MY_OPT_CONE
,  MY_OPT_STACK
,  MY_OPT_COARSE
,  MY_OPT_BASENAME
,  MY_OPT_WRITE
,  MY_OPT_STEM
,  MY_OPT_MULTIPLEX
,  MY_OPT_HELP
,  ALG_OPT_QUIET
,  ALG_OPT_SETENV
,  MY_OPT_APROPOS
,  MY_OPT_VERSION
}  ;


const char* syntax = "Usage: mclcm <mx-name> [mclcm options] [-- \"<mcl-opts>\"*]";

mcxOptAnchor options[] =
{  {  "--version"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_VERSION
   ,  NULL
   ,  "output version information, exit"
   }
,  {  "-h"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "this info (currently)"
   }
,  {  "-b1"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_B1
   ,  "<opts>"
   ,  "dedicated parameters to construct base transformed matrix"
   }
,  {  "-b2"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_B2
   ,  "<opts>"
   ,  "dedicated parameters to construct base expanded matrix"
   }
,  {  "-q"
   ,  MCX_OPT_HASARG
   ,  ALG_OPT_QUIET
   ,  "<log-spec>"
   ,  "quiet level of logging"
   }
,  {  "-set"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  ALG_OPT_SETENV
   ,  "key=val"
   ,  "set key to value val in the environment"
   }
,  {  "-xxx-hdp"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_HDP
   ,  "<num>"
   ,  "take hadamard power for transposed cluster matrix"
   }
,  {  "--xxx-add-tp"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_ADDTP
   ,  NULL
   ,  "symmetrify coarsened matrix by adding transpose"
   }
,  {  "-annot"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ANNOT
   ,  "<tag/description>"
   ,  "describes the experiment, useful when command-line is propagated"
   }
,  {  "-iaf"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_IAF
   ,  "<pct>"
   ,  "inflation adapation factor (decrease when stagnating)"
   }
,  {  "-z"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_Z
   ,  NULL
   ,  "show default shared options"
   }
,  {  "-a"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SHARED
   ,  "<opts>"
   ,  "shared mcl options"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to matrix values"
   }
,  {  "-write-base"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_BASENAME
   ,  "<fname>"
   ,  "file to write base graph to"
   }
,  {  "-coarse"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_COARSE
   ,  "<fname>"
   ,  "file to write coarsened graphs to"
   }
,  {  "-cone"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CONE
   ,  "<fname>"
   ,  "file to write nested cluster stack to"
   }
,  {  "-stack"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STACK
   ,  "<fname>"
   ,  "file to write expanded cluster stack to"
   }
,  {  "-write"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_WRITE
   ,  "<tags>"
   ,  "<tags> may contain 'stack', 'cone', 'coarse', 'steps'"
   }
,  {  "-root"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ROOT
   ,  "y/n"
   ,  "make sure universe clustering is at the top"
   }
,  {  "-n"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NMAX
   ,  "<num>"
   ,  "iterate at most <num> times (default: until root clustering)"
   }
,  {  "-stem"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STEM
   ,  "<str>"
   ,  "file prefix for cone, stack, and coarse"
   }
,  {  "--mplex"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MULTIPLEX
   ,  "y/n"
   ,  "additionally write clusterings to separate files"
   }
,  {  "-c"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CL
   ,  "<fname>"
   ,  "use as start clustering"
   }
,  {  "--contract"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_CONTRACT
   ,  NULL
   ,  "use contraction approach (default)"
   }
,  {  "--subcluster"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SUBCLUSTER
   ,  NULL
   ,  "use subclustering approach"
   }
,  {  "--subcluster-x"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_SUBCLUSTERX
   ,  NULL
   ,  "use subclustering approach, reduce block-internal edges"
   }
,  {  "--integrate"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_INTEGRATE
   ,  NULL
   ,  "construct hierarchy from cluster (file) arguments"
   }
,  {  "--dispatch"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DISPATCH
   ,  NULL
   ,  "construct hierarchy from multiple mcl runs"
   }
,  {  "-ctrl"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_CONTROL
   ,  "<fname>"
   ,  "controlling clustering"
   }
,  {  "--help"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_APROPOS
   ,  NULL
   ,  "this info"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


const char* usagelines[];

const char* me = "mclcm";

void usage
(  const char**
)  ;


static mclx* control_test
(  mclx* clnext
,  const mclx* clctrl
)
   {  if (!clctrl)
      return clnext

   ;  {  mclx* meet = clmMeet(clctrl, clnext)
      ;  dim dist_ctrl_curr, dist_curr_ctrl
      ;  clmSJDistance
         (clnext, clctrl, NULL, NULL, &dist_curr_ctrl, &dist_ctrl_curr)
      ;  mcxLog
         (  MCX_LOG_APP
         ,  me
         ,  "distance to control: %lu %lu %lu"
         ,  (ulong) (dist_curr_ctrl + dist_ctrl_curr)
         ,  (ulong) dist_curr_ctrl
         ,  (ulong) dist_ctrl_curr
         )
      ;  if (dist_curr_ctrl <= dist_ctrl_curr)
         {  mclxFree(&clnext)
         ;  clnext = meet
      ;  }
         else
         mclxFree(&meet)
   ;  }
      return clnext
;  }


static mcxbool subcluster_g   =  FALSE;
static mcxbool subclusterx_g   =  FALSE;
static mcxbool integrate_g    =  FALSE;
static mcxbool dispatch_g     =  FALSE;
static mclxCat stck_g;
static double hdp_g           =  0.0;
static mclv* start_col_sums_g =  NULL;


void write_clustering
(  mclx* cl
,  const mclx* clprev
,  mcxIO* xfcone
,  mcxIO* xfstack
,  const char* plexprefix
,  int multiplex_idx
,  const mclAlgParam* mlp
)
   {  
                        /* this branch is also taken for dispatch mode */
      if (plexprefix)
      {  mcxTing* clname = mcxTingPrint(NULL, "%s.%03d", plexprefix, multiplex_idx)
      ;  mcxIO* xfout = mcxIOnew(clname->str, "w")

      ;  if (dispatch_g && mlp && !mcxIOopen(xfout, RETURN_ON_FAIL))
         fprintf(xfout->fp, "# %s\n", mlp->cline->str)  

      ;  mclxaWrite(cl, xfout, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
      ;  mcxTingFree(&clname)
      ;  mcxIOfree(&xfout)
   ;  }
      
      if (subcluster_g || dispatch_g)
      return

   ;  if (xfstack)
      mclxaWrite(cl, xfstack, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)

   ;  if (xfcone && !clprev)
      mclxaWrite(cl, xfcone, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
   ;  else if (xfcone)
      {  mclx* clprevt = mclxTranspose(clprev)
      ;  mclx* contracted = mclxCompose(clprevt, cl, 0, 0)
      ;  mclxMakeCharacteristic(contracted)
      ;  mclxaWrite(contracted, xfcone, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
      ;  mclxFree(&clprevt)
      ;  mclxFree(&contracted)
   ;  }
   }


void help
(  mcxOptAnchor* options
,  mcxTing* shared
)
   {  if (options)
      mcxOptApropos(stdout, me, syntax, 0, MCX_OPT_DISPLAY_SKIP, options)
   ;  fprintf(stdout, "\nDefault options: %s\n", shared->str)
   ;  fprintf
      (  stdout
      ,  "These can be overridden using -a \"<opts>\" or"
         " with the optional trailing argument specifications\n"
      )
   ;  fprintf
      (  stdout
      ,  "\nThe following is an example of using label input:\n"
         "mclcm <foo> -a \"-I 4\" -- \"--expect-abc --write-tab=<foo>.tab\"\n"
      )
;  }


      /* fixme: more robust would be to use distance to meet */
static int annot_cmp_coarse_first
(  const void* a1
,  const void* a2
)
   {  const mclx* mx1 = ((mclxAnnot*) a1)->mx
   ;  const mclx* mx2 = ((mclxAnnot*) a2)->mx
   ;  return
            N_COLS(mx1) < N_COLS(mx2)
         ?  -1
         :     N_COLS(mx1) > N_COLS(mx2)
            ?  1
            :  0
;  }


static mcxstatus get_interface
(  mclAlgParam** mlpp
,  const char* fn_input          /* Use this as input or mx_input */
,  const char* arg_shared
,  const char* arg_extra
,  mclx* mx_input                /* Use this as input or fn_input */
,  mcxbits CACHE
,  mcxOnFail ON_FAIL
)
   {  mcxTing* spec  =  mcxTingNew(arg_shared)
   ;  int argc1      =  0
   ;  char** argv1
   ;  mcxstatus status
   ;  mclAlgParam* mymlp =  NULL
   ;  mclAlgParam** mymlpp = mlpp ? mlpp : &mymlp

   ;  if (arg_extra)
      mcxTingPrintAfter(spec, " %s", arg_extra)

                           /* warning this clobbers spec->str */
   ;  argv1 = mcxOptParseString(spec->str, &argc1, ' ')
   ;  status
      =  mclAlgInterface
         (  mymlpp
         ,  argv1
         ,  argc1
         ,  fn_input
         ,  mx_input
         ,  CACHE
         )
      
   ;  if (status && ON_FAIL == EXIT_ON_FAIL)
      mcxExit(1)

   ;  mcxFree(argv1)
   ;  mcxTingFree(&spec)
                     /* fixfixfixmefixmefffixme: mclAlgInterface might use opt->val
                      * which points to somewhere in spec->str. Check.
                     */

   ;  if (!mlpp)
      mclAlgParamFree(mymlpp, TRUE)

   ;  return status
;  }


static mclx* get_coarse
(  const mclx* mxbase
,  mclx* clprev
,  mcxbool add_transpose
)
   {  mclx* blockc   =  mclxBlocksC(mxbase, clprev)
   ;  mclx* clprevtp =  mclxTranspose(clprev)
   ;  mclx *p1       =  NULL     /* p_roduct */
   ;  mclx* mx_coarse=  NULL

   ;  mclxMakeStochastic(clprev)

/****************** <EXPERIMENTAL CRAP>  ************************************/
   ;  if (hdp_g)
      mclxUnary(clprev, fltxPower, &hdp_g)
                        /* parameter: use mxbase rather than blockc */
   ;  if (getenv("MCLCM_BLOCK_STOCHASTIC")) /* this works very badly! */
      mclxMakeStochastic(blockc)

   ;  else if (getenv("MCLCM_BASE_UNSCALE") && start_col_sums_g)
      {  dim i
      ;  for (i=0;i<N_COLS(blockc);i++)
         {  double f = start_col_sums_g->ivps[i].val
         ;  mclvUnary(blockc->cols+i, fltxMul, &f)
      ;  }
   ;  }
/****************** </EXPERIMENTAL> *****************************************/

      p1 = mclxCompose(blockc, clprev, 0, 0)
;if (0)
{mcxIO* t = mcxIOnew("-", "w")
;mclxWrite(blockc, t, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
;
}
   ;  mclxFree(&blockc)
   ;  mx_coarse = mclxCompose(clprevtp, p1, 0, 0)
   ;  if (add_transpose)
      mclxAddTranspose(mx_coarse, 0.0)
   ;  mclxAdjustLoops(mx_coarse, mclxLoopCBremove, NULL)

   ;  mclxFree(&p1)
   ;  mclxFree(&clprevtp)

   ;  mclxMakeCharacteristic(clprev)
   ;  return mx_coarse
;  }


static void write_coarse
(  mcxIO* xf_coarse
,  mclx*  mx_coarse
)
   {  mclx* tmp = mclxCopy(mx_coarse)
   ;  double mv = mclxMaxValue(tmp)
   ;  double sc = mv ? mv / 100.0 : 1.0
   ;  mclxUnary(tmp, fltxScale, &sc)
   ;  mclxaWrite(tmp, xf_coarse, 4, RETURN_ON_FAIL)
   ;  mclxFree(&tmp)
;  }


static void integrate_results
(  mclxCat* cat
)
   {  dim i
   ;  qsort(cat->level, cat->n_level, sizeof(mclxAnnot), annot_cmp_coarse_first)
   ;  for (i=1;i<cat->n_level;i++)
      {  mclx* meet = clmMeet(cat->level[i-1].mx, cat->level[i].mx)
      ;  mclxFree(&(cat->level[i].mx))
      ;  cat->level[i].mx = meet
   ;  }
      mclxCatReverse(cat)
;  }


static mclx* get_base
(  const char* fn_input
,  mclx*       mx_input
,  const char* b1opts
,  const char* b2opts
)
   {  mclAlgParam* mlp =   NULL
   ;  mcxstatus status =   STATUS_FAIL
   ;  mclx* mxbase     =   NULL  

   ;  get_interface
      (  &mlp
      ,  fn_input
      ,  b2opts ? b2opts : b1opts
      ,  NULL
      ,  mx_input
      ,  b2opts ? ALG_CACHE_EXPANDED : ALG_CACHE_START
      ,  EXIT_ON_FAIL
      )

   ;  if (b2opts)
      {  mlp->mpp->mainLoopLength = 1
      ;  mlp->mpp->mainInflation = 1.0
      ;  mlp->expand_only = TRUE

      ;  mcxLog(MCX_LOG_APP, me, "computing expanded matrix")
      ;  if ((status = mclAlgorithm(mlp)) == STATUS_FAIL)
         {  mcxErr(me, "failed")
         ;  mcxExit(1)
      ;  }
         mcxLog(MCX_LOG_APP, me, "done computing expanded matrix")
      ;  mxbase = mclAlgParamRelease(mlp, mlp->mx_expanded)
   ;  }
      else
      {  if (mclAlgorithmStart(mlp, FALSE))
         mcxDie(1, me, "-b1 option start-up failed")
      ;  mxbase = mclAlgParamRelease(mlp, mlp->mx_start)
      ;  start_col_sums_g = mclvClone(mlp->mx_start_sums)
   ;  }
      mclAlgParamFree(&mlp, TRUE)
   ;  return mxbase
;  }



#define  OUTPUT_STACK   1 <<  0
#define  OUTPUT_CONE    1 <<  1
#define  OUTPUT_STEPS   1 <<  2
#define  OUTPUT_COARSE  1 <<  3
#define  OUTPUT_BASE    1 <<  4

                                 /* fixme: too big, although half is initialization
                                  * a lot of base1 base2 logic.
                                 */

int main
(  int                  argc
,  const char*          argv[]
)  
   {  mcxIO
         *xfcl    =  NULL
      ,  *xfctrl  =  NULL
      ,  *xfcoarse=  NULL
      ,  *xfbase  =  NULL
      ,  *xfcone  =  NULL
      ,  *xfstack =  NULL

   ;  mclx* mxbase, *cl, *cl_coarse, *clprev, *clctrl = NULL

   ;  mcxTing* shared = mcxTingNew("-I 4 -overlap split")
   ;  mcxbool root = TRUE
   ;  mcxbool have_bootstrap = FALSE
   ;  const char* plexprefix = NULL
   ;  const char* stem = "mcl"
   ;  mcxbool same = FALSE
   ;  mcxbool plex = TRUE
   ;  mcxbool add_transpose = FALSE
   ;  const char* b2opts = NULL
   ;  const char* b1opts = NULL
   ;  mcxbits write_modes = 0

   ;  mclAlgParam* mlp        =  NULL
   ;  mcxstatus status        =  STATUS_OK
   ;  mcxstatus parse_status  =  STATUS_OK
   ;  int multiplex_idx = 1
   ;  int N = 0
   ;  int n_ite = 0
   ;  dim n_components = 0, n_cls = 0


   ;  int a =  1, i= 0
   ;  int n_arg_read = 0
   ;  int delta = 0
   ;  mcxOption* opts, *opt
   ;  mcxTing* cline = mcxOptArgLine(argv+1, argc-1, '\'')
   ;  mcxTing* transform_spec = NULL


   ;  double iaf = 0.84

   ;  mclx_app_init(stderr)

   ;  if (0)
      mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  else
      mcxLogLevelSetByString("xf4g1")

   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)

   ;  if (argc == 2 && argv[1][0] == '-' && mcxOptIsInfo(argv[1], options))
      delta = 1
   ;  else if (argc < 2)
      {  help(options, shared)
      ;  exit(0)
   ;  }

      opts = mcxOptExhaust
            (options, (char**) argv, argc, 2-delta, &n_arg_read, &parse_status)

   ;  if (parse_status != STATUS_OK)
      {  mcxErr(me, "initialization failed")
      ;  exit(1)
   ;  }

   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch

      ;  switch(anch->id)
         {  case MY_OPT_HELP
         :  help(options, shared)
         ;  exit(0)
         ;

            case MY_OPT_APROPOS
         :  help(options, shared)
         ;  exit(0)
         ;  break
         ;

            case MY_OPT_NMAX
         :  N = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_Z
         :  help(NULL, shared)
         ;  exit(0)
         ;  break
         ;

            case MY_OPT_SHARED
         :  mcxTingPrintAfter(shared, " %s", opt->val)
         ;  break
         ;

            case MY_OPT_TRANSFORM
         :  transform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_B1
         :  b1opts = opt->val
         ;  break
         ;

            case MY_OPT_B2
         :  b2opts = opt->val
         ;  break
         ;

            case ALG_OPT_SETENV
         :  mcxSetenv(opt->val)
         ;  break
         ;

            case ALG_OPT_QUIET
         :  mcxLogLevelSetByString(opt->val)
         ;  break
         ;

            case MY_OPT_HDP
         :  hdp_g = atof(opt->val)
         ;  break
         ;

            case MY_OPT_ADDTP
         :  add_transpose = TRUE
         ;  break
         ;

            case MY_OPT_ANNOT       /* only used in command-line copying */
         :  break
         ;

            case MY_OPT_IAF
         :  iaf = atof(opt->val) / 100
         ;  break
         ;

            case MY_OPT_WRITE
         :  if (strstr(opt->val, "stack"))
            write_modes |= OUTPUT_STACK
         ;  if (strstr(opt->val, "cone"))
            write_modes |= OUTPUT_CONE
         ;  if (strstr(opt->val, "levels") || strstr(opt->val, "steps"))
            write_modes |= OUTPUT_STEPS
         ;  if (strstr(opt->val, "coarse"))
            write_modes |= OUTPUT_COARSE
         ;  if (strstr(opt->val, "base"))
            write_modes |= OUTPUT_BASE
         ;  break
         ;

            case MY_OPT_BASENAME
         :  xfbase = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_COARSE
         :  xfcoarse = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_CONE
         :  xfcone = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_ROOT
         :  root = strchr("1yY", (u8) opt->val[0]) ? TRUE : FALSE
         ;  break
         ;

            case MY_OPT_STACK
         :  xfstack = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_STEM
         :  stem = opt->val
         ;  break
         ;

            case MY_OPT_MULTIPLEX
         :  plex = strchr("yY1", (unsigned char) opt->val[0]) ? TRUE : FALSE
         ;  break
         ;

            case MY_OPT_DISPATCH
         :  dispatch_g = TRUE
         ;  break
         ;

            case MY_OPT_INTEGRATE
         :  integrate_g = TRUE
         ;  break
         ;

            case MY_OPT_CONTRACT
         :  break
         ;

            case MY_OPT_SUBCLUSTERX
         :  subclusterx_g = TRUE,  subcluster_g = TRUE
         ;  break
         ;

            case MY_OPT_SUBCLUSTER
         :  subcluster_g = TRUE
         ;  break
         ;

            case MY_OPT_CONTROL
         :  xfctrl = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_CL
         :  xfcl = mcxIOnew(opt->val, "r")
         ;  have_bootstrap = TRUE
         ;  break
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  exit(0)
         ;

            default
         :  mcxExit(1)
         ;
         }
      }

      mcxOptFree(&opts)

   ;  a = 2 + n_arg_read

   ;  if (a < argc)
      {  if (strcmp(argv[a], "--"))
         mcxDie
         (  1
         ,  me
         ,  "trailing %s options require standalone '--' separator (found %s)"
         ,  integrate_g ? "integrate" : "mcl"
         ,  argv[a]
         )
      ;  a++
   ;  }

      if (subcluster_g + dispatch_g + integrate_g > 1)
      mcxDie(1, me, "too many modes!")

   ;  if (N && N < argc-a)
      mcxErr(me, "-n argument leaves spurious option specifications")

   ;  srandom(mcxSeed(89315))
   ;  signal(SIGALRM, mclSigCatch)

   ;  if (dispatch_g)
      plexprefix = "dis"
   ;  else if (!write_modes || (write_modes & OUTPUT_STEPS))
      plexprefix = stem

   ;  {  mcxTing* tg = mcxTingEmpty(NULL, 30)
      ;  if ((write_modes & OUTPUT_COARSE) && !xfcoarse)
            mcxTingPrint(tg, "%s.%s", stem, "coarse")
         ,  xfcoarse = mcxIOnew(tg->str, "w")

      ;  if ((write_modes & OUTPUT_BASE) && !xfbase)
            mcxTingPrint(tg, "%s.%s", stem, "base")
         ,  xfbase = mcxIOnew(tg->str, "w")

      ;  if
         (  (!write_modes || (write_modes & OUTPUT_CONE))
         && !xfcone
         )
         {  mcxTingPrint(tg, "%s.%s", stem, "cone")
         ;  xfcone = mcxIOnew(tg->str, "w")
         ;  mcxIOopen(xfcone, EXIT_ON_FAIL)
         ;  fprintf(xfcone->fp, "# %s %s\n", argv[0], cline->str)
      ;  }

         if ((write_modes & OUTPUT_STACK) && !xfstack)
         {  mcxTingPrint(tg, "%s.%s", stem, "stack")
         ;  xfstack = mcxIOnew(tg->str, "w")
         ;  mcxIOopen(xfstack, EXIT_ON_FAIL)
         ;  fprintf(xfstack->fp, "# %s %s\n", argv[0], cline->str)
      ;  }

         mcxTingFree(&tg)
   ;  }

      if (integrate_g)
      {  for (i=a;i<argc;i++)
         {  mcxIO* xf = mcxIOnew(argv[i], "r")
         ;  mclx* cl = mclxRead(xf, EXIT_ON_FAIL)
         ;  mclxCatPush(&stck_g, cl, NULL, NULL, mclxCBdomStack, NULL, "dummy-integrate", n_cls++)
      ;  }

         integrate_results(&stck_g)

      ;  if (xfstack)
         mclxCatWrite(xfstack, &stck_g, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)

      ;  if (xfcone)
            mclxCatConify(&stck_g)
         ,  mclxCatWrite(xfcone, &stck_g, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)

      ;  return 0
   ;  }

      for (i=a;i<argc;i++)
      {  if (get_interface(NULL, argv[1], shared->str, argv[i], NULL, 0, RETURN_ON_FAIL))
         mcxDie(1, me, "error while testing mcl options viability (%s)", argv[i])
   ;  }


      mcxLog(MCX_LOG_APP, me, "pid %ld", (long) getpid())

                        /* make sure clusters align with this cluster
                         * status: does not seem promising.
                        */
   ;  if (xfctrl)
      clctrl = mclxRead(xfctrl, EXIT_ON_FAIL)
   ;

                        /*
                         * Below: compute cl and mxbase.
                        */
   ;  if (xfcl)
      {  cl = mclxRead(xfcl, EXIT_ON_FAIL)
      ;  write_clustering
         (cl, NULL, xfcone, xfstack, plexprefix, multiplex_idx++, NULL)

      ;  if (subcluster_g || dispatch_g)
         mclxCatPush(&stck_g, cl, NULL, NULL, mclxCBdomStack, NULL, "dummy-mclcm", n_cls++)

      ;  mcxIOfree(&xfcl)
      ;  if (!b1opts && !b2opts)
         b1opts = ""
      ;  mxbase = get_base(argv[1], NULL, b1opts, b2opts)
   ;  }
      else
      {  mcxbits CACHE  =     b1opts || b2opts
                           ?  ALG_CACHE_INPUT       /* cache, transform later */
                           :  ALG_CACHE_START
      ;  get_interface
         (  &mlp
         ,  argv[1]
         ,  shared->str
         ,  a < argc ? argv[a] : NULL
         ,  NULL
         ,  CACHE
         ,  EXIT_ON_FAIL
         )
      ;  if (a < argc)
         a++

      ;  if ((status = mclAlgorithm(mlp)) == STATUS_FAIL)
         {  mcxErr(me, "failed at initial run")
         ;  exit(1)
      ;  }

         cl_coarse =  mclAlgParamRelease(mlp, mlp->cl_result)
      ;  cl_coarse =  control_test(cl_coarse, clctrl)

      ;  write_clustering
         (cl_coarse, NULL, xfcone, xfstack, plexprefix, multiplex_idx++, mlp)

      ;  if (subcluster_g || dispatch_g)
         mclxCatPush(&stck_g, cl_coarse, NULL, NULL, mclxCBdomStack, NULL, "dummy-mclcm", n_cls++)

      ;  cl = cl_coarse
      ;  n_ite++

      ;  if (b1opts || b2opts)
         {  mclx* mx_input =  mclAlgParamRelease(mlp, mlp->mx_input)
         ;  mxbase = get_base(NULL, mx_input, b1opts, b2opts)
                           /* ^ get_base frees mx_input */
      ;  }
         else
         mxbase =  mclAlgParamRelease(mlp, mlp->mx_start)
   ;  }

      clprev = cl

   ;  mclAlgParamFree(&mlp, TRUE)

   ;  if (xfbase)
      {  dim nre = mclxNrofEntries(mxbase)
      ;  mcxLog(MCX_LOG_APP, me, "base has %lu entries", (ulong) nre)
      ;  mclxaWrite(mxbase, xfbase, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
      ;  mcxIOclose(xfbase)
   ;  }

      if (subcluster_g || dispatch_g)
      iaf = iaf ? 1/iaf : 1.414

   ;  while
      (  (!dispatch_g && (!N || n_ite < N))
      || (dispatch_g && a < argc)
      )
      {  mclx* mx_coarse   =  NULL, *clnext = NULL

      ;  dim dist_new_prev = 0, dist_prev_new = 0
      ;  mclx* clnew = NULL
      ;  mcxbool faith = FALSE
      ;  double inflation = -1.0

      ;  if (subcluster_g)
         mx_coarse
         =     subclusterx_g
            ?  mclxBlockPartition(mxbase, clprev, 50)
            :  mclxBlockUnion(mxbase, clprev)

                  /* have to copy mxbase as mx_coarse is freed.
                   * Even if it were not freed, it is probably transformed.
                  */
      ;  else if (dispatch_g)
         mx_coarse = mclxCopy(mxbase)

      ;  else
         {  mx_coarse = get_coarse(mxbase, clprev, add_transpose)

         ;  if (n_ite == 1)
            {  mclx* cc = clmUGraphComponents(mx_coarse, NULL)   /* fixme; mx_coarse garantueed UD ? */
            ;  n_components = N_COLS(cc)
            ;  mclxFree(&cc)
         ;  }
         }

         if (xfcoarse)
         write_coarse(xfcoarse, mx_coarse)

      ;  get_interface
         (  &mlp
         ,  NULL
         ,  shared->str
         ,  a < argc ? argv[a] : NULL
         ,  mx_coarse
         ,  ALG_CACHE_START
         ,  EXIT_ON_FAIL
         )

      ;  inflation = mlp->mpp->mainInflation
      ;  BIT_OFF(mlp->modes, ALG_DO_SHOW_PID | ALG_DO_SHOW_JURY)

      ;  if ((status = mclAlgorithm(mlp)) == STATUS_FAIL)
         {  mcxErr(me, "failed")
         ;  mcxExit(1)
      ;  }

         cl_coarse = mclAlgParamRelease(mlp, mlp->cl_result)

      ;  if (xfcoarse)
         mclxaWrite(cl_coarse, xfcoarse, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)

      ;  if (dispatch_g || subcluster_g)
         clnext = cl_coarse
      ;  else
            clnext = mclxCompose(clprev, cl_coarse, 0, 0)
         ,  clnext = control_test(clnext, clctrl)
         ,  mclxFree(&cl_coarse)

      ;  clmSJDistance
         (clprev, clnext, NULL, NULL, &dist_prev_new, &dist_new_prev)

      ;  if (dist_prev_new + dist_new_prev)
         {  write_clustering
            (clnext, clprev, xfcone, xfstack, plexprefix, multiplex_idx++, mlp)
         ;  clnew = clnext

         ;  if (subcluster_g || dispatch_g)
            mclxCatPush(&stck_g, clnext, NULL, NULL, mclxCBdomStack, NULL, "dummy-mclcm", n_cls++)
         ;  else
            mclxFree(&clprev)

         ;  clprev = clnew
      ;  }
         else if
         (  N_COLS(clnext) > n_components
         && inflation * iaf > 1.2
         && inflation * iaf < 10
         )
         {  mclxFree(&clnext)
         ;  inflation *= iaf
         ;  mcxTingPrintAfter(shared, " -I %.2f", inflation)
         ;  mcxLog(MCX_LOG_APP, me, "setting inflation to %.2f", inflation)
         ;  faith = TRUE
      ;  }
                                       /* i.e. vanilla mode, contraction */
         else if (!subcluster_g && !dispatch_g)
         {  mclx* cc
         ;  mclxFree(&clnext)

         ;  mclxAddTranspose(mx_coarse, 1.0)
         ;  cc = clmUGraphComponents(mx_coarse, NULL)  

         ;  if (N_COLS(cc) < N_COLS(clprev))
            {  mclx* ccback = mclxCompose(clprev, cc, 0, 0)
            ;  write_clustering
               (ccback, clprev, xfcone, xfstack, plexprefix, multiplex_idx++, NULL)
            ;  mclxFree(&clprev)
            ;  clprev = ccback
            ;  mcxTell(me, "connected components added as root clustering")
         ;  }

            if (root && N_COLS(cc) > 1)
            {  mclx* root =   mclxCartesian
                              (  mclvCanonical(NULL, 1, 0)
                              ,  mclvCopy(NULL, mxbase->dom_cols)
                              ,  1.0
                              )
            ;  write_clustering
               (root, clprev, xfcone, xfstack, plexprefix, multiplex_idx++, NULL)

            ;  mclxFree(&clprev)

            ;  mcxTell(me, "universe added as root clustering")
            ;  clprev = root
            ;  clnew = NULL
         ;  }

            mclxFree(&cc)
      ;  }
         else if (subcluster_g || dispatch_g)
         mclxFree(&clnext)

      ;  mclAlgParamFree(&mlp, TRUE)                        /* frees mx_coarse */

      ;  if (!clnew && !faith)
         {  same = TRUE
         ;  break
      ;  }

         a++

      ;  if (dispatch_g && a == argc)
         break

      ;  n_ite++
   ;  }

      if (same)
      mcxLog(MCX_LOG_MODULE, me, "no further contraction: halting")

   ;  if (dispatch_g)
      integrate_results(&stck_g)
   ;  else if (subcluster_g)
      mclxCatReverse(&stck_g)

   ;  if (dispatch_g || subcluster_g)
      {  dim j
      ;  if (xfstack)
         mclxCatWrite(xfstack, &stck_g, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
      ;  if (xfcone && ! mclxCatConify(&stck_g))
         mclxCatWrite(xfcone, &stck_g, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
      ;  for (j=0;j<stck_g.n_level;j++)
         {  mclxAnnot* an = stck_g.level+j
         ;  mclxFree(&an->mx)
      ;  }
         mcxFree(stck_g.level)
   ;  }

      mcxIOfree(&xfcoarse)
   ;  mcxIOfree(&xfbase)
   ;  mcxIOfree(&xfcone)
   ;  mcxIOfree(&xfstack)

   ;  mcxTingFree(&shared)

   ;  if (!dispatch_g && !subcluster_g)          /* fixme fixme fixme */
      mclxFree(&clprev)

   ;  mclxFree(&mxbase)
   ;  mclvFree(&start_col_sums_g)
   ;  mcxTingFree(&cline)
   ;  helpful_reminder()
   ;  return STATUS_OK
;  }


const char* usagelines[] =
{  NULL
}  ;



