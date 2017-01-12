#include <pthread.h>
#include <igraph.h>
#include "include/common.h"
#include "include/dsforest.h"
#include "include/cliques.h"
#include "include/output_communities.h"

extern cliques *all_cliques;
extern int k_max;
extern dsforest_t **global_dsf;
extern pthread_mutex_t *global_dsf_mutexes;
extern int NUM_THREADS;

static int cospoc_thread_epilogue(dsforest_t **thread_dsfs){
  int i,k;
  int root_i, global_dsf_root_i, global_dsf_root_root_i;
  unsigned long int rows_to_be_read;

  for(k=3; k<= k_max; k++){
    cliques_rows_to_be_read(all_cliques, k, &rows_to_be_read);

    pthread_mutex_lock(&global_dsf_mutexes[k-3]);
    /*
      BEGIN OF CRITICAL SECTION
    */
    for(i=0; i < rows_to_be_read; i++){
      root_i = dsforest_find(thread_dsfs[k-3], i);
      if(i == root_i)
	// there is no need to go further since i is the root
	// of a disjoint set
	continue;
      // INVARIANT: i and root_i must belong to the same disjoint set (i.e. the same connected
      // component) in the k-th global disjoint sets forest: gloabal_dsf[k]
      global_dsf_root_i = dsforest_find(global_dsf[k-3], i);
      global_dsf_root_root_i = dsforest_find(global_dsf[k-3], root_i);
      if(global_dsf_root_i == global_dsf_root_root_i)
	// they are yet in the same set
	continue;
      // merge the two components in the global disjoint set forests:
      // these components contains i and root_i respectively
      dsforest_union(global_dsf[k-3], global_dsf_root_i, global_dsf_root_root_i);
    }

    pthread_mutex_unlock(&global_dsf_mutexes[k-3]);
    /*
      END OF CRITICAL SECTION
    */
  }
  return 0;
}

static void cospoc_thread_body(void* params){
  int *overlaps;
  struct constructor_thread_data* ctd = (struct constructor_thread_data*) params;
  int i = ctd->start_from_row;		// row iterator
  int j = 0;				// column itearator
  int root_i, root_j;
  int k;
  dsforest_t **thread_dsfs;

  thread_dsfs = (dsforest_t**)malloc(sizeof(dsforest_t*) * (k_max - 2));
  for(k=3; k<=k_max; k++){
    unsigned long int disjoint_sets_number;
    cliques_rows_to_be_read(all_cliques, k, &disjoint_sets_number);
    dsforest_init(&thread_dsfs[k-3], disjoint_sets_number);
  }

  overlaps = (int*)malloc(sizeof(int) * all_cliques->maximal_cliques_total);

  

  for (; i < all_cliques->maximal_cliques_total; i+=NUM_THREADS) {


    /*
    for ( j = i+1; j < all_cliques->maximal_cliques_total ; j++)
      overlaps[j] =  cliques_overlap_cliques(all_cliques, i, j);
    */

    int j_old = i+1;
    for(k=k_max; k>=3; k--){
      unsigned long int rows_to_be_read;
      
      cliques_rows_to_be_read(all_cliques, k, &rows_to_be_read);

      if(i >= rows_to_be_read)
	continue;

      root_i = dsforest_find(thread_dsfs[k-3], i);

      for ( j = i+1; j < rows_to_be_read; j++){

	if(j >= j_old)
	  overlaps[j] =  cliques_overlap_cliques(all_cliques, i, j);

	if (overlaps[j] < k-1)
	  continue;

	root_j = dsforest_find(thread_dsfs[k-3], j);

	if(root_i != root_j){
	  // since after the union the root node of i, i.e. i_root, could be changed
	  // we must verify if it has become a non-root node and update it accordingly.
	  // i_root can change if it has been linked as son of j_root because of
	  // a smaller rank
	  dsforest_union(thread_dsfs[k-3], root_i, root_j);

	  if(!dsforest_is_root(thread_dsfs[k-3], root_i))
	    root_i = dsforest_find(thread_dsfs[k-3], i);

	}
      }
      j_old = j;
    }
  }
  cospoc_thread_epilogue(thread_dsfs);

  for(k=3; k<=k_max; k++)
    dsforest_destroy(thread_dsfs[k-3]);

  free(thread_dsfs);
  free(overlaps);

  pthread_exit(NULL);
}

int cospoc() {
  // THREADS
  pthread_t* cospoc_threads = NULL;
  constructor_thread_data* cospoc_thread_data = NULL;

  pthread_attr_t attr;				// pthread attribute
  void *status;					// pthread status
	
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // FLAGS
  int thread_flag;			// thread flag (create and join)

  // UTILITIES 
  int i=0; // iterators

  /***********************************************************************
   Initialization of threads parameters
  ***********************************************************************/
  cospoc_thread_data = (constructor_thread_data*) malloc(sizeof(constructor_thread_data) * NUM_THREADS);	
  for (i = 0; i < NUM_THREADS; i++) {
    // tell each constructor thread the first index it will start processing the chunk
    cospoc_thread_data[i].start_from_row = i;
  }

  /***********************************************************************
   Initialization of communities structure
  ***********************************************************************/
  // allocate global disjoint set forests for each k between 3 and k_max
  global_dsf = (dsforest_t**)malloc(sizeof(dsforest_t*) * (k_max - 2));

  // allocate space for the mutexes that will assure mutual exclusion
  // while accessing global_dsf and communities_done
  global_dsf_mutexes = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * (k_max - 2));

  for (i = 0; i < (k_max - 2); i++) {
    // compute the numer of maximal cliques which belong to at least one community
    // this number is used for allocating the right number of independent set
    // data structures. Recall that thread i will compute (i+3)-clique communities
    unsigned long int disjoint_sets_number;
    cliques_rows_to_be_read(all_cliques, i+3, &disjoint_sets_number);
    dsforest_init(&global_dsf[i], disjoint_sets_number);
    pthread_mutex_init(&global_dsf_mutexes[i], NULL);
  }
  /****************************************************************
     create NUM_THREADS threads which are able to read all_cliques
     structure and are able to write a part of current window (m)
     each const_thread has its own data structure
  ****************************************************************/
  cospoc_threads = (pthread_t*) malloc(sizeof(pthread_t) * NUM_THREADS);	

  for (i = 0; i < NUM_THREADS; i++) {
    thread_flag = pthread_create(&cospoc_threads[i], &attr, (void*)(&cospoc_thread_body), (void*)(&cospoc_thread_data[i]));
    if (thread_flag != 0) {
      printf("pthread_create ERROR!\n");
      return -1;
    }
  }
  // WAITING FOR NUM_THREADS THREADS "ENDINGS"
  for (i = 0; i < NUM_THREADS; i++) {
    thread_flag = pthread_join(cospoc_threads[i], &status);
    if (thread_flag != 0) {
      printf("pthread_join ERROR!\n");
      return -1;
    }
  }
  // DEALLOCATING MEMORY for constructor threads
  free(cospoc_threads);

  // write out found communities
  write_found_communities_to_file();

  // deallocate memory used for disjoint set forests
  for(i=0; i < (k_max - 2); i++){
    dsforest_destroy(global_dsf[i]);
    pthread_mutex_destroy(&global_dsf_mutexes[i]);
  }
  free(global_dsf);
  free(global_dsf_mutexes);

  // DEALLOCATING MEMORY for thread parameters
  free(cospoc_thread_data);

  pthread_attr_destroy(&attr);
  
  return 0;
}

