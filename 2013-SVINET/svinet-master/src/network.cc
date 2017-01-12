#include "network.hh"
#include "log.hh"
#include <stdio.h>

static int scurr = 0;

//#define DELIM " "
#define DELIM "\t"

int
Network::read(string s)
{
  fprintf(stdout, "+ Reading network from %s\n", s.c_str());
  FILE *f = fopen(s.c_str(), "r");
  if (!f) {
    lerr("error: cannot open file %s:%s", s.c_str(), strerror(errno));
    exit(-1);
  }
  char b1[512], b2[512];
  string s1, s2;
  uint32_t id1, id2;

  while (!feof(f)) {
    fflush(stdout);
    if (_env.strid) {
      if (fscanf(f, "%s"DELIM"%s\n", b1, b2) < 0) {
	printf("error: unexpected lines in file\n");
	exit(-1);
      }
      //printf("%s -> %s\n", b1, b2);
      s1 = string(b1);
      s2 = string(b2);
      StrMap::iterator r1 = _str2id.find(s1);
      StrMap::iterator r2 = _str2id.find(s2);
      if (r1 == _str2id.end()) {
	_str2id[s1] = scurr;
	_id2str[scurr] = s1;
	scurr++;
      }
      id1 = _str2id[s1];
	
      if (r2 == _str2id.end()) {
	_str2id[s2] = scurr;
	_id2str[scurr]=s2;
	scurr++;
      }
      id2 = _str2id[s2];
    } else {
      if (fscanf(f, "%d"DELIM"%d\n", &id1, &id2) < 0) {
	printf("error: unexpected lines in file\n");
	exit(-1);
      }
    }

    IDMap::iterator i1 = _id2seq.find(id1);
    if (i1 == _id2seq.end() && !add(id1))
      continue;

    IDMap::iterator i2 = _id2seq.find(id2);
    if (i2 == _id2seq.end() && !add(id2))
      continue;

    uint32_t p = _id2seq[id1];
    uint32_t q = _id2seq[id2];

    if (p != q && y(p,q) == 0) { 
      // latter condition is to avoid double
      // counting if edge files use 2 directed
      // edges for an undirected edge
      
      Edge e(p,q);
      Network::order_edge(_env, e);
      //_edges[_ones] = e;
      _edges.push_back(e);

      if (p == e.first)
	e.ftos = true;
      else
	e.ftos = false;
      
      order_edge(_env, e);
      assert(_sparse_y[e.first]);
      _sparse_y[e.first]->push_back(e.second);
      _sparse_y[e.second]->push_back(e.first);

      // todo: _deg only for undirected networks
      // use in_deg and out_deg for directed ones
      if (_env.undirected) {
	_deg[p]++;
	_deg[q]++;
      }
      
      _ones++; // must be same as those pushed into edges
      if (_ones % 10000 == 0) {
	printf("\r+ %d entries", _ones);
	fflush(stdout);
      }
    }
    debug("p = %d, q = %d, y = %d\n", p, q, y(p,q));
    fflush(stdout);

    debug("%d -> %d\n",id1, id2);
    debug("ones = %d\n", _ones);
    debug("distinct nodes = %d\n", (int)_id2seq.size());
  }

  if (_curr_seq != _env.n) {
    _single_nodes = _env.n - _curr_seq;
    printf("n = %d, curr_seq = %d\n", _env.n, _curr_seq);
    printf("+ Creating ids for %d single nodes\n", _env.n - _curr_seq);
    for (uint32_t c = _curr_seq, k = 0; c < _env.n; ++c, ++k)
      add(SINGLE_NODE_START_ID + k);
  }
  fprintf(stdout, "\n+ Done reading network\n");
  fflush(stdout);

  set_env_variables();
  set_avg_deg();
  
  if (_env.nmi) {
    load_ground_truth();
    write_gt_communities();
    //if (_env.model_load) {
    //load_communities();
    //exit(0);
    //}
  }
  if (_env.use_init_communities)
    load_init_communities();

  if (_env.strid) {
    FILE *f = fopen(Env::file_str("/str2id.txt").c_str(), "w");
    assert(f);
    for (StrMap::const_iterator i = _str2id.begin();
	 i != _str2id.end(); ++i) {
      const string &s = i->first;
      const uint32_t &id = i->second;
      fprintf(f, "%s\t%d\n", s.c_str(), id);
    }
    fclose(f);
  }

  info("+ setting sparse zeros\n");

  if (_env.strid)
    fprintf(stdout, "+ Total nodes read = %d\n", scurr);
  
  if (_env.preprocess) {
    set_neighborhood_sets();
    exit(0);
  } else if (_env.informative_sampling)
    load_neighborhood_sets();

  if (_env.groups_file != "")
    load_gt_groups();
  
  info("+ done setting sparse zeros\n");
  return 0;
}

