/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
 *
 * NOTE
 * this file implements both /diameter/ and /ctty/ (centrality)
 * functionality for mcx.
*/

/* TODO
 * -  Handle components.
 * -  Clean up diameter/ctty organization.
*/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include "mcx.h"

#include "util/types.h"
#include "util/ding.h"
#include "util/ting.h"
#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/compile.h"
#include "util/minmax.h"

#include "impala/matrix.h"
#include "impala/tab.h"
#include "impala/io.h"
#include "impala/stream.h"
#include "impala/app.h"

#include "clew/clm.h"
#include "gryphon/path.h"

static const char* mediam = "mcx diameter";
static const char* mectty = "mcx ctty";


         /* this aids in finding heuristically likely starting points
          * for long shortest paths, by looking at dead ends
          * in the lattice.
          * experimental, oefully underdocumented.
         */
static dim diameter_rough
(  mclv*       vec
,  mclx*       mx
,  u8*         rough_scratch
,  long*       rough_priority
)
   {  mclv* curr  =  mclvInsertIdx(NULL, vec->vid, 1.0) 
   ;  mclpAR* par =  mclpARensure(NULL, 1024)

   ;  dim d = 0, n_dead_ends = 0, n_dead_ends_res = 0

   ;  memset(rough_scratch, 0, N_COLS(mx))

   ;  rough_scratch[vec->vid] = 1                        /* seen */
   ;  rough_priority[vec->vid] = -1              /* remove from priority list */

   ;  while (1)
      {  mclp* currivp = curr->ivps
      ;  dim t
      ;  mclpARreset(par)
      ;  while (currivp < curr->ivps + curr->n_ivps)
         {  mclv* ls = mx->cols+currivp->idx
         ;  mclp* newivp = ls->ivps
         ;  int hit = 0
         ;  while (newivp < ls->ivps + ls->n_ivps)
            {  u8* tst = rough_scratch+newivp->idx
            ;  if (!*tst || *tst & 2)
               {  if (!*tst)
                  mclpARextend(par, newivp->idx, 1.0)
               ;  *tst = 2
               ;  hit = 1
            ;  }
               newivp++
         ;  }
            if (!hit && rough_priority[currivp->idx] >= 0)
               rough_priority[currivp->idx] += d+1
            ,  n_dead_ends_res++
         ;  else if (!hit)
            n_dead_ends++
/* ,fprintf(stderr, "[%ld->%ld]", (long) currivp->idx, (long) rough_priority[currivp->idx])
*/
;
#if 0
if (currivp->idx == 115 || currivp->idx == 128)
fprintf(stdout, "pivot %d node %d d %d dead %d pri %d\n", (int) vec->vid, (int) currivp->idx, d, (int) (1-hit), (int) rough_priority[currivp->idx])
#endif
         ;  currivp++
      ;  }
         if (!par->n_ivps)
         break
      ;  d++
      ;  mclvFromIvps(curr, par->ivps, par->n_ivps)
      ;  for (t=0;t<curr->n_ivps;t++)
         rough_scratch[curr->ivps[t].idx] = 1
   ;  }

      mclvFree(&curr)
   ;  mclpARfree(&par)
;if(0)fprintf(stdout, "deadends %d / %d\n", (int) n_dead_ends, (int) n_dead_ends_res)
   ;  return d
;  }


#define SEENx(mx, idx)  (mx->dom_cols->ivps[idx].val)

