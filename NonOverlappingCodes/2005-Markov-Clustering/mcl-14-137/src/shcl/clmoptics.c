/*   (C) Copyright 2012, 2013 Stijn van Dongen
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
#include "clmoptics.h"

#include "util/io.h"
#include "util/minmax.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/compile.h"
#include "util/heap.h"

#include "impala/pval.h"
#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "mcl/interpret.h"

#include "clew/claw.h"
#include "clew/clm.h"
#include "clew/cat.h"


enum
{  MY_OPT_IMX     =  CLM_DISP_UNUSED
,  MY_OPT_OUTPUT
,  MY_OPT_ICL
,  MY_OPT_PS
,  MY_OPT_PSX
,  MY_OPT_PSY
,  MY_OPT_PS_LABELS
,  MY_OPT_PS_ROWS
,  MY_OPT_MAXEPS
,  MY_OPT_MINEPS
,  MY_OPT_MINPTS
}  ;


static mcxOptAnchor opticsOptions[] =
{  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-ps"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PS
   ,  "<fname>"
   ,  "write PS file"
   }
,  {  "-psx"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PSX
   ,  "<num>"
   ,  "scaling correction in x direction (default 1.0)"
   }
,  {  "-psy"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PSY
   ,  "<num>"
   ,  "scaling correction in y direction (default 1.0)"
   }
,  {  "--ps-labels"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_PS_LABELS
   ,  NULL
   ,  "show numerical labels in PS output"
   }
,  {  "-ps-rows"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PS_ROWS
   ,  "<num>"
   ,  "split reachability plot into <num> rows"
   }
,  {  "-imx"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_IMX
   ,  "<fname>"
   ,  "read graph matrix from file"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "read cluster matrix from file"
   }
,  {  "-max-eps"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MAXEPS
   ,  "<threshold>"
   ,  "maximum distance considered (use when weights represent dissimilarities)"
   }
,  {  "-min-eps"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MINEPS
   ,  "<threshold>"
   ,  "minimum distance considered (use when weights represent similarities)"
   }
,  {  "-min-pts"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_MINPTS
   ,  "<int>"
   ,  "number of neighbours required in epsilon-neighbourhood"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
static mcxIO*  xfmx     =  (void*) -1;
static mcxIO*  xfcl     =  (void*) -1;
static mcxIO*  xfps     =  (void*) -1;
static dim     minpts   =  -1;
static double  maxeps   =  -1;
static double  mineps   =  -1;
static double  ps_x_correct   =  1.0;
static double  ps_y_correct   =  1.0;
static double  ps_scalex   =  0.0;
static double  ps_scaley   =  0.0;
static unsigned char ps_nrows = -1;
static mcxbool ps_labels = -1;
static const char* cutoff_g= (void*) -1;


static mcxstatus opticsInit
(  void
)
   {  xfout =  mcxIOnew("-", "w")
   ;  xfmx  =  NULL
   ;  xfcl  =  NULL
   ;  xfps  =  NULL
   ;  minpts =  0
   ;  maxeps =  0
   ;  mineps =  0
   ;  ps_labels = FALSE
   ;  ps_nrows = 1
   ;  cutoff_g = "cutoff"
   ;  return STATUS_OK
;  }


static mcxstatus opticsArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         case MY_OPT_PS
      :  xfps = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_PSX
      :  ps_x_correct = atof(val)
      ;  break
      ;

         case MY_OPT_PSY
      :  ps_y_correct = atof(val)
      ;  break
      ;

         case MY_OPT_PS_ROWS
      :  ps_nrows = atoi(val)
      ;  break
      ;

         case MY_OPT_PS_LABELS
      :  ps_labels = TRUE
      ;  break
      ;

         case MY_OPT_ICL
      :  xfcl = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_IMX
      :  xfmx = mcxIOnew(val, "r")
      ;  break
      ;

         case MY_OPT_MINPTS
      :  minpts = atoi(val)
      ;  break
      ;

         case MY_OPT_MINEPS
      :  mineps = atof(val)
      ;  cutoff_g = val
      ;  break
      ;

         case MY_OPT_MAXEPS
      :  maxeps = atof(val)
      ;  cutoff_g = val
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static double reverse_range
(  pval  val
,  void* reflect
)
   {  return ((double*) reflect)[0] - val
;  }

static int cmp_double(const void* d, const void* e)
   {  return ((double*) d)[0] < ((double*) e)[0] ? -1 : ((double*) d)[0] > ((double*) e)[0] ? 1 : 0
;  }

static const char* me = "clm optics";


#if 0

#define HEAP_SET_PROCESSED(a)  a = -10.0
#define HEAP_TEST_PROCESSED(a) (a < -5.0)
#define HEAP_INIT_VALUE            -2.0
#define HEAP_TEST_UNSEEN(a)    (a > -3.0 && a < 0)

#else

#define HEAP_SET_PROCESSED(a)   a = FLT_MAX / 1e1
#define HEAP_TEST_PROCESSED(a) (a > FLT_MAX / 1e2)
#define HEAP_INIT_VALUE             FLT_MAX / 1e3
#define HEAP_TEST_UNSEEN(a)    (a > 10 * maxeps)

#endif


mclv* init_reachability
(  const mclv* coredistance
,  const mclx* mx
)
   {  mclv* rb = mclvInit(NULL)
   ;  mcxHeap *h = mcxHeapNew(NULL, coredistance->n_ivps+1, sizeof(mclp), mclpValRevCmp)
   ;  dim j
   ;  for (j=0; j< coredistance->n_ivps; j++)
      {  mclp p = coredistance->ivps[j]
      ;  if (mx->cols[j].n_ivps < minpts)
         p.val = HEAP_INIT_VALUE
      ;  else
         p.val = 20 * maxeps + 100.0 * p.val
      ;  mcxHeapInsert(h, &p)
   ;  }

      rb->ivps = h->base         /* fixme heap should have release method; now leaking memory */
   ;  rb->n_ivps = coredistance->n_ivps
   ;  HEAP_SET_PROCESSED(rb->ivps[rb->n_ivps].val)
   ;  return rb
