/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_ting
#define tingea_ting

#include "types.h"

typedef struct
{  char     *str
;  dim      len
;  dim      mxl
;
}  mcxTing  ;


/*  **************************************************************************
 * *
 **            Implementation notes (a few).
 *
*

  Synopsis
 *    This little module provides an objectified C-string compatible interface
 *    to strings.

 * Some philosophy
 *    Do not mind the overhead of 2*sizeof(int) bytes per ting.
 *    If you have humongous amounts of small strings, use C-strings. Optionally
 *    you can do the needed char manipulation in a ting used as scratch-space
 *    using the ting interface, then create a normal string once you are done.
 *    Use mcxTinguish to avoid copying if needed.

 *    Feel free to use the str member with routines from <string.h>
 *    or perhaps from ding.h. Just remember to treat it as a const object
 *    when doing so.

 * Applications
 *    Zoem is the macro processor that processes the zoem language. It is very
 *    string-heavy and its string operations are entirely based on the ting
 *    module.

 * Caveat
 *    Nearly all routines return a NULL pointer to indicate malloc failure.

 * Data structure
 *    str:  array of chars
 *
 *    len:  length of string in str (excluding '\0')
 *          *(str+len) == '\0'
 *
 *    mxl:  current allocated number of writable char's (excluding '\0')
 *          Allocated amount is mxl+1
 *          *(str+mxl) == '\0' - but don't count on that.

 * Notes
 *    mcxTingEnsure is the only routine allowed to fiddle with mxl.
 *    (apart from mcxTingInit which sets it to zero).
 *
 *    Future idea
 *       mcxTingFinalize,  (realloc to length)
 * 
 *    Routines marked `!' will NOT accept ting==NULL argument.
 *    Routines marked `#' should not be called other than by ting routines
 * 
 *                _ #Init  _
 *               /          \
 *          Ensure   <--- #Instantiate
 *         /       \               \
 *   Empty       !Splice        New,Write,Print,PrintAfter
 *   NWrite           |
 *                    |
 *           Append,Insert,Delete,PrintSplice

 * TODO
 *    the library should be able to deal with embedded \0's. Really.
*/



/*  **************************************************************************
 * *
 **   Various instantiation routines.
*/

                          /*     Accepts NULL argument.
                           *     void arguments so that it can be used
                           *     as callback.
                           *
                           *     Should ordinarily *not* be used, unless
                           *     you want to initialize a ting allocated
                           *     on the frame stack.
                          */
void* mcxTingInit
(  void* ting
)  ;  
                          /*     Accepts NULL argument.
                           *     Should ordinarily *not* be used.
                          */
mcxTing* mcxTingInstantiate
(  mcxTing*    dst_ting
,  const char* str
)  ;  
                          /*     Accepts NULL argument.  Does not affect
                           *     ting->str.  used for preallocing, e.g. to
                           *     prepare for successive calls of Append.  (see
                           *     also Empty).
                          */
mcxTing* mcxTingEnsure
(  mcxTing*    ting
,  dim         length
)  ;  
                          /*     Accepts NULL argument. The string part is
                           *     set to the empty string.
                           *     Can be used for preallocing.
                          */
mcxTing* mcxTingEmpty
(  mcxTing*    ting
,  dim         length
)  ;



/*  **************************************************************************
 * *
 **   Some freeing routines.
*/
                          /*     Free members and shell struct
                           *     You pass the address of a variable,
                           *     loosely speaking.
                          */
void mcxTingFree
(  mcxTing     **tingpp
)  ;
                          /*     Free members and shell struct
                           *     Use with callbacks, e.g.
                           *     for freeing hash with tings in one go.
                          */
void mcxTingFree_v
(  void        *tingpp
)  ;
                          /*     Free members
                           *     Use for freeing array of ting;
                           *     e.g. as callback in mcxNFree.
                          */
void mcxTingRelease
(  void        *ting
)  ;



/*  **************************************************************************
 * *
 **   A bunch of user-land creation routines.
*/

                          /*     accepts NULL argument, maps to empty string.
                          */
mcxTing* mcxTingNew
(  const char* str
)  ;
                          /*     accepts NULL argument, maps to empty string.
                          */
