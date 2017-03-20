#include "Greedy.h"
#define plogp( x ) ( (x) > 0.0 ? (x)*log(x) : 0.0 )

Greedy::~Greedy(){	
  vector<int>().swap(modSnode);
}

Greedy::Greedy(MTRand *RR,int nnode,Node **ah,int nmember){
  
  R = RR;
  Nnode = nnode;  
  Nmember = nmember;
  node = ah;
  Nmod = Nnode;
  
  alpha = 0.15; // teleportation probability
  beta = 1.0-alpha; // probability to take normal step
  
  Ndanglings = 0;
  
}

void Greedy::move(bool &moved){
  
  // Generate random enumeration of nodes
  vector<int> randomOrder(Nnode);
  for(int i=0;i<Nnode;i++)
    randomOrder[i] = i;
  for(int i=0;i<Nnode-1;i++){
    int randPos = i + R->randInt(Nnode-i-1);
    int tmp = randomOrder[i];
    randomOrder[i] = randomOrder[randPos];
    randomOrder[randPos] = tmp;
  }
  
  unsigned int offset = 1;    
  vector<unsigned int> redirect(Nnode,0);
  vector<pair<int,pair<double,double> > > flowNtoM(Nnode);
  
  for(int k=0;k<Nnode;k++){
    
    // Pick nodes in random order
    int flip = randomOrder[k]; 
    int oldM = node[flip]->index; 
    
    // Reset offset when int overflows
    if(offset > INT_MAX){ 
      for(int j=0;j<Nnode;j++)
        redirect[j] = 0;
      offset = 1;
    }  
    
    // Size of vector with module links
    int NmodLinks = 0;
    
    // For all outLinks
    int NoutLinks = node[flip]->outLinks.size();
    if(NoutLinks == 0){ //dangling node, add node to calculate flow below
      redirect[oldM] = offset + NmodLinks;
      flowNtoM[NmodLinks].first = oldM;
      flowNtoM[NmodLinks].second.first = 0.0;
      flowNtoM[NmodLinks].second.second = 0.0;
      NmodLinks++;
    }
    else{
      for(int j=0; j<NoutLinks; j++){
        int nb_M = node[node[flip]->outLinks[j].first]->index;
        double nb_flow = node[flip]->outLinks[j].second;
        if(redirect[nb_M] >= offset){
          flowNtoM[redirect[nb_M] - offset].second.first += nb_flow;
        }
        else{
          redirect[nb_M] = offset + NmodLinks;
          flowNtoM[NmodLinks].first = nb_M;
          flowNtoM[NmodLinks].second.first = nb_flow;
          flowNtoM[NmodLinks].second.second = 0.0;
          NmodLinks++;
        }
      }
    }
    
    // For all inLinks
    int NinLinks = node[flip]->inLinks.size();
    for(int j=0; j<NinLinks; j++){
      int nb_M = node[node[flip]->inLinks[j].first]->index;
      double nb_flow = node[flip]->inLinks[j].second;
      
      if(redirect[nb_M] >= offset){
        flowNtoM[redirect[nb_M] - offset].second.second += nb_flow;
      }
      else{
        redirect[nb_M] = offset + NmodLinks;
        flowNtoM[NmodLinks].first = nb_M;
        flowNtoM[NmodLinks].second.first = 0.0;
        flowNtoM[NmodLinks].second.second = nb_flow;
        NmodLinks++;
      }
    }
    
    // For teleportation and dangling nodes
    for(int j=0;j<NmodLinks;j++){
      int newM = flowNtoM[j].first;
      if(newM == oldM){
        flowNtoM[j].second.first += (alpha*node[flip]->size + beta*node[flip]->danglingSize)*(mod_teleportWeight[oldM]-node[flip]->teleportWeight);
        flowNtoM[j].second.second += (alpha*(mod_size[oldM]-node[flip]->size) + beta*(mod_danglingSize[oldM]-node[flip]->danglingSize))*node[flip]->teleportWeight;
      }
      else{
        flowNtoM[j].second.first += (alpha*node[flip]->size + beta*node[flip]->danglingSize)*mod_teleportWeight[newM];
        flowNtoM[j].second.second += (alpha*mod_size[newM] + beta*mod_danglingSize[newM])*node[flip]->teleportWeight;
      }
    }
    
    // Calculate flow to/from own module (default value if no link to own module)
    double outFlowOldM = (alpha*node[flip]->size + beta*node[flip]->danglingSize)*(mod_teleportWeight[oldM]-node[flip]->teleportWeight);
    double inFlowOldM = (alpha*(mod_size[oldM]-node[flip]->size) + beta*(mod_danglingSize[oldM]-node[flip]->danglingSize))*node[flip]->teleportWeight;
    if(redirect[oldM] >= offset){
      outFlowOldM = flowNtoM[redirect[oldM] - offset].second.first;
      inFlowOldM = flowNtoM[redirect[oldM] - offset].second.second;   
    }
    
    // Option to move to empty module (if node not already alone)
    if(mod_members[oldM] > static_cast<int>(node[flip]->members.size())){
      if(Nempty > 0){
        flowNtoM[NmodLinks].first = mod_empty[Nempty-1];
        flowNtoM[NmodLinks].second.first = 0.0;
        flowNtoM[NmodLinks].second.second = 0.0;
        NmodLinks++;
      }
    }
    
    // Randomize link order for optimized search 
    for(int j=0;j<NmodLinks-1;j++){
      int randPos = j + R->randInt(NmodLinks-j-1);
      int tmp_M = flowNtoM[j].first;
      double tmp_outFlow = flowNtoM[j].second.first;
      double tmp_inFlow = flowNtoM[j].second.second;
      flowNtoM[j].first = flowNtoM[randPos].first;
      flowNtoM[j].second.first = flowNtoM[randPos].second.first;
      flowNtoM[j].second.second = flowNtoM[randPos].second.second;
      flowNtoM[randPos].first = tmp_M;
      flowNtoM[randPos].second.first = tmp_outFlow;
      flowNtoM[randPos].second.second = tmp_inFlow;
    }
    
    int bestM = oldM;
    double best_outFlow = 0.0;
    double best_inFlow = 0.0;
    double best_delta = 0.0;
    
    // Find the move that minimizes the description length
    for (int j=0; j<NmodLinks; j++) {
      
      int newM = flowNtoM[j].first;
      double outFlowNewM = flowNtoM[j].second.first;
      double inFlowNewM = flowNtoM[j].second.second;
      
      if(newM != oldM){
        
        double delta_exit = plogp(exitFlow + outFlowOldM + inFlowOldM - outFlowNewM - inFlowNewM) - exit;
        
        double delta_exit_log_exit = - plogp(mod_exit[oldM]) - plogp(mod_exit[newM]) \
        + plogp(mod_exit[oldM] - node[flip]->exit + outFlowOldM + inFlowOldM) + plogp(mod_exit[newM] + node[flip]->exit - outFlowNewM - inFlowNewM);
        
        double delta_size_log_size = - plogp(mod_exit[oldM] + mod_size[oldM]) - plogp(mod_exit[newM] + mod_size[newM]) \
        + plogp(mod_exit[oldM] + mod_size[oldM] - node[flip]->exit - node[flip]->size + outFlowOldM + inFlowOldM) \
        + plogp(mod_exit[newM] + mod_size[newM] + node[flip]->exit + node[flip]->size - outFlowNewM - inFlowNewM);
        
        double deltaL = delta_exit - 2.0*delta_exit_log_exit + delta_size_log_size;
        
        if(deltaL < best_delta){
          bestM = newM;
          best_outFlow = outFlowNewM;
          best_inFlow = inFlowNewM;
          best_delta = deltaL;  
        }	  
      }
    }
    
    // Make best possible move
    if(bestM != oldM){
      
      //Update empty module vector
      if(mod_members[bestM] == 0){
        Nempty--;
      }
      if(mod_members[oldM] == static_cast<int>(node[flip]->members.size())){
        mod_empty[Nempty] = oldM;
        Nempty++;
      }
      
      exitFlow -= mod_exit[oldM] + mod_exit[bestM];
      exit_log_exit -= plogp(mod_exit[oldM]) + plogp(mod_exit[bestM]);
      size_log_size -= plogp(mod_exit[oldM] + mod_size[oldM]) + plogp(mod_exit[bestM] + mod_size[bestM]); 
      
      mod_exit[oldM] -= node[flip]->exit - outFlowOldM - inFlowOldM;
      mod_size[oldM] -= node[flip]->size;
      mod_danglingSize[oldM] -= node[flip]->danglingSize;
      mod_teleportWeight[oldM] -= node[flip]->teleportWeight;
      mod_members[oldM] -= node[flip]->members.size();
      mod_exit[bestM] += node[flip]->exit - best_outFlow - best_inFlow;
      mod_size[bestM] += node[flip]->size;
      mod_danglingSize[bestM] += node[flip]->danglingSize;
      mod_teleportWeight[bestM] += node[flip]->teleportWeight;
      mod_members[bestM] += node[flip]->members.size();
      
      exitFlow += mod_exit[oldM] + mod_exit[bestM];
      
      // Update terms in map equation
      
      exit_log_exit += plogp(mod_exit[oldM]) + plogp(mod_exit[bestM]);
      size_log_size += plogp(mod_exit[oldM] + mod_size[oldM]) + plogp(mod_exit[bestM] + mod_size[bestM]); 
      exit = plogp(exitFlow);
      
      // Update code length
      
      codeLength = exit - 2.0*exit_log_exit + size_log_size - nodeSize_log_nodeSize;
      
      node[flip]->index = bestM;
      moved = true;
    }
    
    offset += Nnode;
    
  }
  
}

