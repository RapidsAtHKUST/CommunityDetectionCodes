/*   (C) Copyright 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* NOTES
 *    (source) "ascend" means going from leaves to root.
 *             "descend" means going from root to leaves.
 *             This is a perverse view of trees dangling upside
 *             down in the sky.
 *
 *    what is efficient data structure to, given a tree, find all leaves of an
 *    interal node very quickly?  perhaps something skip-list inspired.
 * 
 *    ascend functionality is basically algorithmically broken; it is very
 *    order dependent, and it is not clear how to choose the right order / mark
 *    nodes as temporarily dealt with.  Even when doing the exhaustive search
 *    (reset frame_curr to frame_start after each ascend)
 *    we do not seem able to find all clusterings that are consistent with the
 *    tree. If we accept this, a speed-up measure might be to track which
 *    frame nodes are affected by the newly created parent node, rather then
 *    blindly recomputing everything as we do now.
 *
 *    Another type of compatibility would be just to gauge how close we
 *    can get to Y by using the leave clustering X as building blocks.
 *    For a start, one could consider joining all X_i with unambiguous
 *    best map to Y_k together.
 *
 *    nearly or really all linked lists start with a dummy node
 *       for which the value is NULL, leading to code of type
 *    mcxLink* iter = some_argument
 * ;  while ((iter = iter->next))
 *    {  STUFF
 *    }
 *
 *    code/structures is dependent on level. look for +leveldependent,
 *    and for CHILDREN{2,}, N_CHILDREN{2,} PARENT, PARENT2.
 *
 *    in ascend mode the frame is kept sorted by level of parent,
 *    smallest first. This is so that 
 *
*/

/* TODO
 *    -  implement --level-down=3 to indicate starting level
 *    -  Document dependencies between the data structures.
 *    -  test/enforce map_target tn values to be in frame_start.
 *    -  test nestedness of input, partition, and anything else
 *          that might fail.
 *    -  efficiency (down): for large domains with targets with lots of small
 *          clusters mcldCountParts is expensive when applied to
 *          top level of tree (approaches O(N^2)).
 *          ?  are we computing provably useless projections (e.g.
 *             provable because of orderdness properties)
 *          ?  should we cache/precompute tnchild->inc in the
 *             descend case (and avoid duplicated work)
 *          ?  deeply consider caching target->tree projection in map_target.
 *             (but leave sets change so impossible?)
 *          ?  obvious shortcuts, e.g. based on counting arguments?
 *    -  Use true tree representation without trivial branches
 *          -  cone input routine
 *          -  newick format input routine
 *    -  go up as well
 *    -  we reloop over nodes that are stable. don't do that (skip list)
 *          (very minor though)
 *    -  Document data structures (but have a look at dump_state)
 *    -  The code could be made more modular (less globals), but the
 *          required structure is pretty much visible already.
 *    -  Find closest super/sub clusterings.
 *    -  Extend towards tree/tree matching. Perhaps iterate.
 *    -  alternate tree traversal (tests locality independence with this)
 *    -  Accept newick format, possibly restricted version.
 *    -  prove (& write down) properties for both up and down.
 *    #  When there is a single node, go down until split or level 0
 *    #  memclean
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/types.h"
#include "util/list.h"
#include "util/gralloc.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/alloc.h"
#include "util/compile.h"

#include "impala/edge.h"
#include "impala/matrix.h"
#include "impala/compose.h"
#include "impala/io.h"
#include "impala/iface.h"
#include "impala/ivp.h"
#include "impala/app.h"

#include "clew/clm.h"

static int ascend_counter_g = 0;


const char* usagelines[] = { NULL };

const char* me = "clmframe";
const char* syntax
=  "Usage: clmframe [options] -icl <mxfile> -icl-tree <mx-tree file>";

enum
{  MY_OPT_VERSION
,  MY_OPT_ICL
,  MY_OPT_ICLTREE
,  MY_OPT_WHITTLE_NUM
,  MY_OPT_WHITTLE
,  MY_OPT_OUT
,  MY_OPT_DEBUG
,  MY_OPT_STRICT_DELTA
,  MY_OPT_TEST_STABILITY
,  MY_OPT_ASCEND
,  MY_OPT_HELP
}  ;

mcxOptAnchor options[] =
{  {  "--version"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_VERSION
   ,  NULL
   ,  "output version information, exit"
   }
,  {  "-icl"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_ICL
   ,  "<fname>"
   ,  "read clustering matrix from file"
   }
,  {  "-icl-tree"
   ,  MCX_OPT_HASARG | MCX_OPT_REQUIRED
   ,  MY_OPT_ICLTREE
   ,  "<fname>"
   ,  "read nested clusterings from file"
   }
,  {  "-eager"
   ,  MCX_OPT_HASARG | MCX_OPT_HIDDEN
   ,  MY_OPT_WHITTLE_NUM
   ,  "<num>"
   ,  "force initial descend of treenodes with #leaves > num * max(target)"
   }
,  {  "--eager"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_WHITTLE
   ,  NULL
   ,  "force initial descend of treenodes with #leaves > 3 * max(target)"
   }
,  {  "--ascend"
   ,  MCX_OPT_DEFAULT | MCX_OPT_HIDDEN
   ,  MY_OPT_ASCEND
   ,  NULL
   ,  "ascend from leaves"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUT
   ,  "<fname>"
   ,  "write output hither"
   }
,  {  "-debug"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DEBUG
   ,  "<int>"
   ,  "enable debugging output (1,2,..)"
   }
,  {  "--strict-delta"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_STRICT_DELTA
   ,  NULL
   ,  "do no descend when delta is 0"
   }
,  {  "--test-stability"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_TEST_STABILITY
   ,  NULL
   ,  "test stability of descend stop criterion"
   }
,  {  "-h"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_HELP
   ,  NULL
   ,  "this info"
   }
,  {  NULL ,  0 ,  0 ,  NULL, NULL}
}  ;

void usage
(  const char**
)  ;


typedef struct treenode
{  dim      level
;  long     clid
;  mclv*    inc            /* list of incident target clusters       */
;  mclv*    leaves         /* the leaves corresponding to this node  */
;  dim      onto_target    /* projection     */

;  mcxbool  descend_stable /* tested in previous iteration */
;  struct treenode* ascend_parent
;  dim      ascend_ctr     /* used for skipping things */
;                          /* in ascend case, keep track of the parent */
}  treenode ;


typedef struct
{  mclx* cluster  /* parent clustering, */
;  mclx* cluster_tp
;  dim   level    /* its level */
;
}  cnode ;


typedef struct
{  mcxLink* src
;
}  map_start ;


static mcxLink*   src_link_g    =  NULL;
static mcxGrim*   src_treenode_g    =  NULL;
static cnode*     clstack_g         =  NULL;
static mclx*      target_g          =  NULL;
static mclx*      target_tp_g       =  NULL;
static dim        n_clstack_g       =  0;
static dim        n_skip_g          =  0;
static map_start* map_target_g      =  NULL;
static mcxbool    test_stability_g  =  FALSE;
static mcxbool    strict_delta_g    =  FALSE;

static int debug_g = 0;
static FILE* stddebug = NULL;

static mcxTing* scratch1_g = NULL;
static mcxTing* scratch2_g = NULL;
static mcxTing* scratch3_g = NULL;