string
Network::sparse_y_s() const
{
  ostringstream sa;
  sa << "\n[\n";
  for (uint32_t i = 0; i < _sparse_y.size(); ++i) {
    sa << i << ":";
    vector<uint32_t> *v = _sparse_y[i];
    if (v)  {
      for (uint32_t j = 0; j < v->size(); ++j) {
	sa << v->at(j);
	if (j < v->size() - 1)
	  sa << ", ";
      }
      sa << "\n";
    }
  }
  sa << "]";
  return sa.str();
}

void
Network::set_avg_deg()
{
  assert(_env.undirected);
  double a = .0;
  for (uint32_t i = 0; i < _deg.n(); ++i)
    a += _deg[i];
  a = a / _env.n;
  _avg_deg = a;
  info("avg degree set to %.3f\n", _avg_deg);
}

bool
Network::relevant_node_ppc(uint32_t i) const
{
  return true; // consider all relevant
  
  if (_deg[i] > 3 && _deg[i] < 2 * _avg_deg) 
    return true;
  return false;
}

void
Network::deg_stats(uint32_t &max, double &avg) const
{
  assert(_env.undirected);
  max = 0;
  uint32_t s = 0;
  uint32_t k = 0;
  for (uint32_t i = 0; i < _deg.n(); ++i) {
    if (relevant_node_ppc(i)) {
      if (_deg[i] > max)
	max = _deg[i];
      s += _deg[i];
      k++;
    }
  }
  avg = (double)s / k;
}

void
Network::set_env_variables()
{
  _env.total_pairs = _env.n * (_env.n - 1) / 2;
  Env::plog("total pairs", _env.total_pairs);

  _env.ones_prob = (double)ones() / _env.total_pairs;
  _env.zeros_prob = 1 - _env.ones_prob;
  Env::plog("ones_prob", _env.ones_prob);
  Env::plog("zeros_prob", _env.zeros_prob);
  
  if (_env.eta_type == "fromdata") {
    _env.eta0 = _env.total_pairs * _env.ones_prob / _env.k;
    _env.eta1 = _env.total_pairs * 1.0 / (_env.k * _env.k) - _env.eta0;
    if (_env.eta1 <= 0)
      _env.eta1 = 1.0;    
  } else if (_env.eta_type == "uniform") {
    _env.eta0 = 1;
    _env.eta1 = 1;
  } else if (_env.eta_type == "sparse") {
    _env.eta0 = _env.eta0_sparse;
    _env.eta1 = _env.eta1_sparse;
  } else if (_env.eta_type == "dense") {
    _env.eta0 = _env.eta0_dense;
    _env.eta1 = _env.eta1_dense;
  } else {
    lerr("unknown eta_type\n");
    assert(0);
  }
}