void Greedy::initiate(void){
  
  // Take care of dangling nodes, normalize outLinks, and calculate total teleport weight
  for(int i=0;i<Nnode;i++){
    
    if(node[i]->outLinks.empty() && (node[i]->selfLink <= 0.0)){
      danglings.push_back(i);
      Ndanglings++;
    }
    else{ // Normalize the weights
      int NoutLinks = node[i]->outLinks.size();
      double sum = node[i]->selfLink; // Take care of self-links
      for(int j=0;j < NoutLinks; j++)
        sum += node[i]->outLinks[j].second;
      
      node[i]->selfLink /= sum;
      for(int j=0;j < NoutLinks; j++)
        node[i]->outLinks[j].second /= sum;
    }
  }
  
  //   if(Ndanglings > 0){
  //     cout << "Found " << Ndanglings << " dangling node(s)." << endl;
  //   }
  
  // Calculate steady state matrix
  eigenvector();
  
  //  // generate temporary inlink index
  //  vector<map<int,int> > linkIndex = vector<map<int,int> >(Nnode);
  //  for(int i=0;i<Nnode;i++){
  //    int NoutLinks = node[i]->outLinks.size();
  //    for(int j=0; j < NoutLinks; j++){
  //      linkIndex[i].insert(make_pair(node[i]->outLinks[j].first,j));
  //    }
  //  }
  
  // Update links to represent flow
  for(int i=0;i<Nnode;i++){
    node[i]->selfLink = beta*node[i]->size*node[i]->selfLink;
    if(!node[i]->outLinks.empty()){
      int NoutLinks = node[i]->outLinks.size();
      for(int j=0;j < NoutLinks; j++){
        node[i]->outLinks[j].second = beta*node[i]->size*node[i]->outLinks[j].second;
      }

      //      // Update values for corresponding inlinks
      //      for(int j=0; j < NoutLinks; j++)
      //        node[node[i]->outLinks[j].first]->inLinks[linkIndex[node[i]->outLinks[j].first].find(i)->second].second = node[i]->outLinks[j].second;
      
      for(int j=0; j < NoutLinks; j++){
        int NinLinks = node[node[i]->outLinks[j].first]->inLinks.size();
        for(int k=0; k < NinLinks; k++){
          if(node[node[i]->outLinks[j].first]->inLinks[k].first == i){
            node[node[i]->outLinks[j].first]->inLinks[k].second = node[i]->outLinks[j].second;
            k = NinLinks;
          }
        }
      }
    }
  }
    
  // To be able to handle dangling nodes efficiently
  for(int i=0;i<Nnode;i++)
    if(node[i]->outLinks.empty() && (node[i]->selfLink <= 0.0))
      node[i]->danglingSize = node[i]->size;
    else
      node[i]->danglingSize = 0.0;  
  
  // Calculate p log p over all nodes
  nodeSize_log_nodeSize = 0.0;
  for(int i=0;i<Nnode;i++)
    nodeSize_log_nodeSize += plogp(node[i]->size);
  
  // The exit flow from each node at initiation
  for(int i=0;i<Nnode;i++)
    node[i]->exit = node[i]->size - (alpha*node[i]->size + beta*node[i]->danglingSize)*node[i]->teleportWeight - node[i]->selfLink;
  
  calibrate();
  
  //  cout << "Initial bit rate is " << codeLength/log(2.0) << ", starting merging " << Nnode << " nodes..." << endl;
  
}

