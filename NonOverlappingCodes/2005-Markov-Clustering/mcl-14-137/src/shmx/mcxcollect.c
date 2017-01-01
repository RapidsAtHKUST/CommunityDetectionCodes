/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013, 2014  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* fixme
 *    with paste check nr of columns
 *    the code internally uses convention for 'columns' member
 *       e.g. first in array is special (may contain column header)
 *       and !paste + two-column is special.
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

#include "mcx.h"

#include "impala/io.h"
#include "impala/matrix.h"
#include "impala/tab.h"
#include "impala/stream.h"

#include "util/types.h"
#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"

#include "gryphon/path.h"

const char* me = "mcx collect";


enum
{  MY_OPT_OUT = MCX_DISP_UNUSED
,  MY_OPT_TAB
,  MY_OPT_SUMMARY
,  MY_OPT_2COLUMN
,  MY_OPT_PASTE
,  MY_OPT_MATRIX_ADD
,  MY_OPT_MATRIX_MAX
,  MY_OPT_TRANSFORM
,  MY_OPT_NO_HEADER
}  ;


mcxOptAnchor collectOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "specify tab file to map collect indices to labels, if appropriate"
   }
,  {  "--summary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SUMMARY
   ,  NULL
   ,  "as final step output maximum, median, minimum, and average"
   }
,  {  "--add-column"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_2COLUMN
   ,  NULL
   ,  "add two-column files (currently mcx diameter/ctty-derived)"
   }
,  {  "--add-matrix"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_MATRIX_ADD
   ,  NULL
   ,  "add matrix/network files"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to matrix values"
   }
,  {  "--max-matrix"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_MATRIX_MAX
   ,  NULL
   ,  "combine matrix/network files taking max"
   }
,  {  "--paste"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_PASTE
   ,  NULL
   ,  "paste columns"
   }
,  {  "--no-header"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_NO_HEADER
   ,  NULL
   ,  "files do not contain column headers"
   }
,  {  NULL, 0, MCX_DISP_UNUSED, NULL, NULL }
}  ;

static   mcxIO* xftab_g       =   (void*) -1;
static   mcxmode  summary_g   =   -1;
static   mcxbool  header_g    =   -1;
static   mcxmode  collect_g   =   -1;
static   const char*  out_g   =   (void*) -1;
static   double (*fltop_g)(pval, pval) = (void*) -1;
static   mcxTing* transform_spec = (void*) -1;
static   mclgTF* transform  =  (void*) -1;

static mcxstatus collectInit
(  void
)
   {  xftab_g        =  NULL
   ;  summary_g      =  0
   ;  collect_g      =  0
   ;  header_g       =  TRUE
   ;  out_g          =  "-"
   ;  fltop_g        =  fltAdd
   ;  transform_spec =  NULL
   ;  transform      =  NULL
   ;  return STATUS_OK
;  }


static mcxstatus collectArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_TAB
      :  xftab_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_OUT
      :  out_g = val
      ;  break
      ;

         case MY_OPT_PASTE
      :  collect_g = 'p'
      ;  break
      ;

         case MY_OPT_NO_HEADER
      :  header_g = FALSE
      ;  break
      ;

         case MY_OPT_TRANSFORM
      :  transform_spec = mcxTingNew(val)
      ;  break
      ;

         case MY_OPT_MATRIX_MAX
      :  collect_g = 'm'
      ;  fltop_g = fltMax
      ;  break
      ;

         case MY_OPT_MATRIX_ADD
      :  collect_g = 'm'
      ;  break
      ;

         case MY_OPT_2COLUMN
      :  collect_g = 'c'
      ;  break
      ;

         case MY_OPT_SUMMARY
      :  summary_g = 1
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
      }
   ;  return STATUS_OK
;  }


typedef struct
{  const char* label
;  double      val
;  mcxTing*    columns
;
}  aggr        ;


int aggr_cmp_val(const void* a1, const void* a2)
   {  const aggr* ag1 = a1, *ag2 = a2
   ;  return ag1->val > ag2->val ? 1 : ag1->val < ag2->val ? -1 : 0
;  }