void
Network::load_ground_truth()
{
  uint32_t singlenodes = 0;
  FILE *f = fopen(_env.ground_truth_fname.c_str(), "r");
  if (!f) {
    fprintf(stderr, "error: cannot read ground truth file %s;"
	    " check path; skipping file\n", 
	    _env.ground_truth_fname.c_str());
    return;
  }

  size_t nbytes = 1024*1024;
  char *s = new char[nbytes];
  char *my_string = (char *) malloc (nbytes);
  int bytes_read = 0;
  while (!feof(f)) {
    uint32_t nid;
    bytes_read = getline(&my_string, &nbytes, f);
    if (bytes_read <= 0)
      break;
    
    //printf("bytes read = %d\n", bytes_read);

    if (sscanf(my_string, "%d\t%[^\n]s\n", &nid, s) < 0) {
      printf("error: cannot read ground truth file %s\n", 
	     _env.ground_truth_fname.c_str());
      exit(-1);
    }
    //printf("%d -> %s\n", nid, s);

    IDMap::iterator i1 = _id2seq.find(nid);
    if (i1 == _id2seq.end())
      singlenodes++;
    assert (singlenodes == 0);

    char *e;
    char *p;
    long u = 0;
    for (p = s; ; p = e) {
      u = strtol(p, &e, 10);
      if (p == e)
	break;
      //printf("%ld\t%d\n", u, nid);
      vector<uint32_t> &v = _gt_communities[u];
      vector<uint32_t> &v2 = _gt_communities_seq[u];
      v.push_back(nid);
      v2.push_back(i1->second);
    }
  }
  delete[] s;
  fclose(f);
  debug("singlenodes = %d (exp: %d)\n", singlenodes, _single_nodes);
  fflush(stdout);
  printf("+ Done loading ground truth\n");
  fflush(stdout);
  //assert (singlenodes == _single_nodes);
}

void
Network::load_communities()
{
  FILE *f = fopen("communities.txt", "r");
  size_t nbytes = 10 * 1024 * 1024;
  char *s = new char[nbytes];
  char *my_string = (char *) malloc (nbytes);
  size_t bytes_read = 0;
  
  uint32_t cid = 0;
  while (!feof(f)) {
    bytes_read = getline(&my_string, &nbytes, f);
    if (bytes_read <= 0)
      break;

    if (sscanf(my_string, "%[^\n]s\n", s) < 0) {
      printf("error: cannot read ground truth file %s\n", 
	     _env.ground_truth_fname.c_str());
      exit(-1);
    }
    //printf("%d -> %s\n", cid, s);
    vector<uint32_t> &v = _communities[cid];

    char *e;
    char *p;
    long u = 0;
    for (p = s; ; p = e) {
      u = strtol(p, &e, 10);
      if (p == e)
	break;
      v.push_back(u);
    }
    cid++;
  }
  delete[] s;
  fclose(f);

  f = fopen("comm_match.txt", "w");
  uint32_t aid = 0;
  for (MapVec::const_iterator it = _communities.begin(); 
       it != _communities.end(); ++it, ++aid) {
    const vector<uint32_t> &v = it->second;
    
    uint32_t  bid = 0;
    for (MapVec::const_iterator jt = _gt_communities.begin(); 
	 jt != _gt_communities.end(); ++jt, ++bid) {
      const vector<uint32_t> &u = jt->second;
      
      uint32_t c = 0;
      for (uint32_t i = 0; i < v.size(); ++i)
	for (uint32_t j = 0; j < u.size(); ++j)
	  if (v[i] == u[j])
	    c++;
      if (c > 0)
	fprintf(f, "%d\t%d\t%d\n", aid, bid, c);
      fflush(f);
    }
  }
  fclose(f);
}

void
Network::load_init_communities()
{
  fprintf(stdout, "+ Loading init communities from %s\n", 
	 _env.init_communities_fname.c_str());
  fflush(stdout);
  
  FILE *f = fopen(_env.init_communities_fname.c_str(), "r");
  size_t nbytes = 10* 1024 * 1024;
  char *s = new char[nbytes];
  char *my_string = (char *) malloc (nbytes);
  size_t bytes_read = 0;
  
  uint32_t cid = 0;
  while (!feof(f)) {
    bytes_read = getline(&my_string, &nbytes, f);
    if (bytes_read <= 0)
      break;

    if (sscanf(my_string, "%[^\n]s\n", s) < 0)
      continue;
    //printf("%d -> %s\n", cid, s);

    vector<uint32_t> &z = _init_community_to_nodes[cid];
    vector<uint32_t> &zseq = _init_community_to_nodes_seq[cid];
    
    char *e;
    char *p;
    long u = 0;
    for (p = s; ; p = e) {
      u = strtol(p, &e, 10);
      if (p == e)
	break;

      IDMap::iterator i1 = _id2seq.find(u);
      assert (i1 != _id2seq.end());

      vector<uint32_t> &v = _init_communities[u];
      vector<uint32_t> &v2 = _init_communities_seq[i1->second];
      
      v.push_back(cid);
      v2.push_back(cid);

      z.push_back(u);
      zseq.push_back(i1->second);
    }
    cid++;
    memset(my_string, 0, nbytes);
  }
  fprintf(stdout, "+ Loaded %d init communities\n", cid);
  fflush(stdout);
  delete[] s;
  fclose(f);

  FILE *g = fopen(Env::file_str("/init_memberships.txt").c_str(), "w");
  for (uint32_t i = 0; i < _env.n; ++i) {
    const vector<uint32_t> &v = _init_communities_seq[i];
    IDMap::iterator i1 = _seq2id.find(i);
    assert (i1 != _seq2id.end());
    fprintf(g, "%d\t", i1->second);
    for (uint32_t j = 0; j < v.size(); ++j)
      fprintf(g, "%d\t", v[j]);
    fprintf(g, "\n");
  }
  fclose(g);
}