dim diamFlood2
(  mclx* mx
,  ofs root
,  dim extent
,  mclv* order
)
   {  dim i, n_done = 0, n_add = 1, i_wave = 0
   ;  order->ivps[0].idx = root

   ;  for (i=0; i<N_COLS(mx); i++)
         mx->cols[i].val = 0.0
      ,  SEENx(mx, i) = 0.0

   ;  mx->cols[root].val = 1.0               /* bootstrap value */
   ;  SEENx(mx, root) = 1.0

   ;  while (n_add && (!extent || i_wave++ <= extent))
      {  dim j, n_add_new = 0

      ;  for (j=n_done; j<n_done+n_add; j++)
         {  mclv* node = mx->cols+order->ivps[j].idx
         ;  dim k
         ;  for (k=0;k<node->n_ivps;k++)
            {  mclp* edge = node->ivps+k
            ;  mclv* node2 = mx->cols + edge->idx
            ;  if (!SEENx(mx, node2->vid))
               {  if (!node2->val)
                  order->ivps[n_done + n_add + n_add_new++].idx = node2->vid
               ;  node2->val += node->val
;if(0)fprintf(stdout, "node %d value %g SEENx %d\n", (int) node2->vid, node2->val, (int) SEENx(mx, node2->vid))
            ;  }
               else
               edge->val = 0.0
         ;  }
;if(0)fputs("--\n", stdout);
                                       if(0)mclvUnary(node, fltxCopy, NULL)
      ;  }
;if(0)fprintf(stdout, "(added %d)\n", (int) n_add);
      ;  n_done += n_add
      ;  n_add = n_add_new
      ;  for (j=n_done;j<n_done+n_add;j++)
         SEENx(mx, order->ivps[j].idx) = 1.0
   ;  }
      return n_done
;  }


typedef struct
{  ofs      node_idx
;  double   n_paths_lft
;  double   acc_wtd_global
;  double   acc_wtd_global2
;  double   acc_wtd_local  /* accumulator, weighted */
;  dim      acc_chr        /* accumulator, characteristic */
;  mcxLink* nb_lft         /* list of neighbours, NULL terminated */
;  mcxLink* nb_rgt         /* list of neighbours, NULL terminated */
;
}  SSPnode  ;


enum
{  MY_OPT_ABC    =   MCX_DISP_UNUSED
,  MY_OPT_IMX
,  MY_OPT_TAB
,  MY_OPT_OUT
,  MY_OPT_LIST_MAX
,  MY_OPT_LIST_NODES
,  MY_OPT_T
,  MY_OPT_G
,  MY_OPT_littleG
,  MY_OPT_EXTENT
,  MY_OPT_EDGE
,  MY_OPT_WEIGHTED
,  MY_OPT_INCLUDE_ENDS
,  MY_OPT_MOD
,  MY_OPT_ROUGH
}  ;


mcxOptAnchor diameterOptions[] =
{  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "specify input matrix/graph"
   }
,  {  "-abc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ABC
   ,  "<fname>"
   ,  "specify input using label pairs"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "specify tab file to be used with matrix input"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "write to file fname"
   }
,  {  "-t"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_T
   ,  "<int>"
   ,  "number of threads to use"
   }
,  {  "-J"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_G
   ,  "<int>"
   ,  "number of compute jobs overall"
   }
,  {  "-j"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_littleG
   ,  "<int>"
   ,  "index of this compute job"
   }
,  {  "--rough"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_ROUGH
   ,  NULL
   ,  "use direct computation (testing only)"
   }
,  {  "--summary"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LIST_MAX
   ,  NULL
   ,  "return length of longest shortest path"
   }
