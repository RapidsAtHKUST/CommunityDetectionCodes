/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
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
#include <time.h>

#include "impala/ivp.h"
#include "impala/iface.h"
#include "impala/io.h"
#include "impala/version.h"
#include "impala/compose.h"

#include "util/ting.h"
#include "util/equate.h"
#include "util/io.h"
#include "util/types.h"
#include "util/minmax.h"
#include "util/tok.h"
#include "util/opt.h"
#include "util/err.h"

#include "proc.h"
#include "procinit.h"

#define CHB(a,b,c,d,e,f,g) mcxOptCheckBounds("mclInit", a, b, c, d, e, f, g)

static const char* me = "mclInit";

enum
{  PROC_OPT_INITLENGTH = 1000
,  PROC_OPT_MAINLENGTH
,  PROC_OPT_INITINFLATION
,  PROC_OPT_MAININFLATION
,  PROC_OPT_SCHEME
,  PROC_OPT_RESOURCE
,  PROC_OPT_RPRUNE
,  PROC_OPT_SPARSE
,  PROC_OPT_PARTITION_SELECT
,  PROC_OPT_PARTITION_P
,  PROC_OPT_SKID
                        ,  PROC_OPT_ETHREADS
,  PROC_OPT_SHOW        =  PROC_OPT_ETHREADS + 2
,  PROC_OPT_VERBOSITY
,                          PROC_OPT_SILENCE
,  PROC_OPT_PRUNE       =  PROC_OPT_SILENCE + 2
,  PROC_OPT_PPRUNE
,  PROC_OPT_RECOVER
,  PROC_OPT_SELECT
                        ,  PROC_OPT_PCT
,  PROC_OPT_WARNFACTOR  =  PROC_OPT_PCT + 2
                        ,  PROC_OPT_WARNPCT
,  PROC_OPT_DUMPSTEM    =  PROC_OPT_WARNPCT + 2
,  PROC_OPT_DUMP
,  PROC_OPT_DUMPINTERVAL
                        ,  PROC_OPT_DUMPMODULO
,  PROC_OPT_DEVEL       =  PROC_OPT_DUMPMODULO + 2
,  PROC_OPT_THREADS
,  PROC_OPT_ITHREADS
,  PROC_OPT_WEIGHT_MAXVAL
,  PROC_OPT_WEIGHT_SELFVAL

}  ;


