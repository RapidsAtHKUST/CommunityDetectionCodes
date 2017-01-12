#include "include/cliques.h"
#include "include/debug.h"

static int cliques_count_maximal_cliques(cliques *c);
static int cliques_order_cliques_by_decreasing_k(cliques *c, const char *path);
static int cliques_init_member_vectors(cliques *c, int max_size);

int cliques_init(cliques **c) {
  if ((*c = (cliques*)malloc(sizeof(cliques))) == NULL)
    return -1;
  (*c)->maximal_cliques_total = 0;
  (*c)->k_max = 0;
  return 0;
}

int cliques_load_unordered_maximal_cliques_list(cliques *c, const char *path) {
    FILE *input;
    igraph_vector_t *k_clique_v;
    int size, node_id;
    int max_size=0,cur_size=0;
    if (c == NULL || path == NULL)
        return -1;

    if ((input = fopen(path, "r")) == NULL)
        return -2;

    k_clique_v = (igraph_vector_t*) malloc(sizeof (igraph_vector_t));
    igraph_vector_init(k_clique_v, 0);
    // read the file and compute the maximum size
    // of the cliques
    while (fscanf(input, "%i", &node_id) != EOF) {
      if (node_id == -1){
	if(cur_size > max_size)
	  max_size = cur_size;
        cur_size=0;
      } else
        cur_size++;
    }

    // initialize cliques structure internal vectors
    // according to the size of the maximum clique
    if((cliques_init_member_vectors(c, max_size)) < 0)
      return -3;

    // reset the file position indicator to the beginning of the file
    fseek(input, 0L, SEEK_SET);
    // load maximal cliques from the file


    while (fscanf(input, "%i", &node_id) != EOF) {
      if (node_id != -1){
        igraph_vector_push_back(k_clique_v, node_id);
      }else {
        size = igraph_vector_size(k_clique_v);
        igraph_vector_sort(k_clique_v);
        igraph_vector_ptr_push_back(VECTOR(c->maximal_cliques_v_ptr)[size], k_clique_v);
        
        k_clique_v = (igraph_vector_t*) malloc(sizeof (igraph_vector_t));
        igraph_vector_init(k_clique_v, 0);
      }
    }
    cliques_order_cliques_by_decreasing_k(c, NULL);
    igraph_vector_destroy(k_clique_v);
    return 0;
}

static int cliques_init_member_vectors(cliques *c, int max_size){
  igraph_vector_ptr_t *cur_v_ptr;
  int i;
  if(c == NULL)
    return -1;
  if(max_size <= 0)
    return -2;
  // init the vector which will contain, given a k, the total
  // number of maximal cliques with that k
  igraph_vector_init(&c->maximal_cliques_count_v, max_size+1);
  // since maximal k-cliques will be located at position k of c->maximal_cliques_v_ptr,
  // i.e. at VECTOR(c->maximal_cliques_v_ptr)[k], this vector pointers
  // must have size max_size+1 and not max_size
  igraph_vector_ptr_init(&c->maximal_cliques_v_ptr, max_size+1);
  // init its elements to null
  for (i = 0; i < max_size+1; i++) {
    cur_v_ptr = (igraph_vector_ptr_t*)malloc(sizeof (igraph_vector_ptr_t));
    igraph_vector_ptr_init(cur_v_ptr, 0);
    VECTOR(c->maximal_cliques_v_ptr)[i] = cur_v_ptr;
  }
  return 0;
}

static int cliques_count_maximal_cliques(cliques *c) {
    unsigned int k;
    unsigned long int count;
    if (c == NULL)
        return -1;
    for (k = igraph_vector_ptr_size(&c->maximal_cliques_v_ptr) - 1; k >= 3; k--) {
        count = igraph_vector_ptr_size((igraph_vector_ptr_t*)VECTOR(c->maximal_cliques_v_ptr)[k]);
        if (count == 0)
            continue;
        printf("k: %i total: %li\n\n", k, count);
        VECTOR(c->maximal_cliques_count_v)[k] = count;
        c->maximal_cliques_total += count;
    }
    debug((DEBUG_NORMAL, "maximal_cliques_total %li", c->maximal_cliques_total));
    return 0;
}

static int cliques_order_cliques_by_decreasing_k(cliques *c, const char *path) {
  unsigned int i, j, k;
    unsigned long int size;
    igraph_vector_t *k_clique_v;
    igraph_vector_ptr_t *maximal_k_cliques_v_ptr;
    FILE *output;
    if (c == NULL)
        return -1;
    cliques_count_maximal_cliques(c);


    igraph_vector_ptr_init(&c->plain_cliques_v_ptr, c->maximal_cliques_total);

    j = 0;
    for (k = igraph_vector_ptr_size(&c->maximal_cliques_v_ptr) - 1; k >= 3; k--) {
        maximal_k_cliques_v_ptr = (igraph_vector_ptr_t*) VECTOR(c->maximal_cliques_v_ptr)[k];
        size = igraph_vector_ptr_size(maximal_k_cliques_v_ptr);
        if (size == 0)
            continue;
        // initialize the k_max
        if (c->k_max == 0) c->k_max = k;
        // for each size-maximal-clique store its pointer also in the
        // plain_cliques_v_ptr
        for (i = 0; i < size; i++) {
            k_clique_v = (igraph_vector_t*) VECTOR(*maximal_k_cliques_v_ptr)[i];
            VECTOR(c->plain_cliques_v_ptr)[j++] = k_clique_v;
        }
    }

    // if path is not null try to write the previously computed
    // plain ordered list to the file specified by path
    if (path != NULL) {
        if ((output = fopen(path, "w")) == NULL)
            return -2;
        for (k = 0; k < c->maximal_cliques_total; k++) {
            k_clique_v = (igraph_vector_t*) VECTOR(c->plain_cliques_v_ptr)[k];
            size = igraph_vector_size(k_clique_v);
            for (i = 0; i < size; i++)
                fprintf(output, "%li ", (long int) VECTOR(*k_clique_v)[i]);
            fprintf(output, "-1%s\n", "");
        }
        fclose(output);
    }
    return 0;
}

