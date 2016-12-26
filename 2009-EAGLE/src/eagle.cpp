#include "eagle.h"

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "Communities.h"
#include "MaxCache.h"

#include <assert.h>
#include <omp.h>
using namespace std;

#define PROGRESS(p, l, c) {if (p % l) (cout << c).flush();}

void EAGLE::getInitialCommunities()
{
  ic = new Communities(graph);
  ic->prepareForEAGLE();
  
  // il numero di nodi presente nel grafo
  int vs = igraph_vcount(graph);
  cout << "Numero nodi: " << vs << endl;
  
  int output;
  
  // ottieni le maximal cliques
  cout << "maximal_cliques" << endl;
  igraph_vector_ptr_t cliques; // FIXME: devo essere inizializzato?
  output = igraph_vector_ptr_init(&cliques, 0);
  output = igraph_maximal_cliques(graph, &cliques, k, -1); 
  int size = igraph_vector_ptr_size(&cliques);
  cout << "terminato" << endl;
  
  // filtra le community più con meno di k elementi, registra i nodi subordinati
  // unused verrà inizializzato con tutti i nodi
  NodeSet unused;
  for (int i = 0; i < vs; i++) unused.insert(i);
  
  // per ogni clique trovata
  for (int i = 0; i < size; i++)
  {
    // aux è un riferimento alla clique, s sarà la sua dimensione
    igraph_vector_t* aux = (igraph_vector_t*)VECTOR(cliques)[i];
    int s = igraph_vector_size(aux);
    
    NodeSet* p = new NodeSet;
    
    // popoliamola
    for (int j = 0; j < s; j++) 
    {
      int node = VECTOR(*aux)[j];
      p->insert(node);
    }
    
    // se è più piccola di k la saltiamo
    // if (s < k) continue;
    
    ic->addCommunity(p);
    
    // FIXME: getSet non funziona!
    // FIXME: verificare se erase fallisce malamente se manca l'elemento
    BOOST_FOREACH(Node node, *p) unused.erase(node);
  }
  
  igraph_vector_ptr_destroy(&cliques);
  
  // non resta che creare le community per i nodi subordinati
  BOOST_FOREACH(Node node, unused) ic->addCommunity(node);
}

Communities* EAGLE::run()
{
  getInitialCommunities();
  ic->precalculateData();
  
  unsigned int dcache = ic->size()*3;
  cout << "Dimensione cache: " << dcache << endl;
  MaxCache cache(dcache);
  
  // Impostiamo best e calcoliamo beq
  Communities* best = new Communities(*ic);
  double beq = best->EQ();
  
  // dobbiamo avere membership e comdegree già pronti, e infatti questi vengono
  // gestiti dalla classe Communities
  unsigned int progress = 0;
  unsigned int leap = 1 + ic->size()/200;
  
  while (ic->size() > 1)
  {
    PROGRESS(progress, leap, '.')
    
    // Se la cache è vuota (e la prima volta che si entra nel ciclo ovviamente 
    // lo è) viene popolata. La funzione feedCache fa esattamente questo.
    // Se la cache rimane vuota c'è un errore.
    if (cache.isEmpty()) 
    {
      cache.clear();
      feedCache(cache);
    }
    assert(!cache.isEmpty());
    
    // questo invalida già la cache se necessario, ma non la ripopola.
    Max merge = cache.get();
    
    // questo fa il merge delle communities, cancellando le vecchie e 
    // mantenendo consistente le strutture dati (vedi membership e comdegrees)
    // se best ha merge.i, ha pure merge.j (indagare se sarà sempre false)
    int i = ic->merge(merge.i, merge.j, best->has(merge.i));    
    
    // Valuta EQ e se è il migliore ottenuto lo salva
    double eq = ic->EQ();
    if (eq > beq) 
    {
      // Visto che stiamo cancellando best, questo può avere dei riferimenti
      // alle communities non presenti in ic. Non volendo memory leak, le
      // cancelliamo prima del delete.
      best->sync(*ic);
      delete best;
      
      // Ora facciamo la copia
      best = new Communities(*ic);
      beq = eq;
    }
    
    // l'update viene fatto così solo se la cache non è vuota: in questo caso
    // abbiamo ancora il valore massimo dell'insieme (oltre ai successivi), e 
    // quando si espande l'insieme basta controllare se ci sono valori massimi 
    // tra quelli aggiunti, senza ricontrollare tutto. Questo è valido per 
    // qualsiasi passo, e permette all'algoritmo di evitare un calcolo completo
    // della similarity ad ogni iterazione, altrimenti sarebbe ancora più lento!
    if (!cache.isEmpty()) updateCache(cache, i);
    
    progress += 1;
  }
  
  cout << endl;
  
  // eliminiamo ic, non ci serve più. credo che la modularity sia 0 per una 
  // sola community, quindi siamo sicuri che non sarà mai best. allora facciamo
  // free
  ic->free();
  delete ic;
  return best;
}