,  {  "--list"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LIST_NODES
   ,  NULL
   ,  "list eccentricity for all nodes (default)"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


mcxOptAnchor cttyOptions[] =
{  {  "-imx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "specify input matrix"
   }
,  {  "-abc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ABC
   ,  "<fname>"
   ,  "specify input using label pairs"
   }
,  {  "-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_TAB
   ,  "<fname>"
   ,  "specify tab file to be used with matrix input"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "write to file fname"
   }
,  {  "-t"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_T
   ,  "<int>"
   ,  "number of threads to use"
   }
,  {  "-J"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_G
   ,  "<int>"
   ,  "number of compute jobs overall"
   }
,  {  "-j"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_littleG
   ,  "<int>"
   ,  "index of this compute job"
   }
,  {  "--edge"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_EDGE
   ,  NULL
   ,  "compute edge betweeness centrality"
   }
,  {  "-extent"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_EXTENT
   ,  "<int>"
   ,  "limit centrality computation to nodes with step-distance <int>"
   }
,  {  "--rough"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_ROUGH
   ,  NULL
   ,  "use direct computation (testing only)"
   }
,  {  "--with-ends"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_INCLUDE_ENDS
   ,  NULL
   ,  "include scores for lattice sources and sinks"
   }
#if 0             /* nope, not yet implemented. Is there a point even? */
,  {  "--weighted"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WEIGHTED
   ,  NULL
   ,  "compute weighted centrality"
   }
#endif
,  {  "--list"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_LIST_NODES
   ,  NULL
   ,  "(default) list mode"
   }
,  {  "--mod"
   ,  MCX_OPT_HIDDEN
   ,  MY_OPT_MOD
   ,  NULL
   ,  "add 1 to each sub-score"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;

static  unsigned debug_g   =   -1;
static  mcxbool do_edge_g  = FALSE;

static  mcxIO* xfmx_g      =   (void*) -1;
static  mcxIO* xfabc_g     =   (void*) -1;
static  mcxIO* xftab_g     =   (void*) -1;
static  mclTab* tab_g      =   (void*) -1;
static  dim progress_g     =   -1;
static  mcxbool rough      =   -1;
static  mcxbool list_nodes =   -1;
static  mcxbool mod_up     =   -1;
static  mcxbool weighted_g =   -1;

static const char* out_g   =   (void*) -1;

static mcxbool include_ends_g  =  -1;
static dim n_group_G       =  -1;
static dim i_group         =  -1;
static dim n_thread_l      =  -1;
static dim extent_g        =  -1;         /* ctty depth */

static mcxstatus allInit
(  void
)
   {  xfmx_g         =  mcxIOnew("-", "r")
   ;  xfabc_g        =  NULL
   ;  xftab_g        =  NULL
   ;  tab_g          =  NULL
   ;  progress_g     =  0
   ;  rough          =  FALSE
   ;  list_nodes     =  TRUE
   ;  mod_up         =  FALSE
   ;  n_group_G      =  1
   ;  i_group        =  0
   ;  n_thread_l     =  0
   ;  include_ends_g =  FALSE
   ;  out_g          =  "-"
   ;  debug_g        =  0
   ;  weighted_g     =  FALSE
   ;  extent_g       =  0
   ;  return STATUS_OK
;  }


static mcxstatus cttyArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_IMX
      :  mcxIOnewName(xfmx_g, val)
      ;  break
      ;

         case MY_OPT_ABC
      :  xfabc_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_TAB
      :  xftab_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_OUT
      :  out_g = val
      ;  break
      ;

         case MY_OPT_INCLUDE_ENDS
      :  include_ends_g = TRUE
      ;  break
      ;

         case MY_OPT_LIST_NODES
      :  break
      ;

         case MY_OPT_littleG
      :  i_group =  atoi(val)
      ;  break
      ;

         case MY_OPT_G
      :  n_group_G =  atoi(val)
      ;  break
      ;

         case MY_OPT_EDGE
      :  do_edge_g = TRUE
      ;  break
      ;

         case MY_OPT_EXTENT
      :  extent_g = (unsigned) atoi(val)
      ;  break
      ;

         case MY_OPT_T
      :  n_thread_l = (unsigned) atoi(val)
      ;  break
      ;

         case MY_OPT_WEIGHTED
      :  weighted_g = TRUE
      ;  break
      ;

         case MY_OPT_MOD
      :  mod_up = TRUE
      ;  break
      ;

         default
      :  mcxExit(1)
      ;
   ;  }
      return STATUS_OK
;  }


static mcxstatus diameterArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_IMX
      :  mcxIOnewName(xfmx_g, val)
      ;  break
      ;

         case MY_OPT_LIST_NODES
      :  list_nodes = TRUE
      ;  break
      ;

         case MY_OPT_T
      :  n_thread_l = (unsigned) atoi(val)
      ;  break
      ;

         case MY_OPT_LIST_MAX
      :  list_nodes = FALSE
      ;  break
      ;

         case MY_OPT_ROUGH
      :  rough = TRUE
      ;  break
      ;

         case MY_OPT_TAB
      :  xftab_g = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_OUT
      :  out_g = val
      ;  break
      ;

         case MY_OPT_littleG
      :  i_group =  atoi(val)
      ;  break
      ;

         case MY_OPT_G
      :  n_group_G =  atoi(val)
      ;  break
      ;

         case MY_OPT_ABC
      :  xfabc_g = mcxIOnew(val, "r")
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
   ;  }
      return STATUS_OK
;  }


