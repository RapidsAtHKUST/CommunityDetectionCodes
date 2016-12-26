#include "Communities.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/format.hpp>
using namespace std;

Community::~Community() 
{
  if (set == NULL) delete set;
}

Communities::Communities(igraph_t* graph)
{
  // salviamo il puntatore al grafo
  this->graph = graph;
  
  // Vogliamo copiare i gradi in un array
  unsigned int vc = igraph_vcount(graph);
  
  igraph_vector_t degree;
  igraph_vector_init(&degree, vc);
  
  // per quanto riguarda igraph_vss_1, la guida di igraph
  // dice che è immediato, e può essere usato senza preoccuparsi di distruggerlo
  igraph_degree(graph, &degree, igraph_vss_all(), IGRAPH_ALL, false);
  
  // creiamo l'array e lo popoliamo
  degrees = new unsigned int[vc];
  for (unsigned int i = 0; i < vc; i++) degrees[i] = VECTOR(degree)[i];
  igraph_vector_destroy(&degree);
  
  // questa è una costante ricorrente
  factor = 0.5/((double)igraph_ecount(graph));
}


Communities::~Communities()
{
  delete[] degrees;  
  delete[] membership;
  
  // non possiamo fare il delete di altre cose, perché parte sono condivise.
  // si usa sync quando si vuole fare pulizia tra una copia e l'altra,
  // si usa free quando si vuole fare pulizia prima di cancellare TUTTE le 
  // istanze
}

Communities::Communities(const Communities& c)
{
  this->graph = c.graph;  
  this->communities = c.communities;
  this->counter = c.counter;
  this->keys = c.keys;
  this->factor = c.factor;
  
  #ifdef EQ_HARD
  this->mmap = c.mmap;
  #endif
  
  unsigned int vs = igraph_vcount(graph);
  this->degrees = new unsigned int[vs];  
  this->membership = new unsigned int[vs];
  for (unsigned int i = 0; i < vs; i++) {
     degrees[i] = c.degrees[i];
     membership[i] = c.membership[i];
  }
}

void Communities::free()
{
  BOOST_FOREACH(CID c, keys)
  {
    delete communities[c];
  }
}

void Communities::sync(Communities& b)
{
  set<CID> toDelete;
  
  std::set_difference(keys.begin(), keys.end(), b.keys.begin(), b.keys.end(), 
                      std::inserter(toDelete, toDelete.begin()));
  
  BOOST_FOREACH(CID c, toDelete)
  {
    delete communities[c];
  }
}


bool Communities::has(CID i)
{
  return keys.find(i) != keys.end();
}

////////////////////////////////////////////////////////////////////////////////
// utilità
////////////////////////////////////////////////////////////////////////////////
// FIXME: inserire il controllo degli errori
unsigned int Communities::getCommunityInternalEdges(unsigned int s)
{
	int err;
	
	igraph_vector_t *v = communities[s]->toVector();
	igraph_vs_t *vs = communities[s]->toVertexSelector(v);
	
	//creazione sottografo dal vertex_selector;
	igraph_t subgraph;  
	err = igraph_subgraph(graph, &subgraph, *vs);
	
	// quello che ci interessa realmente
	unsigned int e = igraph_ecount(&subgraph);
	
	// queste funzioni non restituiscono niente
	igraph_vs_destroy(vs);
	igraph_vector_destroy(v);
	delete vs;
	delete v;
	igraph_destroy(&subgraph);
	
	return e;
}

// FIXME: inserire il controllo degli errori
unsigned int Communities::getCommunityBoundaryEdges(unsigned int s)
{
  int err;
  unsigned int o = 0;
  
  igraph_vector_t* v = communities[s]->toVector();
  igraph_vs_t*    vs = communities[s]->toVertexSelector(v);
  
  igraph_vit_t vit;
  igraph_vit_create(graph, *vs, &vit);
  
  unsigned int cSize = communities[s]->set->size();
  
  igraph_vector_ptr_t p;  
  err = igraph_vector_ptr_init(&p, cSize);
  err = igraph_neighborhood(graph, &p, *vs, 1, IGRAPH_ALL);
  // la componente i-esima di p contiene l'elenco di vicini di ordine 1 
  // dell'i-esimo nodo della community (quindi anche sé stesso)
  
  //conteggio edge che escono dalla community
  for (unsigned int i = 0; i < cSize; i++)  //per ogni vertice della community
  {
    igraph_vector_t* vv = (igraph_vector_t*)igraph_vector_ptr_e(&p, i);
    
    // per ogni vertice vicino di i
    for (int l = 0; l < igraph_vector_size(vv); l++) 
    {
      //se non appartiene alla community, incrementa o
      if (!ownership(VECTOR(*vv)[l], s))
        o++;
    }
  }
  
  igraph_vit_destroy(&vit);
  igraph_vs_destroy(vs);
  igraph_vector_destroy(v);
  delete vs;
  delete v;
  
  igraph_vector_ptr_destroy(&p);
  
  return o;
}

