#!/usr/local/bin/bash

# mcxminusmeet <modes> <matrix> <low> <high>
#   modes:   1  overwrite matrix vector (reinstate later)
#            2  right rather than left
#            4  check
#            8  use mclvBinary
#           16  dump final matrix (should be identical)
#           32  use dispatching meet implementation

set -e

shmx=../../src/shmx

export MCLXIODIGITS=8
export TINGEA_LOG_TAG=x

if let $(( $# < 5 )); then
   echo "need modes nodes1 edges1 nodes2 edges2 arguments"
   $shmx/mcxminusmeet
   false
fi

modes=$1
nodes1=$2
edges1=$3
nodes2=$4
edges2=$5

$shmx/mcxrand -gen $nodes1 -add $edges1 | $shmx/mcx alter -canonical 10 -o rn1.mci
$shmx/mcxrand -gen $nodes2 -add $edges2 | $shmx/mcx alter -canonical 10 -o rn2.mci

# valgrind --show-reachable=yes --leak-check=full
$shmx/mcxminusmeet $modes rn1.mci rn2.mci

if let $(( $modes & 16 )); then
   echo "-->" $(mcxi /out.mmm lm /rn1.mci lm -1 mul add /- wm | mcxdump | wc -l) "<--"
else
   echo "--> matrix not output - no identity check <--"
fi