static dim do_a_file
(  aggr** collectpp
,  const char* fname
,  dim collect_n
)
   {  mcxIO* xf = mcxIOnew(fname, "r")
   ;  mcxTing* buf = mcxTingEmpty(NULL, 100)
   ;  mcxTing* lbl = mcxTingEmpty(NULL, 100)
   ;  mcxstatus status = STATUS_OK
   ;  aggr* collect = *collectpp
   ;  dim ct = 0, collect_alloc = 0

   ;  if (!collect_n)
         collect_alloc = 100
      ,  collect = mcxNAlloc(collect_alloc, sizeof collect[0], NULL, EXIT_ON_FAIL)

   ;  mcxIOopen(xf, EXIT_ON_FAIL)

   ;  while (STATUS_OK == (status = mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)))
      {  double val
      ;  const char* tabchar = NULL
      ;  mcxbool get_header = collect_g != 'p' && !ct ? TRUE : FALSE

      ;  mcxTingEnsure(lbl, buf->len)

               /* if header_g && !ct && !paste create/check label */
               /* body of this while loop does too many things, refactor */
      ;  if (collect_g == 'p' || get_header)
         {  if (!(tabchar = strchr(buf->str, '\t')))
            mcxDie(1, me, "paste error at line %d file %s (no tab)", (int) xf->lc, fname)
         ;  mcxTingNWrite(lbl, buf->str, tabchar - buf->str)
      ;  }
         else
         {  if (2 != sscanf(buf->str, "%s%lg", lbl->str, &val))
            mcxDie(1, me, "parse error at line %d file %s", (int) xf->lc, fname)
         ;  lbl->len = strlen(lbl->str)
      ;  }

         if (!collect_n)
         {  if (ct >= collect_alloc)
            {  dim collect_realloc = collect_alloc * 1.44
            ;  collect = mcxNRealloc(collect, collect_realloc, collect_alloc, sizeof collect[0], NULL, EXIT_ON_FAIL)
            ;  collect_alloc = collect_realloc
         ;  }
            collect[ct].label = mcxTingStr(lbl)
         ;  collect[ct].val = collect_g == 'p' || get_header ? 0.0 : val
         ;  collect[ct].columns
            =      collect_g == 'p' || get_header
                ?  mcxTingNew(tabchar + (get_header ? 1 : 0))
                :  NULL
      ;  }
         else
         {  if (ct >= collect_n)
            mcxDie(1, me, "additional lines in file %s", fname)
         ;  if (strcmp(collect[ct].label, lbl->str))
            mcxDie
            (  1
            ,  me
            ,  "label conflict %s/%s at line %d in file %s"
            ,  collect[ct].label
            ,  lbl->str
            ,  (int) xf->lc, fname
            )
         ;  if (get_header)            /* only need to check identity */
            {  if (strcmp(tabchar+1, collect[ct].columns->str))
               mcxDie(1, me, "different columns <%s> and <%s>", collect[ct].columns->str, tabchar+1)
         ;  }
            else if (collect_g == 'p')          /* tack it on */
            mcxTingNAppend(collect[ct].columns, tabchar, buf->len - lbl->len)
         ;  else
            collect[ct].val += val 
      ;  }
         ct++
   ;  }

      if (collect_n)
      {  if (ct != collect_n)
         mcxDie(1, me, "not enough lines in file %s", fname)
   ;  }
      else
      {  if (!ct)
         mcxDie(1, me, "empty file(s)")
      ;  *collectpp = collect
   ;  }

      mcxIOfree(&xf)
   ;  return ct
;  }