void
Network::gt_gml(uint32_t cid, const vector<uint32_t> &ids)
{
  char buf[1024];
  sprintf(buf, Env::file_str("/gtcommunity-%d.gml").c_str(), cid);
  FILE *f = fopen(buf, "w");
  fprintf(f, "graph\n[\n\tdirected 0\n");

  for (uint32_t i = 0; i < ids.size(); ++i) {
    fprintf(f, "\tnode\n\t[\n");
    fprintf(f, "\t\tid %d\n", ids[i]);
    fprintf(f, "\t]\n");
  }

  for (uint32_t i = 0; i < ids.size(); ++i)
    for (uint32_t j = 0; j < ids.size(); ++j) {
      const IDMap &m = id2seq();
      IDMap::const_iterator idti = m.find(ids[i]);
      IDMap::const_iterator idtj = m.find(ids[j]);
      
      assert (idti != m.end() && idtj != m.end());
      uint32_t a = idti->second;
      uint32_t b = idtj->second;
      
      if (a < b && y(a,b) != 0) {
	  fprintf(f, "\tedge\n\t[\n");
	  fprintf(f, "\t\tsource %d\n", ids[i]);
	  fprintf(f, "\t\ttarget %d\n", ids[j]);
	  fprintf(f, "\t\tcolor %d\n", cid);
	  fprintf(f, "\t]\n");
	}
    }
  fclose(f);
}

void
Network::gt_gml_seq(uint32_t cid, const vector<uint32_t> &ids)
{
  char buf[1024];
  sprintf(buf, Env::file_str("/gtseqcommunity-%d.gml").c_str(), cid);
  FILE *f = fopen(buf, "w");
  fprintf(f, "graph\n[\n\tdirected 0\n");

  for (uint32_t i = 0; i < ids.size(); ++i) {
    fprintf(f, "\tnode\n\t[\n");
    fprintf(f, "\t\tid %d\n", ids[i]);
    fprintf(f, "\t]\n");
  }

  for (uint32_t i = 0; i < ids.size(); ++i)
    for (uint32_t j = 0; j < ids.size(); ++j) {
      uint32_t a = ids[i];
      uint32_t b = ids[j];
      
      if (a < b && y(a,b) != 0) {
	  fprintf(f, "\tedge\n\t[\n");
	  fprintf(f, "\t\tsource %d\n", ids[i]);
	  fprintf(f, "\t\ttarget %d\n", ids[j]);
	  fprintf(f, "\t\tcolor %d\n", cid);
	  fprintf(f, "\t]\n");
	}
    }
  fclose(f);
}

