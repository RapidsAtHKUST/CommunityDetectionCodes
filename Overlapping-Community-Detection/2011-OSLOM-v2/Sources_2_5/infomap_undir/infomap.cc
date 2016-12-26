#include "infomap.h"

using namespace std;
using std::cout;
using std::cin;
using std::endl;

unsigned stou(char *s){
  return strtoul(s,(char **)NULL,10);
}

void partition(MTRand *R,Node ***node, GreedyBase *greedy, bool silent);
void repeated_partition(MTRand *R, Node ***node, GreedyBase *greedy, bool silent,int Ntrials);
void printTree(string s,multimap<double,treeNode,greater<double> >::iterator it_tM,ofstream *outfile,bool flip);

// Call: trade <seed> <Ntries>
int main(int argc,char *argv[]){
  
  if( argc !=4 ){
    cout << "Call: ./infomap <seed> <network.net> <# attempts>" << endl;
    exit(-1);
  }
  
  int Ntrials = atoi(argv[3]);  // Set number of partition attempts
  string infile = string(argv[2]);
  string networkName(infile.begin(),infile.begin() + infile.find(".net"));
  string line;
  string buf;
  
  MTRand *R = new MTRand(stou(argv[1]));
  
  /* Read network in Pajek format with nodes ordered 1, 2, 3, ..., N,            */
  /* each undirected link occurring only once, and link weights > 0.             */
  /* (if a link is defined more than once, weights are aggregated)               */   
  /* For more information, see http://vlado.fmf.uni-lj.si/pub/networks/pajek/.   */
  /* Example network with three nodes and                                        */
  /* three undirected weighted links:                                            */
  /* *Vertices 3                                                                 */
  /* 1 "Name of first node"                                                      */
  /* 2 "Name of second node"                                                     */
  /* 3 "Name of third node"                                                      */
  /* *Edges 3                                                                    */
  /* 1 2 1.0                                                                     */
  /* 1 3 3.3                                                                     */
  /* 2 3 2.2                                                                     */
  
  cout << "Reading network " << argv[2] << "..." << flush;
  ifstream net(argv[2]);
  int Nnode = 0;
  istringstream ss;
  while(Nnode == 0){ 
    if(getline(net,line) == NULL){
      cout << "the network file is not in Pajek format...exiting" << endl;
      exit(-1);
    }
    else{
      ss.clear();
      ss.str(line);
      ss >> buf;
      if(buf == "*Vertices" || buf == "*vertices" || buf == "*VERTICES"){
        ss >> buf;
        Nnode = atoi(buf.c_str());
      }
      else{
        cout << "the network file is not in Pajek format...exiting" << endl;
        exit(-1);
      }
    }
  }
  
  string *nodeNames = new string[Nnode];
  
  // Read node names, assuming order 1, 2, 3, ...
  for(int i=0;i<Nnode;i++){
    getline(net,line);
    int nameStart = line.find_first_of("\"");
    int nameEnd = line.find_last_of("\"");
    if(nameStart < nameEnd){
      nodeNames[i] =  string(line.begin() + nameStart + 1,line.begin() + nameEnd);
    }
    else{
      ss.clear();
      ss.str(line);
      ss >> buf; 
      ss >> nodeNames[i];
    }
  }
  // Read the number of links in the network
  getline(net,line);
  ss.clear();
  ss.str(line);
  ss >> buf;
  if(buf != "*Edges" && buf != "*edges" && buf != "*Arcs" && buf != "*arcs"){
    cout << endl << "Number of nodes not matching, exiting" << endl;
    exit(-1);
  }
  
  int Nlinks = 0;
  int NdoubleLinks = 0;
  map<int,map<int,double> > Links; 
  // Read links in format "from to weight", for example "1 3 2" (all integers) and each undirected link only ones (weight is optional).
  while(getline(net,line) != NULL){
    ss.clear();
    ss.str(line);
    ss >> buf;
    int linkEnd1 = atoi(buf.c_str());
    ss >> buf;
    int linkEnd2 = atoi(buf.c_str());
    buf.clear();
    ss >> buf;
    double linkWeight;
    if(buf.empty()) // If no information 
      linkWeight = 1.0;
    else
      linkWeight = atof(buf.c_str());
    
    linkEnd1--; // Nodes start at 1, but C++ arrays at 0.
    linkEnd2--;
    
    if(linkEnd2 < linkEnd1){
      int tmp = linkEnd1;
      linkEnd1 = linkEnd2;
      linkEnd2 = tmp;
    }

    // Aggregate link weights if they are definied more than once
    map<int,map<int,double> >::iterator fromLink_it = Links.find(linkEnd1);
    if(fromLink_it == Links.end()){ // new link
      map<int,double> toLink;
      toLink.insert(make_pair(linkEnd2,linkWeight));
      Links.insert(make_pair(linkEnd1,toLink));
      Nlinks++;
    }
    else{
      map<int,double>::iterator toLink_it = fromLink_it->second.find(linkEnd2);
      if(toLink_it == fromLink_it->second.end()){ // new link
        fromLink_it->second.insert(make_pair(linkEnd2,linkWeight));
        Nlinks++;
      }
      else{
        toLink_it->second += linkWeight;
        NdoubleLinks++;
      }
    }
  }
  
  net.close();
  
  cout << "done! (found " << Nnode << " nodes and " << Nlinks << " links";
  if(NdoubleLinks > 0)
    cout << ", aggregated " << NdoubleLinks << " link(s) defined more than once";
  
  /////////// Partition network /////////////////////
  double totalDegree = 0.0;
  vector<double> degree(Nnode);
  Node **node = new Node*[Nnode];
  for(int i=0;i<Nnode;i++){
    node[i] = new Node(i);
    degree[i] = 0.0;
  }
  
  int NselfLinks = 0;
  for(map<int,map<int,double> >::iterator fromLink_it = Links.begin(); fromLink_it != Links.end(); fromLink_it++){
    for(map<int,double>::iterator toLink_it = fromLink_it->second.begin(); toLink_it != fromLink_it->second.end(); toLink_it++){
      
      int from = fromLink_it->first;
      int to = toLink_it->first;
      double weight = toLink_it->second;
      if(weight > 0.0){
        if(from == to){
          NselfLinks++;
        }
        else{
          node[from]->links.push_back(make_pair(to,weight));
          node[to]->links.push_back(make_pair(from,weight));
          totalDegree += 2*weight;
          degree[from] += weight;
          degree[to] += weight;
        }
      }
    }
  }
  if(NselfLinks > 0)
    cout << ", ignoring " <<  NselfLinks << " self link(s)." << endl;
  else
    cout << ")" << endl;
  
  //Swap maps to free memory
  for(map<int,map<int,double> >::iterator it = Links.begin(); it != Links.end(); it++)
    map<int,double>().swap(it->second);
  map<int,map<int,double> >().swap(Links);
  
  // Initiation
  GreedyBase* greedy;
  greedy = new Greedy(R,Nnode,totalDegree,node);
  greedy->initiate();
  
  double uncompressedCodeLength = -greedy->nodeDegree_log_nodeDegree;

  cout << "Now partition the network:" << endl;
  repeated_partition(R,&node,greedy,false,Ntrials);
  int Nmod = greedy->Nnode;
  cout << "Done! Code length " << greedy->codeLength << " in " << Nmod << " modules." << endl;
  cout << "Compressed by " << 100.0*(1.0-greedy->codeLength/uncompressedCodeLength) << " percent." << endl;
  
  // Order modules by size
  multimap<double,treeNode,greater<double> > treeMap;
  multimap<double,treeNode,greater<double> >::iterator it_tM;
  for(int i=0;i<greedy->Nnode;i++){
    
    int Nmembers = node[i]->members.size();
    treeNode tmp_tN;
    it_tM = treeMap.insert(make_pair(node[i]->degree/totalDegree,tmp_tN));
    for(int j=0;j<Nmembers;j++)
      it_tM->second.members.insert(make_pair(degree[node[i]->members[j]]/totalDegree,make_pair(node[i]->members[j],nodeNames[node[i]->members[j]]))); 
    
  }
  
  // Order links by size
  multimap<double,pair<int,int>,greater<double> > sortedLinks;
  for(int i=0;i<Nmod;i++){
    int Nlinks = node[i]->links.size();
    for(int j=0;j<Nlinks;j++){
      if(i <= node[i]->links[j].first)
        sortedLinks.insert(make_pair(node[i]->links[j].second/totalDegree,make_pair(i+1,node[i]->links[j].first+1)));
    }
  }
  
  //Print partition in format "module:rank size name"
  ostringstream oss;
  oss << networkName << ".tree";
  ofstream outfile;
  outfile.open(oss.str().c_str());
  outfile << "# Code length " << greedy->codeLength << " in " << Nmod << " modules." << endl;
  int k = 1;
  for(multimap<double,treeNode,greater<double> >::iterator it = treeMap.begin(); it != treeMap.end(); it++){
    string s;
    s.append(to_string(k));
    s.append(":");
    printTree(s,it,&outfile,false);
    k++;
  }
  outfile.close();
  
  // Print partitions in Pajek's .clu format
  vector<int> clusterVec(Nnode);
  int clusterNr = 0;  
  for(multimap<double,treeNode,greater<double> >::iterator mod = treeMap.begin(); mod != treeMap.end(); mod++){
	for(multimap<double,pair<int,string>,greater<double> >::iterator mem = mod->second.members.begin(); mem != mod->second.members.end(); mem++){
	  clusterVec[mem->second.first] = clusterNr;
	}
	clusterNr++;
  }
  oss.str("");
  oss << networkName << ".clu";
  outfile.open(oss.str().c_str());
  outfile << "*Vertices " << Nnode << "\x0D\x0A";
  for(int i=0;i<Nnode;i++)
    outfile << clusterVec[i]+1 << "\x0D\x0A";
  outfile.close();
  
  // Print map in Pajek's .net format (links sorted in descending order)
  oss.str("");
  oss << networkName << "_map.net";
  outfile.open(oss.str().c_str());
  outfile << "*Vertices " << Nmod << "\x0D\x0A";
  for(int i=0;i<Nmod;i++)
    outfile << i+1 << " \"" << i+1 << "\"" << "\x0D\x0A";
  outfile << "*Edges " << sortedLinks.size() << "\x0D\x0A";
  for(multimap<double,pair<int,int>,greater<double> >::iterator it = sortedLinks.begin();it != sortedLinks.end();it++)   
    outfile << "  " << it->second.first << " " << it->second.second << " " << 1.0*it->first/totalDegree << "\x0D\x0A";
  outfile.close();
  
  // Print size of modules in Pajek's .vec format
  oss.str("");
  oss << networkName << "_map.vec";
  outfile.open(oss.str().c_str());
  outfile << "*Vertices " << Nmod << "\x0D\x0A";
  for(int i=0;i<Nmod;i++)
    outfile << 1.0*node[i]->degree/totalDegree << "\x0D\x0A";
  outfile.close();
  
  // Print map in .map format for the Map Generator at www.mapequation.org
  oss.str("");
  oss << networkName << ".map";
  outfile.open(oss.str().c_str());
  outfile << "# modules: " << Nmod << endl;
  outfile << "# modulelinks: " << sortedLinks.size() << endl;
  outfile << "# nodes: " << Nnode << endl;
  outfile << "# links: " << Nlinks << endl;
  outfile << "# codelength: " << greedy->codeLength << endl;
  outfile << "*Undirected" << endl;
  outfile << "*Modules " << Nmod << endl;
  k = 0;
  for(multimap<double,treeNode,greater<double> >::iterator it = treeMap.begin(); it != treeMap.end(); it++){
    outfile << k+1 << " \"" << it->second.members.begin()->second.second << "\" " << it->first << " " << node[k]->exit/totalDegree << endl;
    k++;
  }
  outfile << "*Nodes " << Nnode << endl;
  k = 1;
  for(multimap<double,treeNode,greater<double> >::iterator it = treeMap.begin(); it != treeMap.end(); it++){
    string s;
    s.append(to_string(k));
    s.append(":");
    printTree(s,it,&outfile,true);
    k++;
  }
  outfile << "*Links " << sortedLinks.size() << endl;
  for(multimap<double,pair<int,int>,greater<double> >::iterator it = sortedLinks.begin();it != sortedLinks.end();it++)   
    outfile << it->second.first << " " << it->second.second << " " << 1.0*it->first << endl;
  outfile.close();
  
  delete [] nodeNames;
  for(int i=0;i<greedy->Nnode;i++){
    delete node[i];
  }
  delete [] node;
  delete greedy;
  delete R;
  
}

