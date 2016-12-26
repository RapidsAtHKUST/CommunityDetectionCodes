#ifndef __MAXCACHE_H
#define __MAXCACHE_H
#include <list>
#include <set>

/** Elemento della cache
 Un massimo è identificato da una coppia di indici, i e j, e da un valore s che
 memorizza la similarity delle due community con indici i e j, appunto.
 
 L'ugaglianza tra un massimo ed un altro è data dall'uguaglianza di i e j.
 Il confronto invece si basa esclusivamente sul valore di s.
 */
struct Max
{
  /// Indice della prima community
  int i;
  
  /// Indice della seconda community
  int j;
  
  /// Similarity delle due community
  double s;
};

/// Confronto LESS THAN per due elementi della cache
inline
bool operator<(const Max& a, const Max& b)
{
  return a.s < b.s;
}


/** Cache dei massimi valori di similarity
 Questa classe implementa la cache dei massimi, cioè un particolare contenitore
 di dimensione limitata che memorizza i valori più alti di similarity che gli 
 vengono dati, assieme agli indici delle community che li hanno generati.
 
 La realizzazione passa attraverso un multiset, e si assicura di rispettare
 tutte le relazioni di consistenza. Quando nella cache è presente l'elemento
 con similarity infinita negativa, significa che la cache è aperta a ricevere 
 elementi qualsiasi essi siano. Questo elemento è inserito in fase di 
 costruzione o dopo una chiamata a clear. Quando invece si fa una get questo
 elemento non viene reinserito. Check è la funzione che inserisce se è il caso
 l'elemento nella collezione. 
 */
class MaxCache
{
  private:
    /// La dimensione massima della cache
    unsigned int size;
  
    /// Insieme senza vincoli di unicità degli elementi di massimo
    std::multiset<Max> cache;
    
  public:
    /**
     Costruttore della cache.
     
     param size La dimensione massima della cache.
     */
    MaxCache(unsigned int size);
    
    /**
     Indica se la cache è vuota, cioè se non ha elementi oppure se l'unico 
     elemento presente è quello a similarity infinita negativa.
     
     return true se la cache è vuota, false altrimenti.
     */
    bool isEmpty();
    
    /**
     Controlla l'elemento fornito per inserirlo eventualmente nella cache.
     
     param i Indice della prima community
     param j Indice della seconda community
     param s valore della similarity calcolato
     */
    void check(unsigned int i, unsigned int j, double s);
  
    /**
     Restituisce l'elemento di massimo valore. Richiede che la cache non sia 
     vouta, altrimenti il risultato è indefinito.
     
     return L'elemento della cache massimo.
     */
    Max get();
    
    // void update(int i, int j, int a);
    
    /**
     Svuota la cache, riportandola alle condizioni iniziali.
     */
    void clear();
};

#endif // __MAXCACHE_H

