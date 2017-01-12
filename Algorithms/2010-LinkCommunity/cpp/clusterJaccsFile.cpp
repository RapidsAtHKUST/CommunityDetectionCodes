// clusterJaccsFile.cpp
// Jim Bagrow
// Last Modified: 2009-03-10

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
//
//  g++ -O3 -o cluster clusterJaccsFile.cpp
//  ./cluster network.pairs network.jaccs network.clusters threshold
//   
//     -- network.pairs is an integer edgelist (one edge, two nodes
//     per line)
//     
//     -- network.jaccs contains network.jaccs the jaccard 
//     coefficient for each pair of edges compared, of the form:
//         i_0 i_1 j_0 j_1 jaccard<newline>
//         ...
//     for edges (i_0,i_1) and (j_0,j_1), etc.
//     
//     -- network.clusters will contain one cluster of edges per line 
//     (edge nodes are comma-separated and edges are space-separated)
//     
//     --threshold is the [0,1] threshold for the clustering

// CORRECTNESS:
//  ??????

// OPTIMIZATION:
//  * Probably pretty slow with all the O(log n) .find map lookups
//  * Better to store edge2iterator? (edge2pointer really)  But do 
//  iterators get invalidated after an insert?
//  * According to http://en.wikipedia.org/wiki/Map_(C%2B%2B_container):
//  "Iterators are not invalidated by insert and erase operations
//  which don't remove the object to which the iterator points."
//  implemented edge2iter but there's no speed up!  Should get fancy...


#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <utility>   // for pairs
#include <algorithm> // for swap
using namespace std;

int main (int argc, char const *argv[]){
    //************* make sure args are present:
    if (argc != 6){
        cout << "ERROR: something wrong with the inputs" << endl;
        cout << "usage:\n    " << argv[0] << " network.pairs network.jaccs network.clusters network.cluster_stats threshold" << endl;
        exit(1);
    }
    float threshold = atof( argv[5] );
    if (threshold < 0.0 || threshold > 1.0){
        cout << "ERROR: specified threshold not in [0,1]" << endl;
        exit(1);
    }
    //************* got the args
    
    
    //************* start load edgelist
    ifstream inFile;
    inFile.open( argv[1] );
    if (!inFile) {
        cout << "ERROR: unable to open input file" << endl;
        exit(1); // terminate with error
    }
    // index should be iterator not integer????
    map< int,           set<pair<int,int> > > index2cluster; // O(log n) access too slow?
    map< pair<int,int>, map< int,set<pair<int,int> > >::iterator > edge2iter;

    int ni, nj, wij, index = 0;
    while (inFile >> ni >> nj){ // scan edgelist to populate 
    //while (inFile >> ni >> nj >> wij){ // scan edgelist to populate WEIGHTED
        if (ni >= nj) swap(ni,nj); // undirected!
        
        index2cluster[ index ].insert( make_pair(ni,nj) );         // build cluster index to set of edge-pairs map
        edge2iter[ make_pair(ni,nj) ] = index2cluster.find(index); // build edge pair to cluster iter map ******????
        index++;
    }
    inFile.close(); inFile.clear();
    //************* end load edgelist
    
    //************* loop over jaccards file and do the clustering
    ifstream jaccFile;  jaccFile.open( argv[2] );
    if (!jaccFile) {
        cout << "ERROR: unable to open jaccards file" << endl;
        exit(1); // terminate with error
    }
    int i0,i1,j0,j1; double jacc;
    int idx_i, idx_j;
    map< int, set<pair<int,int> > >::iterator iter_i,iter_j;
    set<pair<int,int> >::iterator iterS;
    while ( jaccFile >> i0 >> i1 >> j0 >> j1 >> jacc ) {
        if ( jacc >= threshold ) {
            if (i0 >= i1) swap(i0,i1); // undirected!
            if (j0 >= j1) swap(j0,j1); // undirected!
            
            iter_i = edge2iter[ make_pair(i0,i1) ];
            iter_j = edge2iter[ make_pair(j0,j1) ];
            if ( iter_i != iter_j ) {
                // always merge smaller cluster into bigger:
                if ( (*iter_j).second.size() > (*iter_i).second.size() ){ // !!!!!!
                    swap(iter_i, iter_j);
                }
                
                // merge cluster j into i and update index for all elements in j:
                for (iterS = iter_j->second.begin(); iterS != iter_j->second.end(); iterS++){
                    iter_i->second.insert( *iterS );
                    edge2iter[ *iterS ] = iter_i;
                }
                
                // delete cluster j:
                index2cluster.erase(iter_j);
            } 
        } // done merging clusters i and j
    }
    //************* done looping over jaccards file
    
    
    //************* write the clusters to file:
    jaccFile.close();
    cout << "There were " << index2cluster.size() << " clusters at threshold " << threshold << "." << endl;
    
    // all done clustering, write to file (and calculated partition density):
    FILE * clustersFile     = fopen( argv[3], "w" );
    FILE * clusterStatsFile = fopen( argv[4], "w" );
    
    set<int> clusterNodes;
    int mc, nc;
    int M = 0, Mns = 0;
    double wSum = 0.0;
    
    set< pair<int,int> >::iterator S;
    map< int,set<pair<int,int> > >::iterator it;
    
    for ( it = index2cluster.begin(); it != index2cluster.end(); it++ ) {
        clusterNodes.clear();
        for (S = it->second.begin(); S != it->second.end(); S++ ){
            fprintf( clustersFile, "%i,%i ", S->first, S->second ); // this leaves a trailing space...!
            
            clusterNodes.insert(S->first);
            clusterNodes.insert(S->second);
        }
        mc =   it->second.size();
        nc = clusterNodes.size();
        M += mc;
        if (nc != 2) {
            Mns  += mc;
            wSum += mc * (mc - (nc-1.0)) / ((nc-2.0)*(nc-1.0));
        }
        
        fprintf( clustersFile, "\n" );
        fprintf( clusterStatsFile, "%i %i\n", mc, nc );
    }
    fclose(clustersFile);
    fclose(clusterStatsFile);
    //*************
    
    cout << "The partition density is:" << endl;
    cout << "    D = " << 2.0 * wSum / M   << endl;
    cout << "not counting one-edge clusters:" << endl;
    cout << "    D = " << 2.0 * wSum / Mns << endl;
    
    
    return 0;
}
