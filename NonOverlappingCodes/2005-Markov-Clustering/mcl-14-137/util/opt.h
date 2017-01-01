/*   (C) Copyright 2002, 2003, 2004, 2005, 2006  Stijn van Dongen
 *   (C) Copyright 2007, 2008, 2009, 2010  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/


/* TODO
 *    recode dispatch case 'no arguments supplied'. now ugly.
*/

#ifndef tingea_opt_h
#define tingea_opt_h

#include "types.h"
#include "ting.h"
#include "hash.h"


/*  **************************************************************************
 * *
 **            Implementation notes (a few).
 *
 *    This interface munges and parses text presented to it, and creates
 *    arrays and optionally hashes from them. It does so by simply storing
 *    pointers to the data presented to it, and does not copy anything at all.
 *
 *    It is the callers job to make sure that the data scrutinized by this
 *    interface stays constant during interface usage.
 *
 *    Tentatively, this interface treats -I 3 and --I=3 as equivalent
 *    syntax. It is possible to define -I and --I= as separate options.
 *    (by defining -I with MCX_OPT_HASARG and --I with MCX_OPT_DEFAULT).
 *
 * TODO:
 *    implement newline/indent magic in option descriptions.
 *
 *    The prefix thing, is it necessary? caller could pass argv+prefix ..
*/

#define     MCX_OPT_DEFAULT      0     /* -a, xyz */
#define     MCX_OPT_HASARG       1     /* -a 10, xyz foo */
#define     MCX_OPT_REQUIRED     2
#define     MCX_OPT_INFO         4
#define     MCX_OPT_HIDDEN       16
#define     MCX_OPT_UNUSED       32


enum
{  MCX_OPT_STATUS_OK        =    0
,  MCX_OPT_STATUS_NOARG
,  MCX_OPT_STATUS_UNKNOWN
,  MCX_OPT_STATUS_NOMEM
}  ;


/*  struct mcxOptAnchor
 *
 * id
 *    When using mcxOptApropos, if the option MCX_OPT_DISPLAY_SKIP is used,
 *    an increment larger then one between successive ids (from the structs
 *    in the array presented to mcxOptApropos) causes an additional newline
 *    to be output before the option synopsis.
 *    This enables the creation of paragraphs in a simple manner.
 *
 *  descr_usage
 *    By default, this just contains the description of what the option does
 *    It is possible to specify a mark to be printed in front of that
 *    description. This requires both the mark and the description
 *    (which are joined in the same string) to be proceded by a special
 *    sequence. For the mark this is "\tM" and for the description
 *    this is "\tD". The description string should be last. A valid entry is
 *    thus
 *       "\tM!\tDset the resource scheme"
 *    This will print the marker '!' inbetween the option tag and its
 *    description.
 *
 *    Presumably, the legend to such markers is explained by the caller.
*/

typedef struct mcxOptAnchor
{  char*          tag            /* '-foo' or '--expand-only' etc       */
;  int            flags          /* MCX_OPT_HASARG etc                  */
;  int            id             /* ID                                  */
;  char*          descr_arg      /* "<fname>" or "<num>", NULL ok       */
;  char*          descr_usage    /* NULL allowed                        */
;
}  mcxOptAnchor   ;


void mcxOptAnchorSortByTag
(  mcxOptAnchor *anchors
,  dim n_anchors
)  ;


void mcxOptAnchorSortById
(  mcxOptAnchor *anchors
,  dim n_anchors
)  ;


/*
 * An array of these is returned by parse routines below.
 * An entry with .anch == NULL indicates end-of-array.
*/

typedef struct mcxOption
{  mcxOptAnchor*  anch
;  const char*    val
;
}  mcxOption      ;



/*
 * these routines only use status MCX_OPT_NOARG. The interface
 * is not yet frozen.
*/

/*    This tries to find as many arguments as it can, and reports the
 *    number of array elements it skipped.
*/

mcxOption* mcxOptExhaust
(  mcxOptAnchor*  anch
,  char**         argv
,  int            argc
,  int            prefix   /* skip these */
,  int*           n_elems_read
,  mcxstatus*     status
)  ;


/*    This will never read past the last arguments (suffix of them).
 *    It does currently not enforce that the number of arguments left
 *    is exactly equal to suffix (fixme?).
*/

mcxOption* mcxOptParse
(  mcxOptAnchor*  anch
,  char**         argv
,  int            argc
,  int            prefix   /* skip these */
,  int            suffix   /* skip those too */
,  mcxstatus*     status
)  ;


void mcxOptFree
(  mcxOption**    optpp
)  ;

mcxbool mcxOptIsInfo
(  const char*  arg
,  mcxOptAnchor* options
)  ;

