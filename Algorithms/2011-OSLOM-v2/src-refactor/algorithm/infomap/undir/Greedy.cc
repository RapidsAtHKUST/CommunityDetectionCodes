#include "Greedy.h"

Greedy::~Greedy(){
  
  vector<int>().swap(modWnode);
	
}

Greedy::Greedy(MTRand *RR,int nnode,double deg,Node **ah){
	
  R = RR;
  Nnode = nnode;  
  node = ah;
  degree = deg;
  invDegree = 1.0/degree;
  log2 = log(2.0);
  Nmod = Nnode;
	
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
  vector<pair<int,double> > wNtoM(Nnode);
  for(int k=0;k<Nnode;k++){
    
    // Pick nodes in random order
    int flip = randomOrder[k]; 
		
    // Reset offset when int overflows
    if(offset > INT_MAX){ 
      for(int j=0;j<Nnode;j++)
				redirect[j] = 0;
      offset = 1;
    }    
		
    // Create vector with module links
    int Nlinks = node[flip]->links.size();
    int NmodLinks = 0;
    for(int j=0; j<Nlinks; j++){
      int nb_M = node[node[flip]->links[j].first]->index;
      double nb_w = node[flip]->links[j].second;
			
      if(redirect[nb_M] >= offset){
				wNtoM[redirect[nb_M] - offset].second += nb_w;
      }
      else{
				redirect[nb_M] = offset + NmodLinks;
				wNtoM[NmodLinks].first = nb_M;
				wNtoM[NmodLinks].second = nb_w;
				NmodLinks++;
      }
    }    
		
    // Calculate exit weight to own module
    int fromM = node[flip]->index; // 
    double wfromM = 0.0;
    if(redirect[fromM] >= offset)
      wfromM = wNtoM[redirect[fromM] - offset].second;
    
    // Option to move to empty module (if node not already alone)
    if(mod_members[fromM] > static_cast<int>(node[flip]->members.size())){
      if(Nempty > 0){
				wNtoM[NmodLinks].first = mod_empty[Nempty-1];
				wNtoM[NmodLinks].second = 0;
				NmodLinks++;
      }
    }
		
    // Randomize link order
    for(int j=0;j<NmodLinks-1;j++){
      int randPos = j + R->randInt(NmodLinks-j-1);
      int tmp_M = wNtoM[j].first;
      double tmp_w = wNtoM[j].second;
      wNtoM[j].first = wNtoM[randPos].first;
      wNtoM[j].second = wNtoM[randPos].second;
      wNtoM[randPos].first = tmp_M;
      wNtoM[randPos].second = tmp_w;
    }
		
    int bestM = fromM;
    double best_weight = 0.0;
    double best_delta = 0.0;
    
    // Find the move that minimizes the description length
    for (int j=0; j<NmodLinks; j++) {
      
      int toM = wNtoM[j].first;
      double wtoM = wNtoM[j].second;
      
      if(toM != fromM){
				
				double delta_exit = plogp(exitDegree - 2*wtoM + 2*wfromM) - exit;
				
				double delta_exit_log_exit = - plogp(mod_exit[fromM]) - plogp(mod_exit[toM]) \
				+ plogp(mod_exit[fromM] - node[flip]->exit + 2*wfromM) + plogp(mod_exit[toM] + node[flip]->exit - 2*wtoM);
				
				double delta_degree_log_degree = - plogp(mod_exit[fromM] + mod_degree[fromM]) - plogp(mod_exit[toM] + mod_degree[toM]) \
				+ plogp(mod_exit[fromM] + mod_degree[fromM] - node[flip]->exit - node[flip]->degree + 2*wfromM) \
				+ plogp(mod_exit[toM] + mod_degree[toM] + node[flip]->exit + node[flip]->degree - 2*wtoM);
				
				double deltaL = delta_exit - 2.0*delta_exit_log_exit + delta_degree_log_degree;
				
				if(deltaL < best_delta){
					bestM = toM;
					best_weight = wtoM;
					best_delta = deltaL;  
				}
				
      }
    }
    
    // Make best possible move
    if(bestM != fromM){
			
      //Update empty module vector
      if(mod_members[bestM] == 0){
				Nempty--;
      }
      if(mod_members[fromM] == static_cast<int>(node[flip]->members.size())){
				mod_empty[Nempty] = fromM;
				Nempty++;
      }
			
      exitDegree -= mod_exit[fromM] + mod_exit[bestM];
      exit_log_exit -= plogp(mod_exit[fromM]) + plogp(mod_exit[bestM]);
      degree_log_degree -= plogp(mod_exit[fromM] + mod_degree[fromM]) + plogp(mod_exit[bestM] + mod_degree[bestM]); 
			
      mod_exit[fromM] -= node[flip]->exit - 2*wfromM;
      mod_degree[fromM] -= node[flip]->degree;
      mod_members[fromM] -= node[flip]->members.size();
      mod_exit[bestM] += node[flip]->exit - 2*best_weight;
      mod_degree[bestM] += node[flip]->degree;
      mod_members[bestM] += node[flip]->members.size();
			
      exitDegree += mod_exit[fromM] + mod_exit[bestM];
      exit_log_exit += plogp(mod_exit[fromM]) + plogp(mod_exit[bestM]);
      degree_log_degree += plogp(mod_exit[fromM] + mod_degree[fromM]) + plogp(mod_exit[bestM] + mod_degree[bestM]); 
      
      exit = plogp(exitDegree);
      
      codeLength = exit - 2.0*exit_log_exit + degree_log_degree - nodeDegree_log_nodeDegree;
      
      node[flip]->index = bestM;
      moved = true;
			
    }
		
    offset += Nnode;
		
  }
  
  //cout << "Code length = " << codeLength << endl;
	
}

