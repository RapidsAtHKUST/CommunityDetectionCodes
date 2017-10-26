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

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("================================================================================================\n");
    printf("Usage: %s <Input file with voltage levels> <Output filename> \n", argv[0]);
    printf("================================================================================================\n");
    exit(-1);
  }
  printf("Input parameters: %s \n", argv[1]);
  
  graph *G = (graph *) malloc (sizeof(graph));
  //Read input from a file:
  printf("About to read file: %s\n", argv[1]);
  long * Volts = parse_MultiKvPowerGridGraph(G, argv[1]);
  //displayGraphEdgeList(G);

	
   //writeGraphPajekFormat(G, argv[2]);
	
	//return(0);
	
	
	
  long NV = G->numVertices; 
  long *C = (long *) malloc (NV * sizeof(long));
  assert(C != 0);
  for (long i=0; i<NV; i++)
      C[i] = -1;
	
  //Store the voltage of a given community
  //Worst-case size is |C|=|V|
  long *Cvolts = (long *) malloc (NV * sizeof(long));
  assert(Cvolts != 0);
  for (long i=0; i<NV; i++)
		Cvolts[i] = -1;
	
 
  long numCommunities = buildCommunityBasedOnVoltages(G, Volts, C, Cvolts);
  printf("Number of communities = %ld\n", numCommunities);
  printf("*******************\n");
  for (long i=0; i<numCommunities; i++) {
	printf("Voltage[%ld] = %ld\n", i, Cvolts[i]);
  }
  printf("*******************\n");
  /*
  printf("*******************\n");
  for (long i=0; i<NV; i++) {
	  printf("C[%ld] = %ld\n", i+1, C[i]);
  }
  printf("*******************\n");
  */
  
	//return 0;
  //segregateEdgesBasedOnVoltages(G, Volts);
	
  //Now build the new graph based on voltage community structure
  graph *Gnew = (graph *) malloc (sizeof(graph));
  buildNextLevelGraph(G, Gnew, C, numCommunities);


  printf("*******************\n\n");
  displayGraphEdgeList(Gnew);

  writeGraphPajekFormatWithNodeVolts(Gnew, Cvolts,  argv[2]);
	
  return 0;
   
}//End of main()