#define CHILDREN(tn)   (clstack_g[(tn)->level].cluster->cols+(tn)->clid)
#define N_CHILDREN(tn) (clstack_g[(tn)->level].cluster->cols[(tn)->clid].n_ivps)

#define CHILDREN2(level,clid)    (clstack_g[level].cluster->cols+clid)
#define N_CHILDREN2(level,clid)  (clstack_g[level].cluster->cols[clid].n_ivps)

#define PARENT(tn)            (clstack_g[(tn)->level+1].cluster_tp->cols[(tn)->clid].ivps[0].idx)
#define PARENT2(level, clid)  (clstack_g[(level)+1].cluster_tp->cols[clid].ivps[0].idx)

   /* NOTE
    *   level is the starting level.
    *   To facilitate the mclcm type cone output (which may contain trivial clusters)
    *   we go up (ascend towards root) or down (descend towards leaves) the tree
    *   skipping such trivial clusters
   */
treenode* treenode_new
(  mcxGrim* gr
,  mcxbool ascend
,  dim   level
,  long  clid
,  dim   onto_target
,  mclv* inc
,  mclv* leaves
)
   {  treenode* tn = mcxGrimGet(gr)
   ;  tn->ascend_parent = NULL
   ;  tn->ascend_ctr    = ascend_counter_g

   ;  if (ascend)
      {              /* while parent has a single child go up the tree
                     */
         while (level < n_clstack_g-1)
         {  dim parent_clid = PARENT2(level, clid)
         ;  dim n_children_parent = N_CHILDREN2(level+1, parent_clid)
;if(0)fprintf(stddebug, "@@ (%d,%d) [%d] has parent/n_children_parent (%d,%d)/%d\n", (int) level, (int) clid, (int) n_clstack_g, (int) level+1, (int) parent_clid, (int) n_children_parent)
         ;  if (n_children_parent == 1)
               clid = parent_clid
            ,  level++
         ;  else
            break
      ;  }
;if(0)fprintf(stddebug, "@@ picking (%d,%d)\n", (int) level, (int) clid)
;
      }
      else
      while (N_CHILDREN2(level,clid) == 1 && level > 0)
      {  clid = CHILDREN2(level,clid)->ivps[0].idx
      ;  level--
      ;  n_skip_g++
   ;  }

      tn->level = level
   ;  tn->clid  = clid
   ;  tn->inc   = inc
   ;  tn->onto_target = onto_target
   ;  tn->leaves = leaves
   ;  tn->descend_stable = FALSE
   ;

            /* !leaves means we are in nested call already.
             * level == n_clstack_g means we are toplevel already
            */
      if (ascend && leaves && level < n_clstack_g-1)
      {  dim parent_clid = PARENT2(level, clid)
      ;  tn->ascend_parent
         =  treenode_new
            (  gr
            ,  ascend
            ,  level+1
            ,  parent_clid
            ,  0
            ,  NULL
            ,  NULL
            )
;if(0)fprintf(stddebug, "child (%d,%d) parent (%d,%d)\n", (int) tn->level, (int) tn->clid, (int) tn->ascend_parent->level, (int) tn->ascend_parent->clid)
   ;  }

      return tn
;  }


void treenode_release
(  treenode* tn
)
   {  mclvFree(&tn->leaves)
   ;  mclvFree(&tn->inc)
   ;  if (tn->ascend_parent)
      {  treenode_release(tn->ascend_parent)
      ;  mcxGrimLet(src_treenode_g, tn->ascend_parent)
   ;  }
   }


dim get_projection
(  mclv* cluster
,  mclx* target
,  mclv* incident   /* no need to look at any other than these */
)
   {  dim j, n_best_meet = 0, n_incident = 0
   ;  if (!incident)
      incident = target->dom_cols
   ;  for (j=0;j<incident->n_ivps;j++)
      {  dim n_meet
      ;  long clidx = incident->ivps[j].idx
      ;  mclv* target_cluster = target->cols+clidx
      ;  mcldCountParts(cluster, target_cluster, NULL, &n_meet, NULL)
;if(0)fprintf(stdout, "testing %d vs %d nodes\n", (int) cluster->n_ivps, (int) target_cluster->n_ivps)
      ;  if (n_meet > n_best_meet)
         n_best_meet = n_meet
      ;  n_incident += n_meet
      ;  if (n_incident >= cluster->n_ivps)
         break
   ;  }
      return n_best_meet
;  }


/* polish: bound checks */

mclv* get_leaves_from_node
(  dim      level
,  dim      j
)
   {  mclv* inc = mclvClone(clstack_g[level].cluster->cols+j)
   ;  while (level > 0)
      {  mclv* new = mclgUnionv(clstack_g[level-1].cluster, inc, NULL, SCRATCH_READY, NULL)
      ;  mclvFree(&inc)
      ;  inc = new
      ;  level --
   ;  }
      return inc
;  }


mclx* frame_matrix
(  mcxLink* frame_start
,  dim* abdist
,  dim* badist
)
   {  dim n_clusters = 0, n_nodes = 0, i=0
   ;  mclv* dom_rows = mclvClone(target_g->dom_rows)
   ;  mcxLink* lk = frame_start
   ;  mclx* result
   ;  while ((lk = lk->next))
      {  treenode* tn = lk->val
      ;  n_clusters++
      ;  n_nodes += tn->leaves->n_ivps
   ;  }
      if (n_nodes != dom_rows->n_ivps)
      mcxDie
      (  1
      ,  me
      ,  "frame cluster a few nodes short %d/%d"
      ,  (int) n_nodes
      ,  (int) dom_rows->n_ivps
      )

   ;  result = mclxAllocZero(mclvCanonical(NULL, n_clusters, 1.0), dom_rows)
   ;  lk = frame_start
   ;  i = 0

   ;  while ((lk = lk->next))
      {  treenode* tn = lk->val
      ;  mclvCopy(result->cols+i, tn->leaves)
      ;  i++
   ;  }

      clmSJDistance(result, target_g, NULL, NULL, abdist, badist)
   ;  return result
;  }


void report_treenode
(  const char* msg
,  const treenode* tn
,  mcxbool check_incidence
)
   {  mclvSprintf(scratch1_g, tn->inc, MCLXIO_VALUE_NONE, ALL_BITS_OFF)
   ;  mclvSprintf(scratch2_g, CHILDREN(tn), MCLXIO_VALUE_NONE, ALL_BITS_OFF)
   ;  mclvSprintf(scratch3_g, tn->leaves, MCLXIO_VALUE_NONE, ALL_BITS_OFF)

   ;  if (check_incidence)
      {  dim n_meet, n_covered = 0, i
      ;  for (i=0;i<tn->inc->n_ivps;i++)
         {  mcldCountParts
            (  target_g->cols+tn->inc->ivps[i].idx
            ,  tn->leaves
            ,  NULL, &n_meet, NULL
            )
         ;  n_covered += n_meet
      ;  }
         if (n_covered != tn->leaves->n_ivps)
         mcxErrf
         (stddebug, "clmframe panic", "treenode inc list does not cover")
   ;  }
      fprintf
      (  stddebug
      ,  "   [%s] node (%d,%d,%p,%d) [%d,%d]"
         " onto_target %d inc:%s children:%s leaves: %s\n"
      ,  msg
      ,  (int) tn->level
      ,  (int) tn->clid
      ,  (void*) tn
      ,  (int) tn->descend_stable
      ,  (int) (tn->ascend_parent ? tn->ascend_parent->level : -1)   /* note: mixed sign expression */
      ,  (int) (tn->ascend_parent ? tn->ascend_parent->clid : -1)

      ,  (int) tn->onto_target
      ,  scratch1_g->str
      ,  scratch2_g->str
      ,  scratch3_g->str
      )
;  }


