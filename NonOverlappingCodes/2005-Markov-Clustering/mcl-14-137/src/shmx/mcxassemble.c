/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* todo; couple suf with xf_sym */


#include <string.h>
#include <stdio.h>

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/pval.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "mcl/interpret.h"
#include "mcl/transform.h"

#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/types.h"

const char* me = "mcxassemble";

const char* syntax
=  "Usage: mcxassemble [options]\n"
   "Default output is the symmetrized result"
   " of the matrix built from the raw data"
;

enum
{  MY_OPT_BASE
,  MY_OPT_HDR
,  MY_OPT_RAW
,  MY_OPT_SINGLE
,  MY_OPT_OUTPUT
,  MY_OPT_DIGITS
,  MY_OPT_XO
,  MY_OPT_WB
,  MY_OPT_PLUS
,                 MY_OPT_NO
,  MY_OPT_MAP =   MY_OPT_NO  + 2
,  MY_OPT_MAP_
,  MY_OPT_CMAP
,  MY_OPT_CMAP_
,  MY_OPT_RTAG
,  MY_OPT_RMAP_
,  MY_OPT_TAG
,  MY_OPT_CTAG
,                 MY_OPT_RMAP
,  MY_OPT_SKW =   MY_OPT_RMAP + 2
,  MY_OPT_SKW_
,  MY_OPT_PRM
,  MY_OPT_PRM_
,                       MY_OPT_CHECK
,  MY_OPT_RAWTRANSFORM =  MY_OPT_CHECK + 2
,  MY_OPT_PRMTRANSFORM
,  MY_OPT_SYMTRANSFORM
,  MY_OPT_RV
,  MY_OPT_RE
,  MY_OPT_RI
,  MY_OPT_R
,  MY_OPT_QRE
,  MY_OPT_QRV
,                 MY_OPT_QR
,  MY_OPT_HELP =  MY_OPT_QR + 2
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
,  {  "-b"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_BASE
   ,  "<base>"
   ,  "use base.raw, base.hdr, and optionally base.map"
   }
,  {  "-hdr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_HDR
   ,  "<fname>"
   ,  "read header file"
   }
,  {  "-i"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SINGLE
   ,  "<fname>"
   ,  "read from single data file"
   }
,  {  "-raw"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RAW
   ,  "<fname>"
   ,  "read raw data file"
   }
,  {  "-map"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAP
   ,  "<fname>"
   ,  "apply row/col map in file"
   }
,  {  "--map"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_MAP_
   ,  NULL
   ,  "apply row/col map in base.map"
   }
,  {  "-cmap"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CMAP
   ,  "<fname>"
   ,  "apply col map in file"
   }
,  {  "--cmap"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_CMAP_
   ,  NULL
   ,  "apply col map in base.cmap"
   }
,  {  "-rmap"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RMAP
   ,  "<fname>"
   ,  "apply row map in file"
   }
,  {  "--rmap"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_RMAP_
   ,  NULL
   ,  "apply row map in base.cmap"
   }
,  {  "-tag"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAG
   ,  "<tag>"
   ,  "apply row/col map in base.tag"
   }
,  {  "-ctag"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CTAG
   ,  "<tag>"
   ,  "apply col map in base.tag"
   }
,  {  "-rtag"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RTAG
   ,  "<tag>"
   ,  "apply row map in base.tag"
   }
,  {  "-skw"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SKW
   ,  "<fname>"
   ,  "write skew matrix to file"
   }
,  {  "--skw"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SKW_
   ,  NULL
   ,  "write skew matrix to base.skw"
   }
,  {  "-prm"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PRM
   ,  "<fname>"
   ,  "write primary result to file"
   }
,  {  "--prm"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_PRM_
   ,  NULL
   ,  "write primary result matrix to base.prm"
   }
,  {  "-raw-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RAWTRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to raw values"
   }
,  {  "-prm-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PRMTRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to primary matrix"
   }
,  {  "-sym-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SYMTRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to symmetrified matrix"
   }
,  {  "-digits"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DIGITS
   ,  "<int>"
   ,  "precision to use in interchange format"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "write to file fname"
   }
,  {  "-xo"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_XO
   ,  "<suf>"
   ,  "write to base.suf (default .sym)"
   }
,  {  "-n"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_NO
   ,  NULL
   ,  "do not write default symmetrized result"
   }
,  {  "+"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_PLUS
   ,  NULL
   ,  "write result matrices in binary format"
   }
,  {  "--write-binary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WB
   ,  NULL
   ,  "write result matrices in binary format"
   }
,  {  "-s"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_CHECK
   ,  NULL
   ,  "check primary result symmetry by creating skew matrix"
   }
,  {  "-rv"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RV
   ,  "<add|max|mul|left|right>"
   ,  "action for repeated vectors"
   }
,  {  "-re"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RE
   ,  "<add|max|mul|left|right>"
   ,  "action for repeated entries"
   }
,  {  "-ri"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_RI
   ,  "<add|max|mul>"
   ,  "action for adding image with mirror"
   }
,  {  "-r"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_R
   ,  "<add|max|mul|left|right>"
   ,  "same for entries and vectors and matrix"
   }
,  {  "--quiet-re"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_QRE
   ,  NULL
   ,  "do not warn for repeated entries"
   }
,  {  "--quiet-rv"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_QRV
   ,  NULL
   ,  "do not warn for repeated vectors"
   }
,  {  "-q"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_QR
   ,  NULL
   ,  "the two above combined"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


#define MAP_COLS 1
#define MAP_ROWS 2
#define MAP_DOMS 4


int main
(  int                  argc
,  const char*          argv[]
)
   {  mcxTing* obase = mcxTingNew("out"), *ibase = NULL
   ;  mcxIO* xf_hdr = NULL, *xf_raw = NULL
      ,  *xf_prm = NULL, *xf_map = NULL, *xf_skew = NULL, *xf_sym = NULL
      ,  *xf_rmap = NULL, *xf_cmap = NULL
   ;  mclVector* dom_rows = mclvNew(NULL, 0), *dom_cols = mclvNew(NULL, 0)
   ;  const char* suf = NULL, *re = NULL, *rv = NULL, *ra = NULL, *ri = NULL
   ;  const char* tag = NULL, *rtag = NULL, *ctag = NULL
   ;  mclx *mx = NULL, *tp = NULL, *sym = NULL, *skew = NULL
   ;  mclx *cmap = NULL, *rmap = NULL
   ;  mcxbits maptype = 0
   ;  mcxbool write_skw =  FALSE
   ;  mcxbool put_sym   =  TRUE
   ;  mcxbool write_prm =  FALSE
   ;  mcxbool check_sym =  FALSE
   ;  mcxbool single_data_file = FALSE
   ;  mcxbits warn_repeat = MCLV_WARN_REPEAT
   ;  int EODATA = EOF
   ;  int digits = MCLXIO_VALUE_GETENV

   ;  void (*ivpmerge)(void* ivp1, const void* ivp2)  =  mclpMergeMax
   ;  double (*fltvecbinary)(pval val1, pval val2)    =  fltMax
   ;  double (*fltmxbinary) (pval val1, pval val2)    =  fltMax

   ;  mclgTF* rawtransform = NULL
   ;  mcxTing* rawtransform_spec = NULL

   ;  mclgTF* symtransform = NULL
   ;  mcxTing* symtransform_spec = NULL

   ;  mclgTF* prmtransform = NULL
   ;  mcxTing* prmtransform_spec = NULL

   ;  mcxstatus parseStatus = STATUS_OK
   ;  mcxOption* opts, *opt

   ;  char* qmode = mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclx_app_init(stderr)

   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)

   ;  if
      (!(opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)))
      exit(0)