void
Network::write_gt_communities()
{
  printf("+ Writing ground truth communities\n");
  fflush(stdout);
  FILE *f = fopen(Env::file_str("/ground_truth.txt").c_str(), "w");
  FILE *g = fopen(Env::file_str("/ground_truth_community_sizes.txt").c_str(), "w");
  uint32_t c = 0;
  for (MapVec::const_iterator it = _gt_communities.begin(); 
       it != _gt_communities.end(); ++it) {
    const vector<uint32_t> &v = it->second;
    //gt_gml(c, v);    
    fprintf(g, "%d\t%ld\n", c++, v.size());
    for (uint32_t i = 0; i < v.size(); ++i)
      fprintf(f, "%d ", v[i]);
    fprintf(f, "\n");
  }
  fclose(f);
  fclose(g);

  // temporary fix!
  f = fopen("ground_truth.txt", "w");
  for (MapVec::const_iterator it = _gt_communities.begin(); 
       it != _gt_communities.end(); ++it) {
    const vector<uint32_t> &v = it->second;
    for (uint32_t i = 0; i < v.size(); ++i)
      fprintf(f, "%d ", v[i]);
    fprintf(f, "\n");
  }
  fclose(f);
  /*
  c = 0;
  f = fopen("estimated_strengths.txt", "w");
  for (MapVec::const_iterator it = _gt_communities_seq.begin(); 
       it != _gt_communities_seq.end(); ++it) {
    const vector<uint32_t> &v = it->second;
    //gt_gml_seq(c++, v);
    uint32_t m = v.size();
    printf("m = %d\n", m);
    uint32_t c = 0;
    for (uint32_t i = 0; i < v.size(); ++i) 
      for (uint32_t j = 0; j < v.size(); ++j)
	if (v[i] < v[j] && y(v[i],v[j]) == 1)
	  c++;
    double strength = (double)c / (m*(m - 1)/2);
    fprintf(f, "%d\t%d\t%d\t%.5f\n", it->first, c, m, strength);
  }
  fclose(f);
  */
}

void
Network::set_neighborhood_sets()
{
  FILE *f = fopen(Env::file_str("/neighbors.bin").c_str(), "wb");
  uint32_t limit = 100;
  map<uint32_t, bool> m;
  map<uint32_t, bool> exhausted_neighbors;

  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  gsl_rng *_r = gsl_rng_alloc(T);

  for (uint32_t i = 0; i < _env.n; ++i) {
    m.clear();
    exhausted_neighbors.clear();
    
    const vector<uint32_t> *v = get_edges(i);
    vector<uint32_t> *zeros = _sparse_zeros[i];
    assert (zeros == NULL);

    _sparse_zeros[i] = new vector<uint32_t>;
    zeros = _sparse_zeros[i];

    if (v->size() == 0) {
      long unsigned int sz = 0;
      fwrite(&i, sizeof(uint32_t), 1, f);
      fwrite(&sz, sizeof(long unsigned), 1, f);
      continue;
    }

#if 0
    uint32_t mm = 0;
    for (uint32_t j = 0; j < v->size() && mm < limit; ++j) {
      uint32_t q = (*v)[j];
      const vector<uint32_t> *u = get_edges(q);
      for (uint32_t k = 0; u && k < u->size() && mm < limit; ++k) {
	uint32_t p = (*u)[k];
	if (i != p && y(i,p) == 0)  {
	  map<uint32_t, bool>::const_iterator z = m.find(p);
	  if (z == m.end()) {
	    if (!_env.randzeros)
	      zeros->push_back(p);
	    mm++;
	    m[i] = true;
	  }
	}
      }
    }
#endif

    uint32_t mm = 0;
    uint32_t j = 0;
    uint32_t cycles = 0;
    if (!_env.randzeros)
      do {
	uint32_t q = (*v)[j];
	const vector<uint32_t> *u = get_edges(q);
	
	map<uint32_t, bool>::const_iterator r = exhausted_neighbors.find(q);
	if (q != i && r == exhausted_neighbors.end()) {
	  uint32_t k = 0;
	  uint32_t c = 0;
	  for (; u && k < u->size() && mm < limit; ++k) {
	    uint32_t p = (*u)[k];
	    
	    if (i != p && y(i,p) == 0)  {
	      map<uint32_t, bool>::const_iterator z = m.find(p);
	      if (z == m.end()) {
		zeros->push_back(p);
		c++;
		mm++;
		m[p] = true;
		if (c >= 10)
		  break;
	      }
	    }
	  }
	  if (k == u->size())
	    exhausted_neighbors[q] = true;
	}
	j = (j + 1) % v->size();
	if (j == 0)
	  cycles++;
      } while (!v || (mm < limit && exhausted_neighbors.size() < v->size()));
    
    //printf("cycles = %d, j = %d\n", cycles, j);

    if (_env.randzeros) {
      mm += limit;
      printf("generating %d random zeros\n", limit);
      //m.clear();
      uint32_t cc = 0;
      do {
    	uint32_t s = gsl_rng_uniform_int(_r, _env.n);
    	if (s != i && y(i,s) == 0) {
    	  map<uint32_t, bool>::const_iterator z = m.find(s);	
    	  if (z == m.end()) {
    	    zeros->push_back(s);
    	    m[s] = true;
    	  }
    	}
    	cc++;
      } while (zeros->size() < mm);
    }

    long unsigned int sz = zeros->size();
    fwrite(&i, sizeof(uint32_t), 1, f);
    fwrite(&sz, sizeof(long unsigned), 1, f);
    if (zeros->size() > 0) {
      uint32_t pairid = 0;
      for (uint32_t zi = 0; zi < zeros->size(); ++zi) {
	pairid = (*zeros)[zi];
	fwrite(&pairid, sizeof(uint32_t), 1, f);
	assert (pairid != i);
      }
    }

    if (i % 100 == 0) {
      printf("%d %d\n", i, mm);
      fflush(stdout);
    }
    
    printf("NODE = %d, size = %ld\n",i, zeros->size());
    for (uint32_t rr = 0; rr < zeros->size(); ++rr)
      tst("%d ", (*zeros)[rr]);
    tst("\n");
  }
  fclose(f);
}

