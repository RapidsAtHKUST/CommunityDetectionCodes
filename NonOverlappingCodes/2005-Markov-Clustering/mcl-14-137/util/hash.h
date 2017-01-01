/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_hash_h
#define tingea_hash_h

#include <stdio.h>

/* TODO:
 * -  make sort routines for keys and values by key or value criteria.
 * -  make interface for storing integers, preferably without objectifying them.
 * -  shrink hashes dynamically.
 *
*/


/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * *
 **            Implementation notes (a few).
 *
 *
 *    This hash interface is very powerful. It gives you more than enough
 *    rope to hang yourself and then some. It can be used as is, or wrappers
 *    can be made around it that restrict a caller's ability to err.
 *
 *    The danger lies in the fact that this interface only does retrieval
 *    and storage of pointers (both for keys and values), and does not clone
 *    anything. Anything happening with the objects pointed to during the
 *    lifetime of the hash is the responsibility of the caller.
 *
 *    What the interface cannot do currently is hash integers by value (rather
 *    than by reference). This functionality will probably be added someday.

 * Features:
 *    o  Searching, inserting, and deletion are all done by
 *       mcxHashSearch. It returns a pointer to mcxKV. In all modes, the
 *       caller can use the returned mcxKV* structure to obtain
 *       the 'val' and 'key' members.
 *
 *    o  Hashes grow automatically once the average load per bucket
 *       exceeds a settable threshold and if the hash was not declared
 *       constant.
 *
 *    o  Caller supplies both the hash function and the compare function.
 *       This interface provides several hash functions operating on a
 *       (void* base, int len) combo, where base is cast to char by the
 *       hash function. These functions can be used in creating custom hash
 *       functions for your custom objects.
 *
 *    o  You can (of course) have multiple hashes. This is not really
 *       a feature - however, since the idiotic <hsearch.h> does not offer
 *       this I thought I'd mention it.
 *
 *    o  Witness mcxHashWalkInit, mcxHashWalkStep.
 * 
 *    o  There is mcxHashMerge.
 *
 *    o  mcxHashKeys, mcxHashKVs.
 *
 *    Enjoy.

 * Notes
 *    There is a utility hashfile.c (distributed in a separate package)
 *    that can be used to stress-test this module. It allows customization
 *    of several aspects, including the hash function that should be used.
*/

#include "types.h"
#include "list.h"


/* The hash struct is hidden. Use mcxHashGetSettings if you need
 * to peek into the interior. Or read hash.c
*/

typedef struct mcxHash mcxHash;


typedef struct
{  void*       key
;  void*       val
;
}  mcxKV       ;


mcxHash* mcxHashNew
(  dim         n_buckets
,  u32         (*hash)  (const void *a)
,  int         (*cmp)   (const void *a, const void *b)
)  ;


#define MCX_HASH_OPT_DEFAULTS    0
#define MCX_HASH_OPT_CONSTANT    1
#define MCX_HASH_OPT_UNUSED      2


void mcxHashSetOpts
(  mcxHash*    hash
,  double      load
,  int         option      /* negative values will be ignored (feature) */
)  ;

dim mcxHashMemSize
(  mcxHash*    hash
)  ;


typedef struct mcxHashSettings
{  dim         n_buckets
;  dim         n_entries
;  float       load
;  mcxbits     options
;  
}  mcxHashSettings   ;


void mcxHashGetSettings
(  mcxHash*          hash
,  mcxHashSettings*  settings
)  ;


/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * *
 **            mcxHashSearch
 *
 *    action               returns
 *
 *    MCX_DATUM_DELETE   ->    deleted mcxKV* or NULL if not present
 *    MCX_DATUM_INSERT   ->    new or present mcxKV*
 *    MCX_DATUM_FIND     ->    mcxKV* if present NULL otherwise.
 * 
 * usage:
 *
 *    Values have to be inserted by the caller into the returned KV struct.
 *    Make sure that keys point to objects that are constant
 *    (with respect to the cmp function) during the lifetime of the hash.
 *    YOU have to ensure the integrity of both keys and values.
 *    This enables you to do whatever suits you, such as appending to
 *    values.
 *
 *    When inserting, check whether kv->key != key (where kv is returned value)
 *    if this is the case, an identically comparing key is already present.
 *    You may want to destroy one of the two keys and decide what to do
 *    with the value.
 *
 *    When deleting, the key-value pair is removed from the hash *AND RETURNED
 *    TO CALLER* - you have to decide yourself what to do with it.  You have to
 *    fetch the val and key members of the returned mcxKV object immediately:
 *    Subsequent inserts in the hash may reuse it.  If the key was not present,
 *    a value of NULL is returned.
 *
 *    When finding, life is simple. NULL if absent, matching kv otherwise.
 *
 * note:
 *
 *    memory management of keys and values is totally up to caller.
 *    If usage is clean, you can use mcxHashFree for disposal of hash.
*/

#define mcxHashSearch(key, hash, ACTION)  mcxHashSearchx(key, hash, ACTION, NULL)

mcxKV*   mcxHashSearchx
(  void*       key
,  mcxHash*    hash
,  mcxmode     ACTION
,  int*        delta
)  ;



