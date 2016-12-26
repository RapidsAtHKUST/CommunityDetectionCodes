#include "MaxCache.h"
#include <math.h>
#include <iostream>

using namespace std;

bool MaxCache::isEmpty()
{
  int s = cache.size();
  
  if (!s) return true;
  if (s == 1) return cache.begin()->i == -1;
  return false;
}

MaxCache::MaxCache(unsigned int size)
{
  this->size = size;
  
  Max m = {-1, -1, -INFINITY};
  cache.insert(m);
}


void MaxCache::check(unsigned int i, unsigned int j, double s)
{
  if (s > cache.begin()->s) 
  {
    Max m = {i, j, s};
    // multiset<Max>::iterator it = 
    cache.insert(m);
    
    // manteniamo la giusta dimensione della cache.
    while (cache.size() > size) cache.erase(cache.begin());
  }
}

/*
// ma questa funzione viene chiamata?
void MaxCache::update(int i, int j, int a)
{
  multiset<Max>::iterator it = cache.begin(); 
  
  // questa operazione è parallelizzabile, volendo. Come fare?
  // inserire gli iteratori, tutti, in un array e poi darli in pasto a omp for
  while (it != cache.end())
  {
    // questa non è un'operazione molto pulita da fare, ma preferisco fare così
    // (aggiro il const) piuttosto che rimuovere e reinserire. 
    // eppure non capisco molto bene il motivo di questa operazione. 
    // se mettessi &n ? 
    Max n = *it;
   
    // il valore che assume
    ((Max)*it).i += (n.i >= a) -(n.i >= i) -(n.i >= j);
    ((Max)*it).j += (n.j >= a) -(n.j >= i) -(n.j >= j);
    
    // vado avanti con l'iteratore
    it++;
  }
}
*/

Max MaxCache::get()
{
  // il migliore dei massimi sarà alla fine, lo estraggo
  Max r = *(--cache.end());
  cache.erase(--cache.end());
  
  // Iteratore per scorrere nel set. Dobbiamo filtrare gli elementi
  multiset<Max>::iterator it = cache.begin(); 
  
  while (it != cache.end())
  {
    if (it->i == r.i || it->j == r.i || it->i == r.j || it->j == r.j) 
    {
      // copio l'iteratore attuale su uno temporaneo
      multiset<Max>::iterator aux = it;
      
      // incremento quello attuale, così punta al prossimo elemento
      it++;
      
      // cancello l'elemento putanto dall'iteratore temporaneo. Non posso usare
      // it normalmente, perché l'operazione di erase invalida l'iteratore
      cache.erase(aux);
    }
    else it++;
  }
  
  return r;
}


void MaxCache::clear()
{
  cache.clear();
  
  Max m = {-1, -1, -INFINITY};  
  cache.insert(m);
}

