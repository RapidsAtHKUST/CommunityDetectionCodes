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

#include "kruskal.h"

#include <iostream>

KruskalTree::KruskalTree() : largestComponentSize(1) {}

size_t KruskalTree::add()
{
    size_t vecSize = table.size();
    table.push_back(vecSize);
    commSizes.push_back(1);
    return vecSize;
}

size_t KruskalTree::add(const size_t root)
{
    if (root <= table.size())
    {
        table.push_back(root);
        findRoot(table[table.size() - 1]);
        commSizes.push_back(1);
    }
    else
        add();
    return table.size() - 1;
}

void KruskalTree::connect(const size_t first, const size_t second)
{
    if (first < table.size() && second < table.size() && !inSameSet(first, second)) // After the call inSameSet table[first] and table[second] point to their real roots
    {
        commSizes[table[second]] += commSizes[table[first]];
        if (commSizes[table[second]] > largestComponentSize)
            largestComponentSize = commSizes[table[second]];
        table[table[first]] = table[table[second]];
    }
}

bool KruskalTree::inSameSet(const size_t first, const size_t second)
{
    if (findRoot(first) == findRoot(second))
        return true;
    else
        return false;
}

size_t KruskalTree::findRoot(const size_t source)
{
    size_t root = table[source];
    while (root != table[root])
    {
        root = table[root];
    }
    size_t currentRoot = source;
    while (currentRoot != root)
    {
        size_t oldRoot = table[currentRoot];
        table[currentRoot] = root;
        currentRoot = oldRoot;
    }
    return root;
}

size_t KruskalTree::size(const size_t source)
{
    return commSizes[findRoot(source)];
}

size_t KruskalTree::getLargestComponentSize() const
{
    return largestComponentSize;
}

void KruskalTree::printTree(std::ostream & file) const
{
    for (int i = 0; i < table.size(); i++)
        file << i << ": " << table[i] << " s: " << commSizes[i] << std::endl;
    file << std::endl;
}
