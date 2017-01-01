/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* NOTE: with --cleanup, this app should return all memory.
 *             after major rewrites this is sometimes not the case.
 * TODO
 *  mx/mcxload -etc fail.etc.data.1 -restrict-tabr 3.tab --scrub -write-tabr t --canonicalr -o tt
 * -  warn when there is no tab for output matrix (scrub, can).
 * -  with {re}strict-tab, matrix should be alloced on those domains.
 * -  put in place as many argument combination checks as humanly possible.
 *
 *    -  support values with etc/235 formats
 *    -  option to ignore third column.
 *    -  option to pick out columns
 *
*/

#include <string.h>
#include <stdlib.h>

#include "impala/io.h"
#include "impala/stream.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/ivp.h"
#include "impala/app.h"
#include "impala/pval.h"
#include "impala/app.h"

#include "util/ting.h"
#include "util/err.h"
#include "util/types.h"
#include "util/opt.h"

#include "mcl/transform.h"


const char* me = "mcxload";
const char* syntax = "Usage: mcxload -abc <fname> -o <fname> [options]\n"
                     "all file names use - to indicate stdin/stdout";

#define  SELECT_IGQ        1 <<  0
#define  SELECT_ILQ        1 <<  1
#define  SELECT_IFLOOR     1 <<  2
#define  SELECT_ICEIL      1 <<  3
#define  SELECT_OGQ        1 <<  4
#define  SELECT_OLQ        1 <<  5
#define  SELECT_OFLOOR     1 <<  6
#define  SELECT_OCEIL      1 <<  7

enum
{  MY_OPT_ABC
,  MY_OPT_123
,  MY_OPT_ETC
,  MY_OPT_ETC_AI
,  MY_OPT_SIF
,  MY_OPT_235
,  MY_OPT_235_AI
,  MY_OPT_PCK
,  MY_OPT_PCK_COLHI
,  MY_OPT_PCK_ROWHI
,  MY_OPT_STREAM_MIRROR
,  MY_OPT_STREAM_SPLIT
,  MY_OPT_OUT_MX

,  MY_OPT_STRICT_TABG = MY_OPT_OUT_MX + 2
,  MY_OPT_RESTRICT_TABG
,  MY_OPT_EXTEND_TABG
,  MY_OPT_SCRUB_DOMG
,  MY_OPT_CANONICALG
,  MY_OPT_OUT_TABG
,  MY_OPT_DMAX

,  MY_OPT_STRICT_TABC = MY_OPT_OUT_TABG + 2
,  MY_OPT_RESTRICT_TABC
,  MY_OPT_EXTEND_TABC
,  MY_OPT_CANONICALC
,  MY_OPT_SCRUB_DOMC
,  MY_OPT_OUT_TABC
,  MY_OPT_CMAX
,  MY_OPT_CREQUIRE_235

,  MY_OPT_STRICT_TABR = MY_OPT_CMAX + 2
,  MY_OPT_RESTRICT_TABR
,  MY_OPT_EXTEND_TABR
,  MY_OPT_SCRUB_DOMR
,  MY_OPT_CANONICALR
,  MY_OPT_OUT_TABR
,  MY_OPT_RMAX

,  MY_OPT_DEDUP         =  MY_OPT_OUT_TABR + 2
,  MY_OPT_STREAM_TRANSFORM
,  MY_OPT_TRANSFORM
,  MY_OPT_STREAM_LOG
,  MY_OPT_STREAM_NEGLOG
,  MY_OPT_STREAM_NEGLOG10
,  MY_OPT_EXPECT_VALUES
,  MY_OPT_IMAGE
,  MY_OPT_TRANSPOSE
,  MY_OPT_CLEANUP
,  MY_OPT_NW
,  MY_OPT_WB
,  MY_OPT_DEBUG
,  MY_OPT_HELP
,  MY_OPT_APROPOS
,  MY_OPT_VERSION
}  ;


