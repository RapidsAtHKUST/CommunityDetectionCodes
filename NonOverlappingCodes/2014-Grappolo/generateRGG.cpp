// ***********************************************************************
//
//            Grappolo: A C++ library for graph clustering
//               Mahantesh Halappanavar (hala@pnnl.gov)
//               Pacific Northwest National Laboratory     
//
// ***********************************************************************
//
//       Copyright (2014) Battelle Memorial Institute
//                      All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
//
// 1. Redistributions of source code must retain the above copyright 
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
// notice, this list of conditions and the following disclaimer in the 
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its 
// contributors may be used to endorse or promote products derived from 
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// ************************************************************************

//Generate Random Geometric Graphs G=(V, E)
//Input parameters: (N, r): N=number of vertices, r=distance 
//                 in a unit square [0,1]^2 with n vertices.
#include "defs.h"
#include <iostream>
#include "RngStream.h"
using namespace std;

//RGG Generator
//N = Number of nodes; Edge weights are distances
//r = Maximum Eucledian distance between two vertices for an edge to exist
graph * generateRGG(long N, double r) {
  cout<<"Within RGG Generator: N= "<<N<<"; r: "<<r<<"\n";
  int nT;
#pragma omp parallel
  {
    nT = omp_get_num_threads();
  }
  cout<<"Number of threads: "<<nT<<endl;
  double var;
  double max_weight;
  long *permV, tmpVal;
  
  assert(r <= 1);

  long n = N; //#Nodes = N
  long m = 0; //#Edges = will be a function of r and N
  long NumIsloated = 0;

  //Generate n random points in two dimensions
  double *src  = (double *) malloc (n * sizeof(double));
  assert(src != NULL);
  double *dest = (double *) malloc (n * sizeof(double));
  assert(dest != NULL);
  long *degrees = (long *) malloc ((n+1) * sizeof(long));
  assert(degrees != NULL);

#pragma omp parallel for
  for (long i=0; i<=n; i++)
    degrees[i] = 0; //Initialize to zero

  // STEP-1: Generate random coordinate points in parallel
  //Initialize parallel pseudo-random number generator
  unsigned long seed[6] = {1, 2, 3, 4, 5, 6};
  RngStream::SetPackageSeed(seed);
  RngStream RngArray[nT]; //array of RngStream Objects

  cout<<"About to generate "<<n<<" random points\n";
  double start = omp_get_wtime();
  #pragma omp parallel
  {
     int myRank = omp_get_thread_num();
     #pragma omp for schedule(static)
     for (long Ti=0; Ti<n; Ti++) {
    	src[Ti]  =  RngArray[myRank].RandU01();
	dest[Ti] =  RngArray[myRank].RandU01();
     }
  }//End of parallel region
 
  //Count the number of edges
#pragma omp parallel for
  for (long i=0; i<n; i++) {
    bool isolated = true;
    for (long j=i+1; j<n; j++) {
      //Compute the Euclidean distance:
      //Two dimension: sqrt((px-qx)^2 + (py-qy)^2)
      double dx   = src[i]  - src[j];
      double dy   = dest[i] - dest[j];
      double dist = sqrt ( dx*dx + dy*dy );
      //Are the two vertices within the range?
      if ( dist <= r ) {
	__sync_fetch_and_add(&m, 1);
	isolated = false;
	__sync_fetch_and_add(&degrees[i+1], 1); //Increment by one to make space for zero
	__sync_fetch_and_add(&degrees[j+1], 1);
      }
    } //End of inner for loop
    //if (isolated) {
    //  __sync_fetch_and_add(&NumIsolated, 1);
    //}
  } //End of outer for loop

  //Adjust the number of edges variable:
  cout<<"Num Vertices: "<<n<<endl;
  cout<<"Num of Edges: "<<m<<endl;
  cout<<"Sparsity: "<< (double)( (double)n / (double)(N*N))*100.0 <<"%"<<endl;
  cout<<"Average degree = "<<(m/n)<<endl;
  cout<<"Num of isolated vertices = "<<NumIsloated<<endl;
  
  //Build CSR from the edge list:
  long* counter = (long *) malloc (n * sizeof(long));
  assert(counter != NULL);
#pragma omp parallel for
  for (long i=0; i<n; i++)
    counter[i] = 0;
  
  //Allocate memory for Edge list:
  edge *eList = (edge *) malloc ((2*m) * sizeof (edge));
  assert(eList != NULL);
  //Build the Pointer Array:
  for (long i=1; i<=n; i++) {
    degrees[i] += degrees[i-1];
  }
  //Sanity check:
  if(degrees[n] != 2*m) {
    cout<<"Number of edges added is not correct\n";
    exit(1);
  }
  cout<<"Done building pointer array\n";
  
#pragma omp parallel for
  for (long i=0; i<n; i++) {
    for (long j=(i+1); j<n; j++) {
      //Compute the Euclidean distance:
      //Two dimension: sqrt((px-qx)^2 + (py-qy)^2)
      double dx   = src[i]  - src[j];
      double dy   = dest[i] - dest[j];
      double dist = sqrt ( dx*dx + dy*dy );
      //Are the two vertices within the range?
      if ( dist <= r ) {      
	//Add edge i-->j
	long location = degrees[i] + __sync_fetch_and_add(&counter[i], 1);
	assert (location <= 2*m);
	eList[location].head   = i;
	eList[location].tail   = j;
	eList[location].weight = dist;
	
	//Add edge j-->i
	location = degrees[j] + __sync_fetch_and_add(&counter[j], 1);
	assert (location <= 2*m);
	eList[location].head   = j;
	eList[location].tail   = i;
	eList[location].weight = dist;
      }//End of if
    }//End of inner for loop
  }//End of outer for loop
  
  cout<<"Done building compressed format\n";
  //Clean up:
  free(src); 
  free(dest);
  free(counter);
  
  //STEP-5: Build and return a graph data structure
  graph * G = (graph *) malloc (sizeof(graph));
  G->numVertices = n;
  G->sVertices   = n;
  G->numEdges    = m;
  G->edgeListPtrs = degrees;
  G->edgeList     = eList;
  
  return G;
}//End of RGG-generator