dim report_target_incidence
(  const char* msg
,  int tid
,  const treenode* tn
)
   {  dim n_meet
   ;  mcldCountParts(target_g->cols+tid, tn->leaves, NULL, &n_meet, NULL)
   ;  fprintf
      (  stddebug
      ,  "   [%s] target %d is incident with (%d,%d,%p) meet %d\n"
      ,  msg
      ,  (int) tid
      ,  (int) tn->level
      ,  (int) tn->clid
      ,  (void*) tn
      ,  (int) n_meet
      )
   ;  return n_meet
;  }


#define DUMP_FRAME      1 << 0
#define DUMP_MAP        1 << 1
#define DUMP_DISTANCE   1 << 2


void report_distance
(  int n_clusters
,  int abdist
,  int badist
)
   {  fprintf
      (  stddebug
      ,  "   have %d clusters at distance %d/%d\n"
      ,  n_clusters
      ,  abdist
      ,  badist
      )
;  }



dim dump_dist
(  mcxLink* frame_start
,  dim dist
)
   {  dim abdist, badist
   ;  mclx* mx = frame_matrix(frame_start, &abdist, &badist)
   ;  report_distance(N_COLS(mx), abdist, badist)
   ;  if (dist < abdist + badist)
      mcxErr
      (  me
      ,  "BUG: old/new distances %d/%d"
      ,  (int) dist, (int) (abdist + badist)
      )
   ;  mclxFree(&mx)
   ;  return abdist + badist
;  }


         /* todo: test whether tn values in map_target_g lists
          * are all in the current frame
         */
void dump_state
(  const char* msg
,  mcxbits  mode
,  mcxLink* frame_start
)
   {  mcxLink* frame_curr = frame_start
   ;  dim i

   ;  fprintf(stddebug, "\n[%s] dumping state\n", msg)

   ;  if (mode & DUMP_FRAME)
      while ((frame_curr = frame_curr->next))
      report_treenode("", frame_curr->val, TRUE)

   ;  fprintf(stddebug, "\n")

   ;  if (mode & DUMP_MAP)
      for (i=0;i<N_COLS(target_g);i++)
      {  mcxLink* map_link = map_target_g[i].src
      ;  dim n_covered = 0
      ;  while ((map_link = map_link->next))
         {  treenode* tn = map_link->val
         ;  n_covered += report_target_incidence("", i, tn)
      ;  }
         if (n_covered != target_g->cols[i].n_ivps)
         mcxErrf
         (stddebug, "clmframe panic", "target incidence list does not cover")
   ;  }

      if (mode & DUMP_DISTANCE)
      {  dim abdist, badist
      ;  mclx* mx = frame_matrix(frame_start, &abdist, &badist)
      ;  fprintf
         (  stddebug
         ,  "   have %d clusters at distance %d/%d\n"
         ,  (int) N_COLS(mx)
         ,  (int) abdist
         ,  (int) badist
         )
      ;  mclxFree(&mx)
   ;  }
      fputc('\n', stddebug)
;  }


void clean_up_most_globals
(  mcxLink* frame_start
)
   {  mcxLink* frame_curr = frame_start
   ;  dim i

   ;  while ((frame_curr = frame_curr->next))
      treenode_release(frame_curr->val)

   ;  for (i=0;i<n_clstack_g;i++)
      {  mclxFree(&clstack_g[i].cluster)
      ;  mclxFree(&clstack_g[i].cluster_tp)
   ;  }

      mcxFree(clstack_g)
   ;  mcxFree(map_target_g)

   ;  mcxGrimFree(&src_treenode_g)
   ;  mcxListFree(&src_link_g, NULL)
;  }


   /* compute the tree onto target delta
    * and create a linked list of treenodes for the children
   */

mcxLink* fun_descend_tree_predict
(  treenode* tnpivot
,  int *delta_tree_onto_target
)
   {  dim onto_target_children = 0
   ;  mcxLink* child_start    =  mcxLinkSpawn(src_link_g, NULL)
   ;  mcxLink* child_link     =  child_start
   ;  mclv* children_index    =  CHILDREN(tnpivot)
   ;  dim j = 0

   ;  for (j=0;j<children_index->n_ivps;j++)
      {  dim clid_child       =  children_index->ivps[j].idx
                                    /* fixme; costly (need real tree structure) */
      ;  mclv* child_leaves   =  get_leaves_from_node(tnpivot->level-1, clid_child)
      ;  dim child_projection =  get_projection(child_leaves, target_g, tnpivot->inc)
      ;  onto_target_children+=  child_projection

      ;  child_link
         =  mcxLinkAfter
            (  child_link
            ,  treenode_new
               (  src_treenode_g
               ,  FALSE  
               ,  tnpivot->level-1
               ,  clid_child
               ,  child_projection
               ,  mclgUnionv(target_tp_g, child_leaves, NULL, SCRATCH_READY, NULL)
               ,  child_leaves
               )
            )
   ;  }

      *delta_tree_onto_target += onto_target_children - tnpivot->onto_target
   ;  return child_start
;  }


void report_target_list
(  const char* msg
,  int tid
,  mcxLink* map_link
)
   {  if (0)
      return
   ;  while ((map_link = map_link->next))
      report_target_incidence(msg, tid, map_link->val)
;  }


void report_treenode_list
(  const char* msg
,  mcxLink* lk_start
)
   {  mcxLink* lk = lk_start
   ;  while ((lk = lk->next))
      {  treenode* tn = lk->val
      ;  report_treenode(msg, tn, FALSE)
   ;  }
   }


void init_map_target
(  mcxLink* frame_start
)
   {  dim i

   ;  mcxLog(MCX_LOG_MODULE, me, "initializing target/inc ..")

   ;  map_target_g
      =  mcxAlloc(sizeof(map_start*) * N_COLS(target_g), EXIT_ON_FAIL)

   ;  for (i=0;i<N_COLS(target_g);i++)
      {  mcxLink* map_link = mcxLinkSpawn(src_link_g, NULL)
      ;  mcxLink* frame_curr = frame_start
      ;  map_target_g[i].src = map_link

         /* NOTE
          *   we could count intersections and stop
          *   when target_g->cols[i].n_ivps is reached.
          *   That's costly though, with expected gain
          *   0.5 * N_COLS(target_g)
         */
      ;  {  while ((frame_curr = frame_curr->next))
            {  treenode* tn = frame_curr->val
            ;  if (mclvGetIvp(tn->inc, i, NULL))
               map_link = mcxLinkAfter(map_link, tn)
         ;  }
         }
      }
      mcxLog(MCX_LOG_MODULE, me, "done initializing")
;  }


      /* ordered on the level of the parent, smaller first */

