/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/


/* TODO (?)
 *
 * integer overflow handling.
 *
 * better bucket fill statistics.
 *
 * nested hashes.
 *    could lump
 *       options  cmp  hash  src_link
 *    into a structure and make hashes share them.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "hash.h"
#include "minmax.h"
#include "types.h"
#include "inttypes.h"
#include "ting.h"
#include "err.h"
#include "alloc.h"
#include "gralloc.h"
#include "compile.h"
#include "list.h"


            /* the distribution of bit counts over all 32-bit keys */
int promilles[32] = 
{  0,  0,  0,  0,  0,  0,  0,  0
,  2,  7, 15, 30, 53, 81,110,131
,140,131,110, 81, 53, 30, 15,  7
,  2,  0,  0,  0,  0,  0,  0,  0
}  ;


#ifndef TINGEA_HASH_CACHE
#  define   TINGEA_HASH_CACHE 0
#endif


typedef struct hash_link
{  struct hash_link* next
;  mcxKV             kv
#if TINGEA_HASH_CACHE
;  u32               hv
#endif
;
}  hash_link         ;


typedef struct bucket
{  hash_link*        base
;
}  mcx_bucket        ;


               /* For further optimization work, options
                *    options
                *    load
                *    cmp
                *    hash
                *    src_link
                * can be shared between different hashes (e.g.
                * with multidimensional hashes). Consider
                * making src_link file static variable.
               */

struct mcxHash
{  dim         n_buckets      /* 2^n_bits          */
;  mcx_bucket *buckets
;  dim         n_entries

;  mcxbits     options

;  int         (*cmp) (const void *a, const void *b)
;  u32         (*hash) (const void *a)

;  mcxGrim*    src_link

;  float       load
;
}  ;


struct mcxHashWalk
{  mcxHash*    hash
;  dim         i_bucket
;  hash_link*  link
;
}  ;


void* mcx_bucket_init
(  void*  buck
)
   {  ((mcx_bucket*) buck)->base = NULL
   ;  return NULL
;  }

void bitprint
(  u32   key
,  FILE* fp
)  ;

int bitcount
(  u32   key
)  ;


mcxHash* mcxHashNew
(  dim         n_buckets
,  u32         (*hash)(const void *a)
,  int         (*cmp) (const void *a, const void *b)
)
   {  mcxHash  *h
   ;  mcxbool ok  = FALSE
      
   ;  u8 n_bits   =  0

   ;  if (!n_buckets)
      {  mcxErr("mcxHashNew strange", "void alloc request")
      ;  n_buckets = 2
   ;  }

      if (!(h = mcxAlloc(sizeof(mcxHash), RETURN_ON_FAIL)))
      return NULL

   ;  while(n_buckets)
      {  n_buckets >>=  1
      ;  n_bits++
   ;  }

      h->load           =  0.5
   ;  h->n_entries      =  0
   ;  h->n_buckets      =  n_buckets = (1 << n_bits)
   ;  h->cmp            =  cmp
   ;  h->hash           =  hash
   ;  h->options        =  MCX_HASH_OPT_DEFAULTS

   ;  h->src_link       =  NULL

   ;  while (1)                        /* fixme 2nd arg below, have better choice? */
      {  h->src_link = mcxGrimNew(sizeof(hash_link), h->n_buckets, MCX_GRIM_ARITHMETIC)
      ;  if (!h->src_link)
         break

      ;  if
         (! (  h->buckets
            =  mcxNAlloc
               (  h->n_buckets
               ,  sizeof(mcx_bucket)
               ,  mcx_bucket_init
               ,  RETURN_ON_FAIL
         )  )  )
         break

      ;  ok = TRUE
      ;  break
   ;  }

      if (!ok)
      {  mcxGrimFree(&(h->src_link))
      ;  mcxFree(h)
      ;  return NULL
   ;  }

      return h
;  }


dim mcxHashMemSize
(  mcxHash*    hash
)
   {  return
         mcxGrimMemSize(hash->src_link)
      +  sizeof(mcx_bucket) * hash->n_buckets
;  }


void mcxHashGetSettings
(  mcxHash*          hash
,  mcxHashSettings*  settings
)
   {  settings->n_buckets  =  hash->n_buckets
   ;  settings->load       =  hash->load
   ;  settings->n_entries  =  hash->n_entries
   ;  settings->options    =  hash->options
;  }