void partition(MTRand *R,Node ***node, GreedyBase *greedy, bool silent){
  
  int Nnode = greedy->Nnode;
  Node **cpy_node = new Node*[Nnode];
  for(int i=0;i<Nnode;i++){
    cpy_node[i] = new Node();
    cpyNode(cpy_node[i],(*node)[i]);
  }
  
  int iteration = 0;
  double outer_oldCodeLength;
  do{
    outer_oldCodeLength = greedy->codeLength;
    
    if((iteration > 0) && (iteration % 2 == 0) && (greedy->Nnode > 1)){  // Partition the partition
      
      
      if(!silent)
        cout << "Iteration " << iteration+1 << ", moving " << flush;
      
      Node **rpt_node = new Node*[Nnode];
      for(int i=0;i<Nnode;i++){
        rpt_node[i] = new Node();
        cpyNode(rpt_node[i],cpy_node[i]);
      }
      vector<int> subMoveTo(Nnode);
      vector<int> moveTo(Nnode);
      int subModIndex = 0;
      
      for(int i=0;i<greedy->Nnode;i++){
        
        int sub_Nnode = (*node)[i]->members.size();
        
        if(sub_Nnode > 1){
          
          Node **sub_node = new Node*[sub_Nnode]; 
          set<int> sub_mem;
          for(int j=0;j<sub_Nnode;j++)
            sub_mem.insert((*node)[i]->members[j]);
          set<int>::iterator it_mem = sub_mem.begin();
          int *sub_renumber = new int[Nnode];
          int *sub_rev_renumber = new int[sub_Nnode];
          double totalDegree = 0.0;
          for(int j=0;j<sub_Nnode;j++){
            
            //    fprintf(stderr,"%d %d\n",j,(*it_mem));
            int orig_nr = (*it_mem);
            int orig_Nlinks = cpy_node[orig_nr]->links.size(); // ERROR HERE
            sub_renumber[orig_nr] = j;
            sub_rev_renumber[j] = orig_nr;
            sub_node[j] = new Node(j);
            for(int k=0;k<orig_Nlinks;k++){
              int orig_link = cpy_node[orig_nr]->links[k].first;
              int orig_link_newnr = sub_renumber[orig_link];
              double orig_weight = cpy_node[orig_nr]->links[k].second;
              if(orig_link < orig_nr){
                if(sub_mem.find(orig_link) != sub_mem.end()){
                  sub_node[j]->links.push_back(make_pair(orig_link_newnr,orig_weight));
                  sub_node[orig_link_newnr]->links.push_back(make_pair(j,orig_weight));
                  totalDegree += 2.0*orig_weight;
                }
              }
            }
            it_mem++;
          }
          
          GreedyBase* sub_greedy;
          sub_greedy = new Greedy(R,sub_Nnode,totalDegree,sub_node);
          sub_greedy->initiate();
          partition(R,&sub_node,sub_greedy,true);
          for(int j=0;j<sub_greedy->Nnode;j++){
            int Nmembers = sub_node[j]->members.size();
            for(int k=0;k<Nmembers;k++){
              subMoveTo[sub_rev_renumber[sub_node[j]->members[k]]] = subModIndex;
            }
            moveTo[subModIndex] = i;
            subModIndex++;
            delete sub_node[j];
          }
          
          delete [] sub_node;
          delete sub_greedy;
          delete [] sub_renumber;
          delete [] sub_rev_renumber;
          
        }
        else{
          
          subMoveTo[(*node)[i]->members[0]] = subModIndex;
          moveTo[subModIndex] = i;
          
          subModIndex++;
          
        }
      }
      
      for(int i=0;i<greedy->Nnode;i++)
        delete (*node)[i];
      delete [] (*node);
      
      greedy->Nnode = Nnode;
      greedy->Nmod = Nnode;
      greedy->node = rpt_node;
      greedy->initiate();
      greedy->determMove(subMoveTo);
      greedy->level(node,false); 
      greedy->determMove(moveTo);
      (*node) = rpt_node;
      
      outer_oldCodeLength = greedy->codeLength;
      
      if(!silent)
        cout << greedy->Nnode << " modules, looping " << flush;
      
    }
    else if(iteration > 0){
      
      if(!silent)
        cout << "Iteration " << iteration+1 << ", moving " << Nnode << " nodes, looping " << flush;
      
      
      Node **rpt_node = new Node*[Nnode];
      for(int i=0;i<Nnode;i++){
        rpt_node[i] = new Node();
        cpyNode(rpt_node[i],cpy_node[i]);
      }
      
      vector<int> moveTo(Nnode);
      for(int i=0;i<greedy->Nnode;i++){
        int Nmembers = (*node)[i]->members.size();
        for(int j=0;j<Nmembers;j++){
          moveTo[(*node)[i]->members[j]] = i;
        }
      }
      
      for(int i=0;i<greedy->Nnode;i++)
        delete (*node)[i];
      delete [] (*node);
      
      greedy->Nnode = Nnode;
      greedy->Nmod = Nnode;
      greedy->node = rpt_node;
      greedy->initiate();
      greedy->determMove(moveTo);
      
      (*node) = rpt_node;
    }
    else{
      
      if(!silent)
        cout << "Iteration " << iteration+1 << ", moving " << Nnode << " nodes, looping " << flush;
      
    }
    
    double oldCodeLength;
    do{
      oldCodeLength = greedy->codeLength;
      bool moved = true;
      int Nloops = 0;
      int count = 0;
      while(moved){
        moved = false;
        double inner_oldCodeLength = greedy->codeLength;
        greedy->move(moved);
        Nloops++;
        count++;
        if(inner_oldCodeLength-greedy->codeLength < 1.0e-10)
          moved = false;
        
        if(count == 10){	  
          greedy->tune();
          count = 0;
        }
        // 	if(!silent){
        // 	  cerr << Nloops;
        // 	  int loopsize = to_string(Nloops).length();
        // 	  for(int i=0;i<loopsize;i++)
        // 	    cerr << "\b";
        // 	}
      }
      
      greedy->level(node,true);
      
      if(!silent)
        cout << Nloops << " " << flush;
      
    } while(oldCodeLength - greedy->codeLength >  1.0e-10);
    
    iteration++;
    if(!silent)
      cout << "times between mergings to code length " <<  greedy->codeLength << " in " << greedy->Nmod << " modules." << endl;
    
  } while(outer_oldCodeLength - greedy->codeLength > 1.0e-10);
  
  for(int i=0;i<Nnode;i++)
    delete cpy_node[i];
  delete [] cpy_node;
  
}