/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * *
 **            mcxHashMerge
 *
 *    this one COPIES OBJECT POINTERS and DOES NOT CLONE.
 *    so after the merge, hash1 and hash2 keys and values should not be freed.
 *    In case there are equivalent keys in hash1 and hash2, this may
 *    cause trouble when the caller wants to do cleaning afterwards.
 *    This interface is still under development.
 *
 *    hashd may be equal to hash1 or hash2, and it may also be NULL.
*/

mcxHash* mcxHashMerge
(  mcxHash*    hash1
,  mcxHash*    hash2
,  mcxHash*    hashd
,  void*       merge(void* val1, void* val2)
)  ;


/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * *
 **            mcxHashFree
 *
 *    This only works if all keys are of the same type and/or all values
 *    are of the same type, and if your objects were created as expected by
 *    the free routines (presumably malloced heap memory) - be careful with
 *    constant objects like constant strings.
 *
 *    freekey and freeval may not free their argument. This is because
 *    tingea does not allow routines that leave arguments in an
 *    inconsistent state, and free routines in tingea generally accept
 *    an argument of the form <type>** pptr.
 *    In the case of mcxHashFree this means that the interface may
 *    feel slighly more cumbersome.
 *    A way out would have been to make the callbacks of signature
 *
 *          void freemem(void** mempp)
 * 
 *    The caller could access *mempp, cast it to the expected type,
 *    and later set *mempp to NULL. However, this would require
 *    new free routines for lots of types. With the current interface
 *    existing <type>Release routines can be used:
 *
 *    The type of free routine expected by mcxHashFree is generally
 *    called <type>Release or <type>Release_v, e.g. mcxTingRelease.
 *    Release routines release all memory of a composite object except the
 *    memory which holds the outer struct.
 *
 *    If one of key or val is *not* a composite type or is a composite type
 *    that does not contain malloced memory, use mcxHashFreeScalar.
 *
 *    Both freekey and freeval may be NULL. When NULL, the corresponding
 *    KV member is not loooked at. This is useful e.g. when hashing objects
 *    owned by someone else.
*/


void mcxHashFree
(  mcxHash**   hashpp
,  void        freekey(void* keypp)    /* (yourtype1** keypp)     */
,  void        freeval(void* valpp)    /* (yourtype2** valpp)     */
)  ;

void mcxHashFreeScalar
(  void* scalar
)  ;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    It copies the pointers stored in the hash
*/

void** mcxHashKeys
(  mcxHash*    hash
,  dim*        n_entries
,  int       (*cmp)(const void*, const void*)         /* works on keys */
,  mcxbits     opts        /* unused yet */
)  ;
                           /* Future options: SORT, SORT_DESC */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    It copies the pointers stored in the hash
*/

void** mcxHashKVs
(  mcxHash*    hash
,  dim*        n_entries
,  int       (*cmp)(const void*, const void*)
,  mcxbits     opts        /* unused yet */
)  ;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    Prints some information to stdout.
*/

void mcxHashStats
(  FILE*       fp
,  mcxHash*    hash
)  ;


typedef struct mcxHashWalk mcxHashWalk;


mcxHashWalk* mcxHashWalkInit
(  mcxHash  *hash
)  ;


mcxKV* mcxHashWalkStep
(  mcxHashWalk* walk
,  dim          *i_bucket
)  ;


void mcxHashWalkFree
(  mcxHashWalk  **walkpp
)  ;


void mcxHashApply
(  mcxHash* hash
,  void    (*cb)(const void* key, void* val, void* data)
,  void*    data
)  ;

                        /* UNIX ELF hash */
                        /* POOR! */
u32 mcxELFhash
(  const void *key
,  u32 len
)  ;

                        /* created by Bob Jenkins     */
u32 mcxBJhash
(  const void* key
,  u32         len
)  ;

                        /* One at a time hash, Bob Jenkins/Colin Plumb */
u32 mcxOAThash
(  const void *key
,  u32        len
)  ;

                        /* created by Daniel Phillips */
u32 mcxDPhash
(  const void* key
,  u32         len
)  ;

                        /* "Berkely Database" hash (from Ozan Yigit's page) */
                        /* POOR! */
u32 mcxBDBhash
(  const void *key
,  u32        len
)  ;

                        /* Dan Bernstein hash (from Ozan Yigit's page) */
u32 mcxDJBhash
(  const void *key
,  u32        len
)  ;

                        /* created by Chris Torek */
u32 mcxCThash
(  const void* key
,  u32         len
)  ;

                        /* "GNU Emacs" hash (from m4) */
                        /* not among the best */
u32 mcxGEhash
(  const void* key
,  u32         len
)  ;


                        /* Fowler Noll Vo hash */
u32   mcxFNVhash
(  const void *buf
,  u32 len
)  ;

                        /* All experimental with weak points. */
u32   mcxSvDhash
(  const void        *key
,  u32               len
)  ;
u32   mcxSvD2hash
(  const void        *key
,  u32               len
)  ;
u32   mcxSvD1hash
(  const void        *key
,  u32               len
)  ;

                        /* uses mcxDPhash             */
u32 mcxStrHash
(  const void* s
)  ;

int mcxStrCmp
(  const void* a
,  const void* b
)  ;


#endif