void fun_ascend_frame_update
(  mcxLink* frame_offset
,  treenode* pivot_insert     /* it is a new parent */
)
   {  mcxLink* frame_iter = frame_offset

                     /* fixme ugly control structure
                     */
   ;  while (1)
      {  treenode* tniternext

      ;  if (!frame_iter)
         mcxDie(1, me, "no frame_iter SB impossible given dummy start node")

                     /* at frame_start or at very end */
      ;  if (!frame_iter->next)
         {  mcxLinkAfter(frame_iter, pivot_insert)
         ;  break
      ;  }

         tniternext = frame_iter->next->val

      ;  if
         (  tniternext->ascend_parent
         && pivot_insert->ascend_parent
         && tniternext->ascend_parent->level >= pivot_insert->ascend_parent->level
         )
         {  mcxLinkAfter(frame_iter, pivot_insert)
         ;  break
      ;  }

         frame_iter = frame_iter->next
   ;  }
   }


int fun_ascend_treenode_cmp
(  const void* tn1v
,  const void* tn2v
)
   {  const treenode* tn1 = *((treenode**)tn1v)
   ;  const treenode* tn2 = *((treenode**)tn2v)
   ;  if (!tn2->ascend_parent)
      return -1
   ;  else if (!tn1->ascend_parent)
      return -1
   ;  return (tn1->ascend_parent->level - tn2->ascend_parent->level)
;  }


void init_whittle_frame
(  mcxLink* frame_start
,  dim w
)
   {  mcxLink* frame_iter = frame_start
   ;  dim cs_max = 0, i, maxsize

   ;  for (i=0;i<N_COLS(target_g);i++)
      if (target_g->cols[i].n_ivps > cs_max)
      cs_max = target_g->cols[i].n_ivps
   
   ;  maxsize = w * cs_max

   ;  mcxLog
      (  MCX_LOG_MODULE
      ,  me
      ,  "initial descend towards %d dropoff"
      ,  (int) maxsize
      )

   ;  while ((frame_iter = frame_iter->next))
      {  treenode* tn = frame_iter->val
      ;  {  if (tn->leaves->n_ivps > maxsize)
            {  mcxLink* frame_iter_set_aside = frame_iter
            ;  int discard = 0
            ;  mcxLink* child_start = fun_descend_tree_predict(tn, &discard)
            ;  mcxLink* child_end = child_start

            ;  while (child_end->next)
               {  child_end = child_end->next
               ;  { treenode* tnn = child_end->val
                  ;  if (!tnn->leaves) mcxDie(1, me, "aampf")
               ;  }
               }

            ;  mcxLinkClose(frame_iter->prev, child_start->next)
            ;  mcxLinkClose(child_end, frame_iter->next)
            ;  frame_iter = frame_iter->prev

            ;  mcxLinkRemove(child_start)
            ;  treenode_release(tn)
            ;  mcxGrimLet(src_treenode_g, tn)
            ;  mcxLinkRemove(frame_iter_set_aside)
         ;  }
         }
   ;  }

      mcxLog(MCX_LOG_MODULE, me, "at initial descend")
;  }


mcxLink* init_frame
(  mcxbool  ascend
)
   {  dim level = ascend ? 0 : n_clstack_g-1
   ;  mclx* cl = clstack_g[level].cluster
   ;  mcxLink* frame_start =  mcxLinkSpawn(src_link_g, NULL)
   ;  mcxLink* frame_curr  =  frame_start
   ;  dim i

   ;  treenode** tnarray
      =  ascend
         ?  mcxAlloc(sizeof(treenode*) * N_COLS(cl), EXIT_ON_FAIL)
         :  NULL

   ;  mcxLog(MCX_LOG_MODULE, me, "initializing leaves/inc at level %d ..", (int) level)

   ;  for (i=0;i<N_COLS(cl);i++)
      {  mclv* leaves = get_leaves_from_node(level, i)
      ;  mclv* inc = mclgUnionv(target_tp_g, leaves, NULL, SCRATCH_READY, NULL)
      ;  dim projection = get_projection(leaves, target_g, inc)

      ;  treenode* tnpivot
         =  treenode_new
            (  src_treenode_g
            ,  ascend
            ,  level
            ,  i
            ,  projection
            ,  inc
            ,  leaves
            )

      ;  if (ascend)
         tnarray[i] = tnpivot
      ;  else
         frame_curr = mcxLinkAfter(frame_curr, tnpivot)
   ;  }

      if (ascend)
      {  qsort(tnarray, N_COLS(cl), sizeof(treenode*), fun_ascend_treenode_cmp)
      ;  for (i=0;i<N_COLS(cl);i++)
         frame_curr = mcxLinkAfter(frame_curr, tnarray[i])
      ;  mcxFree(tnarray)
   ;  }

      mcxLog(MCX_LOG_MODULE, me, "done initializing")
   ;  return frame_start
;  }


   /* we should be able to just compare tn == tnpivot */
mcxbool not_in_skiplist
(  mcxLink* frame_skipstart
,  treenode* tnpivot
)
   {  mcxLink* skipiter = frame_skipstart
   ;  while ((skipiter = skipiter->next))
      {  mcxLink* frame = skipiter->val
      ;  treenode* tn = frame->val
      ;  if (tnpivot->level == tn->level && tnpivot->clid == tn->clid)
         return FALSE
   ;  }
      return TRUE
;  }


void fun_ascend_target_predict     /* target onto tree */
(  treenode* pivot_parent
,  mcxLink* frame_skipstart
,  int* onto_tree_target_parent
,  int* onto_tree_target_children
)
   {  dim i
   ;  *onto_tree_target_parent = 0
   ;  *onto_tree_target_children = 0

     /*~~~~~~~~~~~~~~~~~~~~ for all incident targets */
   ;  for (i=0;i<pivot_parent->inc->n_ivps;i++)
      {  dim tid = pivot_parent->inc->ivps[i].idx
      ;  mcxLink* map_link = map_target_g[tid].src
      ;  dim n_meet_best_child = 0
      ;  dim n_meet_best_parent = 0
      ;  dim n_meet = 0

        /*~~~~~~~~~~~~~~~~~~~~ ... compute parent projection */
      ;  mcldCountParts
         (  pivot_parent->leaves
         ,  target_g->cols+tid
         ,  NULL
         ,  &n_meet
         ,  NULL
         )
      ;  n_meet_best_parent = n_meet

        /*~~~~~~~~~~~~~~~~~~~~ ... and all current/child projections */
      ;  while ((map_link = map_link->next))
         {  treenode* tnchild = map_link->val
         ;  mcldCountParts
            (  tnchild->leaves
            ,  target_g->cols+tid
            ,  NULL
            ,  &n_meet
            ,  NULL
            )
         ;  if (n_meet > n_meet_best_child)
            n_meet_best_child = n_meet
         ;  if
            (  n_meet > n_meet_best_parent
            && not_in_skiplist(frame_skipstart, tnchild)
            )
            n_meet_best_parent = n_meet
      ;  }
         *onto_tree_target_children += n_meet_best_child
      ;  *onto_tree_target_parent += n_meet_best_parent
;if(0)fprintf(stddebug, "++ best target %d onto previous: %d\n", (int) tid, (int) n_meet_best_child)
;if(0)fprintf(stddebug, "++ best target %d onto parent: %d\n", (int) tid, (int) n_meet_best_parent)
   ;  }
   }


