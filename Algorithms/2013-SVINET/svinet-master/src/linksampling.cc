#include "linksampling.hh"
#include "log.hh"
#include <sys/time.h>

LinkSampling::LinkSampling(Env &env, Network &network)
  :_env(env), _network(network),
   _n(env.n), _k(env.k),
   _t(env.t), _s(env.s),
   _alpha(_k), _beta(_k),
   _eta(_k,_t),
   _gamma(_n,_k), _lambda(_k,_t),
   _gammanext(_n,_k), _lambdanext(_k,_t),
   _tau0(env.tau0 + 1), _kappa(env.kappa),
   _nodetau0(env.nodetau0 + 1), _nodekappa(env.nodekappa),
   _rhot(.0), _noderhot(_n),
   _start_time(time(0)),
   _Elogpi(_n,_k), _Elogbeta(_k,_t), _Elogf(_k),
   _max_t(-2147483647),
   _max_h(-2147483647),
   _max_v(-2147483647),
   _prev_h(-2147483647),
   _prev_w(-2147483647),
   _prev_t(-2147483647),
   _nh(0), _nt(0),
   _training_done(false),
   _links(60000000,2),
   _converged(_n),
   _active_comms(_n), _active_k(_n),
   _mphi(_n,_k), _fmap(_n,_k),
   _s1(_k), _s2(_k), _s3(_k), _sum(_k),
   _nlinks(0),
   _training_links(_n),
   _annealing_phase(true)
{
  info("+ link sampling init begin\n");
  if (env.undirected)
    _total_pairs = _n * (_n - 1) / 2;
  else
    _total_pairs = _n * (_n - 1);

  info("+ running inference on %d nodes\n", _n);

  Env::plog("inference n", _n);
  Env::plog("total pairs", _total_pairs);

  _alpha.set_elements(env.alpha);
  info("alpha set to %s\n", _alpha.s().c_str());
  
  _ones_prob = double(_network.ones()) / _total_pairs;
  _zeros_prob = 1 - _ones_prob;

  info("ones_prob = %.2f", _ones_prob);
  Env::plog("ones_prob", _ones_prob);
  Env::plog("zeros_prob", _zeros_prob);

  uint32_t max;
  double avg;
  _network.deg_stats(max, avg);
  Env::plog("avg degree", avg);
  Env::plog("max degree", max);

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
  if (_env.seed)
    gsl_rng_set(_r, _env.seed);

  if (_env.create_test_precision_sets) {
    _pef = fopen(Env::file_str("/precision-edges.txt").c_str(), "w");
    if (!_pef)  {
      printf("cannot open heldout edges file:%s\n",  strerror(errno));
      exit(-1);
    }
  }

  _vef = fopen(Env::file_str("/validation-edges.txt").c_str(), "w");
  if (!_vef)  {
    printf("cannot open validation edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _tef = fopen(Env::file_str("/test-edges.txt").c_str(), "w");
  if (!_vef)  {
    printf("cannot open test edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  if (!_env.load_heldout) {
    Env::plog("load validation from file:", false);
    init_validation();
  } else {
    Env::plog("load validation from file:", true);
    load_validation();
  }

  if (_env.load_test) {
    Env::plog("load test from file:", true);
    load_test();
  }

  if (_env.model_load) {
    assert(load_model() >= 0);
  } else if (_env.use_init_communities) {
    init_gamma_external();
    if (!_env.nolambda)
      init_lambda();
  } else {
    init_gamma2();
    assert (init_lambda() >= 0);
  }
  info("+ done initializing gamma, lambda\n");

  // initialize expectations
  set_dir_exp(_gamma, _Elogpi);
  set_dir_exp(_lambda, _Elogbeta);

  info("+ done Elogpi and Elogbeta\n");

  _tf = fopen(Env::file_str("/test.txt").c_str(), "w");
  if (!_tf)  {
    printf("cannot open test file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vf = fopen(Env::file_str("/validation.txt").c_str(), "w");
  if (!_vf)  {
    printf("cannot open validation file:%s\n",  strerror(errno));
    exit(-1);
  }

  Env::plog("network ones", _network.ones());
  Env::plog("network singles", _network.singles());

  _lf = fopen(Env::file_str("/logl.txt").c_str(), "w");
  if (!_lf)  {
    printf("cannot open logl file:%s\n",  strerror(errno));
    exit(-1);
  }

  double a, b, c;
  validation_likelihood(a, b, c);

  info("+ link sampling init end\n");
  gettimeofday(&_last_iter, NULL);
  _start_time = time(0);
}

LinkSampling::~LinkSampling()
{
  fclose(_hf);
  fclose(_vf);
  fclose(_lf);
}

void
LinkSampling::init_validation()
{
  int s1 = _env.heldout_ratio * _network.ones();
  int s0 = _env.heldout_ratio * (_total_pairs * _zeros_prob);

  set_validation_sample(s1);
  if (_env.create_test_precision_sets) {
    if (_env.nonuniform)
      set_precision_biased_sample(s1);
    else
      set_precision_uniform_sample(s1);
  }

  Env::plog("heldout ratio", _env.heldout_ratio);
  Env::plog("validation pairs (1s and 0s)", _validation_map.size());
  fprintf(_vef, "%s\n", edgelist_s(_validation_pairs).c_str());
  fclose(_vef);

  if (_env.create_test_precision_sets) {
    Env::plog("precision pairs (1s and 0s)", _precision_map.size());
    fprintf(_pef, "%s\n", edgelist_s(_precision_pairs).c_str());
    fclose(_pef);
  }
}

string
LinkSampling::edgelist_s(EdgeList &elist)
{
  ostringstream sa;
  for (EdgeList::const_iterator i = elist.begin(); i != elist.end(); ++i) {
    Edge p = *i;
    Network::order_edge(_env, p);
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator a = m.find(p.first);
    IDMap::const_iterator b = m.find(p.second);
    yval_t y = _network.y(p.first, p.second);
    if (a != m.end() && b!= m.end()) {
      sa << a->second << "\t" << b->second << "\t" << (int)y << "\n";
    }
  }
  return sa.str();
}

void
LinkSampling::set_precision_biased_sample(int s1)
{
  int ones = 0, zeros = 0;
  bool st = _env.stratified;
  uint32_t limit = 5;

  while (ones < s1 || zeros < limit*s1) {
    Edge e;
    get_random_edge(true, e); // link

    if (ones < s1) {
      ones++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);
    uint32_t r = gsl_rng_uniform_int(_r, 100);

    uint32_t mm = 0;
    const vector<uint32_t> *v = _network.get_edges(r % 2 ? a : b);
    for (uint32_t j = 0; j < v->size() && mm < limit; ++j) {    
      uint32_t q = (*v)[j];
      const vector<uint32_t> *u = _network.get_edges(q);
      for (uint32_t k = 0; u && k < u->size() && mm < limit; ++k) {
	uint32_t c = (*u)[k];
	if (a != c && _network.y(a,c) == 0) {
	  Edge f(a,c);
	  _precision_pairs.push_back(f);
	  _precision_map[f] = true;
	  mm++;
	  zeros++;
	}
      }
    }
  }
  Env::plog("precision ones:", ones);
  Env::plog("precision zeros:", zeros);
}

void
LinkSampling::set_test_sample(int s)
{
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
      _test_pairs.push_back(e);
      _test_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _test_pairs.push_back(e);
      _test_map[e] = true;
    }
  }
}

void
LinkSampling::set_validation_sample(int s)
{
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

    if (y == 0 && c0 < p) {
      c0++;
      _validation_pairs.push_back(e);
      _validation_map[e] = true;
    }
    if (y == 1 && c1 < p) {
      c1++;
      _validation_pairs.push_back(e);
      _validation_map[e] = true;
    }
  }
}

void
LinkSampling::set_precision_uniform_sample(int s)
{
  int c0 = 0;
  int c1 = 0;
  int p = s;
  int q = (_total_pairs - _network.ones()) * _env.heldout_ratio;
  bool st = _env.stratified;
  while (c0 < q || c1 < p) {
    Edge e;
    if (c0 == q) {
      get_random_edge(true, e);
    } else
      get_random_edge(false, e);

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);
    
    if (y == 0 and c0 < q) {
      c0++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }
    printf("\r%d / %d, %d / %d", c0, q, c1, p);
    fflush(stdout);
  }
  Env::plog("heldout ones:", p);
  Env::plog("heldout zeros:", p);
  Env::plog("precision ones:", p);
  Env::plog("precision zeros:", q);
}


void
LinkSampling::init_gamma()
{
  double **d = _gamma.data();
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _k; ++j)  {
      if (j == i)
	d[i][j] = 1.0 + gsl_rng_uniform(_r);
      else
	d[i][j] = gsl_rng_uniform(_r);
    }
  _gammanext.set_elements(_env.alpha);
}

