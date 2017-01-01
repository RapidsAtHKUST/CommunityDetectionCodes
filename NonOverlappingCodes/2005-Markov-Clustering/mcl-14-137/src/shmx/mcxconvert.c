/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011  Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#include <string.h>

#include "mcx.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "clew/cat.h"


static const char* me  =  "mcxconvert";


enum
{  MY_OPT_BINARY = MCX_DISP_UNUSED
,  MY_OPT_C2S
,  MY_OPT_S2C
,  MY_OPT_CAT
,  MY_OPT_CATMAX
,  MY_OPT_RO
}  ;


const char* syntax = "Usage: mcxconvert [options] matrix-argument(s)";


mcxOptAnchor convertOptions[] =
{  {  "--cone-to-stack"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_C2S
   ,  NULL
   ,  "transform cone file to stack file"
   }
,  {  "--stack-to-cone"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_S2C
   ,  NULL
   ,  "transform stack file to cone file"
   }
,  {  "--cat"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_CAT
   ,  NULL
   ,  "read and write cat format"
   }
,  {  "-cat-max"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CATMAX
   ,  "<num>"
   ,  "limit the stack conversion to <num> matrices"
   }
,  {  "--write-binary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_BINARY
   ,  NULL
   ,  "output native binary format"
   }
,  {  "--read-only"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_RO
   ,  NULL
   ,  "read matrix and exit (unit test)"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;



static mcxIO* xfin   =  (void*) -1;
static mcxIO *xfout  =  (void*) -1;
static int main_mode =  -1;
static dim catmax    =  -1;
static mcxbool docat =  -1;
static mcxbool test_read =  -1;


static mcxstatus convertInit
(  void
)
   {  xfin = NULL
   ;  xfout = NULL
   ;  main_mode = 'f'         /* format */
   ;  test_read = FALSE
   ;  catmax = 0
   ;  docat = FALSE
   ;  return STATUS_OK
;  }



static mcxstatus convertArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_BINARY
      :  mclxSetBinaryIO()
      ;  break
      ;

         case MY_OPT_CAT
      :  docat = TRUE         /* unused, idling */
      ;  main_mode = 'l'      /* level(s) */
      ;  break
      ;

         case MY_OPT_RO
      :  test_read = TRUE
      ;  break
      ;

         case MY_OPT_CATMAX
      :  catmax = atoi(val)
      ;  break
      ;

         case MY_OPT_S2C
      :  main_mode = 's'      /* stack to cone */
      ;  break
      ;

         case MY_OPT_C2S
      :  main_mode = 'c'      /* cone to stack */
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
      }
      return STATUS_OK
;  }


static mcxstatus convertMain
(  int          argc_unused   cpl__unused
,  const char*  argv[]
)
   {  mclMatrix*        mx

   ;  mclxCat st

   ;  xfin  = mcxIOnew(argv[0], "r")
   ;  xfout = mcxIOnew(argv[1], "w")

   ;  mclxCatInit(&st)

   ;  if (main_mode == 'l')
      {  if (mclxCatRead(xfin, &st, catmax, NULL, NULL, 0))
         mcxDie(1, me, "failure is, if not an option, the result after all")
      ;  mclxCatWrite(xfout, &st, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
   ;  }
      else if (main_mode == 'f')
      {  int format
      ;  mx = mclxRead(xfin, EXIT_ON_FAIL)
      ;  format = mclxIOformat(xfin)
      ;  if (!test_read)
         {  mcxIOopen(xfout, EXIT_ON_FAIL)
         ;  if (format == 'a')
            mclxbWrite(mx, xfout, EXIT_ON_FAIL)
         ;  else
            mclxaWrite(mx, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
      ;  }
      }
      else
      {  mcxbits bits
         =     main_mode == 'c'
            ?  MCLX_REQUIRE_DOMTREE | MCLX_CATREAD_CLUSTERSTACK
            :  main_mode == 's'
            ?  MCLX_REQUIRE_DOMSTACK | MCLX_CATREAD_CLUSTERTREE
            :  0
      ;  mcxIOopen(xfout, EXIT_ON_FAIL)

      ;  if (mclxCatRead(xfin, &st, catmax, NULL, NULL, bits))
         mcxDie(1, me, "failure is, if not an option, the result after all")

      ;  mclxCatWrite(xfout, &st, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)
   ;  }

      return 0
;  }


mcxDispHook* mcxDispHookConvert
(  void
)
   {  static mcxDispHook convertEntry
   =  {  "convert"
      ,  "convert [options] <cl file>+"
      ,  convertOptions
      ,  sizeof(convertOptions)/sizeof(mcxOptAnchor) - 1

      ,  convertArgHandle
      ,  convertInit
      ,  convertMain

      ,  2
      ,  2
      ,  MCX_DISP_MANUAL
      }
   ;  return &convertEntry
;  }


         /* fixme: query mode, currently lost */
#if 0
   ;  if (main_mode == 'q')
      {  mcxIO* xf = mcxIOnew(argv[a], "r")
      ;  const char* fmt
      ;  int format

      ;  mcxIOopen(xf, EXIT_ON_FAIL)
      ;  if (mclxReadDimensions(xf, &n_cols, &n_rows))
         mcxDie(1, me, "reading %s failed", argv[1])

      ;  format = mclxIOformat(xf)
      ;  fmt = format == 'b' ? "binary" : format == 'a' ? "interchange" : "?"
      ;  fprintf
         (  stdout
         ,  "%s format,  row x col dimensions are %ld x %ld\n"
         ,  fmt
         ,  n_rows
         ,  n_cols
         )
   ;  }
#endif