void fun_descend_target_predict     /* target onto tree */
(  treenode* tnpivot
,  mcxLink* child_start
,  int* onto_tree_target_pivot
,  int* onto_tree_target_children
)
   {  dim i
   ;  *onto_tree_target_pivot = 0
   ;  *onto_tree_target_children = 0

     /*~~~~~~~~~~~~~~~~~~~~ for all incident targets */
   ;  for (i=0;i<tnpivot->inc->n_ivps;i++)
      {  dim tid = tnpivot->inc->ivps[i].idx
      ;  mcxLink* map_link = map_target_g[tid].src
      ;  dim n_meet_best_pivot = 0
      ;  dim n_meet_best_children = 0     /* where s/pivot/children */
      ;  dim clid_best_pivot = 0
      ;  dim clid_best_children = 0
        /*~~~~~~~~~~~~~~~~~~~~ ... and all incident tree clusters */
      ;  while ((map_link = map_link->next))
         {  treenode* tn = map_link->val
         ;  dim n_meet
           /*~~~~~~~~~~~~~~~~~ this is the pivot-level meet */
         ;  mcldCountParts
            (  tn->leaves
            ,  target_g->cols+tid
            ,  NULL
            ,  &n_meet
            ,  NULL
            )
         ;  if (n_meet > n_meet_best_pivot)
            {  n_meet_best_pivot = n_meet
            ;  clid_best_pivot = tn->clid
         ;  }
           /*~~~~~~~~~~~~~~~~~ not parent: account at child level as well */
            if (tnpivot != tn && n_meet > n_meet_best_children)
               n_meet_best_children = n_meet
            ,  clid_best_children = tn->clid
           /*~~~~~~~~~~~~~~~~~ parent: descend to compute child projections */
         ;  else if (tnpivot == tn)
            {  mcxLink* lk = child_start
            ;  while ((lk = lk->next))
               {  treenode* tnchild = lk->val
               ;  dim n_meet_child
               ;  mcldCountParts
                  (  tnchild->leaves
                  ,  target_g->cols+tid
                  ,  NULL
                  ,  &n_meet_child
                  ,  NULL
                  )
            ;  if (n_meet_child > n_meet_best_children)
                  n_meet_best_children = n_meet_child
               ,  clid_best_children = tnchild->clid
            ;  }
            }
         }

      ;  *onto_tree_target_pivot += n_meet_best_pivot
      ;  *onto_tree_target_children += n_meet_best_children
   ;  }
   }


   /*
    * -  insert pivot_parent in appropriate place.
    * -  update map_target_g
    * -  get rid of everything in frame_skipstart->()->()..()->frame_skipend.
   */
void fun_ascend_state_update
(  mcxbool do_ascend
,  treenode* dummy_parent
,  mcxLink*  frame_start
,  mcxLink*  frame_curr_unused cpl__unused
,  mcxLink*  frame_skipstart
)
   {  mcxLink* frame_skipiter = frame_skipstart

   ;  if (do_ascend)
      {  dim i

      ;  treenode* pivot_parent        /* this will create parent of parent */
         =  treenode_new
            (  src_treenode_g
            ,  TRUE  
            ,  dummy_parent->level
            ,  dummy_parent->clid
            ,  dummy_parent->onto_target
            ,  mclvClone(dummy_parent->inc)
            ,  mclvClone(dummy_parent->leaves)
            )

      ;  pivot_parent->ascend_ctr = ascend_counter_g+1

      ;  if (dummy_parent->descend_stable)
         {  mcxErr(me, "TOOT")
         ;  report_treenode("dbg", pivot_parent, FALSE)
         ;  mcxErr(me, "TOOT")
      ;  }

        /*~~~~~~~~~~~~~~~~~~ update map_target_g */
      ;  for (i=0;i<pivot_parent->inc->n_ivps;i++)
         {  dim tid = pivot_parent->inc->ivps[i].idx    /* target id */
         ;  mcxLink* map_link = map_target_g[tid].src

         ;  mcxLinkAfter(map_target_g[tid].src, pivot_parent)
         ;  map_link = map_link->next

;if(0)report_target_list("before", tid, map_target_g[tid].src)

         ;  while ((map_link = map_link->next))
            {  treenode* tn = map_link->val
            ;  mcxLink* skipiter = frame_skipstart
            ;  while ((skipiter = skipiter->next))
               {  if (tn == ((mcxLink*) skipiter->val)->val)
                  break
            ;  }
               if (skipiter)     /* found something so delete map_link */
               {  mcxLink* anchor = map_link->prev
               ;  anchor->next = map_link->next
               ;  if (map_link->next)
                  map_link->next->prev = anchor
               ;  mcxLinkRemove(map_link)
               ;  map_link = anchor
            ;  }
            }
         }

        /*~~~~~~~~~~~~~~~~~~~ remove the frame nodes in skip list */
      ;  while ((frame_skipiter = frame_skipiter->next))
         {  mcxLink* frame_node = frame_skipiter->val
         ;  mcxLink* anchor = frame_node->prev

         ;  anchor->next = frame_node->next
         ;  if (frame_node->next)
            frame_node->next->prev = anchor

                  /* fixme: children of inserted parent all have
                   * ascend_parent node (which is not freed)
                  */
         ;  treenode_release(frame_node->val)
         ;  mcxGrimLet(src_treenode_g, frame_node->val)

         ;  mcxLinkRemove(frame_node)
      ;  }

        /*~~~~~~~~~~~~~~~~~~~ insert the new node in the appropriate place */
         fun_ascend_frame_update(frame_start, pivot_parent)
   ;  }
      else
      {  while ((frame_skipiter = frame_skipiter->next))
         {  mcxLink* frame_node = frame_skipiter->val
         ;  treenode* tn = frame_node->val
#if 0
         ;  tn->ascend_ctr = ascend_counter_g+1
#else
         ;  tn->ascend_ctr = tn->ascend_ctr
#endif
      ;  }
      }

      {  mcxLink* lk = frame_skipstart
      ;  while (lk)
         {  mcxLink* lk_prev = lk
         ;  lk = lk->next
         ;  mcxLinkRemove(lk_prev)
      ;  }
      }
   }



   /*
    * takes ownership of child_start/child_end
    * but not of tnpivot
   */
