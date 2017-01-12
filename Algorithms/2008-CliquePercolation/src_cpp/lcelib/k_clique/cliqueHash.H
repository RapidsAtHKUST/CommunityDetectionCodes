/*
lcelib, A high performance library for complex networks research
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
#include "weighedClique.H"


class cliqueHash {

  // typedef std::list<std::pair<clique,size_t> > cliqueIndexSet;
typedef std::list<std::pair<clique,size_t> > cliqueIndexSet;
  
private:

  size_t size;
  std::vector<cliqueIndexSet> hashTable;
  size_t hash_bits;
  size_t a, b, c, offSet;
  
  size_t hash_function(const clique key) {
    const size_t keySize = key.size();


    // std::cerr << "clique size for hash_function is " << key.size() << "\n";
    size_t hash_key;
    size_t newInd[keySize];
    for (size_t i = 0; i < keySize; i++) 
      newInd[i] = key.at(keySize-1-i) & b;
    
      
    //     for (size_t i = 1; i < keySize; i++) new_ind[i] = A*key.at(i);
    hash_key = newInd[0] << hash_bits - offSet; // move to the left
    size_t move;
    for (size_t i = 1; i < keySize; i++) { 
      move = hash_bits - (i+1)*offSet;
      newInd[i] = newInd[i] << move;
      hash_key = hash_key | newInd[i] ;      // take
    }
    

    hash_key = hash_key | ( key.at(keySize-2) & c );

    // now take only the last hash_bits bits
    
    //     hash_key = hash_key & a;
    //     std::cerr << "hash key is: " << hash_key << "\n";

    /*
    hash_key = key.at(0)+1;
    for (size_t i = 1; i < key.size(); i++) hash_key *= key.at(i);
    hash_key = hash_key%size;
    //     hash_key = (key.at(0)+1)*key.at(1)*key.at(2)*key.at(3)%size;
    // hash_key = ( key.at(0) * key.at(3) + key.at(1) * key.at(2) )%size;
    */
    if (hash_key >= size || hash_key < 0 ) {
      std::cerr << "hash function did not work!\n";
      // std::cerr << "nodes were: " << key.at(0) << " " << key.at(1) << " " << key.at(2) << " " << key.at(3) << "\n";
      std::cerr << "hash key was: " << hash_key << "\n";
    }

    
    return hash_key;
  }

  cliqueIndexSet::iterator iter;
  std::pair<clique,size_t> currPair;
  size_t curr_hash_key;
  size_t keyCount;

public:

  cliqueHash(const size_t hashSize, const size_t t, const size_t keySize) {
    hashTable.resize(hashSize); // reserve memory
    keyCount = 0;
    size = hashSize;
    hash_bits = t;
    offSet = hash_bits/keySize; // this is how much we can move bits for each index    
    a = 1;
    a = a << hash_bits;
    a = a - 1;    // beginning is zeros, only hash_bits lasts bits are one
    b = 1; 
    b = b << offSet;
    b = b-1;
    c = 1;
    c = c << ( hash_bits - keySize*offSet );
    c = c - 1; 
   }

  cliqueHash() {
    cliqueHash(0, 1, 1);
  }


  

  bool contains(const clique & key) {
    // std::cerr << "entering contains-function\n";
    size_t hash_key = hash_function(key);
    //     std::cerr << "hash-key is:" << hash_key << "\n";
    
    bool key_found = false;
    for (cliqueIndexSet::iterator i = hashTable.at(hash_key).begin(); !key_found && i != hashTable.at(hash_key).end(); i++) {
      if ( (*i).first == key ) key_found = true;
    }
    //    std::cerr << "leaving contains-function\n";
    return key_found;
  }

  
  int getValue(const clique & key) {
    size_t hash_key = hash_function(key);
    
    int value = -1;
    bool key_found = false;
    for (cliqueIndexSet::iterator i = hashTable.at(hash_key).begin(); !key_found && i != hashTable.at(hash_key).end(); i++) {
      if ( (*i).first == key ) {
	value = (*i).second;
	key_found = true;
      }
    }
    return value;
  }

  void put(const clique & key, const size_t value) {
    size_t hash_key = hash_function(key);
    
    if ( contains(key) ) {
      bool key_found = false;
      for (cliqueIndexSet::iterator i = hashTable.at(hash_key).begin(); !key_found && i != hashTable.at(hash_key).end(); i++) {
	if ( (*i).first == key ) {
	  (*i).second = value;
	  key_found = true;
	}
      }
    }
    else {
      std::pair<clique,size_t> cliqueValPair(key,value);
      hashTable.at(hash_key).push_back( cliqueValPair );
      keyCount++;
    }
  }


  std::pair<clique,size_t> begin() {
    curr_hash_key = 0;
    while ( curr_hash_key < size && hashTable.at(curr_hash_key).size() == 0  ) {
      curr_hash_key++;
    }
    if ( curr_hash_key < size ) {       // now we have found the first slot that is not empty
      iter = hashTable.at(curr_hash_key).begin();
      return *iter;
    }
    else {
      clique tmpClique;
      currPair = std::make_pair( tmpClique, 0);
      return currPair;
    }
  }

  std::pair<clique,size_t> next() {
    iter++;
    if ( iter == hashTable.at(curr_hash_key).end() )  { // move to the next non-empty slot
      do {
	curr_hash_key++;
      }	
      while ( curr_hash_key < size && hashTable.at(curr_hash_key).size() == 0  ); 
      if ( curr_hash_key < size ) {       // now we have found the first slot that is not empty
	iter = hashTable.at(curr_hash_key).begin();
	return *iter;
      }
      else {
	clique tmpClique;
	currPair = std::make_pair(tmpClique, 0);
	return currPair;
      }
    } 
    else {
      return *iter;
    }
  }


  bool finished() {
    
    if ( curr_hash_key == size ) return true;
    else if ( getKeyCount() == 0 ) return true;
    else return false;
    
  }


  size_t getKeyCount() {
    return keyCount;
  }

};

#endif
