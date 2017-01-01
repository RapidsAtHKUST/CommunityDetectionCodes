/*   (C) Copyright 2004, 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#include "list.h"
#include "alloc.h"
#include "gralloc.h"
#include "compile.h"

#define DEBUG 0

/* TODO. very funny interface. in flux.
 *
 * !  allow null argument for linkdelete.
 *
 * !
 *    little support for corruption checking.
 *    is the spawn thing a good idea ?
 * !
 *    interface with list object, or not?
 *    not requires hidden list pointer everywhere (have that now).
 *    perhaps supply separate interface with list handle .. ?]
 * !
 *    how to unify FLink, BLink, and link (forward/backward/bidi)
 *    optify hidden list pointer, next xor prev pointer.
 *
 * -  Add triple sanity checks for user supplied data (?)
 * -  implement val freeing.
 * -  with deleting, should warn if no handle remains ..
 *    and can check grim count.
*/


/* Note.
 *    We very much do not want to define (as previously thought) next and prev
 *    as const pointers (with the goal of keeping chain consistency up
 *    to this interface).
 *    This interface provides some convenience interfaces, but
 *    the integrity and life-cycle of the chains built with the links
 *    is entirely up to the caller. So tie-rap away with them links.
 *    
 *    So we do not keep track of list characteristics,
 *    e.g. first, last, count. That would be something for a higher layer.
*/


typedef struct
{  mcxGrim*       grim
;
}  mcx_list       ;


typedef struct lsptr
{  mcx_list*      ls
;
}  lsptr          ;


#define mcx_list_find(lk) ((lsptr*)((void*)((char*) lk - sizeof(lsptr))))->ls


void mcx_link_init
(  mcxLink* lk
,  void* val
)
   {  lk->next =  NULL
   ;  lk->prev =  NULL
   ;  lk->val  =  val
;  }


mcxLink* mcx_list_shift
(  mcx_list*   ls
,  void*       val
)
   {  void* mem
   ;  mcxLink* lk

   ;  if (!(mem = mcxGrimGet(ls->grim)))
      return NULL

   ;  lk =  (void*) ((char*) mem + sizeof(lsptr))
   ;  ((lsptr*) mem)->ls = ls

   ;  mcx_link_init(lk, val)
   ;  return lk
;  }


mcxLink*  mcxListSource
(  dim  capacity_start
,  mcxbits  options
)
   {  mcx_list*   ls

   ;  if (!(ls = mcxAlloc(sizeof(mcx_list), RETURN_ON_FAIL)))
      return NULL

#if DEBUG
;  fprintf
   (stderr, "new list ptr <%p> capacity <%ld>\n", (void*) ls, (long) capacity_start)
#endif

   ;  if
      (! (ls->grim
      =  mcxGrimNew(sizeof(lsptr) + sizeof(mcxLink), capacity_start, options)
         )
      )
      return NULL

   ;  return mcx_list_shift(ls, NULL)
;  }


mcxLink*  mcxLinkSpawn
(  mcxLink* lk
,  void* val
)
   {  mcx_list* ls  =  mcx_list_find(lk)
   ;  return mcx_list_shift(ls, val)
;  }


void mcxLinkClose
(  mcxLink* left
,  mcxLink* right
)
   {  if (left)
      left->next = right
   ;  if (right)
      right->prev = left
;  }


mcxLink*  mcxLinkBefore
(  mcxLink*    next
,  void*       val
)
   {  mcx_list* ls  =  mcx_list_find(next)
   ;  mcxLink* new  =  mcx_list_shift(ls, val)

   ;  if (!new)
      return NULL

   ;  new->next   =  next

   ;  new->prev   =  next->prev
   ;  next->prev  =  new

   ;  if (new->prev)
      new->prev->next = new

   ;  return new
;  }


mcxLink*  mcxLinkAfter
(  mcxLink*    prev
,  void*       val
)
   {  mcx_list* ls  =  mcx_list_find(prev)
   ;  mcxLink* new
#if DEBUG
;  fprintf(stderr, "list ptr <%p>\n", (void*) ls)
#endif
   ;  new = mcx_list_shift(ls, val)

   ;  if (!new)
      return NULL

   ;  new->prev   =  prev

   ;  new->next   =  prev->next
   ;  prev->next  =  new

   ;  if (new->next)
      new->next->prev = new

   ;  return new
;  }


void mcxLinkRemove
(  mcxLink*    lk
)
   {  mcx_list* ls   =  mcx_list_find(lk)
   ;  mcxGrimLet(ls->grim, ((char*) lk) - sizeof(lsptr))
;  }


mcxLink* mcxLinkDelete
(  mcxLink*    lk
)
   {  mcx_list* ls   =  mcx_list_find(lk)
   ;  mcxLink*  prev =  lk->prev
   ;  mcxLink*  next =  lk->next
   ;  if (prev)
      prev->next = next
   ;  if (next)
      next->prev = prev
   ;  mcxGrimLet(ls->grim, ((char*) lk) - sizeof(lsptr))
   ;  return lk
;  }


mcxGrim* mcxLinkGrim
(  mcxLink* lk
)
   {  mcx_list* ls = mcx_list_find(lk)
   ;  return ls->grim
;  }


void  mcxListFree
(  mcxLink**   lkp
,  void        freeval(void* valpp) cpl__unused  /* (yourtype1** valpp)     */
)
   {  if (*lkp)
      {  mcx_list* ls = mcx_list_find(*lkp)
      ;  mcxGrimFree(&(ls->grim))
      ;  mcxFree(ls)
      ;  *lkp = NULL
   ;  }
;  }


#undef DEBUG

