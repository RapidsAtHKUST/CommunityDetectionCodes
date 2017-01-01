/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


/* NOTE
 *    the mclx data structure is quite alright. the interfaces
 *    around it could do with some cleaning  up.
 *    There is probably some overlap, some dead code, and some
 *    very infrequently used code.
*/

/* TODO
 *    Unify select routines, mclgUnlinkNodes.
 *    Unify mclxColNums as mclxColSelect ?
 *    Split off mclg graph-type routines
 *    Split off stack routines
 *
 *    make callback/accumulator mechanism to obtain characteristic
 *    vectors for matrix (columns sum, max, self value).
 *
 *    think about mclvTernary in conjunction with sophisticated
 *    mask/merge operations.
 *
 *    cut down on subroutines. unify mclxAccomodate and mclxAllocClone
 *    and possibly others.
 *
 *    Move stack routines somewhere else.
*/


#ifndef impala_matrix_h
#define impala_matrix_h

#include <stdio.h>
#include <stdarg.h>

#include "ivp.h"
#include "vector.h"
#include "pval.h"

#include "util/types.h"
#include "util/ting.h"
#include "util/list.h"



typedef struct
{  mclv*     cols
;  mclv*     dom_cols
;  mclv*     dom_rows
;
}  mclMatrix      ;

#define mclx mclMatrix


/* INTEGRITY
 *    Remember the constraint on a vector: The idx members of the ivp array
 *    need to be nonnegative and ascending.  The val members are usually
 *    positive, but this is not mandated. A val member that is zero
 *    will cause removal of the ivp it is contained in a vector that
 *    is submitted to mclvUnary or mclvBinary. These routines are indirectly
 *    called by many of the routines in this interface.
 *
 *    cols is an array of vectors. The count of vectors is equal to
 *    dom_cols->n_ivps (but always use the macro N_COLS(mx)). The successive
 *    vids (the vid member) of the vectors in cols are the indices (the idx
 *    members of the ivps) in dom_cols. These vids must be ascending and
 *    nonnegative.  Obviously there is duplication of data here; the vid
 *    members of the vectors in cols are exactly the indices of the dom_cols
 *    member.  This duplication is quite useful however, as many matrix
 *    operations require domain access, checking, and manipulation that can
 *    easily be formulated in terms of base vector methods.  Only when doing
 *    non-standard stuff with matrices one must take care to maintain data
 *    integrity; i.e. when adding a column to a matrix. mclxAccomodate
 *    can be used for such a transformation.
 *
 *    The vectors accessible via cols have entries (i.e. ivp idx members) that
 *    must be present as indices (of the ivps in) in dom_rows.
 *
 *    The setting cols == NULL, dom_cols->n_ivps == 0, dom_cols->ivps == NULL
 *    is obviously legal.
 *
 *    In normal mode of operation, all matrix operations are simply carried out
 *    by routines in this interface, and they will ensure the integrity of the
 *    matrix structure. If you need to do something out of the ordinary, beware
 *    of the constraints just given.
 *
 * NOTE
 *    All unary and binary operations remove resulting entries that are zero.
 *    E.g. mclxAdd, mclxHadamard (both binary).
 *    To remove explictily encoded zeroes from a matrix use mclxUnary(mx, fltxCopy)
 *
 *    It is thus dangerous to use those operations on matrices
 *    representing value tables that contain zero-valued entries -
 *    one might want to combine two different such matrices on different
 *    domains simply by using mclxAdd, but the zero-valued entries will
 *    be removed.
 *
 *    There is currently no easy way around this. See also ../README .
*/

 /* only allowed as rvalue */
#define N_COLS(mx) ((dim) ((mx)->dom_cols->n_ivps * 1))
#define N_ROWS(mx) ((dim) ((mx)->dom_rows->n_ivps * 1))

#define MAXID_COLS(mx) (N_COLS(mx) ? ((mx)->dom_cols->ivps)[N_COLS(mx)-1].idx : 0)
#define MAXID_ROWS(mx) (N_ROWS(mx) ? ((mx)->dom_rows->ivps)[N_ROWS(mx)-1].idx : 0)

#define mclxRowCanonical(mx) MCLV_IS_CANONICAL((mx)->dom_rows)
#define mclxColCanonical(mx) MCLV_IS_CANONICAL((mx)->dom_cols)

#define mclxDomCanonical(mx)     (mclxRowCanonical(mx) && mclxColCanonical(mx))
#define mclxGraphCanonical(mx)   (mclxDomCanonical(mx) && N_ROWS(mx) == N_COLS(mx))