void Greedy::tune(void){
  
  exit_log_exit = 0.0;
  size_log_size = 0.0;
  exitFlow = 0.0;
  
  for(int i=0;i<Nmod;i++){
    mod_exit[i] = 0.0;
    mod_size[i] = 0.0;
    mod_danglingSize[i] = 0.0;
    mod_teleportWeight[i] = 0.0;
    mod_members[i] = 0;
  }
	
  // Update all values except contribution from teleportation
  for(int i=0;i<Nnode;i++){
    int i_M = node[i]->index;
    int Nlinks = node[i]->outLinks.size();
    mod_size[i_M] += node[i]->size;
    mod_danglingSize[i_M] += node[i]->danglingSize;
    mod_teleportWeight[i_M] += node[i]->teleportWeight;
    mod_members[i_M]++;
    for(int j=0;j<Nlinks;j++){
      int nb = node[i]->outLinks[j].first;
      double nb_w = node[i]->outLinks[j].second;
      int nb_M = node[nb]->index;
      if(i_M != nb_M)
        mod_exit[i_M] += nb_w;
    }
  }
  
  // Update contribution from teleportation
  for(int i=0;i<Nmod;i++){
    mod_exit[i] += (alpha*mod_size[i] + beta*mod_danglingSize[i])*(1.0 - mod_teleportWeight[i]);
  }
  
  for(int i=0;i<Nmod;i++){
    exit_log_exit += plogp(mod_exit[i]);
    size_log_size += plogp(mod_exit[i] + mod_size[i]);
    exitFlow += mod_exit[i];
  }
	
  exit = plogp(exitFlow);
  
  codeLength = exit - 2.0*exit_log_exit + size_log_size - nodeSize_log_nodeSize; 
	
}

