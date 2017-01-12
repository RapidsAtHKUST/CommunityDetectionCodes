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

#ifndef CLIQUE_HASH
#define CLIQUE_HASH

#include <list>
#include "weighedClique.h"


class cliqueHash
{

    // typedef std::list<std::pair<clique,size_t> > cliqueIndexSet;
    typedef std::list<std::pair<clique,size_t> > cliqueIndexSet;

private:

    size_t size;
    std::vector<cliqueIndexSet> hashTable;
    size_t hash_bits;
    size_t a, b, c, offSet;

    size_t hash_function(const clique key);

    cliqueIndexSet::iterator iter;
    std::pair<clique,size_t> currPair;
    size_t curr_hash_key;
    size_t keyCount;

public:

    cliqueHash(const size_t hashSize, const size_t t, const size_t keySize);


    cliqueHash();


    bool contains(const clique & key);


    int getValue(const clique & key);


    void put(const clique & key, const size_t value);


    std::pair<clique,size_t> begin();


    std::pair<clique,size_t> next();


    bool finished();


    size_t getKeyCount()
    {
        return keyCount;
    }

};

#endif