#define mclxIsGraph(mx) (  mclxGraphCanonical(mx)           \
                        || mcldEquate((mx)->dom_rows, (mx)->dom_cols, MCLD_EQT_EQUAL)  \
                        )


#define     MCLX_PRODUCE_DOMTREE       1 <<  0     /* */
#define     MCLX_PRODUCE_DOMSTACK      1 <<  1
#define     MCLX_REQUIRE_DOMTREE       1 <<  2
#define     MCLX_REQUIRE_DOMSTACK      1 <<  3
#define     MCLX_REQUIRE_NESTED        1 <<  4      /* with stack format */
#define     MCLX_ENSURE_ROOT           1 <<  5
#define     MCLX_PRODUCE_PARTITION     1 <<  6
#define     MCLX_REQUIRE_PARTITION     1 <<  7
#define     MCLX_REQUIRE_CANONICALC    1 <<  8
#define     MCLX_REQUIRE_CANONICALR    1 <<  9
#define     MCLX_REQUIRE_CANONICAL     (MCLX_REQUIRE_CANONICALC | MCLX_REQUIRE_CANONICALR)
#define     MCLX_REQUIRE_GRAPH         1 << 10
#define     MCLX_MODE_UNUSED           1 << 11


/* args become members of object */

mclx* mclxAllocZero
(  mclv*  dom_cols
,  mclv*  dom_rows
)  ;

mclx* mclxAllocClone
(  const mclx* mx
)  ;


/* args become members of object
*/

mclx* mclxCartesian
(  mclv*  dom_cols
,  mclv*  dom_rows
,  double  val
)  ;


   /* Capabilities could be added to interface; for example
    * requirement that the domain be canonical.
    * OTOH, that could be a dedicated routine.
   */
mclx* mclxCollectVectors
(  mclv* domain       /* allowed to be NULL, otherwise ownership taken */
,  dim   vid          /* starting vid in new matrix */
,  ...                /* pointers to vectors to be copied (end with NULL) */
)  ;


   /* new vids will be handed out starting at highest existing vid + 1,
    * zero if the matrix is emtpy.
   */
void mclxAppendVectors
(  mclx* mx
,  ...                /* pointers to vectors to be copied (end with NULL) */
)  ;


   /* Will look in mx whether vec->vid already exists
   */
void mclxMergeColumn
(  mclx* mx
,  const mclv* vec
,  double (*op)(pval arg1, pval arg2)
)  ;


/*    All arguments remain owned by caller.  The select domains need not be
 *    subdomains of the matrix domains.  They can take any form; mclxSub will
 *    do the appropriate thing (keeping those entries that are in line with the
 *    new domains).

 *    The domains of the new matrix will always be clones of the selection
 *    domains as supplied by caller, even if those are not subdomains of the
 *    corresponding matrix domains.  In this respect, the routine acts just
 *    like mclxChangeDomains, which is effectively an in-place variant of
 *    mclxSub.

 *    If the select domains are wider than the matrix domains, nothing will be
 *    removed and only the domains are changed.
 *
 *    If a select domain is NULL it is interpreted as an empty vector.  NOTE
 *    This is different from mclxSubRead[x] in io.h
*/

mclx*  mclxSub
(  const mclx*     mx
,  const mclv*     colSelect
,  const mclv*     rowSelect
)  ;


/*    Removes nodes not in selection, keeps matrix domains the same.
 *    A NULL domain indicates no selection on that domain.
*/

void mclxReduce
(  mclx*  mx
,  const mclv*  col_select
,  const mclv*  row_select
)  ;


/*  gives back extended sub: all nodes to and from
 *  these two domains
*/

mclx*  mclxExtSub
(  const mclx*  mx
,  const mclv*  col_select
,  const mclv*  row_select
)  ;



/*
 *    Merges arguments into existing domains.
*/

void mclxAccommodate
(  mclx* mx
,  const mclv* dom_cols
,  const mclv* dom_rows
)  ;


/*    Change the domains of a matrix. This routine is effectively
 *    an in-place variant of mclxSub. Like mclxSub, it can widen or narrow
 *    or partly change the domains. If some entries of the matrix
 *    are conflicting with the new domain definitions, those entries
 *    will be removed.
 *    The domain arguments are assimilated; don't use them anymore.
 *    It is allowed to pass NULL arguments, meaning that the corresponding
 *    domain is not changed.
*/

