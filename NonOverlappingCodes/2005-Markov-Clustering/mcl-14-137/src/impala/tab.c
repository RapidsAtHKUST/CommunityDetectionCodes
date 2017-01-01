/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "tab.h"
#include "vector.h"
#include "matrix.h"
#include "edge.h"

#include "util/alloc.h"
#include "util/types.h"
#include "util/ding.h"
#include "util/err.h"
#include "util/array.h"
#include "util/ting.h"
#include "util/hash.h"



void mclTabFree
(  mclTab**       tabpp
)
   {  mclTab* tab = *tabpp
   ;  if (tab)
      {  if (tab->labels)
         {  char** lblpp = tab->labels
         ;  while(*lblpp)
            {  mcxFree(*lblpp)
            ;  lblpp++
         ;  }
            mcxFree(tab->labels)
      ;  }
         mclvFree(&(tab->domain))
      ;  mcxTingFree(&(tab->na))
      ;  mcxFree(tab)
      ;  *tabpp = NULL
   ;  }
   }


/* fixme: get rid of mcxResize. */

mclTab*   mclTabRead
(  mcxIO*         xf
,  const mclv*    dom
,  mcxOnFail      ON_FAIL
)
   {  mclTab* tab    =  mcxAlloc(sizeof(mclTab), EXIT_ON_FAIL)
   ;  mcxTing* line  =  mcxTingEmpty(NULL, 100)
   ;  mclpAR*     ar =  mclpARensure(NULL, 100)
   ;  const char* me =  "mclTabRead"
   ;  mcxstatus status= STATUS_OK
   ;  int c_seen     =  0
   ;  dim n_ivps     =  0  
   ;  long vid       =  0
   ;  long vidprev   =  -1

   ;  dim sz_labels  =  80
   ;  char** labels  =  mcxAlloc(sz_labels * sizeof(char*), EXIT_ON_FAIL)

   ;  tab->domain    =  mclvResize(NULL, 0)
   ;  tab->labels    =  NULL
   ;  tab->na        =  mcxTingNew("?")

   ;  if ((status = mcxIOtestOpen(xf, ON_FAIL)))
         mcxErr(me, "stream open error")
      ,  status = STATUS_FAIL
   ;  else
      while(STATUS_OK == (status = mcxIOreadLine(xf, line, MCX_READLINE_CHOMP)))
      {  char* c
      ;  status = STATUS_FAIL

      ;  if (!(c = mcxStrChrAint(line->str, isspace, line->len)) || *c == '#')
         continue
      ;  if (sscanf(line->str, "%ld%n", &vid, &c_seen) != 1)
         {  mcxErr(me, "expected vector index")
         ;  break
      ;  }
         if (vid <= vidprev)
         {  mcxErr
            (me, "order violation: <%ld> follows <%ld>", vid, vidprev)
         ;  break
      ;  }
         if (dom && (!dom->n_ivps || dom->ivps[n_ivps].idx != vid))
         {  mcxErr
            (me, "domain violation: unexpected index <%ld>", vid)
         ;  break
      ;  }

         while (isspace((unsigned char)*(line->str+c_seen)))
         c_seen++

      ;  n_ivps++

      ;  if (mclpARextend(ar, vid, 1.0))
         break

      ;  vidprev = vid

      ;  if
         (  sz_labels <= n_ivps
         && mcxResize
            (  &labels
            ,  sizeof(char*)
            ,  &sz_labels
            ,  n_ivps * 2
            ,  ON_FAIL
            )
         )
         break

      ;  labels[n_ivps-1] = mcxTingSubStr(line, c_seen, -1)
      ;  status = STATUS_OK
   ;  }

      while (1)
      {  if (status == STATUS_FAIL)
         break
      ;  status = STATUS_FAIL
      ;  if (dom && ar->n_ivps != dom->n_ivps)
         {  mcxErr
            (  me
            ,  "label count mismatch: got/need %ld/%ld"
            ,  (long) ar->n_ivps
            ,  (long) dom->n_ivps
            )
         ;  break
      ;  }

         mclvFromPAR(tab->domain, ar, 0, NULL, NULL)

      ;  if (  mcxResize         /* return unused memory */
               (  &labels
               ,  sizeof(char*)
               ,  &sz_labels
               ,  n_ivps + 1
               ,  ON_FAIL
             ) )
         break
      ;  labels[n_ivps] = NULL
      ;  tab->labels = labels
      ;  mclpARfree(&ar)
      ;  status = STATUS_OK
      ;  break
   ;  }

      if (status)
      {  mcxIOpos(xf, stderr)
      ;  mclvFree(&(tab->domain))
      ;  mcxFree(tab->labels)
      ;  mcxFree(tab)
      ;  mcxTingFree(&line)
      ;  tab = NULL
      ;  if (ON_FAIL == EXIT_ON_FAIL)
            mcxErr(me, "curtains")
         ,  mcxExit(1)
   ;  }

      mcxTingFree(&line)
   ;  return tab
;  }