;  }


mclv* get_coredistance
(  const mclx* mx
)
   {  mclv* coredist = mclvCanonical(NULL, N_COLS(mx), 0)
   ;  mcxHeap *h = mcxHeapNew(NULL, minpts, sizeof(double), cmp_double)
   ;  dim j
   ;  for (j=0; j< N_COLS(mx); j++)
      {  mclv* v = mx->cols+j
      ;  if (v->n_ivps < minpts)
         coredist->ivps[j].val = maxeps
      ;  else
         {  double val
         ;  dim jj
         ;  mcxHeapClean(h)
         ;  for (jj=0; jj<v->n_ivps; jj++)
            {  val = v->ivps[jj].val
            ;  mcxHeapInsert(h, &val)
         ;  }
            coredist->ivps[j].val = ((double*) h->base)[0]
      ;  }
if(0)fprintf(stdout, "node %d value %g\n", (int) j, coredist->ivps[j].val)
;     }
      mcxHeapFree(&h)
   ;  return coredist
;  }













/*
   while (unprocessed)
      node <- first unprocessed node
      todo <- (node)
      expand(todo)
   done

   expand(todo)
      while (shift node from todo)
         output(node, coredistance(node), rb(node))
         if (is_core(node))
                                       # note: all epsilon nbs from node.
            merge(nb(node), todo)      # ignore those already processed.
            for (n in nb(node))
               update(reachability(n)) # wrt node
               reorder(todo)
            done
         fi
      done
   end
*/


void test_heap
(  mclv* reachability
,  dim* h_position
,  char* caller
)
   {  dim   i
   ;  if (0)fprintf(stdout, "--- heap test by %s ---\n", caller)
   ;  for (i=0; i< reachability->n_ivps;i++)
      if (reachability->ivps[h_position[i]].idx != i)
      mcxDie(1, me, "heap error")
      /* fprintf(stderr, "node=%d heapptr=%d heapidx=%d\n", (int) i, (int) h_position[i], (int) h[h_position[i]].idx) */
;  }


