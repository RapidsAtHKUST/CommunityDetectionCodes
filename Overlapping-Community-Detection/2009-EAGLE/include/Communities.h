#ifndef __COMMUNITIES_H
#define __COMMUNITIES_H

#include <string>
#include <igraph.h>
#include <set>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <omp.h>

// versione hardcore di valutazione EQ, utilizza accelerazione su cf e dm, 
// introduce delle correzioni in SetCommunities::merge per mantenere cf e dm 
// aggiornati
#define EQ_HARD

// versione soft, modifica leggermente la formula per utilizzare membership;
// introduce un notevole miglioramento delle prestazioni rispetto alla versione
// presente nel paper.
// #define EQ_SOFT 

struct Community; 

typedef unsigned int Node;
typedef unsigned int CID;

typedef Community* CommunityPtr;
typedef std::set<Node> NodeSet;
typedef const NodeSet* NodeSetPtr;

typedef boost::unordered_map<CID, CommunityPtr> CommunitiesMap;
typedef boost::unordered_map<Node, std::set<CID> > MembershipMap;

/** Community
 
 La classe community rappresenta un insieme di nodi. Le informazioni sui nodi 
 contenuti vengono memorizzate in un set, perchŽ operazioni tipiche eseguite
 sulle community sono intersezioni, unioni e verifiche di appartenenza. I set
 forniscono quindi la flessibilitˆ necessaria.
 Una community non ha alcun riferimento ad un grafo, sebbene i suoi valori ne
 dipendano: questo  fatto per non aumentare ulteriormente il footprint in
 memoria della classe. 
 Oltre all'insieme dei nodi, nella classe vengono memorizzati anche alcuni dati
 che  utile precalcolare, come ad esempio la somma dei gradi dei nodi della
 community; con l'EQ HARD vengono memorizzati altri due parametri, necessari per
 il calcolo veloce dell'EQ.
 */
struct Community
{ 
  /// L'insieme dei nodi che appartengono alla community
  NodeSet* set;
  
  /// La somma dei gradi dei nodi della community
  unsigned int comdegree;
  
  #ifdef EQ_HARD
  /// Fattore di connessione interno
  double cf;
  
  /// grado pesato della community
  double dm;
  #endif
 
  /// Distruttore della classe, si occupa di cancellare set.
  ~Community();
  
  /** 
   Crea una community e ne restituisce il puntatore.
   
   return Il puntatore alla community creata.
   */
  static CommunityPtr spawn();
  
  /** 
   Crea una community unione tra quella su cui  chiamata e quella passata.
   
   param b Il puntatore costante alla community con cui fare l'unione.
   
   return Puntatore alla community unione.
   */
  CommunityPtr unite(const CommunityPtr b) const;
  
  /** 
   Crea una community intersezione tra quella su cui  chiamata e quella passata.
   
   param b Il puntatore costante alla community con cui fare l'intersezione.
   */
  CommunityPtr intersect(const CommunityPtr b) const;
  
  /** 
   Fornisce accesso di sola lettura all'insieme dei nodi.
   
   return Il puntatore costante al set dei nodi.
   */
  NodeSetPtr toSet() const;
  
  /** 
   Restituisce la somma dei quadrati dei gradi dei nodi nella community.
   
   param degrees Puntatore costante all'array dei gradi dei nodi del grafo.
   
   return La somma dei quadrati dei gradi dei nodi della community.
   */
  unsigned int degrees(const unsigned int* degrees);
  
  /** 
   Aggiorna tutti i dati precalcolati, a seconda di quali sono presenti.
   
   param graph Puntatore al grafo utilizzato.
   param degrees Puntatore costante all'array dei gradi dei nodi del grafo.
   param membership Puntatore costante all'array dei gradi di appartenenza dei nodi del grafo.
   */
  void update(igraph_t* graph, const unsigned int* degrees, const unsigned int* membership);

  /** 
   Restituisce un vettore di igraph con riferimento ai nodi della community.
   
   return Puntatore al vettore di igraph generato.
   */
  igraph_vector_t* toVector() const;
  
