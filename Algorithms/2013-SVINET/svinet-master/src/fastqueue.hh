#ifndef FASTQUEUE_HH
#define FASTQUEUE_HH

#include <list>
#include <map>
#include <utility>
#include "matrix.hh"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf.h>

typedef std::pair<uint32_t, double> KV;
typedef map<uint32_t, KV *> KVMap;
typedef list<KV *> KVList;

// O(1) updates         -- frequent
// O(log n) inserts     -- infrequent
// no deletions

class FastQueue {
public:
  FastQueue():
    _total(.0), _top_k_total(.0),
    _cache_sum(.0), _cache_nontop_k_pi(.0),
    _cache_psi_sum(.0), _cache_nontop_k_logpi(.0) { }
  ~FastQueue() { reset(); }

  unsigned length() const      		{ return _list.size(); }
  void reset();

  bool update(uint32_t key, double val);
  bool find(uint32_t key, double &val) const;
  void trim();
  void update_cache();
  void update_counts(uint32_t a, const double **gdt,
		     double nrho, double scale);

  double Elogpi(uint32_t k) const;
  double Epi(uint32_t k) const;
  void Epi(Array &) const;
  string s() const;

  double total() const                  { return _total; }
  double nontop_k_weight() const;
  const KVList &list() const { return _list; }
  
  static const double THRESH = 0.8;
  static void static_initialize(uint32_t k, double alpha);
  static uint32_t K() { return _K; }

private:

  KVMap _map;
  KVList _list;
  double _total;
  double _top_k_total;

  double _cache_sum;
  double _cache_nontop_k_pi;

  double _cache_psi_sum;
  double _cache_nontop_k_logpi;

  static uint32_t _K;
  static double _alpha;
};

inline void
FastQueue::static_initialize(uint32_t k, double alpha)
{
  assert (k > 0);
  _K = k;
  _alpha = alpha;
}

inline void
FastQueue::reset()
{
  _map.clear();
  while (KV *e = _list.back()) {
    _list.pop_back();
    delete e;
  }
}

inline bool
FastQueue::find(uint32_t key, double &val) const
{
  assert (key < _K);
  KVMap::const_iterator i = _map.find(key);
  if (i != _map.end()) {
    const KV *kv = i->second;
    if (kv) {
      val = kv->second;
      return true;
    }
  }
  // divide the rest
  val = nontop_k_weight(); 
  return false;
}

inline double
FastQueue::nontop_k_weight() const
{
  return (_total - _top_k_total) / (_K - _list.size());
}

inline bool
FastQueue::update(uint32_t k, double v)
{
  assert (k < _K);
  double oldval = .0;
  bool oldval_topk_exists = false;

  //
  // is (k,v) in top k?
  // if yes, use old entry
  // if no, create new entry
  //
  KVMap::iterator i = _map.find(k);
  KV *kv;
  if (i == _map.end()) {  // create k,v
    //printf("creating new KV\n");
    oldval_topk_exists = false;
    kv = new KV;
  } else {
    kv = i->second;
    oldval_topk_exists = true;
    oldval = kv->second;
  }
  kv->first = k;
  kv->second = v;

  //
  // if (k,v) existed in top k, remove old entry first
  // if not, and if not inserting a new value
  //  adjust total for removing old non-top-k value
  //
  if (oldval_topk_exists) { // val existed in top k
    _top_k_total -= oldval;
    _total -= oldval;
    KVList::iterator delpos = _list.end();
    KVList::iterator k;
    for (k = _list.begin(); k != _list.end(); ++k) {
      KV *curr = *k;
      if (curr->first == kv->first) {
	delpos = k;
	break;
      }
    }
    assert (delpos != _list.end());
    _list.erase(delpos);
  } else
    _total -= nontop_k_weight();
  
  //
  // increment total with new value
  //
  _total += kv->second;

  //
  // does this go in top k?
  //
  bool put_in_top_k = false;
  if (_top_k_total == .0 || _top_k_total / _total < THRESH) {
    _top_k_total += kv->second;
    put_in_top_k = true;
  }
  //printf("put_in_top_k:%d\n", put_in_top_k);
  
  if (put_in_top_k) {
    KVList::iterator pos = _list.end();
    KVList::iterator k;
    for (k = _list.begin(); k != _list.end(); ++k) {
      KV *curr = *k;
      if (curr->first != kv->first && curr->second < kv->second) {
	pos = k;
	break;
      }
    }
    if (pos != _list.end()) 
      _list.insert(pos, kv);
    else
      _list.push_back(kv);
  }

  if (!oldval_topk_exists && put_in_top_k)
    _map[k] = kv;
  else if (oldval_topk_exists && !put_in_top_k) {
    //printf("***\n");
    delete kv;
    _map.erase(k);
  } else if (!oldval_topk_exists && !put_in_top_k)
    delete kv;
  
  trim();
  //printf("Q: %s\n", s().c_str());
  return true;
}

