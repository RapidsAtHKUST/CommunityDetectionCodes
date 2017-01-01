/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/*
 * TODO
 *    -  warn on strange/wrong combinations of options (shuffle & xxx)
 *    -  support DAGs
 *    -  for small matrices and many additions keep track of
 *          matrix with non-edges (worthwhile?).
 *    -  Poisson distribution, others.
 *    #  prove that Pierre's approach to Gaussians works.
 *    -  pick value according to sampled distribution or removals
 *    -  we use only lower bits for edge selection
 *    -  insertion is slow due to array insertion;
 *          ideally store graph in a hash.
*/

#define DEBUG 0

#include <stdio.h>
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

#include "impala/io.h"
#include "impala/stream.h"
#include "impala/ivp.h"
#include "impala/app.h"
#include "impala/compose.h"
#include "impala/iface.h"

#include "util/types.h"
#include "util/ding.h"
#include "util/array.h"
#include "util/ting.h"
#include "util/io.h"
#include "util/err.h"
#include "util/equate.h"
#include "util/rand.h"
#include "util/opt.h"
#include "util/alloc.h"

#include "mcl/proc.h"
#include "mcl/procinit.h"
#include "mcl/alg.h"

#include "clew/clm.h"
#include "gryphon/path.h"


const char* me = "mcxrand";

void usage
(  const char**
)  ;


enum
{  MY_OPT_APROPOS
,  MY_OPT_HELP
,  MY_OPT_VERSION
,  MY_OPT_OUT
,  MY_OPT_WB
,  MY_OPT_PLUS
,  MY_OPT_IMX
,  MY_OPT_ICL
,  MY_OPT_GEN
,  MY_OPT_ADD
,  MY_OPT_N_SDEV
,  MY_OPT_SHUFFLE
,  MY_OPT_PA
,  MY_OPT_NW
,  MY_OPT_REMOVE
,  MY_OPT_N_RADIUS
,  MY_OPT_N_RANGE
,  MY_OPT_E_MIN
,  MY_OPT_E_MAX
,  MY_OPT_G_MEAN
,  MY_OPT_G_SDEV
,  MY_OPT_G_RADIUS
,  MY_OPT_G_MIN
,  MY_OPT_G_MAX
,  MY_OPT_SKEW
}  ;

const char* syntax = "Usage: mcxrand [options] -imx <mx-file>";