void Communities::load(std::istream& s)
{
  // TODO: pulire la struttura dati prima di fare il load
  unsigned int cSize;
  s >> cSize;
  
  for (unsigned int i = 0; i < cSize; i++) 
  {
    unsigned int iSize;
    s >> iSize;
    
    NodeSet* nsp = new NodeSet;
    Node aux;
    
    for (unsigned int j = 0; j < iSize; j++)
    {
      s >> aux;
      
      // calcola membership automaticamente
      nsp->insert(aux);
    }
    
    addCommunity(nsp);
  }
}

void Communities::dump(std::ostream& s)
{
  s << size()<< endl;
  
  BOOST_FOREACH(unsigned int i, keys)
  {  
    // K n1 n2 .. nK\n
    unsigned int size = getCommunitySize(i);
    
    // commentato perché voglio che usi gli id anziché label..
    /*
    igraph_vector_t* v = communities[i]->toVector();
    igraph_vs_t*    vs = communities[i]->toVertexSelector(v);
    igraph_strvector_t result;
    igraph_strvector_init(&result, size);
    
    igraph_cattribute_VASV(graph, "label", *vs, &result);
    */
    s << size;
    BOOST_FOREACH(Node n, * communities[i]->set) s << " " << n;
    s << endl;
    
    /*
    igraph_strvector_destroy(&result);
    igraph_vs_destroy(vs);
    igraph_vector_destroy(v);
    delete vs;
    delete v;*/
  }
}