static dim hash_link_size
(  hash_link* link
)
   {  dim s =  0
   ;  while(link)
         link = link->next
      ,  s++
   ;  return(s)
;  }


void mcxHashStats
(  FILE*       fp
,  mcxHash*    h
)
   {  dim      buckets  =  h->n_buckets
   ;  dim      buckets_used   =  0
   ;  float    ctr      =  0.0
   ;  float    cb       =  0.0
   ;  dim      max      =  0
   ;  dim      entries  =  0
   ;  const    char* me =  "mcxHashStats"

   ;  int      j, k, distr[32]
   ;  mcx_bucket  *buck

   ;  for (j=0;j<32;j++)
      distr[j] = 0

   ;  for (buck=h->buckets; buck<h->buckets + h->n_buckets; buck++)
      {  dim   d        =  hash_link_size(buck->base)
      ;  hash_link* this=  buck->base

      ;  if (d)
         {  buckets_used++
         ;  entries    +=  d
         ;  ctr        +=  (float) d * d
         ;  cb         +=  (float) d * d * d
         ;  max         =  MCX_MAX(max, d)
      ;  }

         while(this)
         {  u32   u     =  (h->hash)(this->kv.key)
         ;  int   ct    =  bitcount(u)
         ;  this        =  this->next
         ;  distr[ct]++
;if (0) fprintf(stderr, "bucket [%d] key [%s]\n", (int)d,  ((mcxTing*) this->kv.key)->str)
      ;  }
      }

      ctr = ctr / MCX_MAX(1, entries)
   ;  cb  = sqrt(cb  / MCX_MAX(1, entries))

   ;  if (buckets && buckets_used)
         mcxTellf
         (  fp
         ,  me
         ,  "%4.2f bucket usage (%ld available, %ld used, %ld entries)"
         ,  (double) ((double) buckets_used) / buckets
         ,  (long) buckets
         ,  (long) buckets_used
         ,  (long) entries
         )
      ,  mcxTellf
         (  fp
         ,  me
         ,  "bucket average: %.2f, center: %.2f, cube: %.2f, max: %ld"
         ,  (double) entries / ((double) buckets_used)
         ,  (double) ctr
         ,  (double) cb
         ,  (long) max
         )

   ;  mcxTellf(fp, me, "bit distribution (promilles):")
   ;  fprintf
      (  fp
      ,  "  %-37s   %s\n"
      ,  "Current bit distribution"
      ,  "Ideally random distribution"
      )
   ;  for (k=0;k<4;k++)
      {  for (j=k*8;j<(k+1)*8;j++)
         fprintf(fp, "%3.0f ",  entries ? (1000 * (float)distr[j]) / entries : 0.0)
      ;  fprintf(fp, "        ");
      ;  for (j=k*8;j<(k+1)*8;j++)
         fprintf(fp, "%3d ",  promilles[j])
      ;  fprintf(fp, "\n")
   ;  }
      mcxTellf(fp, me, "link count: %ld", (long) (mcxGrimCount(h->src_link)))
   ;  mcxTellf(fp, me, "link mem count: %ld", (long) (mcxGrimMemSize(h->src_link)))

   ;  mcxTellf(fp, me, "done")
;  }


void mcxHashSetOpts
(  mcxHash*    h
,  double      load
,  int         options
)
   {  if (options >= 0)
      h->options |= options
  /* fixme; are there states in which either of these can be corrupting ? */
   ;  h->load  =  load
;  }


void mcxHashFreeScalar
(  void* scalar cpl__unused
)
   {  /* this triggers freeing of kv.key or kv.val */
;  }


void mcxHashFree
(  mcxHash**   hpp
,  void        freekey(void* key)
,  void        freeval(void* key)
)
   {  mcxHash* h        =  *hpp
   ;  mcx_bucket* buck  =  h ? h->buckets   : NULL
   ;  dim d             =  h ? h->n_buckets : 0

   ;  if (!h)
      return

   ;  if (freekey || freeval)
      {  while (d-- > 0)      /* careful with unsignedness */
         {  hash_link* link = (buck++)->base

         ;  while(link)
            {  void* key = link->kv.key
            ;  void* val = link->kv.val
            ;  if (freekey && key)
                  freekey(key)
               ,  mcxFree(key)
            ;  if (freeval && val)
                  freeval(val)
               ,  mcxFree(val)
            ;  link = link->next
         ;  }
         }
      }

      mcxGrimFree(&h->src_link)
   ;  mcxFree(h->buckets)
   ;  mcxFree(h)
   ;  *hpp = NULL
;  }


