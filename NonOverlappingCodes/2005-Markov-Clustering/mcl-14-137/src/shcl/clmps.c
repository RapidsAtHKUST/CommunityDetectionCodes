/*   (C) Copyright 2003, 2004, 2005, 2006, 2007, 2008, 2009  Stijn van Dongen
 *   (C) Copyright 2010  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* TODO
 *    Better/easier node colour control.
 *    option to output black nodes.
 *    option to read iterands as stack.
 *    mcl option to write iterands as stack?
 *    __ Optify symmetric/directed.
 *    >> -def edge=edge
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "clm.h"
#include "report.h"
#include "clmps.h"
#include "clmps.defs.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/rand.h"
#include "util/opt.h"
#include "util/opt.h"
#include "util/minmax.h"
#include "util/ding.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/tab.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "mcl/interpret.h"
#include "clew/cat.h"


static const char *me = "clm ps";


enum
{  MY_OPT_ICL  =     CLM_DISP_UNUSED
,  MY_OPT_CLSTACK
,  MY_OPT_NCLMAX
,  MY_OPT_IMX
,  MY_OPT_COORDS
,  MY_OPT_HEADER
,  MY_OPT_OUTPUT
,  MY_OPT_LEVEL
,  MY_OPT_USER
,  MY_OPT_DEF
,  MY_OPT_LISTDEFS
,  MY_OPT_SHOWPAGE
,  MY_OPT_BB
,  MY_OPT_NODEGREY
,  MY_OPT_INFLATION
,  MY_OPT_NORMALIZE
,  MY_OPT_BIN
,  MY_OPT_E_MAX
,  MY_OPT_CAPTION
}  ;


static mcxOptAnchor PSOptions[] =
{  {  "-imx"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "input graph file"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "input cluster file"
   }
,  {  "-cl-stack"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CLSTACK
   ,  "<fname>"
   ,  "assume mclcm-type hierarchical clusterings (stack format)"
   }
,  {  "-cl-height"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NCLMAX
   ,  "<num>"
   ,  "do at most <num> levels of the hierarchy"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "--header"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_HEADER
   ,  NULL
   ,  "write header"
   }
,  {  "-l"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_LEVEL
   ,  "<level>"
   ,  "clustering/tree level"
   }
,  {  "-I"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_INFLATION
   ,  "<num>"
   ,  "apply inflation to input graph"
   }
,  {  "-coords"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_COORDS
   ,  "fname"
   ,  "read and output file with PostScript coordinates"
   }
,  {  "-def"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DEF
   ,  "name=val"
   ,  "set PS parameter name to val"
   }
,  {  "--list-defs"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LISTDEFS
   ,  NULL
   ,  "list parameters settable with -def"
   }
,  {  "--showpage"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_SHOWPAGE
   ,  NULL
   ,  "output showpage (consider image finished)"
   }
,  {  "--bb"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_BB
   ,  NULL
   ,  "draw bounding box"
   }
,  {  "-node-grey"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NODEGREY
   ,  "<num>"
   ,  "PostScript grey level"
   }
,  {  "-usr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_USER
   ,  "PS defs"
   ,  "PS defs included in each page"
   }
,  {  "-bin"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_BIN
   ,  "offset,limit,#bins"
   ,  "specify bin layout"
   }
,  {  "-normalize"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NORMALIZE
   ,  "y/n"
   ,  "make stochastic yes/no (default yes)"
   }
,  {  "-caption"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CAPTION
   ,  "<string>"
   ,  ""
   }
,  {  "-e-max"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_E_MAX
   ,  "<num>"
   ,  ""
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxTing *usr_defs=  (void*) -1;
static mcxIO*  xfout    =  (void*) -1;
static mcxIO   *xfcl    =  (void*) -1;
static mcxIO   *xfmx    =  (void*) -1;
static mcxIO   *xfcoords=  (void*) -1;
static mcxIO* xfstack   =  (void*) -1;

static const char* caption =  (void*) -1;
static double inflation    =  -1;
static mcxbool normalize   =  -1;
static mcxbool showpage    =  -1;
static mcxbool header      =  -1;
static dim     e_max       =  -1;
static double val_ofs      =  -1;
static double val_lim      =  -1;
static dim    n_cl_max     =  -1;
static int    n_bins       =  -1;
static int    level        =  -1;
static dim  n_usr_params   =  -1;


int kvcmp
(  const void* keyval1
,  const void* keyval2
)
   {  const char* const* kv1 = keyval1
   ;  const char* const* kv2 = keyval2
   ;  return strcmp(kv1[0], kv2[0])
;  }


const char* usr_params[][2] =
{  {  "llx"         ,  "10"        }
,  {  "lly"         ,  "10"        }
,  {  "urx"         ,  "585"       }
,  {  "ury"         ,  "832"       }
,  {  "scx"         ,  "15"        }
,  {  "scy"         ,  "15"        }
,  {  "bbtest"      ,  "0"         }
,  {  "shownodes"   ,  "1"         }

,  {  "rmax"        ,  "0.54"      }
,  {  "gmax"        ,  "0.0"       }
,  {  "bmax"        ,  "0.0"       }

,  {  "rmin"        ,  "1.0"       }
,  {  "gmin"        ,  "0.64"      }
,  {  "bmin"        ,  "0.0"       }

,  {  "edge"        ,  "2edge"     }

,  {  "displacement",  "0.1"       }
,  {  "radius"      ,  "0.15"      }
,  {  "snarewidth"  ,  "0.06"      }
,  {  "snarecolor"  ,  "0 0 0"     }
,  {  "edgewidth"   ,  "0.06"      }
,  {  "edgecolor"   ,  "0.5 0.5 0.5"}
,  {  "arcwidth"    ,  "0.06"      }
,  {  "boundwidth"  ,  "0"         }
,  {  NULL          ,  NULL        }
}  ;


const char* ps_header[] =
{  "%!PS-Adobe-3.0"
,  "%%BoundingBox: usr_llx usr_lly usr_urx usr_ury"
,  "/BBLL [usr_llx usr_lly] def"
,  "/BBUR [usr_urx usr_ury] def"
,  "/B_test usr_bbtest def"
,  "/color_min [usr_rmin usr_gmin usr_bmin] def"
,  "/color_max [usr_rmax usr_gmax usr_bmax] def"
,  "/color_xcn [] def"
,  "/displacement usr_displacement def"
,  "/radius usr_radius def"
,  ""
,  "/snarewidth usr_snarewidth def"
,  "/snarecolor [usr_snarecolor] def"
,  ""
,  "/edgewidth usr_edgewidth def"
,  "/edgecolor [usr_edgecolor] def"
,  "/arcwidth usr_arcwidth def"
,  "/boundwidth usr_boundwidth def"
,  ""
,  "/shownodes usr_shownodes def"
,  ""
,  NULL
}  ;


const char* ps_bind[] =
{  "/e { proc_usr_edge } bind def"
,  "/n { proc_circle } bind def"
,  "/cn { proc_circle_colour } bind def"
,  ""
,  NULL
}  ;


void write_defs
(  FILE* fp
)
   {  const char** pp =  defs_ps
   ;  while (*pp)
      {  fputs(*pp, fp)
      ;  fputc('\n', fp)
      ;  pp++
   ;  }
;  }


void write_params
(  FILE* fp
,  const char** pslines
)
   {  const char** line = pslines+0
   ;  const char* kvhook[2] = { NULL, NULL }
   ;  const char** kv
   ;  char buf[100]
   ;  while (line[0])
      {  const char* o = line[0]
      ;  const char* p = strstr(o, "usr_")
      ;  while (p)
         {  fprintf(fp, "%.*s", (int) (p-o), o)
         ;  if (1 != sscanf(p+4, "%[a-z]", buf))
            mcxDie(1, me, "parse error")
         ;  kvhook[0] = buf
         ;  if (!(kv = bsearch(kvhook, usr_params, n_usr_params, sizeof usr_params[0], kvcmp)))
            mcxDie(1, me, "unexpected unknown parameter %s", buf)
         ;  fputs(kv[1], fp)
         ;  o = p+4+strlen(buf)
         ;  p = strstr(o, "usr_")
      ;  }
         fputs(o, fp)
      ;  fputc('\n', fp)
      ;  line++
   ;  }
   }


void list_params
(  void
)
   {  int n   =  0
   ;  while (usr_params[n][0])
      {  fprintf(stdout, "%20s  %s\n", usr_params[n][0], usr_params[n][1])
      ;  n++
   ;  }
   }


static mcxstatus set_param
(  const char* spec
)
   {  const char* p = strchr(spec, '=')
   ;  mcxTing* key
   ;  const char* kvhook[2]  = { NULL, NULL }
   ;  const char** kv
   ;  if (!p || p[1] == '\0')
      return STATUS_FAIL
   ;  key = mcxTingNNew(spec, p-spec)
   ;  kvhook[0] = key->str

   ;  kv = bsearch(kvhook, usr_params, n_usr_params, sizeof usr_params[0], kvcmp)

   ;  if (kv)
      {  char* q = strdup(p+1)
      ;  char* qq = q

      ;  if (strstr(key->str, "color"))
         {  while (*q)
            {  if ((unsigned char) q[0] == ',')
               q[0] = ' '
            ;  q++
         ;  }
         }

         kv[1] = qq
   ;  }

      mcxTingFree(&key)
   ;  return kv ? STATUS_OK : STATUS_FAIL
;  }


static mcxstatus PSInit
(  void
)
   {  xfout          =  mcxIOnew("-", "w")
   ;  xfmx           =  NULL
   ;  xfcoords       =  NULL
   ;  xfstack        =  NULL
   ;  xfcl           =  NULL
   ;  usr_defs       =  mcxTingEmpty(NULL, 200)
   ;  inflation      =  1.0
   ;  normalize      =  TRUE
   ;  header         =  FALSE
   ;  showpage       =  FALSE
   ;  n_cl_max       =  4
   ;  e_max          =  1 << 15
   ;  caption        =  ""
   ;  val_ofs        =  0.0
   ;  val_lim        =  1.0
   ;  n_bins         =  20
   ;  level          =  1

   ;  n_usr_params   =  0
   ;  while (usr_params[n_usr_params][0])
      n_usr_params++
   ;  qsort(usr_params, n_usr_params, sizeof usr_params[0], kvcmp)

   ;  return STATUS_OK
;  }


static mcxstatus PSArgHandle
(  int optid
,  const char* val
)
   {  mcxTing* scratch = mcxTingEmpty(NULL, 20)
   ;  mcxbits status = 0
   ;  switch(optid)
      {  case MY_OPT_ICL
      :  xfcl =  mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_CLSTACK
      :  xfstack = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_NCLMAX
      :  n_cl_max = atoi(val)
      ;  break
      ;

         case MY_OPT_COORDS
      :  xfcoords = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_IMX
      :  xfmx = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_USER
      :  mcxTingPrintAfter(usr_defs, "%s\n", val)
      ;  break
      ;

         case MY_OPT_DEF
      :  if (set_param(val))
         mcxDie(1, me, "unsettable: %s", val)
      ;  break
      ;

         case MY_OPT_LISTDEFS
      :  list_params()
      ;  exit(0)
      ;  break
      ;

         case MY_OPT_SHOWPAGE
      :  showpage = TRUE
      ;  break
      ;

         case MY_OPT_BB
      :  set_param("bbtest=1")
      ;  break
      ;

         case MY_OPT_NODEGREY
      :  
         mcxTingPrint(scratch, "rmin=%s", val)
      ;  status |= set_param(scratch->str)
      ;  mcxTingPrint(scratch, "gmin=%s", val)
      ;  status |= set_param(scratch->str)
      ;  mcxTingPrint(scratch, "bmin=%s", val)
      ;  status |= set_param(scratch->str)
      ;  mcxTingPrint(scratch, "rmax=%s", val)
      ;  status |= set_param(scratch->str)
      ;  mcxTingPrint(scratch, "gmax=%s", val)
      ;  status |= set_param(scratch->str)
      ;  mcxTingPrint(scratch, "bmax=%s", val)
      ;  status |= set_param(scratch->str)
      ;  if (status)
         mcxDie(1, me, "unsettable grey")
      ;  break
      ;

         case MY_OPT_LEVEL
      :  level = atoi(val)
      ;  break
      ;

         case MY_OPT_HEADER
      :  header = TRUE
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         case MY_OPT_BIN
      :  if (3 != sscanf(val, "%lf,%lf,%d", &val_ofs, &val_lim, &n_bins))
         mcxDie(1, me, "-bin accepts lo,hi,N format")
      ;  break
      ;

         case MY_OPT_NORMALIZE
      :  normalize = strchr("1yY", val[0]) ? TRUE : FALSE
      ;  break
      ;

         case MY_OPT_INFLATION
      :  inflation = atof(val)
      ;  break
      ;

         case MY_OPT_E_MAX
      :  e_max = atoi(val)
      ;  break
      ;

         case MY_OPT_CAPTION
      :  caption = val
      ;  break
      ;

         default
      :  return STATUS_FAIL
   ;  }

      mcxTingFree(&scratch)
   ;  return STATUS_OK
;  }



static mcxstatus PSMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclx *mx = NULL, *mt = NULL
   ;  mclv* sums = NULL, *diagv = NULL
   ;  mcxTing* ting = mcxTingEmpty(NULL, 80)
   ;  mcxTing* coords = mcxTingEmpty(NULL, 1000)
   ;  int l
   ;  dim i

   ;  if (!xfmx && !xfcl)
      mcxDie(1, me, "need matrix file and/or cluster file")

   ;  if (xfcoords && mcxIOreadFile(xfcoords, coords))
      mcxDie(1, me, "cannot read coordinates from %s", xfcoords->fn->str)

   ;  srandom(mcxSeed(2308947))

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  if (header)
      {  write_params(xfout->fp, ps_header)
      ;  write_defs(xfout->fp)
      ;  write_params(xfout->fp, ps_bind)
      ;  mcxIOclose(xfout)
      ;  return 0
   ;  }

      if (coords->len)
      fputs(coords->str, xfout->fp)

   ;  if (xfmx)
      {  mx =  mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH)

      ;  if (normalize)
         mclxMakeStochastic(mx)

      ;  mt =  mclxTranspose(mx)

      ;  sums = mclxColSums(mt, MCL_VECTOR_COMPLETE)

      ;  if (inflation != 1.0)
         mclxInflate(mx, inflation)

      ;  diagv = mclxDiagValues(mx, MCL_VECTOR_COMPLETE)

      ;  while (STATUS_OK == mcxIOreadLine(xfmx, ting, MCX_READLINE_DEFAULT))
         {  const char* c = mcxStrChrAint(ting->str, isspace, ting->len)
         ;  if (!c)
            continue
         ;  if (*c == '/' || *c == '%')
            fputs(ting->str, xfout->fp)
      ;  }

         fputc('\n', xfout->fp)
      ;  fprintf(xfout->fp, "%s\n", usr_defs->str)

      ;  for (l=0;val_ofs<val_lim && l<n_bins;l++)
         {  double lo = val_ofs + l*(val_lim-val_ofs)/n_bins
         ;  double hi = val_ofs + (l+1)*(val_lim-val_ofs)/n_bins
         ;  int n_put = 0

         ;  for (i=0;i<N_COLS(mx);i++)
            {  dim j
            ;  mclv* vec = mx->cols+i
            ;  if (e_max)
               mclvSort(vec, mclpValRevCmp)     /* fixme to much sorting */
            ;  for (j=0;j<vec->n_ivps&&(!e_max||j<e_max);j++)
               {  mclp* ivp = vec->ivps+j
               ;  double w_out = ivp->val
               ;  double w_in, w_max

               ;  mclp* ivp_t = mclvGetIvp(mt->cols+i, ivp->idx, NULL)

               ;  if (vec->vid == ivp->idx)
                  continue

;if(0)fprintf(stderr, "%%> pair %ld %ld %.2f\n", (long) vec->vid, (long) ivp->idx, (double) ivp->val)

               ;  w_in = ivp_t ? ivp_t->val : 0.0
               ;  w_max = MCX_MAX(w_in, w_out)

               ;  if (!(lo < w_max && w_max <= hi))
                  continue

               ;  fprintf
                  (  xfout->fp
                  , "v%ld v%ld %.4f %.4f e\n"
                  ,  (long) ivp->idx, (long) vec->vid
                  ,  ceil(1000.0 * w_in) / 1000.0
                  ,  ceil(1000.0 * w_out) / 1000.0
                  )
               ;  n_put++
            ;  }
               if (e_max)
               mclvSort(vec, mclpIdxCmp)
         ;  }

