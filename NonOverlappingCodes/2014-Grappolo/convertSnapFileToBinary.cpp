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

#include "defs.h"

using namespace std;

int main(int argc, char** argv) {
 
  //Parse Input parameters:
  clustering_parameters opts;
  if (!opts.parse(argc, argv)) {
    return -1;
  }
  int nT = 1; //Default is one thread
#pragma omp parallel
  {
      nT = omp_get_num_threads();
  }
  if (nT <= 1) {
	printf("The number of threads should be greater than one.\n");
	return 0;
  }
  graph* G = (graph *) malloc (sizeof(graph));

  int fType = opts.ftype; //File type
  char *inFile = (char*) opts.inFile;
 
  if(fType == 1)
     parse_MatrixMarket_Sym_AsGraph(G, inFile);
  else if(fType == 2)
     parse_Dimacs9FormatDirectedNewD(G, inFile);
  else if(fType == 3)
     parse_PajekFormat(G, inFile);
  else if(fType == 4)
     parse_PajekFormatUndirected(G, inFile);
  else if(fType == 5)
     loadMetisFileFormat(G, inFile); 
  else if(fType == 6)
     parse_DoulbedEdgeList(G, inFile); 
  else if(fType == 7)
     parse_EdgeListBinary(G, inFile);
  else
    parse_SNAP(G, inFile);
  
  displayGraphCharacteristics(G);
  //displayGraph(G);
 
  //Clean up the file:
  long numVtxToFix = 0; //Default zero
  long *C = (long *) malloc (G->numVertices * sizeof(long)); assert(C != 0);
  numVtxToFix = vertexFollowing(G,C); //Find vertices that follow other vertices
  if( numVtxToFix > 0) {  //Need to fix things: build a new graph		      
    printf("Graph will be modified -- %ld vertices need to be fixed.\n", numVtxToFix);          
    graph *Gnew = (graph *) malloc (sizeof(graph));
    long numClusters = renumberClustersContiguously(C, G->numVertices);		         
    buildNewGraphVF(G, Gnew, C, numClusters);
    //Get rid of the old graph and store the new graph
    free(G->edgeListPtrs);
    free(G->edgeList);
    free(G);
    G = Gnew;		
  }
  free(C); //Free up memory
  
  //Display characteristics
  displayGraphCharacteristics(G);

  //Save file to binary:
  char outFile[256];
  sprintf(outFile,"%s.bin", opts.inFile);
  printf("Graph will be stored in binary format in file: %s\n", outFile);
  writeGraphBinaryFormat(G, outFile);
 
  //Cleanup:
  if(G != 0) {
    free(G->edgeListPtrs);
    free(G->edgeList);
    free(G);
  }
  
  return 0;
}//End of main()