mcxOptAnchor mclProcOptions[] =
{  {  "--show"
   ,  MCX_OPT_DEFAULT
   ,  PROC_OPT_SHOW
   ,  NULL
   ,  "(small graphs only [#<20]) dump iterands to *screen*"
   }
,  {  "-l"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_INITLENGTH
   ,  "<int>"
   ,  "length of initial run (default 0)"
   }
,  {  "-L"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_MAINLENGTH
   ,  "<int>"
   ,  "length of main run (default unbounded)"
   }
,  {  "-i"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_INITINFLATION
   ,  "<num>"
   ,  "initial inflation value (default 2.0)"
   }
,  {  "-I"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_MAININFLATION
   ,  "<num>"
   ,  "\tM!\tDmain inflation value (default 2.0)"
   }
,  {  "-v"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_VERBOSITY
   ,  "{pruning|explain|clusters|all}"
   ,  "mode verbose"
   }
,  {  "-V"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_SILENCE
   ,  "{pruning|explain|clusters|all}"
   ,  "mode silent"
   }
,  {  "-devel"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_DEVEL
   ,  "<int>"
   ,  "development lever"
   }
,  {  "-t"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_THREADS
   ,  "<int>"
   ,  "thread number, inflation and expansion"
   }
,  {  "-ti"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_ITHREADS
   ,  "<int>"
   ,  "inflation thread number"
   }
,  {  "-te"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_ETHREADS
   ,  "<int>"
   ,  "\tM!\tDexpansion thread number, use with multiple CPUs"
   }
,  {  "-p"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_PRUNE
   ,  "<num>"
   ,  "the rigid pruning threshold"
   }
,  {  "-P"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_PPRUNE
   ,  "<int>"
   ,  "(inverted) rigid pruning threshold (cf -z)"
   }
,  {  "-S"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_SELECT
   ,  "<int>"
   ,  "select down to <int> entries if needed"
   }
,  {  "-R"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_RECOVER
   ,  "<int>"
   ,  "recover to maximally <int> entries if needed"
   }
,  {  "-pct"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_PCT
   ,  "<pct>"
   ,  "try recovery if mass is less than <pct>"
   }
,  {  "-scheme"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_SCHEME
   ,  "<int>"
   ,  "\tM!\tDuse a preset resource scheme (cf --show-schemes)"
   }
,  {  "-resource"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_RESOURCE
   ,  "<int>"
   ,  "\tM!\tDallow <int> neighbours throughout computation"
   }
,  {  "-sparse"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_SPARSE
   ,  "<num>"
   ,  "estimated sparse matrix-vector overhead per summand (default 10)"
   }
,  {  "--partition-selection"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  PROC_OPT_PARTITION_SELECT
   ,  NULL
   ,  "use partition selection"
   }
,  {  "-partition-p"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_PARTITION_P
   ,  "<int>"
   ,  "use expensive pivot search while N gq <int>"
   }
,  {  "-Q"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_RPRUNE
   ,  "<int>"
   ,  "use inital pruning cutoff <MAX>/<int>"
   }
,  {  "-skid"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_SKID
   ,  "<int>"
   ,  "use a preset (cheap!) resource scheme (cf --show-skid)"
   }
,  {  "-wself"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_WEIGHT_SELFVAL
   ,  "<num>"
   ,  "intermediate iterand interpretation option"
   }
,  {  "-wmax"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  PROC_OPT_WEIGHT_MAXVAL
   ,  "<num>"
   ,  "intermediate iterand interpretation option"
   }
,  {  "-warn-pct"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_WARNPCT
   ,  "<pct>"
   ,  "warn if pruning reduces mass to <pct> weight"
   }
,  {  "-warn-factor"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_WARNFACTOR
   ,  "<int>"
   ,  "warn if pruning reduces entry count by <int>"
   }
,  {  "-dump"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_DUMP
   ,  "<mode>"
   ,  "<mode> in chr|ite|cls|dag (cf manual page)"
   }
,  {  "-dump-stem"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_DUMPSTEM
   ,  "<str>"
   ,  "use <str> to construct dump (file) names"
   }
,  {  "-dump-interval"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_DUMPINTERVAL
   ,  "<int>:<int>"
   ,  "only dump for iterand indices in this interval"
   }
,  {  "-dump-modulo"
   ,  MCX_OPT_HASARG
   ,  PROC_OPT_DUMPMODULO
   ,  "<int>"
   ,  "only dump if the iterand index modulo <int> vanishes"
   }
,  {  NULL
   ,  0
   ,  0
   ,  NULL
   ,  NULL
   }
}  ;

static int dir_scheme[7][4]
=  
   {  {  3000,  400,  500, 90 }
   ,  {  4000,  500,  600, 90 }
   ,  {  5000,  600,  700, 90 }
   ,  {  6000,  700,  800, 90 }
   ,  {  7000,  800,  900, 90 }
   ,  { 10000, 1100, 1400, 90 }
   ,  { 10000, 1200, 1600, 90 }
   }  ;

static int dir_skid[7][4]
=  
   {  {   100,   50,   70, 30 }
   ,  {   200,   60,  100, 35 }
   ,  {   300,   70,  100, 40 }
   ,  {   400,   80,  100, 45 }
   ,  {   600,  100,  150, 50 }
   ,  {   800,  150,  200, 55 }
   ,  {  1000,  200,  250, 60 }
   }  ;

static int           n_prune     =  -1;
static int           n_select    =  -1;
static int           n_recover   =  -1;
static int           n_scheme    =  -1;
static int           user_scheme =   0;
static int           n_pct       =  -1;

void makeSettings
(  mclExpandParam* mxp
)  ;

void  mclSetProgress
(  int n_nodes
,  mclProcParam* mpp
)
   {  mclExpandParam *mxp = mpp->mxp

   ;  if (mxp->vector_progression)
      {  if (mxp->vector_progression > 0)
         mxp->vector_progression
         =  MCX_MAX(1 + (n_nodes -1)/mxp->vector_progression, 1)
      ;  else
         mxp->vector_progression = -mxp->vector_progression
   ;  }
   }