int
LinkSampling::init_lambda()
{
  if (_lambda.copy_from(_eta) < 0 || _lambdanext.copy_from(_eta) < 0) {
    lerr("init lambda failed");
    return -1;
  }
  return 0;
}

void
LinkSampling::init_gamma2()
{
  Array phi(_k);
  for (uint32_t p = 0; p < _n; ++p) {

    const vector<uint32_t> *edges = _network.get_edges(p);
    for (uint32_t r = 0; r < edges->size(); ++r) {
      uint32_t q = (*edges)[r];
	
      if (p >= q)
	continue;
      
      yval_t y = get_y(p,q);
      assert (y == 1);
	
      phi.zero();
      for (uint32_t k = 0; k < _k ; ++k)
	phi[k] = gsl_rng_uniform(_r);
      phi.normalize();
	
      _gamma.add_slice(p, phi);
      _gamma.add_slice(q, phi);
    }
  }
  _gammanext.set_elements(_env.alpha);
  _lambdanext.copy_from(_eta);
}


void
LinkSampling::init_gamma_external()
{
  _gamma.set_elements(_env.alpha);
  const MapVec &m = _network.init_communities_seq();
  Array phi(_k);
  for (uint32_t p = 0; p < _n; ++p) {

    const vector<uint32_t> *edges = _network.get_edges(p);
    for (uint32_t r = 0; r < edges->size(); ++r) {
      uint32_t q = (*edges)[r];
      
      yval_t y = get_y(p,q);
      assert (y == 1);
	
      phi.zero();
      const double  **elogpid = _Elogpi.const_data();
      for (uint32_t k = 0; k < _k ; ++k)
	phi[k] = _env.alpha; // gsl_rng_uniform(_r);

      //phi.normalize();
      //_gamma.add_slice(q, phi);

      //for (uint32_t k = 0; k < _k; ++k)
      //	_sum[k] += phi[k];      

      MapVec::const_iterator itr = m.find(p);
      if (itr != m.end()) {
	const vector<uint32_t> &r = itr->second;
	for (uint32_t j = 0; j < r.size(); ++j) {
	  if (r[j] >= _k)
	    info("r[j] = %d, p = %d\n", r[j], p);
	  phi[r[j]] += (double)_n / r.size();
	}
      }
      phi.normalize();
      
      _gamma.add_slice(p, phi);
      
      for (uint32_t k = 0; k < _k; ++k)
	_sum[k] += phi[k];
    }
    info("\rdone with %d hosts", p);
  }
  info("\n");

  _gammanext.set_elements(_env.alpha);
  _lambdanext.copy_from(_eta);
}


