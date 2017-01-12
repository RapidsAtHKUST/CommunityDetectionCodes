#ifndef COMMON_H
#define COMMON_H

#include "dsforest.h"

struct constructor_thread_data {
  int start_from_row;
};

typedef struct constructor_thread_data constructor_thread_data;

struct community_thread_data {
  dsforest_t *thread_dsf;
  int start_from_row;
};

typedef struct community_thread_data community_thread_data;



#endif /* COMMON_H */