mcxLink* fun_descend_state_update
(  mcxbool do_descend
,  treenode* tnpivot
,  mcxLink*  frame_curr
,  mcxLink*  child_start
,  mcxLink*  child_end
)
   {  mcxLink* frame_curr_sa  = frame_curr    /* set aside */
   ;  if (frame_curr->val != tnpivot)
      mcxDie(1, me, "descend pivot frame mismatch")
      
   ;  if (do_descend)
      {  dim i

      ;  if (tnpivot->descend_stable)
         {  mcxErr(me, "TOOT")
         ;  report_treenode("dbg", tnpivot, FALSE)
         ;  mcxErr(me, "TOOT")
      ;  }

                           /* must be done *before* below, because
                            * child_end->next must be NULL
                           */
        /*~~~~~~~~~~~~~~~~~~ update map_target_g */
      ;  for (i=0;i<tnpivot->inc->n_ivps;i++)
         {  dim tid = tnpivot->inc->ivps[i].idx    /* target id */
         ;  mcxLink* map_link = map_target_g[tid].src

;if(0)report_target_list("before", tid, map_target_g[tid].src)

         ;  while ((map_link = map_link->next))
            {  treenode* tn = map_link->val

            ;  if (tn == tnpivot)
               {  mcxLink* child_iter        = child_start
               ;  mcxLink* map_link_stale    = map_link
               ;  mcxLink* map_link_left     = map_link->prev
               ;  mcxLink* map_link_right    = map_link->next
               ;  map_link = map_link_left

               ;  while ((child_iter = child_iter->next))
                  {  treenode* tnchild = child_iter->val
                  ;  if (mclvGetIvp(tnchild->inc, tid, NULL))
                     map_link = mcxLinkAfter(map_link, tnchild)
               ;  }
                  map_link->next = map_link_right
               ;  if (map_link_right)
                  map_link_right->prev = map_link
               ;  mcxLinkRemove(map_link_stale)

;if(0)report_target_list("updated", tid, map_target_g[tid].src)

               ;  break
            ;  }
            }
            if (!map_link)
            {  report_target_list("failed", tid, map_target_g[tid].src)
            ;  mcxDie
               (  1
               ,  me
               ,  "pivot node (%d,%d,%p) not found in incidence list for target %d"
               ,  (int) tnpivot->level
               ,  (int) tnpivot->clid
               ,  (void*) tnpivot
               ,  (int) tid
               )
         ;  }
         }
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

                           /* must be done *after* above, because
                            * child_end->next is set below
                           */
        /*~~~~~~~~~~~~~~~~~~ insert the child list in the frame list */
#if 0
      ;  frame_curr->prev->next  =  child_start->next
      ;  child_start->next->prev =  frame_curr->prev
      ;  child_end->next         =  frame_curr->next
      ;  if (frame_curr->next)
         frame_curr->next->prev = child_end
#else
      ;  mcxLinkClose(frame_curr->prev, child_start->next)
      ;  mcxLinkClose(child_end, frame_curr->next)
#endif
      ;  frame_curr = child_end
      ;  mcxLinkRemove(frame_curr_sa)
        /*~~~~~~~~~~~~~~~~~~~ make caller do pivot siblings first */
   ;  }
      else
      {  tnpivot->descend_stable = TRUE
   ;  }

      if (do_descend)
      mcxLinkRemove(child_start)
   ;  else
      {  mcxLink* lk = child_start
      ;  while (lk)
         {  mcxLink* lk_prev = lk
         ;  treenode* tn = lk->val
         ;  if (tn)
               treenode_release(tn)
            ,  mcxGrimLet(src_treenode_g, tn)
         ;  lk = lk->next
         ;  mcxLinkRemove(lk_prev)
      ;  }
      }
      return frame_curr
;  }


   /* returns a linked list with ----> pointers to links <-----
    * in frame_pivot->()->() ..
    * Is later used to update the frame_start list.
    *
    * fixme: loops over *all* elements in frame.
    *
    * Sets leaves, inc and onto_target in pivot_parent
   */
mcxLink* fun_ascend_tree_predict
(  mcxLink*    frame_pivot
,  mcxLink*    frame_start
,  treenode*   pivot_parent          /* parent to frame_pivot->val */
,  int*        delta_tree_onto_target
,  mcxbool*    stable_in_selection
)
   {  mcxLink* iter = frame_start, *frame_skiplist, *frame_skipiter
   ;  treenode* tnchild = frame_pivot->val
   ;  mclv* parent_leaves = mclvClone(tnchild->leaves)
   ;  mclv* parent_inc  =  mclvClone(tnchild->inc)
   ;  dim parent_level  =  pivot_parent->level
   ;  dim parent_clid   =  pivot_parent->clid
   ;  dim children_onto_target = tnchild->onto_target
   ;  dim parent_onto_target = 0
   ;  mcxbool found_self = FALSE

   ;  frame_skipiter = frame_skiplist = mcxLinkSpawn(src_link_g, NULL)

         /* don't do this as we need to preserve the same frame order in the
          * skiplist to be sure our frame_anchor somewhere in caller list makes
          * sense; the first in the skip list must also be the first in the
          * frame list, although it is hard to define why.
         */

;if(0)fprintf(stddebug, "@@ searching for leaves below (%d,%d)\n", (int) parent_level, (int) parent_clid)
;if(0)fprintf(stddebug, "@@ reference child (%d,%d)\n", (int) (tnchild->level), (int) (tnchild->clid))
   ;  while ((iter = iter->next))
      {  treenode* tn = iter->val
      ;  dim this_level = tn->level, this_clid = tn->clid

      ;  if (tn->level == parent_level && tn->clid == parent_clid)   /* note: mixed sign comparison */
         mcxDie
         (  1
         ,  me
         ,  "frame node (%d,%d) should have been found before"
         ,  (int) parent_level
         ,  (int) parent_clid
         )

      ;  if (tn->level >= parent_level)
         continue

      ;  if (iter == frame_pivot)
         found_self = TRUE

      ;  while (this_level < parent_level)
         {
;if(0)fprintf(stddebug, "level %d clid %d\n", (int) (this_level), (int) this_clid)
         ;  this_clid = PARENT2(this_level, this_clid)
         ;  this_level++
;if(0)fprintf(stddebug, "parent %d\n", (int) this_clid)
;
         }

      ;  if (this_level == parent_level && this_clid == parent_clid)
         {  frame_skipiter =  mcxLinkAfter(frame_skipiter, iter)
         ;  children_onto_target += tn->onto_target
         ;  if (tn->descend_stable)
            *stable_in_selection = TRUE
         ;  mcldMerge(parent_leaves, tn->leaves, parent_leaves)
         ;  mcldMerge(parent_inc, tn->inc, parent_inc)
;if(0)mclvSprintf(scratch1_g, tn->leaves, MCLXIO_VALUE_NONE, ALL_BITS_OFF)
;if(0)fprintf(stddebug, "adding (%d,%d) [%s]\n", (int) tn->level, (int) tn->clid, scratch1_g->str)
      ;  }
      }

      if (!found_self)
      mcxDie(1, me, "did not find self!")
   ;

      {  dim j
      ;  for (j=0;j<parent_inc->n_ivps;j++)
         {  dim n_meet
         ;  mcldCountParts
            (  parent_leaves
            ,  target_g->cols+parent_inc->ivps[j].idx
            ,  NULL
            ,  &n_meet
            ,  NULL
            )
         ;  if (n_meet > parent_onto_target)
            parent_onto_target = n_meet
      ;  }
      }

      if (parent_onto_target > children_onto_target)
      mcxDie
      (  1
      ,  me
      ,  "parent (%d,%d) outprojects children"
      ,  (int) parent_onto_target
      ,  (int) children_onto_target
      )

   ;  pivot_parent->leaves    =  parent_leaves
   ;  pivot_parent->inc       =  parent_inc
   ;  pivot_parent->onto_target = parent_onto_target

   ;  *delta_tree_onto_target = children_onto_target - parent_onto_target
   ;  return frame_skiplist
;  }