void Greedy::initiate(void){
  
  for(int i=0;i<Nnode;i++){
    double Mdeg = 0.0;
    for(link = node[i]->links.begin(); link != node[i]->links.end();link++)
      Mdeg += (*link).second;
    node[i]->exit = Mdeg;
    node[i]->degree = Mdeg; //Update when self-links exist
  }
	
  nodeDegree_log_nodeDegree = 0.0;
  for(int i=0;i<Nmod;i++)
    nodeDegree_log_nodeDegree += plogp(node[i]->degree);
  
  calibrate();
  
  //cout << "Initial bit rate is " << codeLength << ", starting merging " << Nnode << " nodes..." << endl;
  
}

void Greedy::tune(void){
  
  exit_log_exit = 0.0;
  degree_log_degree = 0.0;
  exitDegree = 0.0;
  
  for(int i=0;i<Nmod;i++){
    mod_exit[i] = 0.0;
    mod_degree[i] = 0.0;
    mod_members[i] = 0;
  }
	
  for(int i=0;i<Nnode;i++){
    int i_M = node[i]->index;
    double i_d = node[i]->degree;
    int Nlinks = node[i]->links.size(); 
    mod_members[i_M]++;
    mod_degree[i_M] += i_d;
    for(int j=0;j<Nlinks;j++){
      int nb = node[i]->links[j].first;
      double nb_w = node[i]->links[j].second;
      int nb_M = node[nb]->index;
      if(i_M != nb_M)
				mod_exit[i_M] += nb_w;
    }
  }
  
  for(int i=0;i<Nmod;i++){
    exit_log_exit += plogp(mod_exit[i]);
    degree_log_degree += plogp(mod_exit[i] + mod_degree[i]);
    exitDegree += mod_exit[i]; 
  }
	
  exit = plogp(exitDegree);
	
  codeLength = exit - 2.0*exit_log_exit + degree_log_degree - nodeDegree_log_nodeDegree; 
	
}

void Greedy::calibrate(void){
  
  vector<int>(Nmod).swap(mod_empty);
  Nempty = 0;
  
  vector<double>(Nmod).swap(mod_exit);
  vector<double>(Nmod).swap(mod_degree);
  vector<int>(Nmod).swap(mod_members);
	
  exit_log_exit = 0.0;
  degree_log_degree = 0.0;
  exitDegree = 0.0;
  
  for(int i=0;i<Nmod;i++){
    
    exit_log_exit += plogp(node[i]->exit);
    degree_log_degree += plogp(node[i]->exit + node[i]->degree);
    exitDegree += node[i]->exit;
		
    mod_exit[i] = node[i]->exit;
    mod_degree[i] = node[i]->degree;
    mod_members[i] = node[i]->members.size();
    node[i]->index = i;
  }
	
  exit = plogp(exitDegree);
	
  codeLength = exit - 2.0*exit_log_exit + degree_log_degree - nodeDegree_log_nodeDegree; 
	
}