#define MCX_OPT_DISPLAY_DEFAULT       0
#define MCX_OPT_DISPLAY_BREAK_HARD    1 << 4   /* break overly long lines */
#define MCX_OPT_DISPLAY_BREAK_SOFT    1 << 6   /* break overly long lines */
#define MCX_OPT_DISPLAY_CAPTION      1 << 10   /* break after option */
#define MCX_OPT_DISPLAY_PAR          1 << 12   /* ? useful ? paragraph mode */
#define MCX_OPT_DISPLAY_SKIP         1 << 14   /* display enum skips as pars */
#define MCX_OPT_DISPLAY_HIDDEN       1 << 16   /* do that */


void mcxOptApropos
(  FILE* fp
,  const char* me                /* unused currently */
,  const char* syntax
,  int      width
,  mcxbits  display
,  const mcxOptAnchor opt[]
)  ;


/*    The ones below are for spreading responsibility for parsing
 *    argument over different modules.
 *    See the mcl implementation for an example.
*/

mcxOption* mcxHOptExhaust
(  mcxHash*       opthash
,  char**         argv
,  int            argc
,  int            prefix   /* skip these */
,  int*           n_elems_read
,  mcxstatus*     status
)  ;

mcxOption* mcxHOptParse
(  mcxHash*       opthash
,  char**         argv
,  int            argc
,  int            prefix   /* skip these */
,  int            suffix   /* skip those too */
,  mcxstatus*     status
)  ;


/*
 * Creates a hash where the tag string is key, and the mcxOptAnchor is value.
*/ 

mcxHash* mcxOptHash
(  mcxOptAnchor*  opts
,  mcxHash*       hash
)  ;

void mcxOptHashFree
(  mcxHash**   hashpp
)  ;

mcxOptAnchor* mcxOptFind
(  char*       tag
,  mcxHash*    hopts  
)  ;

void mcxUsage
(  FILE* fp
,  const char*  caller
,  const char** lines
)  ;


extern int mcxOptPrintDigits;

mcxbool mcxOptCheckBounds
(  const char*    caller
,  const char*    flag
,  unsigned char  type
,  void*          var
,  int            (*lftRlt) (const void*, const void*)
,  void*          lftBound
,  int            (*rgtRlt) (const void*, const void*)
,  void*          rgtBound
)  ;


/* TODO
 * provide escape mechanisms for delim;
 * e.g. mcx tilde, UNIX backslash, or SGML percent sign based.
 * -> smarter space-based separation. whitespace ?
 *
 * NOTE
 *    Separates on spaces, which are replaced with '\0'.
 *    src is modified ('\0' are written throughout).
 *    the char* members 'char* argv[]' all point to within src.
*/

char** mcxOptParseString
(  char* src
,  int*  argc
,  unsigned char delim
)  ;

mcxTing* mcxOptArgLine
(  const char** argv
,  int argc
,  int quote      /*    '[' or '"' */
)  ;



/*
 * Severely undocumented stuff, but quite useful.
 * This is used to write a program that
 *    -  takes modes, e.g. 'clm dist'  'clm info'  'clm meet'
 *    -  has a set of shared options
 *    -  allows modes to have their own unique set of options
 *    -  Program and mode options are not separated by the mode,
 *       but all are specified after the mode.
 * Look at htpp://micans.org/mcl/src/mcl-latest/src/shcl/clm.c
 *         htpp://micans.org/mcl/src/mcl-latest/src/shmx/mcx.c
 * for two fully functioning examples.
*/

#define  MCX_DISP_DEFAULT  0
#define  MCX_DISP_HIDDEN   1
#define  MCX_DISP_MANUAL   2

typedef struct
{  const char*    name
;  const char*    syntax
;  mcxOptAnchor*  options
;  int            n_options
;  mcxstatus      (*arg_cb)(int optid, const char* val)
;  mcxstatus      (*init)( void )
;  mcxstatus      (*main)(int argc, const char* argv[])
;  int            n_at_least     /* trailing arguments */
;  int            n_at_most      /* trailing arguments */
;  mcxbits        flags
;
}  mcxDispHook    ;


typedef struct
{  int        id
;  mcxDispHook* (*get_hk)(void)
;
}  mcxDispEntry   ;



typedef struct mcx_disp_bundle
{  int            disp_argc
;  const char**   disp_argv
;  const char*    disp_name
;  const char*    disp_syntax
;  mcxOptAnchor*  disp_shared
;  dim            n_disp_shared
;  mcxstatus    (*shared_handler)(int optid, const char* val, mcxDispHook*, struct mcx_disp_bundle*)
;  void (*disp_version)(const char* me) 
;  mcxDispEntry*  disp_table
;
}  mcxDispBundle  ;


int mcxDispatch
(  mcxDispBundle* bundle
)  ;


#endif