void mclxChangeDomains
(  mclx* mx
,  mclv* dom_cols
,  mclv* dom_rows
)  ;


/*    Return the union of all blocks defined by columns in dom.
 *    If the columns of dom induce a partition, this will be
 *    a block diagonal matrix.
 *    The domains of the returned matrix are both equal
 *    to the col domain of the mx matrix.
*/

mclx*  mclxBlockUnion
(  const mclx*     mx
,  const mclx*     dom
)  ;


/* alternative implementation of mclxBlockUnion
 * fixme: check
*/

mclx*  mclxBlockUnion2
(  const mclx*     mx
,  const mclx*     domain
)  ;


/*
 * fixme: describe semantics for overlapping blocks
*/

mclMatrix*  mclxBlocksC
(  const mclMatrix*     mx
,  const mclMatrix*     domain
)  ;


/*
 * Removes block-internal edges of weight less than or equal to
 * the median of edges going from within the block to outside.
*/

mclx*  mclxBlockPartition
(  const mclx*     mx
,  const mclx*     domain
,  int             quantile   /* only median supported for now */
)  ;


/*    Change the row domain of a matrix. If some entries of the old domain
 *    are not present in the new domain, the corresponding entries will be
 *    removed from the matrix.
 *    The domain argument is assimilated; don't use it anymore.
*/

void mclxChangeRDomain
(  mclx* mx
,  mclv* domain
)  ;


/*    Change the column domain of a matrix. If some entries of the old domain
 *    are not present in the new domain, the corresponding columns will be
 *    removed from the matrix.
 *    The domain argument is assimilated; don't use it anymore.
*/

void mclxChangeCDomain
(  mclx* mx
,  mclv* domain      /* fixme; check consistency, increasing order */
)  ;


double  mclxSelectValues
(  mclx*  mx
,  double      *lft        /* NULL for turning of lft comparison        */
,  double      *rgt        /* NULL for turning of rgt comparison        */
,  mcxbits     equate      /*  0,1, or 2 of { MCLX_EQT_GT, MCLX_EQT_LT }*/
)  ;
                           /*  By default, LQ and/or GQ are assumed     */


mclx* mclxIdentity
(  mclv* vec
)  ;


mclx* mclxCopy
(  const mclx*        mx
)  ;


void mclxFree
(  mclx**    mx
)  ;


void mclxTransplant
(  mclx* dst
,  mclx** src      /* will be freed */
)  ;


void mclxScaleDiag
(  mclx* mx
,  double fac
)  ;


mclx* mclxDiag
(  mclv* vec
)  ;


mclx* mclxConstDiag
(  mclv* vec
,  double  c
)  ;


void mclxMakeStochastic
(  mclx*    mx
)  ;


mclx* mclxTranspose
(  const mclx*    m
)  ;

mclx* mclxTranspose2
(  const mclx*  m
,  int nozeroes
)  ;


void mclxMakeCharacteristic
(  mclx*          m
)  ;

void mclxZeroValues
(  mclx*          m
)  ;  

dim mclxNrofEntries
(  const mclx*    m
)  ;

double mclxMass
(  const mclx*    m
)  ;



   /* mode:
    *    MCL_VECTOR_SPARSE
    *    MCL_VECTOR_COMPLETE
   */

mclv* mclxColNums
(  const mclx*    m
,  double        (*f_cb)(const mclv * vec)
,  mcxenum        mode
)  ;

mclv* mclxRowSizes
(  const mclx* m
,  mcxenum mode  
)  ;


   /* Returns a domain vector only including those (column) indices
    * for which f_cb returned nonzero.
   */
mclv* mclxColSelect
(  const mclx*    m
,  double        (*f_cb)(const mclv*, void*)
,  void*          arg_cb
)  ;


dim mclxSelectUpper
(  mclx*  mx
)  ;

dim mclxSelectLower
(  mclx*  mx
)  ;


mclx* mclxMax
(  const mclx*    m1
,  const mclx*    m2
)  ;


mclx* mclxMinus
(  const mclx*  m1
,  const mclx*  m2
)  ;


mclx* mclxAdd
(  const mclx*  m1
,  const mclx*  m2
)  ;


void mclxAddTranspose
(  mclx* mx
,  double diagweight
)  ;


mclx* mclxHadamard
(  const mclx*  m1
,  const mclx*  m2
)  ;


void mclxInflate
(  mclx*  mx
,  double power
)  ;


