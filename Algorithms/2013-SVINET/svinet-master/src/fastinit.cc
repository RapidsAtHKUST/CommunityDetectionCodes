#include "fastinit.hh"
#include "log.hh"
#include <sys/time.h>

#define NDEBUG_ITER 5


FastInit::FastInit(Env &env, Network &network, uint32_t max_deg)
  :_env(env), _network(network),
   _n(env.n), _k(5), _max_deg(max_deg),
   _t(env.t), _s(env.s),
   _iter(0), 
   _alpha(_k),
   _family(0), _prev_mbsize0(_s), _prev_mbsize1(_s),
   _eta(_k,_t),
   _gamma(_n,_k), _gammanext(_n),
   _maxgamma(_n),
   _tau0(env.tau0 + 1), _kappa(env.kappa),
   _nodetau0(env.nodetau0 + 1), _nodekappa(env.nodekappa),
   _rhot(.0), _noderhot(_n), _nodec(_n),
   _nodeupdatec(_n),
   _nodeupdate_rn(_n),
   _start_time(time(0)),
   _Epi(_n,_k),
   _count(_n),
   _Elogf(_k),
   _delaylearn_reported(false),
   _max_t(-2147483647),
   _max_h(-2147483647),
   _max_v(-2147483647),
   _prev_h(-2147483647),
   _prev_w(-2147483647),
   _prev_t(-2147483647),
   _nh(0), _nt(0),
   _training_done(false),
   _start_node(0), 
   _n1(0), _n2(0), _n3(0),
   _cn(0), _skipped(0),
   _neighbors(_env.reportfreq),
   _unlikely(0), _prev_unlikely(0)
{
  if (env.undirected)
    _total_pairs = _n * (_n - 1) / 2;
  else
    _total_pairs = _n * (_n - 1);

  fprintf(stdout, "+ Estimating communities on input network with %d nodes\n", _n);
  Env::plog("inference n", _n);
  Env::plog("total pairs", _total_pairs);

  _alpha.set_elements(env.alpha);
  double **d = _eta.data();
  for (uint32_t i = 0; i < _eta.m(); ++i) {
    d[i][0] = env.eta0;
    d[i][1] = env.eta1;
  }
  info("eta = %s", _eta.s().c_str());
  Env::plog("eta", _eta);

  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);

  _hef = fopen(Env::file_str("/heldout-edges.txt").c_str(), "w");
  if (!_hef)  {
    printf("cannot open heldout edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vef = fopen(Env::file_str("/validation-edges.txt").c_str(), "w");
  if (!_vef)  {
    printf("cannot open validation edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _tef = fopen(Env::file_str("/training-edges.txt").c_str(), "w");
  if (!_tef)  {
    printf("cannot open training edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _uf = fopen(Env::file_str("/uncolored-links.txt").c_str(), "w");
  if (!_uf)  {
    printf("cannot open uncolored links file:%s\n",  strerror(errno));
    exit(-1);
  }

  _statsf = fopen(Env::file_str("/stats.txt").c_str(), "w");
  if (!_statsf)  {
    printf("cannot open stats file:%s\n",  strerror(errno));
    exit(-1);
  }

  init_gamma();

  _tf = fopen(Env::file_str("/time.txt").c_str(), "w");
  if (!_tf)  {
    printf("cannot open time file:%s\n",  strerror(errno));
    exit(-1);
  }

  _cf = fopen(Env::file_str("/convergence.txt").c_str(), "w");
  if (!_cf)  {
    printf("cannot open convergence file:%s\n",  strerror(errno));
    exit(-1);
  }

  _cmapf = fopen(Env::file_str("/cmap.txt").c_str(), "w");
  if (!_cmapf)  {
    printf("cannot open cmap file:%s\n",  strerror(errno));
    exit(-1);
  }

  _hf = fopen(Env::file_str("/heldout.txt").c_str(), "w");
  if (!_hf)  {
    printf("cannot open heldout file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vf = fopen(Env::file_str("/validation.txt").c_str(), "w");
  if (!_vf)  {
    printf("cannot open validation file:%s\n",  strerror(errno));
    exit(-1);
  }

  _trf = fopen(Env::file_str("/training.txt").c_str(), "w");
  if (!_trf)  {
    printf("cannot open training file:%s\n",  strerror(errno));
    exit(-1);
  }

  Env::plog("network ones", _network.ones());
  Env::plog("network singles", _network.singles());

  _lf = fopen(Env::file_str("/logl.txt").c_str(), "w");
  if (!_lf)  {
    printf("cannot open logl file:%s\n",  strerror(errno));
    exit(-1);
  }
  _mf = fopen(Env::file_str("/modularity.txt").c_str(), "w");
  if (!_mf)  {
    printf("cannot open modularity file:%s\n",  strerror(errno));
    exit(-1);
  }

  gettimeofday(&_last_iter, NULL);
  _start_time = time(0);

  init_heldout();
}

FastInit::~FastInit()
{
  fclose(_statsf);
  fclose(_hf);
  fclose(_vf);
  fclose(_tf);
  fclose(_lf);
  fclose(_mf);
  fclose(_trf);
  fclose(_cf);
  fclose(_cmapf);
}

string
FastInit::edgelist_s(EdgeList &elist)
{
  ostringstream sa;
  for (EdgeList::const_iterator i = elist.begin(); i != elist.end(); ++i) {
    const Edge &p = *i;
    sa << p.first << "\t" << p.second << "\n";
  }
  return sa.str();
}

void
FastInit::init_gamma()
{
  KV **d = _gamma.data();
  for (uint32_t i = 0; i < _n; ++i) {
    d[i][0].first = i; // group
    d[i][0].second = 1.0 + gsl_rng_uniform(_r);
    _maxgamma[i] = i;
    for (uint32_t j = 1; j < _k; ++j)  {    
      d[i][j].first = (i + j) % _n; // group
      d[i][j].second = gsl_rng_uniform(_r);
    }
  }
}

void
FastInit::do_on_stop()
{
  heldout_likelihood();
  compute_and_log_groups();
}

void
FastInit::set_gamma()
{
  KV **gd = _gamma.data();
  for (uint32_t i = 0; i < _n; ++i) {
    FreqMap &m = _gammanext[i];
    if (!m.size())
      continue;
    // assert (m.size()); // all nodes are connected to at least 1 link

    // at least of size _k
    uint32_t sz = m.size() > _k ? m.size() : _k;
    KVArray v(sz);
    uint32_t c = 0;
    for (FreqMap::const_iterator j = m.begin(); j != m.end(); ++j, ++c) {
      v[c].first = j->first;
      v[c].second = j->second;
    }

    while (c < _k) { // pad with random communities
      uint32_t k = 0;
      do {
	k = gsl_rng_uniform_int(_r, _n);
      } while (m.find(k) != m.end());
      v[c].first = k;
      v[c].second = _env.alpha;
      c++;
    }
    v.sort_by_value();
    // keep the top k from this list
    for (uint32_t k = 0; k < _k; ++k) {
      gd[i][k].first = v[k].first;
      gd[i][k].second = v[k].second + _env.alpha;
    }
    _maxgamma[i] = v[0].first;
    _gammanext[i].clear();
  }
  //printf("gamma = %s\n", _gamma.s().c_str());
}

void
FastInit::batch_infer()
{
  while (1) {
    //if ((_iter > 5 && _prev_unlikely > 0 && _unlikely > _prev_unlikely) || _iter > 15)
    //exit(0);

    if (_iter > log10(_n)) {
      printf("+ Done\n");
      exit(0);
    }
    uint32_t bad = 0;
    for (uint32_t p = 0; p < _n; ++p) {
      const vector<uint32_t> *edges = _network.get_edges(p);
      for (uint32_t r = 0; r < edges->size(); ++r) {
	uint32_t q = (*edges)[r];
      
      if (p >= q)
	continue;
      
      yval_t y = get_y(p,q);
      if (y == 0)
	continue;

      Edge e(p,q);
      Network::order_edge(_env, e);
      const SampleMap::const_iterator u = _heldout_map.find(e);
      if (u != _heldout_map.end())
	continue;

      FreqMap &pmap = _gammanext[p];
      FreqMap &qmap = _gammanext[q];
      pmap[_maxgamma[q]] += 1;
      qmap[_maxgamma[p]] += 1;
      }
    }
    set_gamma();
    estimate_all_pi();

    info("avg. link training likelihood = %.5f\n", training_likelihood());
    
    _iter++;

    tst("iteration = %d took %d secs (family:%d)\n",
	_iter, duration(), _family);
    fflush(stdout);
    do_on_stop();
  }
}


void
FastInit::compute_and_log_groups()
{
  FILE *commf = fopen(Env::file_str("/communities.txt").c_str(), "w");
  FILE *sizef = fopen(Env::file_str("/communities_size.txt").c_str(), "w");
  FILE *aggf = fopen(Env::file_str("/aggregate.txt").c_str(), "w");
  _communities.clear();

  _prev_unlikely = _unlikely;
  _unlikely = 0;

  uint32_t c = 0;
  const IDMap &seq2id = _network.seq2id();
  ostringstream sa;
  Array groups(_n);
  KVArray pi_i(_k);
  for (uint32_t i = 0; i < _n; ++i) {
    _Epi.slice(0, i, pi_i);
    KVArray pi_m(_k);
    const vector<uint32_t> *edges = _network.get_edges(i);
    
    for (uint32_t e = 0; e < edges->size(); ++e) {
      uint32_t m = (*edges)[e];
      if (true) {
	yval_t y = get_y(i,m);
	assert  (y == 1);
	c++;

	_Epi.slice(0, m, pi_m);
	//printf("pi_i: %s\n", pi_i.s().c_str());
	//printf("pi_m: %s\n", pi_m.s().c_str());
	uint32_t max_k = 65535;
	double max = .0;
	double sum = .0;
	for (uint32_t k1 = 0; k1 < _k; ++k1) 
	  for (uint32_t k2 = 0; k2 < _k; ++k2) {
	    if (pi_i[k1].first == pi_m[k2].first) {
	      double u = pi_i[k1].second * pi_m[k2].second;
	      sum += u;
	      if (u > max) {
		max = u;
		max_k = pi_i[k1].first;
	      }
	    }
	  }
	if (sum > .0)
	  max = max / sum;
	else
	  max = .0;
	if (max < _env.link_thresh) {
	  _unlikely++;
	  continue;
	}
	//assert (max_k < _n);
	if (max_k != 65535) {
	  _communities[max_k].push_back(i);
	  _communities[max_k].push_back(m);
	}
      }
    }
  }
  fprintf(_uf, "%d\n", _unlikely);
  //printf("unlikely = %d\n", _unlikely);

  for (std::map<uint32_t, vector<uint32_t> >::const_iterator i = _communities.begin();
       i != _communities.end(); ++i) {
    const vector<uint32_t> &u = i->second;
    
    map<uint32_t, bool> uniq;
    vector<uint32_t> ids;
    for (uint32_t p = 0; p < u.size(); ++p) {
      map<uint32_t, bool>::const_iterator ut = uniq.find(u[p]);
      if (ut == uniq.end()) {
	IDMap::const_iterator it = seq2id.find(u[p]);
	uint32_t id = 0;
	assert (it != seq2id.end());
	id = (*it).second;
	ids.push_back(id);
	uniq[u[p]] = true;
      }
    }
    uArray vids(ids.size());
    for (uint32_t j = 0; j < ids.size(); ++j)
      vids[j] = ids[j];
    vids.sort();
    for (uint32_t j = 0; j < vids.size(); ++j)
      fprintf(commf, "%d ", vids[j]);

    fprintf(commf, "\n");
    fprintf(sizef, "%d\t%ld\n", i->first, uniq.size());
  }
  map<uint32_t, uint32_t> agg;
  for (std::map<uint32_t, uint32_t>::const_iterator i = _mcount.begin();
       i != _mcount.end(); ++i) {
    agg[i->second]++;
  }

  for (std::map<uint32_t, uint32_t>::const_iterator i = agg.begin();
       i != agg.end(); ++i)
    fprintf(aggf, "%d\t%d\n", i->first, i->second);

  fclose(commf);
  fclose(sizef);
  fclose(aggf);

  if (_env.nmi) {
    if (!_env.benchmark) {
      char cmd[1024];
      sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s",
	      _env.ground_truth_fname.c_str(),
	      Env::file_str("/communities.txt").c_str(),
	      Env::file_str("/mutual.txt").c_str());
      if (system(cmd) < 0)
	lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    } else {
      char cmd[1024];
      sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s",
	      Env::file_str("/ground_truth.txt").c_str(),
	      Env::file_str("/communities.txt").c_str(),
	      Env::file_str("/mutual.txt").c_str());
      if (system(cmd) < 0)
	lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    }
  }
}

double
FastInit::edge_likelihood(uint32_t p, uint32_t q, yval_t y) const
{
  KVArray pi_p(_k);
  KVArray pi_q(_k);  
  _Epi.slice(0, p, pi_p);
  _Epi.slice(0, q, pi_q);  
  double s = .0;

  if (y == 1) {
    for (uint32_t k1 = 0; k1 < _k; ++k1) 
      for (uint32_t k2 = 0; k2 < _k; ++k2) {
	if (pi_p[k1].first == pi_q[k2].first) {
	  double u = pi_p[k1].second * pi_q[k2].second;
	  s += u;
	}
      }
  } else {
    for (uint32_t k1 = 0; k1 < _k; ++k1) 
      for (uint32_t k2 = 0; k2 < _k; ++k2) {
	if (pi_p[k1].first != pi_q[k2].first)
	  s += pi_p[k1].second * pi_q[k2].second;
      }
  }

  if (s < 1e-30)
    s = 1e-30;
  return log(s);
}

double
FastInit::training_likelihood() const
{
  double u = .0;
  uint32_t c = 0;
  for (uint32_t p = 0; p < _n; ++p) {
    const vector<uint32_t> *edges = _network.get_edges(p);
    for (uint32_t r = 0; r < edges->size(); ++r) {
      uint32_t q = (*edges)[r];
      
      if (p >= q)
	continue;
      
      u += edge_likelihood(p,q,1);
      c++;
    }
  }
  return u / c;
}

void
FastInit::init_heldout()
{
  int s = _env.heldout_ratio * _network.ones();
  set_heldout_sample(s);
  Env::plog("heldout ratio", _env.heldout_ratio);
  Env::plog("heldout edges (1s and 0s)", _heldout_map.size());
  fprintf(_hef, "%s\n", edgelist_s(_heldout_edges).c_str());
  fclose(_hef);
}

void
FastInit::set_heldout_sample(int s)
{
  if (_env.accuracy)
    return;
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  while (c0 < p || c1 < p) {
    Edge e;
    if (c0 == p)
      get_random_edge(true, e); // link
    else
      get_random_edge(false, e); // nonlink

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p) {
      c0++;
      _heldout_pairs.push_back(e);
      _heldout_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _heldout_pairs.push_back(e);
      _heldout_map[e] = true;
    }
  }
}


void
FastInit::heldout_likelihood()
{
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _heldout_map.begin();
       i != _heldout_map.end(); ++i) {
    
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);
    yval_t y = _network.y(p,q);

    double u = edge_likelihood(p,q,y);
    s += u;
    k += 1;
    if (y) {
      sones += u;
      kones++;
    } else {
      szeros += u;
      kzeros++;
    }
  }
  double nshol = (_zeros_prob * (szeros / kzeros)) + (_ones_prob * (sones / kones));
  fprintf(_hf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(_hf);

  // Use hol @ network sparsity as stopping criteria
  double a = nshol;

  bool stop = false;
  int why = -1;
  if (a > _prev_h && _prev_h != 0 && fabs((a - _prev_h) / _prev_h) < 0.00001) {
    stop = true;
    why = 0;
  } else if (a < _prev_h)
    _nh++;
  else if (a > _prev_h)
    _nh = 0;
  
  if (a > _max_h)
      _max_h = a;
  
  if (_nh > 10) {
    why = 1;
    stop = true;
  }
  _prev_h = a;
  
  if (stop)
    exit(0);
}