mcxOptAnchor options[] =
{
   {  "-h"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_HELP
   ,  NULL
   ,  "print this help"
   }
,  {  "--debug"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DEBUG
   ,  NULL
   ,  "debug"
   }
,  {  "--help"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_APROPOS
   ,  NULL
   ,  "print this help"
   }
,  {  "--write-binary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WB
   ,  NULL
   ,  "output matrix in binary format"
   }
,  {  "--clean-up"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_CLEANUP
   ,  NULL
   ,  "free all memory used (test purpose)"
   }
,  {  "--no-write"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_NW
   ,  NULL
   ,  "exit after loading of matrix"
   }
,  {  "--expect-values"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_EXPECT_VALUES
   ,  NULL
   ,  "accept extended SIF or ETC format (label:weight fields)"
   }
,  {  "--version"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_VERSION
   ,  NULL
   ,  "print version information"
   }
,  {  "-ri"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMAGE
   ,  "<max|min|add|mul>"
   ,  "combine input matrix with its transpose"
   }
,  {  "-re"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DEDUP
   ,  "<max|min|add|first|last>"
   ,  "deduplicate repeated entries"
   }
,  {  "--stream-mirror"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_STREAM_MIRROR
   ,  NULL
   ,  "add y -> x when x -> y"
   }
,  {  "--stream-split"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_STREAM_SPLIT
   ,  NULL
   ,  "assume two independent domains (e.g. bipartite graph)"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT_MX
   ,  "<fname>"
   ,  "output matrix to file <fname>"
   }

,  {  "--scrub"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_SCRUB_DOMG
   ,  NULL
   ,  "with -(re)strict-tab, remove unseen labels"
   }
,  {  "--scrubc"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_SCRUB_DOMC
   ,  NULL
   ,  "with -(re)strict-tabc, remove unseen labels"
   }
,  {  "--scrubr"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_SCRUB_DOMR
   ,  NULL
   ,  "with -(re)strict-tabr, remove unseen labels"
   }

,  {  "--canonical"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_CANONICALG
   ,  NULL
   ,  "map result matrix and tab onto canonical domains"
   }
,  {  "--canonicalc"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_CANONICALC
   ,  NULL
   ,  "map result matrix and tab onto canonical column domain"
   }
,  {  "--canonicalr"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_CANONICALR
   ,  NULL
   ,  "map result matrix and tab onto canonical row domain"
   }

,  {  "-write-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT_TABG
   ,  "<fname>"
   ,  "output domain tab to file <fname>"
   }
,  {  "-write-tabc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT_TABC
   ,  "<fname>"
   ,  "output column tab to file <fname>"
   }
,  {  "-write-tabr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT_TABR
   ,  "<fname>"
   ,  "output row tab to file <fname>"
   }

,  {  "-123-max"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DMAX
   ,  "<num>"
   ,  "set column and row ranges with -123 option"
   }
,  {  "-123-maxc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CMAX
   ,  "<num>"
   ,  "set column range with -123 option"
   }
,  {  "-123-maxr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RMAX
   ,  "<num>"
   ,  "set row range with -123 option"
   }
,  {  "-235-maxc"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_CREQUIRE_235
   ,  "<num>"
   ,  "number of columns is set to <num> at least"
   }
,  {  "-extend-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_EXTEND_TABG
   ,  "<fname>"
   ,  "use dom tab in file <fname>, extend if necessary"
   }
,  {  "-extend-tabc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_EXTEND_TABC
   ,  "<fname>"
   ,  "use col tab in file <fname>, extend if necessary"
   }
,  {  "-extend-tabr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_EXTEND_TABR
   ,  "<fname>"
   ,  "use row tab in file <fname>, extend if necessary"
   }

,  {  "-strict-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STRICT_TABG
   ,  "<fname>"
   ,  "use dom tab in file <fname>, die on miss"
   }
,  {  "-strict-tabc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STRICT_TABC
   ,  "<fname>"
   ,  "use col tab in file <fname>, die on miss"
   }
,  {  "-strict-tabr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STRICT_TABR
   ,  "<fname>"
   ,  "use row tab in file <fname>, die on miss"
   }

,  {  "-restrict-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RESTRICT_TABG
   ,  "<fname>"
   ,  "use dom tab in file <fname>, ignore miss"
   }
,  {  "-restrict-tabc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RESTRICT_TABC
   ,  "<fname>"
   ,  "use col tab in file <fname>, ignore miss"
   }
,  {  "-restrict-tabr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RESTRICT_TABR
   ,  "<fname>"
   ,  "use row tab in file <fname>, ignore miss"
   }

,  {  "-abc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ABC
   ,  "<fname>"
   ,  "input file in abc format"
   }
,  {  "-sif"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SIF
   ,  "<fname>"
   ,  "input file in sif format"
   }
,  {  "-etc-ai"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ETC_AI
   ,  "<fname>"
   ,  "input file in etc format, auto-increment columns"
   }
,  {  "-etc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ETC
   ,  "<fname>"
   ,  "input file in etc format"
   }
,  {  "-235-ai"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_235_AI
   ,  "<fname>"
   ,  "input file in numerical etc format, auto-increment columns"
   }
,  {  "-235"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_235
   ,  "<fname>"
   ,  "input file in numerical etc format"
   }
,  {  "-123"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_123
   ,  "<fname>"
   ,  "input file in 123 format"
   }
,  {  "-packed"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PCK
   ,  "<fname>"
   ,  "input file in packed format"
   }
,  {  "-pack-cnum"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PCK_COLHI
   ,  "<num>"
   ,  "Number of columns"
   }
,  {  "-pack-rnum"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PCK_ROWHI
   ,  "<num>"
   ,  "Number of rows"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to matrix"
   }
,  {  "-stream-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_STREAM_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to stream values"
   }
,  {  "--stream-log"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_STREAM_LOG
   ,  NULL
   ,  "take log of stream value"
   }
,  {  "--stream-neg-log"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_STREAM_NEGLOG
   ,  NULL
   ,  "take negative log of stream value"
   }
,  {  "--stream-neg-log10"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_STREAM_NEGLOG10
   ,  NULL
   ,  "take negative log-10 of stream value"
   }
,  {  "--transpose"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TRANSPOSE
   ,  NULL
   ,  "transpose result"
   }
,  {  NULL, 0, 0, NULL, NULL  }
}  ;