void Greedy::prepare(bool sort){
	
  Nmod = 0;
  vector<int>().swap(modWnode);
	
  if(sort){
    
    multimap<double,int> Msize;
    for(int i=0;i<Nnode;i++){
      if(mod_members[i] > 0){
				Nmod++;
				Msize.insert(make_pair(mod_degree[i],i));
      }
    }
    
    for(multimap<double,int>::reverse_iterator it = Msize.rbegin(); it != Msize.rend(); it++)
      modWnode.push_back(it->second);
    
  }
  else{
    
    for(int i=0;i<Nnode;i++){
      if(mod_members[i] > 0){
				Nmod++;
				modWnode.push_back(i);
      }
    }
    
  }
	
}

void Greedy::level(Node ***node_tmp, bool sort){
  
  prepare(sort);
  
  (*node_tmp) = new Node*[Nmod];
  
  vector<int> nodeInMod(Nnode);
  for(int i=0;i<Nmod;i++){
    (*node_tmp)[i] = new Node();
    (*node_tmp)[i]->index = i;
    (*node_tmp)[i]->exit = mod_exit[modWnode[i]];
    (*node_tmp)[i]->degree = mod_degree[modWnode[i]];
    nodeInMod[modWnode[i]] = i;
  }
  
  // Calculate weight of links to different modules
  vector<map<int,double> > wNtoM(Nmod);
  //  map<int,double> wNtoM[Nmod];
  map<int,double>::iterator it_M;
  
  for(int i=0;i<Nnode;i++){
		
    int i_M = nodeInMod[node[i]->index];
    
    copy(node[i]->members.begin(),node[i]->members.end(),back_inserter((*node_tmp)[i_M]->members));
		
    int Nlinks = node[i]->links.size(); 
    for(int j=0;j<Nlinks;j++){
      int nb = node[i]->links[j].first;
      int nb_M = nodeInMod[node[nb]->index];
      double nb_w = node[i]->links[j].second;
      if (nb != i) {
				it_M = wNtoM[i_M].find(nb_M);
				if (it_M != wNtoM[i_M].end())
					it_M->second += nb_w;
				else
					wNtoM[i_M].insert(make_pair(nb_M,nb_w)); 
      }
    }
  }
  
  // Create network at new level
  for(int i=0;i<Nmod;i++){
    for(it_M = wNtoM[i].begin(); it_M != wNtoM[i].end(); it_M++){
      if(it_M->first != i){
				(*node_tmp)[i]->links.push_back(make_pair(it_M->first,it_M->second));
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

double Greedy::plogp(double d){
	
  if(d < 1.0e-10)
    return 0.0;
  else
    return invDegree*d*log(invDegree*d)/log2;
	
}

void Greedy::determMove(vector<int> &moveTo){
	
  for(int i=0;i<Nnode;i++){
    int fromM = i;
    int bestM = moveTo[i];
    double wfromM = 0.0;
    double best_weight = 0.0;
		
    if(fromM != bestM){
      int Nlinks = node[i]->links.size();
      
      for(int j=0;j<Nlinks;j++){
				if(node[node[i]->links[j].first]->index == bestM)
					best_weight += node[i]->links[j].second;
				else if(node[node[i]->links[j].first]->index == fromM){
					wfromM += node[i]->links[j].second;
				}
      }
      
      //Update empty module vector
      if(mod_members[bestM] == 0){
				Nempty--;
      }
      if(mod_members[fromM] == static_cast<int>(node[i]->members.size())){
				mod_empty[Nempty] = fromM;
				Nempty++;
      }
			
      exitDegree -= mod_exit[fromM] + mod_exit[bestM];
      exit_log_exit -= plogp(mod_exit[fromM]) + plogp(mod_exit[bestM]);
      degree_log_degree -= plogp(mod_exit[fromM] + mod_degree[fromM]) + plogp(mod_exit[bestM] + mod_degree[bestM]); 
      
      mod_exit[fromM] -= node[i]->exit - 2*wfromM;
      mod_degree[fromM] -= node[i]->degree;
      mod_members[fromM] -= node[i]->members.size();
      mod_exit[bestM] += node[i]->exit - 2*best_weight;
      mod_degree[bestM] += node[i]->degree;
      mod_members[bestM] += node[i]->members.size();
			
      exitDegree += mod_exit[fromM] + mod_exit[bestM];
      exit_log_exit += plogp(mod_exit[fromM]) + plogp(mod_exit[bestM]);
      degree_log_degree += plogp(mod_exit[fromM] + mod_degree[fromM]) + plogp(mod_exit[bestM] + mod_degree[bestM]); 
      
      exit = plogp(exitDegree);
      
      codeLength = exit - 2.0*exit_log_exit + degree_log_degree - nodeDegree_log_nodeDegree;
      
      node[i]->index = bestM;
			
    }
  }
	
};
