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
  else
     parse_EdgeListBinary(G, inFile);
  
  displayGraphCharacteristics(G);

  //Build vector for storing colors
  int numColors = 0;
  int *colors = (int *) malloc (G->numVertices * sizeof(int)); assert (colors != 0);
#pragma omp parallel for
  for (long i=0; i<G->numVertices; i++) {
	colors[i] = -1;
  }
  double tmpTime;
  numColors = algoDistanceOneVertexColoringOpt(G, colors, nT, &tmpTime);
  printf("Time to color: %lf\n", tmpTime);
  //return 0;

  /* Step : Set up output parameters */
  char buf1[256];
  sprintf(buf1,"%s_colorFreq.txt",argv[1]);
  FILE* out1 = fopen(buf1,"w");
  char buf2[256];
  sprintf(buf2,"%s_colorTiming.txt",argv[1]);
  FILE* out2 = fopen(buf2,"w");
 
  int curThread = 2; //Start with two threads
  while (curThread <= nT) {
	printf("\n\n***************************************\n");
	printf("Starting run with %d threads.\n", curThread);
	printf("***************************************\n");
	fprintf(out2, "*******************************\n");
	fprintf(out2, "Number of threads: %d\n",curThread);
	fprintf(out2, "*******************************\n");
				
#pragma omp parallel for
	for (long i=0; i<G->numVertices; i++) {
		colors[i] = -1;
	}
	numColors = algoDistanceOneVertexColoringOpt(G, colors, curThread, &tmpTime);
	fprintf(out2, "Time to color : %lf\n",tmpTime);
		
	curThread = curThread*2;
  }//End of while()
	
  //Count the frequency of colors:
  long *colorFreq = (long *) malloc (numColors * sizeof(long)); assert(colorFreq != 0);
#pragma omp parallel for
  for(long i = 0; i < numColors; i++) {
	colorFreq[i] = 0;
  }
	
  #pragma omp parallel for
  for(long i = 0; i < G->numVertices; i++) {
	__sync_fetch_and_add(&colorFreq[colors[i]],1);
  }
	
  for(long i=0; i < numColors; i++) {
 	fprintf(out1,"%ld \t %ld\n", i, colorFreq[i]);
  }
	
  /* Step 5: Clean up */
  free(G->edgeListPtrs);
  free(G->edgeList);
  free(G);
	
  free(colorFreq);
  free(colors);
  
  return 0;

}
