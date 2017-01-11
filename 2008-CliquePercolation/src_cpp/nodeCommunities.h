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

#ifndef NODECOMMUNITIES_H
#define NODECOMMUNITIES_H

#include "weighedClique.h"

#include <map>
#include <iostream>

#include "lcelib/Containers.H"

typedef Set<size_t> nodeSet;

class nodeCommunities
{
public:
    nodeCommunities();

    void addClique(const size_t index, const clique & newClique);

    size_t connect(size_t first, size_t second); // connects two communities, the smaller one is merged into the bigger one, return the index of the merged community

    size_t getLargest() const { return largest; }

    size_t getSize(const size_t comIndex) { return communities[comIndex].size(); }

    void printCommunities(std::ostream & file) const;

private:
    std::map<size_t, nodeSet> communities;

    size_t largest; // holds the size of the largest community
    size_t largestCom; // the index of the largest community

    void erase(const size_t index); // erases the community with the given index, erasing the largest community fucks up the largest counter
};

#endif
