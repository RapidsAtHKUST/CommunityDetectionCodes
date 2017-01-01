/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

/* STATUS:
 *    usable:  yes
 *    tested:  yes, stress-tested in zoem and mcl
 *    ad hoc:  somewhat
 *    quirks:  probably a few
 *    support: limited
 *
 * AIMS
 *    -  Provide convenient and efficient wrappers for reading lines, files, searching.
 *    -  Within these wrappers, account bytes and lines read.
 *    It is explicitly not an aim to be an all-encompassing interface, wrapping
 *    everything provided by stdio.h.  The type is not opaque and you are
 *    encouraged to inspect its fp member.
 *    -  The open modes are inspected to infer some knowledge,
 *       then passed on directly to fopen.
 *    -  File "-" is interpreted as either STDIN or STDOUT depending on the open mode.
 *
 * BUGS
 *    -  buffer framework is fully implemented:
 *          mcxIOexpectNum and mcxIOexpectReal ignore buffer.
 *    -  Should incorporate more (f)error checking.
 *
 * TODO:
 *    -  document interfaces.
 *    -  document which routines corrupt the counts.
 *    -  make sure that buffer treats \0 bytes correctly. Should be
 *       pretty close.
 *    -  buffered reads (problematic: mcxIOexpectNum and friends).
 *    -  design {reset,close} framework, esp related to usr member.
 *    ?  support for pipes
*/

#ifndef tingea_file_h
#define tingea_file_h

#include <stdio.h>
#include <sys/types.h>

#include "ting.h"
#include "types.h"


/* The thing below seems a reasonable test for seekability.
 * Let's agree that the main thing is the encapsulation.
 *
*/

#define mcxFPisSeekable(fp) (!fseek(fp, 0, SEEK_CUR))

/* Possibly more stringent check:
 * int had_error  = ferror(file);
 * long curpos    = ftell(file);
 * bool seekable  = (curpos != -1L && fseek(file, curpos, SEEK_SET) == 0);
 * if (!had_error)
 * clearerr(file);
*/



/*  **************************************************************************
 * *
 **            Implementation notes.
 *
 *
 *    This is meant to be a lightweight layer for file operations.
 *    It is so lightweight that the pivotal data structure is not hidden.
 *
 *    Basic usage:
 *       mcxIO* xf = mcxIOnew(somestr, "r");
 *       mcxIOopen(xf, EXIT_ON_FAIL);
 *
 *
 *    Searching:
 *       mcxIOfind(xf, pattern, ON_FAIL)
 *
 *
 *    Reading lines:
 *       mcxIOreadLine(xf, txt, mode)
 *       modes (xor'ed bits):
 *          MCX_READLINE_CHOMP
 *          MCX_READLINE_SKIP_EMPTY
 *          MCX_READLINE_PAR  (read a paragraph)
 *          MCX_READLINE_BSC  (backslash continues line)
 *          MCX_READLINE_DOT  (single dot on single line ends paragraph)
 *    Reading files:
 *       mcxIOreadFile(xf, txt)
 *
 *
 *    Reading bytes:
 *       int c = mcxIOstep(xf)
 *       mcxIOstepback(c, xf)
 *
 *       These keep track of byte count, line count, and ofset within line.
 *
 *
 *    Reset attributes for file name object - change name or mode.
 *       mcxIOrenew(xf, name, mode)
 *
 *
 *    There are some more small utility functions.
 *
    **************************************************************************
   *
  *
 *
 * TODO:
 *    much todo about everything.
 *
 *    mcxIOdiscardLine
 *    mcxIOskipSpace
 *       Change to instance of sth more general.
 *
*/


#define mcxIOateof(xf)  (xf->ateof)
#define mcxIOstdio(xf)  (xf->stdio)
#define mcxIOlc(xf)     ((long) ((xf->lc) + (xf->lo ? 1 : 0)))
                              /* this also takes care of EOF
                               * not preceded by a newline
                              */

/* As long as you did not use mcxIOopen, feel free to do anything with the fn
 * member, especially right after mcxIOnew.
*/

typedef struct
{  mcxTing*       fn
;  char*          mode
;  FILE*          fp
;  dim            lc       /*    line count        */
;  dim            lo       /*    line offset       */
;  dim            lo_      /*    line offset backup, only valid when lo == 0 */
;  dim            bc       /*    byte count        */
;  int            ateof
;  int            stdio
;  mcxTing*       buffer   /*    e.g. when tryCookie fails and unseekable stream */
;  dim            buffer_consumed
;  void*          usr      /*    user object       */
;  mcxstatus    (*usr_reset)(void*)    /*  function to reset user object */
;  void         (*usr_free)(void*)     /*  function to free user object  */
;
}  mcxIO    ;


/*
 *    mcxIOrenew does *not* support callback for resetting the usr object
*/

mcxIO* mcxIOrenew
(  mcxIO*         xf
,  const char*    name
,  const char*    mode
)  ;


mcxIO* mcxIOnew
(  const char*    name
,  const char*    mode
)  ;


