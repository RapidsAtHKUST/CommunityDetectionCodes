#ifndef GREEDYBASE_H
#define GREEDYBASE_H
#include "MersenneTwister.h"
#include <cstdio>
#include <vector>
using namespace std;
// forward declaration
class Node;
class GreedyBase{
 public:
  GreedyBase(){};
  virtual ~GreedyBase(){};
  virtual void initiate(void){};
  virtual void tune(void){};
  virtual void calibrate(void){};
  virtual void prepare(bool sort){};
  virtual void level(Node ***,bool sort){};
  virtual void move(bool &moved){};
  virtual void determMove(vector<int> &moveTo){};
  virtual void eigenvector(void){};
  int Nmod;
  int Nnode;
  int Nmember;
  int Ndanglings;
   
  double exit;
  double exitFlow;
  double exit_log_exit;
  double size_log_size;
  double nodeSize_log_nodeSize;
  
  double codeLength;
 
  Node **node;
  bool bottom;
  double alpha,beta;

 protected:

  MTRand *R;
  
};

#endif
