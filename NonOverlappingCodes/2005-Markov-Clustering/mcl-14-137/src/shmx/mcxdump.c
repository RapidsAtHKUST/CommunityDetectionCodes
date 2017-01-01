/*   (C) Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *   (C) Copyright 2010, 2011, 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* TODO
*/

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "impala/io.h"
#include "impala/stream.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/tab.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/minmax.h"

#include "clew/cat.h"
#include "mcl/transform.h"

const char* me = "mcxdump";
const char* sep_lead_g = "\t";
const char* sep_row_g = "\t";
const char* sep_val_g = ":";
const char* sep_cat_g = "===";
const char* prefixc_g = "";
const char* sif_g     = NULL;

const char* syntax = "Usage: mcxdump -imx <fname> [-o <fname>] [options]";


enum
{  MY_OPT_IMX
,  MY_OPT_ICL
,  MY_OPT_CAT
,  MY_OPT_TREECAT
,  MY_OPT_OUTPUT
,  MY_OPT_TAB
,  MY_OPT_TABC
,  MY_OPT_TABR
,  MY_OPT_LAZY_TAB
,  MY_OPT_RESTRICT_TAB
,  MY_OPT_RESTRICT_TABC
,  MY_OPT_RESTRICT_TABR
,  MY_OPT_NO_VALUES
,  MY_OPT_PREFIXC
,  MY_OPT_NO_LOOPS
,  MY_OPT_OMIT_EMPTY
,  MY_OPT_SORT
,  MY_OPT_FORCE_LOOPS
,  MY_OPT_CAT_LIMIT
,  MY_OPT_SPLIT_STEM
,  MY_OPT_DUMP_MATRIX
,  MY_OPT_TRANSPOSE
,  MY_OPT_TRANSFORM
,  MY_OPT_DUMP_PAIRS
,  MY_OPT_DUMP_UPPER
,  MY_OPT_DUMP_UPPERI
,  MY_OPT_DUMP_LOWER
,  MY_OPT_DUMP_LOWERI
,  MY_OPT_DUMP_LINES
,  MY_OPT_DUMP_RLINES
,  MY_OPT_DUMP_VLINES
,  MY_OPT_DUMP_SIF
,  MY_OPT_DUMP_SIFX
,  MY_OPT_DUMP_JSON
,  MY_OPT_DUMP_NEWICK
,  MY_OPT_NEWICK_MODE
,  MY_OPT_DUMP_TABLE
,  MY_OPT_TABLE_NFIELDS
,  MY_OPT_TABLE_NLINES
,  MY_OPT_DUMP_NOLEAD
,  MY_OPT_DIGITS
,  MY_OPT_WRITE_TABC
,  MY_OPT_WRITE_TABR
,  MY_OPT_WRITE_TABR_SHADOW
,  MY_OPT_DUMP_RDOM
,  MY_OPT_DUMP_CDOM
,  MY_OPT_SKEL
,  MY_OPT_SEP_LEAD
,  MY_OPT_SEP_FIELD
,  MY_OPT_SEP_VAL
,  MY_OPT_SEP_CAT
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
,  {  "--lazy-tab"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LAZY_TAB
   ,  NULL
   ,  "tab file(s) may mismatch matrix domain(s)"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "read matrix from file <fname>"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "read clustering from file <fname>, dump lines"
   }
,  {  "-imx-cat"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CAT
   ,  "<fname>"
   ,  "dump multiple matrices encoded in cat file"
   }
,  {  "-imx-tree"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TREECAT
   ,  "<fname>"
   ,  "stackify and dump multiple matrices encoded in cone file"
   }
,  {  "--skeleton"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SKEL
   ,  NULL
   ,  "create empty matrix, honour domains"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "read tab file from <fname> for all identifiers"
   }
,  {  "-tabc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TABC
   ,  "<fname>"
   ,  "read tab file from <fname> for column domain identifiers"
   }
,  {  "-tabr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TABR
   ,  "<fname>"
   ,  "read tab file from <fname> for row domain identifiers"
   }
,  {  "-restrict-tab"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_RESTRICT_TAB
   ,  "<fname>"
   ,  "restrict matrix to tab domain"
   }
,  {  "-restrict-tabc"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_RESTRICT_TABC
   ,  "<fname>"
   ,  "restrict matrix column domain to tab domain"
   }
,  {  "-restrict-tabr"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_RESTRICT_TABR
   ,  "<fname>"
   ,  "restrict matrix row domain to tab domain"
   }
,  {  "--no-values"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_NO_VALUES
   ,  NULL
   ,  "do not emit values"
   }
,  {  "-prefixc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PREFIXC
   ,  "<string>"
   ,  "prefix column indices with <string>"
   }
,  {  "--omit-empty"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_OMIT_EMPTY
   ,  NULL
   ,  "skip columns with no entries"
   }
,  {  "--no-loops"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_NO_LOOPS
   ,  NULL
   ,  "do not include self in listing"
   }
,  {  "--force-loops"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_FORCE_LOOPS
   ,  NULL
   ,  "force self in listing"
   }
,  {  "-sort"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SORT
   ,  "size-{ascending,descending}"
   ,  "sort mode"
   }
,  {  "--write-matrix"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_MATRIX
   ,  NULL
   ,  "dump mcl matrix"
   }
,  {  "-split-stem"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SPLIT_STEM
   ,  "<file-name-stem>"
   ,  "split multiple matrices over different files"
   }
,  {  "-cat-max"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CAT_LIMIT
   ,  "<num>"
   ,  "only do the first <num> files"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output to file <fname> (- for STDOUT)"
   }
,  {  "-digits"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DIGITS
   ,  "<int>"
   ,  "precision to use in interchange format"
   }
,  {  "--write-tabr"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITE_TABR
   ,  NULL
   ,  "write tab file on row domain"
   }
,  {  "--write-tabc"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WRITE_TABC
   ,  NULL
   ,  "write tab file on column domain"
   }
,  {  "--write-tabr-shadow"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_WRITE_TABR_SHADOW
   ,  NULL
   ,  "write shadow tab file on row domain"
   }
,  {  "--dump-domr"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_RDOM
   ,  NULL
   ,  "dump the row domain"
   }
,  {  "--dump-domc"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_CDOM
   ,  NULL
   ,  "dump the col domain"
   }
,  {  "--dump-pairs"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_PAIRS
   ,  NULL
   ,  "dump a single column/row matrix pair per output line"
   }
,  {  "--dump-upper"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_UPPER
   ,  NULL
   ,  "dump upper part of the matrix excluding diagonal"
   }
,  {  "--dump-upperi"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_UPPERI
   ,  NULL
   ,  "dump upper part of the matrix including diagonal"
   }
,  {  "--dump-lower"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_LOWER
   ,  NULL
   ,  "dump lower part of the matrix excluding diagonal"
   }
,  {  "--dump-loweri"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_LOWERI
   ,  NULL
   ,  "dump lower part of the matrix including diagonal"
   }
,  {  "--json"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_DUMP_JSON
   ,  NULL
   ,  "write JSON string"
   }
,  {  "--newick"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_NEWICK
   ,  NULL
   ,  "write newick string"
   }
,  {  "-newick"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NEWICK_MODE
   ,  "[NBIS]+"
   ,  "no Number, no Branch length, no Indent, no Singleton parentheses"
   }
,  {  "--dump-table"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_TABLE
   ,  NULL
   ,  "dump complete matrix including zeroes"
   }
,  {  "-table-nlines"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TABLE_NLINES
   ,  "<num>"
   ,  "limit table dump to first <num> lines"
   }
,  {  "-table-nfields"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TABLE_NFIELDS
   ,  "<num>"
   ,  "limit table dump to first <num> fields"
   }
,  {  "--dump-lead-off"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_NOLEAD
   ,  NULL
   ,  "do not dump lead node (with --dump-table)"
   }
,  {  "--transpose"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TRANSPOSE
   ,  NULL
   ,  "work with the transposed matrix"
   }
,  {  "-tf"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TRANSFORM
   ,  "<func(arg)[, func(arg)]*>"
   ,  "apply unary transformations to input matrix values"
   }
,  {  "--dump-lines"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_LINES
   ,  NULL
   ,  "join all row entries on a single line"
   }
,  {  "--dump-vlines"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_VLINES
   ,  NULL
   ,  "join all row entries on a single line, print column value"
   }
,  {  "--dump-rlines"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_DUMP_RLINES
   ,  NULL
   ,  "as --dump-lines, do not emit the leading column identifier"
   }
,  {  "-dump-sif"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DUMP_SIF
   ,  "<relationship-type>"
   ,  "dump SIF format with second field set to <relationship-type>"
   }
,  {  "-dump-sifx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DUMP_SIFX
   ,  "<relationship-type>"
   ,  "as -dump-sif, output extended label:weight format"
   }
,  {  "-sep-cat"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SEP_CAT
   ,  "<string>"
   ,  "use <string> to separate cat matrix dumps (cf -imx-cat)"
   }
,  {  "-sep-lead"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SEP_LEAD
   ,  "<string>"
   ,  "use <string> to separate col from row list (default tab)"
   }
,  {  "-sep-field"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SEP_FIELD
   ,  "<string>"
   ,  "use <string> to separate row indices (default tab)"
   }
,  {  "-sep-value"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SEP_VAL
   ,  "<string>"
   ,  "use <string> as node/value separator (default colon)"
   }
,  {  NULL, 0, 0, NULL, NULL  }
}  ;

