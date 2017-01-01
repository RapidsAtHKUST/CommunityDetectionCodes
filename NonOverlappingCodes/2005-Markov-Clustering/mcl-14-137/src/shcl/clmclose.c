/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010 Stijn van Dongen
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
#include "clmclose.h"

#include "util/io.h"
#include "util/err.h"
#include "util/types.h"
#include "util/opt.h"
#include "util/minmax.h"
#include "util/compile.h"

#include "impala/edge.h"
#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/app.h"
#include "impala/iface.h"

#include "mcl/interpret.h"
#include "mcl/transform.h"

#include "clew/scan.h"
#include "clew/clm.h"


#include "impala/matrix.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/compose.h"
#include "impala/ivp.h"
#include "impala/app.h"
#include "impala/stream.h"

#include "clew/clm.h"

#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"

static const char* me = "clmclose";

enum
{  MY_OPT_IMX = CLM_DISP_UNUSED
,  MY_OPT_ABC
,  MY_OPT_DOMAIN
,  MY_OPT_OUTPUT
,  MY_OPT_READASIS
,  MY_OPT_WRITECC
,  MY_OPT_WRITECOUNT
,  MY_OPT_WRITESIZES
,  MY_OPT_WRITEGRAPH
,  MY_OPT_WRITEGRAPHC
,  MY_OPT_CCBOUND
,  MY_OPT_TABIN
,  MY_OPT_MXOUT
,  MY_OPT_TABOUT
,  MY_OPT_TABXOUT
,  MY_OPT_MAPOUT
,  MY_OPT_TF
,  MY_OPT_CAN
}  ;


mcxOptAnchor closeOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "input matrix file, presumably dumped mcl iterand or dag"
   }
,  {  "-abc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ABC
   ,  "<fname>"
   ,  "specify input using label pairs"
   }
,  {  "--is-undirected"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_READASIS
   ,  NULL
   ,  "use if graph is known to be symmetric (slightly faster)"
   }
,  {  "--write-cc"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITECC
   ,  NULL
   ,  "output cluster/connected-component file"
   }
,  {  "--write-sizes"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITESIZES
   ,  NULL
   ,  "output list of component sizes"
   }
,  {  "--write-count"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITECOUNT
   ,  NULL
   ,  "output number of components"
   }
,  {  "--write-block"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITEGRAPH
   ,  NULL
   ,  "write graph restricted to -dom argument"
   }
,  {  "--write-blockc"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITEGRAPHC
   ,  NULL
   ,  "write the complement of graph restricted to -dom argument"
   }
,  {  "-cc-bound"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CCBOUND
   ,  "<num>"
   ,  "select selected components of size at least <num>"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_TABIN
   ,  "<fname>"
   ,  "read tab file"
   }
,  {  "-write-tab"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_TABOUT
   ,  "<fname>"
   ,  "write tab file of selected domain"
   }
,  {  "-write-tabx"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_TABXOUT
   ,  "<fname>"
   ,  "write tab file of deselected domain"
   }
,  {  "-write-matrix"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_MXOUT
   ,  "<fname>"
   ,  "write matrix of selected domain"
   }
,  {  "-write-map"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_MAPOUT
   ,  "<fname>"
   ,  "write mapping"
   }
,  {  "-dom"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DOMAIN
   ,  "<fname>"
   ,  "input domain file"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TF
   ,  "<tf-spec>"
   ,  "first apply tf-spec to matrix"
   }