mcxTing* mcxTingNNew
(  const char* str
,  dim         n
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingWrite
(  mcxTing*    ting
,  const char* str
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingNWrite
(  mcxTing*    ting
,  const char* str
,  dim         n
)  ;


                          /*    usurps argument.
                          */
mcxTing* mcxTingify
(  char* str
)  ;


                          /*    destroys argument, returns str member.
                          */
char* mcxTinguish
(  mcxTing*    ting
)  ;


/*  **************************************************************************
 * *
 **   Appending, inserting, deleting, shrinking.
 *    Some can be used as creation routine (e.g. append).
*/


#define TING_INS_CENTER      -3
#define TING_INS_OVERRUN     -4
#define TING_INS_OVERWRITE   -5

/*
 *    The offset argument can be negative, for subscripting from the end.
 *
 *    The n_delete argument can be nonnegative, or one of
 *    TING_INS_CENTER     center, (try to) overwrite without changing length.
 *    TING_INS_OVERRUN    overwrite till the end.
 *    TING_INS_OVERWRITE  (try to) overwrite without changing length.
 *
 *    The n_copy argument is not magical like the previous two.
 *
*/
                          /*     does NOT accept NULL argument.
                          */
mcxstatus mcxTingSplice
(  mcxTing*    ting
,  const char* pstr
,  ofs         d_offset   /*     negative offset refers to end     */
,  ofs         n_delete   /*     special modes as documented above */
,  dim         n_copy
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingInsert
(  mcxTing*    ting
,  const char* str
,  ofs         offset
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingNInsert
(  mcxTing*    ting
,  const char* str
,  ofs         offset     /*  of ting->str */
,  dim         length     /*  of str       */
)  ;

                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingAppend
(  mcxTing*    ting
,  const char* str
)  ;

                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingKAppend
(  mcxTing*    ting
,  const char* str
,  dim         n
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingNAppend
(  mcxTing*    ting
,  const char* str
,  dim         n
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingDelete
(  mcxTing*    ting
,  ofs         offset
,  dim         width
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing* mcxTingShrink
(  mcxTing*    ting
,  ofs         length
)  ;



/* Fails only for memory reason */

mcxstatus mcxTingTackc
(  mcxTing*  ting
,  unsigned char c
)  ;


/* Fails if last char is not the same as c */

mcxstatus mcxTingTickc
(  mcxTing*  ting
,  unsigned char c
)  ;


/*  **************************************************************************
 * *
 **   A bunch of printf like routines.
*/

                          /*     Accepts NULL argument.
                          */
mcxTing* mcxTingPrint
(  mcxTing*    ting
,  const char* fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;
                          /*     Accepts NULL argument.
                          */
mcxTing*  mcxTingPrintAfter
(  mcxTing*    dst
,  const char* fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;
                          /*     Accepts NULL argument.
                           *     same offset and delete interface as
                           *     mcxTingSplice.
                          */
mcxTing*  mcxTingPrintSplice
(  mcxTing*    dst
,  ofs         offset
,  ofs         n_delete   /* count of chars to delete, special modes */
,  const char* fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 4, 5)))
#endif
   ;


/*  **************************************************************************
 * *
 **   Miscellaneous.
*/

char*  mcxTingStr
(  const mcxTing* ting
)  ;


char* mcxTingSubStr
(  const mcxTing* ting
,  ofs            offset
,  ofs            length  /*     use -1 to indicate remainder of string */
)  ;

                          /*     accepts NULL argument.
                          */
mcxTing*  mcxTingRoman
(  mcxTing*    dst
,  long        x
,  mcxbool     ucase
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing*  mcxTingInteger
(  mcxTing*    dst
,  long        x
)  ;
                          /*     accepts NULL argument.
                          */
mcxTing*  mcxTingDouble
(  mcxTing* dst
,  double   x
,  int      decimals
)  ;


/*  **************************************************************************
 * *
 **   Comparing.
*/

                  /* compare two mcxTing* pointers */
int mcxTingCmp
(  const void* t1
,  const void* t2
)  ;

                  /* compare two mcxTing* pointers */
int mcxTingRevCmp
(  const void* t1
,  const void* t2
)  ;

                  /* compare two mcxTing** pointers */
int mcxTingPCmp
(  const void* t1
,  const void* t2
)  ;

                  /* compare two mcxTing** pointers */
int mcxTingPRevCmp
(  const void* t1
,  const void* t2
)  ;

                  /* compare two mcxKV** pointers by key as mcxTing* */
int mcxPKeyTingCmp
(  const void* k1
,  const void* k2
)  ;

                  /* compare two mcxKV** pointers by key as mcxTing* */
int mcxPKeyTingRevCmp
(  const void* k1
,  const void* k2
)  ;


/*  **************************************************************************
 * *
 **   Hashing.
*/

u32 (*mcxTingHFieByName(const char* id))(const void* ting)  ;


u32 mcxTingELFhash
(  const void* ting
)  ;

u32 mcxTingHash
(  const void* ting
)  ;

u32 mcxTingBJhash
(  const void* ting
)  ;

u32 mcxTingCThash
(  const void* ting
)  ;

u32 mcxTingDPhash
(  const void* ting
)  ;

u32 mcxTingBDBhash
(  const void* ting
)  ;

u32 mcxTingGEhash
(  const void* ting
)  ;

u32 mcxTingOAThash
(  const void* ting
)  ;

u32 mcxTingFNVhash
(  const void* ting
)  ;

u32 mcxTingSvDhash
(  const void* ting
)  ;

u32 mcxTingSvD2hash
(  const void* ting
)  ;

u32 mcxTingSvD1hash
(  const void* ting
)  ;

u32 mcxTingDJBhash
(  const void* ting
)  ;

#endif

