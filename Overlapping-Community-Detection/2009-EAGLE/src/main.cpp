#include "eagle.h"
#include <iostream>
#include <fstream>
#include <omp.h>

using namespace std;

int main(int argc, char* argv[])
{
  int mode;
  const char* src = "";
  const char* dest = "";
  int nthreads;
  cout << "argc: " << argc << endl;
  switch (argc) {
    case 3:
      mode = 1;
      nthreads = atoi(argv[1]);
      src = argv[2];
      break;
    
    case 4:
      mode = 2;
      nthreads = atoi(argv[1]);
      src = argv[2];
      dest = argv[3];
      break;
    
    default:
      cerr << "Sinopsi: " << argv[0] << " nThreads src [dest]" << endl;
      return EXIT_FAILURE;
  }
  
  cout << "EAGLE" << endl;
  cout << "Caricato su: " << src << endl;
  if (mode == 2) cout << "Salvataggio dati su: " << dest << endl;
  cout << "Numero di thread usati: " << nthreads << endl;
  
  omp_set_num_threads(nthreads);	
  igraph_i_set_attribute_table(&igraph_cattribute_table);
  
  int result;
  igraph_t graph;
  
  FILE* instream = fopen(src, "r");
  if (instream == NULL) 
  {
    cerr << "File non esistente." << endl;
    
    return EXIT_FAILURE;
  }
  
  cout << "Lettura grafo" << endl;
  result = igraph_read_graph_graphml(&graph, instream, 0);
  cout << "OK." << endl;
  
  fclose(instream);
  
  EAGLE e(&graph);
  
  Communities* r = e.run();
  
  
  cout << r->summary(mode == 1) << endl;
  
  if (mode == 2)
  {
    ofstream ofs(dest);
    r->dump(ofs);
    ofs.close();
  }
  
  // liberiamo la memoria
  r->free();
  delete r;
  
  return EXIT_SUCCESS;
}