static void rough_it
(  mclx* mx
,  dim* tabulator
,  dim i
,  u8* rough_scratch
,  long* rough_priority
,  dim* pri
)
   {  dim dd = 0, p = i, p1 = 0, p2 = 0, priority = 0, p1p2 = 0, j = 0
      
   ;  for (j=0;j<N_COLS(mx);j++)
      {  if (1)
         {  if (rough_priority[j] >= rough_priority[p1])
               p2 = p1
            ,  p1 = j
         ;  else if (rough_priority[j] >= rough_priority[p2])
            p2 = j
      ;  }
         else
         {  if (!p1 && rough_priority[j] >= 1)
            p1 = j
      ;  }
      }
      p = p1
   ;  priority = rough_priority[p]
   ;  p1p2 = rough_priority[p1] + rough_priority[p2]
   ;  dd = diameter_rough(mx->cols+p, mx, rough_scratch, rough_priority)
;if (0)
fprintf
(  stdout
,  "guard %6d--%-6d %6d %6d NODE %6d ECC %6d PRI\n"
,  (int) p1p2
,  (int) (i * dd)
,  (int) i
,  (int) p
,  (int) dd
,  (int) priority
)
   ;  *pri = priority
   ;  tabulator[p] = dd
;  }


static void ecc_compute
(  dim* tabulator
,  const mclx* mx
,  dim offset
,  dim inc
,  mclv* scratch
)
   {  dim i
   ;  for (i=offset;i<N_COLS(mx);i+=inc)
      {  dim dd = mclgEcc2(mx->cols+i, mx, scratch)
      ;  tabulator[i] = dd
   ;  }
   }



typedef struct
{  dim*     tabulator
;  mclv*    scratch
;
}  diam_data   ;


static void diam_dispatch
(  mclx* mx
,  dim i
,  void* data
,  dim thread_id 
)
   {  diam_data* d = ((diam_data*) data) + thread_id
   ;  dim dd = mclgEcc2(mx->cols+i, mx, d->scratch)
   ;  d->tabulator[i] = dd
;  }