,  {  "--canonical"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_CAN
   ,  NULL
   ,  "make result matrix canonical"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
static mcxIO*  xfmx     =  (void*) -1;
static mcxIO*  xfabc    =  (void*) -1;
static mcxIO*  xftabin  =  (void*) -1;
static mcxIO*  xftabout =  (void*) -1;
static mcxIO*  xftabxout=  (void*) -1;
static mcxIO*  xfmapout =  (void*) -1;
static mcxIO*  xfmxout  =  (void*) -1;
static mcxIO*  xfdom    =  (void*) -1;
static mcxTing* tfting  =  (void*) -1;
static dim     ccbound_num  =  -1;
static mcxbool canonical=  -1;
static mcxbool make_symmetric=  -1;
static mcxmode write_mode = -1;


static mcxstatus closeInit
(  void
)
   {  xfout    =  mcxIOnew("-", "w")
   ;  write_mode = MY_OPT_WRITESIZES
   ;  xfmapout =  NULL
   ;  xfmxout  =  NULL
   ;  xftabout =  NULL
   ;  xftabxout=  NULL
   ;  xftabin  =  NULL
   ;  xfmx     =  mcxIOnew("-", "r")
   ;  xfabc    =  NULL
   ;  xfdom    =  NULL
   ;  tfting   =  NULL
   ;  ccbound_num  =  0
   ;  canonical=  FALSE
   ;  make_symmetric =  TRUE
   ;  return STATUS_OK
;  }


static mcxstatus closeArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_IMX
      :  mcxIOnewName(xfmx, val)
      ;  break
      ;

         case MY_OPT_READASIS
      :  make_symmetric = FALSE
      ;  break
      ;

         case MY_OPT_ABC
      :  xfabc = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         case MY_OPT_WRITEGRAPHC
      :  write_mode = MY_OPT_WRITEGRAPHC
      ;  break
      ;

         case MY_OPT_WRITEGRAPH
      :  write_mode = MY_OPT_WRITEGRAPH
      ;  break
      ;

         case MY_OPT_WRITECC
      :  write_mode = MY_OPT_WRITECC
      ;  break
      ;

         case MY_OPT_WRITECOUNT
      :  write_mode = MY_OPT_WRITECOUNT
      ;  break
      ;

         case MY_OPT_WRITESIZES
      :  write_mode = MY_OPT_WRITESIZES
      ;  break
      ;

         case MY_OPT_DOMAIN
      :  xfdom= mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_MAPOUT
      :  xfmapout = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_MXOUT
      :  xfmxout = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_CCBOUND
      :  ccbound_num = atoi(val)
      ;  break
      ;

         case MY_OPT_TABXOUT
      :  xftabxout = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_TABOUT
      :  xftabout = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_TABIN
      :  xftabin = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_CAN
      :  canonical = TRUE
      ;  break
      ;

         case MY_OPT_TF
      :  tfting = mcxTingNew(val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
   ;  }
      return STATUS_OK
;  }


static double mclv_check_ccbound
(  const mclv* vec
,  void* data
)
   {  dim bound = *((dim*) data)
   ;  return vec->n_ivps >= bound ? 1.0 : 0.0
;  }