void
LinkSampling::check_and_set_converged(uint32_t p)
{
  double **gd = _gamma.data();
  uint32_t active_comms = 0;
  uint32_t pk = 0, qk = 0;
  _active_k[p].clear();
  for (uint32_t k = 0; k < _k; ++k)
    if (gd[p][k] - _env.alpha >= 1) {
      active_comms++;
      if (active_comms <= _k / 10)
	_active_k[p].push_back(k);
      pk = k;
    }
  if (active_comms > _k / 10) 
    _active_k[p].clear(); // no use for it; save memory
  
  if (active_comms == 1)
    _converged[p] = pk + 1;
  _active_comms[p] = active_comms;
}

void
LinkSampling::prune()
{
  map<uint32_t, uint32_t> m;
  for (uint32_t  p = 0; p < _n; ++p) {
    check_and_set_converged(p);
    m[_active_comms[p] / 10]++;
  }
  for (map<uint32_t, uint32_t>::const_iterator j = m.begin(); 
       j != m.end(); ++j) {
    uint32_t k = j->first;
    uint32_t v = j->second;
    info("prune: %d\t%d\n", k, v);
  }
}

void
LinkSampling::assign_training_links()
{
  _nlinks = 0;
  double **linksd = _links.data();
  for (uint32_t p = 0; p < _n; ++p)  {
    const vector<uint32_t> *edges = _network.get_edges(p);
    for (uint32_t r = 0; r < edges->size(); ++r) {
      uint32_t q = (*edges)[r];

      if (!_env.accuracy) {
	Edge e(p, q);
	Network::order_edge(_env, e);
	if (!edge_ok(e)) {
	  debug("edge %d,%d is held out\n", e.first, e.second);
	  continue; 
	}
      }

      _training_links[p]++;
      _training_links[q]++;

      if (p >= q)
	continue;
      
      linksd[_nlinks][0] = p;
      linksd[_nlinks][1] = q;
      _nlinks++;
    }
  }
}


void
LinkSampling::compute_mean_indicators()
{
  double **mphid = _mphi.data();
  double **gnextd = _gammanext.data();
  for (uint32_t p = 0; p < _n; ++p) {
    if (_training_links[p] == 0)
      continue;
    
    for (uint32_t k = 0; k < _k; ++k) {
      mphid[p][k] = (gnextd[p][k] - _env.alpha) / _training_links[p];
      _s1[k] += mphid[p][k];
      _s2[k] += mphid[p][k] * mphid[p][k];
      gnextd[p][k] += (_n - _training_links[p] - 1) * mphid[p][k];
      
      if (_annealing_phase)
	gnextd[p][k] *= _network.ones() / _sum[k];
    }
  }
}

void
LinkSampling::clear()
{
  _s1.zero();
  _s2.zero();
  _s3.zero();
  _sum.zero();
}