mcxOptAnchor options[] =
{
   {  "--help"
   ,  MCX_OPT_DEFAULT | MCX_OPT_INFO
   ,  MY_OPT_APROPOS
   ,  NULL
   ,  "print this help"
   }
,  {  "-h"
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
,  {  "-gen"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_GEN
   ,  "<num>"
   ,  "node count"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "input matrix"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "input clustering (shuffled version will be output)"
   }
#if 0
,  {  "-noise-radius"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_N_RADIUS
   ,  "<num>"
   ,  "edge perturbation maximum absolute size"
   }
,  {  "-noise-sdev"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_N_SDEV
   ,  "<num>"
   ,  "edge perturbation standard deviation"
   }
,  {  "-noise-range"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_N_RANGE
   ,  "<num>"
   ,  "number of standard deviations allowed"
   }
,  {  "-new-g-radius"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_G_RADIUS
   ,  "<num>"
   ,  "maximum spread to generate new edges with"
   }
,  {  "-new-g-mean"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_G_MEAN
   ,  "<num>"
   ,  "mean to generate new edges with"
   }
,  {  "-new-g-min"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_G_MIN
   ,  "<num>"
   ,  "absolute lower bound for generated weights"
   }
,  {  "-new-g-max"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_G_MAX
   ,  "<num>"
   ,  "absolute upper bound for generated weights"
   }
,  {  "-new-g-sdev"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_G_SDEV
   ,  "<num>"
   ,  "standard deviation to generate new edges with"
   }
,  {  "-edge-min"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_E_MIN
   ,  "<num>"
   ,  "edge addition weight minimum (use -edge-max as well)"
   }
,  {  "-edge-max"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_E_MAX
   ,  "<num>"
   ,  "edge addition weight maximum (use -edge-min as well)"
   }
,  {  "-skew"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_SKEW
   ,  "<num>"
   ,  "skew towards min (<num> > 1) or max (<num> < 1)"
   }
#endif
,  {  "-remove"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_REMOVE
   ,  "<num>"
   ,  "remove <num> edges"
   }
,  {  "--write-binary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WB
   ,  NULL
   ,  "write binary format"
   }
,  {  "+"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_PLUS
   ,  NULL
   ,  "write binary format"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "output matrix to <fname>"
   }
,  {  "-add"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ADD
   ,  "<num>"
   ,  "add <num> edges not yet present"
   }
,  {  "-shuffle"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SHUFFLE
   ,  "<num>"
   ,  "shuffle edge, repeat <num> times"
   }
,  {  "--no-write"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_NW
   ,  NULL
   ,  "exit after computation (some modes only)"
   }
,  {  "-pa"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PA
   ,  "<V>/<m>"
   ,  "create graph with V nodes using preferential attachment, m edges per step"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


int dimCmp
(  const void*   xp
,  const void*   yp
)
   {  return *((dim*) xp) < *((dim*) yp) ? -1 : *((dim*) xp) > *((dim*) yp) ? 1 : 0
;  }


static void mx_readd_diagonal
(  mclx*  mx
,  mclv*  diag
)
   {  dim i
   ;  for (i=0;i<diag->n_ivps;i++)
      {  mclp* ivp = diag->ivps+i
      ;  if (ivp->val)
         mclvInsertIdx(mx->cols+i, ivp->idx, ivp->val)
   ;  }
   }


   /* NOTE currently callers needs to ensure edge sorting by src and dst.
    * -> to be upgraded.
    * NOTE within a chunk of sorted src, dst may be repeated.
    * NOTE currently callers need to ensure there is a sentinel edge
    * (with src member larger than largest columnd domain index)
   */
mclx* mcleAddtoCanonical
(  mcle* E
,  dim   n_e
,  mclx* dst
)
   {  if (!mclxColCanonical(dst))
      {  mcxErr("mcleAddtoCanonical", "require canonical column domain")
      ;  return dst
   ;  }

      {  const mcle* ez = E+n_e, *ea = E, *eb = NULL
      ;  for (eb=ea; eb < ez; eb++)
         {  if (eb->src != ea->src)          /* works for last vector courtesy of sentinel */
            {  if (ea->src < N_COLS(dst))
               {  mclv* v = dst->cols+ea->src
               ;  dim i = 0, ii = 0
               ;  mclvResize(v, eb-ea)       /* iterating over this many */
               ;  for (i=0; i< eb-ea; i++)
                  {  if (i && v->ivps[ii-1].idx == ea[i].dst)
                     v->ivps[ii-1].val += ea[i].val
                  ;  else
                        v->ivps[ii].idx = ea[i].dst
                     ,  v->ivps[ii].val = ea[i].val
                     ,  ii++
;if(0)fprintf(stderr, "make %d->%d %g at %d, vec %d\n", (int) v->vid, (int) ea[i].dst, v->ivps[ii-1].val, (int) ii, (int) v->vid)
               ;  }
                  v->n_ivps = ii
            ;  }
               ea = eb
         ;  }
         }
      }
      return dst
;  }


static mclx* pref_attach3
(  dim   n_V
,  dim   m
,  double p
)
   {  dim n = 0, e_i = 2
   ;  const mcle e0 = { 0, 1, 1.0 }, e1 = { 1, 0, 1.0 }
   ;  mcle* E = calloc(1 + m * n_V, sizeof E[0])

   ;  mclx* mx
      =  mclxAllocZero
         (  mclvCanonical(NULL, n_V, 1.0)
         ,  mclvCanonical(NULL, n_V, 1.0)
         )
   ;  if (n_V <= 1)
      return mx

   ;  E[0] = e0
   ;  E[1] = e1

   ;  for (n=2; n < n_V; n++)
      {  int i_m = 0
      ;  dim e_ii = e_i
      ;  do
         {  unsigned long rx = random()
         ;  unsigned long range = 2 * e_ii
         ;  unsigned long ry = rx % range
         ;  unsigned long rlimit = range * (RAND_MAX / range)
         ;  unsigned lucky = 0
;if(0)fprintf(stderr, "lucky %d\n", (int) lucky)

         ;  if (rx >= rlimit)    /* fixme slight risk of infinite loop */
            {  fputc('.', stderr)
            ;  continue
         ;  }

            lucky = ry & 1 ? E[ry/2].dst : E[ry/2].src

         ;  E[e_i].src = n
         ;  E[e_i].dst = lucky
         ;  E[e_i++].val = 1.0
         ;  i_m++
      ;  }
         while (i_m < m)
      ;  qsort(E+e_ii, e_i - e_ii, sizeof E[0], mcleDstCmp)
   ;  }

      fputc('\n', stderr)
   ;  E[e_i].src = n_V
   ;  E[e_i].dst = n_V
   ;  E[e_i++].val = 0.0

   ;  mcleAddtoCanonical(E, e_i, mx)
   ;  mclxAddTranspose(mx, 0.0)
   ;  free(E)
   ;  return mx
;  }


#if 0

static mclx* pref_attach2
(  dim   n_V
,  dim   m
,  double p
)
   {  dim n = 0, e_i = 2
   ;  const mcle e0 = { 0, 1, 1.0 }, e1 = { -1, 0, 1.0 }
   ;  mcle* E     =  calloc(1 + 2 * m * n_V, sizeof E[0])

   ;  mclx* mx =
         mclxAllocZero
         (  mclvCanonical(NULL, n_V, 1.0)
         ,  mclvCanonical(NULL, n_V, 1.0)
         )
   ;  if (n_V <= 1)
      return mx

   ;  E[0] = e0
   ;  E[1] = e1
   ;  E[0].val = 1.0
   ;  E[1].val = 1.0

   ;  for (n=2; n < n_V; n++)
      {  int i_m = 0
      ;  dim e_ii = e_i
      ;  do
         {  unsigned long rx = random()
         ;  unsigned long ry = rx % e_ii
         ;  unsigned long rlimit = e_ii * (RAND_MAX / e_ii)
         ;  unsigned lucky = E[ry].dst

;if(0)fprintf(stderr, "lucky %d\n", (int) lucky)
         ;  if (rx >= rlimit)    /* fixme slight risk of the infinite */
            {  fputc('.', stderr)
            ;  continue
         ;  }

            E[e_i].src = n       /* this is the one to keep */
         ;  E[e_i].dst = lucky
         ;  E[e_i++].val = 1.0

         ;  E[e_i].src = -1      /* this is just to aid preferential selection (on dst) */
         ;  E[e_i].dst = n       /* NOTE e_i not attainable by ry in this do() */
         ;  E[e_i++].val = 1.0

         ;  i_m++
      ;  }
         while (i_m < m)
      ;  qsort(E+e_ii, e_i - e_ii, sizeof E[0], mcleDstCmp)
   ;  }

                        /* only keep a single arc per edge; add sentinel edge */
      {  dim e_ii, e_a = 1
      ;  for (e_ii=1; e_ii<e_i; e_ii++)
         if (E[e_ii].src >= 0)
         E[e_a++] = E[e_ii]
      ;  e_i = e_a
      ;  E[e_i].src = n_V
      ;  E[e_i].dst = n_V
      ;  E[e_i++].val = 0.0
   ;  }

      mcleAddtoCanonical(E, e_i, mx)
   ;  mclxAddTranspose(mx, 0.0)
   ;  free(E)
   ;  return mx
;  }

#endif
#if 0

static mclx* pref_attach
(  dim   n_V
,  dim   m
,  double p
)
   {  dim n = 0, sumdegree = 0
   ;  dim* degree = calloc(n_V, sizeof degree[0])

   ;  mclx* mx =
         mclxAllocZero
         (  mclvCanonical(NULL, n_V, 1.0)
         ,  mclvCanonical(NULL, n_V, 1.0)
         )
   ;  if (n_V <= 1)
      return mx

   ;  mclgEdgeAdd(mx, 0, 1, 1.0 * m)
   ;  degree[0] = m
   ;  degree[1] = m
   ;  sumdegree = 2 * m

   ;  for (n=2; n < n_V; n++)
      {  int i_m = 0
      ;  do
         {  unsigned long rx = random()
         ;  unsigned long ry = 1 + (rx % sumdegree), rtest = 0, j = 0
;if(0 && ry >= sumdegree)fprintf(stderr, "ry sumdegree %d %d at %d\n", (int) ry, (int) sumdegree, (int) n)
         ;  if (rx >= sumdegree * (RAND_MAX / sumdegree))         /* fixme slight risk of the infinite */
            continue
         ;  for (j=0;j<n;j++)
            {  rtest += degree[j]
            ;  if (rtest >= ry)
               break
         ;  }
            if (j == n)
            mcxErr(me, "test %d / %d failed at %d", (int) rtest, (int) sumdegree, (int) n)
         ;  mclgEdgeAddto(mx, j, n, 1.0)
         ;  degree[j]++
         ;  degree[n]++
         ;  sumdegree++    /* only accounts for j node, *not* for n node; that's done below */
         ;  i_m++
      ;  }
         while (i_m < m)
      ;  if (degree[n] != m)
         mcxErr(me, "low %d %d at %d", (int) degree[n], (int) m, (int) n)
      ;  sumdegree += degree[n]
   ;  }
      return mx
;  }

#endif


void static do_the_shuffle
(  mclx* mx
,  dim N_shuffle
,  dim* offsets   /* size N_COLS(mx) */
,  dim N_edge
,  dim random_ignore
)
   {  dim n_shuffle = 0
   ;  while (n_shuffle < N_shuffle)
      {  unsigned long rx = (unsigned long) random()
      ;  unsigned long ry = (unsigned long) random()
      ;  mclp* ivpll, *ivplr, *ivprl, *ivprr
      ;  dim edge_x, edge_y, *edge_px, *edge_py
      ;  ofs xro, yro, xlo, ylo = -1, vxo, vyo
      ;  long xl, xr, yl, yr
      ;  mclv* vecxl, *vecxr, *vecyl, *vecyr
      ;  double xlval, xrval, ylval, yrval

      ;  if (rx >= random_ignore || ry >= random_ignore)
         continue

      ;  edge_x = rx % N_edge    /* fixme probably not optimal */
      ;  edge_y = ry % N_edge    /* fixme probably not optimal */

      ;  if (!(edge_px = mcxBsearchFloor(&edge_x, offsets, N_COLS(mx), sizeof edge_x, dimCmp)))
         mcxDie(1, me, "edge %ld not found (max %ld)", (long) edge_x, (long) N_edge)

      ;  if (!(edge_py = mcxBsearchFloor(&edge_y, offsets, N_COLS(mx), sizeof edge_y, dimCmp)))
         mcxDie(1, me, "edge %ld not found (max %ld)", (long) edge_y, (long) N_edge)

      ;  vxo   =  edge_px - offsets
      ;  xl    =  mx->dom_cols->ivps[vxo].idx
      ;  vecxl =  mx->cols+vxo
      ;  xro   =  edge_x - offsets[vxo]

      ;  vyo   =  edge_py - offsets
      ;  yl    =  mx->dom_cols->ivps[vyo].idx
      ;  vecyl =  mx->cols+vyo
      ;  yro   =  edge_y - offsets[vyo]

                        /* Offset computation gone haywire */
      ;  if (xro >= vecxl->n_ivps || yro >= vecyl->n_ivps)     /* note: mixed sign comparison */
         mcxDie(1, me, "paradox 1 in %ld or %ld", xl, yl)

      ;  xr = vecxl->ivps[xro].idx
      ;  yr = vecyl->ivps[yro].idx
      ;  xrval = vecxl->ivps[xro].val
      ;  yrval = vecyl->ivps[yro].val

                        /* Impossible, should have graph */
      ;  vecxr = mclxGetVector(mx, xr, EXIT_ON_FAIL, NULL)
      ;  vecyr = mclxGetVector(mx, yr, EXIT_ON_FAIL, NULL)

                        /* check that we have four different nodes
                         * loops are not present so no need to check those
                        */
      ;  if (xl == yl || xl == yr || xr == yl || xr == yr)
         continue

      ;  if
         (  (0 > (xlo = mclvGetIvpOffset(vecxr, xl, -1)))
         || (0 > (ylo = mclvGetIvpOffset(vecyr, yl, -1)))
         )
         mcxDie
         (  1
         ,  me
         ,  "symmetry violation 1"
            " %ld not found in %ld/%ld OR %ld not found in %ld/%ld"
         ,  (long) xl, (long) vecxr->vid, (long) vecxr->n_ivps
         ,  (long) yl, (long) vecyr->vid, (long) vecyr->n_ivps
         )

                        /* Now:  xl yl :  ivpll
                         *       xl yr :  ivplr
                         *       xr yl :  ivprl
                         *       xr yr :  ivprr
                        */
      ;  xlval = vecxr->ivps[xlo].val
      ;  ylval = vecyr->ivps[ylo].val

      ;  ivpll = mclvGetIvp(vecxl, yl, NULL)
      ;  ivplr = mclvGetIvp(vecxl, yr, NULL)
      ;  ivprl = mclvGetIvp(vecxr, yl, NULL)
      ;  ivprr = mclvGetIvp(vecxr, yr, NULL)

      ;  if
         (  (ivpll && !mclvGetIvp(vecyl, xl, NULL))
         || (ivplr && !mclvGetIvp(vecyr, xl, NULL))
         || (ivprl && !mclvGetIvp(vecyl, xr, NULL))
         || (ivprr && !mclvGetIvp(vecyr, xr, NULL))
         )
         mcxDie(1, me, "symmetry violation 2")

      ;  if ((ivpll && ivplr) || (ivprl && ivprr))
         continue

      ;  {  if (!ivpll && !ivprr) 
            {        /* vecxl <-> xr   becomes vecxl <-> yl
                      * vecxr <-> xl   becomes vecxr <-> yr
                      * vecyl <-> yr   becomes vecyl <-> xl 
                      * vecyr <-> yl   becomes vecyr <-> xr
                     */
            ;  if
               (  mclvReplaceIdx(vecxl, xro, yl, xrval)
               || mclvReplaceIdx(vecyl, yro, xl, xrval)
               || mclvReplaceIdx(vecxr, xlo, yr, ylval)
               || mclvReplaceIdx(vecyr, ylo, xr, ylval)
               )
               mcxDie(1, me, "parallel replacement failure\n")
#if DEBUG
;fprintf(stderr, "parallel edge change remove  %d-%d  %d-%d add  %d-%d  %d-%d\n",
   vecxl->vid, xr, vecyr->vid, yl,  vecxl->vid, yl, vecyr->vid, xr)
#endif
         ;  }
            else if (!ivplr && !ivprl)
            {        /* vecxl -> xr   becomes vecxl <-> yr
                      * vecxr -> xl   becomes vecxr <-> yl
                      * vecyl -> yr   becomes vecyl <-> xr 
                      * vecyr -> yl   becomes vecyr <-> xl
                     */
               if
               (  mclvReplaceIdx(vecxl, xro, yr, xrval)
               || mclvReplaceIdx(vecyr, ylo, xl, xlval)
               || mclvReplaceIdx(vecxr, xlo, yl, yrval)
               || mclvReplaceIdx(vecyl, yro, xr, yrval)
               )
               mcxDie(1, me, "cross replacement failure\n")
#if DEBUG
;fprintf(stderr, "cross edge change remove  %d-%d  %d-%d add  %d-%d  %d-%d\n",
   vecxl->vid, xr, vecyl->vid, yr,  vecxl->vid, yr, vecyl->vid, xr)
#endif
         ;  }
         }
         n_shuffle++
   ;  }
   }


static dim do_remove
(  mclx* mx
,  dim N_remove
,  dim* offsets      /* size N_COLS(mx) */
,  dim N_edge
,  dim random_ignore
)
   {  dim n_remove = 0
   ;  while (n_remove < N_remove)
      {  unsigned long r = (unsigned long) random()
      ;  dim e, *ep
      ;  ofs xo, yo
      ;  long x
      ;  mclv* vecx

      ;  if (r >= random_ignore)
         continue

      ;  e = r % N_edge

      ;  if (!(ep = mcxBsearchFloor(&e, offsets, N_COLS(mx), sizeof e, dimCmp)))
         mcxDie(1, me, "edge %ld not found (max %ld)", (long) e, (long) N_edge)

      ;  xo = ep - offsets
;if(DEBUG)fprintf(stderr, "have e=%d o=%d\n", (int) e, (int) xo)
      ;  x = mx->dom_cols->ivps[xo].idx
      ;  vecx = mx->cols+xo;
      ;  yo = e - offsets[xo]

      ;  if (yo >= vecx->n_ivps)    /*  note: mixed sign comparison */
         mcxDie
         (  1
         ,  me
         ,  "vector %ld has not enough entries: %ld < %ld"
         ,  (long) vecx->vid
         ,  (long) vecx->n_ivps
         ,  (long) yo
         )

      ;  if (!vecx->ivps[yo].val)
         continue

      ;  vecx->ivps[yo].val = 0.0

;if(DEBUG)fprintf(stderr, "remove [%d] %ld %ld\n", (int) n_remove, (long) x, (long) vecx->ivps[yo].idx)
      ;  n_remove++
   ;  }
      return n_remove
;  }


static dim do_add2
(  mclx* mx
,  dim N_add
,  dim N_edge
)
   {  dim n_add = 0
   ;  while (n_add < N_add)
      {  unsigned long r = (unsigned long) random()
      ;  unsigned long s = (unsigned long) random()
      ;  long x, y
      ;  double val = 1.0
      ;  mclp* ivp

      ;  dim xo = r % N_COLS(mx)       /* fixme, modulo is commonly recommended against */
      ;  dim yo = s % N_COLS(mx)

      ;  if (xo > yo)
         {  long zo = xo
         ;  xo = yo
         ;  yo = zo
      ;  }
         else if (xo == yo)            /* never add loops */
         continue

      ;  x = mx->dom_cols->ivps[xo].idx
      ;  y = mx->dom_cols->ivps[yo].idx

      ;  if (N_edge >= N_COLS(mx) * (N_COLS(mx)-1) / 2)
         break

      ;  ivp = mclvGetIvp(mx->cols+xo, y, NULL)

      ;  if (ivp && ivp->val)
         continue

;if(DEBUG)fprintf(stderr, "add [%d] %ld %ld value %f\n", (int) n_add, (long) x, (long) y, val)
      ;  mclvInsertIdx(mx->cols+xo, y, val)
      ;  N_edge++
      ;  n_add++
   ;  }
      return n_add
;  }


static dim do_add
(  mclx* mx
,  dim N_add
,  dim N_edge
,  double *l_mean
,  double l_radius
,  double l_sdev
,  double l_min
,  double l_max
,  double skew
,  double e_min
,  double e_max
)
   {  dim n_add = 0
   ;  while (n_add < N_add)
      {  unsigned long r = (unsigned long) random()
      ;  unsigned long s = (unsigned long) random()
      ;  long x, y
      ;  double val
      ;  mclp* ivp

      ;  dim xo = r % N_COLS(mx)       /* fixme, modulo is commonly recommended against */
      ;  dim yo = s % N_COLS(mx)

      ;  if (xo > yo)
         {  long zo = xo
         ;  xo = yo
         ;  yo = zo
      ;  }
         else if (xo == yo)            /* never add loops */
         continue

      ;  x = mx->dom_cols->ivps[xo].idx
      ;  y = mx->dom_cols->ivps[yo].idx

      ;  if (N_edge >= N_COLS(mx) * (N_COLS(mx)-1) / 2)
         break

      ;  ivp = mclvGetIvp(mx->cols+xo, y, NULL)

      ;  if (ivp && ivp->val)
         continue

      ;  if (l_mean)
         {  do
            {  val = mcxNormalCut(l_radius, l_sdev)
            ;  if (skew)
               {  val = (l_radius + val) / (2 * l_radius)
                                 /* ^ map (l_radius + val) to lie within [0,1] */
               ;  val = pow(val, skew)
                                 /* skew it */
               ;  val = (val * 2 * l_radius) - l_radius
                                 /* map it back */
            ;  }
               val += l_mean[0]
         ;  }
            while (l_min < l_max && (val < l_min || val > l_max))
      ;  }
               /* docme: uniform */
         else
         {  val = (((unsigned long) random()) * 1.0) / RAND_MAX
         ;  if (skew)
            val = pow(val, skew)
         ;  val = e_min + val * (e_max - e_min)
      ;  }

         if (!val)
         continue

;if(DEBUG)fprintf(stderr, "add [%d] %ld %ld value %f\n", (int) n_add, (long) x, (long) y, val)
      ;  mclvInsertIdx(mx->cols+xo, y, val)
      ;  N_edge++
      ;  n_add++
   ;  }
      return n_add
;  }


int main
(  int                  argc
,  const char*          argv[]
)  
   {  mcxIO* xfmx          =  mcxIOnew("-", "r"), *xfout = mcxIOnew("-", "w"), *xfcl = NULL
   ;  mclx* mx             =  NULL,  *cl = NULL
   ;  mclv* mx_diag        =  NULL

   ;  mcxstatus parseStatus = STATUS_OK
   ;  mcxOption* opts, *opt
   ;  dim N_edge = 0
   ;  dim*  offsets

   ;  dim template_n_nodes = 0
   ;  mcxbool plus = FALSE

#if 0
   ;  double e_min  = 1.0
   ;  double e_max  = 0.0
   ;  double skew = 0.0

   ;  double radius = 0.0
   ;  double n_sdev  = 0.5
   ;  double n_range = 2.0

   ;  double g_radius  = 0.0
   ;  double g_mean   = 0.0
   ;  double g_sdev = 0.0
   ;  double g_min  = 1.0
   ;  double g_max  = 0.0
   ;  mcxbool do_gaussian = FALSE
#endif

   ;  mcxbool no_write = FALSE

   ;  dim i = 0

   ;  dim N_remove  = 0
   ;  dim N_add     = 0
   ;  dim N_shuffle = 0
   ;  unsigned N_pa = 0
   ;  unsigned m_pa = 0

   ;  unsigned long random_ignore = 0

   ;  srandom(mcxSeed(2308947))
   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)

   ;  if
      (!(opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)))
      exit(0)

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
   ;  mclx_app_init(stderr)

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

            case MY_OPT_GEN
         :  template_n_nodes = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_ICL
         :  xfcl = mcxIOnew(opt->val, "r")
         ;  cl = mclxReadx(xfcl, EXIT_ON_FAIL, MCLX_REQUIRE_PARTITION)
         ;  break
         ;

            case MY_OPT_IMX
         :  mcxIOrenew(xfmx, opt->val, NULL)
         ;  break
         ;

            case MY_OPT_PLUS
         :  case MY_OPT_WB
         :  plus = TRUE
         ;  break
         ;

            case MY_OPT_OUT
         :  mcxIOrenew(xfout, opt->val, NULL)
         ;  break
         ;

#if 0
            case MY_OPT_E_MAX
         :  if (!strcmp(opt->val, "copy"))
            e_max = -DBL_MAX
         ;  else
            e_max = atof(opt->val)
         ;  break
         ;

            case MY_OPT_SKEW
         :  skew = atof(opt->val)
         ;  break
         ;

            case MY_OPT_E_MIN
         :  e_min = atof(opt->val)
         ;  break
         ;

            case MY_OPT_G_MIN
         :  g_min = atof(opt->val)
         ;  break
         ;

            case MY_OPT_G_MAX
         :  g_max = atof(opt->val)
         ;  break
         ;

            case MY_OPT_G_SDEV
         :  g_sdev = atof(opt->val)
         ;  break
         ;

            case MY_OPT_G_MEAN
         :  g_mean = atof(opt->val)
         ;  do_gaussian = TRUE
         ;  break
         ;

            case MY_OPT_G_RADIUS
         :  g_radius = atof(opt->val)
         ;  break
         ;

            case MY_OPT_N_RANGE
         :  n_range = atof(opt->val)
         ;  break
         ;

            case MY_OPT_N_SDEV
         :  n_sdev = atof(opt->val)
         ;  break
         ;

            case MY_OPT_N_RADIUS
         :  radius = atof(opt->val)
         ;  break
         ;
#endif

            case MY_OPT_NW
         :  no_write = TRUE
         ;  break
         ;

            case MY_OPT_PA
         :  if (2 != sscanf(opt->val, "%u/%u", &N_pa, &m_pa))
            mcxDie(1, me, "-pa argument takes V/m form")
         ;  break
         ;

            case MY_OPT_SHUFFLE
         :  N_shuffle = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_ADD
         :  N_add = atoi(opt->val)
         ;  break
         ;
            
            case MY_OPT_REMOVE
         :  N_remove  = atoi(opt->val)
         ;  break
      ;  }
      }

      if (cl)
      {  mclv* dom2 = mclvCopy(NULL, cl->dom_rows)
      ;  mclp theivp = { 0 }
      ;  long i, n_copied = 0
      ;  mcxShuffle(dom2->ivps, dom2->n_ivps, sizeof theivp, (char*) (void*) &theivp)
      ;  for (i=0;i<N_COLS(cl);i++)
         {  if (n_copied + cl->cols[i].n_ivps > N_ROWS(cl))
            break
         ;  memcpy(cl->cols[i].ivps, dom2->ivps+n_copied, cl->cols[i].n_ivps * sizeof theivp) 
         ;  n_copied += cl->cols[i].n_ivps
      ;  }
         mclxWrite(cl, xfout, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
      ;  exit(0)
   ;  }
                                 /* hitting y% in vi tells me the size of this block */
      {  if (template_n_nodes)
         mx =  mclxAllocZero
               (  mclvCanonical(NULL, template_n_nodes, 1.0)
               ,  mclvCanonical(NULL, template_n_nodes, 1.0)
               )
      ;  else if (N_pa)
         {  mx =  pref_attach3(N_pa, m_pa, 1.0)
         ;  if (!no_write)
            mclxWrite(mx, xfout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
         ;  exit(0)
      ;  }

         else
         mx =  mclxReadx
               (  xfmx
               ,  EXIT_ON_FAIL
               ,  MCLX_REQUIRE_GRAPH
               )

      ;  mx_diag = mclxDiagValues(mx, MCL_VECTOR_COMPLETE)

      ;  if (N_shuffle)
         mclxAdjustLoops(mx, mclxLoopCBremove, NULL)
      ;  else
         mclxSelectUpper(mx)
      /* ^ apparently we always work on single arc representation (docme andsoon) */

      ;  offsets = mcxAlloc(sizeof offsets[0] * N_COLS(mx), EXIT_ON_FAIL)

      ;  N_edge = 0
      ;  for (i=0;i<N_COLS(mx);i++)
         {  offsets[i] = N_edge
         ;  N_edge += mx->cols[i].n_ivps
      ;  }

         if (N_edge < N_remove)
         {  mcxErr
            (  me
            ,  "removal count %ld exceeds edge count %ld"
            ,  (long) N_remove
            ,  (long) N_edge
            )
         ;  N_remove = N_edge
      ;  }

         random_ignore = RAND_MAX - (N_edge ? RAND_MAX % N_edge : 0)

      ;  if (RAND_MAX / 2 < N_edge)
         mcxDie(1, me, "graph too large!")

      ;  if (N_shuffle)
         {  do_the_shuffle(mx, N_shuffle, offsets, N_edge, random_ignore)
         ;  mx_readd_diagonal(mx, mx_diag)
         ;  mclxWrite(mx, xfout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
         ;  exit(0)
      ;  }

      ;  if (N_remove)
         {  dim n_remove = do_remove(mx, N_remove, offsets, N_edge, random_ignore)
               /* Need to recompute N_edge and random_ignore.
                * NOTE we work with *upper* matrix; this counts graph edges.
               */
         ;  N_edge = mclxNrofEntries(mx) - n_remove
         ;  random_ignore = RAND_MAX - (RAND_MAX % N_COLS(mx))
      ;  }

         if (N_add)
         N_edge += do_add2(mx, N_add, N_edge)

      ;  mclxUnary(mx, fltxCopy, NULL)    /* remove zeroes */
      ;  mclxAddTranspose(mx, 0.0)
      ;  mx_readd_diagonal(mx, mx_diag)

      ;  if (plus)
         mclxbWrite(mx, xfout, RETURN_ON_FAIL)
      ;  else
         mclxWrite(mx, xfout, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
   ;  }
      return 0
;  }