/* result has col domain: the union of col domains
 * and row domain: the union of row domains
*/

mclx* mclxBinary
(  const mclx*  m1
,  const mclx*  m2
,  double (*f_cb)(pval, pval)
)  ;


/* Inline merge; m1 is modified and returned.  domains of m1 are *not* changed.
 * any entries in m2 not in the domains of m1 is discarded.
 *
 * Note: this uses mclvBinary internally, which is inefficient for meet/diff
 * type operations.
*/

void mclxMerge
(  mclx* m1
,  const mclx* m2
,  double (*f_cb)(pval, pval)
) ;

void mclxMergeTranspose
(  mclx* mx
,  double (*op)(pval arg1, pval arg2)
,  double diagweight
)  ;

void mclxMergeTranspose3
(  mclx* mx
,  double (*op)(pval arg1, pval arg2, pval arg3)
,  double diagweight
,  double arg3
)  ;

/* inline add; m1 is modified.
 * domains of m1 are changed if necessary
*/

void mclxAugment
(  mclMatrix* m1
,  const mclMatrix* m2
,  double (*fltop)(pval, pval)
)  ;


double mclxMaxValue
(  const mclx*  m
)  ;


/* mode one of
 *    MCL_VECTOR_COMPLETE or
 *    MCL_VECTOR_SPARSE
*/

mclv* mclxColSums
(  const mclx*    m
,  mcxenum        mode
)  ;

mclv* mclxPowColSums
(  const mclx*    m
,  unsigned       e           /* e-xponent */
,  mcxenum        mode
)  ;

mclv* mclxDiagValues
(  const mclx*    m
,  mcxenum        mode
)  ;


#define mclxColSizes(m, mode) mclxColNums(m, mclvSize, mode)

         /* returns -1 on error, unless always_succeed was set */
ofs mclxGetClusterID
(  const mclx*    cl
,  long           nid
,  mcxbool        always_succeed    /* return 0 on error */
)  ;


ofs mclxGetVectorOffset
(  const mclx*    mx
,  long           vid
,  mcxOnFail      ON_FAIL
,  ofs            offset      /* to indicate: "don't know" use -1 */
)  ;


         /* Does binary search from offset onwards */
mclv* mclxGetVector
(  const mclx*    mx
,  long           vid
,  mcxOnFail      ON_FAIL
,  const mclv*    offset      /* allowed to be NULL */
)  ;


         /* Requires vidc to be in column domain
          * Currently does NOT check vidr to be in row domain.
          * Adds vidr to column vidc if not already present.
         */
mclp* mclxInsertIvp
(  mclx*  mx
,  long   vidc
,  long   vidr
)  ;


         /* Does linear search from offset onwards. Possibly useless. */
mclv* mclxGetNextVector
(  const mclx*    mx
,  long           vid
,  mcxOnFail      ON_FAIL
,  const mclv*    offset      /* allowed to be NULL */
)  ;


         /* vids-column association is disrupted! */
void  mclxColumnsRealign
(  mclx*          m
,  int          (*cmp)(const void* vec1, const void* vec2)
)  ;


         /* Map (necessarily) preserves ordering
          * Use e.g. to canonify domains.
         */
mclx* mclxMakeMap
(  mclv*  dom_cols
,  mclv*  new_dom_cols
)  ;


/* Uses scratch in SCRATCH_READY mode
*/

mcxbool mclxMapTest
(  mclx*    map
)  ;

#define mclxMapInvert(map) (mclxMapTest(map) ? mclxTranspose(map) : NULL)

         /* dom should be subset of map->dom_cols.
          *    if successful
          *    -  returns the image of dom under map as an ordered set.
          *    -  *ar_dompp contains the image of dom.
         */
mclv* mclxMapVectorPermute
(  mclv  *dom
,  mclx  *map
,  mclpAR** ar_dompp
)  ;


/* These can be used to map domains (and the corresponding
 * matrix entries accordingly).
 *
 * Mapping of a matrix can also be achieved using matrix
 * multiplication. These two methods do in-place modification.
 * In matrix algrebra the mapping of a matrix is known as 
 * a matrix permutation.
*/

mcxstatus mclxMapRows
(  mclx     *mx
,  mclx     *map
)  ;


mcxstatus mclxMapCols
(  mclx     *mx
,  mclx     *map
)  ;


/* ************************************************************************* */


struct mclx_thread_map
{  const mclx* mx
;
}  ;

         /* In the callback function, dim thread_id
          * /can/ be used if data is an array.
          * The callback function then has to use array[thread_id]
         */
