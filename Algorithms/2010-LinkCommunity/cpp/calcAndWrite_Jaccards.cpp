// calcAndWrite_Jaccards.cpp
// Jim Bagrow
// Last Modified: 2008-12-30

/*
Copyright 2008,2009,2010 James Bagrow


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


// USAGE:
//      g++ -O3 -o calc calcAndWrite_Jaccards.cpp
//      ./calc network.pairs network.jaccs
//
//  -- network.pairs is an integer edgelist (one edge, two nodes
//  per line)
//  -- network.jaccs will contain the jaccard coefficient for each
//  pair of edges compared, of the form:
//      i_0 i_1 j_0 j_1 jaccard<newline>
//      ...
//  for edges (i_0,i_1) and (j_0,j_1)


// same as calcAndWrite_Jaccards.cpp, but without the graph code...
// but not much quicker (:-()

// all this does is calculate the jaccard for "each" edge pair and
// write it to a file.  Two make this into real code will take some
// more work (the next step can be the hierarchical clustering over
// the output jacc file...)

// CORRECTNESS:
//  Returns same jaccard file as calcAndWrite_Jaccards.cpp, as shown
//  by compareTwoJaccs.py...

#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm> // for swap
using namespace std;

int intersection_size( const set<int> &A, const set<int> &B ) {
    // only get number of elements, don't build a new set
    // this assumes the sets are ordered, which std::sets are!
    int num = 0;
    set<int>::const_iterator As = A.begin(), Af = A.end(),
                             Bs = B.begin(), Bf = B.end();
    while ( As != Af && Bs != Bf ) {
        if ( *As < *Bs) {
            ++As;
        } else if ( *Bs < *As ) {
            ++Bs;
        } else {
            ++num;
            ++As;
            ++Bs;
        }
    }
    return num;
}

int main (int argc, char const *argv[]){
    // make sure args are present:
    if (!argv[1]){
        cerr << "ERROR: no input file specified" << endl;
        cerr << "usage:\n    " << argv[0] << " input.pairs output.jaccs" << endl;
        exit(1);
    }
    if (!argv[2]){
        cerr << "ERROR: no output file specified" << endl;
        cerr << "usage:\n    " << argv[0] << " input.pairs output.jaccs" << endl;
        exit(1);
    }


    // load edgelist into two arrays, and into an array of sets:
    ifstream inFile;
    inFile.open (argv[1]);
    if (!inFile) {
        cerr << "ERROR: unable to open input file" << endl;
        exit(1); // terminate with error
    }
    int ni,nj, max_node = -1;
    while (inFile >> ni >> nj){ // scan edgelist once to get number of edges and nodes, for allocation
        if (ni > max_node){  max_node = ni;  }
        if (nj > max_node){  max_node = nj;  }
    }
    inFile.close();
    
    int num_nodes = max_node + 1; // assumes nodes are contiguous ints starting at zero
    set<int> *neighbors = NULL;
    neighbors = new set<int>[num_nodes]; // this stores the network

    inFile.open( argv[1] );
    int index = 0;
    while (inFile >> ni >> nj){ // rescan edgelist to populate 
        neighbors[ni].insert(nj);
        neighbors[nj].insert(ni); // undirected
    }
    for (int i=0; i < num_nodes; i++){
        neighbors[i].insert(i); // neighbors[] is now INCLUSIVE!
    }
    inFile.close();
    // end load edgelist


    // do the gosh darn calculation, fool!
    FILE * jaccFile = fopen(argv[2],"w");
    int n_i, n_j, keystone, len_int;
    double curr_jacc;
    set<int>::iterator i, j;
    for (int keystone=0; keystone < num_nodes; keystone++) { // loop over keystones 
        
        for ( i = neighbors[keystone].begin(); i != neighbors[keystone].end(); i++) { // neighbors of keystone
            n_i = *i;
            if (n_i == keystone)
                continue;
            
            for ( j = neighbors[keystone].begin(); j != neighbors[keystone].end(); j++ ) { // neighbors of keystone
                n_j = *j;
                if (n_j == keystone or n_i >= n_j)
                    continue;
                
                len_int = intersection_size( neighbors[n_i], neighbors[n_j] );    // my set intersection function
                curr_jacc = (double) len_int / (double)( neighbors[n_i].size() + neighbors[n_j].size() - len_int );
                
                if (keystone < n_i && keystone < n_j){
                    fprintf( jaccFile, "%i\t%i\t%i\t%i\t%f\n", keystone, n_i, keystone, n_j, curr_jacc );
                } else if (keystone < n_i && keystone > n_j){
                    fprintf( jaccFile, "%i\t%i\t%i\t%i\t%f\n", keystone, n_i, n_j, keystone, curr_jacc );
                } else if (keystone > n_i && keystone < n_j){
                    fprintf( jaccFile, "%i\t%i\t%i\t%i\t%f\n", n_i, keystone, keystone, n_j, curr_jacc );
                } else {
                    fprintf( jaccFile, "%i\t%i\t%i\t%i\t%f\n", n_i, keystone, n_j, keystone, curr_jacc );
                }
            }
        }
    } // done loop over keystones
    fclose(jaccFile);

    delete [] neighbors; // all done, clean up memory...

    return 0;
}