static mcxstatus diameterMain
(  int          argc_unused      cpl__unused
,  const char*  argv_unused[]    cpl__unused
)
   {  mclx* mx                =  NULL
   ;  mcxbool canonical       =  FALSE

   ;  dim* tabulator          =  NULL

   ;  mclv* ecc_scratch       =  NULL
   ;  mcxIO* xfout            =  mcxIOnew(out_g, "w")

   ;  double sum = 0.0

   ;  dim thediameter = 0
   ;  dim i

   ;  progress_g  =  mcx_progress_g
   ;  debug_g     =  mcx_debug_g

   ;  n_thread_l = mclx_set_threads_or_die(mediam, n_thread_l, i_group, n_group_G)

;if(0)fprintf(stderr, "%d %d %d\n", (int) n_thread_l, (int) n_thread_l, (int) n_group_G)
   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  mx = mcx_get_graph("mcx diameter", xfmx_g, xfabc_g, xftab_g, &tab_g, NULL, MCLX_REQUIRE_GRAPH | MCLX_REQUIRE_CANONICAL | MCL_READX_REMOVE_LOOPS)
   ;  mcxIOfree(&xfmx_g)

   ;  tabulator      =  calloc(N_COLS(mx), sizeof tabulator[0])
   ;  ecc_scratch    =  mclvCopy(NULL, mx->dom_rows)
                                       /* ^ used as ecc scratch: should have values 1.0 */

   ;  canonical = MCLV_IS_CANONICAL(mx->dom_cols)

   ;  if (rough && !mclxGraphCanonical(mx))
      mcxDie(1, mediam, "rough needs canonical domains")

   ;  if (rough)
      {  u8* rough_scratch    =  calloc(N_COLS(mx), sizeof rough_scratch[0])
      ;  long* rough_priority =  mcxAlloc(N_COLS(mx) * sizeof rough_priority[0], EXIT_ON_FAIL)
      ;  for (i=0;i<N_COLS(mx);i++)
         rough_priority[i] = 0
      ;  for (i=0;i<N_COLS(mx);i++)
         {  dim priority = 0
         ;  rough_it(mx, tabulator, i, rough_scratch, rough_priority, &priority)
      ;  }
         mcxFree(rough_scratch)
      ;  mcxFree(rough_priority)
   ;  }
      else if (n_group_G * n_thread_l <= 1)
      ecc_compute(tabulator, mx, 0, 1, ecc_scratch)
   ;  else
      {  dim t = 0
      ;  mclx* scratchen            /* annoying UNIXen-type plural */
         =  mclxCartesian(mclvCanonical(NULL, n_thread_l, 1.0), mclvClone(mx->dom_rows), 1.0)

      ;  diam_data* data = mcxAlloc(n_thread_l * sizeof data[0], EXIT_ON_FAIL)

      ;  for (t=0;t<n_thread_l;t++)
         {  diam_data* d=  data+t
         ;  d->scratch  =  scratchen->cols+t
         ;  d->tabulator= tabulator
      ;  }

         mclxVectorDispatchGroup(mx, data, n_thread_l, diam_dispatch, n_group_G, i_group, NULL)
      ;  mcxFree(data)
      ;  mclxFree(&scratchen)
   ;  }

      if (list_nodes)
      fprintf(xfout->fp, "node\tdiam\n")

   ;  for (i=0;i<N_COLS(mx);i++)    /* report everything so that results can be collected */
      {  dim dd = tabulator[i]
      ;  sum += dd

      ;  if (list_nodes)
         {  long vid = mx->cols[i].vid
         ;  if (tab_g)
            {  const char* label = mclTabGet(tab_g, vid, NULL)
            ;  if (!label) mcxDie(1, mediam, "panic label %ld not found", vid)
            ;  fprintf(xfout->fp, "%s\t%ld\n", label, (long) dd)
         ;  }
            else
            fprintf(xfout->fp, "%ld\t%ld\n", vid, (long) dd)
      ;  }

         if (dd > thediameter)
         thediameter = dd
   ;  }

      if (!list_nodes && N_COLS(mx))
      {  sum /= N_COLS(mx)
      ;  fprintf
         (  xfout->fp
         ,  "%d\t%.3f\n"
         ,  (unsigned) thediameter
         ,  (double) sum
         )
   ;  }

      mcxIOfree(&xfout)
   ;  mclxFree(&mx)
   ;  mclvFree(&ecc_scratch)
   ;  mcxFree(tabulator)
   ;  return 0
;  }


