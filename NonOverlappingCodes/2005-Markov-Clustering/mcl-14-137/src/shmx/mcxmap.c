/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011  Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#include <string.h>
#include <stdio.h>

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/app.h"
#include "mcl/interpret.h"

#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/types.h"


enum
{  MY_OPT_IMX
,  MY_OPT_TAB
,  MY_OPT_OUT
,  MY_OPT_MUL
,  MY_OPT_CMUL
,  MY_OPT_RMUL
,  MY_OPT_SHIFT
,  MY_OPT_CSHIFT
,  MY_OPT_RSHIFT
,  MY_OPT_MAP
,  MY_OPT_CMAP
,  MY_OPT_RMAP
,  MY_OPT_MAPI
,  MY_OPT_CMAPI
,  MY_OPT_RMAPI
,  MY_OPT_MAKE_MAP
,  MY_OPT_MAKE_MAPC
,  MY_OPT_MAKE_MAPR
,  MY_OPT_HELP
,  MY_OPT_APROPOS
,  MY_OPT_VERSION
}  ;


const char* me = "mcxmap";
const char* syntax = "Usage: mcxmap [options]";


mcxOptAnchor options[] =
{  {  "-h"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "print this help"
   }
,  {  "--version"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_VERSION
   ,  NULL
   ,  "print version information"
   }
,  {  "--help"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_APROPOS
   ,  NULL
   ,  "print this help"
   }
,  {  "-shift"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SHIFT
   ,  "<num>"
   ,  "shift domain indices by <num>"
   }
,  {  "-cshift"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CSHIFT
   ,  "<num>"
   ,  "shift column indices by <num>"
   }
,  {  "-rshift"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RSHIFT
   ,  "<num>"
   ,  "shift row indices by <num>"
   }
,  {  "-mul"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MUL
   ,  "<num>"
   ,  "multiply domain indices by <num>"
   }
,  {  "-cmul"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CMUL
   ,  "<num>"
   ,  "multiply column indices by <num>"
   }
,  {  "-rmul"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RMUL
   ,  "<num>"
   ,  "multiply row indices by <num>"
   }
,  {  "-map"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAP
   ,  "<fname>"
   ,  "map domain indices"
   }
,  {  "-cmap"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CMAP
   ,  "<fname>"
   ,  "map column indices"
   }
,  {  "-rmap"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RMAP
   ,  "<fname>"
   ,  "map row indices"
   }
,  {  "-mapi"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAPI
   ,  "<fname>"
   ,  "map domain indices with inverse"
   }
,  {  "-cmapi"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CMAPI
   ,  "<fname>"
   ,  "map column indices with inverse"
   }
,  {  "-rmapi"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RMAPI
   ,  "<fname>"
   ,  "map row indices with inverse"
   }
,  {  "-make-map"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAKE_MAP
   ,  "<fname>"
   ,  "canonify domain indices, write map file"
   }
,  {  "-make-mapc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAKE_MAPC
   ,  "<fname>"
   ,  "canonify column indices, write map file"
   }
,  {  "-make-mapr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAKE_MAPR
   ,  "<fname>"
   ,  "canonify row indices, write map file"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "write output to file <fname>"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "use matrix from file <fname>"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "use tab file from <fname>"
   }
,  {  NULL, 0, 0, NULL, NULL  }
}  ;



