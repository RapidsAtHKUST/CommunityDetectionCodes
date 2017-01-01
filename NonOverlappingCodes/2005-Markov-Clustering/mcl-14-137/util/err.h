/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_err_h
#define tingea_err_h

#include <stdio.h>
#include <signal.h>
#include "types.h"


/* Experimental scheme for logging
 * Contains
 *    Three axes that have scales:
 *    -  data            1  2  3
 *                       CELL
 *                          LIST  
 *                             AGGR
 *    -  function/code   1  2  3  4
 *                       LINE
 *                          FUNCTION
 *                             MODULE
 *                                APP
 *    -  monitoring      1  2  3  4  5
 *                       DEBUG
 *                          INFO
 *                             WARN
 *                                ERR
 *                                   PANIC
 *
 *    These scales correspond with
 *
 *    what     (data, what are its parameters)
 *    how      (function, what is changing)
 *    pulse    (anything)
 *
 *    Several axes that are radio buttons:
 *    -  IO
 *    -  Thread
 *    -  gauge       progress bars
 *    -  IP          inter (process)
 *    -  SLOT1       custom
 *    -  SLOT2       custom
 *    -  SLOT3       custom
 *
 * The programmer associates one or more logging levels with a logging
 * statement. The user sets logging levels in terms of quietness.
 * Quietness is quantified as shown in the list below. It may be applied
 * to a single axis or to several axes simultaneously.
 *
 *    x  is silent
 *    5  is tersest monitoring level
 *    4  is tersest function/code level
 *    3  is tersest data level
 *    9  is tersest level possible, may exceed actual upper bound
 *    1  is yappiest level.
 *
 * Only those statements for which the quiet level is quieter than the
 * user-specified level will be printed.  If the programmer combines several
 * axes generally all the corresponding levels are checked and all have to be
 * OK, unless the user has specified that she is happy if at least one level is
 * OK. This is done by including a literal 'V' in the MCXLOGTAG string or in
 * the corresponding command-line option (usually -q).
 *
 * MCXLOGTAG is of the following form:
 *
 *       [<lead-tag>]<[dfgimstABC][0-9]>+[V]
 *
 * The optional <lead-tag> is one of [19x] as described above. The rest are
 * pairs consisting of an indicator
 *
 *    d  data
 *    f  function
 *    g  gauge/progress
 *    i  IO
 *    m  monitoring
 *    n  network
 *    t  thread
 *    A  custom axis 1
 *    B  custom axis 1
 *    C  custom axis 1
 *
 * followed by a level. Each indicator specifies an axis; each axis has its
 * own scale and number of steps on the scale (see above).  Finally, a 'V'
 * occurring anywhere signifies that OR logic is applied to loggin clauses
 * rather than AND logic.
 *
 *    The lead tag sets a logging level for all axes simultaneously as follows:
 *    1  MCX_LOG_ALL
 *    9  MCX_LOG_TERSER
 *    8  MCX_LOG_TERSE
 *    x  MCX_LOG_NONE
*/


extern mcxbits mcxLogLevel;

   /* When called this one optionally checks environment variable
    * MCXLOGTAG to set the logging levels mcxLogLevelSetByString
   */
void mcxLogSetFILE
(  FILE* fp
,  mcxbool  ENV_LOG
)  ;

   /* When writing to this file embed the print statement in an if statement
    * that checks whether your priority is ok with mcxLogGet.
    * This is paramount with GAUGE type logging as a GAUGE = x setting
    * may indicate that the user is logging to a shared stream.
   */
FILE* mcxLogGetFILE
(  void
)  ;


extern volatile sig_atomic_t mcxLogSigGuard ;

void mcxLogSig
(  int sig
)  ;


/* ******************** data */

#define MCX_LOG_CELL       1 <<   0    /* node, point, scalar */
#define MCX_LOG_LIST       1 <<   1    /* list, tree, hash    */
#define MCX_LOG_AGGR       1 <<   2    /* everything else     */

#define     MCX_LOG_DATA  (MCX_LOG_CELL | MCX_LOG_LIST | MCX_LOG_AGGR)
#define     MCX_LOG_DATA0  MCX_LOG_CELL


/* ******************** function */