#define MCX_HASH_DOUBLING MCX_HASH_OPT_UNUSED


#if TINGEA_HASH_CACHE

static hash_link* mcx_bucket_search
(  mcxHash*          h
,  void*             ob
,  mcxmode           ACTION
,  u32*              hashval
)
   {  u32   thishash    =  hashval ? *hashval : (h->hash)(ob)
   ;  mcx_bucket *buck  =  h->buckets + (thishash & (h->n_buckets-1))
   ;  hash_link* link   =  buck->base, *prev = NULL, *new
   ;  int delta         =  0

   ;  while
      (   link
      &&  (  link->hv != thishash
          || h->cmp(ob, link->kv.key)
          )
      ) 
         prev = link
      ,  link = link->next

   ;  if (link && ACTION == MCX_DATUM_DELETE)
      {  if (buck->base == link)
         buck->base = link->next
      ;  else
         prev->next = link->next
      ;  delta = -1
      ;  mcxGrimLet(h->src_link, link)
               /* we return link below though */
   ;  }
      else if (!link)
      {  if (ACTION == MCX_DATUM_FIND || ACTION == MCX_DATUM_DELETE)
         link = NULL

      ;  else if (ACTION == MCX_DATUM_INSERT)
         {  new = mcxGrimGet(h->src_link)            /* fixme could be NULL */
         ;  new->next   = NULL
         ;  new->kv.val = NULL
         ;  new->kv.key = ob
         ;  new->hv     =  thishash

         ;  if (!buck->base)
            buck->base = new
                              /* in TINGEA_HASH_CACHE case we always append */
         ;  else
               new->next  = prev->next
            ,  prev->next = new
         ;  delta = 1
         ;  link = new
      ;  }
      }

      h->n_entries += delta
   ;  return link
;  }

#else

static hash_link* mcx_bucket_search
(  mcxHash*          h
,  void*             ob
,  mcxmode           ACTION
,  u32*              hashval
)
   {  u32   thishash    =  hashval ? *hashval : (h->hash)(ob)
   ;  mcx_bucket *buck  =  h->buckets + (thishash & (h->n_buckets-1))
   ;  hash_link* link   =  buck->base, *prev = NULL, *new
   ;  int c             =  1
   ;  int delta         =  0

   ;  while
      (   link
      &&  (c = h->cmp(ob, link->kv.key)) > 0
      ) 
         prev = link
      ,  link = link->next

   ;  if (!c && ACTION == MCX_DATUM_DELETE)
      {  if (buck->base == link)
         buck->base = link->next
      ;  else
         prev->next = link->next
      ;  delta = -1
      ;  mcxGrimLet(h->src_link, link)
               /* we return link below though */
   ;  }
      else if (!link || c < 0)
      {  if (ACTION == MCX_DATUM_FIND || ACTION == MCX_DATUM_DELETE)
         link = NULL

      ;  else if (ACTION == MCX_DATUM_INSERT)
         {  new = mcxGrimGet(h->src_link)          /* fixme could be NULL */
         ;  new->next   = NULL
         ;  new->kv.val = NULL
         ;  new->kv.key = ob

         ;  if (!buck->base)
            buck->base = new
         ;  else if (link == buck->base)
               new->next = buck->base
            ,  buck->base = new
         ;  else
               new->next  = prev->next
            ,  prev->next = new
         ;  delta = 1
         ;  link = new
      ;  }
      }

      h->n_entries += delta
   ;  return link
;  }

#endif


static mcxstatus mcx_hash_double
(  mcxHash* h
)  ;


mcxKV* mcxHashSearchx
(  void*       key
,  mcxHash*    h
,  mcxmode     ACTION
,  int*        delta
)
   {  hash_link *link
   ;  dim n_entries = h->n_entries

   ;  if
      (  h->load * h->n_buckets < h->n_entries
      && !(h->options & (MCX_HASH_OPT_CONSTANT | MCX_HASH_DOUBLING))
      && mcx_hash_double(h)
      )
      mcxErr("mcxHashSearch", "cannot double hash")

   ;  link = mcx_bucket_search(h, key, ACTION, NULL)

   ;  if (delta)
      *delta = h->n_entries < n_entries ? -1 : (int) (h->n_entries - n_entries)

   ;  return link ? &link->kv : NULL
;  }