fprintf(stderr, "%%> level %.2f-%.2f put %d\n", lo, hi, (int) n_put)
;
            if (n_put)
            fputc('\n', xfout->fp)
      ;  }

                        /* fixme: document xfstack semantics */
         if (!xfstack)
         {  for (i=0;i<sums->n_ivps;i++)
            fprintf
            (  xfout->fp
            ,  "v%ld %.2f n\n"
            ,  (long) sums->ivps[i].idx
            ,  (double) sums->ivps[i].val
            )
                  /* ^ fixme: uncouple from !xfstack */
         ;  fprintf(xfout->fp, "/caption (%s) def\n\n", caption)
      ;  }
      }

      if (xfstack)
      {  mclxCat st
      ;  mclx* cl = NULL, *cltp = NULL, *colors = NULL
      ;  dim i,j,k
      ;  mcxbits bits   =     MCLX_REQUIRE_DOMSTACK
                           |  MCLX_REQUIRE_CANONICAL
                           |  MCLX_PRODUCE_PARTITION

      ;  mclxCatInit(&st)
      ;  if (mclxCatRead(xfstack, &st, n_cl_max, NULL, NULL, bits))
         mcxDie(1, me, "error reading stack")

      ;  for (i=0;i<st.n_level;i++)
         {  int lev = i+1
         ;  cl = st.level[i].mx
         ;  cltp = mclxTranspose(cl)

         ;  if (mx && !MCLD_EQUAL(mx->dom_cols, cltp->dom_cols))
            mcxDie(1, me, "domain mismatch")
         ;  colors = mclxCartesian(cl->dom_cols, mclvCanonical(NULL, 3, 1.0), 1.0)

         ;  fputc('\n', stdout)

         ;  for (j=0;j<N_COLS(colors);j++)
            {  for (k=0;k<3;k++)
               {  double f = (1.0 * (unsigned long) random()) / RAND_MAX
               ;  double g = ((3*j+k) % 5) && ((3*j+k) % 7) ? sqrt(f) : f * f
               ;  colors->cols[j].ivps[k].val = g
;fprintf(stderr, "[%.2f,%.2f]", f, g)
            ;  }
               fprintf
               (  xfout->fp
               ,  "/clcolor%dlev%d [%.2f %.2f %.2f] def\n"
               ,  (int) j
               ,  lev
               ,  colors->cols[j].ivps[0].val
               ,  colors->cols[j].ivps[1].val
               ,  colors->cols[j].ivps[2].val
               )
;fputc('\n', stderr)
         ;  }

            fputc('\n', stdout)
         ;  fprintf
            (  stdout
            ,  "/clensembl%d {\n\n"
            ,  lev
            )

         ;  for (j=0;j<N_COLS(cl);j++)
            {  mclv* clus = cl->cols+j
            ;  for (k=0;k<clus->n_ivps;k++)
               {  long nid = clus->ivps[k].idx
               ;  mclv* cv
                  =  mclxGetVector
                     (  colors
                     ,  clus->vid
                     ,  EXIT_ON_FAIL
                     ,  NULL
                     )
               ;  fprintf
                  (  xfout->fp
                  ,  "v%ld clcolor%dlev%d %d cn\n"
                  ,  (long) nid
                  ,  (int) cv->vid
                  ,  lev
                  ,  lev
                  )
            ;  }
               fputc('\n', stdout)
         ;  }

            fputs("} def\n\n", stdout)

         ;  mclxFree(&colors)
         ;  mclxFree(&cltp)
      ;  }
      }

      if (showpage)
      fputs("\nshowpage\n\n", xfout->fp)

   ;  mcxIOclose(xfout)
   ;  return 0
;  }



static mcxDispHook PSEntry
=  {  "ps"
   ,  "ps [options] -imx <mx file>"
   ,  PSOptions
   ,  sizeof(PSOptions)/sizeof(mcxOptAnchor) - 1
   ,  PSArgHandle
   ,  PSInit
   ,  PSMain
   ,  0
   ,  0
   ,  MCX_DISP_HIDDEN
   }
;


mcxDispHook* mcxDispHookPS
(  void
)
   {  return &PSEntry
;  }


