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

#include "nodeCommunities.h"

nodeCommunities::nodeCommunities() : largest(0) {}

void nodeCommunities::addClique(const size_t index, const clique & newClique)
{
    // add the nodes to the set
    for (size_t i = 0; i < newClique.size(); i++)
        communities[index].put(newClique.at(i));
    // check if this community becomes the largest
    if (communities[index].size() > largest)
    {
        largest = communities[index].size();
        largestCom = index;
    }
}

void nodeCommunities::erase(const size_t index)
{
    communities.erase(index);
    if (index == largestCom)
    {
        largest = 0;
        for (std::map<size_t, nodeSet>::iterator i = communities.begin(); i != communities.end(); i++)
            if (i->second.size() > largest)
            {
                largest = i->second.size();
                largestCom = i->first;
            }
    }
}

size_t nodeCommunities::connect(size_t first, size_t second)
{
    for (nodeSet::iterator i = communities[first].begin(); !i.finished(); ++i)
        communities[second].put(*i);

    erase(first);
    if (communities[second].size() > largest)
    {
        largest = communities[second].size();
        largestCom = second;
    }

    return second;
}

void nodeCommunities::printCommunities(std::ostream & file) const
{
    file << "largest: " << largest << std::endl;
    for (std::map<size_t, nodeSet>::const_iterator i = communities.begin(); i != communities.end(); i++)
    {
        file << i->first << ": ";
        for (nodeSet::const_iterator j = i->second.begin(); !j.finished(); ++j)
            file << *j << " ";
        file << std::endl;
    }
    file << std::endl;
}