enum
{  ARRAY_OF_KEY
,  ARRAY_OF_KV
}  ;


void mcxHashApply
(  mcxHash* hash
,  void    (*cb)(const void* key, void* val, void* data)
,  void*    data
)
   {  mcxHashWalk* walk = mcxHashWalkInit(hash)
   ;  mcxKV* kv
   ;  dim i_bucket
   ;  while ((kv = mcxHashWalkStep(walk, &i_bucket)))
      cb(kv->key, kv->val, data)
   ;  mcxHashWalkFree(&walk)
;  }


static void** hash_array
(  mcxHash*    hash
,  dim*        n_entries
,  int       (*cmp)(const void*, const void*)
,  mcxbits     opts     cpl__unused
,  mcxenum     mode
)
   {  void** obs   =  mcxAlloc(sizeof(void*) * hash->n_entries, RETURN_ON_FAIL)
   ;  dim d = 0
   ;  mcxKV* kv
   ;  const char* me = mode == ARRAY_OF_KEY ? "mcxHashKeys" : "mcxHashKVs"

   ;  mcxHashWalk* walk = mcxHashWalkInit(hash)

   ;  if (!walk || !obs)
      return NULL

   ;  while ((kv = mcxHashWalkStep(walk, NULL)))   /* fixme extract */
      {  if (d >= hash->n_entries)
         {  mcxErr
            (  me
            ,  "PANIC inconsistent state (n_entries %ld)"
            ,  (long) hash->n_entries
            )
         ;  break
      ;  }
         obs[d] = mode == ARRAY_OF_KEY ? kv->key : kv
      ;  d++
   ;  }
      if (d != hash->n_entries)
      mcxErr(me, "PANIC inconsistent state (n_entries %lu)", (ulong) hash->n_entries)

   ;  if (cmp)
      qsort(obs, d, sizeof(void*), cmp)
   ;  mcxHashWalkFree(&walk)

   ;  *n_entries = d
   ;  return obs
;  }



void** mcxHashKeys
(  mcxHash*    hash
,  dim*        n_entries
,  int       (*cmp)(const void*, const void*)
,  mcxbits     opts                    /* unused yet */
)
   {  return hash_array(hash, n_entries, cmp, opts, ARRAY_OF_KEY)
;  }



void** mcxHashKVs
(  mcxHash*    hash
,  dim*        n_entries
,  int       (*cmp)(const void*, const void*)
,  mcxbits     opts                    /* unused yet */
)
   {  return hash_array(hash, n_entries, cmp, opts, ARRAY_OF_KV)
;  }



mcxKV* mcxHashWalkStep
(  mcxHashWalk  *walk
,  dim          *i_bucket
)
   {  hash_link* step  =  walk->link
   
   ;  while (!step && ++walk->i_bucket < walk->hash->n_buckets)
      step = (walk->hash->buckets+walk->i_bucket)->base

   ;  if (step)
      {  walk->link = step->next
      ;  if (i_bucket)
         *i_bucket = walk->i_bucket
      ;  return &step->kv
   ;  }
      return NULL
;  }


mcxHashWalk* mcxHashWalkInit
(  mcxHash  *h
)
   {  mcxHashWalk* walk = mcxAlloc(sizeof *walk, RETURN_ON_FAIL)
   ;  if (!walk)
      return NULL
      
   ;  walk->hash =  h

   ;  if (!h || !h->buckets)
      {  mcxFree(walk)
      ;  return NULL
   ;  }

      walk->i_bucket =  0
   ;  walk->link     =  (h->buckets+0)->base
   ;  return walk
;  }


void mcxHashWalkFree
(  mcxHashWalk  **walkpp
)
   {  mcxFree(*walkpp)
   ;  *walkpp =  NULL
;  }