void mclTabHashSet
(  mcxHash* hash
,  ulong u
)
   {  mcxHashWalk* walk = mcxHashWalkInit(hash)
   ;  mcxKV* kv
   ;  while ((kv = mcxHashWalkStep(walk, NULL)))
      kv->val = ULONG_TO_VOID u
;  }


   /* hierverder: mclx_collect_vectors does not allow vids to be set. */

mclx* mclTabDuplicated
(  mclTab* tab
,  mcxHash** hp
)
   {  dim size = tab->domain ? tab->domain->n_ivps : 0
   ;  mclx* dup = mclxAllocZero(mclvInit(NULL), mclvCopy(NULL, tab->domain))
   ;  mcxHash* h = mcxHashNew(2 * size, mcxTingDPhash, mcxTingCmp)
   ;  dim d
   ;  for (d=0;d<size;d++)
      {  mcxTing* tg =  mcxTingNew(tab->labels[d])
      ;  mcxKV* kv   =  mcxHashSearch(tg, h, MCX_DATUM_INSERT)
      ;  long vid    =  tab->domain->ivps[d].idx
      ;  if (kv->key == tg)
         kv->val = (void*) vid
#if 0
,fprintf(stderr, "insert vid %ld\n", (long) kv->val)
#endif
      ;  else
         {  long vid1 = (long) kv->val
         ;  mclv* v = mclxGetVector(dup, vid1, RETURN_ON_FAIL, NULL)
;if(1)fprintf(stderr, "retrieve vid %ld\n", vid1)
         ;  if (!v)
            {  mclv* newv = mclvInsertIdx(NULL, vid, 1.0)
            ;  newv->vid = vid1
            ;  mclxMergeColumn(dup, newv, fltMax)
            ;  mclvFree(&newv)
            ;  mclvFree(&newv)
         ;  }
            else
            mclvInsertIdx(v, vid, 1.0)
      ;  }
      }
      if (hp)
      hp[0] = h
   ;  else
      mcxHashFree(&h, mcxTingRelease, NULL)

;fprintf(stderr, "matrix has %d entries\n", (int) mclxNrofEntries(dup))
   ;  return dup
;  }


   /* TODO: use mcxTabDuplicated, and have separate routine to deduplicate
    * labels
   */
mcxHash* mclTabHash
(  mclTab* tab
)
   {  dim size = tab->domain ? tab->domain->n_ivps : 0
   ;  mcxHash* h = mcxHashNew(2 * size, mcxTingDPhash, mcxTingCmp)
   ;  dim d
   ;  for (d=0;d<size;d++)
      {  mcxTing* tg = mcxTingNew(tab->labels[d])
      ;  mcxKV* kv   = mcxHashSearch(tg, h, MCX_DATUM_INSERT)
      ;  unsigned long index

      ;  if (kv->key != tg)
         {  unsigned short s = 2
         ;  mcxErr("mclTabHash", "duplicate label <%s>", tg->str)
         ;  while (s)
            {  mcxTingPrint(tg, "%s_%ld", tab->labels[d], (long) s)
            ;  kv = mcxHashSearch(tg, h, MCX_DATUM_INSERT)
            ;  if (kv && kv->key == tg)
               break
            ;  s++
         ;  }
            if (!s)
            {  mcxErr("mclTabHash", "giving up on label <%s>", tab->labels[d])
            ;  mcxTingFree(&tg) 
            ;  continue
         ;  }
            else
            mcxErr
            (  "mclTabHash"
            ,  "deduplicated label %s at index %ld"
            ,  tg->str
            ,  (long) tab->domain->ivps[d].idx
            )
      ;  }
         index = tab->domain->ivps[d].idx
      ;  kv->val = ULONG_TO_VOID index
   ;  }
      return h
;  }