  /** 
   Restituisce un vertex selector per i nodi della community, associato al 
   vettore passato. Attenzione, perchŽ il vertex selector creato abbia senso,
   il vettore NON deve essere distrutto prima del vertex selector (che  
   quindi una sorta di vista).
   
   param v Puntatore al vettore di igraph su cui costruire il vertex selector.
   
   return Puntatore al vertex selector creato.
   */
  igraph_vs_t* toVertexSelector(igraph_vector_t* v) const;
 
  // Visto che in fase di confronto degli algoritmi questi parametri saranno usati molto,
  // ritengo opportuno calcolarli una volta per tutte anziché chiamare le funzioni getCommunitySize(),
  // getCommunityInternalEdges() e getCommunityBoundaryEdges()
  // virtual bool ownership(unsigned int node, SetCommunityPtr s);
  // virtual unsigned int getCommunityInternalEdges(SetCommunityPtr s); // m_s
  // virtual unsigned int getCommunityBoundaryEdges(SetCommunityPtr s); // c_s    
  unsigned int communitySize;
  unsigned int communityInternalEdges;
  unsigned int communityBoundaryEdges;
  
};

inline
NodeSetPtr Community::toSet() const
{
  return set;
}

inline
void Community::update(igraph_t* graph, const unsigned int* degrees, const unsigned int* membership)
{
  comdegree = 0;
  
  #ifdef EQ_HARD
  cf = 0.0;
  dm = 0.0;
  igraph_bool_t result;
  #endif
  
  
  BOOST_FOREACH(Node v, *set)
  {
    comdegree += degrees[v];
    
    #ifdef EQ_HARD
    dm += (double)degrees[v]/(double)membership[v]; 
    
    BOOST_FOREACH(Node w, *set)
    {
      igraph_are_connected(graph, v, w, &result);
      cf += ((double)result)/(membership[v]*membership[w]);
    }
    #endif
  }
  
  #ifdef EQ_HARD
  dm = dm*dm;
  #endif
}

inline
unsigned int Community::degrees(const unsigned int* degrees)
{
  unsigned int result = 0;
  BOOST_FOREACH(Node n, *set) result += degrees[n]*degrees[n];
  
  return result;
}

inline
CommunityPtr Community::unite(const CommunityPtr b) const
{
  CommunityPtr r = Community::spawn();
  r->set = new NodeSet;
  
  std::set_union(set->begin(), set->end(), b->set->begin(), b->set->end(), 
            std::inserter(*(r->set), r->set->begin()));
  
  return r;
}

inline
CommunityPtr Community::intersect(const CommunityPtr b) const
{
  CommunityPtr r = Community::spawn();
  r->set = new NodeSet;
  std::set_intersection(set->begin(), set->end(), b->set->begin(), b->set->end(), 
                        std::inserter(*(r->set), r->set->begin()));
  
  return r;
}

inline
igraph_vector_t* Community::toVector() const
{
  igraph_vector_t* v = new igraph_vector_t;
  double vids[set->size()];
  
  int i = 0;
  BOOST_FOREACH(unsigned int w, *set) vids[i++] = w;
  
  igraph_vector_init_copy(v, vids, set->size());
  
  return v;
}

inline
igraph_vs_t* Community::toVertexSelector(igraph_vector_t* v) const
{
  igraph_vs_t *vs = new igraph_vs_t;
  igraph_vs_vector(vs, v);
  
  return vs;
}

inline
CommunityPtr Community::spawn()
{  
  CommunityPtr temp = new Community();
  temp->set = NULL;
  
  return temp;
}

/** Communities
 
 La classe Communities gestisce un insieme di istanze di Community;
 essenzialmente questa classe memorizza tutte le informazioni utili su come 
 viene diviso il grafo. 
 Vengono memorizzate molte informazioni ridondanti, per accelerare il calcolo di
 alcuni parametri.
 */
class Communities
{
  protected:
    /// Puntatore al grafo igraph che ha queste communities
    igraph_t* graph;
  
    /// Array contenente i gradi dei nodi del grafo
    unsigned int* degrees;
  
    /// Lista delle communities rappresentate.
    CommunitiesMap communities;
  
    /// Lista dei CID validi.
    std::set<CID> keys;
    