void fun_ascend_the_tree
(  mcxLink* frame_start
)
   {  dim dist = DIM_MAX
   ;  while (1)
      {  int have_ascend = 0
      ;  int n_tried = 0
      ;  int n_ascend = 0
      ;  mcxLink* frame_curr = frame_start

      ;  if (debug_g)
         dump_state("before ascend", debug_g, frame_start)
      ;  else
         dist = dump_dist(frame_start, dist)

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ loop over current frame */
      ;  while ((frame_curr = frame_curr->next))
         {  treenode* tnpivot = frame_curr->val, *dummy_parent
         ;  mcxLink* frame_skipstart = NULL, *frame_skipend, *frame_anchor
         ;  int delta_tree_onto_target = 0
         ;  int delta_target_onto_tree = 0 

         ;  int delta = 0
         ;  int ascend = 0
         ;  mcxbool stable_in_selection = FALSE

         ;  if (tnpivot->ascend_ctr > ascend_counter_g)  /* note: mixed sign comparison */
            continue

         ;  if (tnpivot->level == n_clstack_g-1)
            continue

                                            /* pbb redundant with level check*/
         ;  if (tnpivot->ascend_parent == NULL && tnpivot->level < n_clstack_g-1)
            mcxDie(1, me, "why is ascend_parent NULL?")

                                            /* dummy is later discarded */
         ;  dummy_parent = tnpivot->ascend_parent
         ;  n_tried++

         ;  if (dummy_parent->leaves || dummy_parent->inc)                  
            mcxDie(1, me, "stupid test (SNBN) fails")
         ;

                                            /*  frame_skipstart has val NULL    */
           /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  create the list of children */
            {  frame_skipend
               =  frame_skipstart
               =  fun_ascend_tree_predict
                  (  frame_curr
                  ,  frame_start
                  ,  dummy_parent
                  ,  &delta_tree_onto_target
                  ,  &stable_in_selection
                  )

            ;  while (frame_skipend->next)
               frame_skipend = frame_skipend->next

            ;  if (!dummy_parent->leaves || !dummy_parent->inc)                  
               mcxDie(1, me, "stupid test II (SNBN) fails")
         ;  }

           /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ compute target onto tree */
            {  int onto_tree_target_parent = 0
            ;  int onto_tree_target_children = 0

            ;  fun_ascend_target_predict
               (  dummy_parent
               ,  frame_skipstart
               ,  &onto_tree_target_parent
               ,  &onto_tree_target_children
               )

            ;  delta_target_onto_tree
               =  onto_tree_target_parent - onto_tree_target_children
;if(0)fprintf(stddebug, "++ %d %d\n", onto_tree_target_parent, onto_tree_target_children)
         ;  }

         ;  if (debug_g)
 mclvSprintf(scratch1_g, tnpivot->leaves, MCLXIO_VALUE_NONE, ALL_BITS_OFF)
,mclvSprintf(scratch2_g, dummy_parent->leaves, MCLXIO_VALUE_NONE, ALL_BITS_OFF)
,           fprintf
            (  stddebug
            ,  "-- the deltas for node (%d,%d,%p [%s]) parent (%d,%d,%p [%s]) are %d %d\n"
            ,  (int) tnpivot->level
            ,  (int) tnpivot->clid
            ,  (void*) tnpivot
,  scratch1_g->str
            ,  (int) dummy_parent->level
            ,  (int) dummy_parent->clid
            ,  (void*) dummy_parent
,  scratch2_g->str
            ,  (int) -delta_tree_onto_target
            ,  (int) delta_target_onto_tree
            )


                           /* hierverder; test */
         ;  delta =  (delta_target_onto_tree - delta_tree_onto_target)
         ;  ascend
            =  (  1
               && (  (strict_delta_g && delta > 0)
                  || delta >= 0
                  )
               )

         ;  have_ascend |= ascend
         ;  n_ascend += ascend > 0

         ; if (debug_g)
            fprintf
            (  stddebug
            ,  "  -- %s level %d clid %d # %d delta %d/%d\n"
            ,  ascend ? "ascend" : "stay"
            ,  (int) tnpivot->level
            ,  (int) tnpivot->clid
            ,  (int) N_CHILDREN(dummy_parent)
            ,  (int) delta_tree_onto_target
            ,  (int) delta_target_onto_tree
            )

                                      /* may change frame_curr->next */
           /*~~~~~~~~~~~~~~~~~~~~~~~~~~~ transfers child_{start,end} */
         ;  frame_anchor = ((mcxLink*) frame_skipstart->next->val)->prev
         ;  fun_ascend_state_update
            (  ascend
            ,  dummy_parent
            ,  frame_start
            ,  frame_curr
            ,  frame_skipstart
            )
         ;  treenode_release(dummy_parent)

         ;  if (ascend)
            frame_curr = frame_anchor
;if(0)dump_state("after considering node", debug_g, frame_start)
;
         }

         fprintf(stddebug, "   - tried/ascend %d/%d\n", (int) n_tried, (int) n_ascend)
      ;  ascend_counter_g++;
      ;  if (!have_ascend)
         break
   ;  }
   }


void fun_descend_the_tree
(  mcxLink* frame_start
)
   {  dim dist = DIM_MAX
   ;  while (1)
      {  int have_descend = 0
      ;  mcxLink* frame_curr = frame_start

      ;  if (debug_g)
         dump_state("before descend", debug_g, frame_start)
      ;  else
         dist = dump_dist(frame_start, dist)

        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ loop over current frame */
      ;  while ((frame_curr = frame_curr->next))
         {  treenode* tnpivot = frame_curr->val
         ;  int delta_tree_onto_target = 0
         ;  int delta_target_onto_tree = 0 
         ;  mcxLink* child_start, *child_end
         ;  int delta = 0
         ;  int descend = 0

                                            /* test independence of stability */
         ;  if (tnpivot->descend_stable && !test_stability_g)
            continue

         ;  if (tnpivot->level == 0)
            continue
         ;
                                            /*  child_start has val NULL    */
           /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  create the list of children */
            {  child_end = child_start
               =  fun_descend_tree_predict
                  (  tnpivot
                  ,  &delta_tree_onto_target
                  )

            ;  while (child_end->next)
               child_end = child_end->next

;if(0)report_treenode_list("new child list", child_start)
         ;  }

           /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ compute target onto tree */
            {  int onto_tree_target_pivot = 0
            ;  int onto_tree_target_children = 0

            ;  fun_descend_target_predict
               (  tnpivot
               ,  child_start
               ,  &onto_tree_target_pivot
               ,  &onto_tree_target_children
               )

            ;  delta_target_onto_tree
               =  onto_tree_target_pivot - onto_tree_target_children
         ;  }

         ;  if (debug_g)
            fprintf
            (  stddebug
            ,  "-- the deltas for node (%d,%d,%p) are %d %d\n"
            ,  (int) tnpivot->level
            ,  (int) tnpivot->clid
            ,  (void*) tnpivot
            ,  (int) delta_tree_onto_target
            ,  (int) -delta_target_onto_tree
            )


         ;  delta = delta_tree_onto_target - delta_target_onto_tree
         ;  descend = (strict_delta_g && delta > 0) || delta >= 0

         ;  have_descend |= descend

         ; if (debug_g)
            fprintf
            (  stddebug
            ,  "  -- %s level %d clid %d # %d delta %d/%d\n"
            ,  descend ? "descend" : "stay"
            ,  (int) tnpivot->level
            ,  (int) tnpivot->clid
            ,  (int) N_CHILDREN(tnpivot)
            ,  (int) delta_tree_onto_target
            ,  (int) delta_target_onto_tree
            )

                                      /* may change frame_curr->next */
           /*~~~~~~~~~~~~~~~~~~~~~~~~~~~ transfers child_{start,end} */
         ;  frame_curr = fun_descend_state_update
            (  descend
            ,  tnpivot
            ,  frame_curr
            ,  child_start
            ,  child_end
            )
         ;  if (descend)
            {  treenode_release(tnpivot)
            ;  mcxGrimLet(src_treenode_g, tnpivot)
         ;  }
         }
         if (!have_descend)
         break
   ;  }
   }


