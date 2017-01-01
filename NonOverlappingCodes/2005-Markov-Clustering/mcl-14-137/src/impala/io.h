/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011, 2012 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef impala_io_h
#define impala_io_h

#include <math.h>
#include <stdio.h>

#include "ivp.h"
#include "vector.h"
#include "matrix.h"
#include "tab.h"

#include "util/io.h"
#include "util/types.h"
#include "util/ting.h"


/* TODO:
 *    Everything is woefully un(der)documented.
 *    Some fancy stuff should be taken over by cat.[ch]
 *
 *    Make ascii format parsing looser (not line based).
 *    The ascii parsing code is not all that great, heavy-weight.
 *
 *    mcl input format needs a clean approach with grammar and parsing.
 *    Then it would be (much?) faster if not fgetc based but does buffering.
 *
 *    There is now a callback mclxIOinfoReset, invoked by mclxIOclose.
 *    Look out for implications.
*/


#define MCL_APP_VB_NO   2
#define MCL_APP_VB_YES  8

#define MCL_APP_WB_NO   2
#define MCL_APP_WB_YES  8


/*                                1,2         4,8
 * MCLXIOFORMAT                  ascii       binary
 * MCLXIOVERBOSITY               silent      verbose
 *
 * Quad mode:
 *    1  modeX app-alterable
 *    2  modex user-fixed 
 *    4  modeY app-alterable
 *    8  modeY users fixed
*/


mcxbool mclxIOgetQMode        /* quad mode */
(  const char* opt
)  ;


   /* Returns NULL if setting opt failed.
    * Otherwise returns pointer to string put into the environment otherwise.
   */
char* mclxIOsetQMode        /* quad mode */
(  const char* opt
,  unsigned long mode
)  ;

                              /* get format: 'a' or 'b' or '0' (unknown) */
int mclxIOformat
(  mcxIO* xf
)  ;

int get_interchange_digits
(  int valdigits
)  ;


 /* *********************
*/

mclx* mclxRead
(  mcxIO*      xfIn
,  mcxOnFail   ON_FAIL
)  ;



 /* *********************
 *
 *  bits may contain:
 *    MCLX_IS_{GRAPH,CANR,CANC,CAN}
 *
*/

#define MCL_READX_REMOVE_LOOPS  MCLX_MODE_UNUSED << 0

mclx* mclxReadx
(  mcxIO*      xfIn
,  mcxOnFail   ON_FAIL
,  mcxbits     bits
)  ;


 /* *********************
 *
 *    colmask and rowmask are assimilated into matrix.
*/

mclx* mclxSubRead
(  mcxIO*      xfIn
,  mclv*  colmask        /* create submatrix on this domain */
,  mclv*  rowmask        /* create submatrix on this domain */
,  mcxOnFail   ON_FAIL
)  ;


 /* *********************
 *
 *    colmask and rowmask are assimilated into matrix.
*/

mclx* mclxSubReadx
(  mcxIO* xf
,  mclv* colmask    /* create submatrix on this domain */
,  mclv* rowmask    /* create submatrix on this domain */
,  mcxOnFail ON_FAIL
,  mcxbits    bits       /* refer to mclxReadx */
)  ;


 /* *********************
 *
 *
 *    Tries to read cookie, then branches of ascii or binary.
*/
mcxstatus mclxReadDomains
(  mcxIO* xf
,  mclv* dom_cols
,  mclv* dom_rows
)  ;


 /* *********************
*/

mcxstatus mclxReadDimensions
(  mcxIO*      xfIn
,  long        *n_cols
,  long        *n_rows
)  ;



/* ***********************
 * read just the domains.
 * You can still read the full matrix (but you'll get another
 * matrix again).
 * bits only acknowledges MCLX_REQUIRE_GRAPH
*/

mclx* mclxReadSkeleton
(  mcxIO*    xf
,  mcxbits   bits
,  mcxbool   flushmatrix
)  ;


#define MCLXIO_VALUE_NONE    -1
#define MCLXIO_VALUE_INT      0
#define MCLXIO_VALUE_GETENV  -2

#define MCLXIO_FORMAT_ASCII   1
#define MCLXIO_FORMAT_BINARY  2
#define MCLXIO_FORMAT_GETENV  3

mcxstatus mclxWrite
(  const mclx*        mx
,  mcxIO*                  xfout
,  int                     valdigits   /* only relevant for ascii writes */
,  mcxOnFail               ON_FAIL
)  ;

mcxstatus mclxaWrite
(  const mclx*  mx
,  mcxIO*            xfOut
,  int               valdigits
,  mcxOnFail         ON_FAIL
)  ;

mcxstatus  mclxbWrite
(  const mclx*  mtx
,  mcxIO*            xfOut
,  mcxOnFail         ON_FAIL
)  ;


enum
{  MCLXR_ENTRIES_ADD
,  MCLXR_ENTRIES_MAX
,  MCLXR_ENTRIES_MUL
,  MCLXR_ENTRIES_POP
,  MCLXR_ENTRIES_PUSH

,  MCLXR_VECTORS_ADD
,  MCLXR_VECTORS_MAX
,  MCLXR_VECTORS_MUL
,  MCLXR_VECTORS_POP
,  MCLXR_VECTORS_PUSH
}  ;