    /// Array contentente il grado di appartenenza di ciascun nodo.
    unsigned int* membership;
  
    /// Contatore utilizzato per assegnare i CID alle communities.
    unsigned int counter;
    
    #ifdef EQ_HARD
    /// Indica a quali communities appartiene ciascun nodo.
    MembershipMap mmap;
    #endif
    
    /// Restituisce un puntatore alla community c.
    CommunityPtr get(unsigned int c);
  
    /// Fattore di scala presente un po' in tutte le formule di EAGLE
    double factor;
  
  public:
    /// Costruttore
    Communities(igraph_t* graph);
  
    /// Costruttore di copia
    Communities(const Communities& c);
    
    /// Distruttore
    ~Communities();
    
    /// Libera la memoria usata dalle community
    void free(); 
    
    /// Porta lo stato di un'istanza al pari dell'altra, liberando memoria
    void sync(Communities& b);
    
    /// Indica se si ha la community i
    bool has(CID i);
  
    ////////////////////////////////////////////////////////////////////////////    
    // utilitˆ
    ////////////////////////////////////////////////////////////////////////////    
    /// Numero di communities
    unsigned int size();
  
    /// Numero di nodi di una community
    unsigned int getCommunitySize(CID s); // n_s
    
    /// Numero di edge interni di una community
    unsigned int getCommunityInternalEdges(CID s); // m_s
    
    /// Numero di edge al confine di una community
    unsigned int getCommunityBoundaryEdges(CID s); // c_s    
  
    /// Restituisce true se il nodo appartiene a quella community
    bool ownership(Node node, CID s);
  
    /// Restituisce il grado di un nodo
    unsigned int getDegree(Node node);
    
    /// Restituisce true se i due nodi sono connessi
    bool areConnected(Node a, Node b);
    
    /** Genera un resoconto sullo stato attuale delle communities.
     param verbose Indica se mostrare i nodi che appartengono alle communities.
     */
    std::string summary(bool verbose = false, bool debug = false);
  
    /// Genera un resoconto su una community
    std::string summary(CID i, bool debug = false);
    
    /// Scrive in uno ostream lo stato attuale dell'istanza
    void dump(std::ostream& stream);
    
    /// Carica da uno istream uno stato
    void load(std::istream& stream);
    
    ////////////////////////////////////////////////////////////////////////////    
    // metriche 
    ////////////////////////////////////////////////////////////////////////////
    // documento 3, sez 4.1
    double conductance(CID s);
    double expansion(CID s);
    double internalDensity(CID s);
    double cutRatio(CID s);
    double normalizedCut(CID s);
    double maxODF(CID s);
    double avgODF(CID s); 
    double flakeODF(CID s);
    double linkDensity(CID s); 
    double scaledLinkDensity(CID s); 

    ////////////////////////////////////////////////////////////////////////////    
    // Metodi di supporto a GCE
    ////////////////////////////////////////////////////////////////////////////    
    int setCommunities(std::string fileName);
    ////////////////////////////////////////////////////////////////////////////    
    // metriche 
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////    
    // Metodi di supporto ad EAGLE
    ////////////////////////////////////////////////////////////////////////////    
    /// Calcola l'EQ della suddivisione attuale
    double EQ();
    
    /// Aggiunge una community alle altre
    CID registerCommunity(CommunityPtr c);
    
    /// Calcola la similarity tra due community
    double similarity(CID i, CID j);
  
    /// Fa il merge tra due community
    unsigned int merge(CID a, CID b, bool hasComms);
    
    /// Crea una community a partire da un set
    unsigned int addCommunity(NodeSet* s);
  
    /// Crea una community a partire da un vettore di igraph
    unsigned int addCommunity(igraph_vector_t* vector);
  
    /// Crea una community con un unico nodo
    unsigned int addCommunity(Node i);
    
    /// Prepara l'istanza per l'esecuzione di EAGLE (fa alcune pulizie)
    void prepareForEAGLE();
    
    /// Precalcola tutti i dati delle community
    void precalculateData();
    
    /// Ottiene il set di nodi di una certa community
    NodeSetPtr getSet(CID s) const;
    