void Greedy::calibrate(void){
  
  vector<int>(Nmod).swap(mod_empty);
  Nempty = 0;
  
  vector<double>(Nmod).swap(mod_exit);
  vector<double>(Nmod).swap(mod_size);
  vector<double>(Nmod).swap(mod_danglingSize);
  vector<double>(Nmod).swap(mod_teleportWeight);
  vector<int>(Nmod).swap(mod_members);
  
  exit_log_exit = 0.0;
  size_log_size = 0.0;
  exitFlow = 0.0;
  
  for(int i=0;i<Nmod;i++){
    
    exit_log_exit += plogp(node[i]->exit);
    size_log_size += plogp(node[i]->exit + node[i]->size);
    exitFlow += node[i]->exit;
    
    mod_exit[i] = node[i]->exit;
    mod_size[i] = node[i]->size;
    mod_danglingSize[i] = node[i]->danglingSize;
    mod_teleportWeight[i] = node[i]->teleportWeight;
    mod_members[i] = node[i]->members.size();
    node[i]->index = i;
  }
  
  exit = plogp(exitFlow);
  
  codeLength = exit - 2.0*exit_log_exit + size_log_size - nodeSize_log_nodeSize; 
  
}

void Greedy::prepare(bool sort){
  
  Nmod = 0;
  vector<int>().swap(modSnode);
  
  if(sort){
    
    multimap<double,int> Msize;
    for(int i=0;i<Nnode;i++){
      if(mod_members[i] > 0){
        Nmod++;
        Msize.insert(make_pair(mod_size[i],i));
      }
    }
    
    for(multimap<double,int>::reverse_iterator it = Msize.rbegin(); it != Msize.rend(); it++)
      modSnode.push_back(it->second);
    
  }
  else{
    
    for(int i=0;i<Nnode;i++){
      if(mod_members[i] > 0){
        Nmod++;
        modSnode.push_back(i);
      }
    }
    
  }
  
}