#define MCX_LOG_LINE       1u <<  3
#define MCX_LOG_FUNCTION   1u <<  4
#define MCX_LOG_MODULE     1u <<  5
#define MCX_LOG_APP        1u <<  6

#define     MCX_LOG_FUNC  (MCX_LOG_LINE | MCX_LOG_FUNCTION | MCX_LOG_MODULE | MCX_LOG_APP)
#define     MCX_LOG_FUNC0  MCX_LOG_LINE


/* ******************** monitoring */

#define MCX_LOG_DEBUG      1u <<  7
#define MCX_LOG_INFO       1u <<  8
#define MCX_LOG_WARN       1u <<  9
#define MCX_LOG_ERR        1u << 10
#define MCX_LOG_PANIC      1u << 11

#define     MCX_LOG_MON  ( MCX_LOG_DEBUG| MCX_LOG_INFO | MCX_LOG_WARN | MCX_LOG_ERR| MCX_LOG_PANIC )
#define     MCX_LOG_MON0   MCX_LOG_DEBUG


/* ********************* unimodal axes   */

#define MCX_LOG_IO         1u << 12
#define MCX_LOG_IP         1u << 13
#define MCX_LOG_THREAD     1u << 15
#define MCX_LOG_NETWORK    1u << 16

#define  MCX_LOG_ASPECTS ( MCX_LOG_IO | MCX_LOG_IP | MCX_LOG_THREAD | MCX_LOG_GAUGE | MCX_LOG_NETWORK )


/* ********************* miscellaneous*/

#define MCX_LOG_GAUGE      1u << 17


/* ********************* control      */

#define MCX_LOG_AND        1u << 18
#define MCX_LOG_OR         1u << 19
#define MCX_LOG_NULL       1u << 20     /* turns of logging */


/* ********************* unspecified  */

#define MCX_LOG_SLOT1      1u << 22
#define MCX_LOG_SLOT2      1u << 22
#define MCX_LOG_SLOT3      1u << 23

#define  MCX_LOG_SLOT    ( MCX_LOG_SLOT1 | MCX_LOG_SLOT2 | MCX_LOG_SLOT3 )


/* ********************* all / terse  */

#define MCX_LOG_NONE       0
#define MCX_LOG_VERBOSE  ( MCX_LOG_CELL | MCX_LOG_LINE | MCX_LOG_DEBUG | MCX_LOG_SLOT | MCX_LOG_ASPECTS )
#define MCX_LOG_TERSER   ( MCX_LOG_PANIC | MCX_LOG_AGGR | MCX_LOG_APP )
#define MCX_LOG_TERSE    ( MCX_LOG_TERSER | MCX_LOG_ASPECTS )
#define MCX_LOG_UNIVERSE ( MCX_LOG_DATA | MCX_LOG_FUNC | MCX_LOG_MON | MCX_LOG_SLOT | MCX_LOG_ASPECTS )


/* ********************* unused       */

#define MCX_LOG_UNUSED     1u << 24


void mcxLogLevelSetByString
(  const char* str
)  ;

void mcxLog
(  mcxbits level_programmer
,  const char* tag
,  const char* fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 3, 4)))
#endif
   ;

mcxbool mcxLogGet
(  mcxbits level_programmer
)  ;


void mcxLog2
(  const char* tag
,  const char* fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;


void  mcxTell
(  const char  *caller
,  const char  *fmt
,  ...
)  
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;


void  mcxTellf
(  FILE*       fp
,  const char  *caller
,  const char  *fmt
,  ...
)  
#ifdef __GNUC__
__attribute__ ((format (printf, 3, 4)))
#endif
   ;


void  mcxWarn
(  const char  *caller
,  const char  *fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;

void  mcxErr
(  const char  *caller
,  const char  *fmt
,  ...
)  
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;

void  mcxErrf
(  FILE*       fp
,  const char  *caller
,  const char  *fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 3, 4)))
#endif
   ;

void  mcxDie
(  int status
,  const char  *caller
,  const char  *fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 3, 4)))
#endif
   ;

void mcxTellFile
(  FILE* fp
)  ;

void mcxWarnFile
(  FILE* fp
)  ;

void mcxErrorFile
(  FILE* fp
)  ;


void mcxFail
(  void
)  ;

void mcxExit
(  int val
)  ;

#endif