int main
(  int                  argc
,  const char*          argv[]
)
   {  mcxIO* xf_tab     =  NULL
   ;  mcxIO* xf_tabr    =  NULL
   ;  mcxIO* xf_tabc    =  NULL
   ;  mcxIO* xf_restrict_tab     =  NULL
   ;  mcxIO* xf_restrict_tabr    =  NULL
   ;  mcxIO* xf_restrict_tabc    =  NULL
   ;  mcxIO* xf_mx      =  mcxIOnew("-", "r")
   ;  mcxIO* xfout    =  NULL
   ;  const char*  fndump  =  "-"
   ;  mclTab* tabr      =  NULL
   ;  mclTab* tabc      =  NULL
   ;  mclTab* restrict_tabr =  NULL
   ;  mclTab* restrict_tabc =  NULL
   ;  mcxbool transpose =  FALSE
   ;  mcxbool lazy_tab  =  FALSE
   ;  mcxbool write_tabc =  FALSE
   ;  mcxbool write_tabr =  FALSE
   ;  mcxbool cat       =  FALSE
   ;  mcxbool tree      =  FALSE
   ;  mcxbool skel      =  FALSE
   ;  mcxbool newick    =  FALSE
   ;  mcxbool json      =  FALSE
   ;  mcxbits newick_bits = 0
   ;  mcxbits cat_bits  =  0
   ;  dim catmax        =  1
   ;  dim n_max         =  0
   ;  dim table_nlines  =  0
   ;  dim table_nfields =  0
   ;  int split_idx     =  1
   ;  int split_inc     =  1
   ;  const char* split_stem =  NULL
   ;  const char* sort_mode = NULL
   ;  mcxTing* line     =  mcxTingEmpty(NULL, 10)
   ;  mclgTF* tf_result =   NULL
   ;  mcxTing* tf_result_spec = NULL

   ;  mcxbits modes     =  MCLX_DUMP_VALUES

   ;  mcxbits mode_dump =  MCLX_DUMP_PAIRS
   ;  mcxbits mode_part =  0
   ;  mcxbits mode_loop =  MCLX_DUMP_LOOP_ASIS
   ;  mcxbits mode_matrix = 0
   ;  int digits        =  MCLXIO_VALUE_GETENV

   ;  mcxOption* opts, *opt
   ;  mcxstatus parseStatus = STATUS_OK

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
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

            case MY_OPT_TAB
         :  xf_tab = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_TABC
         :  xf_tabc = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_TABR
         :  xf_tabr = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_OUTPUT
         :  fndump = opt->val
         ;  break
         ;

            case MY_OPT_SEP_LEAD
         :  sep_lead_g = opt->val
         ;  break
         ;

            case MY_OPT_SEP_FIELD
         :  sep_row_g = opt->val
         ;  break
         ;

            case MY_OPT_SEP_CAT
         :  sep_cat_g = opt->val
         ;  break
         ;

            case MY_OPT_SEP_VAL
         :  sep_val_g = opt->val
         ;  break
         ;

            case MY_OPT_PREFIXC
         :  prefixc_g = opt->val
         ;  break
         ;

            case MY_OPT_RESTRICT_TAB
         :  xf_restrict_tab = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_RESTRICT_TABC
         :  xf_restrict_tabc = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_RESTRICT_TABR
         :  xf_restrict_tabr = mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_LAZY_TAB
         :  lazy_tab = TRUE
         ;  break
         ;

            case MY_OPT_NO_VALUES
         :  BIT_OFF(modes, MCLX_DUMP_VALUES)
         ;  break
         ;

            case MY_OPT_DUMP_SIFX
         :  sif_g = opt->val
         ;  mode_dump = MCLX_DUMP_LINES
         ;  break
         ;

            case MY_OPT_DUMP_SIF
         :  sif_g = opt->val
         ;  mode_dump = MCLX_DUMP_LINES
         ;  BIT_OFF(modes, MCLX_DUMP_VALUES)
         ;  break
         ;

            case MY_OPT_DUMP_RLINES
         :  mode_dump = MCLX_DUMP_LINES
         ;  BIT_ON(modes, MCLX_DUMP_NOLEAD)
         ;  break
         ;

            case MY_OPT_DUMP_VLINES
         :  mode_dump = MCLX_DUMP_LINES
         ;  BIT_ON(modes, MCLX_DUMP_LEAD_VALUE)
         ;  break
         ;

            case MY_OPT_DUMP_LINES
         :  mode_dump = MCLX_DUMP_LINES
         ;  break
         ;

            case MY_OPT_OMIT_EMPTY
         :  BIT_ON(modes, MCLX_DUMP_OMIT_EMPTY)
         ;  break
         ;

            case MY_OPT_SORT
         :  sort_mode = opt->val
         ;  break
         ;

            case MY_OPT_NO_LOOPS
         :  mode_loop = MCLX_DUMP_LOOP_NONE
         ;  break
         ;

            case MY_OPT_CAT_LIMIT
         :  n_max = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_SPLIT_STEM
         :  split_stem = opt->val
         ;  sep_cat_g = NULL
         ;  break
         ;

            case MY_OPT_FORCE_LOOPS
         :  mode_loop = MCLX_DUMP_LOOP_FORCE
         ;  break
         ;

            case MY_OPT_SKEL
         :  skel = TRUE
         ;  break
         ;

            case MY_OPT_WRITE_TABC
         :  write_tabc = TRUE
         ;  break
         ;

            case MY_OPT_DIGITS
         :  digits = strtol(opt->val, NULL, 10)
         ;  break
         ;

            case MY_OPT_WRITE_TABR
         :  write_tabr = TRUE
         ;  break
         ;

            case MY_OPT_DUMP_RDOM
         :  transpose = TRUE
         ;  skel = TRUE
         ;  mode_dump = MCLX_DUMP_LINES
         ;  break
         ;

            case MY_OPT_DUMP_CDOM
         :  skel = TRUE
         ;  mode_dump = MCLX_DUMP_LINES
         ;  break
         ;

            case MY_OPT_IMX
         :  mcxIOnewName(xf_mx, opt->val)
         ;  break
         ;

            case MY_OPT_ICL
         :  mcxIOnewName(xf_mx, opt->val)
         ;  mode_dump = MCLX_DUMP_LINES
         ;  BIT_ON(modes, MCLX_DUMP_NOLEAD)
         ;  BIT_OFF(modes, MCLX_DUMP_VALUES)
         ;  break
         ;

            case MY_OPT_TREECAT
         :  mcxIOnewName(xf_mx, opt->val)
         ;  tree = TRUE
         ;  cat_bits |= MCLX_PRODUCE_DOMSTACK
         ;  break
         ;

            case MY_OPT_CAT
         :  mcxIOnewName(xf_mx, opt->val)
         ;  cat = TRUE
         ;  break
         ;

            case MY_OPT_DUMP_MATRIX
         :  mode_matrix |= MCLX_DUMP_MATRIX
         ;  break
         ;

            case MY_OPT_TRANSFORM
         :  tf_result_spec = mcxTingNew(opt->val)
         ;  break
         ;

            case MY_OPT_TRANSPOSE
         :  transpose = TRUE
         ;  break
         ;

            case MY_OPT_DUMP_UPPER
         :  mode_part = MCLX_DUMP_PART_UPPER
         ;  break
         ;

            case MY_OPT_DUMP_UPPERI
         :  mode_part = MCLX_DUMP_PART_UPPERI
         ;  break
         ;

            case MY_OPT_DUMP_LOWER
         :  mode_part = MCLX_DUMP_PART_LOWER
         ;  break
         ;

            case MY_OPT_DUMP_LOWERI
         :  mode_part = MCLX_DUMP_PART_LOWERI
         ;  break
         ;

            case MY_OPT_DUMP_NOLEAD
         :  BIT_ON(modes, MCLX_DUMP_NOLEAD)
         ;  break
         ;

            case MY_OPT_NEWICK_MODE
         :  if (strchr(opt->val, 'N'))
            newick_bits |= (MCLX_NEWICK_NONL | MCLX_NEWICK_NOINDENT)
         ;  if (strchr(opt->val, 'I'))
            newick_bits |= MCLX_NEWICK_NOINDENT
         ;  if (strchr(opt->val, 'B'))
            newick_bits |= MCLX_NEWICK_NONUM
         ;  if (strchr(opt->val, 'S'))
            newick_bits |= MCLX_NEWICK_NOPTHS
         ;  newick = TRUE
         ;  break
         ;

            case MY_OPT_DUMP_JSON
         :  json = TRUE
         ;  break
         ;

            case MY_OPT_DUMP_NEWICK
         :  newick = TRUE
         ;  break
         ;

            case MY_OPT_DUMP_TABLE
         :  mode_dump = MCLX_DUMP_TABLE
         ;  break
         ;

            case MY_OPT_TABLE_NFIELDS
         :  table_nfields = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_TABLE_NLINES
         :  table_nlines = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_DUMP_PAIRS
         :  mode_dump = MCLX_DUMP_PAIRS
         ;  break
      ;  }
      }

   ;  if (skel)
      cat_bits |= MCLX_READ_SKELETON

   ;  modes |= mode_loop | mode_dump | mode_part | mode_matrix

   ;  xfout = mcxIOnew(fndump, "w")
   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  mcxIOopen(xf_mx, EXIT_ON_FAIL)

   ;  if (cat || tree)
      catmax = n_max ? n_max : 0

   ;  if ((write_tabc || write_tabr) && !xf_tab)
      mcxDie(1, me, "need a single tab file (-tab option) with --write-tabc or --write-tabr")

   ;  if (xf_tab && mcxIOopen(xf_tab, RETURN_ON_FAIL))
      mcxDie(1, me, "no tab")
   ;  else
      {  if (xf_tabr && mcxIOopen(xf_tabr, RETURN_ON_FAIL))
         mcxDie(1, me, "no tabr")
      ;  if (xf_tabc && mcxIOopen(xf_tabc, RETURN_ON_FAIL))
         mcxDie(1, me, "no tabc")
   ;  }

      {  if (xf_restrict_tab && mcxIOopen(xf_restrict_tab, RETURN_ON_FAIL))
         mcxDie(1, me, "no restriction tab")
      ;  else
         {  if (xf_restrict_tabr && mcxIOopen(xf_restrict_tabr, RETURN_ON_FAIL))
            mcxDie(1, me, "no restriction tabr")
         ;  if (xf_restrict_tabc && mcxIOopen(xf_restrict_tabc, RETURN_ON_FAIL))
            mcxDie(1, me, "no restriction tabc")
      ;  }
                              /* fixme: below is pretty boilerplate, happens in other places as well */
         if (xf_restrict_tab)
         {  if (!(restrict_tabr = mclTabRead (xf_restrict_tab, NULL, RETURN_ON_FAIL)))
            mcxDie(1, me, "error reading restriction tab")
         ;  restrict_tabc = restrict_tabr
         ;  mcxIOclose(xf_restrict_tab)
      ;  }
         else
         {  if (xf_restrict_tabr)
            {  if (!(restrict_tabr = mclTabRead(xf_restrict_tabr, NULL, RETURN_ON_FAIL)))
               mcxDie(1, me, "error reading restriction tabr")
            ;  mcxIOclose(xf_restrict_tabr)
         ;  }
            if (xf_restrict_tabc)
            {  if (!(restrict_tabc = mclTabRead(xf_restrict_tabc, NULL, RETURN_ON_FAIL)))
               mcxDie(1, me, "error reading restriction tabc")
            ;  mcxIOclose(xf_restrict_tabc)
         ;  }
         }
      }

                        /* fixme: restructure code to include bit below */

      if (write_tabc || write_tabr)
      {  mclv* dom_cols = mclvInit(NULL)
      ;  mclv* dom_rows = mclvInit(NULL)
      ;  mclv* dom = write_tabc ? dom_cols : dom_rows

      ;  if (!(tabc =  mclTabRead(xf_tab, NULL, RETURN_ON_FAIL)))
         mcxDie(1, me, "error reading tab file")

      ;  if (mclxReadDomains(xf_mx, dom_cols, dom_rows))
         mcxDie(1, me, "error reading matrix file")
      ;  mcxIOclose(xf_mx)

                                       /* fixme check status */
      ;  mclTabWrite(tabc, xfout, dom, RETURN_ON_FAIL) 

      ;  mcxIOclose(xfout)
      ;  return 0
   ;  }

      if (newick || json)
      {  mcxTing* thetree
      ;  mclxCat  cat

      ;  if (xf_tab && !(tabr =  mclTabRead(xf_tab, NULL, RETURN_ON_FAIL)))
         mcxDie(1, me, "error reading tab file")

      ;  mclxCatInit(&cat)

      ;  if
         (  mclxCatRead
            (  xf_mx
            ,  &cat
            ,  0
            ,  NULL
            ,  tabr ? tabr->domain : NULL
            ,  MCLX_CATREAD_CLUSTERTREE | MCLX_ENSURE_ROOT
            )
         )
         mcxDie(1, me, "failure reading file")
;if(0){mclxCatWrite(xfout, &cat, 0, EXIT_ON_FAIL)
;exit(0);}
      ;  thetree = mclxCatNewick(&cat, tabr, newick_bits)
      ;  fwrite(thetree->str, 1, thetree->len, xfout->fp)
      ;  fputc('\n', xfout->fp)
      ;  mcxIOclose(xfout)
      ;  return 0
   ;  }

      while (1)
      {  mclxIOdumper dumper = { 0 }
      ;  mclxCat    cat
      ;  dim i

      ;  if (xf_tab && !lazy_tab)
         cat_bits |= MCLX_REQUIRE_GRAPH

      ;  mclxCatInit(&cat)

      ;  if (mclxCatRead(xf_mx, &cat, catmax, NULL, NULL, cat_bits))
         break

      ;  for (i=0;i<cat.n_level;i++)
         {  mclx* mx = cat.level[i].mx

         ;  if (restrict_tabr || restrict_tabc)
            {  mclx* sub
            ;  sub
               =  mclxSub
                  (  mx
                  ,  restrict_tabc
                     ?  restrict_tabc->domain
                     :  mx->dom_cols
                  ,  restrict_tabr
                     ?  restrict_tabr->domain
                     :  mx->dom_rows
                  )
            ;  mx = sub
         ;  }
            /* noteme fixme dangersign mx now may violate some 'cat' invariant */

            if (sort_mode)
            {  if (!strcmp(sort_mode, "size-ascending"))
               mclxColumnsRealign(mx, mclvSizeCmp)
            ;  else if (!strcmp(sort_mode, "size-descending"))
               mclxColumnsRealign(mx, mclvSizeRevCmp)
            ;  else
               mcxErr(me, "unknown sort mode <%s>", sort_mode)
            ;  if (catmax != 1)
               mcxErr(me, "-sort option and cat mode may fail or corrupt")
         ;  }

            if (xf_tab && !tabr)
            {  if (!(  tabr = mclTabRead
                       (xf_tab, lazy_tab ? NULL : mx->dom_rows, RETURN_ON_FAIL)
                  ) )
               mcxDie(1, me, "consider using --lazy-tab option")
            ;  tabc = tabr
            ;  mcxIOclose(xf_tab)
         ;  }
            else
            {  if (!tabr && xf_tabr)
               {  if (!(tabr =  mclTabRead
                        (xf_tabr, lazy_tab ? NULL : mx->dom_rows, RETURN_ON_FAIL)
                     ) )
                  mcxDie(1, me, "consider using --lazy-tab option")
               ;  mcxIOclose(xf_tabr)
            ;  }
               if (!tabc && xf_tabc)
               {  if (!( tabc = mclTabRead
                        (xf_tabc, lazy_tab ? NULL : mx->dom_cols, RETURN_ON_FAIL)
                     ) )
                  mcxDie(1, me, "consider using --lazy-tab option")
               ;  mcxIOclose(xf_tabc)
            ;  }
            }

         ;  if (transpose)
            {  mclx* tp = mclxTranspose(mx)
            ;  mclxFree(&mx)
            ;  mx = tp
            ;  if (tabc || tabr)
               {  mclTab* tabt = tabc
               ;  tabc = tabr
               ;  tabr = tabt
            ;  }
            }

         ;  if (tf_result_spec && !(tf_result = mclgTFparse(NULL, tf_result_spec)))
            mcxDie(1, me, "input -tf spec does not parse")

         ;  if (tf_result)
            mclgTFexec(mx, tf_result)

         ;  if (mode_dump == MCLX_DUMP_TABLE)
            BIT_ON(modes, MCLX_DUMP_TABLE_HEADER)

         ;  mclxIOdumpSet(&dumper, modes, sep_lead_g, sep_row_g, sep_val_g)
                                                /* fixme, below is ugly (leaky abstraction) */
         ;  dumper.table_nlines  = table_nlines
         ;  dumper.table_nfields = table_nfields
         ;  dumper.prefixc = prefixc_g
         ;  dumper.siftype = sif_g

         ;  if (split_stem)
            {  mcxTing* ting = mcxTingPrint(NULL, "%s.%03d", split_stem, split_idx)
            ;  mcxIOclose(xfout)
            ;  mcxIOrenew(xfout, ting->str, "w")
            ;  split_idx += split_inc
         ;  }

            if
            (  mclxIOdump
               (  mx
               ,  xfout
               ,  &dumper
               ,  tabc
               ,  tabr
               ,  digits
               ,  RETURN_ON_FAIL
             ) )
            mcxDie(1, me, "something suboptimal")

         ;  mclxFree(&mx)

         ;  if (sep_cat_g && i+1 < cat.n_level)
            fprintf(xfout->fp, "%s\n", sep_cat_g)
      ;  }
         break
   ;  }

      mcxIOfree(&xf_mx)
   ;  mcxIOfree(&xfout)
   ;  mcxIOfree(&xf_tab)
   ;  mcxIOfree(&xf_tabr)
   ;  mcxIOfree(&xf_tabc)
   ;  mcxTingFree(&line)
   ;  return 0
;  }


