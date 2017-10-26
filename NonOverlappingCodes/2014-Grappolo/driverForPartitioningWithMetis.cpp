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
#include "utilityGraphPartitioner.h"

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
  double time1, time2;
  graph* G = (graph *) malloc (sizeof(graph));

  /* Step 2: Parse the graph in Matrix Market format */  
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
  else 
     parse_EdgeListBinary(G, inFile);
    
  displayGraphCharacteristics(G);
 
  long NV = G->numVertices;
  long *vPart = (long *) malloc (NV * sizeof(long)); assert(vPart != 0);
  
  int myVec[7]={2,4,8,16,24,32,36};
  for (int i=0; i<7; i++) {       
       char outFile[256];
       sprintf(outFile,"%s_%d", opts.inFile, myVec[i]);       
       printf("Processing with %d partitions; will be stored in file: %s\n", myVec[i], outFile);
       MetisGraphPartitioner( G, vPart, myVec[i]);
       FILE* out = fopen(outFile,"w");
       for(long i = 0; i<NV;i++) {
          fprintf(out,"%ld\n",vPart[i]);
       }		
       fclose(out);
       for (long j=0; j<NV; j++ )
          vPart[j] = -1;
  }

  //Cleanup:
  if(G != 0) {
    free(G->edgeListPtrs);
    free(G->edgeList);
    free(G);
  }
  free(vPart);
  
  return 0;
}//End of main()