void EAGLE::updateCache(MaxCache& cache, unsigned int c)
{
  // copia dei CID in un array temporaneo per openmp
  set<unsigned int>::const_iterator b = ic->begin();
  set<unsigned int>::const_iterator e = ic->end();
  
  // il -1 è perché non vogliamo calcolare la similarity tra c e c.
  int csize = ic->size() -1;
  unsigned int* comms = new unsigned int[csize];
  unsigned int commc = 0;
  
  // facciamo la copia, saltando il caso in cui abbiamo *b == c
  for (; b != e; b++) 
    if (*b != c) comms[commc++] = *b; 
  
  // qui mettiamo i risultati dei thread
  double* temp = new double[csize];
  
  // calcoliamo i risultati; avendo l'array senza l'elemento spurio possiamo
  // fare più in fretta così
#pragma omp parallel for shared(c, temp, comms)
  for (int j = 0; j < csize; j++)
    temp[j] = ic->similarity(c, comms[j]);
  
  // adesso raccogliamo i risultati
  for (int j = 0; j < csize; j++)
    cache.check(c, comms[j], temp[j]);
  
  // eliminiamo gli array temporanei
  delete[] comms;
  delete[] temp;
}

void EAGLE::feedCache(MaxCache& cache)
{
  // limiti
  int maxi = ic->size() -1;
  int maxj = maxi +1;
  
  // numero di iterazioni e progresso
  // unsigned long long int iters = maxi*maxj/2;
  unsigned int leap = 1+ maxj/200;
  unsigned int iters = 0;
  
  // creazione array per omp
  unsigned int* comms = new unsigned int[maxj];
  set<unsigned int>::const_iterator bi = ic->begin();
  set<unsigned int>::const_iterator ei = ic->end();
  unsigned int commc = 0;
  for (; bi != ei; bi++) comms[commc++] = *bi; 
  
  // array dei risultati
  double* temp = new double[maxj];
  
  // ciclo esterno, fissa la prima community (a)
  for (int i = 0; i < maxi; i++)
  {
    PROGRESS(iters, leap, '+')
    
    unsigned int a = comms[i];
    
    // calcoliamo la similarity su più thread, ciclo interno
#pragma omp parallel for default(none) shared(temp, comms, i, a, maxj)
    for (int j = i+1; j < maxj; j++)
      temp[j] = ic->similarity(a, comms[j]);
    
    // il master thread soltanto darà alla cache gli elementi generati
    for (int j = i+1; j < maxj; j++)
      cache.check(a, comms[j], temp[j]);
    
    // abbiamo fatto un'iterazione.
    iters++;
  }
  
  // cancella gli array temporanei
  delete[] comms;
  delete[] temp;
}

EAGLE::EAGLE(igraph_t* graph, int k, std::string prefix)
{
  this->graph = graph;
  this->k = k;
  this->prefix = prefix;
}