void
LinkSampling::infer()
{
  _converged.zero();

  set_dir_exp(_gamma, _Elogpi);
  if (!_env.nolambda)
    set_dir_exp(_lambda, _Elogbeta);

  Array phi(_k);
  assign_training_links();
  
  bool write_comm = false;
  double **fmapd = _fmap.data();
  
  while (1) {
    
    if (_env.max_iterations && _iter > _env.max_iterations) {
      printf("+ Quitting: reached max iterations.\n");
      Env::plog("maxiterations reached", true);
      _env.terminate = true;
      do_on_stop();
      exit(0);
    }

    if (_env.max_iterations == 1)
      write_comm = true;

    if (write_comm) {
      _communities.clear();
      _fmap.zero();
    }

    double **gnextd = _gammanext.data();
    double **lnextd = _lambdanext.data();

    double **ld = _lambda.data();
    const double **elogbetad = _Elogbeta.const_data();
    uint32_t links = 0;

    uint32_t single = 0;
    uint32_t remove_links = 0;
    uint32_t total_links  = 0;

    clear();

    uint32_t c = 0, d = 0;
    const double **linksd = _links.const_data();    
    const double  **elogpid = _Elogpi.const_data();
    for (uint32_t n = 0; n < _nlinks; ++n) {
      uint32_t p = linksd[n][0];
      uint32_t q = linksd[n][1];

      if (!_env.accuracy) {
	Edge e(p,q);
	Network::order_edge(_env,e);
	const SampleMap::const_iterator u = _validation_map.find(e);
	assert (u == _validation_map.end());
      }
      
      links = 0;
      phi.zero();

      uint32_t pc = _converged[p];
      uint32_t qc = _converged[q];

      if (pc && !qc) {
	gnextd[p][pc - 1] += 1;
	gnextd[q][pc - 1] += 1;
	_sum[pc - 1]+=2;
	lnextd[pc - 1][0]+=2;
      } else if (!pc && qc) {
	gnextd[q][qc - 1] += 1;
	gnextd[p][qc - 1] += 1;
	_sum[qc - 1]+=2;
	lnextd[qc - 1][0]+=2;
      } else {
	double r = .0;
	if (_iter > 1000 && (_active_comms[p] < _k / 10) && (_active_comms[q] < _k / 10))  {
	  list<uint16_t> l1 = _active_k[p];
	  list<uint16_t> l2 = _active_k[q];
	  l1.sort();
	  l2.sort();
	  l1.merge(l2);
	  l1.unique();
	  
	  bool first = false;
	  for (list<uint16_t>::const_iterator itr = l1.begin();
	       itr != l1.end(); itr++) {
	    uint32_t k = *itr;
	    phi[k] = elogpid[p][k] + elogpid[q][k] + elogbetad[k][0];
	    if (!first) {
	      r = phi[k];
	      first = true;
	    } else if (phi[k] < r)
	      r = r + log(1 + ::exp(phi[k] - r));	    
	    else
	      r = phi[k] + log(1 + ::exp(r - phi[k]));
	  }
	  phi.lognormalize(r, l1);

	  for (list<uint16_t>::const_iterator itr = l1.begin();
	       itr != l1.end(); itr++) {
	    uint32_t k = *itr;
	    gnextd[p][k] += phi[k];
	    gnextd[q][k] += phi[k];
	    lnextd[k][0] += 2 * phi[k];
	    _sum[k] += 2 * phi[k];
	  }
	  d++;


	  if (write_comm) {
	    uint32_t max_k = 65535;
	    double max = phi.max(max_k);
	    
	    if (max > _env.link_thresh) {
	      fmapd[p][max_k]++;
	      fmapd[q][max_k]++;
	      
	      if (fmapd[p][max_k] > _env.lt_min_deg)
		_communities[max_k].push_back(p);
	      if (fmapd[q][max_k] > _env.lt_min_deg)
		_communities[max_k].push_back(q);
	    }
	  }

	} else {

	  for (uint32_t k = 0; k < _k; ++k) {
	    phi[k] = elogpid[p][k] + elogpid[q][k] + elogbetad[k][0];
	    if (k == 0)
	      r = phi[k];
	    else if (phi[k] < r)
	      r = r + log(1 + ::exp(phi[k] - r));	    
	    else
	      r = phi[k] + log(1 + ::exp(r - phi[k]));
	  }
	  phi.lognormalize(r);
	  
	  for (uint32_t k = 0; k < _k; ++k) {
	    gnextd[p][k] += phi[k];
	    gnextd[q][k] += phi[k];
	    lnextd[k][0] += 2 * phi[k];
	    _sum[k] += 2 * phi[k];
	  }
	  c++;

	  if (write_comm) {
	    uint32_t max_k = 65535;
	    double max = phi.max(max_k);
	    
	    if (max > _env.link_thresh) {
	      fmapd[p][max_k]++;
	      fmapd[q][max_k]++;
	      
	      if (fmapd[p][max_k] > _env.lt_min_deg)
		_communities[max_k].push_back(p);
	      if (fmapd[q][max_k] > _env.lt_min_deg)
		_communities[max_k].push_back(q);
	    }
	  }

	}
      }
      if (n % 1000 == 0) {
	printf("\riteration %d: processing %d links", _iter, n);
	fflush(stdout);
      }
    }
    info("\nlocal step on (%d,%d,%d,%d) links\n", c, d, c+d, _nlinks);
    compute_mean_indicators();
    
    info("duration = %d secs\n", duration());
    
    const double **mphid = _mphi.const_data();
    for (uint32_t n = 0; n < _nlinks; ++n) {
      uint32_t p = linksd[n][0];
      uint32_t q = linksd[n][1];

      uint32_t pc = _converged[p];
      uint32_t qc = _converged[q];
      
      if (pc && !qc)
	_s3[pc - 1] += mphid[q][pc];
      else if (!pc && qc) 
	_s3[qc - 1] += mphid[p][qc];
      else
	for (uint32_t k = 0; k < _k; ++k)
	  _s3[k] += mphid[p][k] * mphid[q][k];
    }
    
    for (uint32_t k = 0; k < _k; ++k)
      lnextd[k][1] += _s1[k] * _s1[k] - _s2[k] - _s3[k];

    _gamma.swap(_gammanext);
    _lambda.swap(_lambdanext);

    _gammanext.set_elements(_env.alpha);
    _lambdanext.copy_from(_eta);

    set_dir_exp(_gamma, _Elogpi);
    if (!_env.nolambda)
      set_dir_exp(_lambda, _Elogbeta);
    
    prune();

    if (_env.terminate) {
      do_on_stop();
      _env.terminate = false;
    }

    if (_iter % _env.reportfreq == _env.reportfreq - 1) {
      write_comm = true;
      info("write comm set to %s", write_comm ? "true" : "false");
    } else {
      write_comm = false;
      info("write comm set to %s", write_comm ? "true" : "false");
    }

    info("annealing phase : %s", _annealing_phase ? "true" : "false");
    
    if (_iter % _env.reportfreq == 0) {
      double a, b, c;
      validation_likelihood(a, b, c);
      test_likelihood(_test_map, _tf);

      if (_env.create_test_precision_sets)
	compute_test_likelihood();
      log_communities();
    }
    _iter++;
    info("iteration = %d, duration = %d secs\n", _iter, duration());
  }
}