    /// Ottiene l'iteratore alla posizione iniziale dell'insieme delle chiavi
    std::set<CID>::const_iterator begin() const;
    
    /// Ottiene l'iteratore alla posizione finale dell'insieme delle chiavi
    std::set<CID>::const_iterator end() const;
  
};


inline
unsigned int Communities::size()
{
  return communities.size();
}


inline
unsigned int Communities::getDegree(Node node)
{
  return degrees[node];
}

inline
bool Communities::areConnected(Node a, Node b)
{
  igraph_bool_t result;
  igraph_are_connected(graph, a, b, &result);
  
  return result;
}

inline
unsigned int Communities::getCommunitySize(CID s)
{
  return communities[s]->set->size();
}

inline
bool Communities::ownership(Node node, CID s)
{
  return communities[s]->set->find(node) != communities[s]->set->end();
}

inline
CommunityPtr Communities::get(CID c)
{
  return communities[c];
}

inline
std::set<unsigned int>::const_iterator Communities::begin() const
{
  return keys.begin();
}

inline
std::set<unsigned int>::const_iterator Communities::end() const
{
  return keys.end();
}

////////////////////////////////////////////////////////////////////////////    
// metriche 
////////////////////////////////////////////////////////////////////////////

inline
double Communities::conductance(CID s)
{
  double m = (double)communities[s]->communityInternalEdges;
  double c = (double)communities[s]->communityBoundaryEdges;
  return c / (2.0*m + c);
}
 
inline 
double Communities::expansion(CID s)
{
  double n = (double)communities[s]->communitySize;
  double c = (double)communities[s]->communityBoundaryEdges;
  return c / n;
}
 
inline
double Communities::internalDensity(CID s)
{
  double n = (double)communities[s]->communitySize;
  double m = (double)communities[s]->communityInternalEdges;
  return 1.0 - ((2.0 * m) / (n * (n - 1.0)));
}
 
inline
double Communities::cutRatio(CID s)
{
  double n = (double)communities[s]->communitySize;
  double c = (double)communities[s]->communityBoundaryEdges;
  return c / (n * (igraph_vcount(graph) - n));
}

inline 
double Communities::normalizedCut(CID s)
{
  double m = (double)communities[s]->communityInternalEdges;
  double c = (double)communities[s]->communityBoundaryEdges;
  double r1 = conductance(s);
  double r2 = c / (2.0* (igraph_ecount(graph) - m) + c);
  return r1 + r2;
}

inline 
double Communities::linkDensity(CID s)
{
  double n = (double)communities[s]->communitySize;
  double m = (double)communities[s]->communityInternalEdges;
  return 2.0 * m / (n * (n - 1.0));
}

inline 
double Communities::scaledLinkDensity(CID s)
{
  double n = (double)communities[s]->communitySize;
  double m = (double)communities[s]->communityInternalEdges;
  return 2.0 * m / (n - 1.0);
}

inline
double Communities::maxODF(CID s)
{
  double maximumODF = 0.0;
  int err;
  double ODF;
  CommunityPtr a = communities[s];
  
  BOOST_FOREACH(unsigned int v, * a->set)
  {
    igraph_vs_t  vs;
    igraph_vit_t vit;
    // qualcosa non torna nella firma di questa funzione =|
    err = igraph_vs_adj(&vs, v, IGRAPH_ALL);
    
    err = igraph_vit_create(graph, vs, &vit);
    
    double t_degree = (double)IGRAPH_VIT_SIZE(vit); //grado del nodo
    //unsigned int t_degree = degrees[v];
    double o_degree = 0.0; //numero nodi adiacenti esterni alla community
    
    while (!IGRAPH_VIT_END(vit))
	  {
      if (!ownership((long int)IGRAPH_VIT_GET(vit), s)) o_degree += 1.0;
	    IGRAPH_VIT_NEXT(vit);
	  }
    
    ODF = o_degree / t_degree;
    if (ODF > maximumODF)
      maximumODF = ODF;
      
    igraph_vs_destroy(&vs);
    igraph_vit_destroy(&vit);
  }
  return maximumODF;  
}