////////////////////////////////////////////////////////////////////////////////    
// Metodi di supporto a GCE e EAGLE
////////////////////////////////////////////////////////////////////////////////
// ho rimosso il parametro graph, tanto gli viene passato al costruttore
int Communities::setCommunities(string fileName)
{
  counter = 0;
  ifstream f1(fileName.c_str());
  
  if (!f1) {
    return -1;
  }

  communities.clear();

  string s;
  string sub;
  
  while (f1.good())
  {          
    //legge tutta la riga dal file e la mette nella variabile s
    getline(f1, s);
    
    if(s.length() == 0)
      continue;

    NodeSet* ns = new NodeSet;
    //unsigned int vs = igraph_vcount(graph);
    
    //Estrapolazione interi dalla stringa s
    istringstream iss(s);
    
    while (iss.good())
    {
      unsigned int node;
      
      iss >> node;
      //node--;
      //cout << node << " < " << vs << " == " << (node < vs) << endl;
      //assert(node < vs);
      ns->insert(node);
    }
    //cout << "Inserisco tutto e speriamo" << endl;
    addCommunity(ns);
  }
  
  // ricordarsi di chiamare
  return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Metodi di supporto ad EAGLE
////////////////////////////////////////////////////////////////////////////////
CID Communities::registerCommunity(CommunityPtr c)
{
  CID r = counter++;
  communities[r] = c;
  keys.insert(r);
  
  return r;
}

CID Communities::addCommunity(NodeSet* s)
{
  CommunityPtr c = Community::spawn();
  c->set = s;
  
  CID r = registerCommunity(c); 
  
  BOOST_FOREACH(Node n, * c->set) 
  {
    membership[n]++;
#ifdef EQ_HARD
    mmap[n].insert(r);
#endif
  }
  
  c->communitySize = getCommunitySize(r);
  c->communityInternalEdges = getCommunityInternalEdges(r);
  c->communityBoundaryEdges = getCommunityBoundaryEdges(r);
  
  return r;
}

CID Communities::addCommunity(igraph_vector_t* vector)
{
  // Creiamo una nuova community
  CommunityPtr c = Community::spawn();
  c->set = new NodeSet;
  CID r = registerCommunity(c);
  
  // popoliamola
  int s = igraph_vector_size(vector);
  for (int j = 0; j < s; j++) 
  {
    int node = VECTOR(*vector)[j];
    membership[node]++;
    c->set->insert(node);
#ifdef EQ_HARD
    mmap[node].insert(r);
#endif
  }
  
  return r;
}

CID Communities::addCommunity(unsigned int i)
{
  // Creiamo una nuova community
  CommunityPtr c = Community::spawn();
  c->set = new NodeSet;
  
  // inseriamo la community nella lista 
  CID r = registerCommunity(c);
  
  // Popoliamola
  membership[i]++;
  c->set->insert(i);
#ifdef EQ_HARD
  mmap[i].insert(r);
#endif
  
  return r;
}

void Communities::prepareForEAGLE()
{
  unsigned int s = igraph_vcount(graph);
  
  communities.clear();
  keys.clear();
  
#ifdef EQ_HARD
  mmap.clear();
#endif
  
  membership = new unsigned int[s];
  for (unsigned int i = 0; i < s; i++) membership[i] = 0;
  
  counter = 0;
}

void Communities::precalculateData()
{
  BOOST_FOREACH(CommunitiesMap::value_type cp, communities)
    cp.second->update(graph, degrees, membership);
}

unsigned int Communities::merge(unsigned int a, unsigned int b, bool hasComms)
{ 
  // m è la community unione
  CommunityPtr m = get(a)->unite(get(b));
  CID r = registerCommunity(m);
  
  // abbiamo bisogno della community intersezione per poter aggiustare il 
  // vettore membership
  CommunityPtr is = get(a)->intersect(get(b));
  
  // questo fa cambiare membership, ed è necessario fare l'update di tutte le 
  // communities che hanno questi nodi
  BOOST_FOREACH(Node n, * is->set)
    membership[n]--;
  delete is;
  
#ifdef EQ_HARD
  BOOST_FOREACH(Node n, * m->set)
  {
    mmap[n].erase(a);
    mmap[n].erase(b);
    mmap[n].insert(r);
  }
#endif
  /*
  if (!hasComms)
  {
    delete get(a);
    delete get(b);
  }
  */
  communities.erase(a);
  keys.erase(a);
  communities.erase(b);
  keys.erase(b);
  
  
  #ifdef EQ_HARD  
  // qui mettiamo i CID delle communities da aggiornare
  set<CID> toUpdate;
  
  // popliamo update, prendendo tutte le communities che hanno nodi la cui
  // membership è cambiata (e questi per forza sono quelli dell'intersezione)
  BOOST_FOREACH(Node n, * m->set)
    BOOST_FOREACH(CID cid, mmap[n]) 
      toUpdate.insert(cid); 
  
  int upSize = toUpdate.size();
  CommunityPtr* temp = new CommunityPtr[upSize];
  int upc = 0;
  
  // a questo punto chiamiamo l'update solo su queste communities
  BOOST_FOREACH(CID cid, toUpdate)
    temp[upc++] = get(cid);
  
#pragma omp parallel for if (upSize > 10) default(shared)
  for (int iterator = 0; iterator < upSize; iterator++)
    temp[iterator]->update(graph, degrees, membership);
  
  #else
  m->update(graph, degrees, membership);
  #endif
  
  return r;
}

////////////////////////////////////////////////////////////////////////////////
// grim, forgotten and frostbitten
////////////////////////////////////////////////////////////////////////////////    
std::string Communities::summary(bool verbose, bool debug) 
{
  ostringstream s;
  s << "Dimensione: " << size() << endl;
  
  if (verbose)
  {
    s << "Elenco communities:" << endl;
    BOOST_FOREACH(CID c, keys) s << summary(c, debug) << endl;
  }
  
  return s.str();
}

std::string Communities::summary(CID i, bool debug) 
{
  
  ostringstream s;
  if (debug) s << i << " (" << communities[i] << "):";
  else       s << i << ":";
  
  // procuriamoci il vertex selector
  igraph_vector_t* v = communities[i]->toVector();
  igraph_vs_t*    vs = communities[i]->toVertexSelector(v);
  igraph_strvector_t result;
  unsigned int size = getCommunitySize(i);
  igraph_strvector_init(&result, size);
  
  igraph_cattribute_VASV(graph, "label", *vs, &result);
  
  for (unsigned int j = 0; j < size; j++)
    s << " " << STR(result, j);
  
  if (debug)
  {
    s << " - " << communities[i]->comdegree;
#ifdef EQ_HARD
    s << ", " << communities[i]->cf << ", " << communities[i]->dm;
#endif
  }
  
  igraph_strvector_destroy(&result);
  igraph_vs_destroy(vs);
  igraph_vector_destroy(v);
  delete vs;
  delete v;
  
  return s.str();
}