void heap_update_node
(  mclv* reachability
,  dim* h_position
,  dim nbid
)
   {  mclp* h = reachability->ivps
   ;  dim   p = h_position[nbid]
   ;  double testval = h[p].val
   ;  mclp  movee = h[p]

   ;  while (p > 0 && h[(p+1)/2 - 1].val > testval)
      {  dim parent = (p+1) / 2 - 1
      ;  h[p] = h[parent]
      ;  h_position[h[p].idx]  = p
      ;  p = parent
   ;  }

      h[p] = movee
   ;  h_position[h[p].idx] = p

;if(1)test_heap(reachability, h_position, "update")
   ;  if (h_position[h[0].idx] != 0)
      mcxDie(1, me, "update heap error (index %d at root, points to %d", (int) h[0].idx, (int) h_position[h[0].idx])
;  }


                        /* refreshes from top to bottom; leaves leaf gap */
void heap_refresh_root
(  mclv* reachability
,  dim* h_position
)
   {  mclp* h = reachability->ivps
   ;  dim exile = 0, child = 0
   ;  mclp root = h[0]
   ;  int delta
   
   ;  if (h_position[h[0].idx] != 0)
      mcxDie(1, me, "intro refresh heap error (index %d at root, points to %d", (int) h[0].idx, (int) h_position[h[0].idx])

                  /* note: reachability was over-alloc'ed, so we can quiz 2 * exile + 2 */
   ;  while (2 * exile + 1 < reachability->n_ivps)
      {  child = 2*exile+1
      ;  delta = h[child].val < h[child+1].val ? 0 : 1
      ;  child += delta


      ;  if (HEAP_TEST_PROCESSED(h[child].val))
         break
      ;  h[exile] = h[child]                   /* move target up */
      ;  h_position[h[exile].idx] = exile
      ;  exile = child
   ;  }

      if (exile)
      {  h[exile] = root
      ;  h_position[h[exile].idx] = exile
   ;  }

      HEAP_SET_PROCESSED(h[exile].val)

;if(0)test_heap(reachability, h_position, "refresh")
   ;  if (h_position[h[0].idx] != 0)
      mcxDie(1, me, "exit refresh heap error (index %d at root, points to %d", (int) h[0].idx, (int) h_position[h[0].idx])
;  }


/* fixme:
 *    ps logic too much intertwined: the usual.
*/

