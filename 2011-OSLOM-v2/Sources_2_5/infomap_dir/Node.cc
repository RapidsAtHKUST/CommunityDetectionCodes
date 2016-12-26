#include "Node.h"

Node::Node(){
  
  exit = 0.0;
  size = 0.0;
  selfLink = 0.0;
  
}


Node::Node(int nodenr,double tpweight){
  
  teleportWeight = tpweight;
  index = nodenr;
  exit = 0.0;
  size = 0.0;
  selfLink = 0.0;
  members.push_back(nodenr);
  
}