mclv* mclvaReadRaw
(  mcxIO          *xf
,  mclpAR*        ar
,  mcxOnFail      ON_FAIL
,  int            fintok     /* e.g. '\n' or EOF or '$' */
,  mcxbits        warn_repeat
,  void (*ivpmerge)(void* ivp1, const void* ivp2)
)  ;


mcxstatus mclxaSubReadRaw
(  mcxIO          *xf
,  mclx      *mx
,  mclv           *dom_cols
,  mclv           *dom_rows
,  mcxOnFail      ON_FAIL
,  int            fintok     /* e.g. EOF or ')' */
,  mcxbits        warn_repeat
,  mclpAR*        transform
,  void (*ivpmerge)(void* ivp1, const void* ivp2)
,  double (*fltbinary)(pval val1, pval val2)
)  ;



void mcxPrettyPrint
(  const mclx*  mx
,  FILE*             fp
,  int               width
,  int               digits
,  const char        msg[]
)  ;


void mclFlowPrettyPrint
(  const mclx*  mx
,  FILE*             fp
,  int               digits
,  const char        msg[]
)  ;


mcxstatus mclxTaggedWrite
(  const mclx*  mx
,  const mclx*  dom
,  mcxIO*            xfOut
,  int               valdigits
,  mcxOnFail         ON_FAIL
)  ;



void                 mclxBoolPrint
(  mclx*        mx
,  int          mode
)  ;


mcxstatus mclIOvcheck
(  mclv* vec
,  mclv* dom
)  ;


mcxstatus mclvEmbedRead
(  mclv*        vec
,  mcxIO*       xf
,  mcxOnFail    ON_FAIL
)  ;

mcxstatus mclvWrite
(  mcxIO*            xfout
,  mclv*             dom_rows
,  mclv*             col         /* vid will be set >= 0 if necessary */
,  mcxOnFail         ON_FAIL
)  ;


mcxstatus mclvEmbedWrite
(  const mclv*  vec
,  mcxIO*            xfOut
)  ;


void mclvaWrite
(  const mclv*  vec
,  FILE*             fp
,  int               valdigits
)  ;


               /*    Does *not* accept MCLXIO_VALUE_GETENV.
                *    The default corresponds with a vector printed in a matrix:
                *    no header nor trail, eov values and vid indeed.
               */
#define MCLVA_DUMP_HEADER_ON  1
#define MCLVA_DUMP_VALUE_OFF  2
#define MCLVA_DUMP_VID_OFF    4
#define MCLVA_DUMP_EOV_OFF    8
#define MCLVA_DUMP_TRAIL_ON  16

void mclvaDump
(  const mclv*       vec
,  FILE*             fp
,  int               valdigits         /* -1 for none */
,  const char*       sep
,  mcxbits           opts
)  ;

#define mclvDebug(v,fp,vd) mclvaDump(v, fp, vd, "\t", 0)




/*************************************
 * *
 **
 *
*/


#define MCLX_DUMP_VALUES          1 <<  0
#define MCLX_DUMP_PAIRS           1 <<  1
#define MCLX_DUMP_LINES           1 <<  2
#define MCLX_DUMP_NOLEAD          1 <<  3
#define MCLX_DUMP_PART_UPPER      1 <<  4
#define MCLX_DUMP_PART_LOWER      1 <<  5
#define MCLX_DUMP_PART_UPPERI     1 <<  6
#define MCLX_DUMP_PART_LOWERI     1 <<  7
#define MCLX_DUMP_LOOP_ASIS       1 <<  8
#define MCLX_DUMP_LOOP_NONE       1 <<  9
#define MCLX_DUMP_LOOP_FORCE      1 << 10
#define MCLX_DUMP_MATRIX          1 << 11
#define MCLX_DUMP_TABLE           1 <<  12
#define MCLX_DUMP_KEYS            1 <<  13
#define MCLX_DUMP_TABLE_HEADER    1 <<  14
#define MCLX_DUMP_LEAD_VALUE      1 <<  15
#define MCLX_DUMP_OMIT_EMPTY      1 <<  16


typedef struct
{  mcxbits        modes
;  const char*    sep_lead
;  const char*    sep_row
;  const char*    sep_val
;  const char*    prefixc
;  const char*    siftype
;  double         threshold
;  dim            table_nlines      /* correspond to columns (but lines/rows in output)  */
;  dim            table_nfields     /* correspond to matrix rows (but columns in output) */
;
}  mclxIOdumper   ;


/* 
 * NOTE
 * this routine initializes table_nlines and table_nfields both to zero.
 *
 * Note. we could stabilize the interface by a string-based format
 * + hiding the mclxIOdumper struct. oh well.
 *
*/

void mclxIOdumpSet
(  mclxIOdumper*  dump
,  mcxbits        modes
,  const char*    sep_lead
,  const char*    sep_row
,  const char*    sep_val
)  ;


/* Using MCLX_DUMP_LOOP* options may change the loops
 * in the matrix itself.
*/
mcxstatus mclxIOdump
(  mclx*       mx
,  mcxIO*      xf_dump
,  mclxIOdumper* dump
,  const mclTab*  tabc
,  const mclTab*  tabr
,  int         valdigits
,  mcxOnFail   ON_FAIL
)  ;

/*************************************/


mclpAR *mclpaReadRaw
(  mcxIO       *xf
,  mcxOnFail   ON_FAIL
,  int         fintok     /* e.g. EOF or '$' */
)  ;


void mclxDebug
(  const char* name
,  const mclx* mx
,  int   valdigits
,  const char* msg
)  ;

#endif