int cliques_rows_to_be_read(const cliques *c, unsigned int k, unsigned long int *rows) {
    unsigned int i;
    *rows = 0;
    for (i = igraph_vector_size(&c->maximal_cliques_count_v) - 1; i >= k; i--) {
        if (VECTOR(c->maximal_cliques_count_v)[i] == 0)
            continue;
        *rows += VECTOR(c->maximal_cliques_count_v)[i];
    }
    return 0;
}

igraph_vector_t* cliques_get_clique(cliques* c, int i){
  return igraph_vector_ptr_e(&(c->plain_cliques_v_ptr), i);
}

uint8_t cliques_get_clique_size(cliques* c, int i){
  igraph_vector_t* ptr = igraph_vector_ptr_e(&(c->plain_cliques_v_ptr), i);
  return igraph_vector_size(ptr);
}

uint8_t cliques_overlap_cliques(cliques* c, int i, int j){
  igraph_vector_t *i_clique, *j_clique;
  uint8_t overlap_size = 0;
  int node_it;
  int clique_size;

  i_clique = (igraph_vector_t*)VECTOR(c->plain_cliques_v_ptr)[i];
  j_clique = (igraph_vector_t*)VECTOR(c->plain_cliques_v_ptr)[j];

  if (i == j)
    return igraph_vector_size(i_clique);
  
  if( (igraph_vector_tail(i_clique) >= VECTOR(*j_clique)[0]) && (VECTOR(*i_clique)[0] <= igraph_vector_tail(j_clique))){		
    clique_size = igraph_vector_size(j_clique);
    for (node_it = 0; node_it < clique_size; node_it++) {
      if (VECTOR(*j_clique)[node_it] < VECTOR(*i_clique)[0])
	// if the current node is lower than i_clique minimum node, then continue
	continue;
      if (VECTOR(*j_clique)[node_it] > igraph_vector_tail(i_clique))
	// if the current node is greater than i_clique maximum node, than exit
	break;
      // search element node_it^th of j_clique within i_clique vector
      if (igraph_vector_binsearch2(i_clique, VECTOR(*j_clique)[node_it]))
	overlap_size++;
    }
  }
  return overlap_size;
}

/*
cliques* load_cliques(){

	
  cliques* c = (cliques*) malloc(sizeof(cliques));		// allocating space for cliques structure
	
  if (c == NULL) {
    printf("ERROR creating cliques structure\n");
    return NULL;
  }
	
  // c->plain_cliques_v_ptr is the vector of pointers containing all the cliques
  igraph_vector_ptr_init(&(c->plain_cliques_v_ptr), 0);		// initializing internal igraph_vector

	
  FILE *cliques_file;
	
  cliques_file=fopen("./plain_cliques_v_ptr.txt","r");
  if(cliques_file==NULL){
    printf("ERROR opening file!\n");
    return NULL;
  }
	
  int as_id = 0;										// AS number igraph identifier read from file
  igraph_real_t AS_identifier;			
	
  long int clique_size = -1;
  long int max_clique_size = -1;
	
  // vector pointer allocating the correct memory space for each current clique
  igraph_vector_t* current_clique_pointer;
	
  // allocating space for the first clique
  current_clique_pointer = (igraph_vector_t*) malloc(sizeof(igraph_vector_t));		
  igraph_vector_init(current_clique_pointer, 0);
	
	
  while(fscanf( cliques_file, "%i", &as_id) !=EOF ){        
		
    if (as_id != -1) {					// -1 is the terminating integer 
      AS_identifier = as_id;
      igraph_vector_push_back(current_clique_pointer,  AS_identifier);		// insert AS_identifier (int) within the current clique vector
			
    }
    else {			
			
      clique_size = igraph_vector_size(current_clique_pointer);						// compute current clique size
			
      if (clique_size > 2) {		// if clique size is equal to 2 then "forget" the clique

	igraph_vector_sort(current_clique_pointer);									// sorting clique
	igraph_vector_ptr_push_back (&(c->plain_cliques_v_ptr), current_clique_pointer);	//  insert new clique within cliques matrix           
				
	if (max_clique_size < clique_size) {
	  max_clique_size = clique_size;
	}
				
      }
			
      current_clique_pointer = (igraph_vector_t*) malloc(sizeof(igraph_vector_t));	// allocating space for the next clique
      igraph_vector_init(current_clique_pointer, 0);									// current_clique vector initialization
			
    }
		
  }	
	
  fclose(cliques_file);

  c->num_cliques = igraph_vector_ptr_size(&(c->plain_cliques_v_ptr));
  c->max_k = max_clique_size;
	
  return c;
}
*/
