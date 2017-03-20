#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "MersenneTwister.h"
#include "GreedyBase.h" 
#include "Greedy.h" 
#include "Node.h" 
#define PI 3.14159265
using namespace std;

unsigned stou(char *s);

class treeNode{
 public:
  double exit;
  multimap<double,pair<int,string>,greater<double> > members;
  multimap<double,treeNode,greater<double> > nextLevel;
};

template <class T>
inline std::string to_string (const T& t){
  std::stringstream ss;
  ss << t;
  return ss.str();
}

void cpyNode(Node *newNode,Node *oldNode){
  
  newNode->index = oldNode->index;
  newNode->exit = oldNode->exit;
  newNode->size = oldNode->size;
  newNode->teleportWeight = oldNode->teleportWeight;
  newNode->danglingSize = oldNode->danglingSize;
  
  int Nmembers = oldNode->members.size();
  newNode->members = vector<int>(Nmembers);
  for(int i=0;i<Nmembers;i++)
    newNode->members[i] = oldNode->members[i];
  
  newNode->selfLink = oldNode->selfLink;
  
  int NoutLinks = oldNode->outLinks.size();
  newNode->outLinks = vector<pair<int,double> >(NoutLinks);
  for(int i=0;i<NoutLinks;i++){
    newNode->outLinks[i].first = oldNode->outLinks[i].first;
    newNode->outLinks[i].second = oldNode->outLinks[i].second;
  }
  
  int NinLinks = oldNode->inLinks.size();
  newNode->inLinks = vector<pair<int,double> >(NinLinks);
  for(int i=0;i<NinLinks;i++){
    newNode->inLinks[i].first = oldNode->inLinks[i].first;
    newNode->inLinks[i].second = oldNode->inLinks[i].second;
  }
  
}




  