   ;  for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch

      ;  switch(anch->id)
         {  case MY_OPT_HELP
         :  case MY_OPT_APROPOS
         :  mcxOptApropos(stdout, me, syntax, 20, MCX_OPT_DISPLAY_SKIP, options)
         ;  return 0
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  return 0
         ;

            case MY_OPT_MAP
         :  xf_map = mcxIOnew(opt->val, "r")
         ;  maptype |= MAP_DOMS
         ;  break
         ;

            case MY_OPT_MAP_
         :  maptype |= MAP_DOMS
         ;  break
         ;

            case MY_OPT_PRMTRANSFORM
         :  prmtransform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_SYMTRANSFORM
         :  symtransform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_RAWTRANSFORM
         :  rawtransform_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_RMAP
         :  xf_rmap = mcxIOnew(opt->val, "r")
         ;  maptype |= MAP_ROWS
         ;  break
         ;

            case MY_OPT_RMAP_
         :  maptype |= MAP_ROWS
         ;  break
         ;

            case MY_OPT_CMAP
         :  xf_cmap = mcxIOnew(opt->val, "r")
         ;  maptype |= MAP_COLS
         ;  break
         ;

            case MY_OPT_CMAP_
         :  maptype |= MAP_COLS
         ;  break
         ;



            case MY_OPT_SKW
         :  xf_skew = mcxIOnew(opt->val, "w")
         ;  write_skw = TRUE
         ;  break
         ;

            case MY_OPT_SKW_
         :  write_skw = TRUE
         ;  break
         ;

            case MY_OPT_PRM
         :  xf_prm = mcxIOnew(opt->val, "w")
         ;  write_prm = TRUE
         ;  break
         ;

            case MY_OPT_WB
         :  case MY_OPT_PLUS
         :  mclxSetBinaryIO()
         ;  break
         ;

            case MY_OPT_PRM_
         :  write_prm = TRUE
         ;  break
         ;


            case MY_OPT_TAG
         :  tag = opt->val
         ;  break
         ;

            case MY_OPT_CTAG
         :  ctag = opt->val
         ;  break
         ;

            case MY_OPT_RTAG
         :  rtag = opt->val
         ;  break
         ;

            case MY_OPT_SINGLE
         :  xf_hdr = mcxIOnew(opt->val, "r")
         ;  xf_raw = xf_hdr
         ;  single_data_file = TRUE
         ;  EODATA = ')'
         ;  break
         ;

            case MY_OPT_OUTPUT
         :  xf_sym = mcxIOnew(opt->val, "w")
         ;  break
         ;

            case MY_OPT_XO
         :  suf = opt->val
         ;  break
         ;

            case MY_OPT_NO
         :  put_sym = FALSE
         ;  break
         ;

            case MY_OPT_RE
         :  re = opt->val
         ;  break
         ;

            case MY_OPT_RI
         :  ri = opt->val
         ;  break
         ;

            case MY_OPT_RV
         :  rv = opt->val
         ;  break
         ;

            case MY_OPT_R
         :  ra = opt->val
         ;  break
         ;


            case MY_OPT_QR
         :  warn_repeat = 0
         ;  break
         ;

            case MY_OPT_QRE
         :  warn_repeat |= MCLV_WARN_REPEAT_ENTRIES
         ;  warn_repeat ^= MCLV_WARN_REPEAT_ENTRIES
         ;  break
         ;

            case MY_OPT_QRV
         :  warn_repeat |= MCLV_WARN_REPEAT_VECTORS
         ;  warn_repeat ^= MCLV_WARN_REPEAT_VECTORS
         ;  break
         ;


            case MY_OPT_BASE
         :  ibase = mcxTingNew(opt->val)
         ;  mcxTingWrite(obase, opt->val)
         ;  break
         ;

            case MY_OPT_CHECK
         :  check_sym = TRUE
         ;  break
         ;

            case MY_OPT_RAW
         :  xf_raw = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_HDR
         :  xf_hdr = mcxIOnew(opt->val, "r")
         ;  break
      ;  }
      }

      mcxOptFree(&opts)

   ;  if (single_data_file && xf_raw != xf_hdr)
         mcxErr(me, "some other option conflicts -i usage")
      ,  mcxExit(1)

   ;  if
      (  rawtransform_spec
      && !(rawtransform = mclgTFparse(NULL, rawtransform_spec))
      )
      mcxDie(1, me, "input tf-spec does not parse")

   ;  if
      (  symtransform_spec
      && !(symtransform = mclgTFparse(NULL, symtransform_spec))
      )
      mcxDie(1, me, "sym tf-spec does not parse")

   ;  if
      (  prmtransform_spec
      && !(prmtransform = mclgTFparse(NULL, prmtransform_spec))
      )
      mcxDie(1, me, "prm tf-spec does not parse")

   ;  if (ibase)
      {  if (!xf_raw)
            xf_raw = mcxIOnew(ibase->str, "r")
         ,  mcxIOappendName(xf_raw, ".raw")
      ;  if (!xf_hdr)
            xf_hdr = mcxIOnew(ibase->str, "r")
         ,  mcxIOappendName(xf_hdr, ".hdr")

      ;  if (!xf_map && (tag || maptype & MAP_DOMS))
         {  xf_map = mcxIOnew(ibase->str, "r")
         ;  if (tag)
               mcxIOappendName(xf_map, ".")
            ,  mcxIOappendName(xf_map, tag)
         ;  else
            mcxIOappendName(xf_map, ".map")
      ;  }

      ;  if (!xf_map && !xf_rmap && (rmap || maptype & MAP_ROWS))
         {  xf_rmap = mcxIOnew(ibase->str, "r")
         ;  if (rtag)
               mcxIOappendName(xf_rmap, ".")
            ,  mcxIOappendName(xf_rmap, rtag)
         ;  else
            mcxIOappendName(xf_rmap, ".rmap")
      ;  }

      ;  if (!xf_map && !xf_cmap && (cmap || maptype & MAP_COLS))
         {  xf_cmap = mcxIOnew(ibase->str, "r")
         ;  if (ctag)
               mcxIOappendName(xf_cmap, ".")
            ,  mcxIOappendName(xf_cmap, ctag)
         ;  else
            mcxIOappendName(xf_cmap, ".cmap")
      ;  }
      }

      if (obase)
      {  if (!xf_prm && write_prm)
            xf_prm = mcxIOnew(obase->str, "w")
         ,  mcxIOappendName(xf_prm, ".prm")
      ;  if (!xf_skew && write_skw)
            xf_skew = mcxIOnew(obase->str, "w")
         ,  mcxIOappendName(xf_skew, ".skw")
      ;  if (!xf_sym && put_sym)
            xf_sym = mcxIOnew(obase->str, "w")
         ,  mcxIOappendName(xf_sym, ".")
         ,  mcxIOappendName(xf_sym, suf ? suf : "sym")
   ;  }

      if (ra || re)
      {  re = ra ? ra : re
      ;  if (!strcmp(re, "add"))
         ivpmerge = mclpMergeAdd
      ;  else if (!strcmp(re, "max"))
         ivpmerge = mclpMergeMax
      ;  else if (!strcmp(re, "min"))
         ivpmerge = mclpMergeMin
      ;  else if (!strcmp(re, "mul"))
         ivpmerge = mclpMergeMul
      ;  else if (!strcmp(re, "left"))
         ivpmerge = mclpMergeLeft
      ;  else if (!strcmp(re, "right"))
         ivpmerge = mclpMergeRight
   ;  }

      if (ra || rv)
      {  rv = ra ? ra : rv
      ;  if (!strcmp(rv, "add"))
         fltvecbinary = fltAdd
      ;  else if (!strcmp(rv, "max"))
         fltvecbinary = fltMax
      ;  else if (!strcmp(rv, "min"))
         fltvecbinary = fltMin
      ;  else if (!strcmp(rv, "mul"))
         fltvecbinary = fltCross
      ;  else if (!strcmp(rv, "left"))
         fltvecbinary = fltLeft
      ;  else if (!strcmp(rv, "right"))
         fltvecbinary = fltRight
   ;  }

      if (ra || ri)
      {  ri = ra ? ra : ri
      ;  if (!strcmp(ri, "add"))
         fltmxbinary = fltAdd
      ;  else if (!strcmp(ri, "max"))
         fltmxbinary = fltMax
      ;  else if (!strcmp(ri, "min"))
         fltmxbinary = fltMin
      ;  else if (!strcmp(ri, "mul"))
         fltmxbinary = fltCross
      ;  else if (!strcmp(ri, "left"))
         fltmxbinary = fltLeft
      ;  else if (!strcmp(ri, "right"))
         fltmxbinary = fltRight
   ;  }

      if (!xf_raw || !xf_hdr)
         mcxErr(me, "raw file and header file required")
      ,  mcxExit(1)

   ;  mcxIOopen(xf_hdr, EXIT_ON_FAIL)
   ;  if (xf_raw != xf_hdr)
      mcxIOopen(xf_raw, EXIT_ON_FAIL)
   /* else: -i option was used */

   ;  if (xf_prm)
      mcxIOopen(xf_prm, EXIT_ON_FAIL)

   ;  if (xf_map)
      mcxIOopen(xf_map, EXIT_ON_FAIL)
   ;  else
      {  if (xf_rmap)
         mcxIOopen(xf_rmap, EXIT_ON_FAIL)
      ;  if (xf_cmap)
         mcxIOopen(xf_cmap, EXIT_ON_FAIL)
   ;  }

      if (xf_skew)
      mcxIOopen(xf_skew, EXIT_ON_FAIL)

   ;  if (mclxReadDomains(xf_hdr, dom_cols, dom_rows))
         mcxErr(me, "error parsing header file")
      ,  mcxExit(1)

   ;  if (single_data_file)         /* dangersign, hackish */
      mcxIOexpect(xf_raw, "begin", EXIT_ON_FAIL)

   ;  mx = mclxAllocZero(dom_cols, dom_rows)

   ;  if (xf_map)
         cmap = mclxRead(xf_map, EXIT_ON_FAIL)
      ,  rmap = cmap
      ,  mcxIOclose(xf_map)
   ;  else
      {  if (xf_cmap)
         cmap = mclxRead(xf_cmap, EXIT_ON_FAIL)
      ;  if (xf_rmap)
         rmap = mclxRead(xf_rmap, EXIT_ON_FAIL)
   ;  }

      mclxaSubReadRaw
         (  xf_raw
         ,  mx
         ,  dom_cols
         ,  dom_rows
         ,  EXIT_ON_FAIL
         ,  EODATA
         ,  warn_repeat
         ,  rawtransform ? mclgTFgetEdgePar(rawtransform) : NULL
         ,  ivpmerge
         ,  fltvecbinary
         )
      ,  mcxIOclose(xf_raw)

   ;  if (cmap && mclxMapCols(mx, cmap))
      mcxDie(1, me, "could not map columns")
   ;  if (rmap && mclxMapRows(mx, rmap))
      mcxDie(1, me, "could not map rows")

   ;  if (prmtransform)
      mclgTFexec(mx, prmtransform)

   ;  if (xf_prm)
      mclxWrite(mx, xf_prm, digits, EXIT_ON_FAIL)

   ;  if (put_sym)
      {  tp = mclxTranspose(mx)
      ;  sym = mclxBinary(mx, tp, fltmxbinary)
      ;  if (symtransform)
         mclgTFexec(sym, symtransform)
      ;  mclxWrite(sym, xf_sym, digits, EXIT_ON_FAIL)
      ;  mclxFree(&sym)
               /* not using mclxMergeTranspose because mx is still used */
   ;  }

      if (write_skw || check_sym)
      {  double minone = -1.0
      ;  long n
      ;  tp = tp ? tp : mclxTranspose(mx)
      ;  mclxUnary(tp, fltxMul, &minone)
      ;  skew = mclxAdd(mx, tp)        /* could use mclxMergeTranspose(mx, fltSubtract, 1.0) */
      ;  if (write_skw)
         mclxWrite(skew, xf_skew, digits, EXIT_ON_FAIL)
      ;  n = mclxNrofEntries(skew)
      ;  fprintf(stdout, "symmetry check: %ld skew edges\n", (long) (n/2))
   ;  }

      if (qmode)
      mcxFree(qmode)

   ;  mcxTingFree(&ibase)
   ;  mcxTingFree(&obase)

   ;  if (xf_hdr == xf_raw)
      mcxIOfree(&xf_hdr)
   ;  else
         mcxIOfree(&xf_hdr)
      ,  mcxIOfree(&xf_raw)

   ;  mcxIOfree(&xf_prm)
   ;  mcxIOfree(&xf_map)
   ;  mcxIOfree(&xf_rmap)
   ;  mcxIOfree(&xf_cmap)
   ;  mcxIOfree(&xf_skew)
   ;  mcxIOfree(&xf_sym)

   ;  mclxFree(&mx)
   ;  mclxFree(&tp)
   ;  mclxFree(&sym)
   ;  mclxFree(&skew)

   ;  if (cmap == rmap)
      mclxFree(&cmap)
   ;  else
         mclxFree(&rmap)
      ,  mclxFree(&cmap)

   ;  return 0
;  }