mclTab* mclTabFromMap
(  mcxHash* map
)
   {  mclTab* tab    =  mcxAlloc(sizeof(mclTab), EXIT_ON_FAIL)
   ;  dim n_keys     =  0
   ;  void** keys    =  mcxHashKeys(map, &n_keys, mcxTingCmp, 0)
                                          /* fixme: ssize_t iface */
   ;  dim d          =  0
   ;  const char* me =  "mclTabFromMap"
   ;  dim n_missing  =  0

   ;  if (!(tab->labels=mcxAlloc((n_keys+1) * sizeof(char*), RETURN_ON_FAIL)))
      return NULL

   ;  tab->domain    =  mclvCanonical(NULL, n_keys, 1.0)
   ;  tab->na        =  mcxTingNew("?")

   ;  for (d=0;d<n_keys;d++)
      tab->labels[d] = NULL

                              /* first create the domain */
   ;  for (d=0;d<n_keys;d++)
      {  mcxTing* lbl   =  keys[d]
      ;  mcxKV*  kv     =  mcxHashSearch(lbl, map, MCX_DATUM_FIND)
      ;  unsigned long  idx = kv ? VOID_TO_ULONG kv->val : 0

      ;  if (!kv)
         {  mcxErr("mclTabFromMap panic", "cannot retrieve <%s>!?", lbl->str)
         ;  return NULL
      ;  }
         tab->domain->ivps[d].idx = idx     /* dangersign */
   ;  }

      mclvSort(tab->domain, mclpIdxCmp)
   ;  if (mclvCheck(tab->domain, -1, -1, 0, RETURN_ON_FAIL))      /* fixme memleak */
      return NULL

   ;  for (d=0;d<n_keys;d++)
      {  mcxTing* lbl   =  keys[d]
      ;  mcxKV*  kv     =  mcxHashSearch(lbl, map, MCX_DATUM_FIND)
      ;  unsigned long  idx = kv ? VOID_TO_ULONG kv->val : 0
      ;  ofs offset = -1

      ;  if (!kv)
         {  mcxErr("mclTabFromMap panic", "cannot retrieve <%s>!?", lbl->str)
         ;  return NULL
      ;  }

         if (0 > (offset = mclvGetIvpOffset(tab->domain, idx, offset)))
         {  mcxErr("mclTabFromMap panic", "cannot find %lu in tab", (ulong) idx)
         ;  return NULL
      ;  }

         tab->labels[offset] =  mcxTingStr(lbl)
   ;  }

      tab->labels[n_keys] = NULL

   ;  for (d=0;d<n_keys;d++)
      if (!tab->labels[d])
      {  mcxTing* tmp = mcxTingPrint(NULL, "%s%lu", tab->na->str, (ulong) ++n_missing)
      ;  mcxErr(me, "mapping missing %lu index to %s", (ulong) d, tmp->str)
      ;  tab->labels[d] = mcxTinguish(tmp)
   ;  }

      mcxFree(keys)
   ;  return tab
;  }



mcxstatus mclTabWriteDomain
(  mclv*          select
,  mcxIO*         xfout
,  mcxOnFail      ON_FAIL
)
   {  dim d
   ;  if (mcxIOtestOpen(xfout, ON_FAIL))
      return STATUS_FAIL

   ;  for (d=0;d<select->n_ivps;d++)
      {  long idx = select->ivps[d].idx
      ;  fprintf(xfout->fp, "%ld\t%ld\n", idx, idx)
   ;  }
      mcxLog
      (  MCX_LOG_IO  
      ,  "mclIO"
      ,  "wrote %ld tab entries to stream <%s>"
      ,  (long) select->n_ivps
      ,  xfout->fn->str
      )
   ;  return STATUS_OK
;  }



mcxstatus mclTabWrite
(  mclTab*        tab
,  mcxIO*         xfout
,  const mclv*    select   /* if NULL, use all */
,  mcxOnFail      ON_FAIL
)
   {  long label_o = -1
   ;  long miss = 1
   ;  dim i

   ;  if (!tab)
      {  mcxErr("mclTabWrite", "no tab! target file: <%s>", xfout->fn->str)
      ;  return STATUS_FAIL
   ;  }

      if (!select)
      select = tab->domain

   ;  if (mcxIOtestOpen(xfout, ON_FAIL))
      return STATUS_FAIL

   ;  for (i=0;i<select->n_ivps;i++)
      {  long idx = select->ivps[i].idx
      ;  const char* label = mclTabGet(tab, idx, &label_o)

                              /* fixme: below does 1) seem unreachable
                               * and 2) is a horrible idea
                              */
      ;  if (label == tab->na->str)
            mcxErr("mclTabWrite", "warning index %ld not found", idx)
         ,  fprintf(xfout->fp, "%ld\t%s%ld\n", idx, label, miss)
      ;  else
         fprintf(xfout->fp, "%ld\t%s\n", idx, label)
   ;  }
      mcxLog
      (  MCX_LOG_IO
      ,  "mclIO"
      ,  "wrote %ld tab entries to stream <%s>"
      ,  (long) select->n_ivps
      ,  xfout->fn->str
      )
   ;  return STATUS_OK
;  }