mcxDispHook* mcxDispHookDiameter
(  void
)
   {  static mcxDispHook diameterEntry
   =  {  "diameter"
      ,  "diameter [options]"
      ,  diameterOptions
      ,  sizeof(diameterOptions)/sizeof(mcxOptAnchor) - 1

      ,  diameterArgHandle
      ,  allInit
      ,  diameterMain

      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &diameterEntry
;  }


void cttyUpdateDS
(  mclx* down
,  mclx* up
,  mclv* domain
,  float value
)
   {  dim i
   ;  mclx* src, *dst
   ;  if (value)              /* set down and up both back to one */
         src = up
      ,  dst = down
   ;  else                    /* set up to zero where down is zero */
         src = down
      ,  dst = up

   ;  for (i=0;i<domain->n_ivps;i++)
      {  dim lft = domain->ivps[i].idx
      ;  mclv* v = src->cols+lft
      ;  dim j
      ;  for (j=0; j<v->n_ivps;j++)
         {  mclp* p = v->ivps+j
         ;  if (!p->val)
            {  mclp* q = mclxInsertIvp(dst, p->idx, lft)
            ;  q->val = value
            ;  v->ivps[j].val = value
         ;  }
         }
      }
   }




#define SEEN(mx, idx)  (mx->dom_cols->ivps[idx].val)

   /* This routine
    *    -  computes a lattice of shortest path counts
    *    -  sets edge weights not in the lattice to zero
    *    -  fills in the order in which the lattice was filled (used for back-traversal)
    *    -  uses mx->cols[i].val for annotation.
   */

dim cttyFlood2
(  mclx* mx
,  ofs root
,  dim extent
,  mclv* order
)
   {  dim i, n_done = 0, n_add = 1, i_wave = 0
   ;  order->ivps[0].idx = root

;if(0)fputc('\n', stdout)
   ;  for (i=0; i<N_COLS(mx); i++)
         mx->cols[i].val = 0.0
      ,  SEEN(mx, i) = 0.0

   ;  mx->cols[root].val = 1.0               /* bootstrap value */
   ;  SEEN(mx, root) = 1.0

   ;  while (n_add && (!extent || i_wave++ <= extent))
      {  dim j, n_add_new = 0

      ;  for (j=n_done; j<n_done+n_add; j++)
         {  mclv* node = mx->cols+order->ivps[j].idx
         ;  dim k
         ;  for (k=0;k<node->n_ivps;k++)
            {  mclp* edge = node->ivps+k
            ;  mclv* node2 = mx->cols + edge->idx
            ;  if (!SEEN(mx, node2->vid))
               {  if (!node2->val)
                  order->ivps[n_done + n_add + n_add_new++].idx = node2->vid
               ;  node2->val += node->val
;if(0)fprintf(stdout, "node %d value %g seen %d\n", (int) node2->vid, node2->val, (int) SEEN(mx, node2->vid))
            ;  }
               else
               edge->val = 0.0
         ;  }
;if(0)fputs("--\n", stdout);
                                       if(0)mclvUnary(node, fltxCopy, NULL)
      ;  }
;if(0)fprintf(stdout, "(added %d)\n", (int) n_add);
      ;  n_done += n_add
      ;  n_add = n_add_new
      ;  for (j=n_done;j<n_done+n_add;j++)
         SEEN(mx, order->ivps[j].idx) = 1.0
   ;  }
      return n_done
;  }


/*
   going upward in the lattice, for each edge do
      parent->val (initialised to 1.0 / #paths(parent))
      += (child->val)

   #paths is encoded in down->cols[X].val
   new accumulator encoded in up->cols[X].val
*/

void compute_scores_up_ec              /* edge centrality */
(  mclx* down
,  mclx* up       /* transpose of down for now */
,  mclx* edge
,  mclv* order
,  dim   n_order
)
   {  dim j

   ;  for (j=0;j<N_COLS(up);j++)
      {  if (down->cols[j].val)
         up->cols[j].val = 1.0/down->cols[j].val
   ;  }

;if(0)fputc('\n', stderr);
      for (j=0;j<n_order;j++)
      {  dim child = order->ivps[n_order-j-1].idx
;if(0)fprintf(stderr, "order %d\n", (int) child)
      ;  mclv* parents = up->cols+child
      ;  dim k
      ;  for (k=0; k<parents->n_ivps; k++)
         {  mclp* p = parents->ivps+k
         ;  if (p->val)
            up->cols[p->idx].val +=  up->cols[child].val
      ;  }
      }

   ;  for (j=0;j<n_order;j++)
      {  dim child = order->ivps[n_order-j-1].idx
;if(0)fprintf(stderr, "oo %d\n", (int) child)
      ;  mclv* parents = up->cols+child
      ;  dim k
      ;  for (k=0; k<parents->n_ivps; k++)
         {  mclp* p = parents->ivps+k
         ;  if (p->val)
            mclgArcAddto(edge, p->idx, child, down->cols[p->idx].val * up->cols[child].val)
#if 0
            {  if (p->idx < child)
            ;  else
               mclgArcAddto(edge, child, p->idx, down->cols[p->idx].val * up->cols[child].val)
         ;  }
#endif
      ;  }
      }
   }


/*
   going upward in the lattice, for each edge do
      parent->val (initialised to 1.0)
      += (child->val * #paths(parent) / #paths(child))

   #paths is encoded in down->cols[X].val
   new accumulator encoded in up->cols[X].val
*/

void compute_scores_up_vc              /* vertex centrality */
(  mclx* down
,  mclx* up       /* transpose of down for now */
,  mclv* order
,  dim   n_order
)
   {  dim j
   ;  for (j=0;j<N_COLS(up);j++)
      up->cols[j].val = 1.0            /* accumulator; initalised */
                                       /* question: initialise everywhere? */

   ;  for (j=0;j<n_order;j++)
      {  dim child = order->ivps[n_order-j-1].idx
;if(0)fprintf(stderr, "o %d\n", (int) child)
      ;  mclv* parents = up->cols+child
      ;  dim k
      ;  for (k=0; k<parents->n_ivps; k++)
         {  mclp* p = parents->ivps+k
         ;  if (p->val)
            up->cols[p->idx].val
            +=    (up->cols[child].val * down->cols[p->idx].val)
               /  down->cols[child].val
      ;  }
      }
   }



static void ctty_compute2
(  const mclx* mx
,  dim         offset
,  mclv*       ctty
,  mclx*       down
,  mclx*       up
,  mclx*       edge
,  mclv*       lattice_order
)  
   {  dim j, n_order
   ;  mclv* domain = NULL

   ;  n_order = cttyFlood2(down, offset, extent_g, lattice_order)

                                       /* should only do transpose for elements in order .. */
   ;  if (extent_g)
      {  domain = mclvFromIvps(NULL, lattice_order->ivps, n_order)
      ;  cttyUpdateDS(down, up, domain, 0.0)
   ;  }
      else
      up = mclxTranspose2(down, 0)
                                                      ;  if (0)mclxDebug("-", down, 0, "yoo")
   ;  if (do_edge_g)
      {  compute_scores_up_ec(down, up, edge, lattice_order, n_order)
   ;  }
      else
      {  compute_scores_up_vc(down, up, lattice_order, n_order)
      ;  for (j=1;j<n_order;j++)                   /* j =0 is an endpoint */
         {  dim k = lattice_order->ivps[j].idx
         ;  if (mclvSum(down->cols+k) > 0.5)       /* fixme semantics ?   */
            ctty->ivps[k].val += up->cols[k].val - 1.0
;if (0 && k == 0)
fprintf(stderr, "%d\t%.10g\n", (int) offset, up->cols[k].val - 1.0)
      ;  }
      }

                        /* set zero values to 1 in both up and down */
      if(extent_g)
      cttyUpdateDS(down, up, domain, 1.0)
   ;  else
         mclxFree(&up)
      ,  mclxMakeCharacteristic(down)

   ;  mclvFree(&domain)
;  }


typedef struct
{  mclv*    ctty
;  mclx*    up
;  mclx*    down
;  mclx*    edge
;  mclv*    lattice_order
;
}  ctty_data   ;


static void ctty_dispatch
(  mclx* mx
,  dim i
,  void* data
,  dim thread_id
)
   {  ctty_data* d = ((ctty_data*) data) + thread_id
   ;  ctty_compute2(mx, i, d->ctty, d->down, d->up, d->edge, d->lattice_order)
;  }



static mcxstatus cttyMain
(  int          argc_unused      cpl__unused
,  const char*  argv_unused[]    cpl__unused
)  
   {  mclx* mx, *ctty, *the_edge = NULL
   ;  mcxIO* xfout = mcxIOnew(out_g, "w")
   ;  ctty_data* data = NULL
   ;  dim i

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  n_thread_l = mclx_set_threads_or_die(mectty, n_thread_l, i_group, n_group_G)

   ;  mx = mcx_get_graph("mcx ctty", xfmx_g, xfabc_g, xftab_g, &tab_g, NULL, MCLX_REQUIRE_GRAPH | MCLX_REQUIRE_CANONICAL | MCL_READX_REMOVE_LOOPS)
   ;  mcxIOfree(&xfmx_g)
   ;  mclxMakeCharacteristic(mx)

   ;  if (do_edge_g)
      {  the_edge = mclxCopy(mx)
      ;  mclxZeroValues(the_edge)
   ;  }

      ctty
      =  mclxCartesian(mclvCanonical(NULL, MCX_MAX(n_thread_l,1), 1.0), mclvClone(mx->dom_rows), 0.0)

   ;  debug_g     =  mcx_debug_g
   ;  progress_g  =  mcx_progress_g

   ;  if (n_group_G * n_thread_l <= 1)
      {  mclx* down = mclxCopy(mx), *up
      ;  mclv* lattice_order = mclvCopy(NULL, mx->dom_cols)
      ;  mclxMakeCharacteristic(down)
      ;  up   = mclxTranspose(down)
      ;  for (i=0; i<N_COLS(mx); i++)
         ctty_compute2(mx, i, ctty->cols+0, down, up, the_edge, lattice_order)
   ;  }
      else
      {  dim t = 0
      ;  data = mcxAlloc(n_thread_l * sizeof data[0], EXIT_ON_FAIL)
      ;  for (t=0; t<n_thread_l; t++)
         {  ctty_data* d  = data+t
         ;  d->ctty  =  ctty->cols+t
         ;  d->down  =  mclxCopy(mx)
         ;  d->edge  =  do_edge_g ? mclxCopy(mx) : NULL
         ;  d->lattice_order = mclvCopy(NULL, mx->dom_cols)
         ;  mclxMakeCharacteristic(d->down)
         ;  if (d->edge)
            mclxZeroValues(d->edge)
         ;  d->up    =  mclxTranspose(d->down)
      ;  }
         mclxVectorDispatchGroup(mx, data, n_thread_l, ctty_dispatch, n_group_G, i_group, NULL)

      ;  if (debug_g && !do_edge_g)          /* do_edge_g needs data to collect results */
         {  for (t=0; t<n_thread_l; t++)
            {  ctty_data* d  = data+t
            ;  mclxFree(&d->down)
            ;  mclvFree(&d->lattice_order)
            ;  mclxFree(&d->up)
         ;  }
            mcxFree(data)
      ;  }
      }

                                             /* Scaling by 0.25:
                                              * factor two from two-way computation,
                                              * factor two from mergetranspose.
                                             */
      if (do_edge_g)
      {  double quart = 0.25

      ;  if (data)
         {  dim t
         ;  for (t=0; t<n_thread_l; t++)
            {  mclxAddTranspose(data[t].edge, 0.0)
            ;  mclxAugment(the_edge, data[t].edge, fltAdd)
         ;  }
         }
         else
         mclxMergeTranspose(the_edge, fltAdd, 0.0)

      ;  mclxUnary(the_edge, fltxMul, &quart)

      ;  if (tab_g)
         {  mclxIOdumper dumper = { 0 }
         ;  mclxIOdumpSet(&dumper, MCLX_DUMP_VALUES | MCLX_DUMP_PAIRS, "\t", "\t", "\t")
         ;  mclxIOdump(the_edge, xfout, &dumper, tab_g, tab_g, MCLXIO_VALUE_GETENV, RETURN_ON_FAIL)
      ;  }
         else
         mclxWrite(the_edge, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)
   ;  }
      else
      {  if (ctty->cols[0].n_ivps != N_COLS(mx))
         mcxDie(1, mectty, "internal error, tabulator miscount")

                        /* add subtotals to first vector */
      ;  for (i=1;i<N_COLS(ctty);i++)
         {  mclv* vec = ctty->cols+i
         ;  dim j
         ;  if (vec->n_ivps != N_COLS(mx))
            mcxDie(1, mectty, "internal error, tabulator miscount")
         ;  for (j=0;j<vec->n_ivps;j++)
            ctty->cols[0].ivps[j].val += vec->ivps[j].val
      ;  }

                        /* and report first vector, optionally with labels */
         {  mclv* v = ctty->cols+0
         ;  fprintf(xfout->fp, "node\tctty\n")
         ;  for (i=0;i<v->n_ivps;i++)
            {  double ctt  =  v->ivps[i].val / 2.0
            ;  long  vid   =  v->ivps[i].idx
            ;  if (tab_g)
               {  const char* label = mclTabGet(tab_g, vid, NULL)
               ;  if (!label) mcxDie(1, mectty, "panic label %ld not found", vid)
               ;  fprintf(xfout->fp, "%s\t%.12g\n", label, ctt)
            ;  }
               else
               fprintf(xfout->fp, "%ld\t%.12g\n",  vid, ctt)
         ;  }
         }
      }

      mcxIOfree(&xfout)
   ;  mclxFree(&mx)
   ;  mclxFree(&ctty)
   ;  return 0
;  }


mcxDispHook* mcxDispHookCtty
(  void
)
   {  static mcxDispHook cttyEntry
   =  {  "ctty"
      ,  "ctty [options]"
      ,  cttyOptions
      ,  sizeof(cttyOptions)/sizeof(mcxOptAnchor) - 1

      ,  cttyArgHandle
      ,  allInit
      ,  cttyMain

      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &cttyEntry
;  }




