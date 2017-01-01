/*   (C) Copyright 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea under
 * the terms of the GNU General Public License; either version 3 of the License
 * or (at your option) any later version.  You should have received a copy of
 * the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_tr
#define tingea_tr

#include <string.h>

#include "ting.h"
#include "types.h"
#include "inttypes.h"


/*
 * README
 *    This interface is not POSIX compliant. It might evolve to
 *    optionally be indeed.
 *    However, given some of the braindeadliness of POSIX tr compliance,
 *    I don't think the worlds needs another tr implementation.
 *    My gripe is mainly about derailed syntax such as '[:alpha:0'.
 *    It should go down in a ball of flames, not happily parse.
 *    To be honest, I don't know for sure whether this is a POSIX
 *    lack of requirement or an implementation choice.
 *
 *    I did choose to follow most of the POSIX syntax. It is probably
 *    a sign of weakness.
 *    This interface should be able to do everything a POSIX interface can,
 *    possibly more.
 *
 * -  It allows separate specification of src, dst, del and squash sets.
 * -  Provisionally we accept "^spec" to indicate complement,
 *       for any of src dst del squash sets.
 * -  It uses [*c*20] to denote repeats, rather than [c*20].
 *       rationale: do not slam door shut on new syntax.
 * -  It does not recognize '[a-z]' ranges, only 'a-z'.
 *       rationale: none. If ever, notation will be [-a-z] or similar.
 * -  The magic repeat operator [*c#] stops on boundaries
 *       rationale: I like it.
 *       A boundary is introduced by stop/start of ranges and classes.
 * -  The magic repeat operator [*c*] does not stop on boundaries.
 * -  For now, the interface does 1) deletion, 2) translation, 3) squashing.
 *       in the future it may provide a custom order of doing things.
 * 
 *
 * Apart from the fact that you cannot have '\0' in C strings, everything
 * here should work for '\0' as well - specifically the mcxTrTable structure.
 * However, the current interface uses C strings for dst and src and C strings
 * for data.
 *
 * More documentation to follow.
 *
*/

extern const char* mcx_tr_err;
extern mcxbool     mcx_tr_debug;


typedef struct
{  u32      tlt[256]
;  mcxbits  modes
;
}  mcxTR    ;


#define MCX_TR_DEFAULT           0
#define MCX_TR_TRANSLATE   1 <<  1

#define MCX_TR_SOURCE      1 <<  2
#define MCX_TR_DEST        1 <<  3
#define MCX_TR_SQUASH      1 <<  4
#define MCX_TR_DELETE      1 <<  5

#define MCX_TR_SOURCE_C    1 <<  6
#define MCX_TR_DEST_C      1 <<  7
#define MCX_TR_DELETE_C    1 <<  8
#define MCX_TR_SQUASH_C    1 <<  9


#define MCX_TR_COMPLEMENT  1 << 10


mcxstatus mcxTRloadTable
(  mcxTR*      tr
,  const char* src
,  const char* dst
,  const char* set_delete
,  const char* set_squash
,  mcxbits     modes
)  ;


  /*  returns new length of string.
   *  fixme: document map/squash semantics.
  */
ofs mcxTRtranslate
(  char*    src
,  mcxTR*   tr
)  ;


ofs mcxTingTranslate
(  mcxTing*       src
,  mcxTR*         tr
)  ;

ofs mcxTingTr
(  mcxTing*       txt
,  const char*    src
,  const char*    dst
,  const char*    set_delete
,  const char*    set_squash
,  mcxbits        flags
)  ;


/* Accepts e.g. \012 and sets *value to 10.
 * idem \xa0 and \n (\t, \r, \b etc)
 * Does *not* yet accept \0xa0
 *
 * Returns next parsable character.
 *
 * This interface should be moved to ding.
*/

char* mcxStrEscapedValue
(  const char* p
,  const char* z
,  int   *value
)  ;



/* 
 * returns a ting containing all the characters according to bits.
 * bits accept
 *    MCX_TR_SOURCE
 *    MCX_TR_SOURCE_C
 *    MCX_TR_SQUASH
 *    MCX_TR_SQUASH_C
 *    MCX_TR_DELETE
 *    MCX_TR_DELETE_C
 *
 * NOTE
 *    MCX_TR_DEST
 *    MCX_TR_DEST_C
 *    are not yet implemented.
 *
 * NOTE DANGER SIGN
 *    tr no longer contains information on complements that were
 *    used in constructing it.
 *    The complements that bits refer to is simply the information
 *    present in tr.
 *    So a   source of "^a-z"  given to mcxTRloadTable
 *    and    MCX_TR_SOURCE_C given to mcxTRsplash
 *    result in a string containing all of a-z.
*/

mcxTing* mcxTRsplash
(  mcxTR*   tr
,  mcxbits  bits
)  ;


#endif