mcxstatus mcxIOopen
(  mcxIO*         xf
,  mcxOnFail      ON_FAIL
)  ;


mcxstatus mcxIOtestOpen
(  mcxIO*         xf
,  mcxOnFail      ON_FAIL
)  ;


/*
 *    mcxIOfree does *not* support callback for freeing the usr object
*/

void mcxIOfree
(  mcxIO**  xf
)  ;


void mcxIOfree_v
(  void*  xfpp
)  ;  

void mcxIOrelease
(  mcxIO*   xf
)  ;


void mcxIOerr
(  mcxIO*   xf
,  const char     *complainer
,  const char     *complaint
)  ;


/* Currently, for stdin/stdout/stderr clearerr is issued if necessary.
 * This makes e.g. repeated reads from STDIN possible.
 *
 * usr_reset is called if present.
*/
mcxstatus mcxIOclose
(  mcxIO       *xf
)  ;


mcxstatus mcxIOreset
(  mcxIO       *xf
)  ;


mcxstatus  mcxIOreadFile
(  mcxIO       *xf
,  mcxTing     *fileTxt
)  ;


#define MCX_READLINE_DEFAULT      0
#define MCX_READLINE_CHOMP        1
#define MCX_READLINE_SKIP_EMPTY   2
#define MCX_READLINE_PAR          4
#define MCX_READLINE_BSC          8
#define MCX_READLINE_DOT          16


mcxstatus  mcxIOreadLine
(  mcxIO       *xf
,  mcxTing     *lineTxt
,  mcxbits     flags
)  ;


ofs mcxIOappendChunk
(  mcxIO        *xf
,  mcxTing      *dst
,  dim          sz
,  mcxbits      flags
)  ;


/* Returns the number of bytes that could be discarded.
*/
dim mcxIOdiscardLine
(  mcxIO       *xf
)  ;


/* Returns the number of bytes that could be discarded.
 * ONLY keeps the xf->bc counter up to date.
*/
dim mcxIOdiscard
(  mcxIO       *xf
,  dim         amount
)  ;


/* OK to call this after mcxIOnew, before mcxIOopen */

mcxstatus mcxIOnewName
(  mcxIO*    xf
,  const char* newname
)  ;


/* OK to call this after mcxIOnew, before mcxIOopen */

mcxstatus mcxIOappendName
(  mcxIO*    xf
,  const char* suffix
)  ;


int mcxIOstep
(  mcxIO*    xf
)  ;


int mcxIOstepback
(  int c
,  mcxIO*    xf
)  ;


void mcxIOpos
(  mcxIO*   xf
,  FILE*    channel
)  ;


void mcxIOlistParmodes
(  void
)  ;


/*    
 *    Returns count of trailing characters in str not matching.
*/

int mcxIOexpect
(  mcxIO*         xf
,  const char*    str
,  mcxOnFail      ON_FAIL
)  ;

mcxstatus mcxIOexpectReal
(  mcxIO*         xf
,  double*        dblp
,  mcxOnFail      ON_FAIL
)  ;

mcxstatus mcxIOexpectNum
(  mcxIO*         xf
,  long*          lngp
,  mcxOnFail      ON_FAIL
)  ;


/*
 *    Returns next non-white space char,
 *    which is pushed back onto stream after reading.
*/

int mcxIOskipSpace
(  mcxIO*        xf
)  ;


/*
 *    Purpose: find str in file. If str is found file pointer is set at the end
 *    of match (fgetc or mcxIOstep would retrieve the next byte), otherwise,
 *    the stream is at EOF.
 *
 *    Internally this uses Boyer Moore Horspool (bmh) search.
 *    It processes the stream with fgetc, so the input file need not be
 *    seekable. This means that finding is relatively slow.
 *
 *    An improvement would be to implement faster input munging for seekable
 *    streams, (using reads of size pagesize) and then reposition the stream
 *    after searching.
 *
*/

mcxstatus mcxIOfind
(  mcxIO*         xf
,  const char*    str
,  mcxOnFail      ON_FAIL
)  ;


/*
 *    NOTE
 *       When the cookie is not found this routine does
 *       1) It tries to fseek to the point of departure
 *       2) If that fails, it stores the bytes it could not rewind
 *             in xfin->buffer
 *
 *    +  mcxIOstep
 *    +  mcxIOfind
 *    +  mcxIOskipSpace
 *    +  mcxIOexpect
 *    +  mcxIOreadLine
 *
 *    will access this buffer, but certain other routines will not, e.g.
 *
 *    -  mcxIOreadFile
 *    -  mcxIOexpectNum
 *    -  mcxIOexpectReal
 *    -  all stdio routines (fread, fgetc)
 *
 *    For all mcxIO routines this is an open bug.
 *
*/

mcxbool mcxIOtryCookie
(  mcxIO*        xfin
,  const unsigned char abcd[4]
)  ;

mcxbool mcxIOwriteCookie
(  mcxIO*        xfout
,  const unsigned char abcd[4]
)  ;



#endif

