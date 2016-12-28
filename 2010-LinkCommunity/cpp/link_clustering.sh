#!/usr/bin/env bash

# link_clustering.sh
# Jim Bagrow
# Last Modified: 2010-02-16

# Copyright 2009,2010 James Bagrow
# 
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


usage() {
echo 1>&2 "USAGE: $0 [options] network.pairs jaccard_threshold

This script calculates the link communities for the 
network stored in network.pairs, using jaccard_threshold.

network.pairs must contain one edge per line, represented as two
integers separated by whitespace. Each integer represents a node
and the nodes must be numbered sequentially from zero.  The file
edgelist2pairs.py may be used to convert your network to this
integer-based format.

Files network.jaccs and network.clusters will be created, unless
otherwise indicated. network.jaccs contains the jaccard
coefficient for a pair of edges per line, represented as four
space-separated integers and a float.  This file may become very
large for large/dense networks

Executables calcJaccards and clusterJaccards will be compiled
(using gcc) if not already present.  To compile, the files
calcAndWrite_Jaccards.cpp and clusterJaccsFile.cpp must be
present in the same directory as $0.

OPTIONS:
   -h    Show this message only
   -d    Delete jaccard file upon completion
   -f    Force GCC to compile executables"
}


# parse the command line
DELETE=
FORCE_GCC=
while getopts “hdf” OPTION
do
    case $OPTION in
        h)  usage; exit 666 ;;
        d)  DELETE=1 ;;
        f)  FORCE_GCC=1 ;;
    esac
done
shift $(($OPTIND - 1)) # $1 now points at right arg...

if [ $# -ne 2 ]; then
    usage
    exit 666
fi
if [ ! -f $1 ]; then
    echo 1>&2 Input file $1 does not exist
    exit 666
fi
# done parsing


# compile executables if necessary:
COMPILER=g++ # make this controllable by the user or use a MAKE file?
if [[ ! -x calcJaccards || $FORCE_GCC ]]; then
    echo -n "compiling calcAndWrite_Jaccards.cpp..."
    $COMPILER -O5 -o calcJaccards calcAndWrite_Jaccards.cpp
    echo " done"
fi
if [[ ! -x clusterJaccards || $FORCE_GCC ]]; then
    echo -n "compiling clusterJaccsFile.cpp..."
    $COMPILER -O5 -o clusterJaccards clusterJaccsFile.cpp
    echo " done"
    echo ''
fi


# run the calculations:
F=${1%.*} # remove extension
echo -n "calculating edge-pair jaccards... "
./calcJaccards    $1 ${F}.jaccs;
echo 'done'
echo 'clustering edges and computing partition density...'
echo ''
./clusterJaccards $1 ${F}.jaccs ${F}.clusters ${F}.mc_nc $2;


# remove jaccards file, if necessary:
if [ $DELETE ]; then
    echo ""
    echo "removing jaccard file"
    rm ${F}.jaccs
fi
