#ifndef __EAGLE_H
#define __EAGLE_H

#include "Communities.h"
#include <string>

class MaxCache;

class EAGLE
{
private:
  igraph_t* graph;
  
  double factor;
  unsigned int k;
  std::string prefix;
  
  Communities* ic;
  
  void feedCache(MaxCache& cache);
  void updateCache(MaxCache& cache, unsigned int c);
  void getInitialCommunities();
public:
  EAGLE(igraph_t* graph, int k = 4, std::string prefix = ".serialized.");
  
  Communities* run();
};

#endif /* __EAGLE_H */