void do_optics
(  mcxIO* xf
,  mcxIO* xfps
,  const mclx* mx
,  const mclx* cl
,  double maxeps
,  double extreme_value
,  dim    minpts
,  double reverse_post
)
   {  mclv*  v_coredist       =  get_coredistance(mx)
   ;  mclv*  h_reachability   =  mclvCanonical(NULL, N_COLS(mx)+1, HEAP_INIT_VALUE)     /* this is a heap .... dangersign */
   ;  dim*   h_position       =  mcxNAlloc(N_COLS(mx), sizeof h_position[0], NULL, EXIT_ON_FAIL)
   ;  dim n_processed = 0
   ;  dim n_component = 0
   ;  mclx* cltp = cl ? mclxTranspose(cl) : NULL
   ;  double cutoff = maxeps
   ;  double yrange_width = maxeps

   /*
   ;  unsigned B[13] =  { 4, 5, 6, 7, 8, 9, 10, 10, 9, 8, 7, 6, 5 }
   ;  unsigned S[15] =  { 10, 9, 8, 7, 6, 5, 4, 3, 3, 4, 5, 6, 7, 8, 9 }

   ;  unsigned B[13] =  { 10, 9, 8, 7, 6, 5, 4, 5, 6, 7, 8, 9, 10 }
   ;  unsigned S[7] =  { 3, 5, 7, 10, 8, 6, 4 }

   ;  unsigned B[11] =  { 10, 9, 8, 6, 4, 3, 4, 5, 7, 9, 10}
   ;  unsigned S[7] =  { 3, 5, 7, 10, 8, 6, 4 }
   */
   ;  unsigned H[23] =  {  3,  6,  9, 12, 16
                        , 20, 24, 28, 32, 36
                        , 41, 46, 51, 56, 61
                        , 66, 73, 79, 84, 89
                        , 93, 97
                        }
   ;  unsigned B[11] =  { 10, 9, 8, 6, 4, 3, 4, 5, 7, 9, 10}
   ;  unsigned S[7] =  { 3, 5, 7, 10, 8, 6, 4 }

   ;  unsigned hue_i  = 0
   ;  unsigned brightness_i = 0
   ;  unsigned saturation_i = 0

   ;  unsigned h_inc = 3
   ;  unsigned b_inc = 1
   ;  unsigned s_inc = 1
   ;  dim j
   ;  dim break_at   =     ps_nrows <= 1
                        ?  N_COLS(mx)
                        :  (N_COLS(mx) + ps_nrows * (N_COLS(mx) % ps_nrows != 0)) / ps_nrows
   ;  dim i_row = 0
   ;  double mark = ((int) (0.5 + (100.0) * extreme_value)) / 100.0
   ;  char mark_buf[10]

   ;  snprintf(mark_buf, 10, "%g", mark)

;fprintf(stderr, "break at %d, yrange %g, cutoff %g, reverse_post %g\n", (int) break_at, yrange_width, cutoff, reverse_post)

               /* below is a bit of an evil hack; it ensures that
                  2 * i < reachability->n_ivps implies 2 * i + 1 < reachability->n_ivps
               * This is useful when dealing with heaps.
               */
   ;  if (0)
      {  h_reachability->n_ivps  =  N_COLS(mx)
      ;  HEAP_SET_PROCESSED(h_reachability->ivps[N_COLS(mx)].val)
   ;  }
      else
      h_reachability = init_reachability(v_coredist, mx)

   ;  for (j=0;j<h_reachability->n_ivps;j++)
      h_position[h_reachability->ivps[j].idx] = j

   ;  test_heap(h_reachability, h_position, "main")

   ;  ps_scalex = ps_x_correct * 1100 * 1.0 / break_at
   ;  mcxTell(me, "Set x scaling factor to %g", ps_scalex)

   ;  ps_scaley = ps_y_correct * ((842.0 - 140.0) / ps_nrows) / yrange_width
   ;  mcxTell(me, "Set y scaling factor to %g", ps_scaley)

   ;  if (xfps)
      {  fputs(
"%%BoundingBox: 1 1 1189 841\n"
"%%Pages: 1\n"
"%%Orientation: Landscape\n"
"%%DocumentMedia: A3land 1190 842 0 () ()\n"
"%%EndComments\n"
"%%BeginFeature: *PageSize A3\n"
"<< /PageSize [1190 842] /Orientation 0 >> setpagedevice\n"
"%%EndFeature\n"
"\n"
"/proc_set_font {\n"
"   findfont exch scalefont setfont\n"
"}  def\n"
"/Times-Roman findfont 10 scalefont setfont\n"

,  xfps->fp
   )
      ;  fprintf(xfps->fp, "\n/proc_scale { exch %g mul exch %g mul } def\n", 1.0 / ps_scalex, 1.0 / ps_scaley)
      ;  fprintf(xfps->fp, "\n60 110 translate %g %g scale 0 %g translate\n", ps_scalex, ps_scaley, (ps_nrows-1) * yrange_width)
   ;  }

      while (n_processed < N_COLS(mx))
      {  mclp* pivot_ivp = h_reachability->ivps+0
      ;  dim pivot_idx   = pivot_ivp->idx
      ;  double rb_val   = pivot_ivp->val
      ;  mclv* nb        = mx->cols+pivot_idx
      ;  double cdist    = v_coredist->ivps[pivot_idx].val
      ;  double rb_out   = HEAP_TEST_UNSEEN(rb_val) ? cdist : rb_val

      ;  ofs clsid = cltp ? mclxGetClusterID(cltp, pivot_idx, 1) : -1
      ;  dim jj

      ;  if (HEAP_TEST_UNSEEN(rb_val))
         {  fprintf
            (  xf->fp
            ,  "---\t%.2f\t%.2f\t0\t%d\n"
            ,  cutoff
            ,  cutoff
            ,  (int) n_processed
            )
         ;  if (0 && xfps)
            fprintf(xfps->fp,
            "gsave %d %f moveto 0 %g rlineto 0.7 0 0 setrgbcolor stroke grestore\n", (int) n_processed, maxeps, maxeps/2)

         ;  n_component++
      ;  }

         fprintf
         (  xf->fp
         ,  "%d\t%.2f\t%.2f\t%d\t%d\n"
         ,  (int) pivot_idx
         ,  !reverse_post ? rb_out : reverse_post - rb_out
         ,  !reverse_post ? cdist : reverse_post - cdist
         ,  (int) nb->n_ivps
         ,  (int) (n_processed)
         )

      ;  if (clsid >= 0)
         {  hue_i = (3 + h_inc * clsid) % (sizeof H / sizeof H[0])
         ;  brightness_i = (1 + b_inc * clsid) % (sizeof B / sizeof B[0])
         ;  saturation_i = (1 + s_inc * clsid) % (sizeof S / sizeof S[0])
      ;  }
         else
         {  hue_i        = (hue_i        + h_inc) % (sizeof H / sizeof H[0])
         ;  brightness_i = (brightness_i + b_inc) % (sizeof B / sizeof B[0])
         ;  saturation_i = (saturation_i + s_inc) % (sizeof S / sizeof S[0])
      ;  }

         if (xfps)
         fprintf
         (  xfps->fp
         ,  "%.2f %.2f %.2f sethsbcolor %d 0 moveto 0 %g rlineto stroke\n"
         ,  H[hue_i]        / 100.0
         ,  S[saturation_i] / 10.0
         ,  B[brightness_i] / 10.0
         ,  (int) (n_processed % break_at)
         ,  rb_out
         )
      ;  if (ps_labels && xfps)
         fprintf
         (  xfps->fp
         ,  "gsave %g %g moveto %g %g scale 90 rotate (%d) show grestore\n"
         ,  (n_processed % break_at) + 0.5
         ,  rb_out
         ,  1.0/ps_scalex
         ,  1.0/ps_scaley
         ,  (int) pivot_idx
         )

      ;  heap_refresh_root(h_reachability, h_position)

      ;  if (nb->n_ivps < minpts)

      ;  else
         for (jj = 0; jj < nb->n_ivps; jj++)
         {  dim    nb_id      =  nb->ivps[jj].idx
         ;  mclp*  nb_rb      =  h_reachability->ivps + h_position[nb_id]
         ;  double nb_rb_new  =  MCX_MAX(v_coredist->ivps[pivot_idx].val, nb->ivps[jj].val)

         ;  if (HEAP_TEST_PROCESSED(nb_rb->val))
            continue

         ;  if (nb_rb->idx != nb_id)
            mcxDie(1, me, "heap error in loop: pivot %d to nb %d heap %d, %g", (int) pivot_idx, (int) nb_id, (int) nb_rb->idx, (double) nb_rb->val)

         ;  if (nb_rb_new < nb_rb->val)       /* bit ugly to split overwrite (here) and update (below), but hey */
            nb_rb->val = nb_rb_new

;if(0)fprintf(stdout, "push %d value %g\n", (int) nb_id, (double) nb_rb->val)
         ;  heap_update_node(h_reachability, h_position, nb_id)
;if(0)fprintf(stdout, "heap root %d %g (updated %d value %g)\n", (int) h_reachability->ivps[0].idx, h_reachability->ivps[0].val, (int) nb_id, nb_rb_new)
      ;  }

         n_processed++
      ;  if (xfps && ((n_processed % break_at == 0) || n_processed == N_COLS(mx)))        /* yuckly */
         {  i_row++
   ;  fputs("gsave\n", xfps->fp)
   ;  fputs("0 0 0 sethsbcolor\n", xfps->fp)
      ;  fprintf(xfps->fp, "-30 0 proc_scale moveto %g setlinewidth 0 %g rlineto stroke\n", 1.0 / ps_scaley, maxeps)
      ;  fprintf
         (  xfps->fp
         ,  "-30 0 proc_scale moveto 0 %g rmoveto"
            " gsave 1 1 proc_scale scale (%s_) stringwidth pop -1 mul -7 rmoveto (%s) show grestore\n"
         ,  maxeps, cutoff_g, cutoff_g
         )
      ;  fprintf
         (  xfps->fp
         ,  "-30 0 proc_scale moveto 0 %g rmoveto"
            " gsave 1 1 proc_scale scale (%s_) stringwidth pop -1 mul 0 rmoveto (%s) show grestore\n"
         ,  reverse_post ? reverse_post - extreme_value : extreme_value, mark_buf, mark_buf
         )
   ;  fputs("grestore\n", xfps->fp)
         ;  fprintf(xfps->fp, "\n0 -%g -%g add translate\n", yrange_width, 100.0 / ((ps_nrows) * ps_scaley))
      ;  }
   ;  }
      fprintf
      (  xf->fp
      ,  "---\t%.2f\t%.2f\t0\t%d\n"
      ,  !reverse_post ? maxeps : reverse_post - maxeps
      ,  !reverse_post ? maxeps : reverse_post - maxeps
      ,  (int) (n_processed + n_component)
      )
   ;  if (xfps)
      fprintf(xfps->fp, "showpage\n")
;  }



