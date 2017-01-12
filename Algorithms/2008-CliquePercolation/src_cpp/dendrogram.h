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

#ifndef DENDRO_TREE_H
#define DENDRO_TREE_H

#include <cstddef>
#include <vector>
#include <map>
#include <iostream>
#include <set>
#include <algorithm>

class Dendrogram
{
public:
    Dendrogram() {}

    void addNewBottom(const float weight, const size_t size, const size_t comIndex);

    void changeSize(const float weight, const size_t size, const size_t comIndex);

    void addConnection(const float weight, const size_t size, const std::set<size_t> & comsToMerge, const size_t newComIndex);

    void printFromNode(const size_t comIndex, std::ostream & file);

    void printTree(std::ostream & file);

    ~Dendrogram();

private:

    class DendrogramNode
    {
        friend class Dendrogram;

        public:
            DendrogramNode(float weight, size_t size, size_t comIndex) : weight(weight), size(size), comIndex(comIndex), parent(NULL) {}

            void addChild(DendrogramNode * child) { children.insert(child); } // TESTI

        private:
            float weight;
            size_t size;
            size_t comIndex;
            DendrogramNode * parent;

            std::set<DendrogramNode*> children; // TESTI
    };

    static bool compareNodes(DendrogramNode * first, DendrogramNode * second) { return first->weight > second->weight; }

    // the nodes which no other node points to
    std::vector<DendrogramNode*> bottom;

    // all nodes in the dendrogram, used for freeing the memory
    std::vector<DendrogramNode*> nodes;

    // map that holds the mapping from community indexes
    std::map<size_t, DendrogramNode*> comIndexMap;
};

#endif