void usage
(  const char**
)  ;


mclx* read_packed
(  mcxIO*   xfin
,  dim      colhi
,  dim      rowhi
)
   {  long  src_and_len[2] = { -1, -1 }
   ;  mclpAR par
   ;  mclx* mx
      =  mclxAllocZero
         (  mclvCanonical(NULL, colhi, 1.0)
         ,  mclvCanonical(NULL, rowhi, 1.0)
         )
   ;  mclpARinit(&par)
   ;  mclpARensure(&par, rowhi)
   ;  while (1)
      {  if (2 != fread(&src_and_len, sizeof src_and_len[0], 2, xfin->fp))
         break
      ;  if (src_and_len[0] < 0 || src_and_len[0] >= colhi || src_and_len[1] < 0 || src_and_len[1] > rowhi)
         {  mcxErr(me, "src/dst pair [%ld %ld] not conforming to bounds", src_and_len[0], src_and_len[1])
         ;  break
      ;  }
         par.n_ivps = src_and_len[1]
      ;  if (par.n_ivps != fread(par.ivps, sizeof par.ivps[0], par.n_ivps, xfin->fp))
         {  mcxErr(me, "read for vector %ld (%ld entries) failed", src_and_len[0], src_and_len[1])
         ;  break
      ;  }
         if (!mclpARbatchCheck(&par, 0, rowhi))
         {  mcxErr(me, "batch check for vector %ld (%ld entries) failed", src_and_len[0], src_and_len[1])
         ;  break
      ;  }
         mclvFromPAR(mx->cols+src_and_len[0], &par, 0, mclpMergeMax, fltMax)
   ;  }
      if (!feof(xfin->fp))
      {  mcxErr(me, "error occurred during read, discarding result")
      ;  mclxFree(&mx)
   ;  }
      else
      {  long n = mclxNrofEntries(mx)
      ;  mcxTell(me, "read matrix with %ld entries\n", n)
   ;  }

      return mx
;  }