inline
double Communities::avgODF(CID s)
{
  double averageODF = 0.0;
  // double ODF;
  double n = (double)communities[s]->communitySize;
  CommunityPtr a = communities[s];
  int err;
  
  BOOST_FOREACH(Node v, * a->set)
  {
    igraph_vs_t  vs;
    igraph_vit_t vit;
    
    //restituizione nodi adiacenti
    err = igraph_vs_adj(&vs, v, IGRAPH_ALL); 
    err = igraph_vit_create(graph, vs, &vit);
    
    double t_degree=(double)IGRAPH_VIT_SIZE(vit); //grado del nodo
    double o_degree = 0.0; //numero nodi adiacenti esterni alla community
    
    while (!IGRAPH_VIT_END(vit)){
      //verifica se i nodi appartengono alla community s
      if (!ownership((long int)IGRAPH_VIT_GET(vit), s)) o_degree += 1.0;
	    IGRAPH_VIT_NEXT(vit);
	}
    
    double ODF = o_degree / t_degree;
    averageODF += ODF;
    
    igraph_vs_destroy(&vs);
    igraph_vit_destroy(&vit);
  }
  
  return averageODF / n;  
}

inline
double Communities::flakeODF(CID s)
{
  double flkODF = 0.0;
  double n = (double)communities[s]->communitySize;
  CommunityPtr a = communities[s];
  
  int err;
  BOOST_FOREACH(Node v, * a->set)
  {
    igraph_vs_t  vs;
    igraph_vit_t vit;
    
    //restituizione nodi adiacenti
    err = igraph_vs_adj(&vs, v, IGRAPH_ALL); 
    err = igraph_vit_create(graph, vs, &vit);
    
    double t_degree=(double)IGRAPH_VIT_SIZE(vit); //grado del nodo
    double i_degree = 0.0;                   //numero nodi adiacenti interni alla community
    
	  while (!IGRAPH_VIT_END(vit))
	  {
      if (ownership((long int)IGRAPH_VIT_GET(vit), s)) //verifica se i nodi appartengono alla community s
        i_degree += 1.0;
	    IGRAPH_VIT_NEXT(vit);
	  }
    
    if (i_degree < (t_degree / 2)) flkODF ++;
    
    igraph_vs_destroy(&vs);
    igraph_vit_destroy(&vit);
  }
  return flkODF / n; 
}


inline
double Communities::EQ()
{	
  double r = 0;
  
  const int tempSize = keys.size();
  CommunityPtr* temporary = new CommunityPtr[tempSize];
  std::set<CID>::const_iterator start = keys.begin(), end = keys.end();
  
  int tempIter = 0;
  for (; start != end; start++)
    temporary[tempIter++] = communities[*start];
   
  #ifdef EQ_SOFT
  #pragma omp  parallel for  default(shared) reduction(+:r)
  for (int i = 0; i < tempSize; i++)
  {
    CommunityPtr p  = temporary[i];
    
    BOOST_FOREACH(Node v, * p->set)
      BOOST_FOREACH(Node w, * p->set)
        r += (areConnected(v, w) -factor*getDegree(v)*getDegree(w))/(membership[v]*membership[w]);
  }
  #endif
  
  #ifdef EQ_HARD
  #pragma omp parallel for default(shared) reduction(+:r)
  for (int i = 0; i < tempSize; i++)
    r += temporary[i]->cf - factor*temporary[i]->dm;
  #endif
  
  delete[] temporary;
  return r;
}

inline
double Communities::similarity(CID ia, CID ib)
{
  CommunityPtr a   = communities[ia];
  CommunityPtr b   = communities[ib];
  CommunityPtr bia = a->intersect(b);
  
  double adjs = 0;  
  BOOST_FOREACH(unsigned int v, * a->set)
    BOOST_FOREACH(unsigned int w, * b->set)
      if (v != w && areConnected(v, w)) adjs++;
  
  double tempo = adjs - factor*(a->comdegree*b->comdegree -bia->degrees(degrees));
  
  delete bia;
  
  return tempo;
}

inline
NodeSetPtr Communities::getSet(CID s) const
{
  CommunityPtr c;
  c = communities.at(s);
  
  return c->toSet();
}    

#endif /* __COMMUNITIES_H */
