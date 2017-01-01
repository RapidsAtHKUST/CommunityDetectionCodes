/*   (C) Copyright 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/*  This program outputs reordered clusterings so that ordering represents
 *  block structure at different levels from multiple clusterings (which are
 *  preferably more or less hierarchically organized).

 *  It first transforms a list of input matrices by taking successive
 *  meets, yielding a perfectly nested list of clusterings.  The
 *  clusterings are ordered from coarse to fine-grained.
 *  The meets are all internally reordered so that largest clusters
 *  come first.

 *  The resulting Babushka structure is then traversed to create the
 *  ordering.
*/

#include <string.h>
#include <stdio.h>

#include "clm.h"
#include "report.h"
#include "clmorder.h"

#include "util/io.h"
#include "util/types.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/compile.h"

#include "impala/matrix.h"
#include "impala/vector.h"
#include "impala/io.h"
#include "impala/compose.h"
#include "impala/iface.h"
#include "impala/app.h"

#include "clew/clm.h"
#include "clew/cat.h"

static const char* me = "clm order";

enum
{  MY_OPT_PREFIX = CLM_DISP_UNUSED
,  MY_OPT_OUTPUT
}  ;


enum
{  STABLE_OPT_IDENTITY = CLM_DISP_UNUSED
}  ;