mcxHash* mcxHashMerge
(  mcxHash*    h1
,  mcxHash*    h2
,  mcxHash*    hd    /* hash destination */
,  void*       merge(void* val1, void* val2)
)
   {  mcxHash* ha[2] /* hash array */
   ;  mcxHash* h
   ;  int i

   ;  if (!h1 || !h2)
      mcxDie(1, "mcxHashMerge FATAL", "clone functionality not yet supported")

      /*
       * fixme/note I am comparing fie pointers here, is that ok?
      */
   ;  if (h1->hash != h2->hash || h1->cmp != h2->cmp)
      mcxErr("mcxHashMerge WARNING", "non matching hash or cmp fie")

   ;  if (merge)
      mcxErr("mcxHashMerge WARNING", "merge functionality not yet supported")

   ;  hd =  hd
            ?  hd
            :  mcxHashNew
               (  h1->n_entries + h2->n_entries
               ,  h1->hash
               ,  h1->cmp
               )

   ;  if (!hd)
      return NULL

   ;  ha[0] = h1
   ;  ha[1] = h2

   ;  for (i=0;i<2;i++)
      {  h = ha[i]
      ;  if (h != hd)
         {  mcx_bucket* buck

         ;  for (buck = h->buckets; buck<h->buckets + h->n_buckets; buck++)
            {  hash_link* this =  buck->base

            ;  while(this)
               {  mcxKV* kv = mcxHashSearch(this->kv.key, hd, MCX_DATUM_INSERT)
               ;  if (!kv)
                  return NULL
/*  note/fixme: cannot free hd, don't have key/val free functions */
               ;  if (!kv->val)
                  kv->val = this->kv.val
               ;  this = this->next
            ;  }
            }
         }
      }

      return hd
;  }


static mcxstatus mcx_hash_double
(  mcxHash* h
)
   {  mcx_bucket* ole_bucket  =  h->buckets
   ;  mcx_bucket* ole_buckets =  h->buckets
   ;  dim d                   =  h->n_buckets
   ;  dim n_fail              =  0

   ;  if (h->options & MCX_HASH_DOUBLING)     /* called before */
      {  mcxErr("mcx_hash_double PANIC", "double trouble")
      ;  return STATUS_FAIL
   ;  }

      h->options |= MCX_HASH_DOUBLING

   ;  if
      (! (  h->buckets
         =  mcxNAlloc
            (  2 * h->n_buckets
            ,  sizeof(mcx_bucket)
            ,  mcx_bucket_init
            ,  RETURN_ON_FAIL
      )  )  )
      {  h->options ^= MCX_HASH_DOUBLING
      ;  h->buckets = ole_buckets
      ;  return STATUS_FAIL
   ;  }

      h->n_buckets  *=  2
   ;  h->n_entries   =  0

   ;  while(d-- > 0)    /* careful with unsignedness */
      {  hash_link* this   =  ole_bucket->base

      ;  while(this)
         {  hash_link* next = this->next, *clone
         ;  void* val   =  this->kv.val
         ;  void* key   =  this->kv.key

         ;  mcxGrimLet(h->src_link, this)  /* will be used immediately */
#if TINGEA_HASH_CACHE
         ;  clone = mcx_bucket_search(h, key, MCX_DATUM_INSERT, &this->hv)
#else
         ;  clone = mcx_bucket_search(h, key, MCX_DATUM_INSERT, NULL)
#endif
         
         ;  if (clone)
            clone->kv.val = val
         ;  else
            n_fail++

         ;  this = next
      ;  }
         ole_bucket++
   ;  }

      if (n_fail)
      mcxErr
      (  "mcx_hash_double PANIC"
      ,  "<%ld> reinsertion failures in hash with <%ld> entries"
      ,  (long) n_fail
      ,  (long) h->n_entries
      )

   ;  mcxFree(ole_buckets)
   ;  h->options ^= MCX_HASH_DOUBLING
   ;  return STATUS_OK
;  }


#define BJmix(a,b,c)             \
{                                \
  a -= b; a -= c; a ^= (c>>13);  \
  b -= c; b -= a; b ^= (a<< 8);  \
  c -= a; c -= b; c ^= (b>>13);  \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16);  \
  c -= a; c -= b; c ^= (b>> 5);  \
  a -= b; a -= c; a ^= (c>> 3);  \
  b -= c; b -= a; b ^= (a<<10);  \
  c -= a; c -= b; c ^= (b>>15);  \
}


/*
 * Thomas Wang says Robert Jenkins says this is a good integer hash function:
 *unsigned int inthash(unsigned int key)
 *{
 *   key += (key << 12);
 *   key ^= (key >> 22);
 *   key += (key << 4);
 *   key ^= (key >> 9);
 *   key += (key << 10);
 *   key ^= (key >> 2);
 *   key += (key << 7);
 *   key ^= (key >> 12);
 *   return key;
 *}
*/

                        /* created by Bob Jenkins */