void Greedy::level(Node ***node_tmp, bool sort){
  
  prepare(sort);
  
  //Node ***ntmp = node_tmp;
  
  (*node_tmp) = new Node*[Nmod];
  
  vector<int> nodeInMod = vector<int>(Nnode);
  
  for(int i=0;i<Nmod;i++){
    (*node_tmp)[i] = new Node();
    (*node_tmp)[i]->index = i;
    (*node_tmp)[i]->exit = mod_exit[modSnode[i]];
    (*node_tmp)[i]->size = mod_size[modSnode[i]];
    (*node_tmp)[i]->danglingSize = mod_danglingSize[modSnode[i]];
    (*node_tmp)[i]->teleportWeight = mod_teleportWeight[modSnode[i]];
    nodeInMod[modSnode[i]] = i;
  }
  
  // Calculate outflow of links to different modules
  vector<map<int,double> > outFlowNtoM(Nmod);
  map<int,double>::iterator it_M;
  
  for(int i=0;i<Nnode;i++){
    
    int i_M = nodeInMod[node[i]->index];
    
    copy(node[i]->members.begin(),node[i]->members.end(),back_inserter((*node_tmp)[i_M]->members));
    
    int NoutLinks = node[i]->outLinks.size(); 
    for(int j=0;j<NoutLinks;j++){
      int nb = node[i]->outLinks[j].first;
      int nb_M = nodeInMod[node[nb]->index];
      double nb_flow = node[i]->outLinks[j].second;
      if (nb != i) {
        it_M = outFlowNtoM[i_M].find(nb_M);
        if (it_M != outFlowNtoM[i_M].end())
          it_M->second += nb_flow;
        else
          outFlowNtoM[i_M].insert(make_pair(nb_M,nb_flow));
      }
    }
  }
  
  // Create outLinks at new level
  for(int i=0;i<Nmod;i++){
    for(it_M = outFlowNtoM[i].begin(); it_M != outFlowNtoM[i].end(); it_M++){
      if(it_M->first != i){
        (*node_tmp)[i]->outLinks.push_back(make_pair(it_M->first,it_M->second));
      }
    }
  }
  
  
  // Calculate inflow of links from different modules
  vector<map<int,double> > inFlowNtoM(Nmod);
  
  for(int i=0;i<Nnode;i++){
    
    int i_M = nodeInMod[node[i]->index];
    
    int NinLinks = node[i]->inLinks.size(); 
    for(int j=0;j<NinLinks;j++){
      int nb = node[i]->inLinks[j].first;
      int nb_M = nodeInMod[node[nb]->index];
      double nb_flow = node[i]->inLinks[j].second;
      if (nb != i) {
        it_M = inFlowNtoM[i_M].find(nb_M);
        if (it_M != inFlowNtoM[i_M].end())
          it_M->second += nb_flow;
        else
          inFlowNtoM[i_M].insert(make_pair(nb_M,nb_flow));
      } 
    }
  }
  
  // Create inLinks at new level
  for(int i=0;i<Nmod;i++){
    for(it_M = inFlowNtoM[i].begin(); it_M != inFlowNtoM[i].end(); it_M++){
      if(it_M->first != i){
        (*node_tmp)[i]->inLinks.push_back(make_pair(it_M->first,it_M->second));
      }
    } 
  }
  
  // Option to move to empty module
  vector<int>().swap(mod_empty);
  Nempty = 0;
  for(int i=0;i<Nnode;i++){
    delete node[i];
  }
  delete [] node;
  
  Nnode = Nmod;
  node = (*node_tmp);
  
  calibrate();
  
}