mcxstatus mclxVectorDispatch
(  mclx* mx
,  void* data
,  dim n_thread
,  void (*cb)(mclx* mx, dim i, void* data, dim thread_id)
,  const struct mclx_thread_map* map
)  ;


mcxstatus mclxVectorDispatchGroup
(  mclx* mx
,  void* data
,  dim n_thread
,  void (*cb)(mclx* mx, dim i, void* data, dim thread_id)
,  dim n_group
,  dim group_id
,  const struct mclx_thread_map* map
)  ;


/*************************************
 * *
 **
 * returns number of columns that had zero entries.
*/

dim mclxAdjustLoops
(  mclx*    mx
,  double (*op)(mclv* vec, long r, void* data)
,  void* data
)  ;


double mclxLoopCBremove
(  mclv  *vec
,  long r
,  void*data
)  ;

double mclxLoopCBset
(  mclv  *vec
,  long r_unused
,  void* data
)  ;

double mclxLoopCBifEmpty
(  mclv  *vec
,  long r
,  void*data
)  ;


/* returns 1.0 if vector has no entries */

double mclxLoopCBmax
(  mclv  *vec
,  long r
,  void*data
)  ;

double mclxLoopCBsum
(  mclv  *vec
,  long r
,  void*data
)  ;


/*************************************/


#define MCLX_SCRUB_COLS 1
#define MCLX_SCRUB_ROWS 2
#define MCLX_SCRUB_GRAPH 4

void mclxScrub
(  mclx* mx
,  mcxbits bits
)  ;



/* operation's second arg should be double */

void mclxUnary
(  mclx*  m1
,  double  (*f_cb)(pval, void*)
,  void*  arg           /* double*  */
)  ;

dim mclxUnaryList
(  mclx*    mx
,  mclpAR*  ar       /* idx: MCLX_UNARY_mode, val: arg */
)  ;



      /* Selects on column domain, but removes from row domain as well.
       * Use this for undirected graphs.
       * Returns the domain that was kept (note that domains of m
       * are not touched)
      */
mclv* mclgUnlinkNodes
(  mclMatrix* m
,  dim        sel_gq
,  dim        sel_lq
)  ;


      /* Replaces edge weights by Inverted log-weighted similarity between nodes.
       * Intended use for unweighted graphs.
      */
void mclxILS
(  mclx* mx
)  ;  

      /* Prunes highly-connected nodes to take only the neighbours with
       * highest weights, starting from most connected going to least
       * connected.  The returned vector is the number of nodes considered; the
       * value in node n is the number of discarded neighbours when n was
       * considered.
      */
mclv* mclgCeilNB
(  mclx* mx
,  dim max_neighbours
,  dim* n_hub
,  dim* n_edges_in
,  dim* n_edges_out
)  ;


mclp* mclgArcAdd   (mclx* mx, long vidc, long vidr, double val);
mclp* mclgArcAddto (mclx* mx, long vidc, long vidr, double val);
ofs   mclgEdgeAdd  (mclx* mx, long vidc, long vidr, double val);
ofs   mclgEdgeAddto(mclx* mx, long vidc, long vidr, double val);


                        /* Only retain nodes (i,j) with arc in both directions */
void mclxSymReduce
(  mclx* mx
)  ;


void mclxSymReduceDispatch
(  mclx* mx
,  dim n_thread
)  ;


#define MCLX_PERTURB_RAND  1  <<  0
#define MCLX_PERTURB_CORR  1  <<  1
#define MCLX_PERTURB_SYMMETRIC  1  <<  2

void  mclxPerturb
(  mclx*    mx
,  double   radius
,  mcxbits  modes
)  ;


dim mclxQuantiles
(  mclx* mx
,  double q          /* should be between 0.0 and 1.0 */
)  ;


   /* Normalize column value by self weight.
    * If no self value is found (no loop present for that node)
    * the maximum value is used.
   */
void mclxNormSelf
(  mclx* m
)  ;


   /* 
    * Dup is as produced by mclTabDuplicated, so
    * -  column vid indicates first occurrence,
    * -  column entries indicate further occurrences.
    * Currently this folds using max(), and assumes matrix is a graph.
    * TODO
    *    -  generalise max()
    *    -  per-domain action
    *    -  contract for dup and mx domain mismatches
   */
void mclxFold
(  mclx* mx
,  mclx* dup
)  ;



#endif