mcxstatus mclProcessInit
(  const mcxOption*  opts
,  mcxHash*          myOpts
,  mclProcParam*     mpp
)
   {  int               i        =  0
   ;  float             f        =  0.0
   ;  float             f_0      =  0.0
   ;  int               i_0      =  0
   ;  int               i_1      =  1
   ;  int               i_7      =  7
   ;  int               i_100    =  100
   ;  float             f_e1     =  1e-1
   ;  float             f_30     =  30.0
   ;  mclExpandParam    *mxp     =  mpp->mxp
   ;  const mcxOption*  opt

   ;  mcxOptPrintDigits  =  1

   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = mcxOptFind(opt->anch->tag, myOpts)
      ;  mcxbool  vok = TRUE            /* value ok (not illegal) */
      ;  mcxbool  verbosity
      ;  mcxbits  bit = 0
      ;  const char* arg

      ;  if (!anch)     /* not in myOpts */
         continue

      ;  switch(anch->id)
         {  case PROC_OPT_SHOW
         :  mpp->printMatrix  =  TRUE
         ;  break
         ;

            case PROC_OPT_INITINFLATION
         :  f = atof(opt->val)
         ;  if (CHB(anch->tag, 'f', &f, fltGq, &f_e1, fltLq, &f_30))
            mpp->initInflation = f
         ;  break
         ;

            case PROC_OPT_MAININFLATION
         :  f =  atof(opt->val)
         ;  if (CHB(anch->tag, 'f', &f, fltGt, &f_0, fltLq, &f_30))
            mpp->mainInflation = f
         ;  break
         ;

            case PROC_OPT_VERBOSITY
         :  case PROC_OPT_SILENCE
         :  verbosity = anch->id  == PROC_OPT_VERBOSITY ? TRUE : FALSE
         ;  arg = opt->val

         ;  if (strstr(arg, "cls"))
            bit |= XPNVB_CLUSTERS
         ;  if (strstr(arg, "all"))
            {  bit = ~0
            ;  if (!verbosity)
               mcxLogLevelSetByString("x")
         ;  }

            if (!bit)
            mcxWarn(me, "no match in verbosity string <%s>", opt->val)

         ;  if (verbosity)
            BIT_ON(mxp->verbosity, bit)
         ;  else
            BIT_OFF(mxp->verbosity, bit)
         ;  break
         ;

            case PROC_OPT_INITLENGTH
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok) mpp->initLoopLength = i
         ;  break
         ;

            case PROC_OPT_MAINLENGTH
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok) mpp->mainLoopLength = i
         ;  break
         ;

            case PROC_OPT_THREADS
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok)
            {  mxp->n_ethreads = mclxComposeSetThreadCount(i)
            ;  mpp->n_ithreads = mclxComposeSetThreadCount(i)
         ;  }
            break
         ;

            case PROC_OPT_ITHREADS
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok)
            mpp->n_ithreads = mclxComposeSetThreadCount(i)
         ;  break
         ;

            case PROC_OPT_ETHREADS
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok)
            mxp->n_ethreads = mclxComposeSetThreadCount(i)
         ;  break
         ;

            case PROC_OPT_PPRUNE
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok)
               n_prune =  i
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_PRUNE
         :  f = atof(opt->val)
         ;  vok = CHB(anch->tag, 'f', &f, fltGq, &f_0, fltLq, &f_e1)
         ;  if (vok)
               n_prune = f ? (int) (1.0 / f) : 0
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_WARNFACTOR
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok) mxp->warn_factor =  i
         ;  break
         ;

            case PROC_OPT_WARNPCT
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, intLt, &i_100)
         ;  if (vok) mxp->warn_pct  =  ((double) i) / 100.0
         ;  break
         ;

            case PROC_OPT_WEIGHT_MAXVAL
         :  mpp->ipp->w_maxval = atof(opt->val)
         ;  break
         ;

            case PROC_OPT_WEIGHT_SELFVAL
         :  mpp->ipp->w_selfval = atof(opt->val)
         ;  break
         ;

            case PROC_OPT_SKID
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_1, intLq, &i_7)
         ;  if (vok)
            {  n_scheme    =  i-1
            ;  n_prune     =  dir_skid[n_scheme][0]
            ;  n_select    =  dir_skid[n_scheme][1]
            ;  n_recover   =  dir_skid[n_scheme][2]
            ;  n_pct       =  dir_skid[n_scheme][3]
            ;  mxp->scheme =  i
         ;  }
            break
         ;

            case PROC_OPT_SCHEME
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_1, intLq, &i_7)
         ;  if (vok)
            {  n_scheme    =  i-1
            ;  n_prune     =  dir_scheme[n_scheme][0]
            ;  n_select    =  dir_scheme[n_scheme][1]
            ;  n_recover   =  dir_scheme[n_scheme][2]
            ;  n_pct       =  dir_scheme[n_scheme][3]
            ;  mxp->scheme =  i
         ;  }
            break
         ;

            case PROC_OPT_PCT
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, intLt, &i_100)
         ;  if (vok)
               n_pct =  i
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_RECOVER
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok)
               n_recover   =  i
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_RPRUNE
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_1, NULL, NULL)
         ;  if (vok)
               n_prune  = i
            ,  mxp->implementation |= MCL_USE_RPRUNE
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_RESOURCE
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_1, NULL, NULL)
         ;  if (vok)
               n_select =  i
            ,  n_recover =  i
            ,  n_prune = n_prune >= 0 ? n_prune : 10 * i
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_SELECT
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_0, NULL, NULL)
         ;  if (vok)
               n_select =  i
            ,  user_scheme = 1
         ;  break
         ;

            case PROC_OPT_SPARSE
         :  mxp->sparse_trigger = atoi(opt->val)
         ;  break
         ;

            case PROC_OPT_PARTITION_SELECT
         :  mxp->implementation |= MCL_USE_PARTITION_SELECTION
         ;  break
         ;

            case PROC_OPT_PARTITION_P
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_7, NULL, NULL)
         ;  if (vok)
            mxp->partition_pivot_sort_n = i
         ;  mxp->implementation |= MCL_USE_PARTITION_SELECTION
         ;  break
         ;

            case PROC_OPT_DEVEL
         :  mpp->devel = atoi(opt->val)
         ;  break
         ;

            case PROC_OPT_DUMPSTEM
         :  mcxTingWrite(mpp->dump_stem, opt->val)
         ;  break
         ;

            case PROC_OPT_DUMP
         :  arg = opt->val
         ;  if (strstr(arg, "ite"))
            BIT_ON(mpp->dumping, MCPVB_ITE)
         ;  if (strstr(arg, "cls"))
            BIT_ON(mpp->dumping, MCPVB_CLUSTERS)
         ;  if (strstr(arg, "dag"))
            BIT_ON(mpp->dumping, MCPVB_DAG)
         ;  if (strstr(arg, "lines"))
            BIT_ON(mpp->dumping, MCPVB_LINES)
         ;  if (strstr(arg, "cat"))
            BIT_ON(mpp->dumping, MCPVB_CAT)
         ;  if (strstr(arg, "labels"))
            BIT_ON(mpp->dumping, MCPVB_TAB)
         ;  break
         ;

            case PROC_OPT_DUMPINTERVAL
         :  if (!strcmp(opt->val, "all"))
            mpp->dump_bound = 1000
         ;  else if (sscanf(opt->val,"%d:%d",&mpp->dump_offset,&mpp->dump_bound)!=2)
            {  mcxErr
               (  me
               ,  "flag -dump-interval expects i:j format, j=0 denoting infinity"
               )
            ;  vok = FALSE
            /* hierverder mq, no bound checking */
         ;  }
            break
         ;

            case PROC_OPT_DUMPMODULO
         :  i = atoi(opt->val)
         ;  vok = CHB(anch->tag, 'i', &i, intGq, &i_1, NULL, NULL)
         ;  if (vok) mpp->dump_modulo = i
         ;  break
         ;

         }

         if (vok != TRUE)            
         return STATUS_FAIL
   ;  }

  /*
   * this does the scheme thing
  */
      makeSettings(mxp)

   ;  return STATUS_OK
