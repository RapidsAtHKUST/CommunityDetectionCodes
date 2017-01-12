#include <pthread.h>
#include "include/matrix.h"
#include "include/common.h"
#include "include/dsforest.h"
#include "include/cliques.h"
#include "include/output_communities.h"

extern cliques *all_cliques;
extern int k_max;
extern dsforest_t **global_dsf;
extern pthread_mutex_t *global_dsf_mutexes;
extern int NUM_THREADS;
extern unsigned long int WINDOW_SIZE;

static matrix *m;

static void constructor_thread_body(void* params){
  uint8_t overlap_value;
  struct constructor_thread_data* ctd = (struct constructor_thread_data*) params;
  int i = ctd->start_from_row;		// row iterator
  int j = 0;				// column itearator
  int counter = 0;

  for (; matrix_row_in_window(m,i) == TRUE; i+=NUM_THREADS) {
    for ( j = i+1; j < all_cliques->maximal_cliques_total ; j++) {
      overlap_value =  cliques_overlap_cliques(all_cliques, i, j);
      matrix_set(m, i, j, overlap_value);
      counter++;	
    }
  }
  pthread_exit(NULL);
}

static int community_thread_epilogue(dsforest_t *thread_dsf, unsigned int k, unsigned long int rows_to_be_read){
  int i;
  int root_i, global_dsf_root_i, global_dsf_root_root_i;

  pthread_mutex_lock(&global_dsf_mutexes[k-3]);
  /*
    BEGIN OF CRITICAL SECTION
  */
  for(i=0; i < rows_to_be_read; i++){
    root_i = dsforest_find(thread_dsf, i);
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
  return 0;
}

static void community_thread_body(void* params){
  int root_i, root_j;
  // matrix iterators
  int i=-1,j;
  int k;
  uint8_t overlapping_value;
  unsigned long int rows_to_be_read;
  dsforest_t *thread_dsf;

  // cast the void* pointer to the struct used for passing us our parameters
  // and get them
  struct community_thread_data* ctdata = (struct community_thread_data*) params;

  dsforest_init(&thread_dsf, all_cliques->maximal_cliques_total);

  for(k=k_max; k >= 3; k--){
    // get the first index in the window
    i = ctdata->start_from_row;
    // get the number of rows that must be read, given a k
    cliques_rows_to_be_read(all_cliques, k, &rows_to_be_read);
    if(i >= rows_to_be_read)
      continue;

    for (;matrix_row_in_window(m,i) == TRUE && i < rows_to_be_read; i+=NUM_THREADS) {
      root_i = dsforest_find(thread_dsf, i);
      j = i+1;
      for (; j < rows_to_be_read; j++) {
	// overlapping
	if(matrix_get(m, i, j, &overlapping_value) != 0){
	  printf("Error reading the matrix\n");
	  pthread_exit(NULL);
	}

	if (overlapping_value < k-1)
	  continue;

	overlapping_value=0;
	if(matrix_set(m, i, j, overlapping_value) != 0){
	  printf("Error writing the matrix\n");
	  pthread_exit(NULL);
	}	

	root_j = dsforest_find(thread_dsf, j);

	if(root_i != root_j){
	  // since after the union the root node of i, i.e. i_root, could be changed
	  // we must verify if it has become a non-root node and update it accordingly.
	  // i_root can change if it has been linked as son of j_root because of
	  // a smaller rank
	  dsforest_union(thread_dsf, root_i, root_j);
	  if(!dsforest_is_root(thread_dsf, root_i))
	    root_i = dsforest_find(thread_dsf, i);
	}
      }
    }
    community_thread_epilogue(thread_dsf, k, rows_to_be_read);
  }
  dsforest_destroy(thread_dsf);
  pthread_exit(NULL);
}

int cos_() {
  // THREADS
  pthread_t* const_threads = NULL;		    // constructor threads
  constructor_thread_data* const_thread_data = NULL;  // constructor threads data
	
  pthread_t* comm_threads = NULL;                	// community threads
  community_thread_data* comm_thread_data = NULL;	// community threads data

  pthread_attr_t attr;				// pthread attribute
  void *status;					// pthread status
	
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  // FLAGS
  int m_check = -1;			// control flag which verifies matrix_init()
  int slide_fw = -1;			// slide_forward control flag
  int thread_flag;			// thread flag (create and join)

  // UTILITIES 
  int i=0; // iterators
  int chunk_rows = 0;      // keeps the number of rows being processed during this chunk
  int rows_processed = 0;  // keeps the number of rows processed

  /***********************************************************************
   matrix_init() initializes matrix structure (malloc inside),
   the second parameter is equal to the number of cliques.
  ***********************************************************************/
  m_check = matrix_init(&m, all_cliques->maximal_cliques_total);	

  if (m_check != 0) {
    printf("ERROR in matrix_init()!\n");
    matrix_destroy(m);
    m = NULL;
    return -1;
  }

  //	If the program is here, m contains the first window

  /***********************************************************************
   Initialization of threads parameters
  ***********************************************************************/
  const_thread_data = (constructor_thread_data*) malloc(sizeof(constructor_thread_data) * NUM_THREADS);	
  for (i = 0; i < NUM_THREADS; i++) {
    // tell each constructor thread the first index it will start processing the chunk
    const_thread_data[i].start_from_row = i;
  }
	
  comm_thread_data = (community_thread_data*) malloc(sizeof(community_thread_data) * NUM_THREADS);	
  for (i = 0; i < NUM_THREADS; i++) {
    // initialize a disjoint set forest specific to each thread. It will use that forest
    // for processing the overlap between maximal cliques of every k and hence it must
    // be of maximum size
    comm_thread_data[i].start_from_row = i;
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

  int chunk = 0;			// chunk counter

  /****************************************************************
   MAIN LOOP BEGINS HERE...
  ****************************************************************/
  do {
    chunk++;
    printf("Chunk %d\n", chunk);
		
    /****************************************************************
     create NUM_THREADS threads which are able to read all_cliques
     structure and are able to write a part of current window (m)
     each const_thread has its own data structure
    ****************************************************************/
    const_threads = (pthread_t*) malloc(sizeof(pthread_t) * NUM_THREADS);	

    for (i = 0; i < NUM_THREADS; i++) {
      // tell the thread the right index from which it has to start processing
      const_thread_data[i].start_from_row = i + rows_processed;
      thread_flag = pthread_create(&const_threads[i], &attr, (void*)(&constructor_thread_body), (void*)(&const_thread_data[i]));
      if (thread_flag != 0) {
	printf("pthread_create ERROR!\n");
	return -1;
      }
    }
		
    //    printf("Waiting for %d constructor threads\n", NUM_THREADS);
		
    // WAITING FOR NUM_THREADS CONSTRUCTOR THREAD "ENDINGS"
    for (i = 0; i < NUM_THREADS; i++) {
      thread_flag = pthread_join(const_threads[i], &status);
      if (thread_flag != 0) {
	printf("pthread_join ERROR!\n");
	return -1;
      }
    }
		
    // DEALLOCATING MEMORY for constructor threads
    free(const_threads);
				
    /****************************************************************
     create NUM_THREADS threads which are able to read matrix structure
     and are able to compute communities
     each comm_thread has its own data structure
    ****************************************************************/
		
    comm_threads = (pthread_t*) malloc(sizeof(pthread_t) * NUM_THREADS);
				

    for (i = 0; i < NUM_THREADS; i++) {
      // tell the thread the right index from which it has to start processing
      comm_thread_data[i].start_from_row = i + rows_processed;
      thread_flag = pthread_create(&comm_threads[i], &attr, (void*)(&community_thread_body), (void*)(&comm_thread_data[i]));
      if (thread_flag != 0) {
	printf("pthread_create ERROR!\n");
	return -1;
      }
    }
    // printf("Waiting for %d community threads\n", NUM_THREADS);
    // WAITING FOR NUM_THREADS COMMUNITY THREAD "ENDINGS"
    for (i = 0; i < NUM_THREADS; i++) {
      thread_flag = pthread_join(comm_threads[i],&status);
      if (thread_flag != 0) {
	printf("pthread_join ERROR!\n");
	return -1;
      }
    }
		
    // DEALLOCATING MEMORY for community threads
    free(comm_threads);

    // before sliding the window forward we have to keep track of
    // how many rows have yet been processed
    matrix_get_num_rows_in_window(m, &chunk_rows);
    debug((DEBUG_NORMAL, "\tchunk_rows: %i", chunk_rows));

    rows_processed += chunk_rows;
    debug((DEBUG_NORMAL, "\trows_processed: %i", rows_processed));
    slide_fw = matrix_slide_forward(m);

  } while ( slide_fw != I_END_OF_MATRIX_REACHED );

  printf("Done processing matrix chunks...\n");

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
  free(const_thread_data);
  free(comm_thread_data);

  pthread_attr_destroy(&attr);
  
  return 0;
}