static mcxstatus closeMain
(  int          argc_unused      cpl__unused
,  const char*  argv_unused[]    cpl__unused
)
   {  mclx *dom =  NULL, *cc = NULL, *ccbound = NULL
   ;  mclx *mx  =  NULL
   ;  mclx *map  =  NULL
   ;  const mclv *ccbound_rows = NULL
   ;  const mclv *ccbound_cols = NULL
   ;  mclTab* tab = NULL
   ;  dim N_start = 0
   ;  dim N_bound = 0

   ;  mclxIOstreamer streamer = { 0 }

   ;  if ((xftabout || xftabxout) && !xftabin)
      mcxDie(1, me, "-write-tab currently requires -tab or -abc")

   ;  if (xftabin)
      tab = streamer.tab_sym_in = mclTabRead(xftabin, NULL, EXIT_ON_FAIL)

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  if (xfabc)
      mx
      =  mclxIOstreamIn
         (  xfabc
         ,     MCLXIO_STREAM_ABC
            |  MCLXIO_STREAM_MIRROR
            |  MCLXIO_STREAM_SYMMETRIC
            |  (tab ? MCLXIO_STREAM_GTAB_RESTRICT : 0)
         ,  NULL
         ,  mclpMergeMax
         ,  &streamer
         ,  EXIT_ON_FAIL
         )
   ;  else
      mx = mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)

   ;  dom =    xfdom
            ?  mclxRead(xfdom, EXIT_ON_FAIL)
            :  NULL

   ;  if (write_mode == MY_OPT_WRITEGRAPH && !dom)
      mcxDie(1, me, "--write-graph requires -dom option")
   ;  else if (write_mode == MY_OPT_WRITEGRAPHC && !dom)
      mcxDie(1, me, "--write-graphc requires -dom option")
   ;  else if (dom && !MCLD_EQUAL(dom->dom_rows, mx->dom_cols))
      mcxDie(1, me, "domains not equal")

   ;  N_start = N_ROWS(mx)

   ;  if (xftabout || xftabxout)
      {  if (streamer.tab_sym_out)
         tab = streamer.tab_sym_out
      ;  else if (streamer.tab_sym_in)
         tab = streamer.tab_sym_in
      ;  else
         mcxDie(1, me, "no tab read, no tab created")
   ;  }

      if (tfting)
      {  mclgTF* tfar = mclgTFparse(NULL, tfting)
      ;  if (!tfar)
         mcxDie(1, me, "errors in tf-spec")
      ;  mclgTFexec(mx, tfar)
   ;  }

      cc = make_symmetric ? clmComponents(mx, dom) : clmUGraphComponents(mx, dom)

                              /*
                               * thin out domain based on cc
                              */
   ;  if (ccbound_num)
      {  ccbound_cols = mclxColSelect(cc, mclv_check_ccbound, &ccbound_num)
      ;  ccbound_rows = mclgUnionv(cc, ccbound_cols, NULL, SCRATCH_READY, NULL)
   ;  }
      else
         ccbound_cols = cc->dom_cols
      ,  ccbound_rows = cc->dom_rows

   ;  N_bound = ccbound_rows->n_ivps

   ;  if
      (  canonical
      && (! (  map
            =  mclxMakeMap
               (  mclvClone(ccbound_rows)
               ,  mclvCanonical(NULL, ccbound_rows->n_ivps, 1.0)
               )
            )
         )
      )
      mcxDie(1, me, "cannot make a map")

   ;  if (N_bound < N_start)
      ccbound = mclxSub(cc, ccbound_cols, ccbound_rows)
   ;  else
      ccbound = cc

   ;  if (xfmxout)
      {  if (N_bound < N_start)      /* thin out matrix */
         {  mclx* sub = mclxSub(mx, ccbound_rows, ccbound_rows)
         ;  mclxFree(&mx)
         ;  mx = sub
      ;  }

         if (map)
         {  if (mclxMapRows(mx, map))
            mcxDie(1, me, "cannot map rows")

         ;  if (mclxMapCols(mx, map))
            mcxDie(1, me, "cannot map cols")
      ;  }
         mclxWrite(mx, xfmxout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  }

      if (xftabxout)
      {  mclv* deselect = mcldMinus(tab->domain, ccbound->dom_rows, NULL)
      ;  if (canonical)
         mcxErr(me, "--canonical and writing tab not yet implemented. beerware.")
      ;  else
         mclTabWrite(tab, xftabxout, deselect, RETURN_ON_FAIL)
      ;  mclvFree(&deselect)
   ;  }


      if (xftabout)
      {  mclTab* tabsel = mclTabSelect(tab, ccbound->dom_rows), *tabout
      ;  if (map)
            tabout = mclTabMap(tabsel, map)
         ,  mclTabFree(&tabsel)
      ;  else
         tabout = tabsel
      ;  if (!tabout)
         mcxDie(1, me, "no tab, baton")
      ;  mclTabWrite(tabout, xftabout, NULL, RETURN_ON_FAIL)

      ;  mclTabFree(&tabout)
   ;  }

      if (map)
      {  if (mclxMapRows(ccbound, map))
         mcxDie(1, me, "cannot map rows")

      ;  if (mclxMapCols(ccbound, NULL))
         mcxDie(1, me, "cannot map cols")
   ;  }

      if (write_mode == MY_OPT_WRITEGRAPH)
      {  mclx* bl = mclxBlockUnion(mx, cc)
      ;  mclxWrite(bl, xfout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
      ;  mclxFree(&mx)
      ;  mx = bl
   ;  }
      else if (write_mode == MY_OPT_WRITEGRAPHC)
      {  mclx* bl = mclxBlocksC(mx, cc)
      ;  mclxWrite(bl, xfout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
      ;  mclxFree(&mx)
      ;  mx = bl
   ;  }
      if (write_mode == MY_OPT_WRITECC)
      {  if (streamer.tab_sym_out)
         {  mclxIOdumper dumper
         ;  mclxIOdumpSet(&dumper, MCLX_DUMP_LINES | MCLX_DUMP_NOLEAD, NULL, NULL, NULL)
         ;  mclxIOdump
            (  ccbound
            ,  xfout
            ,  &dumper
            ,  NULL
            ,  streamer.tab_sym_out
            ,  MCLXIO_VALUE_NONE
            ,  EXIT_ON_FAIL
            )
      ;  }
         else
         mclxaWrite(ccbound, xfout, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
   ;  }

      else if (write_mode == MY_OPT_WRITECOUNT)
      fprintf(xfout->fp, "%lu\n", (ulong) N_COLS(ccbound))

   ;  else if (write_mode == MY_OPT_WRITESIZES)
      {  dim j 
      ;  for (j=0;j<N_COLS(ccbound);j++)
         {  if (j)
            fprintf(xfout->fp, " %lu", (ulong) ccbound->cols[j].n_ivps)
         ;  else
            fprintf(xfout->fp, "%lu", (ulong) ccbound->cols[j].n_ivps)
      ;  }
         fputc('\n', xfout->fp)
   ;  }

      if (xfmapout && map)
      mclxaWrite(map, xfmapout, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)

   ;  mcxIOfree(&xfmx)
   ;  mcxIOfree(&xfabc)
   ;  mcxIOfree(&xfout)

   ;  mclTabFree(&(streamer.tab_sym_out))

   ;  mclxFree(&mx)
   ;  mclxFree(&cc)
   ;  mclxFree(&dom)
   ;  return STATUS_OK
;  }


mcxDispHook* mcxDispHookClose
(  void
)
   {  static mcxDispHook closeEntry
   =  {  "close"
      ,  "close [options] -imx <mx file>"
      ,  closeOptions
      ,  sizeof(closeOptions)/sizeof(mcxOptAnchor) - 1
      ,  closeArgHandle
      ,  closeInit
      ,  closeMain
      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &closeEntry
;  }


