/*   (C) Copyright 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_tok
#define tingea_tok

#include <string.h>
#include "inttypes.h"
#include "types.h"
#include "list.h"
#include "ting.h"



/*  This is a first sketchy attempt at some parse/tokenize routines.
 *  The scope is not very well defined yet. Should it do basic constructs
 *  only, or aim for more power, possibly aided by callbacks, and
 *  god forbid, a state description?
 *
 *  TODO
 *    quoted strings not yet implemented!
 *    SGML not yet implemented!
 *
 *    unify with mcxIOExpect, possibly stuff from ding.h
 *    wide chars?
*/


#define MCX_TOK_MODE_UNIX     1  /* Unix escapes, including esc newlines */
#define MCX_TOK_MODE_QUOTED   2  /* Quotes delimit tokens, hide brackets */ 
#define MCX_TOK_MODE_PLAIN    4
#define MCX_TOK_MODE_SGML     8  /* &code; other? */

#define MCX_TOK_DEL_WS        16 /* only delimiting whitespace */


/*    Returns first character not matching fbool, NULL if none.
*/

char* mcxTokSkip
(  const char* offset
,  int (*fbool)(int c)
,  ofs  len
)  ;


/*
 *  Accounts for nesting.
 *  Will do '}', ')', ']', '>', assuming one of several conventions.
*/

mcxstatus mcxTokMatch
(  const char* offset
,  char**      end
,  mcxbits     mode
,  ofs         len            /* considered if >= 0 */
)  ;


/*
 * Find some token, skipping over expressions.
 * Either *pos == NULL and retval == STATUS_FAIL
 * or     *pos != NULL and retval == STATUS_OK
 * or     *pos != NULL and retval == STATUS_DONE
*/

mcxstatus mcxTokFind
(  const char* offset
,  char*       tok            /* Only tok[0] considered for now! */
,  char**      pos
,  mcxbits     mode
,  ofs         len            /* considered if >= 0 */
)  ;


                              /* fixme.
                                 -  document;
                                 -  add free routine.
                                 -  then perhaps optify.
                              */
mcxLink* mcxTokArgs
(  const char* str
,  long        str_len
,  int*        n_args
,  mcxbits     opts
)  ;


typedef struct
{  mcxTing*    key
;  mcxLink*    args
;  mcxbits     opts
;
}  mcxTokFunc  ;


void mcxTokFuncFree
(  mcxTokFunc* tf
)  ;

mcxstatus mcxTokExpectFunc
(  mcxTokFunc* tf
,  const char* str
,  dim         str_len
,  char**      z
,  int         n_min
,  int         n_max
,  int        *n_args
)  ;


#endif


