/*   (C) Copyright 2004, 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_list_h
#define tingea_list_h

#include "gralloc.h"
#include "types.h"

/* TODO:
 *    -  linkDelete  linkRemove semantics interface documentation.
 *    -  make prev xor next link optional.
 *    -  make hidden pointer optional.
 *    -  provide interface that uses list struct.
 *    -  convenience interface for tying two chains together.
 *    -  list-to-array interface.
*/


/*
 * History.
 *    The data structure used is similar to that by Jan van der Steen's pool.c
 *    code, which used to be part of this library. So kudos to Jan.  The
 *    present implementation, which is a bit different, was not directly copied
 *    nor modified. It descended from a botched linked list implementation
 *    having it's own buffered storage. Only at that time I realized that the
 *    right solution for lists and hashes is to have a private pool of gridmem,
 *    rather than deriving it from a global pool (so that we still can defer
 *    thread-safety to malloc). Right?
 *    The present implementation was then derived from the botched linked list
 *    and sanitized afterwards. The linked list.c is still slightly botched.
*/


/* Description
 *    This provides a doubly linked list/link interface with buffered storage
 *    (invisible to the user).  The current interface operates entirely via
 *    links, the commanding structure is hidden. Other interface types may
 *    arise later.
 *   
 *    You can create a number of chains drawing memory from the same pool.
 *   
 *    It only provides basic link insertions and deletions as a convenience
 *    interface, and does not maintain consistency checks, neither on links,
 *    nor on storage.
*/


typedef struct mcxLink
{  struct mcxLink*   next
;  struct mcxLink*   prev
;  void*             val
;
}  mcxLink                 ;


/* Options:
 * same as for mcxGrimNew
*/

mcxLink*  mcxListSource
(  dim      capacity_start
,  mcxbits  options
)  ;

/* 
 * This removes all links that have the same parent link as lk. [huh?]
 * BEWARE freeval doesn't do anything yet
*/

void  mcxListFree
(  mcxLink**   lk
,  void        freeval(void* valpp)    /* (yourtype1** valpp)     */
)  ;



/* Creates new chain, that can later be tied to other chains.
*/

mcxLink*  mcxLinkSpawn
(  mcxLink* lk
,  void* val
)  ;


/* 
 * This inspects prev and next and links them if possible.
 *
 * You can use the val pointer, immediately after deleting.
 * That makes it unsafe in threads (but you need locking anyway).
 * The feature is used in the hash library.
*/

mcxLink*  mcxLinkDelete
(  mcxLink*    lk
)  ;


/*
 * This just deallocates the link.
*/

void mcxLinkRemove
(  mcxLink*    lk
)  ;


mcxLink*  mcxLinkAfter
(  mcxLink*    prev
,  void*       val
)  ;

mcxLink*  mcxLinkBefore
(  mcxLink*    prev
,  void*       val
)  ;


void  mcxLinkClose
(  mcxLink*    left
,  mcxLink*    right
)  ;


/*
 * Get the grim that serves this list
*/

mcxGrim* mcxLinkGrim
(  mcxLink* lk
)  ;


#endif