u32 mcxBJhash
(  register const void*    key
,  register u32            len
)
   {  register u32      a, b, c, l
   ;  const char* k =  key

   ;  l           =  len
   ;  a = b       =  0x9e3779b9u
   ;  c           =  0xabcdef01u

   ;  while (l >= 12)
      {
         a += k[0] + (k[1]<<8) + (k[2]<<16) + (k[3]<<24)
      ;  b += k[4] + (k[5]<<8) + (k[6]<<16) + (k[7]<<24)
      ;  c += k[8] + (k[9]<<8) + (k[10]<<16)+ (k[11]<<24)
      ;  BJmix(a,b,c)
      ;  k += 12
      ;  l -= 12
   ;  }

      c += len
   ;  switch(l)         /* all the case statements fall through */
      {
         case 11: c+= k[10]<<24
      ;  case 10: c+= k[9]<<16
      ;  case 9 : c+= k[8]<<8
                        /* the first byte of c is reserved for the length */
      ;  case 8 : b+= k[7]<<24
      ;  case 7 : b+= k[6]<<16
      ;  case 6 : b+= k[5]<<8
      ;  case 5 : b+= k[4]
      ;  case 4 : a+= k[3]<<24
      ;  case 3 : a+= k[2]<<16
      ;  case 2 : a+= k[1]<<8
      ;  case 1 : a+= k[0]
                        /* case 0: nothing left to add */
   ;  }

      BJmix(a,b,c)
   ;  return c
;  }


                        /* created by Chris Torek */
u32   mcxCThash
(  const void *key
,  u32        len
)
#define ctHASH4a   h = (h << 5) - h + *k++;
#define ctHASH4b   h = (h << 5) + h + *k++;
#define ctHASH4 ctHASH4b

   {   u32 h               =  0
   ;   const unsigned char *k    =  key

   ;   if (len > 0)
       {   unsigned loop = (len + 8 - 1) >> 3    /* loop >= 1 */
       ;   switch (len & (8 - 1))
           {
               case 0:
                  do
                  {        /* All fall through */
                             ctHASH4
                     case 7: ctHASH4
                     case 6: ctHASH4
                     case 5: ctHASH4
                     case 4: ctHASH4
                     case 3: ctHASH4
                     case 2: ctHASH4
                     case 1: ctHASH4
                  }
                  while (--loop)                  /* unsignedcmpok */
               ;
           }
       }
   ;  return h
;  }


/* All 3 hash fies below play on a similar theme.  Interesting: as long as only
 * << >> and ^ are used, a hash function does a partial homogeneous fill of all
 * 2^k different strings of length k built out of two distinct characters --
 * not all buckets need be used. E.g. for k=15, such a hash function might fill
 * 2^13 buckets with 4 entries each, or it might fill 2^10 buckets with 32
 * entries each.  This was observed, not proven.
*/

u32   mcxSvDhash
(  const void        *key
,  u32               len
)
   {  u32   h     =  0x7cabd53e /* 0x7cabd53e */
   ;  const char* k =  key

   ;  h           =  0x0180244a

   ;  while (len--)
      {  u32  g   =  *k
      ;  u32  gc  =  0xff ^ g
      ;  u32  hc  =  0xffffffffu

      ;  hc      ^=  h
      ;  h        =  (  (h << 2) +  h +  (h >> 3))
                  ^  ( (g << 25) + (gc << 18) + (g << 11) + (g << 5) + g )
      ;  k++
   ;  }

   ;  return h
;  }


                        /* created by me */
u32   mcxSvD2hash
(  const void        *key
,  u32               len
)
   {  u32   h     =  0x7cabd53e /* 0x7cabd53e */
   ;  const char* k     =  key

   ;  while (len--)
      {  u32  g   =  *k
      ;  u32  gc  =  0xff ^ g

      ;  h        =  (  (h << 3) ^ h ^ (h >> 5) )
                  ^  ( (g << 25) ^ (gc << 18) ^ (g << 11) ^ (gc << 5) ^ g )
      ;  k++
   ;  }

   ;  return h
;  }


                        /* created by me */