void repeated_partition(MTRand *R, Node ***node, GreedyBase *greedy, bool silent,int Ntrials){
  
  double shortestCodeLength = 1000.0;
  int Nnode = greedy->Nnode;
  vector<int> cluster(Nnode);
  
  for(int trial = 0; trial<Ntrials;trial++){
    
    if(!silent)
      cout << "Attempt " << trial+1 << "/" << Ntrials << endl;
    
    Node **cpy_node = new Node*[Nnode];
    for(int i=0;i<Nnode;i++){
      cpy_node[i] = new Node();
      cpyNode(cpy_node[i],(*node)[i]);
    }
    
    greedy->Nnode = Nnode;
    greedy->Nmod = Nnode;
    greedy->node = cpy_node;
    greedy->initiate();
    
    partition(R,&cpy_node,greedy,silent);
    
    if(greedy->codeLength < shortestCodeLength){
      
      shortestCodeLength = greedy->codeLength;
      
      // Store best partition
      for(int i=0;i<greedy->Nnode;i++){
        for(vector<int>::iterator mem = cpy_node[i]->members.begin(); mem != cpy_node[i]->members.end(); mem++){
          cluster[(*mem)] = i;
        }
      }
    }
    
    for(int i=0;i<greedy->Nnode;i++){
      delete cpy_node[i];
    }
    delete [] cpy_node;
    
  }
  
  // Commit best partition
  greedy->Nnode = Nnode;
  greedy->Nmod = Nnode;
  greedy->node = (*node);
  greedy->initiate();
  greedy->determMove(cluster);
  greedy->level(node,true);
  
}

void printTree(string s,multimap<double,treeNode,greater<double> >::iterator it_tM,ofstream *outfile,bool flip){
  
  multimap<double,treeNode,greater<double> >::iterator it;
  if(it_tM->second.nextLevel.size() > 0){
    int i=1;
    for(it = it_tM->second.nextLevel.begin(); it != it_tM->second.nextLevel.end(); it++){
      string cpy_s(s + to_string(i) + ":");
      printTree(cpy_s,it,outfile,flip);
      i++;
    }
  }
  else{
    int i = 1;
    for(multimap<double,pair<int,string>,greater<double> >::iterator mem = it_tM->second.members.begin(); mem != it_tM->second.members.end(); mem++){
      if(flip){
        string cpy_s(s + to_string(i) + " \"" + mem->second.second + "\" " + to_string(mem->first));
        (*outfile) << cpy_s << endl;
      }
      else{
        string cpy_s(s + to_string(i) + " " + to_string(mem->first) + " \"" + mem->second.second + "\"");
        (*outfile) << cpy_s << endl;
      }
      i++;
    } 
  }  
}