inline void
FastQueue::trim()
{
  if (_top_k_total / _total <= THRESH)
    return;

  double v = _top_k_total;
  KV *kv = NULL;
  do {
    if (kv) {
      _map.erase(kv->first);
      delete kv;
      kv = NULL;
    }
    kv = _list.back();
    if (kv) {
      //printf("popped %d:%f\n", kv->first, kv->second);
      fflush(stdout);
    }
    _list.pop_back();
    v -= kv->second;
  } while (v / _total > THRESH);
  
  if (kv) {
    if (v == 0 || (v + kv->second) / _total >= THRESH) {
      _list.push_back(kv);
      v += kv->second;
    }
    _top_k_total = v;
  }
}

inline void
FastQueue::update_counts(uint32_t a, const double **gdt,
			 double nrho, double scale)
{
  for (uint32_t k = 0; k < _K; ++k) {
    double v = .0;
    find(k, v);
    double u = (1 - nrho) * v + nrho * scale * gdt[a][k];
    //printf("updating (%d:%d->%.5f) with %.5f\n", a, k, v, u);
    update(k, u);
  }
  //update_cache();
  //Array q(_K);
  //Epi(q);
  //printf("Q:%s\n", s().c_str());
  //printf("Epi:%s\n", q.s().c_str());
}

inline double
FastQueue::Elogpi(uint32_t k) const
{
  KVMap::const_iterator i = _map.find(k);
  if (i != _map.end()) {
    return gsl_sf_psi(_alpha + i->second->second) - _cache_psi_sum;
  } else
    return _cache_nontop_k_logpi;
}

inline double
FastQueue::Epi(uint32_t k) const
{
  KVMap::const_iterator i = _map.find(k);
  if (i != _map.end()) {
    return (_alpha + i->second->second) / _cache_sum;
  } else
    return _cache_nontop_k_pi;
}

inline void
FastQueue::Epi(Array &a) const
{
  assert (a.n() == _K);
  for (uint32_t i = 0; i < _K; ++i)
    a[i] = Epi(i);
}

inline void
FastQueue::update_cache()
{
  _cache_sum = _alpha * _K + _total;
  _cache_nontop_k_pi  = (_alpha + nontop_k_weight()) / _cache_sum;
  _cache_psi_sum = gsl_sf_psi(_cache_sum);
  _cache_nontop_k_logpi = gsl_sf_psi(_alpha + nontop_k_weight()) - _cache_psi_sum;
}

inline string
FastQueue::s() const
{
  ostringstream sa;
  sa << "\ntotal:" << _total << "\n";
  sa <<  "top k total:" << _top_k_total << "\n";
  sa << "top k weight frac:" << _top_k_total / _total << "\n";
  sa << "top " << _list.size() << ": (";
  for (KVList::const_iterator k = _list.begin(); k != _list.end(); ++k) {
    const KV *curr = *k;
    sa << curr->first << "," << curr->second << " ";
  }
  sa << ")\n";
  return sa.str();
}

typedef map<uint32_t, FastQueue> SparseCounts;

#endif
