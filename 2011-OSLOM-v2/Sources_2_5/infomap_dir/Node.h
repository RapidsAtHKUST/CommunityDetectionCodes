#ifndef NODE_H
#define NODE_H

#include <cmath>
#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <stack>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <cstring>
class Node;
using namespace std;

class Node{
 public:
  
  Node();
  Node(int modulenr,double tpweight);
  vector<int> members;
  vector<pair<int,double> > inLinks;
  vector<pair<int,double> > outLinks;
  double selfLink;

  double teleportWeight;
  double danglingSize;
  double exit;
  double size;
  int index;
  
 protected:
  
};

#endif
