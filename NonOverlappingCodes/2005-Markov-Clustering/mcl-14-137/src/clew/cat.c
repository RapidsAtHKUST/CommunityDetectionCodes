/*   (C) Copyright 2007, 2008, 2009, 2010 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* TODO
 * add ensureRoot routine.
 *
 * read_skeleton fails in unlimited cat mode. Only trailing whitespace
 * is allowed, but read_skeleton leaves the body.
 * Conceivably read_skeleton should flush the body if more than
 * one matrix is being read.
*/


#include "cat.h"
#include "clm.h"

#include "util/err.h"
#include "util/compile.h"
#include "util/alloc.h"

#include "impala/io.h"
#include "impala/compose.h"
#include "impala/tab.h"
#include "impala/edge.h"

static const char* us = "mclxCat";

mcxstatus mclxCatPush
(  mclxCat*    stack
,  mclx*       mx
,  mcxstatus   (*cb1) (mclx* mx, void* cb_data)
,  void*       cb1_data
,  mcxstatus   (*cb2) (mclx* left, mclx* right, void* cb_data)
,  void*       cb2_data
,  const char* fname
,  dim         fidx
)
   {  if
      (  stack->n_level
      && cb2
      && cb2(stack->level[stack->n_level-1].mx, mx, cb2_data)
      )
      {  mcxErr
         (  "mclxCatPush"
         ,  "chain error at level %d"
         ,  (int) stack->n_level
         )
      ;  return STATUS_FAIL
   ;  }

      if (cb1 && cb1(mx, cb1_data))
      {  mcxErr
         (  "mclxCatPush"
         ,  "matrix error at level %d"
         ,  (int) stack->n_level
         )
      ;  return STATUS_FAIL
   ;  }

      if
      (  !stack->level
      || (stack->n_level >= stack->n_alloc)
      )
      {  dim n_alloc = 5 + 1.5 * stack->n_alloc
      ;  mclxAnnot* level2
         =  mcxRealloc
            (  stack->level
            ,  sizeof stack->level[0] * n_alloc
            ,  RETURN_ON_FAIL
            )
      ;  if (!level2)
         return STATUS_FAIL
      ;  stack->level = level2
      ;  stack->n_alloc = n_alloc
   ;  }

      stack->level[stack->n_level].mx     =  mx
   ;  stack->level[stack->n_level].mxtp   =  NULL
   ;  stack->level[stack->n_level].usr    =  NULL
   ;  stack->level[stack->n_level].fname  =     fidx
                                             ?  mcxTingPrint(NULL, "%s(%d)", fname, (int) fidx)
                                             :  mcxTingNew(fname)
   ;  stack->n_level++
   ;  return STATUS_OK
;  }


mcxstatus mclxCatTransposeAll
(  mclxCat* cat
)
   {  dim l
   ;  for (l=0;l<cat->n_level;l++)
      {  if (!cat->level[l].mxtp)
         cat->level[l].mxtp = mclxTranspose(cat->level[l].mx)
      ;  if (!cat->level[l].mxtp)
         break
   ;  }
      return l == cat->n_level ? STATUS_OK : STATUS_FAIL
;  }


void mclxCatInit
(  mclxCat*  stack
)
   {  stack->n_level = 0
   ;  stack->n_alloc = 0
   ;  stack->type =  'n'
   ;  stack->level = NULL
;  }
 

mcxstatus mclxCatConify
(  mclxCat* st
)
   {  dim i
#if 0
;mcxIO* xf = mcxIOnew("-", "w")
;mcxIOopen(xf, EXIT_ON_FAIL)
;fprintf(xf->fp, "inin\n")
#endif
   ;  if (st->n_level <= 1)
      return STATUS_OK
   ;  for (i=st->n_level-1;i>0;i--)
      {  mclx* cltp = mclxTranspose(st->level[i-1].mx)
      ;  mclx* clcontracted = mclxCompose(cltp, st->level[i].mx, 0, 1)
      ;  mclxFree(&cltp)
 /*;mclxWrite(st->level[i].mx, xf, 0, EXIT_ON_FAIL)*/
      ;  mclxFree(&st->level[i].mx)

      ;  mclxMakeCharacteristic(clcontracted)
      ;  st->level[i].mx = clcontracted
/*;mclxWrite(clcontracted, xf, 0, EXIT_ON_FAIL)*/
   ;  }
      return STATUS_OK
;  }


