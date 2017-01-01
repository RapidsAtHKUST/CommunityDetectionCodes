/*   (C) Copyright 2006, 2007, 2008 Stijn van Dongen
 *
*/


/* TODO
 *    -  Use bit arrays for seen etc.
 *    -  pathmx is expensive. similar approach to pre-alloc'ed SSPnodes ?
*/

#ifndef gryph_path_h
#define gryph_path_h


#include "impala/matrix.h"
#include "util/types.h"
#include "util/list.h"

/*  SSPxy
 *    Shortest Simple Paths between x and y
*/



typedef struct
{  ofs      src
;  ofs      dst
;  u8*      seen
;  long*    aow         /* alternation of waves              */
;  dim      aow_n       /* aow[0] .. aow[n-1] are meaningful */
;  const    mclx* mx    /* forward arcs */
;  const    mclx* mxtp  /* backward arcs; for undirected graphs use mx */
;  ofs      length
;  dim      n_considered
;  dim      n_involved
;  mclx*    pathmx      /* each column is next layer of nodes */
;  mclx*    stepmx      /* submatrix of input matrix */
;
}  SSPxy    ;


/*
 * ASSUMES
 *    mx encodes an undirected graph.
*/

SSPxy* mclgSSPxyNew
(  const mclx* mx
,  const mclx* mxtp
)  ;


mcxstatus mclgSSPxyQuery
(  SSPxy* sspo
,  long a
,  long b
)  ;


void mclgSSPxyReset
(  SSPxy* sspo
)  ;


void mclgSSPxyFree
(  SSPxy** sspopp
)  ;


/*
 * ASSUMES
 *    graph encodes an undirected graph.
*/

mclv* mclgSSPd
(  const mclx* graph
,  const mclv* domain
)  ;


/*
 * INTERFACE
 *    If has_loop != NULL it should encode those nodes in mx with loops.
 *    Obtain e.g. with mclxColNums(mx, mclvHasLoop, MCL_VECTOR_SPARSE) In that
 *    case only mclnCLCF will offset the presence of loops.  Otherwise mclnCLCF
 *    will assume loops are absent. If has_loop is NULL and loops are present
 *    results will be incorrect.
 *
 * Use mclxAdjustLoops(mx, mclxLoopCBremove, NULL)
 *    to remove loops from the graph.
 *
 * ASSUMES
 *    mx encodes an undirected graph.
*/

double mclnCLCF
(  const mclx* mx
,  const mclv* vec
,  const mclv* has_loop
)  ;


mclv* mclgCLCFdispatch
(  mclx* mx
,  dim n_thread
)  ;


   /* This one requires a clean mx->dom_rows */
dim mclgEcc
(  mclv*       vec
,  mclx*       mx
)  ;


dim mclgEcc2
(  mclv*       vec
,  const mclx* mx
,  mclv*       scratch
)  ;


#endif


