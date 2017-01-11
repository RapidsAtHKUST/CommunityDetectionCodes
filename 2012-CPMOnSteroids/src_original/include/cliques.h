#ifndef CLIQUES_H
#define CLIQUES_H


#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <igraph.h>
#include "debug.h"

struct cliques{
  //  long int num_cliques;
  //  int max_k;

  igraph_vector_ptr_t maximal_cliques_v_ptr;
  igraph_vector_ptr_t plain_cliques_v_ptr;
  igraph_vector_t maximal_cliques_count_v;
  unsigned long int maximal_cliques_total;
  unsigned int k_max;
};
typedef struct cliques cliques;

extern int cliques_init(cliques **c);
extern int cliques_load_unordered_maximal_cliques_list(cliques *c, const char *path);
extern int cliques_rows_to_be_read(const cliques *c, unsigned int k, unsigned long int *rows);
extern igraph_vector_t* cliques_get_clique(cliques* c, int i);
extern uint8_t cliques_get_clique_size(cliques* c, int i);
extern uint8_t cliques_overlap_cliques(cliques* c, int i, int j);

#endif	/* CLIQUES_H */

