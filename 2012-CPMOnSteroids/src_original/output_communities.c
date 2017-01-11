#include <pthread.h>
#include <igraph.h>
#include "include/dsforest.h"
#include "include/cliques.h"
#include "include/output_communities.h"

extern cliques *all_cliques;
extern int k_max;
extern dsforest_t **global_dsf;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int write_found_communities_to_file_thread(void *k_param){
  int j,z,k;
  // used for labeling communities with an incremental id
  int comm_id, this_comm_id;
  int *community_ids;
  int current_clique_size;
  char k_file_name[2048];
  FILE *k_communities_file,*k_num_com;
  int root_comm;
  igraph_vector_t *current_clique;
  long unsigned int rows_to_be_read;

  k = (intptr_t)k_param;
  cliques_rows_to_be_read(all_cliques, k, &rows_to_be_read);  

  comm_id = 0;
  current_clique_size=0;


  sprintf (k_file_name, "./%i_communities.txt", k);
  k_communities_file=fopen(k_file_name,"w");
  if(k_communities_file==NULL){
    printf("ERROR opening file!\n");
    return -1;
  }
  
  // initialize an array which will contain community id for each maximal clique
  community_ids = (int*)malloc(sizeof(int)*rows_to_be_read);
  // set every id to a no-id value
  for(j=0; j < rows_to_be_read; j++)
    community_ids[j] = NO_COMMUNITY_ID_YET;

  for (j = 0; j < rows_to_be_read; j++) {
    root_comm = dsforest_find(global_dsf[k-3], j);

    if(community_ids[root_comm] == NO_COMMUNITY_ID_YET){
      community_ids[root_comm] = comm_id++;
    }
    this_comm_id = community_ids[root_comm];

    current_clique = cliques_get_clique(all_cliques, j);
    current_clique_size = cliques_get_clique_size(all_cliques, j);

    fprintf(k_communities_file, "%i:", this_comm_id);
    for (z = 0; z < current_clique_size; z++)
      fprintf(k_communities_file, "%i ", (int)VECTOR(*current_clique)[z]);
    fprintf(k_communities_file, "\n");
  }
  fclose(k_communities_file);	
  free(community_ids);
  printf("Found\t%i\t%i-clique-communities\n", comm_id, k);
  // since this file is accessed concurrently we must
  // acquire a lock first and the write to it
  pthread_mutex_lock(&mutex);
  k_num_com=fopen("./k_num_communities.txt","a+");
  if(k_num_com==NULL){
    printf("ERROR opening file!\n");
    return -2;
  }
  // maximum comm_id is equal to the number of communities found
  fprintf(k_num_com, "%i\t%i\n", k, comm_id);
  fclose(k_num_com);
  pthread_mutex_unlock(&mutex);
  return 0;
}

int write_found_communities_to_file(){
  // THREADS
  pthread_t* output_threads = NULL;		    // constructor threads

  pthread_attr_t attr;				// pthread attribute
  void *status;					// pthread status
	
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  int thread_flag;			// thread flag (create and join)
  intptr_t k;

  output_threads = (pthread_t*)malloc(sizeof(pthread_t) * (k_max - 2));
  for (k = 3; k <= k_max; k++) {
    thread_flag = pthread_create(&output_threads[k-3], &attr, (void*)(&write_found_communities_to_file_thread), (void*)k);
    if (thread_flag != 0) {
      printf("pthread_create ERROR!\n");
      return -1;
    }
  }
  // printf("Waiting for %d community threads\n", COM);
  // WAITING FOR COMMUNITY THREAD "ENDINGS"
  for (k = 3; k <= k_max; k++) {
    thread_flag = pthread_join(output_threads[k-3],&status);
    if (thread_flag != 0) {
      printf("pthread_create ERROR!\n");
      return -1;
    }
  }
  free(output_threads);
  return 0;
}