mcxstatus mclxCatUnconify
(  mclxCat* st
)
   {  dim i
   ;  mcxstatus status = STATUS_OK
   ;  for (i=0;i<st->n_level-1;i++)
      {  mclx* clprojected = mclxCompose(st->level[i].mx, st->level[i+1].mx, 0, 1)
      ;  if
         (  mclxCBdomTree
            (  st->level[i].mx
            ,  st->level[i+1].mx
            ,  NULL
            )
         )
         {  mcxErr
            (  "mclxCatUnconify warning"
            ,  "domain inconsistency at level %d-%d"
            ,  (int) i
            ,  (int) (i+1)
            )
         ;  status = STATUS_FAIL
      ;  }

         mclxFree(&st->level[i+1].mx)
      ;  st->level[i+1].mx = clprojected
   ;  }
      return status
;  }


mcxstatus mclxCBdomTree
(  mclx* left
,  mclx* right
,  void* cb_data_unused cpl__unused
)
   {  return
         MCLD_EQUAL(left->dom_cols, right->dom_rows)
      ?  STATUS_OK
      :  STATUS_FAIL
;  }


mcxstatus mclxCBdomStack
(  mclx* left
,  mclx* right
,  void* cb_data_unused cpl__unused
)
   {  return
         mcldEquate(left->dom_rows, right->dom_rows, MCLD_EQT_EQUAL)
      ?  STATUS_OK
      :  STATUS_FAIL
;  }


mcxstatus mclxCatUnaryCheck
(  mclx* mx
,  void* cb_data
)
   {  dim o, m, e
   ;  mcxbits bits = *((mcxbits*) cb_data)

   ;  if
      (  bits & MCLX_REQUIRE_CANONICALR
      && !mclxRowCanonical(mx)
      )
      return STATUS_FAIL

   ;  if
      (  bits & MCLX_REQUIRE_CANONICALC
      && !mclxColCanonical(mx)
      )
      return STATUS_FAIL

   ;  if
      (  bits & MCLX_REQUIRE_GRAPH
      && !mclxIsGraph(mx)
      )
      return STATUS_FAIL

   ;  if
      (  bits & MCLX_REQUIRE_PARTITION
      && clmEnstrict(mx, &o, &m, &e, ENSTRICT_REPORT_ONLY)
      )
      return STATUS_FAIL

   ;  if (bits & MCLX_PRODUCE_PARTITION)
      {  dim x = clmEnstrict(mx, &o, &m, &e, ENSTRICT_PARTITION)
      ;  if (x)
         mcxErr("mclxCatRead", "not a partition (fixed, o=%d, m=%d, e=%d)", (int) o, (int) m, (int) e)
   ;  }

      return STATUS_OK
;  }


void mclxCatReverse
(  mclxCat*  stack
)
   {  dim i
   ;  for (i=0;i<stack->n_level/2;i++)
      {  mclxAnnot annot = stack->level[i]
      ;  stack->level[i] = stack->level[stack->n_level-i-1]
      ;  stack->level[stack->n_level-i-1] = annot
   ;  }
   }


mcxstatus mclxCatWrite
(  mcxIO*      xf
,  mclxCat*  stack
,  int         valdigits
,  mcxOnFail   ON_FAIL
)
   {  dim i
   ;  if (mcxIOtestOpen(xf, ON_FAIL))
      return STATUS_FAIL
   ;  for (i=0;i<stack->n_level;i++)
      if (mclxWrite(stack->level[i].mx, xf, valdigits, ON_FAIL))
      return STATUS_FAIL
   ;  return STATUS_OK
;  }


/* fixme: need mclxCatIsCone
 *
*/

struct newicky
{  mcxTing* node
;
}  ;


void* newicky_init
(  void* nkp
)
   {  struct newicky* nk = nkp
   ;  nk->node = NULL
   ;  return nkp
;  }


   /* For each node, count the branch length, i.e.
    * number of trivial nodes right beneath it.
    *
    * The result is stored in cat->level[lev].usr
    *
    * hierverder: change to compute the number of trivial nodes right above it.
   */