static mcxOptAnchor orderOptions[] =
{  {  "-prefix"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PREFIX
   ,  "<string>"
   ,  "write new clusterings to files <string>1 <string>2 and so on"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "write cluster stack/tree to <fname>"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxOptAnchor stableOptions[] =
{  {  "-identity"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_PREFIX
   ,  "<num>"
   ,  "|meet| / |union| threshold for identity (1.0 maximally strict)"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;


static mcxIO*  xfout    =  (void*) -1;
static const char* prefix_g = (void*) -1;
static mcxbool multiplex = 1;
static mcxbool i_am_stable =  FALSE;
static double identity_g = -1;


static mcxstatus orderInit
(  void
)
   {  xfout = mcxIOnew("-", "w")
   ;  prefix_g = ""
   ;  multiplex = FALSE
   ;  return STATUS_OK
;  }


static mcxstatus stableInit
(  void
)
   {  i_am_stable    =  TRUE
   ;  xfout = mcxIOnew("-", "w")
   ;  identity_g = 0.9
   ;  return STATUS_OK
;  }


static mcxstatus orderArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_PREFIX
      :  prefix_g = val
      ;  multiplex = TRUE
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  mcxIOnewName(xfout, val)
      ;  break
      ;

         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus stableArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case STABLE_OPT_IDENTITY
      :  identity_g = atof(val)
      ;  break
      ;
      
         default
      :  return STATUS_FAIL
      ;
      }
      return STATUS_OK
;  }


static mcxstatus rerank
(  long     clid
,  dim      height
,  dim      height_return
,  mclxCat*  st
,  mclx**   maps
,  mclx*    newcls
,  dim*     n_cls
)  ;

#if 0

static void report_sizes
(  mclxCat* cat
)
   {  dim i
   ;  for (i=0;i<cat->n_level;i++)
      fprintf(stderr, "%4d", (int) N_COLS(cat->level[i].mx))
   ;  fputc('\n', stderr)
;  }

static void stable_dump_clustering
(  FILE* fp
,  int   level
,  double score
,  const mclv* cl
)
   {  dim k
   ;  fprintf
      (  fp
      ,  "%d\t%d\t%d\t%.2f\t"
      ,  (int) level
      ,  (int) cl->vid
      ,  (int) cl->n_ivps
      ,  score
      )
   ;  for (k=0;k<cl->n_ivps;k++)
      fprintf(stdout, "\t%d", (int) cl->ivps[k].idx)
   ;  fputc('\n', stdout)
;  }
#endif


static void extend_result
(  mclx* res
,  dim n_res
,  double value
,  const mclv* ls
)
   {  if (n_res >= res->dom_cols->n_ivps)
      {  mclv* new_domain = mclvCanonical(NULL, 2*n_res+1, 1.0)
      ;  mclxAccommodate(res, new_domain, NULL)
;if(0)fprintf(stderr, "result now allows %d clusters\n", (int) (2*n_res+1))
      ;  mclvFree(&new_domain)
   ;  }
      mclvCopy(res->cols+n_res, ls)
   ;  res->cols[n_res].val = value
;  }


static mcxstatus orderMain
(  int          argc
,  const char*  argv[]
)
   {  mcxIO       *xfin    =  mcxIOnew("-", "r")
   ;  mcxTing     *fname   =  mcxTingEmpty(NULL, 50)
   ;  dim         i        =  0, ii = 0
   ;  int         a        =  0

   ;  mclx       **maps    =  NULL     /* fixme/document purpose */
   ;  mcxbits     bits     =     MCLX_PRODUCE_PARTITION
                              |  MCLX_PRODUCE_DOMSTACK
                              |  MCLX_REQUIRE_CANONICALC

   ;  mclxCat st
   ;  mcxstatus status
   
   ;  mclxCatInit(&st)

   ;  if (i_am_stable)
      me = "clm stable"

   ;  while(a < argc)
      {  mcxIOnewName(xfin, argv[a])
      ;  status = mclxCatRead(xfin, &st, 0, NULL, NULL, bits)
      ;  mcxIOclose(xfin)
      ;  if (status)
         break
      ;  a++
   ;  }

      if (!a || a != argc)
      mcxDie(1, me, "failed to read one or more cluster files")

   ;  mclxCatSortCoarseLast(&st)

   ;  if (i_am_stable)
      {  dim lev
      ;  const mclv* dom = st.level[0].mx->dom_rows
      ;  const mclx* coarsest = st.level[st.n_level-1].mx

      ;  mclx* res = mclxAllocZero(mclvInit(NULL), mclvClone(dom))
      ;  dim n_res = 0

      ;  mcxIOopen(xfout, EXIT_ON_FAIL)

      ;  for (lev=0;lev<N_COLS(st.level[0].mx);lev++)
         st.level[0].mx->dom_cols->ivps[lev].val = 1.0  /* initialize */
      
      ;  for (lev=0;lev<st.n_level-1;lev++)
         {  mclx* cl_fi =  st.level[lev].mx
         ;  mclx* cl_co =  st.level[lev+1].mx
         ;  mclx* meet  =  clmContingency(cl_co, cl_fi)
         ;  mclv* co_acc = cl_co->dom_cols          /* co=coarse, fi=fine */
         ;  mclv* fi_acc = cl_fi->dom_cols
         ;  mclv* fi_terminated = mclvClone(cl_fi->dom_cols)
         ;  dim i_co, i

         ;  mclvMakeConstant(fi_terminated, 1.0)

         ;  for (i_co=0;i_co<N_COLS(cl_co);i_co++)     /* loop over coarser clusters */
            {  double co_size = cl_co->cols[i_co].n_ivps
            ;  double co_best_identity = 0.0
            ;  ofs co_choice_fi = -1
            ;  dim m

            ;  for (m=0;m<meet->cols[i_co].n_ivps;m++)
               {  double n_meet  =  meet->cols[i_co].ivps[m].val, identity
               ;  ofs i_fi       =  meet->cols[i_co].ivps[m].idx

               ;  identity       =  (1.0 * n_meet) / (cl_fi->cols[i_fi].n_ivps + co_size - n_meet)

               ;  if (identity > co_best_identity)
                     co_best_identity = identity
                  ,  co_choice_fi = i_fi
            ;  }

               if (co_best_identity > identity_g)
                  co_acc->ivps[i_co].val = 1.0 + fi_acc->ivps[co_choice_fi].val
               ,  fi_terminated->ivps[co_choice_fi].val = 0.5
         ;  }

            for (i=0;i<fi_terminated->n_ivps;i++)
            {  if (fi_terminated->ivps[i].val > 0.75 && fi_acc->ivps[i].val > 1.5)
               extend_result(res, n_res++, fi_acc->ivps[i].val,  cl_fi->cols+i)
         ;  }

            mclxFree(&meet)
         ;  mclvFree(&fi_terminated)
      ;  }

         for (i=0;i<N_COLS(coarsest);i++)
         if (coarsest->dom_cols->ivps[i].val > 1.5)
         extend_result(res, n_res++, coarsest->dom_cols->ivps[i].val,  coarsest->cols+i)

      ;  mclvResize(res->dom_cols, n_res)
      ;  if (1)
         mclxColumnsRealign(res, mclvValRevCmp)
      ;  mclxWrite(res, xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)

      ;  mclxFree(&res)
      ;  mcxIOclose(xfout)
      ;  mcxIOfree(&xfout)
      ;  exit(0)
   ;  }

      mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
   ;  mclx_app_init(stderr)

   ;  maps = mcxAlloc(st.n_level * sizeof(mclx*), EXIT_ON_FAIL)

   ;  if (status || !st.n_level)
      mcxDie(1, me, "not happy, not happy at all")

#define MX(i) (st.level[i].mx)

   ;  for (i=st.n_level;i>0;i--)
      {  if (i == st.n_level)
         {  mclxColumnsRealign(MX(i-1), mclvSizeRevCmp)
         ;  mclxMakeCharacteristic(MX(i-1))
      ;  }
         else
         {  mclx* finer = MX(i-1)
         ;  mclx* clinv, *map

         ;  MX(i-1)  =  clmMeet(MX(i), finer)
         ;  mclxColumnsRealign(MX(i-1), mclvSizeRevCmp)
         ;  mclxMakeCharacteristic(MX(i-1))

         ;  clinv    =  mclxTranspose(MX(i-1))
         ;  map      =  mclxCompose(clinv, MX(i), 0, 0)
         ;  maps[i]  =  map

         ;  mclxFree(&finer)
         ;  mclxFree(&clinv)
      ;  }
      }

      maps[0]  =  NULL

   ;  for (ii=0;ii<st.n_level;ii++)
      {  dim lev        =  ii
      ;  mclx* cls0     =  MX(st.n_level-1)
      ;  mclx* clsnew   =  mclxAllocZero(mclvClone(MX(0)->dom_rows), mclvClone(MX(0)->dom_rows))
      ;  dim n_cls      =  0

      ;  if (multiplex)
         {  mcxTingPrint(fname, "%s%d", prefix_g, (int) (st.n_level-lev))
         ;  mcxIOnewName(xfout, fname->str)
         ;  mcxIOopen(xfout, EXIT_ON_FAIL)
      ;  }

         for (i=0;i<cls0->dom_cols->n_ivps;i++)
         if (rerank(cls0->dom_cols->ivps[i].idx, st.n_level-1, lev, &st, maps, clsnew, &n_cls))
         break

      ;  if (i == cls0->dom_cols->n_ivps)
         {  mclvResize(clsnew->dom_cols, n_cls)
         ;  mclxWrite(clsnew, xfout, MCLXIO_VALUE_NONE, EXIT_ON_FAIL)
         ;  if (prefix_g[0])
            mcxIOclose(xfout)
      ;  }
         mclxFree(&clsnew)
   ;  }

      for (i=0;i<st.n_level;i++)
      {  mclxAnnot* ant = st.level+i
      ;  if (ant->mx)
         mclxFree(&(ant->mx))
      ;  if (ant->mxtp)
         mclxFree(&(ant->mxtp))
      ;  mclxFree(&(maps[i]))
   ;  }

      mcxIOfree(&xfin)
   ;  mcxIOfree(&xfout)
   ;  mcxTingFree(&fname)
   ;  mcxFree(maps)
   ;  mcxFree(st.level)
   ;  return 0
;  }


static mcxstatus rerank
(  long     clid
,  dim      height
,  dim      height_return
,  mclxCat *st
,  mclx**   maps
,  mclx*    clsnew
,  dim*     n_cls
)
   {  mclx* cls = st->level[height].mx
   ;  mclx* map = maps[height]
   ;  mcxstatus status = STATUS_FAIL
   ;  mclv* domv
   ;  const char* errmsg = "all's well"

;if(0) fprintf(stderr, "in at %ld/%ld\n", (long) clid, (long) height)

   ;  while (1)
      {  if (!cls)
         {  errmsg = "no cls"
         ;  break
      ;  }

         domv = mclxGetVector(cls, clid, RETURN_ON_FAIL, NULL)

      ;  if (!domv)
         {  errmsg = "no domv"
         ;  break
      ;  }

         if (height == height_return)
         {  mclvCopy(clsnew->cols+n_cls[0], domv)
         ;  (*n_cls)++
      ;  }
         else
         {  mclv* mapv = mclxGetVector(map, clid, RETURN_ON_FAIL, NULL)
         ;  dim i
         ;  if (!mapv)
            {  errmsg = "no mapv"
            ;  break
         ;  }

            for (i=0;i<mapv->n_ivps;i++)
            if (rerank(mapv->ivps[i].idx, height-1, height_return, st, maps, clsnew, n_cls))
            break

         ;  if (i != mapv->n_ivps)
            break
      ;  }

         status = STATUS_OK
      ;  break
   ;  }

      if (status)
      {  mcxErr("rerank", "panic <%s> clid/height %ld/%ld", errmsg, (long) clid, (long) height)
      ;  return STATUS_FAIL
   ;  }
      return STATUS_OK
;  }


mcxDispHook* mcxDispHookOrder
(  void
)
   {  static mcxDispHook orderEntry
   =  {  "order"
      ,  "order [options] <cl-[stack-]file>+"
      ,  orderOptions
      ,  sizeof(orderOptions)/sizeof(mcxOptAnchor) - 1
      ,  orderArgHandle
      ,  orderInit
      ,  orderMain
      ,  1
      ,  -1
      ,  MCX_DISP_MANUAL
      }
   ;  return &orderEntry
;  }


mcxDispHook* mcxDispHookStable
(  void
)
   {  static mcxDispHook stableEntry
   =  {  "stable"
      ,  "stable <cl-[stack-]file>+"
      ,  stableOptions
      ,  sizeof(stableOptions)/sizeof(mcxOptAnchor) - 1
      ,  stableArgHandle
      ,  stableInit
      ,  orderMain
      ,  1
      ,  -1
      ,  MCX_DISP_HIDDEN
      }
   ;  return &stableEntry
;  }

