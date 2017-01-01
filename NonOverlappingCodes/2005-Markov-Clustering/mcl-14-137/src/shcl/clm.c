/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004 Stijn van Dongen
 *   (C) Copyright 2005, 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* TODO
 *    mode-dispatch:
 *       check number of arguments left after exhaust.
 *       apropos should grep out appropriate options.
 *
 *    plugin mode where master C code does not need to be modified.
*/


#include <string.h>
#include <stdio.h>

#include "clm.h"
#include "clmmate.h"
#include "clmclose.h"
#include "clmorder.h"
#include "clmmeet.h"
#include "clmdist.h"
#include "clmimac.h"
#include "clmresidue.h"
#include "clmadjust.h"
#include "clmoptics.h"
#include "clminfo.h"
#include "clminfo2.h"
#include "clmps.h"

#include "impala/matrix.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/compose.h"
#include "impala/ivp.h"
#include "impala/app.h"

#include "clew/clm.h"

#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/ding.h"


const char* clm_syntax =
   "Usage: clm <mode> [--nop] [options] [files]\n"
   "       clm <mode> -h (for mode specific options)"
   ;


enum
{  ID_MATE = 0
,  ID_MEET
,  ID_DIST
,  ID_IMAC
,  ID_INFO
,  ID_INFO2
,  ID_ORDER
,  ID_FRAME
,  ID_CLOSE
,  ID_VOL
,  ID_STABLE
,  ID_DAG
,  ID_FORMAT
,  ID_RESIDUE
,  ID_ADJUST
,  ID_OPTICS
,  ID_ENSTRICT
,  ID_FOLD
,  ID_PS
,  ID_UNUSED
}  ;


/* help function should get the name from the hook callback
 * function in clm_dir.
*/

mcxDispEntry clm_dir[] =
{  {  ID_MATE,    mcxDispHookMate  }
,  {  ID_MEET,    mcxDispHookMeet  }
,  {  ID_VOL,     mcxDispHookVol   }
,  {  ID_STABLE,  mcxDispHookStable}
,  {  ID_DIST,    mcxDispHookDist  }
,  {  ID_INFO,    mcxDispHookInfo  }
,  {  ID_INFO2,   mcxDispHookInfo2 }
,  {  ID_IMAC,    mcxDispHookImac  }
,  {  ID_ORDER,   mcxDispHookOrder }
,  {  ID_CLOSE,   mcxDispHookClose }
,  {  ID_RESIDUE, mcxDispHookResidue }
,  {  ID_ADJUST,  mcxDispHookAdjust }
,  {  ID_OPTICS,  mcxDispHookOptics }
,  {  ID_ENSTRICT, mcxDispHookEnstrict }
,  {  ID_FOLD,    mcxDispHookFold }
,  {  ID_PS,      mcxDispHookPS    }
,  {  -1,         NULL         }
}  ;


mcxOptAnchor clmSharedOptions[] =
{  {  "--version"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_VERSION
   ,  NULL
   ,  "output version information, exit"
   }
,  {  "--test"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_TEST
   ,  NULL
   ,  "test"
   }
,  {  "-progress"
   ,  MCX_OPT_HASARG
   ,  CLM_DISP_PROGRESS
   ,  "<int>"
   ,  "set progress interval size"
   }
,  {  "--progress"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_PROGRESS2
   ,  NULL
   ,  "turn on progress reporting"
   }
,  {  "-debug"
   ,  MCX_OPT_HASARG
   ,  CLM_DISP_DEBUG
   ,  "<int>"
   ,  "set debug level or bits"
   }
,  {  "--debug"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_DEBUG2
   ,  NULL
   ,  "turn on debugging"
   }
,  {  "-set"
   ,  MCX_OPT_HASARG
   ,  CLM_DISP_SET
   ,  "key=val"
   ,  "set key to val in ENV"
   }
,  {  "--nop"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_NOP
   ,  NULL
   ,  "this option has no affect but changing the argument count"
   }
,  {  "-h"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_HELP
   ,  NULL
   ,  "this"
   }
,  {  "--help"
   ,  MCX_OPT_DEFAULT
   ,  CLM_DISP_APROPOS
   ,  NULL
   ,  "this"
   }
,  {  "--amoixa"
   ,  MCX_OPT_HIDDEN
   ,  CLM_DISP_AMOIXA
   ,  NULL
   ,  "show hidden options too"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


mcxbits  mcx_debug_g     =  0;
unsigned mcx_progress_g  =  0;
mcxbool  mcx_test_g      =  FALSE;


mcxstatus sharedArgHandle
(  int optid
,  const char* val
,  mcxDispHook*   hook
,  mcxDispBundle* bundle
)
   {  mcxTing* full_syntax = mcxTingPrint(NULL, "%s %s", "clm", hook->syntax)
   ;  switch(optid)
      {  case CLM_DISP_HELP
      :  case CLM_DISP_APROPOS
      :  case CLM_DISP_AMOIXA
      :  
         mcxOptApropos
         (  stdout
         ,  hook->name
         ,  full_syntax->str
         ,  15
         ,  0
         ,  bundle->disp_shared
         )
      ;  mcxOptApropos
         (  stdout
         ,  hook->name
         ,  NULL
         ,  15
         ,     MCX_OPT_DISPLAY_SKIP
            |  (  optid == CLM_DISP_AMOIXA
               ?  MCX_OPT_DISPLAY_HIDDEN
               :  0
               )
         ,  hook->options
         )
      ;  mcxExit(0)
      ;  break
      ;

         case CLM_DISP_VERSION
      :  bundle->disp_version(hook->name)
      ;  mcxExit(0)
      ;  break
      ;

         case CLM_DISP_NOP
      :  NOTHING
      ;  break
      ;

         case CLM_DISP_TEST
      :  mcx_test_g = TRUE
      ;  break
      ;

         case CLM_DISP_PROGRESS
      :  mcx_progress_g = atoi(val)
      ;  break
      ;

         case CLM_DISP_PROGRESS2
      :  mcx_progress_g = 1
      ;  break
      ;

         case CLM_DISP_DEBUG
      :  mcx_debug_g = atoi(val)
      ;  break
      ;

         case CLM_DISP_DEBUG2
      :  mcx_debug_g = -1u
      ;  break
      ;

         case CLM_DISP_SET
      :  mcxSetenv(val)
      ;  break
      ;

         default
      :  break
      ;
      }
      mcxTingFree(&full_syntax)
   ;  return STATUS_OK
;  }


int main
(  int                  argc
,  const char*          argv[]
)
   {  mcxDispBundle bundle 

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_NO)
   ;  mclx_app_init(stderr)

   ;  bundle.disp_argc     =  argc
   ;  bundle.disp_argv     =  argv
   ;  bundle.disp_name     =  "clm"
   ;  bundle.disp_syntax   =  clm_syntax
   ;  bundle.disp_shared   =  clmSharedOptions
   ;  bundle.n_disp_shared =  sizeof(clmSharedOptions) / sizeof(mcxOptAnchor) -1;
   ;  bundle.disp_version  =  app_report_version
   ;  bundle.disp_table    =  clm_dir
   ;  bundle.shared_handler=  sharedArgHandle

   ;  return mcxDispatch(&bundle)
;  }


