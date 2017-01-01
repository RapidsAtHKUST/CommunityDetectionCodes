#!/usr/local/bin/bash
set -e


prefix=$HOME/local

if [[ ! -e "$prefix/lib/libimpala.a" ]]; then
   echo "Set prefix so that prefix/lib/libimpala.a exists"
   false
fi

if [[ ! -e "$prefix/include/mcl/impala/ivp.h" ]]; then
   echo "Set prefix so that prefix/include/mcl/ exists"
   echo "and contains impala/ivp.h impala/ivptypes.h util/inttypes.h util/types.h"
   false
fi

N=4
fnout=out.combined

if (( $# > 0 )); then
   N=$1
fi
if (( $# > 1 )); then
   fnout=$2
fi



##
##    NOTE
##
## The C program below writes to file descriptors 4 5 6 and 7.
## These have to be set up externally; the shell code following
## the C code does just that.


cat <<EOC >mcx-test-packed2.c

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <string.h>


#include "impala/ivp.h"

int main(int argc, char **argv)
   {  mclp small[12][4] = {  { { 1, 1.0 }, { 9, 1.0 }, { 6, 1.0 } , { 5, 1.0 } }
                           , { { 4, 1.0 }, { 2, 1.0 }, { 0, 1.0 } ,                 {-1, 1.0 } }
                           , { { 1, 1.0 }, { 3, 1.0 }, { 4, 1.0 } ,                 {-1, 1.0 } }
                           , { { 2, 1.0 }, { 7, 1.0 }, { 8, 1.0 } , {10, 1.0 } }
                           , { { 1, 1.0 }, { 2, 1.0 }, { 6, 1.0 } , { 7, 1.0 } }
                           , { { 0, 1.0 }, { 9, 1.0 },                              {-1, 1.0 } , {-1, 1.0 } }
                           , { { 0, 1.0 }, { 4, 1.0 }, { 9, 1.0 } ,                 {-1, 1.0 } }
                           , { { 3, 1.0 }, { 4, 1.0 }, { 8, 1.0 } , {10, 1.0 } }
                           , { { 3, 1.0 }, { 7, 1.0 }, {11, 1.0 } , {10, 1.0 } }
                           , { { 5, 1.0 }, { 0, 1.0 }, { 6, 1.0 } ,                 {-1, 1.0 } }
                           , { { 3, 1.0 }, { 7, 1.0 }, { 8, 1.0 } , {11, 1.0 } }
                           , { { 8, 1.0 }, {10, 1.0 }, {10, 1.0 } , { 8, 1.0 } }
                           }
   ;  long src_and_len[2]
   ;  long sizes[12] = { 4, 3, 3, 4, 4, 2, 3, 4, 4, 3, 4, 4 }
   ;  long i

   ;  mclp ivp = { 1, 1.0 }
   ;  for (i=0; i<12; i++)
      {  src_and_len[0] = i
      ;  src_and_len[1] = sizes[i]
      ;  int fd = 4 + i % $N
      ;  ssize_t sz = sizeof src_and_len[0] * 2

                        /*  Write source node and number of neighbours specified; two longs */
      ;  if (sz != write(fd, src_and_len, sz))
         exit(13)
                        /*  A list of ivps (index value pairs), each containing a neighbour index and a weight */
      ;  sz = sizeof ivp * sizes[i]
      ;  if (sz != write(fd, small[i], sz))
         exit(13)
   ;  }
      return 0
;  }
EOC


gcc -g -I$HOME/local/include/mcl -c mcx-test-packed2.c
gcc -g -L$HOME/local/lib -limpala -o mcx-test-packed2 mcx-test-packed2.o


##
##    Open file descriptors and connect them to process substitution streams.
##

rm -f out.p2.*


for i in $(seq 4 1 $((3+$N))); do
   eval "exec $i> >(mcxload -packed - -pack-cnum 12 -pack-rnum 12 -ri max -o out.p2.$i)"
done

./mcx-test-packed2

for i in $(seq 4 1 $((3+$N))); do
   eval "exec $i>&-"
done

while lsof -c mcxload > /dev/null; do
   echo 'waiting for mcxload to finish'
   sleep 1
done

mcx collect -o "$fnout" --max-matrix out.p2.*