static void compute_branch_length
(  mclxCat* cat
,  dim      lev
,  dim      v
,  double   value
)
   {  mclv* vec = cat->level[lev].mx->cols+v
   ;  mclv* usrvec = cat->level[lev].usr
   ;  double new_value = vec->n_ivps == 1 ? value + 1.0 : 1.5
   ;  dim i

   ;  usrvec->ivps[v].val = value

   ;  if (lev > 0)
      for (i=0;i<vec->n_ivps;i++)
      compute_branch_length(cat, lev-1, vec->ivps[i].idx, new_value)
;  }



   /* For each node, count the number of trivial nodes
    * in the path to root.
    * A trivial node is one in which the cluster does not split
    * in the next level.
   */
static void compute_trivial_count
(  mclxCat* cat
,  dim      lev
,  dim      v
,  double   value
)
   {  mclv* vec = cat->level[lev].mx->cols+v
   ;  double delta = vec->n_ivps == 1 ? 1.0 : 0.0
   ;  dim i

   ;  vec->val = value

   ;  if (lev > 0)
      for (i=0;i<vec->n_ivps;i++)
      compute_trivial_count(cat, lev-1, vec->ivps[i].idx, value+delta)
;  }


void compute_branch_factors
(  mclxCat* cat
,  mcxbits  bits
)
   {  dim v
   ;  mclx* mx
   ;  if (!cat->n_level)
      return

   ;  if
      (  bits & MCLX_NEWICK_NOINDENT
      && bits & MCLX_NEWICK_NONUM
      )
      return

   ;  mx = cat->level[cat->n_level-1].mx
   ;  for (v=0;v<N_COLS(mx);v++)
      {  if (!(bits & MCLX_NEWICK_NOINDENT))
         compute_trivial_count(cat, cat->n_level-1, v, 0.5)
      ;  if (!(bits & MCLX_NEWICK_NONUM))
         compute_branch_length(cat, cat->n_level-1, v, 1.5)
   ;  }
;  }



