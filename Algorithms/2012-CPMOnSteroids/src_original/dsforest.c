#include "include/dsforest.h"

static int clear_node(forest_node_t* node){
  node->comm_id = NO_COMMUNITY_ID_YET;
  node->parent = NULL;
  node->rank = 0;  
  return 0;
}

static forest_node_t* make_set(int label) {
  forest_node_t* node = (forest_node_t*)malloc(sizeof(forest_node_t));
  clear_node(node);
  node->label = label;
  return node;
}

static forest_node_t* find(forest_node_t* node) {
  forest_node_t *root;
  if(node->parent==NULL)
    // we have a root node
    return node;
  root = find(node->parent);
  node->parent = root;
  return root;
}

static int union_(forest_node_t* node1, forest_node_t* node2) {
  if (node1->rank > node2->rank) {
    node2->parent = node1;
  } else if (node2->rank > node1->rank) {
    node1->parent = node2;
  } else { /* they are equal */
    node2->parent = node1;
    node1->rank++;
  }
  return 0;
}

extern int dsforest_init(dsforest_t **dsf, unsigned int num_elements){
  int i;
  if ((*dsf = (dsforest_t*)malloc(sizeof(dsforest_t))) == NULL)
    return -1;
  if (((*dsf)->elements = (forest_node_t**)malloc(sizeof(forest_node_t*)*num_elements)) == NULL)
    return -2;
  (*dsf)->num_elements = num_elements;
  for(i=0; i<num_elements; i++)
    (*dsf)->elements[i] = make_set(i);
  return 0;
}

extern int dsforest_clear(dsforest_t *dsf){
  int i;
  for(i=0; i<dsf->num_elements; i++)
    clear_node(dsf->elements[i]);
  return 0;
}

extern int dsforest_destroy(dsforest_t *dsf){
  int i;
  for(i=0; i<dsf->num_elements; i++)
    free(dsf->elements[i]);
  free(dsf->elements);
  free(dsf);
  return 0;
}

extern int dsforest_find(dsforest_t *dsf, unsigned int node_label){
  forest_node_t *node_ptr, *node_root_ptr;  
  if(node_label >= dsf->num_elements)
    return -1;
  // get the pointer to the node with label node_label which is located
  // in the node_label-th entry of the array dsf->elements
  node_ptr = dsf->elements[node_label];
  node_root_ptr = find(node_ptr);
  return node_root_ptr->label;
}

extern int dsforest_union(dsforest_t *dsf, unsigned int root1_label, unsigned int root2_label) {
  forest_node_t *r1_ptr,*r2_ptr;
  if(root1_label >= dsf->num_elements || root2_label >= dsf->num_elements)
    return -1;
  // get the pointers to the nodes with the right labels
  r1_ptr = dsf->elements[root1_label];
  r2_ptr = dsf->elements[root2_label];
  // they must be root nodes
  if (r1_ptr->parent != NULL || r2_ptr->parent != NULL){
    printf("HERE\n");
    return -2;
  }
  return union_(r1_ptr, r2_ptr);
}

extern int dsforest_is_root(dsforest_t *dsf, unsigned int label){
  return (dsf->elements[label]->parent==NULL) ? 1 : 0;
}