;  }


void mclShowSchemes
(  mcxbool print_skid
)
   {  int* sch = print_skid ? dir_skid[0] : dir_scheme[0]
   ;  int i
   ;  fprintf
      (  stdout
      ,  "%20s%15s%15s%15s\n"  
      ,  "Pruning"
      ,  "Selection"
      ,  "Recovery"
      ,  "  Recover percentage"
      )
   ;  for (i=0;i<7;i++)
      fprintf
      (  stdout
      ,  "Scheme %1d%12d%15d%15d%15d\n"
      ,  i+1
      ,  sch[4*i+0]
      ,  sch[4*i+1]
      ,  sch[4*i+2]
      ,  sch[4*i+3]
      )
;  }


void makeSettings
(  mclExpandParam* mxp
)
   {  int s = mxp->scheme-1
   ;  mxp->num_prune    =  n_prune  >= 0 ?  n_prune :  dir_scheme[s][0]

   ;  mxp->precision    =  mxp->num_prune ?  0.99999 / mxp->num_prune :  0.0

   ;  mxp->num_select   =  n_select < 0  ?  dir_scheme[s][1] :  n_select
   ;  mxp->num_recover  =  n_recover< 0  ?  dir_scheme[s][2] :  n_recover
   ;  mxp->pct          =  n_pct    < 0  ?  dir_scheme[s][3] :  n_pct

   ;  if (user_scheme)
      mxp->scheme = 0          /* this interfaces to alg.c. yesitisugly */

   ;  mxp->pct         /=  100.0
;  }