void Greedy::determMove(vector<int> &moveTo){
  
  for(int i=0;i<Nnode;i++){
    int oldM = i;
    int newM = moveTo[i];
    
    if(newM != oldM){
      
      double outFlowOldM = (alpha*node[i]->size + beta*node[i]->danglingSize)*(mod_teleportWeight[oldM]-node[i]->teleportWeight);
      double inFlowOldM = (alpha*(mod_size[oldM]-node[i]->size) + beta*(mod_danglingSize[oldM]-node[i]->danglingSize))*node[i]->teleportWeight;
      double outFlowNewM = (alpha*node[i]->size + beta*node[i]->danglingSize)*mod_teleportWeight[newM];
      double inFlowNewM = (alpha*mod_size[newM] + beta*mod_danglingSize[newM])*node[i]->teleportWeight;
      
      // For all outLinks
      int NoutLinks = node[i]->outLinks.size();
      for(int j=0; j<NoutLinks; j++){
        int nb_M = node[node[i]->outLinks[j].first]->index;
        double nb_flow = node[i]->outLinks[j].second;
        if(nb_M == oldM){
          outFlowOldM += nb_flow; 
        }
        else if(nb_M == newM){
          outFlowNewM += nb_flow;
        }
      }
      
      // For all inLinks
      int NinLinks = node[i]->inLinks.size();
      for(int j=0; j<NinLinks; j++){
        int nb_M = node[node[i]->inLinks[j].first]->index;
        double nb_flow = node[i]->inLinks[j].second;
        if(nb_M == oldM){
          inFlowOldM += nb_flow; 
        }
        else if(nb_M == newM){
          inFlowNewM += nb_flow;
        }
      }
      
      //Update empty module vector
      if(mod_members[newM] == 0){
        Nempty--;
      }
      if(mod_members[oldM] == static_cast<int>(node[i]->members.size())){
        mod_empty[Nempty] = oldM;
        Nempty++;
      }
      
      exitFlow -= mod_exit[oldM] + mod_exit[newM];
      exit_log_exit -= plogp(mod_exit[oldM]) + plogp(mod_exit[newM]);
      size_log_size -= plogp(mod_exit[oldM] + mod_size[oldM]) + plogp(mod_exit[newM] + mod_size[newM]); 
      
      mod_exit[oldM] -= node[i]->exit - outFlowOldM - inFlowOldM;
      mod_size[oldM] -= node[i]->size;
      mod_danglingSize[oldM] -= node[i]->danglingSize;
      mod_teleportWeight[oldM] -= node[i]->teleportWeight;
      mod_members[oldM] -= node[i]->members.size();
      mod_exit[newM] += node[i]->exit - outFlowNewM - inFlowNewM;
      mod_size[newM] += node[i]->size;
      mod_danglingSize[newM] += node[i]->danglingSize;
      mod_teleportWeight[newM] += node[i]->teleportWeight;
      mod_members[newM] += node[i]->members.size();
      
      exitFlow += mod_exit[oldM] + mod_exit[newM];
      exit_log_exit += plogp(mod_exit[oldM]) + plogp(mod_exit[newM]);
      size_log_size += plogp(mod_exit[oldM] + mod_size[oldM]) + plogp(mod_exit[newM] + mod_size[newM]);
      exit = plogp(exitFlow);
      
      codeLength = exit - 2.0*exit_log_exit + size_log_size - nodeSize_log_nodeSize;
      
      node[i]->index = newM;
      
    }
    
  }
  
};

void Greedy::eigenvector(void){
  
  // cout << "Calculating steady state distribution of flow..."; 
  
  vector<double> size_tmp = vector<double>(Nnode,1.0/Nnode);
  int Niterations = 0;
  double danglingSize;
  double sqdiff = 1.0;
  double sqdiff_old;
  double sum;
  do{
    
    // Calculate dangling size
    danglingSize = 0.0;
    for(int i=0;i<Ndanglings;i++){
      danglingSize += size_tmp[danglings[i]];
    }
    
    // Flow from teleportation
    for(int i=0;i<Nnode;i++)
      node[i]->size = (alpha + beta*danglingSize)*node[i]->teleportWeight;
    
    // Flow from network steps
    for(int i=0;i<Nnode;i++){
      node[i]->size += beta*node[i]->selfLink*size_tmp[i];
      int Nlinks = node[i]->outLinks.size();
      for(int j=0; j < Nlinks; j++)
        node[node[i]->outLinks[j].first]->size += beta*node[i]->outLinks[j].second*size_tmp[i];
    }
    
    // Normalize
    sum = 0.0;
    for(int i=0;i<Nnode;i++){
      sum += node[i]->size; 
    }
    sqdiff_old = sqdiff;
    sqdiff = 0.0;
    for(int i=0;i<Nnode;i++){
      node[i]->size /= sum;
      sqdiff += fabs(node[i]->size - size_tmp[i]);
      size_tmp[i] = node[i]->size;
    }
    Niterations++;
    
    if(sqdiff == sqdiff_old){
      //    fprintf(stderr,"\n1.0e-10 added to alpha for convergence (precision error)\n");
      alpha += 1.0e-10;
      beta = 1.0-alpha;
    }
    
    //fprintf(stderr,"\rCalculating steady state distribution of flow...the error is %e after %d iterations\n",sqdiff,Niterations);
    
  }  while((Niterations < 200) && (sqdiff > 1.0e-15 || Niterations < 50));
  
  danglingSize = 0.0;
  for(int i=0;i<Ndanglings;i++){
    danglingSize += size_tmp[danglings[i]];
  }
  
  //cout << "done! (the error is " << sqdiff << " after " << Niterations << " iterations)" << endl;
  
}
