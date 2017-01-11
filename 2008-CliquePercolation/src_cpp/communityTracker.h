/*
scp, The sequential clique percolation algorithm.
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef COMMUNITYTRACKER_H
#define COMMUNITYTRACKER_H

#include <iostream>
#include <vector>
#include <utility>

#include "lcelib/Nets.H"
#include "cliqueHash.h"
#include "nodeCommunities.h"
#include "kruskal.h"
#include "dendrogram.h"

class CommunityTracker
{
public:

    CommunityTracker(cliqueHash & hash, KruskalTree & tree, nodeCommunities & nodeComs, Dendrogram & dendrogram, NetType & net);

    void addClique(weighedClique & kclique);

    void print(std::ostream & file) const;

private:

    cliqueHash & hash;
    KruskalTree & tree;
    nodeCommunities & nodeComs;
    Dendrogram & dendrogram;
    NetType & net;

    std::vector<size_t> tempVector; // used when creating (k-1)-cliques, this way we avoid creating a new vector object for every k-clique
    weighedClique k1clique;

    //std::vector<std::pair<size_t, size_t> > mergingCommunities; // tells which communities merge as a result from adding a clique
    std::set<size_t> comsToMerge;

};

#endif