int main
(  int                  argc
,  const char*          argv[]
)
   {  mcxIO      *xfin        =  mcxIOnew("-", "r")
   ;  mcxIO      *xfout       =  mcxIOnew("-", "w")
   ;  mclMatrix  *mx          =  NULL
   ;  mclx* cmapx = NULL, *rmapx = NULL
   ;  const char* me          =  "mcxmap"
   ;  long        cshift      =  0
   ;  long        rshift      =  0
   ;  long        cmul        =  1
   ;  long        rmul        =  1
   ;  mcxIO*     xf_cannc     =  NULL
   ;  mcxIO*     xf_cannr     =  NULL
   ;  mcxstatus   status      =  STATUS_OK
   ;  mcxbool     invert      =  FALSE
   ;  mcxbool     invertr     =  FALSE
   ;  mcxbool     invertc     =  FALSE
   ;  mcxIO* xf_map_c = NULL, *xf_map_r = NULL, *xf_map = NULL, *xf_tab = NULL

   ;  mcxOption* opts, *opt
   ;  mcxstatus parseStatus = STATUS_OK

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_NO)
   ;  mclx_app_init(stderr)
   
   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)
   ;  opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)

   ;  if (!opts)
      exit(0)

   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch

      ;  switch(anch->id)
         {  case MY_OPT_HELP
         :  case MY_OPT_APROPOS
         :  mcxOptApropos(stdout, me, syntax, 0, 0, options)
         ;  return 0
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  return 0
         ;

            case MY_OPT_IMX
         :  mcxIOnewName(xfin, opt->val)
         ;  break
         ;

            case MY_OPT_OUT
         :  mcxIOnewName(xfout, opt->val)
         ;  break
         ;

            case MY_OPT_MUL
         :  cmul =  atol(opt->val)
         ;  rmul =  cmul
         ;  break
         ;

            case MY_OPT_CMUL
         :  cmul =  atol(opt->val)
         ;  break
         ;

            case MY_OPT_RMUL
         :  rmul =  atol(opt->val)
         ;  break
         ;

            case MY_OPT_SHIFT
         :  cshift =  atol(opt->val)
         ;  rshift =  atol(opt->val)
         ;  break
         ;

            case MY_OPT_CSHIFT
         :  cshift =  atol(opt->val)
         ;  break
         ;

            case MY_OPT_RSHIFT
         :  rshift =  atol(opt->val)
         ;  break
         ;

            case MY_OPT_MAP
         :  xf_map =  mcxIOnew(opt->val, "r")
         ;  invert =  FALSE
         ;  break
         ;

            case MY_OPT_CMAP
         :  invertc  =  FALSE  
         ;  xf_map_c =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_RMAP
         :  invertr  =  FALSE  
         ;  xf_map_r =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_MAPI
         :  invert =  TRUE  
         ;  xf_map =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_CMAPI
         :  invertc  =  TRUE  
         ;  xf_map_c =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_RMAPI
         :  invertr  =  TRUE  
         ;  xf_map_r =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_MAKE_MAP
         :  xf_cannc = mcxIOnew(opt->val, "w")
         ;  xf_cannr = xf_cannc
         ;  break
         ;

            case MY_OPT_MAKE_MAPC
         :  xf_cannc = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_MAKE_MAPR
         :  xf_cannr = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_TAB
         :  xf_tab = mcxIOnew(opt->val, "r")
         ;  break
         ;
         }
      }

                     /* little special case. restructure when it grows */
      if (xf_tab)
      {  mclTab* tab1, *tab2
      ;  if (xf_map)
         {  mcxIOopen(xf_map, EXIT_ON_FAIL)
         ;  cmapx = mclxRead(xf_map, EXIT_ON_FAIL)  
      ;  }
         else
         mcxDie(1, me, "-tab option requires -map option")

      ;  tab1 = mclTabRead(xf_tab, NULL, EXIT_ON_FAIL)
      ;  if ((tab2 = mclTabMap(tab1, cmapx)))
         mclTabWrite(tab2, xfout, NULL, EXIT_ON_FAIL)
       ; else
         mcxDie(1, me, "map file error (subsumption/bijection)")

      ;  return 0
   ;  }

      mx = mclxRead(xfin, EXIT_ON_FAIL)

   ;  if (xf_map)
      {  mcxIOopen(xf_map, EXIT_ON_FAIL)
      ;  cmapx = mclxRead(xf_map, EXIT_ON_FAIL)  
      ;  rmapx = cmapx
   ;  }
      else
      {  if (xf_map_c)
         {  mcxIOopen(xf_map_c, EXIT_ON_FAIL)
         ;  cmapx = mclxRead(xf_map_c, EXIT_ON_FAIL)  
      ;  }
         else if (cshift || cmul > 1)
         cmapx
         =  mclxMakeMap
            (  mclvCopy(NULL, mx->dom_cols)
            ,  mclvMap(NULL, cmul, cshift, mx->dom_cols)
            )
      ;  else if (xf_cannc)      /* fixme slightly flaky interface */
         {  cmapx 
            =  mclxMakeMap
               (  mclvCopy(NULL, mx->dom_cols)
               ,  mclvCanonical(NULL, mx->dom_cols->n_ivps, 1.0)
               )
         ;  mclxWrite(cmapx, xf_cannc, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
      ;  }

         if (xf_map_r)
         {  mcxIOopen(xf_map_r, EXIT_ON_FAIL)
         ;  rmapx = mclxRead(xf_map_r, EXIT_ON_FAIL)  
      ;  }
         else if (rshift || rmul > 1)
         rmapx
         =  mclxMakeMap
            (  mclvCopy(NULL, mx->dom_rows)
            ,  mclvMap(NULL, rmul, rshift, mx->dom_rows)
            )
      ;  else if (xf_cannr)
         {  rmapx 
            =  mclxMakeMap
               (  mclvCopy(NULL, mx->dom_rows)
               ,  mclvCanonical(NULL, mx->dom_rows->n_ivps, 1.0)
               )
         ;  if (xf_cannr != xf_cannc)
            mclxWrite(rmapx, xf_cannr, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
         ;  else if (!mclxIsGraph(mx))
            mcxErr(me, "row map not written but matrix is not a graph")
      ;  }
      }

      if (invert && cmapx && cmapx == rmapx)
      {  mclx* cmapxi = mclxTranspose(cmapx)
      ;  mclxFree(&cmapx)
      ;  cmapx = rmapx = cmapxi
   ;  }
      else
      {  if ((invert || invertr) && rmapx)
         {  mclx* rmapxi = mclxTranspose(rmapx)
         ;  mclxFree(&rmapx)
         ;  rmapx = rmapxi
      ;  }
         if ((invert || invertc) && cmapx)
         {  mclx* cmapxi = mclxTranspose(cmapx)
         ;  mclxFree(&cmapx)
         ;  cmapx = cmapxi
      ;  }
      }

   ;  status = STATUS_FAIL

   ;  do
      {  if (cmapx && mclxMapCols(mx, cmapx))
         break
      ;  if (rmapx && mclxMapRows(mx, rmapx))
         break
      ;  status = STATUS_OK
   ;  }
      while (0)

   ;  if (status)
      {  mcxErr(me, "error, nothing written")
      ;  return 1
   ;  }

      mclxWrite(mx, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
   ;  return 0
;  }