int main
(  int                  argc
,  const char*          argv[]
)  
   {  mcxIO* xftgt   =  NULL
   ;  mcxIO* xfstack =  NULL
   ;  mcxIO* xfout   =  mcxIOnew("out.frame", "w")
   ;  mcxLink* frame_start

   ;  mcxstatus parseStatus   =  STATUS_OK
   ;  mcxOption* opts, *opt

   ;  mcxTing* line  = mcxTingEmpty(NULL, 100)
   ;  dim a = 0

   ;  mcxbool ascend = FALSE
   ;  dim whittle_num = 0

   ;  dim C_clstack_g = 10
   ;  stddebug = stdout
   ;  scratch1_g = mcxTingEmpty(NULL, 1000)
   ;  scratch2_g = mcxTingEmpty(NULL, 1000)
   ;  scratch3_g = mcxTingEmpty(NULL, 1000)

   ;  src_treenode_g=  mcxGrimNew(sizeof(treenode), 1000, MCX_GRIM_GEOMETRIC)
   ;  src_link_g=  mcxListSource(1000, 0)

   ;  clstack_g     =  mcxAlloc(C_clstack_g * sizeof clstack_g[0], EXIT_ON_FAIL)
   ;  n_clstack_g   =  0

   ;  mcxOptAnchorSortById(options, sizeof(options)/sizeof(mcxOptAnchor) -1)
   ;  opts = mcxOptParse(options, (char**) argv, argc, 1, 0, &parseStatus)

   ;  mcxLogLevel =
      MCX_LOG_AGGR | MCX_LOG_MODULE | MCX_LOG_IO | MCX_LOG_GAUGE | MCX_LOG_WARN
   ;  mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_YES)
   ;  mclx_app_init(stderr)

   ;  if (parseStatus != STATUS_OK)
      {  mcxErr(me, "initialization failed")
      ;  exit(1)
   ;  }

      for (opt=opts;opt->anch;opt++)
      {  mcxOptAnchor* anch = opt->anch

      ;  switch(anch->id)
         {  case MY_OPT_ICL
         :  xftgt =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_OUT
         :  mcxIOnewName(xfout, opt->val)
         ;  break
         ;

            case MY_OPT_ICLTREE
         :  xfstack =  mcxIOnew(opt->val, "r")
         ;  break
         ;

            case MY_OPT_WHITTLE
         :  whittle_num = 5
         ;  break
         ;

            case MY_OPT_WHITTLE_NUM
         :  whittle_num = atoi(opt->val)
         ;  break
         ;

            case MY_OPT_ASCEND
         :  ascend = TRUE
         ;  mcxDie(1, me, "ascend functionality needs ascend order algorithm")
         ;  break
         ;

            case MY_OPT_STRICT_DELTA
         :  strict_delta_g = TRUE
         ;  break
         ;

            case MY_OPT_TEST_STABILITY
         :  test_stability_g = TRUE
         ;  break
         ;

            case MY_OPT_DEBUG
         :  debug_g = (unsigned) atoi(opt->val)
         ;  break
         ;

            case MY_OPT_HELP
         :  mcxOptApropos(stdout, me, syntax, 0, MCX_OPT_DISPLAY_SKIP, options)
         ;  fprintf
            (  stdout
            ,
"\nUse this program, given a cone of clusters (encoding a tree) and a clustering,"
"\nto generate a clustering consistent with the tree internal node structure"
"\nthat is the best match for the clustering."
"\n"
            )
         ;  exit(0)
         ;

            case MY_OPT_VERSION
         :  app_report_version(me)
         ;  exit(0)
         ;

            default
         :  mcxDie(1, me, "option parsing bug") 
      ;  }
      }

      if (!xftgt || !xfstack)
      mcxDie(1, me, "-icl and -icl-tree required (cf --help)")

   ;  mcxOptFree(&opts)

   ;  target_g = mclxRead(xftgt, EXIT_ON_FAIL)
   ;  target_tp_g = mclxTranspose(target_g)
   ;  mcxIOopen(xfout, EXIT_ON_FAIL)

   ;  while (1)
      {  mclx* cl
      ;  if (!(cl = mclxRead(xfstack, EXIT_ON_FAIL)))
         break

      ;  if (a >= C_clstack_g)
            C_clstack_g *= 1.4
         ,  clstack_g = mcxRealloc(clstack_g, C_clstack_g * sizeof clstack_g[0], EXIT_ON_FAIL)

      ;  if
         (  a > 0
         && !mcldEquate(cl->dom_rows, clstack_g[a-1].cluster->dom_cols, MCLD_EQT_EQUAL)
         )
         mcxDie(1, me, "domains do not match at level %d", (int) a)
      ;  else if
         (  a == 0
         && !mcldEquate(cl->dom_rows, target_g->dom_rows, MCLD_EQT_EQUAL)
         )
         mcxDie(1, me, "target domain and leaf domain not identical")

      ;  if (!mclxDomCanonical(cl))
         mcxDie(1, me, "only canonical domains are supported")

      ;  clstack_g[a].cluster = cl
      ;  clstack_g[a].level   = a
      ;  clstack_g[a].cluster_tp = ascend ? mclxTranspose(cl) : NULL

      ;  a++

      ;  if (mclxIOformat(xfstack) == 'a')
         mcxIOreadLine(xfstack, line, MCX_READLINE_CHOMP)
      ;  mcxIOreset(xfstack)
      ;  if (EOF == mcxIOskipSpace(xfstack))
         break
   ;  }

      n_clstack_g = a
   ;

              /* initialize linked list with root clustering
               *
               * in the ascend case, we maintain a very specific
               * ordering based on the level of the closest
               * non-trivial parent. When this code is rewritten
               * to deal with general trees the level sentinel must be
               * generalized. +leveldependent
              */
      frame_start = init_frame(ascend)

   ;  if (whittle_num)
      init_whittle_frame(frame_start, whittle_num)
   ;

              /* create confusion storage from target to tree clustering;
               * each target cluster points to a linked list of treenodes
               * to which it is incident.
              */
      init_map_target(frame_start)
   ;

               /* rock and roll baby, yeah */
   ;  if (ascend)
      fun_ascend_the_tree(frame_start)
   ;  else
      fun_descend_the_tree(frame_start)
   ;

      {  dim abdist, badist
      ;  mclx* mx = frame_matrix(frame_start, &abdist, &badist)
      ;  report_distance(N_COLS(mx), abdist, badist)
      ;  mclxWrite(mx, xfout, MCLXIO_VALUE_NONE, RETURN_ON_FAIL)
      ;  mclxFree(&mx)
   ;  }


      {  clean_up_most_globals(frame_start)

      ;  mclxFree(&target_g)
      ;  mclxFree(&target_tp_g)

      ;  mcxIOfree(&xfout)
      ;  mcxIOfree(&xfstack)
      ;  mcxIOfree(&xftgt)

      ;  mcxTingFree(&scratch1_g)
      ;  mcxTingFree(&scratch2_g)
      ;  mcxTingFree(&scratch3_g)
      ;  mcxTingFree(&line)
   ;  }

      fprintf(stderr, "skipped %d redundant nodes\n", (int) n_skip_g)
   ;  return 0
;  }