u32   mcxSvD1hash
(  const void        *key
,  u32               len
)
   {  u32   h     =  0xeca96537u
   ;  const char* k     =  key

   ;  while (len--)
      {  u32  g   =  *k
      ;  h        =  (  (h << 3)  ^ h ^ (h >> 5) )
                  ^  ( (g << 21) ^  (g << 12)   ^ (g << 5) ^ g )
      ;  k++
   ;  }

   ;  return h
;  }


                        /* created by Daniel Phillips */
u32   mcxDPhash
(  const void        *key
,  u32               len
)
   {  u32   h0    =  0x12a3fe2du
   ,        h1    =  0x37abe8f9u
   ;  const char* k     =  key

   ;  while (len--)
      {
         u32 h    =  h1 + (h0 ^ (*k++ * 71523))
      ;  h1       =  h0
      ;  h0       =  h
   ;  }
      return h0
;  }


                           /* "GNU Emacs" hash (from m4) */
u32 mcxGEhash
(  const void* key
,  u32         len
)
   {  const char* k  =  key
   ;  u32 hash =  0
   ;  int t
   ;  while (len--)
      {  if ((t = *k++) >= 0140)
         t -= 40
      ;  hash = ((hash << 3) + (hash >> 28) + t)
   ;  }
      return hash
;  }


                           /* Fowler Noll Vo hash */
u32   mcxFNVhash
(  const void *buf
,  u32 len
)
   {  u32 hval = 0x811c9dc5
   ;  const char *bp = buf

   ;  while (len--)
      {
#if 0                               /* other branch supposedly optimizes gcc */
         hval *= 0x01000193
#else
         hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24)
#endif
      ;  hval ^= *bp++;
   ;  }
       return hval
;  }



                           /* Berkely Database hash */
u32 mcxBDBhash
(  const void *key
,  u32        len
)
   {  const char* k  =  key
   ;  u32   hash     =  0

   ;  while (len--)
      {  hash = *k++ + (hash << 6) + (hash << 16) - hash
   ;  }
      return hash
;  }


                           /* One at a time hash, Bob Jenkins/Colin Plumb */
u32 mcxOAThash
(  const void *key
,  u32        len
)
   {  const char* k     =  key
   ;  u32   hash  =  0

   ;  while (len--)
      {  hash += *k++
      ;  hash += (hash << 10)
      ;  hash ^= (hash >> 6)
   ;  }

      hash += (hash << 3);
   ;  hash ^= (hash >> 11);
   ;  hash += (hash << 15);
   ;  return hash
;  }


                           /* by Dan Bernstein  */
u32 mcxDJBhash
(  const void *key
,  u32        len
)
   {  const char* k     =  key
   ;  u32   hash  =  5381
   ;  while (len--)
      {  hash = *k++ + (hash << 5) + hash
   ;  }
      return hash
;  }


                           /* UNIX ELF hash */
u32 mcxELFhash
(  const void *key
,  u32        len
)
   {  const char* k     =  key
   ;  u32   hash  =  0
   ;  u32   g

   ;  while (len--)
      {  hash = *k++ + (hash << 4)
      ;  if ((g = (hash & 0xF0000000u)))
         hash ^= g >> 24

      ;  hash &= ~g
   ;  }
      return hash
;  }



u32 mcxStrHash
(  const void* s
)
   {  dim l =  strlen(s)
   ;  return(mcxDPhash(s, l))
;  }


int mcxStrCmp
(  const void* a
,  const void* b
)
   {  return strcmp(a, b)
;  }



void bitprint
(  u32   key
,  FILE* fp
)
   {  do
      {  fputc(key & 1 ? '1' : '0',  fp)
   ;  }  while
         ((key = key >> 1))
;  }


int bitcount
(  u32   key
)
   {  int ct = 0
   ;  do
      {  if (key & 1)
         ct++
   ;  }
      while ((key = key >> 1))
   ;  return ct
;  }



#if 0
/* The old legacy hash */
static __u32 dx_hack_hash (const char *name, int len)
{
        __u32 hash0 = 0x12a3fe2d, hash1 = 0x37abe8f9;
        while (len--) {
                __u32 hash = hash1 + (hash0 ^ (*name++ * 7152373));

                if (hash & 0x80000000) hash -= 0x7fffffff;
                hash1 = hash0;
                hash0 = hash;
        }
        return (hash0 << 1);
}
#endif