char* mclTabGet
(  const mclTab*  tab
,  long     id
,  long*    ofs
)
   {  long old_ofs = ofs ? *ofs : -1
   ;  long new_ofs = mclvGetIvpOffset(tab->domain, id, old_ofs)
   ;  if (ofs)
      *ofs = new_ofs
   ;  return new_ofs >= 0 ? tab->labels[new_ofs] : tab->na->str
;  }


/* sort labels based on map.
 *
 * 0 -> 8 -> 5 -> 16 -> 0
 *    tag 0   take 8
 *    store 0 take 5 
 *    store 8 take 16
 *    store 5 take 0 [tag hit]
 *    store 16
*/

mclTab* mclTabMap
(  const mclTab*  tab
,  mclx*          map
)
   {  char** new_labels
   ;  mclv*  new_domain
   ;  mclTab* new_tab
   ;  const char* me = "mclTabMap"
   ;  dim i

   ;  if (!mcldEquate(tab->domain, map->dom_cols, MCLD_EQT_SUB))
      {  mcxErr(me, "mapping col domain not a superdomain")
      ;  return NULL
   ;  }

      else if (!mclxMapTest(map))
      {  mcxErr(me, "mapping matrix does not map")
      ;  return NULL
   ;  }

      if
      (! (  new_labels
         =  mcxAlloc(tab->domain->n_ivps * sizeof(char*), EXIT_ON_FAIL)
      )  )
      return NULL

   ;  new_domain = mclgUnionv(map, tab->domain, NULL, SCRATCH_READY, NULL)

   ;  if (new_domain->n_ivps != tab->domain->n_ivps)
      {  mclvFree(&new_domain)
      ;  return NULL
   ;  }

      for (i=0;i<new_domain->n_ivps;i++)
      new_labels[i] = NULL

   ;  for (i=0;i<tab->domain->n_ivps;i++)
      {  long map_ofs = mclvGetIvpOffset(map->dom_cols, tab->domain->ivps[i].idx, -1)
      ;  long map_idx = map->cols[map_ofs].ivps[0].idx
      ;  long dom_ofs = mclvGetIvpOffset(new_domain, map_idx, -1)
      ;  if (dom_ofs < 0)
         break
      ;  new_labels[dom_ofs] = mcxStrDup(tab->labels[i])
   ;  }

      if (tab->domain->n_ivps != i)
      {  mclvFree(&new_domain)
      ;  mcxFree(new_labels)
      ;  return NULL
   ;  }

      if (!(new_tab = mcxAlloc(sizeof(mclTab), RETURN_ON_FAIL)))
      return NULL

   ;  new_tab->labels = new_labels
   ;  new_tab->domain = new_domain
   ;  new_tab->na     = mcxTingNew("?")
   ;  return new_tab
;  }


static mclTab* mclTabAlloc
(  dim n
)
   {  mclTab* tab
   ;  if (!(tab =  mcxAlloc(sizeof(mclTab), RETURN_ON_FAIL)))
      return NULL
   ;  if (!(tab->labels = mcxAlloc((n+1) * sizeof(char*), RETURN_ON_FAIL)))
      return NULL
   ;  tab->domain= NULL
   ;  tab->na = mcxTingNew("?")
   ;  return tab
;  }


mclTab* mclTabSelect
(  const mclTab*  tab
,  const mclv*    select
)
   {  dim k
   ;  ofs ivpofs = -1
   ;  mclTab* new
   ;  if (!mcldEquate(select, tab->domain, MCLD_EQT_SUB))
      return NULL

   ;  new = mclTabAlloc(select->n_ivps)
   ;  new->domain = mclvClone(select)

   ;  for (k=0;k<select->n_ivps;k++)
      {  if
         (  (  ivpofs
            =  mclvGetIvpOffset(tab->domain, select->ivps[k].idx, ivpofs)
            )
            >= 0
         )
         new->labels[k] = mcxStrDup(tab->labels[ivpofs])
      ;  else
         new->labels[k] = "?"
   ;  }
      if (k != select->n_ivps)
      mcxErr
      (  "mclTabSelect"
      ,  "panic: %d/%d inconsistency"
      ,  (int) select->n_ivps, (int) k
      )
   ;  new->labels[k] = NULL
   ;  return new
;  }


