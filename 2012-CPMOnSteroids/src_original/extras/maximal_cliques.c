#include <igraph.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static void usage(const char *progname);


int main(int argc, char **argv){
  // DO NOT forget to attach the attibute table otherwise you cannot use attributes!
  igraph_i_set_attribute_table(&igraph_cattribute_table);

  long int i=0,j=0,ecount=0,vcount=0;
  igraph_vector_t *c=NULL;
  igraph_vector_ptr_t v_ptr;
  igraph_vector_ptr_init(&v_ptr, 0);
  igraph_t graph;
  FILE *input,*out;
  char outname[1024];


  if( argc!=2 ){
    usage(argv[0]);
    return -1;
  }


  if( (input=fopen(argv[1], "r")) == NULL ){
    printf("Unable to open input file... exiting.\n");
    return -1;
  }

  if( igraph_read_graph_ncol(&graph, input, NULL, 1/*TRUE*/,
                            IGRAPH_ADD_WEIGHTS_NO, IGRAPH_UNDIRECTED)
     == IGRAPH_PARSEERROR ){
    printf("Errors in parsing %s... exiting.\n", argv[1]);
    return -1;
  }

  fclose(input);
  ecount = (long int)igraph_ecount(&graph);
  vcount = (long int)igraph_vcount(&graph);

  printf("Graph loaded:\n");
  printf("\tNodes:\t%li\n", vcount);
  printf("\tEdges:\t%li\n", ecount);

  igraph_simplify(&graph, 1, 0, NULL);
  // check if multiple edges have been removed
  if( (long int)igraph_ecount(&graph) < ecount )
    printf("Removed %li multiple edges\n", ecount - (long int)igraph_ecount(&graph) );  
  ecount = (long int)igraph_ecount(&graph);

  

  igraph_simplify(&graph, 0, 1, NULL);
  // check if self edges have been removed
  if( (long int)igraph_ecount(&graph) < ecount )
    printf("Removed %li self edges\n", ecount - (long int)igraph_ecount(&graph) );  

  printf("Extracting maximal cliques... ");
  // extract maximal cliques without limiting maximum and minimum size
  igraph_maximal_cliques(&graph, &v_ptr, -1, -1);
  printf("done.\n");

  // write out found maximal cliques to file
  sprintf(outname, "%s.mcliques", basename(argv[1]) );

  if( (out=fopen(outname, "w")) == NULL ){
    printf("Unable to open output %s file... exiting.\n", outname);
    return -1;
  }

  for(i=0; i<igraph_vector_ptr_size(&v_ptr); i++){
    c = (igraph_vector_t*)VECTOR(v_ptr)[i];
    for(j=0; j<igraph_vector_size(c); j++){
      fprintf(out, "%li ", (long int)VECTOR(*c)[j]);
    }
    fprintf(out, "-1\n");
  }
  fclose(out);

  // write out the conversion between node symbolic name and associated integer
  sprintf(outname, "%s.map", basename(argv[1]) );

  if( (out=fopen(outname, "w")) == NULL ){
    printf("Unable to open output %s file... exiting.\n", outname);
    return -1;
  }

  for(i=0; i<vcount; i++){
    fprintf(out, "%s %li\n", VAS(&graph, "name", i), i);
  }
  fclose(out);
  
  return 0;
}

static void usage(const char* progname){
  printf("Usage:\n");
  printf("%s <input_edgelist_file>\n\n", progname);

  printf("Where:\n");
  printf(" <input_edgeslist_file> Is a generic symbolic unweighted edgelist file.\n"
         "                        It is a simple text file with one edge per line.\n"
         "                        An edge is defined by two symbolic vertex names\n"
         "                        separated by whitespace. The symbolic vertex names\n"
         "                        themselves cannot contain whitespace.\n\n");
 
   printf("Output:\n");
  printf("The first output file, with extension .mcliques, lists the extracted\n"
         "maximal cliques, one for each row. Each maximal clique is expressed as\n"
         "a whitespace-separated list of its nodes and ends with a virtual node -1.\n"
         "Prior to be output, each node is converted in a positive integer ranging\n"
         "from 0 to N-1, where N is the number of nodes in the graph.\n"
         "The association between symbolic node names and integers is output\n"
         "in the second file. The second output file, with extension .map, lists\n"
         "for each node its associated integer.\n\n");

  printf("Example:\n");
  printf("An example edgelist file of a fully-connected graph with 4 nodes is the following:\n"
         "node1 node2\n"
         "node1 N3\n"
         "node1 myFourthNode\n"
         "node2 N3\n"
         "node2 myFourthNode\n"
         "N3 myFourthNode\n\n");

  printf("By providing as input the previous edgelist file, the program produces the\n"
         "following two files.\n\n"
         "File .map:\n"
         "node1 0\n"
         "node2 1\n"
         "N3 2\n"
         "myFourthNode 3\n\n"

         "File .mcliques:\n"
         "0 1 2 3 -1\n\n");

  return;
}
