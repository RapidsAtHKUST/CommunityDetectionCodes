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

#ifndef KRUSKAL_H
#define KRUSKAL_H

#include <vector>
#include <iostream>

#include "weighedClique.h"

class KruskalTree
{
public:
    KruskalTree();

    size_t add(); // return the index where the element was added ( = table.size() - 1 )

    size_t add(const size_t root); // same as above

    void connect(const size_t first, const size_t second); // connects two sets, after the operation the first points to the root of second

    bool inSameSet(const size_t first, const size_t second); // checks if two elements belong to same set

    size_t findRoot(const size_t source); // finds the real root of an element

    size_t size(const size_t source);

    size_t getLargestComponentSize() const;

    void printTree(std::ostream & file) const;

private:

    std::vector<size_t> table; // stores the pointer to root
    std::vector<size_t> commSizes; // sizes of the sets are kept here

    size_t largestComponentSize;

};

#endif