static mcxstatus collectMain
(  int                  argc
,  const char*          argv[]
)
   {  aggr* collect = NULL
   ;  int a
   ;  dim i, collect_n = 0
   ;  mclTab* tab = NULL
   ;  double avg = 0.0
   ;  mclx* aggr = NULL, *mx = NULL
                                               /*  mcxHash* map = NULL */
   ;  mcxIO* xfout = mcxIOnew(out_g, "w")
   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  if
      (  transform_spec
      && (!(transform = mclgTFparse(NULL, transform_spec)))
      )
      mcxDie(1, me, "input -tf spec does not parse")

   ;  if (xftab_g)
         tab = mclTabRead(xftab_g, NULL, EXIT_ON_FAIL)
            /* map not used; perhaps someday we want to map labels to indexes?
             * in that case, we could also simply reverse the tab when reading ..
      ,  map = mclTabHash(tab)
            */

   ;  if (!collect_g)
      mcxDie(1, me, "require one of --paste, --add-column, --add-matrix")

   ;  if (argc)
      {  if (collect_g == 'm')
         {  mcxIO* xf = mcxIOnew(argv[0], "r")
         ;  mcxIOopen(xf, EXIT_ON_FAIL)
         ;  aggr = mclxRead(xf, EXIT_ON_FAIL)
         ;  mcxIOfree(&xf)
      ;  }
         else
         collect_n = do_a_file(&collect, argv[0], 0)
   ;  }

      if (tab && collect_n != N_TAB(tab) + (header_g ? 1 : 0))
      mcxErr
      (  me
      ,  "tab has differing size (%lu vs %lu), continuing anyway"
      ,  (ulong) N_TAB(tab)
      ,  (ulong) (collect_n ? collect_n -1 : 0)
      )

   ;  for (a=1;a<argc;a++)
      {  if (collect_g == 'm')
         {  mcxIO* xf = mcxIOnew(argv[a], "r")
         ;  mcxIOopen(xf, EXIT_ON_FAIL)
         ;  mx = mclxRead(xf, EXIT_ON_FAIL)
         ;  mclxAugment(aggr, mx, fltop_g)
         ;  mcxIOfree(&xf)
         ;  mclxFree(&mx)
      ;  }
         else
         do_a_file(&collect, argv[a], collect_n)
   ;  }

      if (collect_g == 'm')
      {  if (transform)
         mclgTFexec(aggr, transform)
      ;  if (mcx_wb_g)
         mclxbWrite(aggr, xfout, EXIT_ON_FAIL)
      ;  else
         mclxWrite(aggr, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
      ;  mcxIOclose(xfout)
      ;  exit(0)
   ;  }

   /* fimxe: dispatch on binary_g */

      for (i=0;i<collect_n;i++)
      {  const char* lb = collect[i].label

      ;  if (!i && collect[i].columns && collect_g != 'p')
         {  fprintf(xfout->fp, "%s\t%s\n", lb, collect[i].columns->str)
         ;  continue
      ;  }

         if (tab && (!header_g || i > 0))
         {  unsigned u = atoi(lb)
         ;  lb = mclTabGet(tab, u, NULL)
         ;  if (TAB_IS_NA(tab, lb))
            mcxDie(1, me, "no label found for index %ld - abort", (long) u)
      ;  }
         if (summary_g)
         avg += collect[i].val
      ;  else
         {  if (collect_g == 'p')
            fprintf(xfout->fp, "%s%s\n", lb, collect[i].columns->str)
         ;  else
            fprintf(xfout->fp, "%s\t%.8g\n", lb, collect[i].val)
      ;  }
      }
      if (summary_g && collect_n)
      {  dim middle1 = (collect_n-1)/2, middle2 = collect_n/2
      ;  qsort(collect, collect_n, sizeof collect[0], aggr_cmp_val)
      ;  avg /= collect_n
      ;  fprintf                    /* --summary option is a bit rubbish interface-wise */
         (  xfout->fp
         ,  "%g %g %g %g\n"
         ,  collect[0].val
         ,  (collect[middle1].val + collect[middle2].val) / 2
         ,  collect[collect_n-1].val
         ,  avg
         )
   ;  }
      return STATUS_OK
;  }


static mcxDispHook collectEntry
=  {  "collect"
   ,  "collect [files]"
   ,  collectOptions
   ,  sizeof(collectOptions)/sizeof(mcxOptAnchor) - 1

   ,  collectArgHandle
   ,  collectInit
   ,  collectMain

   ,  1
   ,  -1
   ,  MCX_DISP_DEFAULT
   }
;


mcxDispHook* mcxDispHookCollect
(  void
)
   {  return &collectEntry
;  }