void
Network::load_neighborhood_sets()
{
  FILE *f = fopen("neighbors.bin", "rb");

  if (!f) {
    fprintf(stderr, "error: cannot find neighbors.bin from preprocessing the network\n");
    exit(-1);
  }

  for (uint32_t i = 0; i < _env.n; ++i) {
    uint32_t nid;
    assert (fread(&nid, sizeof(uint32_t), 1, f) != 0);
    assert (nid == i);
    vector<uint32_t> *zeros = _sparse_zeros[nid];
    if (!zeros) {
      _sparse_zeros[i] = new vector<uint32_t>;
      zeros = _sparse_zeros[i];
    }
    long unsigned int sz = 0;
    assert (fread(&sz, sizeof(long unsigned), 1, f) != 0);
    for (uint32_t k = 0; k < sz; ++k) {
      uint32_t pairid;
      assert (fread(&pairid, sizeof(uint32_t), 1, f) != 0);
      zeros->push_back(pairid);
      assert(pairid != nid);
    }
    //if (nid % 1000 == 0) 
    //printf("%d, %ld\n", nid, zeros->size());
  }
  fclose(f);
}

int
Network::load_gt_groups()
{
  FILE *f = fopen(_env.groups_file.c_str(), "r");
  if (!f) {
    lerr("cannot open groups file: %s", strerror(errno));
    return -1;
  }
  uint32_t n = 0;
  while (!feof(f)) {
    char b1[512];
    string s1;
    uint32_t id;
    char gp[512];

    if (_env.strid) { // XXX note trailing \t below
      if (fscanf(f, "%s\t%s\n", b1, gp) < 0) {
	printf("error: unexpected lines in file\n");
	exit(-1);
      }
      s1 = string(b1);
      StrMap::iterator r1 = _str2id.find(s1);
      if (r1 == _str2id.end()) {
	lerr("did not find id for \"%s\"\n", s1.c_str());
	continue;
      }
      id = _str2id[s1];
    } else {
      if (fscanf(f, "%d\t%s\n", &id, gp) < 0) {
	printf("error: unexpected lines in file\n");
	exit(-1);
      }
    }
    IDMap::iterator i = _id2seq.find(id);
    if (i == _id2seq.end()) // more nodes in metadata than in network
      lerr("cannot find seq for id %d\n", id);
    else {
      uint32_t seq = i->second;
      StrMapInv::const_iterator itr = _gt_groups.find(seq);      
      if (itr == _gt_groups.end()) {
	_gt_groups[seq] = string(gp);
	n++;
      }
    }
  }
  printf("Loaded %d group entries\n", n);
  fclose(f);

  FILE *g = fopen(Env::file_str("/gt_groups.txt").c_str(), "w");
  for (StrMapInv::const_iterator i = _gt_groups.begin(); i != _gt_groups.end();
       ++i)
    fprintf(g, "%d\t%s\n", i->first, i->second.c_str());
  fclose(g);
  return 0;
}
