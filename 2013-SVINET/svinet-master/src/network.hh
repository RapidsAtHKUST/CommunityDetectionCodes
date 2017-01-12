#ifndef NETWORK_HH
#define NETWORK_HH

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <stdint.h>
#include "matrix.hh"
#include "env.hh"
#include <string.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf.h>

using namespace std;

#define SPARSE_NETWORK 1 

class Network {
public:
  Network(Env &env):
    _sparse_y(env.n),
    _sparse_zeros(env.n),
    _env(env),
    _curr_seq(0), _ones(0), _single_nodes(0),
    _deg(env.n),_avg_deg(.0) { }
  ~Network() { }

  int read(string s);
  const SparseMatrix &sparse_y() const { return _sparse_y; }
  SparseMatrix &sparse_y() { return _sparse_y; }
  SparseMatrix &sparse_zeros() { return _sparse_zeros; }

  vector<uint32_t> *sparse_zeros(uint32_t a) { return _sparse_zeros[a]; }

  uint32_t n() const;
  yval_t y(uint32_t i, uint32_t j) const;

  const EdgeList &edges() const { return _edges; }
  EdgeList &edgelist() { return _edges; }
  
  uint32_t nlinks() const { return _edges.size(); }

  const Array &deg() const { assert(_env.undirected); return _deg; }
  Array &deg() { assert(_env.undirected); return _deg; }
  void deg_stats(uint32_t &max, double &avg) const;
  uint32_t deg(uint32_t a) const;

  const MapVec &gt_communities() const { return _gt_communities; }
  const MapVec &gt_communities_seq() const { return _gt_communities_seq; }

  const MapVec &init_communities() const { return _init_communities; }
  const MapVec &init_communities_seq() const { return _init_communities_seq; }

  const MapVec &init_community_to_nodes() const { return _init_community_to_nodes; }
  const MapVec &init_community_to_nodes_seq() const { return _init_community_to_nodes_seq; }
  
  uint32_t ones() const { return _ones; }
  uint32_t singles() const { return _single_nodes; }
  bool is_single(uint32_t p) const;

  void set_avg_deg();
  bool relevant_node_ppc(uint32_t i) const;
  void set_neighborhood_sets();
  void load_neighborhood_sets();

  void load_ground_truth();
  void write_gt_communities();
  void load_communities();
  void load_init_communities();
  void gt_gml(uint32_t cid, const vector<uint32_t> &ids);
  void gt_gml_seq(uint32_t cid, const vector<uint32_t> &ids);
  int load_gt_groups();
  void set_env_variables();

  const vector<uint32_t> *get_edges(uint32_t a);

  const IDMap &id2seq() const { return _id2seq; }
  const IDMap &seq2id() const { return _seq2id; }

  const StrMap &str2id() const { return _str2id; }
  const StrMapInv &id2str() const { return _id2str; }

  string gt_group(uint32_t id) const;

  string sparse_y_s() const;
  static void order_edge(const Env &env, Edge &e);
  static bool check_edge_order(const Edge &e);

  static const unsigned int SINGLE_NODE_START_ID = 100000;

private:
  bool add(uint32_t id);

  SparseMatrix _sparse_y;
  SparseMatrix _sparse_zeros;
  EdgeList _edges;
  Env &_env;
  IDMap _id2seq;
  IDMap _seq2id;
  StrMap _str2id;
  StrMapInv _id2str;
  uint32_t _curr_seq;
  uint32_t _ones;
  uint32_t _single_nodes;
  Array _deg;
  double _avg_deg;

  // ground truth
  MapVec _gt_communities;
  MapVec _gt_communities_seq;
  MapVec _communities;
  MapVec _init_communities;
  MapVec _init_communities_seq;

  MapVec _init_community_to_nodes;
  MapVec _init_community_to_nodes_seq;
  StrMapInv _gt_groups;

  // collaboration networks
  int _min_author_degree;
  vector<uint32_t> _core_authors;
};

inline uint32_t
Network::n() const
{
  return _sparse_y.size();
}

inline bool
Network::add(uint32_t id)
{
  if (_curr_seq >= _env.n)
    return false;

  _id2seq[id] = _curr_seq;
  _seq2id[_curr_seq] = id;

  assert (!_sparse_y[_curr_seq]);
  std::vector<uint32_t> **v = _sparse_y.data();
  v[_curr_seq] = new vector<uint32_t>;
  _curr_seq++;
  return true;
}

inline bool
Network::is_single(uint32_t a) const
{
  if (a >= SINGLE_NODE_START_ID)
    return true;
  return false;
}

inline yval_t
Network::y(uint32_t a, uint32_t b) const
{
  Edge e(a,b);
  order_edge(_env, e);
  assert (e.first < _sparse_y.size());
  
  const vector<uint32_t> *v = _sparse_y[e.first];
  if (!v) // singleton node
    return 0;

  //assert (v->size()); // all nodes in sparse list have edges
  uint32_t s = v->size();
  for (uint32_t j = 0; j < s; ++j)
    if (e.second == v->at(j))
      return 1;
  return 0;
}

inline const vector<uint32_t> *
Network::get_edges(uint32_t a)
{
  assert (a < _sparse_y.size());
  const vector<uint32_t> *v = _sparse_y[a];
  return v;
}

inline void
Network::order_edge(const Env &env, Edge &e)
{
  if (!env.undirected)
    return;
  if (Network::check_edge_order(e))
    return;
  e = Edge(e.second, e.first);
}

inline bool
Network::check_edge_order(const Edge &e)
{
  return e.first < e.second;
}

inline uint32_t
Network::deg(uint32_t a) const
{
  return _deg[a];
}

inline string
Network::gt_group(uint32_t id) const
{
  StrMapInv::const_iterator itr = _gt_groups.find(id);
  if (itr == _gt_groups.end())
    return "";
  return itr->second;
}

#endif