void mclShowSettings
(  FILE* fp
,  mclProcParam* mpp
,  mcxbool user
)
   {  mclIvp ivps[10]
   ;  mclExpandParam *mxp = mpp->mxp

   ;  if (user)
      {  fprintf(fp, "[mcl] cell size: %u\n", (unsigned) (sizeof(ivps)/10))
      ;  fprintf
         (  fp
         ,  "[mcl] cell contents: "
            IVP_NUM_TYPE
            " and "
            IVP_VAL_TYPE
            "\n"
         )
      ;  fprintf(fp, "[mcl] largest index allowed: %ld\n", (long) PNUM_MAX)
      ;  fprintf(fp, "[mcl] smallest index allowed: 0\n")
   ;  }

      fprintf
      (  fp , "%-40s%8lu%8s%s\n"
            ,  "Prune number"
            ,  (ulong) mxp->num_prune
            ,  ""
            ,  "[-P n]"
      )

   ;  fprintf
      (  fp ,  "%-40s%8lu%8s%s\n"
            ,  "Selection number"
            ,  (ulong) mxp->num_select
            ,  ""
            , "[-S n]"
      )

   ;  fprintf
      (  fp ,  "%-40s%8lu%8s%s\n"
            ,  "Recovery number"
            ,  (ulong) mxp->num_recover
            ,  ""
            ,  "[-R n]"
      )

   ;  fprintf
      (  fp ,  "%-40s%8d%8s%s\n"
            ,  "Recovery percentage"
            ,  (int) (100*mxp->pct+0.5)
            ,  ""
            ,  "[-pct n]"
      )

   ;  if (user) fprintf
      (  fp ,  "%-40s%8d%8s%s\n"
            ,  "warn-pct"
            ,  (int) ((100.0 * mxp->warn_pct) + 0.5)
            ,  ""
            ,  "[-warn-pct k]"
      )

   ;  if (user) fprintf
      (  fp ,  "%-40s%8d%8s%s\n"
            ,  "warn-factor"
            ,  mxp->warn_factor
            ,  ""
            ,  "[-warn-factor k]"
      )

   ;  if (user) fprintf
      (  fp ,  "%-40s%8s%8s%s\n"
            ,  "dumpstem"
            ,  mpp->dump_stem->str
            ,  ""
            ,  "[-dump-stem str]"
      )

   ;  if (user || mpp->initLoopLength) fprintf
      (  fp ,  "%-40s%8d%8s%s\n"
            ,  "Initial loop length"
            ,  mpp->initLoopLength
            ,  ""
            ,  "[-l n]"
      )

   ;  fprintf
      (  fp ,  "%-40s%8d%8s%s\n"
            ,  "Main loop length"
            ,  mpp->mainLoopLength
            ,  ""
            ,  "[-L n]"
      )

   ;  if (user || mpp->initLoopLength) fprintf
      (  fp ,  "%-40s%10.1f%6s%s\n"
            ,  "Initial inflation"
            ,  mpp->initInflation
            ,  ""
            ,  "[-i f]"
      )

   ;  fprintf
      (  fp ,  "%-40s%10.1f%6s%s\n"
            ,  "Main inflation"
            ,  mpp->mainInflation
            ,  ""
            ,  "[-I f]"
      )
;  }


void mclProcOptionsInit
(  void
)
   {  mcxOptAnchorSortById
      (mclProcOptions, sizeof(mclProcOptions)/sizeof(mcxOptAnchor) -1)
;  }

