#ifndef GREEDY_H
#define GREEDY_H

#include "MersenneTwister.h"
#include "GreedyBase.h"
#include "Node.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <stack>
#include <map>
#include <algorithm>
using namespace std;


class Greedy : public GreedyBase{
 public:
  Greedy(MTRand *RR,int nnode,Node **node,int nmembers);
  virtual ~Greedy();
  virtual void initiate(void);
  virtual void calibrate(void);
  virtual void tune(void);
  virtual void prepare(bool sort);
  virtual void level(Node ***,bool sort);
  virtual void move(bool &moved);
  virtual void determMove(vector<int> &moveTo);
  virtual void eigenvector(void);

  vector<int> danglings;
  
  int Nempty;
  vector<int> mod_empty;
  
  vector<double> mod_exit;
  vector<double> mod_size;
  vector<double> mod_danglingSize;
  vector<double> mod_teleportWeight;
  vector<int> mod_members;
  
 protected:
  
  vector<int> modSnode;
};

#endif