void
LinkSampling::do_on_stop()
{
  log_communities();
  save_model();
  write_groups();
  if (_env.create_test_precision_sets) {
    precision_likelihood();
    auc();
  }
}

void
LinkSampling::save_model()
{
  FILE *gammaf = fopen(Env::file_str("/gamma.txt").c_str(), "w");
  const double ** const gd = _gamma.const_data();
  for (uint32_t i = 0; i < _n; ++i) {
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);
    if (idt != m.end()) {
      fprintf(gammaf,"%d\t", i);
      fprintf(gammaf,"%d\t", (*idt).second);
      for (uint32_t k = 0; k < _k; ++k) {
	if (k == _k - 1)
	  fprintf(gammaf,"%.5f\n", gd[i][k]);
	else
	  fprintf(gammaf,"%.5f\t", gd[i][k]);
      }
    }
  }
  fclose(gammaf);

  FILE *lambdaf = fopen(Env::file_str("/lambda.txt").c_str(), "w");
  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _k; ++k) {
    fprintf(lambdaf,"%d\t", k);
    for (uint32_t t = 0; t < _t; ++t) {
      if (t == _t - 1)
	fprintf(lambdaf,"%.5f\n", ld[k][t]);
      else
	fprintf(lambdaf,"%.5f\t", ld[k][t]);
    }
  }
  fclose(lambdaf);
}

void
LinkSampling::log_communities()
{
  write_communities(_communities, "/communities.txt");
  if (_env.nmi) {
    char cmd[1024];
    sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s",
	    Env::file_str("/ground_truth.txt").c_str(),
	    Env::file_str("/communities.txt").c_str(),
	    Env::file_str("/mutual.txt").c_str());
    if (system(cmd) < 0)
      lerr("error spawning cmd %s:%s", cmd, strerror(errno));
  }
}

void
LinkSampling::auc()
{
  uint32_t c = 0;
  FILE *f = fopen(Env::file_str("/auc.txt").c_str(), "w");
  for (SampleMap::const_iterator i = _precision_map.begin();
       i != _precision_map.end(); ++i) {
    
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);
    
    yval_t y = _network.y(p,q);
    double u = link_prob(p,q);

    fprintf(f, "%d %.3f\n", y, u);
  }
  fclose(f);
  char cmd[1024];
  sprintf(cmd, "/usr/local/bin/roc < %s >> %s",
	  Env::file_str("/auc.txt").c_str(),
	  Env::file_str("/auc-all.txt").c_str());
  if (system(cmd) < 0)
    lerr("error spawning cmd %s:%s", cmd, strerror(errno));
}