int main
(  int                  argc
,  const char*          argv[]
)  
   {  mcxIO* xfin          =  mcxIOnew("-", "r")
   ;  mcxIO* xfmx          =  mcxIOnew("-", "w")
   ;  mcxIO* xfcachetabg   =  NULL
   ;  mcxIO* xfcachetabc   =  NULL
   ;  mcxIO* xfcachetabr   =  NULL
   ;  mcxIO* xfusetabg     =  NULL
   ;  mcxIO* xfusetabc     =  NULL
   ;  mcxIO* xfusetabr     =  NULL
   ;  mcxIO* xferr         =  mcxIOnew("stderr", "w")

   ;  mclgTF* stream_transform   =  NULL
   ;  mcxTing* stream_transform_spec = NULL

   ;  mclTab *tab_map_col  =  NULL
   ;  mclTab *tab_map_row  =  NULL
   ;  mclTab *tab_map_sym  =  NULL

   ;  mclgTF* transform          =  NULL
   ;  mcxTing* transform_spec    =  NULL

   ;  mcxbits bits_stream_input  =  MCLXIO_STREAM_ABC
   ;  mcxbits bits_stream_tabg   =  0     /* g for graph */
   ;  mcxbits bits_stream_tabc   =  0
   ;  mcxbits bits_stream_tabr   =  0
   ;  mcxbits bits_stream_tabx   =  0
   ;  mcxbits bits_stream_other  =  MCLXIO_STREAM_SYMMETRIC

   ;  mcxbool packed_g = FALSE
   ;  dim packed_col_hi = 0
   ;  dim packed_row_hi = 0

   ;  double (*symfunc) (pval val1, pval val2) = NULL

   ;  mclx* mx = NULL

   ;  mclxIOstreamer streamer
   ;  void (*merge)(void* ivp1, const void* ivp2) = NULL

   ;  mcxbool symmetric =  FALSE
   ;  mcxbool transpose =  FALSE
   ;  mcxbool cleanup   =  FALSE
   ;  mcxbool dowrite   =  TRUE
   ;  mcxbits scrub     =  0
   ;  mcxbool write_binary = FALSE

#define COL_ON 1
#define ROW_ON 2
#define SYM_ON 3

   ;  mcxbits canonical =  0

   ;  mcxOption* opts, *opt
   ;  mcxstatus parseStatus = STATUS_OK

   ;  streamer.tab_sym_in  =  NULL
   ;  streamer.tab_sym_out =  NULL
   ;  streamer.tab_col_in  =  NULL
   ;  streamer.tab_col_out =  NULL
   ;  streamer.tab_row_in  =  NULL
   ;  streamer.tab_row_out =  NULL
   ;  streamer.cmax_123    =  0
   ;  streamer.rmax_123    =  0
   ;  streamer.cmax_235    =  0

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN

   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)
   ;  opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)

   ;  if (!opts)
      exit(0)

   ;  mclx_app_init(stderr)
   ;  mcxIOopen(xferr, EXIT_ON_FAIL)

   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch
      ;  int t = 0

      ;  switch(anch->id)
         {  case MY_OPT_HELP
         :  case MY_OPT_APROPOS
         :  mcxOptApropos(stdout, me, syntax, 0, MCX_OPT_DISPLAY_SKIP, options)
         ;  return 0
         ;

            case MY_OPT_CLEANUP
         :  cleanup = TRUE
         ;  break
         ;

            case MY_OPT_EXPECT_VALUES
         :  bits_stream_other |= MCLXIO_STREAM_EXPECT_VALUE
         ;  break
         ;

            case MY_OPT_NW
         :  dowrite = FALSE
         ;  break
         ;

            case MY_OPT_DEBUG
         :  bits_stream_other |= MCLXIO_STREAM_DEBUG | MCLXIO_STREAM_WARN
         ;  break
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  return 0
         ;

            case MY_OPT_WB
         :  write_binary = TRUE
         ;  break
         ;

            case MY_OPT_OUT_MX
         :  mcxIOrenew(xfmx, opt->val, NULL)
         ;  break
         ;

            case MY_OPT_CANONICALG
         :  canonical |=  COL_ON | ROW_ON
         ;  break
         ;

            case MY_OPT_CANONICALC
         :  canonical |= COL_ON
         ;  break
         ;

            case MY_OPT_CANONICALR
         :  canonical |= ROW_ON
         ;  break
         ;

            case MY_OPT_SCRUB_DOMR
         :  scrub |= MCLX_SCRUB_ROWS
         ;  break
         ;

            case MY_OPT_SCRUB_DOMC
         :  scrub |= MCLX_SCRUB_COLS
         ;  break
         ;

            case MY_OPT_SCRUB_DOMG
         :  scrub |= MCLX_SCRUB_GRAPH
         ;  break
         ;

            case MY_OPT_CMAX
         :  streamer.cmax_123 = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_CREQUIRE_235
         :  streamer.cmax_235 = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_RMAX
         :  streamer.rmax_123 = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_DMAX
         :  streamer.cmax_123 = atoi(opt->val)
         ;  streamer.rmax_123 = streamer.cmax_123
         ;  break
         ;

            case MY_OPT_OUT_TABG
         :  xfcachetabg = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_OUT_TABC
         :  xfcachetabc = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_OUT_TABR
         :  xfcachetabr = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_STRICT_TABG :   t++
         ;  case MY_OPT_RESTRICT_TABG : t++
         ;  case MY_OPT_EXTEND_TABG
         :
            {  xfusetabg = mcxIOnew(opt->val, "r")
            ;  bits_stream_tabg
               |=    t == 2 ? MCLXIO_STREAM_GTAB_STRICT
                  :  t == 1 ? MCLXIO_STREAM_GTAB_RESTRICT
                  :           MCLXIO_STREAM_GTAB_EXTEND
            ;  break
         ;  }

            case MY_OPT_STRICT_TABC :   t++
         ;  case MY_OPT_RESTRICT_TABC : t++
         ;  case MY_OPT_EXTEND_TABC
         :
            {  xfusetabc = mcxIOnew(opt->val, "r")
            ;  bits_stream_tabc
               |=    t == 2 ? MCLXIO_STREAM_CTAB_STRICT
                  :  t == 1 ? MCLXIO_STREAM_CTAB_RESTRICT
                  :           MCLXIO_STREAM_CTAB_EXTEND
            ;  break
         ;  }

            case MY_OPT_STRICT_TABR :   t++
         ;  case MY_OPT_RESTRICT_TABR : t++
         ;  case MY_OPT_EXTEND_TABR
         :
            {  xfusetabr = mcxIOnew(opt->val, "r")
            ;  bits_stream_tabr
               |=    t == 2 ? MCLXIO_STREAM_RTAB_STRICT
                  :  t == 1 ? MCLXIO_STREAM_RTAB_RESTRICT
                  :           MCLXIO_STREAM_RTAB_EXTEND
            ;  break
         ;  }

            case MY_OPT_PCK
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  packed_g = TRUE
         ;  break
         ;

            case MY_OPT_PCK_ROWHI
         :  packed_row_hi = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_PCK_COLHI
         :  packed_col_hi = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_123
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_123
         ;  break
         ;

            case MY_OPT_ETC_AI
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_ETC_AI
         ;  BIT_OFF(bits_stream_other, MCLXIO_STREAM_SYMMETRIC)
         ;  break
         ;

            case MY_OPT_SIF
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_SIF
         ;  BIT_OFF(bits_stream_other, MCLXIO_STREAM_SYMMETRIC)
         ;  break
         ;

            case MY_OPT_ETC
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_ETC
         ;  BIT_OFF(bits_stream_other, MCLXIO_STREAM_SYMMETRIC)
         ;  break
         ;

            case MY_OPT_235_AI
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_235_AI
         ;  BIT_OFF(bits_stream_other, MCLXIO_STREAM_SYMMETRIC)
         ;  break
         ;

            case MY_OPT_235
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_235
         ;  BIT_OFF(bits_stream_other, MCLXIO_STREAM_SYMMETRIC)
         ;  break
         ;

            case MY_OPT_ABC
         :  mcxIOrenew(xfin, opt->val, NULL)
         ;  bits_stream_input = MCLXIO_STREAM_ABC
         ;  break
         ;

            case MY_OPT_STREAM_NEGLOG10
         :  bits_stream_other |= MCLXIO_STREAM_NEGLOGTRANSFORM | MCLXIO_STREAM_LOG10
         ;  break
         ;

            case MY_OPT_STREAM_NEGLOG
         :  bits_stream_other |= MCLXIO_STREAM_NEGLOGTRANSFORM
         ;  break
         ;

            case MY_OPT_STREAM_LOG
         :  bits_stream_other |= MCLXIO_STREAM_LOGTRANSFORM
         ;  break
         ;

            case MY_OPT_TRANSFORM
         :  transform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_STREAM_TRANSFORM
         :  stream_transform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_STREAM_SPLIT
         :  BIT_OFF(bits_stream_other, MCLXIO_STREAM_SYMMETRIC)
         ;  break
         ;

            case MY_OPT_STREAM_MIRROR
         :  bits_stream_other |=  MCLXIO_STREAM_MIRROR | MCLXIO_STREAM_SYMMETRIC
         ;  break
         ;

            case MY_OPT_IMAGE
         :  if (!strcmp(opt->val, "max"))
            symfunc = fltMax
         ;  else if (!strcmp(opt->val, "add"))
            symfunc = fltAdd
         ;  else if (!strcmp(opt->val, "mul"))
            symfunc = fltCross
         ;  else if (!strcmp(opt->val, "min"))
            symfunc = fltMin
         ;  else
            mcxDie(1, me, "unknown image mode %s", opt->val)
         ;  break
         ;

            case MY_OPT_DEDUP
         :  if (!strcmp(opt->val, "max"))
            merge = mclpMergeMax
         ;  else if (!strcmp(opt->val, "min"))
            merge = mclpMergeMin
         ;  else if (!strcmp(opt->val, "add"))
            merge = mclpMergeAdd
         ;  else if (!strcmp(opt->val, "first"))
            merge = mclpMergeLeft
         ;  else if (!strcmp(opt->val, "last"))
            merge = mclpMergeRight
         ;  else
            mcxDie(1, me, "unknown dup mode %s", opt->val)
         ;  break
         ;

            case MY_OPT_TRANSPOSE
         :  transpose = TRUE
         ;  break
      ;  }
      }
   
      mcxOptFree(&opts)

   ;  symmetric = bits_stream_other & MCLXIO_STREAM_SYMMETRIC

   ;  if ((xfusetabc || xfusetabr || xfcachetabc || xfcachetabr) && symmetric)
      mcxDie(1, me, "(implied) symmetric mode precludes all tabc and tabr options")

   ;  if ((xfusetabg || xfcachetabg) && !symmetric)
      mcxDie(1, me, "two domain mode precludes all symmetric tab options")

   ;  if (scrub && canonical)
      mcxDie(1, me, "scrub and canonical options not yet working together")
         /* see further below */

   ;  if
      (  (xfusetabc || xfusetabr || xfusetabg || xfcachetabc || xfcachetabr || xfcachetabr)
      && (bits_stream_input & MCLXIO_STREAM_NUMERIC)
      )
      mcxDie(1, me, "(implied) numeric mode currently precludes all tab options")

   ;  if
      (  stream_transform_spec
      && !(stream_transform = mclgTFparse(NULL, stream_transform_spec))
      )
      mcxDie(1, me, "stream tf-spec does not parse")

   ;  if
      (  transform_spec
      && !(transform = mclgTFparse(NULL, transform_spec))
      )
      mcxDie(1, me, "matrix tf-spec does not parse")

   ;  if (symmetric && bits_stream_tabg)
         streamer.tab_sym_in = mclTabRead(xfusetabg, NULL, EXIT_ON_FAIL)
      ,  bits_stream_tabx = bits_stream_tabg
   ;  else
      {  if (bits_stream_tabc)
         streamer.tab_col_in = mclTabRead(xfusetabc, NULL, EXIT_ON_FAIL)
      ;  if (bits_stream_tabr)
         streamer.tab_row_in = mclTabRead(xfusetabr, NULL, EXIT_ON_FAIL)
      ;  bits_stream_tabx = bits_stream_tabc | bits_stream_tabr
   ;  }

      mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)

   ;  mcxIOopen(xfin, EXIT_ON_FAIL)

   ;  mx
      =  packed_g
         ?  read_packed(xfin, packed_col_hi, packed_row_hi)
         :  mclxIOstreamIn
            (  xfin
            ,  bits_stream_input | bits_stream_other | bits_stream_tabx
            ,  stream_transform ? mclgTFgetEdgePar(stream_transform) : NULL
            ,  merge
            ,  &streamer
            ,  EXIT_ON_FAIL
            )

   ;  if (!mx)
      mcxDie(1, me, "error occurred")
   ;  mcxIOclose(xfin)

   ;  if (symmetric && !mclxIsGraph(mx))
      mcxErr(me, "error detected, symmetric on but domains differ (continuing)")

   ;  if (transpose || symfunc)
      {  mclx* tp = mclxTranspose(mx)

      ;  if (symfunc)
         {  mclx* sym = mclxBinary(mx, tp, symfunc)
         ;  mclxFree(&mx)
         ;  mclxFree(&tp)
         ;  mx = sym
      ;  }
         else
            mclxFree(&mx)
         ,  mx = tp
   ;  }

      if (transform)
      mclgTFexec(mx, transform)

   ;  if (scrub)
      mclxScrub(mx, scrub)

         /* map domains onto canonical domains
          * This is not the prettiest of functionality, in how it logically
          * works with other modalities. Below should be funcified to boot.
          * Below will fail if the matrix was scrubd,
          * as the transformation matrix can no longer deal with
          * tab domain.
          * Perhaps it is time to have some wrapper functions that
          * act on { mx, tab } combo.
         */
   ;  if (canonical)
      {  mclx* dom_map_col = NULL, *dom_map_row = NULL, *dom_map_sym = NULL

      ;  if (symmetric && !mclxColCanonical(mx))
         dom_map_sym
         =  mclxMakeMap
            (  mclvCopy(NULL, mx->dom_cols)
            ,  mclvCanonical(NULL, mx->dom_cols->n_ivps, 1.0)
            )
      ;  else  if (!symmetric)
         {  if ((canonical & COL_ON) && !mclxColCanonical(mx))
            dom_map_col
         =  mclxMakeMap
            (  mclvCopy(NULL, mx->dom_cols)
            ,  mclvCanonical(NULL, mx->dom_cols->n_ivps, 1.0)
            )
         ;  if ((canonical & ROW_ON) && !mclxRowCanonical(mx))
            dom_map_row
         =  mclxMakeMap
            (  mclvCopy(NULL, mx->dom_rows)
            ,  mclvCanonical(NULL, mx->dom_rows->n_ivps, 1.0)
            )
      ;  }

         if (dom_map_sym)
         {  mclTab* tab_orig = streamer.tab_sym_out 

         ;  if (mclxMapCols(mx, dom_map_sym))
            mcxDie(1, me, "error mapping column domains")
         ;  if (mclxMapRows(mx, dom_map_sym))
            mcxDie(1, me, "error mapping row domains")

         ;  if (tab_orig && !(tab_map_sym = mclTabMap(tab_orig, dom_map_sym)))
            mcxDie(1, me, "could not map tab domain")
      ;  }
         if (dom_map_col)
         {  if (mclxMapCols(mx, dom_map_col))
            mcxDie(1, me, "error mapping column domains")
         ;  if
            (  streamer.tab_col_out
            && !(tab_map_col = mclTabMap(streamer.tab_col_out, dom_map_col))
            )
            mcxDie(1, me, "could not map cols")
      ;  }
         if (dom_map_row)
         {  if (mclxMapRows(mx, dom_map_row))
            mcxDie(1, me, "error mapping row domains")
         ;  if
            (  streamer.tab_row_out
            && !(tab_map_row = mclTabMap(streamer.tab_row_out, dom_map_row))
            )
            mcxDie(1, me, "could not map rows")
      ;  }
      }

      if (dowrite)
      {  if (write_binary)
         mclxbWrite(mx, xfmx, EXIT_ON_FAIL)
      ;  else
         mclxWrite(mx, xfmx, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  }

      mcxIOclose(xfmx)

                  /* fixme: the tab_map_sym check is ugly.  It would be neater
                   * to have a { mx, tabsym, tabcol, tabrow }  tuple that is
                   * kept consistent at all time.  As it is, the consistency
                   * semantics are spread like spaghetti. A heartening thought
                   * though, it used to be ten times worse before
                   * mclxIOstreamIn rewrite.
                  */
   ;  if
      (  bits_stream_input
      &  (MCLXIO_STREAM_ABC | MCLXIO_STREAM_ETC | MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_SIF)
      )
      {  mclTab* tab_sym = tab_map_sym ? tab_map_sym : streamer.tab_sym_out
      ;  mclTab* tab_col = tab_map_col ? tab_map_col : streamer.tab_col_out
      ;  mclTab* tab_row = tab_map_row ? tab_map_row : streamer.tab_row_out
;if (bits_stream_other & MCLXIO_STREAM_DEBUG)
fprintf(stderr, "tab s=%p c=%p r=%p\n", (void*) tab_sym, (void*) tab_col, (void*) tab_row)
      ;  if (symmetric && xfcachetabg && tab_sym)
         {  mclTabWrite
            (  tab_sym
            ,  xfcachetabg
            ,  scrub & MCLX_SCRUB_COLS ? mx->dom_cols : NULL
            ,  RETURN_ON_FAIL
            )
         ;  mcxIOclose(xfcachetabg)
         ;  if (!streamer.tab_sym_in)
            mcxLog(MCX_LOG_MODULE, me, "tab has %ld entries", (long) N_TAB(tab_sym))
         ;  else if (N_TAB(streamer.tab_sym_in) != N_TAB(tab_sym))
            mcxLog
            (  MCX_LOG_MODULE
            ,  me
            ,  "tab went from %ld to %ld nodes"
            ,  (long) N_TAB(streamer.tab_sym_in)
            ,  (long) N_TAB(tab_sym)
            )
         ;  else
            mcxLog(MCX_LOG_MODULE, me, "tab same as before")
      ;  }

         /* fixme funcify this to unify row and col cases (unless etc plays up) */

         if (!symmetric && xfcachetabc)
         {  if (bits_stream_input & MCLXIO_STREAM_ETC_AI)
            {  mclTabWriteDomain(mx->dom_cols, xfcachetabc, RETURN_ON_FAIL)
            ;  mcxIOclose(xfcachetabc)
         ;  }
            else if (tab_col)
            {  mclTabWrite
               (  tab_col
               ,  xfcachetabc
               ,  scrub & MCLX_SCRUB_COLS ? mx->dom_cols : NULL
               ,  RETURN_ON_FAIL
               )
            ;  mcxIOclose(xfcachetabc)
            ;  if (!streamer.tab_col_in)
               mcxLog(MCX_LOG_MODULE, me, "coltab has %ld entries", (long) N_TAB(tab_col))
            ;  else if (N_TAB(streamer.tab_col_in) != N_TAB(tab_col))
               mcxLog
               (  MCX_LOG_MODULE
               ,  me
               ,  "tabc went from %ld to %ld nodes"
               ,  (long) N_TAB(streamer.tab_col_in)
               ,  (long) N_TAB(tab_col)
               )
            ;  else
               mcxLog(MCX_LOG_MODULE, me, "tabc same as before")
         ;  }
         }

         if (!symmetric && xfcachetabr && tab_row)
         {  mclTabWrite
            (  tab_row
            ,  xfcachetabr
            ,  scrub & MCLX_SCRUB_ROWS ? mx->dom_rows : NULL
            ,  RETURN_ON_FAIL
            )
         ,  mcxIOclose(xfcachetabr)
         ;  if (!streamer.tab_row_in)
            mcxLog(MCX_LOG_MODULE, me, "tabr has %ld entries", (long) N_TAB(tab_row))
         ;  else if (N_TAB(streamer.tab_row_in) != N_TAB(tab_row))
            mcxLog
            (  MCX_LOG_MODULE
            ,  me
            ,  "tabr went from %ld to %ld nodes"
            ,  (long) N_TAB(streamer.tab_row_in)
            ,  (long) N_TAB(tab_row)
            )
         ;  else
            mcxLog(MCX_LOG_MODULE, me, "tabr same as before")
      ;  }
            mclTabFree(&(tab_sym))
         ;  mclTabFree(&(tab_col))
         ;  mclTabFree(&(tab_row))
   ;  }

      if (cleanup)
      {  mclxFree(&mx)
      ;  mclTabFree(&(streamer.tab_sym_in))
      ;  mclTabFree(&(streamer.tab_sym_out))
      ;  mclTabFree(&(streamer.tab_col_in))
      ;  mclTabFree(&(streamer.tab_col_out))
      ;  mclTabFree(&(streamer.tab_row_in))
      ;  mclTabFree(&(streamer.tab_row_out))
      ;  mcxTingFree(&stream_transform_spec)
      ;  mclgTFfree(&stream_transform)
      ;  mclgTFfree(&transform)
   ;  }

      mcxIOfree(&xfmx)
   ;  mcxIOfree(&xfcachetabc)
   ;  mcxIOfree(&xfcachetabr)
   ;  mcxIOfree(&xfcachetabg)
   ;  mcxIOfree(&xfusetabc)
   ;  mcxIOfree(&xfusetabr)
   ;  mcxIOfree(&xferr)
   ;  mcxIOfree(&xfin)
   ;  mclxFree(&mx)
   ;  return 0
;  }




