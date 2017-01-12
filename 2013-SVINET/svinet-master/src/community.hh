#ifndef COMMUNITY_HH
#define COMMUNITY_HH

#include "network.hh"

class Community {
public:
  Community() { }
  Community(uint32_t id)
    : _id(id),_n(0),_m(0) { }
  ~Community() { }
  void reset();
  void add_edge(const Env &env, const Edge &a);
  void deg_stats(double &avg, double &max, uint32_t &n);
  void most_influential_node(uint32_t &node, uint32_t &deg);
  uint32_t deg(uint32_t a);
  uint32_t nodes() const { return _network.size(); }

private:
  void add(uint32_t a, uint32_t b);

  uint32_t _id;
  uint32_t _n;
  uint32_t _m;
  SparseMatrix2 _network;
};

inline void
Community::add_edge(const Env &env, const Edge &e)
{
  Edge f = e;
  Network::order_edge(env, f);
  SparseMatrix2::iterator i = _network.find(e.first);
  if (i == _network.end()) 
    _n++; // new node
  SparseMatrix2::iterator j = _network.find(e.second);
  if (j == _network.end()) 
    _n++; // new node
  add(e.first, e.second);
  add(e.second, e.first);
}

inline void
Community::add(uint32_t a, uint32_t b)
{
  vector<uint32_t> &v = _network[a];
  bool found = false;
  for (uint32_t i = 0; i < v.size(); ++i)
    if (v[i] == b)
      found = true;
  assert (!found);
  if (!found) // defensive, there shd be no dups
    v.push_back(b);
}

inline uint32_t
Community::deg(uint32_t a)
{
  SparseMatrix2::iterator i = _network.find(a);
  if (i == _network.end())
    return 0;
  vector<uint32_t> &v = _network[a];
  return v.size();
}

inline void
Community::reset()
{
  _id = 0; _n = 0; _m = 0;
  _network.clear();
}

inline void
Community::deg_stats(double &avg, double &max, uint32_t &node)
{
  double s = .0;
  uint32_t m = 0;
  max = 0;
  avg = 0;
  for (SparseMatrix2::const_iterator i = _network.begin();
       i != _network.end(); ++i) {
    uint32_t n = i->first;
    const vector<uint32_t> &e = i->second;
    s += e.size();
    if (e.size() > max) {
      max = e.size();
      node = n;
    }
    m++;
  }
  avg = s / m ;
}

#endif
