/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "stack.h"
#include "util.h"

#include "util/ting.h"
#include "util/alloc.h"


zscard_p   stacktop_g    =  NULL;

typedef struct zscard_t
{  struct zscard_t*  prev
;  zgglob_p          glob
;
}  zscard_t          ;


zscard_p zsNew
(  zscard_p     prev
,  zgglob_p     glob
)
   {  zscard_p new      =  mcxAlloc(sizeof(zscard_t), EXIT_ON_FAIL)
   ;  new->prev         =  prev
   ;  new->glob         =  glob
   ;  return new
;  }


zgglob_p zsGetGlob
(  int         depth
)
   {  zscard_p   card   =  zsGetCard(depth)
   ;  return card ? card->glob : NULL
;  }


int zsHaveNargs
(  int n
)
   {  return zsGetCard(n-1) ? 1 : 0
;  }


int zsDoSequence
(  char* seq
)
   {  mcxTing* seqting  =  mcxTingNew(seq)
   ;  mcxTing* argtxt   =  mcxTingEmpty(NULL, 5)
   ;  int ok            =  1
   ;  char* p           =  seqting->str

   ;  while(p && *p)
      {
         while(isspace(*p) && *++p)
      ;  
         if (*p)
         {  char *q = strpbrk(p, " \t\n") 
         ;  if (q)
            *q = '\0'
         ;  /* else end of string and we are ok */

            mcxTingWrite(argtxt, p)
         ;  if (!zgUser(argtxt->str))
            {  zsList(0)
            ;  ok = 0
            ;  goto finish
         ;  }
         ;  p  = q ? q+1 : NULL
      ;  }
      }

   finish
   :
      mcxTingFree(&argtxt)
   ;  mcxTingFree(&seqting)
   ;  return ok
;  }


int zsEmpty
(  void
)
   {  return !stacktop_g ? 1 : 0
;  }


int zsList
(  int n
)
   {  zscard_p card     =  stacktop_g

   ;  if (!n && card)
      {  fprintf(stderr, "/// stack listing ///\n")
      ;  n--
   ;  }

      for (card=stacktop_g; n-- && card!=NULL; card=card->prev)
      zgInfo(card->glob)

   ;  return 1
;  }


int zsShift
(  int n
,  int time
)
   {  zscard_p  left  =  zsGetCard(n)
   ;  zscard_p  rite  =  stacktop_g     /* yeh, rite stinks as a name */

   ;  if (!left)
      return 0

   ;  if (time > 0)
      {  stacktop_g  =  stacktop_g->prev
      ;  rite->prev  =  left->prev
      ;  left->prev  =  rite
   ;  }
      else
      {  zscard_p  newleft  =  zsGetCard(n-1) 
      ;  if (!newleft)
         {  zmTell('e', "[zsShift] frankly, I am bewildered: No newleft!")
         ;  return 0
      ;  }
      ;  newleft->prev     =  left->prev
      ;  stacktop_g        =  left
      ;  stacktop_g->prev  =  rite
   ;  }
   ;  return 1
;  }


int zsRoll
(  int n
,  int j
)
   {  int time =  j > 0 ? 1 : -1

   ;  j *= time

   ;  while (j--)
      {  if (!zsShift(n, time))
         return 0
   ;  }
      return 1
;  }


int zsPop
(  void
)
   {  zscard_p  card     =  stacktop_g

   ;  if (!card)
      {  zmTell('e', "stack underflow")
      ;  return 0
   ;  }

   ;  stacktop_g        =  stacktop_g->prev
   ;  zsFree(&card)
   ;  return 1
;  }


int zsClear
(  void
)
   {  zscard_p card     =  stacktop_g

   ;  while(stacktop_g)
      {  stacktop_g = card->prev
      ;  zsFree(&card)
      ;  card = stacktop_g  
   ;  }
   ;  return 1
;  }


int zsExch
(  void
)
   {  zscard_p c3  =  zsGetCard(0)
   ;  zscard_p c2  =  zsGetCard(1)
   ;  zscard_p c1

   ;  if (!c2 || !c3)
      return 0

   ;  c1          =  c2->prev  /* zsGetCard(2) fails at NULL */

   ;  c3->prev    =  c1
   ;  c2->prev    =  c3
   ;  stacktop_g  =  c2
   ;  return 1
;  }


zscard_p zsGetCard
(  int depth
)
   {  zscard_p   card    =  stacktop_g

   ;  while (card && depth--)
      card = card->prev

   ;  if (!card)
      {  zmTell('e', "stack underflow")
      ;  return NULL
   ;  }

   ;  return card
;  }


int zsPush
(  zgglob_p   glob
)
   {  stacktop_g    =  zsNew(stacktop_g, glob)
   ;  return 1
;  }


void zsFree
(  zscard_p*   cardpp
)
   {  zscard_p card = *cardpp
   ;  if (card)
      {
         if (card->glob)
         zgFree(&(card->glob))
   ;  }
   ;  mcxFree(card)
   ;  *cardpp = NULL
;  }


