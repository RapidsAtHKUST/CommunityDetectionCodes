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

#include "communityTracker.h"

CommunityTracker::CommunityTracker(cliqueHash & hash, KruskalTree & tree, nodeCommunities & nodeComs, Dendrogram & dendrogram, NetType & net) : hash(hash), tree(tree), nodeComs(nodeComs), dendrogram(dendrogram), net(net) {}

void CommunityTracker::addClique(weighedClique & kclique)
{
    // the index of the first (k-1)-clique found, others (k-1)-cliques will be connected to that one
    // may change during merging of (k-1)-communities, but always tells the community where to add other (k-1)-cliques
    int firstCliqueIndex;

    // true if this k-clique forms a new community, false otherwise, e.g. when community size changes
    bool formsNewCommunity = true;

    // the starting size of the community, used to track if the size of the community changes
    size_t initialSize;

    // tells if the initialSize has been initialized
    // this prevents us from changing the initialSize after initialization
    bool sizeInitialized = false;

    for (size_t i = 0; i < kclique.size(); i++)
    {
        // clear the tempVector and push all nodes to it except the i-th node
        tempVector.clear();
        for (size_t j = 0; j < kclique.size(); j++)
            if (i != j)
                tempVector.push_back(kclique.at(j));

        // replace the nodes in the k1clique to thos of the tempVector
        k1clique.replaceNodes(tempVector, net);

        // find the k1clique from the cliqueHash and store it's index
        // this is smaller than 0 if the clique is not in the hash, otherwise this is the cliques index
        int currentCliqueIndex = hash.getValue(k1clique);

        if (!i) // first (k-1)-clique from this k-clique
        {
            if (currentCliqueIndex >= 0) // found before
            {
                // we store the index of the first (k-1)-clique so others can be connected to it
                firstCliqueIndex = tree.findRoot(currentCliqueIndex);

                // belongs to an existing community, so no new community can form
                formsNewCommunity = false;

                // get the initial size of the excisting community from nodeComs
                initialSize = nodeComs.getSize(firstCliqueIndex);

                // mark that the initialSize has been initialized
                sizeInitialized = true;
            }
            else // not found before
            {
                // add this (k-1)-clique to the kruskal tree and store its index
                firstCliqueIndex = tree.add();

                // use the index to put the clique into cliqueHash and nodeComs
                hash.put(k1clique, firstCliqueIndex);
                nodeComs.addClique(firstCliqueIndex, k1clique);
            }
        }
        else // not the first (k-1)-clique from this k-clique
        {
            if (currentCliqueIndex >= 0) // found before
            {
                // check if the (k-1)-cliques are already in the same community
                // if they're not, merge the communities
                if (!tree.inSameSet(currentCliqueIndex, firstCliqueIndex))
                {
                    // if the size is not initialized, initialize it
                    if (!sizeInitialized)
                    {
                        // we use currentCliqueIndex here because if the sizeInitialized is not true, the firstCliqueIndex
                        // does not point to any existing community and so the community pointed by currentCliqueIndex
                        // becomes the initial community
                        initialSize = nodeComs.getSize(tree.findRoot(currentCliqueIndex));
                        sizeInitialized = true;
                    }

                    // connect the communities in nodeComs
                    nodeComs.connect(tree.findRoot(firstCliqueIndex), tree.findRoot(currentCliqueIndex));

                    // if a new community does not form, add the indexes to comsToMerge
                    if (!formsNewCommunity)
                    {
                        comsToMerge.insert(tree.findRoot(currentCliqueIndex));
                        comsToMerge.insert(tree.findRoot(firstCliqueIndex));
                    }

                    // connect the communities in the kruskal tree
                    tree.connect(firstCliqueIndex, currentCliqueIndex);

                    // update the firstCliqueIndex so that it points to the root of the kruskal tree
                    firstCliqueIndex = tree.findRoot(currentCliqueIndex);

                    // this k-clique belongs to an existing community, so no new community can form
                    formsNewCommunity = false;
                }
            }
            else // not found before
            {
                // add the new (k-1)-clique to the kruskal tree and store the index
                size_t newCliqueIndex = tree.add();

                // store it to the cliqueHash
                hash.put(k1clique, newCliqueIndex);

                // connect it to the first (k-1)-clique in the kruskal tree and in nodeComs
                tree.connect(newCliqueIndex, firstCliqueIndex);
                nodeComs.addClique(tree.findRoot(firstCliqueIndex), k1clique);
            }
        }
    }

    // new community has formed, so we add a new bottom to the dendrogram
    if (formsNewCommunity)
        dendrogram.addNewBottom(kclique.getWeight(), kclique.size(), tree.findRoot(firstCliqueIndex));

    // there are communities merging, so merge these to make a new dendrogram node
    if (comsToMerge.size())
        dendrogram.addConnection(kclique.getWeight(), nodeComs.getSize(tree.findRoot(firstCliqueIndex)), comsToMerge, tree.findRoot(firstCliqueIndex));

    // if no communities are merging, but the initialSize has been initialized and has changed after initialization
    // then add a new size change into the dendrogram
    else if (!comsToMerge.size() && nodeComs.getSize(tree.findRoot(firstCliqueIndex)) != initialSize && sizeInitialized)
        dendrogram.changeSize(kclique.getWeight(), nodeComs.getSize(tree.findRoot(firstCliqueIndex)), tree.findRoot(firstCliqueIndex));

    comsToMerge.clear();
}

void CommunityTracker::print(std::ostream & file) const
{
    file << "tree:" << std::endl;
    tree.printTree(file);
    file << std::endl;
    file << "nodeComs:" << std::endl;
    nodeComs.printCommunities(file);
    file << "-------------------" << std::endl;
}
