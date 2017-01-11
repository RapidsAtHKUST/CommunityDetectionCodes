#ifndef DSFOREST_H
#define DSFOREST_H

#include <stdlib.h>
#include <stdio.h>

#define NO_COMMUNITY_ID_YET   -1

typedef struct forest_node {
  int label;
  int comm_id;
  struct forest_node* parent;
  int rank;
}forest_node_t;

typedef struct dsforest {
  unsigned int num_elements;
  forest_node_t** elements;
}dsforest_t;

extern int dsforest_init(dsforest_t **dsf, unsigned int num_elements);
extern int dsforest_clear(dsforest_t *dsf);
extern int dsforest_destroy(dsforest_t *dsf);
extern int dsforest_union(dsforest_t *dsf, unsigned int root1_label, unsigned int root2_label);
extern int dsforest_find(dsforest_t *dsf, unsigned int node_label);
extern int dsforest_is_root(dsforest_t *dsf, unsigned int label);

#endif /* DSFOREST_H */