void
LinkSampling::write_communities(MapVec &communities, string name)
{
  const IDMap &seq2id = _network.seq2id();
  FILE *commf = fopen(Env::file_str(name.c_str()).c_str(), "w");
  for (std::map<uint32_t, vector<uint32_t> >::const_iterator i 
	 = communities.begin(); i != communities.end(); ++i) {
    const vector<uint32_t> &u = i->second;

    map<uint32_t, bool> uniq;
    vector<uint32_t> ids;
    vector<uint32_t> seq_ids;
    for (uint32_t p = 0; p < u.size(); ++p) {
      map<uint32_t, bool>::const_iterator ut = uniq.find(u[p]);
      if (ut == uniq.end()) {
	IDMap::const_iterator it = seq2id.find(u[p]);
	uint32_t id = 0;
	assert (it != seq2id.end());
	id = (*it).second;
	ids.push_back(id);
	seq_ids.push_back(u[p]);
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
  }
  fclose(commf);
}

void
LinkSampling::gml(uint32_t cid, const vector<uint32_t> &ids)
{
  char buf[1024];
  sprintf(buf, "community-%d.gml", cid);
  FILE *f = fopen(buf, "w");
  fprintf(f, "graph\n[\n\tdirected 0\n");
  for (uint32_t i = 0; i < ids.size(); ++i) {
    uint32_t g = most_likely_group(ids[i]);
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(ids[i]);
    assert (idt != m.end());
    fprintf(f, "\tnode\n\t[\n");
    fprintf(f, "\t\tid %d\n", ids[i]);
    fprintf(f, "\t\textid %d\n", idt->second);
    fprintf(f, "\t\tgroup %d\n", g);    
    fprintf(f, "\t\tdegree %d\n", _network.deg(ids[i]));
    fprintf(f, "\t]\n");
  }

  Array pp(_k);
  Array qp(_k);
  for (uint32_t i = 0; i < ids.size(); ++i)
    for (uint32_t j = 0; j < ids.size(); ++j) {
      if (ids[i] < ids[j] && _network.y(ids[i],ids[j]) != 0) {
	get_Epi(ids[i], pp);
	get_Epi(ids[j], qp);
	
	Array beta(_k);
	estimate_beta(beta);
	uint32_t max_k = 65536;
	double max = inner_prod_max(pp, qp, beta, max_k);
	if (max < _env.link_thresh)
	  continue;

	if (max_k == cid) {
	  fprintf(f, "\tedge\n\t[\n");
	  fprintf(f, "\t\tsource %d\n", ids[i]);
	  fprintf(f, "\t\ttarget %d\n", ids[j]);
	  fprintf(f, "\t\tcolor %d\n", max_k);
	  fprintf(f, "\t]\n");
	}
      }
    }
  fclose(f);
}

void
LinkSampling::validation_likelihood(double &a, double &a0, double &a1)
{
  if (_env.accuracy)
    return;
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  uint32_t sz = _validation_map.size();
  for (SampleMap::const_iterator i = _validation_map.begin();
       i != _validation_map.end(); ++i) {
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

  double nshol = (_zeros_prob * (szeros / kzeros)) + 
    (_ones_prob * (sones / kones));
  fprintf(_vf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(_vf);

  a = nshol;

  bool stop = false;
  int why = -1;
  if (_iter > 10) {
    if (a > _prev_h && _prev_h != 0 && 
	fabs((a - _prev_h) / _prev_h) < 0.00001) {
      stop = true;
      why = 100;
    } else if (a < _prev_h)
      _nh++;
    else if (a > _prev_h)
      _nh = 0;

    if (a > _max_h) {
      double at0 = 0;
      _max_h = a;
      _max_t = at0;
    }
    
    if (_nh > 2) { // be robust to small fluctuations in predictive likelihood
      why = 1;
      stop = true;
    }
  }
  _prev_h = nshol;
  FILE *f = fopen(Env::file_str("/max.txt").c_str(), "w");
  fprintf(f, "%d\t%d\t%.5f\t%.5f\t%.5f\t%d\n", 
	  _iter, duration(), 
	  a, _max_t, _max_h, why);
  fclose(f);

  if (_annealing_phase && stop) {
    info("annealing phase completed (%d secs) at iteration %d\n", 
	 duration(), _iter);
    _annealing_phase = false;
    _nh = 0;
    stop = false;
    why = 0;
    _prev_h = 0;
  } else if (!_annealing_phase && stop) {
    if (_env.use_validation_stop) {
      do_on_stop();
      exit(0);
    }
  }
}

void
LinkSampling::compute_test_likelihood()
{
  FILE *uf = fopen(Env::file_str("/uniform-hol.txt").c_str(), "a");
  if (!uf)  {
    printf("cannot open uniform hol output file:%s\n",  strerror(errno));
    exit(-1);
  }
  test_likelihood(_uniform_map, uf);
  fclose(uf);

  FILE *bf = fopen(Env::file_str("/biased-hol.txt").c_str(), "a");
  if (!bf)  {
    printf("cannot open biased hol output file:%s\n",  strerror(errno));
    exit(-1);
  }
  test_likelihood(_biased_map, bf);
  fclose(bf);

  biased_auc();
  uniform_auc();
}

void
LinkSampling::load_test_sets()
{
  uint32_t n = 0;
  uint32_t a, b;

  const IDMap &id2seq = _network.id2seq();
  FILE *f = fopen("uniform-heldout-pairs.txt", "r");
  while (!feof(f)) {
    if (fscanf(f, "%d\t%d\n", &a, &b) < 0) {
      fprintf(stderr, "error: cannot read uniform heldout test file\n");
      exit(-1);
    }
    
    IDMap::const_iterator i1 = id2seq.find(a);
    IDMap::const_iterator i2 = id2seq.find(b);
    
    if ((i1 == id2seq.end()) || (i2 == id2seq.end())) {
      fprintf(stderr, "error: id %d or id %d not found in original network\n", 
	      a, b);
      exit(-1);
    }
    Edge e(i1->second,i2->second);
    Network::order_edge(_env, e);
    _uniform_pairs.push_back(e);
    _uniform_map[e] = true;
    ++n;
  }
  Env::plog("loaded uniform test pairs:", n);

  FILE *uef = fopen(Env::file_str("/uniform-test-pairs.txt").c_str(), "w");
  if (!uef)  {
    printf("cannot open uniform pairs file:%s\n",  strerror(errno));
    exit(-1);
  }
  fprintf(uef, "%s\n", edgelist_s(_uniform_pairs).c_str());
  fclose(uef);
  fclose(f);
 
  n = 0;
  f = fopen("biased-heldout-pairs.txt", "r");
  while (!feof(f)) {
    if (fscanf(f, "%d\t%d\n", &a, &b) < 0) {
      fprintf(stderr, "error: cannot read biased heldout test file\n");
      exit(-1);
    }
    
    IDMap::const_iterator i1 = id2seq.find(a);
    IDMap::const_iterator i2 = id2seq.find(b);
    
    if ((i1 == id2seq.end()) || (i2 == id2seq.end())) {
      fprintf(stderr, "error: id %d or id %d not found in original network\n", 
	      a, b);
      exit(-1);
    }
    Edge e(i1->second,i2->second);
    Network::order_edge(_env, e);
    _biased_pairs.push_back(e);
    _biased_map[e] = true;
    ++n;
  }
  FILE *bef = fopen(Env::file_str("/biased-test-pairs.txt").c_str(), "w");
  if (!bef)  {
    printf("cannot open biased pairs file:%s\n",  strerror(errno));
    exit(-1);
  }
  Env::plog("loaded biased test pairs:", n);
  fprintf(bef, "%s\n", edgelist_s(_biased_pairs).c_str());
  fclose(bef);
  fclose(f);
}

void
LinkSampling::test_likelihood(const SampleMap &m, FILE *outf)
{
  if (_env.accuracy)
    return;
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = m.begin(); i != m.end(); ++i) {
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
    debug("edge likelihood for (%d,%d) is %f\n", p,q,u);
  }

  double nshol = (_zeros_prob * (szeros / kzeros)) + (_ones_prob * (sones / kones));
  fprintf(outf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(outf);
}

void
LinkSampling::biased_auc()
{
  uint32_t c = 0;
  FILE *f = fopen(Env::file_str("/biased_auc.txt").c_str(), "w");
  for (SampleMap::const_iterator i = _biased_map.begin();
       i != _biased_map.end(); ++i) {
    
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);
    
    yval_t y = _network.y(p,q);
    double u = link_prob(p,q);

    fprintf(f, "%d %.3f\n", y, u);
  }
  fclose(f);
}

void
LinkSampling::uniform_auc()
{
  uint32_t c = 0;
  FILE *f = fopen(Env::file_str("/uniform_auc.txt").c_str(), "w");
  for (SampleMap::const_iterator i = _uniform_map.begin();
       i != _uniform_map.end(); ++i) {
    
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);
    
    yval_t y = _network.y(p,q);
    double u = link_prob(p,q);

    fprintf(f, "%d %.3f\n", y, u);
  }
  fclose(f);
}


void
LinkSampling::precision_likelihood()
{
  if (_env.accuracy)
    return;
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _precision_map.begin();
       i != _precision_map.end(); ++i) {
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
    debug("edge likelihood for (%d,%d) is %f\n", p,q,u);
  }

  double nshol = (_zeros_prob * (szeros / kzeros)) + (_ones_prob * (sones / kones));
  fprintf(_hf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(_hf);
  auc();
}

int
LinkSampling::load_model()
{
  info("+ loading gamma.txt from %s\n", _env.gamma_location.c_str());
  double **gd = _gamma.data();
  FILE *gammaf = fopen(Env::file_str(_env.gamma_location, "gamma.txt").c_str(), "r");
  if (!gammaf) {
    lerr("no gamma.txt found\n");
    return -1;
  }
  uint32_t n = 0;
  int sz = 32*_k;
  char *line = (char *)malloc(sz);
  while (!feof(gammaf)) {
    if (fgets(line, sz, gammaf) == NULL)
      break;
    //assert (fscanf(gammaf, "%[^\n]", line) > 0);
    debug("line = %s\n", line);
    uint32_t k = 0;
    char *p = line;
    do {
      char *q = NULL;
      double d = strtod(p, &q);
      if (q == p) {
	if (k < _k - 1) {
	  fprintf(stderr, "error parsing gamma file\n");
	  assert(0);
	}
	break;
      }
      p = q;
      if (k >= 2) // skip node id and seq
	gd[n][k-2] = d;
      k++;
    } while (p != NULL);
    n++;
    debug("read %d lines\n", n);
    if (n % 1000 == 0)  {
      printf("\r%d nodes done", n);
      fflush(stdout);
    }
    memset(line, 0, sz);
  }
  assert (n == _n);
  fclose(gammaf);
  memset(line, 0, sz);
  free(line);
  info( "+ loading gamma done\n");

  info("+ loading lambda\n");
  double **ld = _lambda.data();
  FILE *lambdaf = fopen(Env::file_str(_env.gamma_location, "lambda.txt").c_str(), "r");
  if (!lambdaf) {
    lerr("no lambda.txt found\n");
    return -1;
  }
  uint32_t k = 0;
  while (!feof(lambdaf)) {
    if (fgets(line, sz, lambdaf) == NULL)
      break;
    debug("line = %s\n", line);
    uint32_t t = 0;
    char *p = line;
    do {
      char *q = NULL;
      double d = strtod(p, &q);
      if (q == p) {
	if (t < _t - 1) {
	  lerr("error parsing gamma file\n");
	  assert(0);
	}
	break;
      }
      p = q;
      if (t >= 1) // skip seq
	ld[k][t-1] = d;
      t++;
    } while (p != NULL);
    k++;
    memset(line, 0, sz);
  }
  assert (k == _k);
  fclose(lambdaf);
  free(line);

  return 0;
}

uint32_t
LinkSampling::most_likely_group(uint32_t p)
{
  Array Epi(_k);
  get_Epi(p, Epi);
  double max_k = .0, max_p = .0;
  
  for (uint32_t k = 0; k < _k; ++k)
    if (Epi[k] > max_p) {
      max_p = Epi[k];
      max_k = k;
    }
  return max_k;
}

void
LinkSampling::get_Epi(uint32_t n, Array &Epi)
{
  const double ** const gd = _gamma.const_data();
  double *epid = Epi.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[n][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    epid[k] = gd[n][k] / s;
}

void
LinkSampling::load_validation()
{
  uint32_t n = 0;
  uint32_t a, b;
  const IDMap &id2seq = _network.id2seq();
  FILE *f = fopen(_env.load_heldout_fname.c_str(), "r");
  while (!feof(f)) {
    if (fscanf(f, "%d\t%d\n", &a, &b) < 0) {
      lerr("error: cannot read test validation file %s\n", 
	   _env.load_heldout_fname.c_str());
      exit(-1);
    }
    
    IDMap::const_iterator i1 = id2seq.find(a);
    IDMap::const_iterator i2 = id2seq.find(b);
    
    if ((i1 == id2seq.end()) || (i2 == id2seq.end())) {
      lerr("error: id %d or id %d not found in original network\n", 
	   a, b);
      exit(-1);
    }
    Edge e(i1->second,i2->second);
    Network::order_edge(_env, e);
    _validation_pairs.push_back(e);
    _validation_map[e] = true;
    ++n;
  }
  Env::plog("link sampling: loaded validation heldout pairs:", n);
  fprintf(_vef, "%s\n", edgelist_s(_validation_pairs).c_str());
  fclose(_vef);
  fclose(f);
}

void
LinkSampling::load_test()
{
  uint32_t n = 0;
  uint32_t a, b;
  const IDMap &id2seq = _network.id2seq();
  FILE *f = fopen(_env.load_test_fname.c_str(), "r");
  while (!feof(f)) {
    if (fscanf(f, "%d\t%d\n", &a, &b) < 0) {
      lerr("error: cannot read test test file %s\n", 
	   _env.load_test_fname.c_str());
      exit(-1);
    }
    
    IDMap::const_iterator i1 = id2seq.find(a);
    IDMap::const_iterator i2 = id2seq.find(b);
    
    if ((i1 == id2seq.end()) || (i2 == id2seq.end())) {
      lerr("error: id %d or id %d not found in original network\n", 
	   a, b);
      exit(-1);
    }
    Edge e(i1->second,i2->second);
    Network::order_edge(_env, e);
    _test_pairs.push_back(e);
    _test_map[e] = true;
    ++n;
  }
  Env::plog("link sampling: loaded test heldout pairs:", n);
  fprintf(_tef, "%s\n", edgelist_s(_test_pairs).c_str());
  fclose(_tef);
  fclose(f);
}



void
LinkSampling::write_groups()
{
  Array pi(_k);
  const IDMap &seq2id = _network.seq2id();
  FILE *f = fopen(Env::file_str("/groups.txt").c_str(), "w");
  Array groups(_n);
  uint32_t id = 0;
  for (uint32_t i = 0; i < _n; ++i) {
    IDMap::const_iterator it = seq2id.find(i);
    if (it == seq2id.end()) { // single node
      id = i;
    } else
      id = it->second;
    get_Epi(i, pi);
    fprintf(f, "%d\t%d\t", i, id);
    for (uint32_t j = 0; j < _k; ++j) {
      if (j == _k - 1)
	fprintf(f,"%.3f\n", pi[j]);
      else
	fprintf(f,"%.3f\t", pi[j]);
    }
  }
  fclose(f);
}