#if 0
{  "label": "root",
   "children":
   [
      {  "label": "internal_1",
         "children":
         [
            {  "label": "internal_11",
               "children":
               [
                  {  "label": "internal_111",
                     "children":
                     [ { "label": "nul" }
                     , { "label": "vijf" }
                     , { "label": "negen" }
                     ]
                  }
               ,
                  {  "label": "internal_112",
                     "children":
                     [  { "label": "zes" }
                     ]
                  }
               ]
            }
         ,
            {  "label": "internal_12",
               "children":
               [
                  {  "label": "internal_121",
                     "children":
                     [ { "label": "een" } ]
                  }
               ,
                  {  "label": "internal_122",
                     "children":
                     [ { "label": "vier" } ]
                  }
               ]
            }
         ]
      }

#endif


void open_node
(  mcxTing* pivot
,  mcxbool  json
)
   {  if (json)
      mcxTingNAppend(pivot, "(", 1)
   ;  else
      mcxTingNAppend(pivot, "(", 1)
;  }



/* TODO
 * smarter string building: assemble the bits.
 * and then print them rather than repeated assembling/printing.
 *
 * sort according to node size.
 *
 * make sure that cat does in fact encode a tree.
*/

mcxTing* mclxCatNewick
(  mclxCat*  cat
,  mclTab*   tab
,  mcxbits   bits
)
   {  struct newicky* nklast, *nknext
   ;  mcxTing* tree = NULL
   ;  mcxTing* spacey = mcxTingKAppend(NULL, " ", cat->n_level)
   ;  const char* prefix = spacey->str
   ;  dim l, j, lm = cat->n_level            /* level-max */
   ;  mcxbool json = bits & MCLX_JSON

   ;  if (!lm)
      return mcxTingNew("")
   ;  nklast
      =  mcxNAlloc
         (  N_COLS(cat->level[0].mx)
         ,  sizeof(struct newicky)
         ,  newicky_init
         ,  RETURN_ON_FAIL
         )

   ;  for (l=0;l<lm;l++)
      {  if (cat->level[l].usr)
         mcxDie(1, us, "user object defined")
      ;  else
         cat->level[l].usr = mclvCopy(NULL, cat->level[l].mx->dom_cols)
;fprintf(stderr, "level %d has %d clusters\n", (int) l, (int) N_COLS(cat->level[l].mx))
   ;  }
   
      compute_branch_factors(cat, bits)

   ;  for (j=0;j<N_COLS(cat->level[0].mx);j++)
      {  mclv* vec = cat->level[0].mx->cols+j
      ;  mcxTing* pivot
      ;  dim k

      ;  nklast[j].node = mcxTingNew("")

      ;  if (!vec->n_ivps)
         continue

      ;  pivot = nklast[j].node

      ;  if (!(bits & MCLX_NEWICK_NOINDENT))
         mcxTingPrint(pivot, "%s", prefix+1+((int)vec->val))      /* apparently denotes depth .. */

      ;  if (vec->n_ivps > 1 || !(bits & MCLX_NEWICK_NOPTHS))
         open_node(pivot, json)

      ;  if (tab)
         mcxTingPrintAfter(pivot, "%s", tab->labels[vec->ivps[0].idx])
      ;  else
         mcxTingPrintAfter(pivot, "%ld", (long) vec->ivps[0].idx)

      ;  for (k=1;k<vec->n_ivps;k++)
         {  if (tab)
            mcxTingPrintAfter(pivot, ",%s", tab->labels[vec->ivps[k].idx])
         ;  else
            mcxTingPrintAfter(pivot, ",%ld", (long) vec->ivps[k].idx)
      ;  }

         if (vec->n_ivps > 1 || !(bits & MCLX_NEWICK_NOPTHS))
         mcxTingNAppend(pivot, ")", 1)

      ;  if (!(bits & MCLX_NEWICK_NONUM))
         mcxTingPrintAfter
         (  pivot
         ,  ":%d"
         ,  (int) ((mclv*)cat->level[0].usr)->ivps[j].val
         )
/* fprintf(stderr, "node %d %s\n", (int) j, pivot->str) */
   ;  }

      for (l=1;l<lm;l++)
      {  prefix = spacey->str+l
      ;  nknext
         =  mcxNAlloc
            (  N_COLS(cat->level[l].mx)
            ,  sizeof(struct newicky)
            ,  newicky_init
            ,  ENQUIRE_ON_FAIL
            )
      ;  for (j=0;j<N_COLS(cat->level[l].mx);j++)
         {  mclv* vec = cat->level[l].mx->cols+j
         ;  mcxTing* pivot
         ;  dim k
         ;  ofs idx

         ;  if (!vec->n_ivps)
            continue
         ;  if (vec->n_ivps == 1)
            {  ofs idx = vec->ivps[0].idx
            ;  if (!nklast[idx].node)
               mcxDie(1, "newick panic", "corruption 1")
            ;  nknext[j].node = nklast[idx].node
            ;  nklast[idx].node = NULL
            ;  continue
         ;  }

            idx = vec->ivps[0].idx

         ;  pivot = nknext[j].node = mcxTingEmpty(NULL, 20)

         ;  if (!(bits & MCLX_NEWICK_NOINDENT))
            mcxTingPrint(pivot, "%s", prefix+1+((int) vec->val))

         ;  mcxTingNAppend(pivot, "(", 1)

         ;  if (!(bits & MCLX_NEWICK_NONL))
            mcxTingNAppend(pivot, "\n", 1)

         ;  mcxTingPrintAfter
            (  pivot
            ,  "%s"
            ,  nklast[idx].node->str
            )
         ;  mcxTingFree(&nklast[idx].node)

         ;  for (k=1;k<vec->n_ivps;k++)
            {  ofs idx = vec->ivps[k].idx
;  if (!nklast[idx].node)
   mcxDie(1, "newick panic", "corruption 2 level %d vec %d idx %d", (int) l, (int) j, (int) idx)

            ;  mcxTingNAppend(pivot, ",", 1)

            ;  if (!(bits & MCLX_NEWICK_NONL))
               mcxTingNAppend(pivot, "\n", 1)

            ;  mcxTingPrintAfter(pivot, "%s", nklast[idx].node->str)
            ;  mcxTingFree(&nklast[idx].node)
         ;  }

            if (!(bits & MCLX_NEWICK_NONL))
            mcxTingNAppend(pivot, "\n", 1)

         ;  if (!(bits & MCLX_NEWICK_NOINDENT))
            mcxTingPrintAfter
            (pivot, "%s", prefix+1+((int) vec->val))

         ;  mcxTingNAppend(pivot, ")", 1)

         ;  if (!(bits & MCLX_NEWICK_NONUM))
            mcxTingPrintAfter
            (pivot, ":%d", (int) ((mclv*)cat->level[l].usr)->ivps[j].val)
      ;  }
         mcxFree(nklast)
      ;  nklast = nknext
   ;  }

      tree = nklast[0].node

   ;  for (l=0;l<lm;l++)
      {  mclv* vec = cat->level[l].usr
      ;  mclvFree(&vec)
      ;  cat->level[l].usr = NULL
   ;  }

      mcxFree(nklast)
   ;  return tree
;  }


/* Currently this fails if there is no matrix to be read.
 * allowing zero matrices *could* be an option.
 * Even more so in conjunction with MCLX_ENSURE_ROOT
*/

mcxstatus mclxCatRead
(  mcxIO*      xf
,  mclxCat*    st
,  dim         n_max
,  mclv*       base_dom_cols
,  mclv*       base_dom_rows
,  mcxbits     bits
)
   {  dim n_read = 0
   ;  mcxstatus status = STATUS_OK
   ;  mcxTing* line = mcxTingEmpty(NULL, 20)
   ;  const char* me = "mclxCatRead"
   ;  dim n_uncanon = 0          
   ;  dim n_xf = 0

   ;  mclx* mx = NULL
   ;  mcxbool stack_or_cone
      =     MCLX_PRODUCE_DOMTREE | MCLX_PRODUCE_DOMSTACK
         |  MCLX_REQUIRE_DOMTREE | MCLX_REQUIRE_DOMSTACK
   ;  const char* err = NULL

   ;  while (status == STATUS_OK)
      {  dim o, m, e
      ;  status = STATUS_FAIL

      ;  if (bits & MCLX_READ_SKELETON)
         {                          /* fixme docme: hard mclx_require_graph. */
            if (!(mx = mclxReadSkeleton(xf, bits & MCLX_REQUIRE_GRAPH, TRUE)))
            break
      ;  }
         else
         {  if (bits & MCLX_REQUIRE_GRAPH)
            {  if (!(mx = mclxReadx(xf, RETURN_ON_FAIL, MCLX_REQUIRE_GRAPH)))
               break
         ;  }
            else if (!(mx = mclxRead(xf, RETURN_ON_FAIL)))
            break
      ;  }
                           /* we required this for clusters; docme why!
                            * enstrict / map related ?
                           */
         if (stack_or_cone && !MCLV_IS_CANONICAL(mx->dom_cols) && ++n_uncanon == 2)
         {  mcxErr(me, "matrix indices not in canonical format")
         ;  mcxErr(me, "code path not tested!")
         ;  mcxErr(me, "you might experience bugs!")
         ;  mcxErr(me, "three exclamations for cargo cult programming!")
      ;  }

         if ((bits & MCLX_REQUIRE_CANONICALC) && !MCLV_IS_CANONICAL(mx->dom_cols))
         {  err = "column domain not canonical"
         ;  break
      ;  }

         if ((bits & MCLX_REQUIRE_CANONICALR) && !MCLV_IS_CANONICAL(mx->dom_rows))
         {  err = "row domain not canonical"
         ;  break
      ;  }

         if
         (  (bits & MCLX_REQUIRE_PARTITION)
         && clmEnstrict(mx, &o, &m, &e, ENSTRICT_REPORT_ONLY)
         )
         {  err = "not a partition"
         ;  break
      ;  }
                           /* check base if given
                           */
         if
         (  st->n_level == 0
         && (  (  base_dom_cols
               && !MCLD_EQUAL(base_dom_cols, mx->dom_cols)
               )
            || (  base_dom_rows
               && !MCLD_EQUAL(base_dom_rows, mx->dom_rows)
               )
            )
         )
         {  err = "base domain mismatch"
         ;  break
      ;  }

                           /* check stack/cone status
                           */
         else if (stack_or_cone && st->n_level >= 1)
         {  mclx* mxprev = st->level[st->n_level-1].mx
         ;  mcxbool see_stack = MCLD_EQUAL(mxprev->dom_rows, mx->dom_rows)
         ;  mcxbool see_cone  = MCLD_EQUAL(mxprev->dom_cols, mx->dom_rows)
         ;  if (!see_stack && !see_cone)
            {  err = "fish nor fowl"
            ;  break
         ;  }
            if (st->type == 'n')
            {  if (see_stack && see_cone)
               NOTHING              /* all clusters could be singletons */
            ;  else if (see_stack)
               st->type = 's'
            ;  else if (see_cone)
               st->type = 'c'
         ;  }
            else if (st->type == 'c')
            {  if (see_stack || (bits & MCLX_REQUIRE_DOMSTACK))
               {  err = "cone/stack violation"
               ;  break
            ;  }
            }
            else if (st->type == 's')
            {  if (see_cone || (bits & MCLX_REQUIRE_DOMTREE))
               {  err = "stack/cone violation"
               ;  break
            ;  }
            }

            if ((bits & MCLX_REQUIRE_NESTED) && st->type == 's')
            {  mclx* cing = clmContingency(mxprev, mx)
            ;  mcxbool ok = TRUE
            ;  dim j
            ;  for (j=0;j<N_COLS(cing);j++)
               if (cing->cols[j].n_ivps != 1)
               {  ok = FALSE
               ;  break
            ;  }
               mclxFree(&cing)
            ;  if (!ok)
               break
         ;  }
         }

      ;  if (mclxCatPush(st, mx, mclxCatUnaryCheck, &bits, NULL, NULL, xf->fn->str, n_xf))
         {  err = "no push!"
         ;  break
      ;  }

         n_xf++
                     /* read trailing ')' so that EOF check later works
                      * fixme/docme: has that ')' been ungetc"ed?
                      * Even worse, should that not be mcxIOskipski of some kind?
                     */
      ;  if (mclxIOformat(xf) == 'a')
         mcxIOreadLine(xf, line, MCX_READLINE_CHOMP)
      ;  mcxIOreset(xf)

      ;  status = STATUS_OK
      ;  if
         (  (n_max && ++n_read >= n_max)
         || EOF == mcxIOskipSpace(xf)
         )        /* skipSpace is funny for binary format, but it works */
         break
   ;  }

      mcxTingFree(&line)

   ;  if
      (  !status
      && (bits & MCLX_ENSURE_ROOT)
      && N_COLS(mx) != 1
      )
      {  mclx* root
         =  mclxCartesian
            (  mclvCanonical(NULL, 1, 1.0)
            ,  mclvCopy(NULL, st->type == 'c' ? mx->dom_cols : mx->dom_rows)
            ,  1.0
            )
      ;  if (mclxCatPush(st, root, mclxCatUnaryCheck, &bits, NULL, NULL, xf->fn->str, n_xf))
            err = "no push!"
         ,  status = STATUS_FAIL
      ;  n_xf++
      ;  mx = root
   ;  }

      if (status && st->n_level && st->level[st->n_level-1].mx != mx)
      mclxFree(&mx)

   ;  if (err)
      mcxErr(me, "%s at level %lu in file %s", err, (ulong) st->n_level, xf->fn->str)

   ;  if (!status && stack_or_cone)
      {  if (st->type == 's' && (bits & MCLX_PRODUCE_DOMTREE))
         return mclxCatConify(st)
      ;  else if (st->type == 'c' && (bits & MCLX_PRODUCE_DOMSTACK))
         return mclxCatUnconify(st)
   ;  }

      return status
;  }


mclx*  clmContingency
(  const mclx*  cla
,  const mclx*  clb
)
   {  mclx  *clbt =  mclxTranspose(clb)
   ;  mclx  *ct   =  mclxCompose(clbt, cla, 0, 1)
   ;  mclxFree(&clbt)
   ;  return ct
;  }



dim clmStats
(  mclx* cls
,  dim   clmstat[N_CLM_STATS]
)
   {  mclv* acc = mclvInit(NULL)
   ;  double onep5 = 1.5
   ;  mclv* clssizes = mclxColSizes(cls, MCL_VECTOR_SPARSE)
   ;  dim d

   ;  mclxMakeCharacteristic(cls)

   ;  for (d=0;d<N_COLS(cls);d++)
      mclvAdd(cls->cols+d, acc, acc)

   ;  clmstat[CLM_STAT_NODES_MISSING]  =  N_ROWS(cls) - acc->n_ivps
   ;  clmstat[CLM_STAT_NODES_OVERLAP]  =  mclvCountGiven(acc, mclpGivenValGQ, &onep5)
   ;  clmstat[CLM_STAT_CLUSTERS]       =  clssizes->n_ivps
   ;  clmstat[CLM_STAT_NODES]          =  N_ROWS(cls)
   ;  clmstat[CLM_STAT_CLUSTERS_EMPTY] =  N_COLS(cls) - clssizes->n_ivps
   ;  clmstat[CLM_STAT_SUM_OVERLAP]    =  mclxNrofEntries(cls)
                                             - N_ROWS(cls)
                                             - clmstat[CLM_STAT_CLUSTERS_EMPTY]

   ;  mclvFree(&clssizes)
   ;  mclvFree(&acc)
   ;  return
         (  clmstat[CLM_STAT_NODES_OVERLAP]
         +  clmstat[CLM_STAT_NODES_MISSING]
         +  clmstat[CLM_STAT_CLUSTERS_EMPTY]
         )
;  }



static void clm_cut_overlap
(  mclx* cl
)
   {  mclx* cltp = mclxTranspose(cl), *cl2
   ;  dim d
   ;  for (d=0;d<N_COLS(cltp);d++)
      mclvResize(cltp->cols+d, 1)
   ;  cl2 = mclxTranspose(cltp)
   ;  mclxFree(&cltp)
   ;  mclxTransplant(cl, &cl2)
;  }


   /* Now for each non-empty non-self intersection make a star graph,
      connecting one node with all the others. The connected
      components on the resulting graph will give us the clusters with
      all overlap split off and merged.

      For the self-intersection (of a cluster with itself)
      we wish to exclude all the nodes that are in overlap (those
      that are in a non-empty non-self intersection).
    */
static void clm_split_overlap
(  mclx* cl
)
   {  mclx* ctgy  =  clmContingency(cl, cl)  
   ;  mclx* clustergraphcomponents = NULL
   ;  mclx* clustergraph
               =  mclxAllocZero
                  (  mclvCopy(NULL, cl->dom_rows)
                  ,  mclvCopy(NULL, cl->dom_rows)
                  )
   ;  mclv* clus_overlap = mclvInit(NULL)
   ;  dim d

   ;  mclgUnionvReset(cl)

                                 /* loop over sets of projections */
   ;  for (d=0; d<N_COLS(ctgy);d++)
      {  mclv* ctgyvec = ctgy->cols+d     /* all projections for cluster d */
      ;  mclv* clusvec = cl->cols+d       /* cluster d itself */
      ;  dim e

                                 /* do not consider self-self intersection */
      ;  mclvRemoveIdx(ctgyvec, ctgyvec->vid)
      ;  if (clus_overlap->n_ivps)
         mclgUnionvReset(cl)
                                 /* clus_overlap contains all nodes in overlap */
      ;  mclgUnionv(cl, ctgyvec, NULL, SCRATCH_READY, clus_overlap)
#if 0
;fprintf(stderr, "<---- clus_overlap now\n")
;mclvaDump(clus_overlap, stdout, -1, " ", 0)
#endif

                                 /* clus_unique contains nodes unique to the
                                  * cluster; create a star graph on those
                                 */
      ;  {  mclv* clus_unique = mcldMinus(clusvec, clus_overlap, NULL)
         ;  if (clus_unique->n_ivps)
            {  mclv* nodevec
               =  mclxGetVector
                  (clustergraph, clus_unique->ivps[0].idx, RETURN_ON_FAIL, NULL)
            ;  if (nodevec)
               mclvAdd(nodevec, clus_unique, nodevec)
         ;  }
            mclvFree(&clus_unique)
      ;  }

                                 /* for the overlapping bits (each bit
                                  * defined by two clusters, one of which
                                  * is clusvec), create star graphs as well.

                                  * we removed source cluster d from ctgyvec earlier
                                 */
         for (e=0;e<ctgyvec->n_ivps;e++)
         {  long cid = ctgyvec->ivps[e].idx
         ;  mclv* clusvec2 = mclxGetVector(cl, cid, RETURN_ON_FAIL, NULL)
         ;  if (clusvec2)
            {  mclv* meet = mcldMeet(clusvec, clusvec2, NULL)
            ;  if (meet->n_ivps)
               {  mclv* nodevec
                  =  mclxGetVector
                     (clustergraph, meet->ivps[0].idx, RETURN_ON_FAIL, NULL)
                                 /* this makes meet->ivps[0] the central point
                                  * in the star graph. all outgoing arcs
                                  * are added. Reverse arcs will be added later
                                  * by symmetrification.
                                 */
               ;  if (nodevec)
                  mclvAdd(nodevec, meet, nodevec)
            ;  }
               mclvFree(&meet)
         ;  }
         }
      }
                                 /* Need to make it symmetric in order for
                                  * clmUGraphComponents to work.
                                 */
      mclxAddTranspose(clustergraph, 0.5)
   ;  clustergraphcomponents = clmUGraphComponents(clustergraph, NULL)
   ;  mclvFree(&clus_overlap)
   ;  mclxTransplant(cl, &clustergraphcomponents)
                                 /* now no need to use mclgUnionvReset */
   ;  mclxFree(&ctgy)
   ;  mclxFree(&clustergraph)
;  }



/* optional features: argument slot for vector of missing nodes.
   warning sign: may change number of clusters.
 */
dim clmEnstrict
(  mclx*    cl
,  dim     *overlap
,  dim     *missing
,  dim     *empty
,  mcxbits  bits
)
   {  dim n_overlap = 0, n_empty = 0, n_missing = 0, n_found = 0
   ;  double one = 1.0

   ;  mclxUnary(cl, fltxConst, &one)

                           /* compute how many are empty. do this at
                            * the start, as in some implementations
                            * of overlap removal new empty clusters might
                            * be generated. This has happened, historically,
                            * so we keep this order.
                           */
   ;  {  mclv* szs = mclxColSizes(cl, MCL_VECTOR_SPARSE)
      ;  n_empty = N_COLS(cl) - szs->n_ivps
      ;  n_found = (dim) (mclvSum(szs) + 0.5)
      ;  if (empty)
         *empty = n_empty
      ;  mclvFree(&szs)    /* empty clusters are removed further below */
   ;  }

                           /* simply compute the join of all clusters
                            * to find out whether nodes are missing.
                            * also compute overlap.
                           */
      {  mclv* nodes_found
      ;  mclgUnionvReset(cl)
      ;  nodes_found = mclgUnionv(cl, cl->dom_cols, NULL, SCRATCH_READY, NULL)
      ;  n_missing = 0
      ;  if (nodes_found->n_ivps < N_ROWS(cl) && !(bits & ENSTRICT_REPORT_ONLY))
         {  mclv* truants = mcldMinus(cl->dom_rows, nodes_found, NULL)
         ;  n_missing = truants->n_ivps
         ;  mclxAppendVectors(cl, truants, NULL)
                           /* ^ dangersign; changes N_COLS(cl) */
         ;  mclvFree(&truants)
      ;  }
         n_overlap = n_found - nodes_found->n_ivps
      ;  if (missing) *missing = n_missing
      ;  if (overlap) *overlap = n_overlap
      ;  mclvFree(&nodes_found)
   ;  }
            /* no more missing nodes. whee. */

      if (n_overlap && !(bits & ENSTRICT_REPORT_ONLY))
      {  if (bits & ENSTRICT_SPLIT_OVERLAP) 
         clm_split_overlap(cl)
      ;  else if (bits & ENSTRICT_CUT_OVERLAP)
         clm_cut_overlap(cl)
   ;  }
            /* no more overlap. cracking */

   ;  if (!(bits & ENSTRICT_REPORT_ONLY))
         mclxScrub(cl, MCLX_SCRUB_COLS)
      ,  mclxMapCols(cl, NULL)
            /* no more empty clusters. yay. */

   ;  return n_empty + n_missing + n_overlap
;  }


static int cmp_annot_ssq
(  const void* va
,  const void* vb
)
   {  const mclxAnnot* a = va
   ;  const mclxAnnot* b = vb
   ;  dim i, sa = 0, sb = 0
   ;  for (i=0;i<N_COLS(a->mx);i++)
      sa += a->mx->cols[i].n_ivps * a->mx->cols[i].n_ivps
   ;  for (i=0;i<N_COLS(b->mx);i++)
      sb += b->mx->cols[i].n_ivps * b->mx->cols[i].n_ivps
   ;  return sa < sb ? -1 : sa > sb ? 1 : 0
;  }


static int cmp_annot_ssq_rev
(  const void* a
,  const void* b
)
   {  return -1 * cmp_annot_ssq(a, b)
;  }


void mclxCatSortCoarseFirst
(  mclxCat*    cat
)
   {  qsort(cat->level, cat->n_level, sizeof(cat->level[0]), cmp_annot_ssq_rev)
;  }


void mclxCatSortCoarseLast
(  mclxCat*    cat
)
   {  qsort(cat->level, cat->n_level, sizeof(cat->level[0]), cmp_annot_ssq)
;  }