static mcxstatus opticsMain
(  int         argc_unused    cpl__unused
,  const char* argv_unused[]  cpl__unused
)
   {  mclx  *mx         =  NULL, *cl = NULL
   ;  double reverse_post     =  0.0
   ;  double extreme_value    =  0.0

   ;  if (!xfmx || !minpts || ((maxeps != 0) + (mineps != 0) != 1))
      mcxDie(1, me, "need -imx, -min-pts, and one of -min-eps or -max-eps")

   ;  mx = mclxReadx(xfmx, EXIT_ON_FAIL, MCLX_REQUIRE_GRAPH | MCLX_REQUIRE_CANONICAL)

   ;  if (xfcl)
      cl = mclxReadx(xfcl, EXIT_ON_FAIL, MCLX_REQUIRE_PARTITION)

   ;  mclxAdjustLoops(mx, mclxLoopCBremove, NULL)

   ;  if (mineps)
      {  extreme_value = mclxMaxValue(mx)
      ;  reverse_post = extreme_value * 1.2
      ;  mclxSelectValues(mx, &mineps, NULL, 0)
      ;  mclxUnary(mx, reverse_range, &reverse_post)
      ;  maxeps = reverse_post - mineps
      ;  mclxSelectValues(mx, NULL, &maxeps, 0)
      ;  mcxTell(me, "converted range [0,%g] to [%g,%g], maxeps %g", extreme_value, reverse_post-extreme_value, reverse_post, maxeps)
   ;  }
      else if (maxeps)
      {  mclxSelectValues(mx, NULL, &maxeps, 0)
   ;  }

      if (0) mclxWrite(mx, xfout, MCLXIO_VALUE_GETENV, EXIT_ON_FAIL)

   ;  if (xfps)
      mcxIOopen(xfps, EXIT_ON_FAIL)
   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  do_optics(xfout, xfps, mx, cl, maxeps, extreme_value, minpts, reverse_post)

   ;  mcxIOclose(xfout)
   ;  if (xfps)
      mcxIOclose(xfps)

   ;  return STATUS_OK
;  }



mcxDispHook* mcxDispHookOptics
(  void
)
   {  static mcxDispHook opticsEntry
   =  {  "optics"
      ,  "optics -imx <mx-file> -min-pts <int> -{max,min}-eps <threshold>"
      ,  opticsOptions
      ,  sizeof(opticsOptions) / sizeof(mcxOptAnchor) - 1
      ,  opticsArgHandle
      ,  opticsInit
      ,  opticsMain
      ,  0
      ,  0 
      ,  MCX_DISP_DEFAULT
      }
   ;  return &opticsEntry
;  }


