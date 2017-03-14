#include "sbm.hh"
#include "log.hh"
#include <sys/time.h>

SBM::SBM(Env &env, Network &network)
  :_env(env), _network(network),
   _n(env.n), _k(env.k), _K(_k+1),
   _t(env.t), _iter(0),
   _alpha(_k), _family(0), 
   _eta(_K,_t), 
   _ones_prob(.0), _zeros_prob(.0),
   _hitcurve_id(0), 
   _inf_epsilon(0.001),
   _r(NULL),
   _gamma(_k), _lambda(_K,_t), _phi(_n,_k),
   _gammat(_k), _lambdat(_K,_t), _phit(_n,_k),
   _tau0(1024*1024 + 1), _kappa(env.kappa),
   _nodetau0(env.nodetau0 + 1), _nodekappa(_env.nodekappa),
   _rhot(.0), _noderhot(_n), _nodec(_n),
   _start_time(time(0)),
   _Elogpi(_k), 
   _Elogbeta(_K,_t), 
   _Epi(_k),
   _max_t(-2147483647),
   _max_h(-2147483647),
   _max_v(-2147483647),
   _prev_h(-2147483647),
   _prev_w(-2147483647),
   _prev_t(-2147483647),
   _nh(0), _nt(0),
   _start_node(0)
{
  printf("nodec = %s\n", _nodec.s().c_str());
  
  fprintf(stdout, "+ sbm initialization begin\n");
  fflush(stdout);

  if (env.undirected)
    _total_pairs = _n * (_n - 1) / 2;
  else
    _total_pairs = _n * (_n - 1);

  Env::plog("inference n", _n);
  Env::plog("total pairs", _total_pairs);

  _alpha.set_elements(env.sbm_alpha);
  info("alpha set to %s\n", _alpha.s().c_str());
  fflush(stdout);

  double **d = _eta.data();
  for (uint32_t i = 0; i < _k; ++i) {
    d[i][0] = env.eta0;
    d[i][1] = env.eta1;
  }
  d[_k][0] = 0.0001;
  d[_k][1] = 1.0;

  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);
  if (_env.seed)
    gsl_rng_set(_r, _env.seed);

  _hef = fopen(Env::file_str("/heldout-pairs.txt").c_str(), "w");
  if (!_hef)  {
    lerr("cannot open heldout edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vef = fopen(Env::file_str("/validation-pairs.txt").c_str(), "w");
  if (!_vef)  {
    lerr("cannot open validation edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  init_heldout();
  printf("+ done heldout\n");

  init_gamma();
  assert (init_lambda() >= 0);
  init_phi();
  Env::plog("lambda", _lambda);

  printf("+ done initializing gamma, lambda\n");

  // initialize expectations
  info("+ Elogpi and Elogbeta\n");
  set_dir_exp(_gamma, _Elogpi);
  set_dir_exp(_lambda, _Elogbeta);
  
  printf("+ done Elogpi and Elogbeta\n");

  tst("Elogpi = %s", _Elogpi.s().c_str());
  tst("Elogbeta = %s", _Elogbeta.s().c_str());

  _tf = fopen(Env::file_str("/time.txt").c_str(), "w");
  if (!_tf)  {
    lerr("cannot open time file:%s\n",  strerror(errno));
    exit(-1);
  }

  _cf = fopen(Env::file_str("/convergence.txt").c_str(), "w");
  if (!_cf)  {
    lerr("cannot open convergence file:%s\n",  strerror(errno));
    exit(-1);
  }

  _hf = fopen(Env::file_str("/heldout.txt").c_str(), "w");
  if (!_hf)  {
    lerr("cannot open heldout file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vf = fopen(Env::file_str("/validation.txt").c_str(), "w");
  if (!_vf)  {
    lerr("cannot open validation file:%s\n",  strerror(errno));
    exit(-1);
  }

  _pf = fopen(Env::file_str("/precision.txt").c_str(), "w");
  if (!_pf)  {
    lerr("cannot open precision file:%s\n",  strerror(errno));
    exit(-1);
  }

  Env::plog("network ones", _network.ones());
  Env::plog("network singles", _network.singles());

  _lf = fopen(Env::file_str("/logl.txt").c_str(), "w");
  if (!_lf)  {
    lerr("cannot open logl file:%s\n",  strerror(errno));
    exit(-1);
  }
  _mf = fopen(Env::file_str("/modularity.txt").c_str(), "w");
  if (!_mf)  {
    lerr("cannot open modularity file:%s\n",  strerror(errno));
    exit(-1);
  }

  gettimeofday(&_last_iter, NULL);
  //_start_time = time(0);
  printf("+ sbm initialization end\n");

#ifdef GLOBALPHIS
  approx_log_likelihood();
#endif
  heldout_likelihood();
  validation_likelihood();
}

SBM::~SBM()
{
  fclose(_hf);
  fclose(_pf);
  fclose(_tf);
  fclose(_lf);
  fclose(_mf);
  fclose(_trf);
  fclose(_cf);
}

void
SBM::init_heldout()
{
  int s = _env.heldout_ratio * _network.ones();
  info("set heldout sample\n");
  set_heldout_sample(s);
  set_validation_sample(s);
  info("set heldout sample done\n");
  info("set precision sample\n");
  set_precision_sample();
  info("set precision sample done\n");

  Env::plog("heldout ratio", _env.heldout_ratio);
  Env::plog("heldout pairs (1s and 0s)", _heldout_map.size());
  Env::plog("precision ratio", _env.precision_ratio);
  
  Env::plog("precision links", precision_ones());
  Env::plog("precision nonlinks", precision_zeros());
  Env::plog("precision pairs (1s and 0s)", _precision_map.size());
  
  fprintf(_hef, "%s\n", edgelist_s(_heldout_pairs).c_str());
  fclose(_hef);
}

string
SBM::edgelist_s(EdgeList &elist)
{
  ostringstream sa;
  for (EdgeList::const_iterator i = elist.begin(); i != elist.end(); ++i) {
    const Edge &p = *i;
    sa << p.first << "\t" << p.second << "\n";
  }
  return sa.str();
}

void
SBM::set_precision_sample()
{
  if (_env.accuracy)
    return;

  int c0 = 0;
  int c1 = 0;
  int p0 = precision_zeros();
  int p1 = precision_ones();

  while (c0 < p0 || c1 < p1) {
    Edge e;
    if (c0 == p0)
      get_random_edge(true, e); // link
    else 
      get_random_edge(false, e); // nonlink

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p0) {
      c0++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }
    if (y == 1 and c1 < p1) {
      c1++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }
  }
}

void
SBM::set_heldout_sample(int s)
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
SBM::set_validation_sample(int s)
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
      _validation_pairs.push_back(e);
      _validation_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _validation_pairs.push_back(e);
      _validation_map[e] = true;
    }
  }
}

void
SBM::save_model()
{
  FILE *phif = fopen(Env::file_str("/phi.txt").c_str(), "w");
  const double ** const phid = _phi.const_data();
  for (uint32_t i = 0; i < _n; ++i) {
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);
    if (idt != m.end()) {
      fprintf(phif,"%d\t", i);
      fprintf(phif,"%d\t", (*idt).second);
      for (uint32_t k = 0; k < _k; ++k) {
	if (k == _k - 1)
	  fprintf(phif,"%.5f\n", phid[i][k]);
	else
	  fprintf(phif,"%.5f\t", phid[i][k]);
      }
    }
  }
  fclose(phif);

  FILE *lambdaf = fopen(Env::file_str("/lambda.txt").c_str(), "w");
  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _K; ++k) {
    fprintf(lambdaf,"%d\t", k);
    for (uint32_t t = 0; t < _t; ++t) {
      if (t == _t - 1)
	fprintf(lambdaf,"%.5f\n", ld[k][t]);
      else
	fprintf(lambdaf,"%.5f\t", ld[k][t]);
    }
  }
  fclose(lambdaf);

  FILE *gammaf = fopen(Env::file_str("/gamma.txt").c_str(), "w");
  const double * const gd = _gamma.const_data();
  for (uint32_t k = 0; k < _k; ++k) {
    fprintf(gammaf,"%d\t", k);
    fprintf(gammaf,"%.5f\n", gd[k]);
  }
  fclose(gammaf);
}


void
SBM::init_gamma()
{
  double *d = _gamma.data();
  for (uint32_t i = 0; i < _k; ++i) {
    double v = (double)100.0 / _k;
    d[i] = gsl_ran_gamma(_r, 100 * v, 0.01);
  }
  info("init gamma = %s\n", _gamma.s().c_str());
}


void
SBM::init_phi()
{
  double **d = _phi.data();
  for (uint32_t n = 0; n < _n; ++n) {
    double s = .0;
    for (uint32_t k = 0; k < _k; ++k) {
      double v = (double)100.0 / _k;
      d[n][k] = gsl_ran_gamma(_r, v * 100, 0.01);
      s += d[n][k];
    }
    for (uint32_t k = 0; k < _k; ++k)
      d[n][k] /= s;
  }
  info("init phi = %s\n", _phi.s().c_str());
}

int
SBM::init_lambda()
{
  tst("init lambda: %s\n", _lambda.s().c_str());
  if (_lambda.copy_from(_eta) < 0) {
    lerr("init lambda failed");
    return -1;
  }
  double **ld = _lambda.data();
  for (uint32_t k = 0; k < _K; ++k)
    for (uint32_t t = 0; t < _t; ++t) {
      double v = (_k <= 100) ? 1.0 : (double)100.0 / _k;
      ld[k][t] = gsl_ran_gamma(_r, 100 * v, 0.01);
    }
  return 0;
}

void
SBM::update_phit(uint32_t p, const vector<Edge> &pairs, 
		 double gamma_scale, double phi_scale)
{
  Array u(_k);
  // compute PHI[p][k]
  for (uint32_t k = 0; k < _k; ++k) {
    double sum = .0;
    tst("pairs size = %d\n", pairs.size());
    for (uint32_t j = 0; j < pairs.size(); j++) {
      uint32_t a = pairs[j].first;
      uint32_t b = pairs[j].second;
      if (p == a)
	update_phi_helper(_phi, a, b, k, sum);
      else
	update_phi_helper(_phi, b, a, k, sum);
      tst("after (%d:%d) sum = %.5f\n", a, b, sum);
    }
    u[k] = _Elogpi[k] + phi_scale * sum;
  }
  tst("before norm phit for %d = %s\n", p, u.s().c_str());
  u.lognormalize();
  tst("setting phit for %d to %s\n", p, u.s().c_str());
  _phit.set_elements(p, u);
  tst("Elogpi = %s\n", _Elogpi.s().c_str());
}

void
SBM::batch_update_phi(uint32_t p) // batch
{
  Array a(_k);
  tst("batch: init phi = %s", a.s().c_str());
  // compute _phi[p][k]
  for (uint32_t k = 0; k < _k; ++k) {
    double sum = _Elogpi[k];
    for (uint32_t q = 0; q < _n; ++q) {
      Edge e(p,q);
      Network::order_edge(_env, e);
      if (!edge_ok(e))
	continue;
      update_phi_helper(_phi, p, q, k, sum);
    }
    a[k] = sum;
  }
  tst("batch: before norm phi = %s", a.s().c_str());
  a.lognormalize();
  tst("batch: after norm phi = %s", a.s().c_str());
  _phi.set_elements(p, a);
}

void
SBM::update_phi_helper(const Matrix &m, 
		       uint32_t p, uint32_t q, uint32_t k, 
		       double &sum)
{
  const double **phid = m.data();
  const double ** const elogbetad = _Elogbeta.const_data();
  yval_t y = get_y(p, q);
  
  // j takes on group that is not equal to k
  tst("q=%d, k=%d",q, k);
  tst("phid [%d][%d] = %.5f", q, k, phid[q][k]);
  sum += (1 - phid[q][k]) * (y * elogbetad[_k][0] + (1 - y) * elogbetad[_k][1]);

  // j takes on group k
  sum += phid[q][k] * (y * elogbetad[k][0] + (1 - y) * elogbetad[k][1]);
  return;
}

void
SBM::batch_infer()
{
  printf("Running batch inference\n");
  while (1) {
    tst("gamma = %s\n", _gamma.s().c_str());
    tst("lambda = %s\n", _lambda.s().c_str());
    tst("phi = %s\n", _phi.s().c_str());

    double **ld = _lambda.data();
    double **phid = _phi.data();
    const double ** const etad = _eta.const_data();

    Array prev(_k), curr(_k), diff(_k);
    uint32_t max_estep_itr = 10;
    uint32_t itr = 0;
    while (itr < max_estep_itr) {
      double msum = .0;
      for (uint32_t p = 0; p < _n; ++p) {
	_phi.slice(0, p, prev);
	batch_update_phi(p);
	_phi.slice(0, p, curr);
	sub(prev, curr, diff);
	msum += diff.abs().sum();
      }
      if (msum < 0.01)
	break;
      itr++;
    }
    
    double *gd = _gamma.data();
    for (uint32_t k = 0; k < _k; ++k) {
      double sum = .0;
      for (uint32_t i = 0; i < _n; ++i)
	sum += phid[i][k];
      gd[k] = _alpha[k] + sum;
    }

    Array sum_a(_K), sum_b(_K);
    for (uint32_t k = 0; k < _k; ++k) {
      
      Array s(_k);
      for (uint32_t i = 0; i < _n; ++i) {
	for (uint32_t j = 0; j < _n; ++j) {
	  if (i >= j)
	    continue;

	  Edge e(i,j);
	  Network::order_edge(_env, e);
	  if (!edge_ok(e))
	    continue;
	  
	  yval_t y = get_y(i,j);                    // todo: check heldout
	  sum_a[k] += y * phid[i][k] * phid[j][k];
	  sum_b[k] += (1 - y) * phid[i][k] * phid[j][k];
	  s[k] += phid[i][k] * phid[j][k];
	  if (y == 1)
	    sum_a[_k] += (1 - phid[i][k] * phid[j][k]);
	  else
	    sum_b[_k] += (1 - phid[i][k] * phid[j][k]);
	}
      }
    }
    for (uint32_t k = 0; k < _K; ++k) {
      ld[k][0] = etad[k][0] + sum_a[k];
      ld[k][1] = etad[k][1] + sum_b[k];
    }

    set_dir_exp(_gamma, _Elogpi);
    set_dir_exp(_lambda, _Elogbeta);
    estimate_pi();

#ifdef GLOBALPHIS
    approx_log_likelihood();
#endif

    heldout_likelihood();
    validation_likelihood();
    _iter++;
    if (_env.terminate) {
      save_model();
      gml();
      printf("Saved model and gml file\n");
      fflush(stdout);
      _env.terminate = false;
    }
  }
}

void
SBM::infer()
{
  while (1) {
    _lambdat.zero();
    _gammat.zero();

    vector<uint32_t> nodes;
    uint32_t type = gsl_ran_bernoulli(_r, _inf_epsilon);
    
    if (type == 0)
      opt_process(nodes);
    else // non-informative sets
      opt_process_noninf(nodes);
    
    set_dir_exp(_gamma, _Elogpi);
    set_dir_exp(_lambda, _Elogbeta);

    if (_iter % _env.reportfreq == 0 || _env.terminate)  {
      estimate_pi();
      printf("%d iterations, type = %d, %d secs\n", _iter, type, duration());
      //printf("gamma = %s\n", _gamma.s().c_str());
      //printf("lambda = %s\n", _lambda.s().c_str());
      
      heldout_likelihood();
      validation_likelihood();
      uint32_t c10, c100, c1000;
      compute_precision(c10, c100, c1000);

#ifdef GLOBALPHIS
    approx_log_likelihood();
#endif
      if (_env.terminate) {
	save_model();
	gml();
	_env.terminate = false;
	printf("Saved model and gml file\n");
	fflush(stdout);
      }
      fflush(stdout);
    }
    _iter++;
  }
}

// update all phis
void
SBM::update_phi(const vector<uint32_t> &nodes)
{
  double **phid = _phi.data();
  double **phitd = _phit.data();

  for (uint32_t i = 0; i < nodes.size(); ++i) {
    uint32_t a = nodes[i];
    _noderhot[a] = pow(_nodetau0 + _nodec[a], -1 * _nodekappa);    
    _nodec[a]++;

    double q = _noderhot[a];
    
    Array v(_k);
    double sum = .0;
    for (uint32_t k = 0; k < _k; ++k) {
      v[k] = (1 - q) * phid[a][k] + q * phitd[a][k];
      sum += v[k];
    }
    for (uint32_t k = 0; k < _k; ++k)
      phid[a][k] = v[k] / sum;
  }
}

void
SBM::update_phi2(uint32_t a)
{
  double **phid = _phi.data();
  double **phitd = _phit.data();

  _noderhot[a] = pow(_nodetau0 + _nodec[a], -1 * _nodekappa);    
  _nodec[a]++;
    
  Array v(_k);
  double sum = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    v[k] = (1 - _noderhot[a]) * phid[a][k] + 
      _noderhot[a] * phitd[a][k];
    sum += v[k];
  }
  for (uint32_t k = 0; k < _k; ++k)
    phid[a][k] = v[k] / sum;
}

void
SBM::update_gamma_lambda()
{
  double *gd = _gamma.data();
  double *gdt = _gammat.data();

  _rhot = pow(_tau0 + (_iter + 1), -1 * _kappa);
  tst("rhot = %.5f\n", _rhot);
  for (uint32_t k = 0; k < _k; ++k) 
    gd[k] = (1 - _rhot) * gd[k] + _rhot * gdt[k];

  double **ldt = _lambdat.data();
  double **ld = _lambda.data();

  for (uint32_t k = 0; k < _K; ++k)
    for (uint32_t t = 0; t < _t; ++t)
      ld[k][t] = (1 - _rhot) * ld[k][t] + _rhot * ldt[k][t];
}

void
SBM::update_gammat_lambdat(const vector<Edge> &pairs, 
			   double gamma_scale, 
			   double lambda_scale)
{
  double *gdt = _gammat.data();
  double **ldt = _lambdat.data();
  double **phid = _phi.data();
  double **etad = _eta.data();

  Array sum_a(_k), sum_b(_k);
  uint32_t n0 = 0, n1 = 0;
  double s0 = .0,  s1 = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    double gsum = .0;
    for (uint32_t i = 0; i < pairs.size(); i++) {
      uint32_t a = pairs[i].first;
      uint32_t b = pairs[i].second;
      
      Edge e(a, b);
      Network::order_edge(_env, e);
      if (!edge_ok(e))
	continue;

      yval_t y = get_y(a,b);
      
      gsum += phid[a][k] + phid[b][k];
      sum_a[k] += y * phid[a][k] * phid[b][k];
      sum_b[k] += (1 - y) * phid[a][k] * phid[b][k];
      if (y == 1) {
	s1 += phid[a][k] * phid[b][k];
	n1++;
      } else {
	s0 += phid[a][k] * phid[b][k];
	n0++;
      }
    }
    tst("k=%d, gsum=%.5f", k, gsum);
    gdt[k] = _alpha[k] + gamma_scale * gsum;
    ldt[k][0] = etad[k][0] + lambda_scale * sum_a[k];
    ldt[k][1] = etad[k][1] + lambda_scale * sum_b[k];
  }
  ldt[_k][0] = etad[_k][0] + lambda_scale * ((double)n1 / _k - s1);
  ldt[_k][1] = etad[_k][1] + lambda_scale * ((double)n0 / _k - s0);
}

void
SBM::get_inf_sample(vector<Edge> &c,
		    vector<uint32_t> &nodes)
{
  const vector<uint32_t> *edges = _network.get_edges(_start_node);
  const vector<uint32_t> *zeros = _network.sparse_zeros(_start_node);
  vector<uint32_t> dummy;

  const vector<uint32_t> &a = edges ? *edges : dummy;
  const vector<uint32_t> &b = zeros ? *zeros : dummy;

  info("edges:%d, zeros:%d\n", a.size(), b.size());

  nodes.push_back(_start_node);
  // no duplicates among a and b
  for (uint32_t i = 0; i < a.size(); ++i) {
    Edge e(_start_node, a[i]);
    Network::order_edge(_env, e);
    if (!edge_ok(e))
      continue;
    nodes.push_back(a[i]);
    c.push_back(e);
  }
  for (uint32_t i = 0; i < b.size(); ++i) {
    Edge e(_start_node, b[i]);
    Network::order_edge(_env, e);
    if (!edge_ok(e))
      continue;    
    nodes.push_back(b[i]);
    c.push_back(e);
  }
  tst("sample = %s", edgelist_s(c).c_str());
  // todo: check nodes has unique pairs
}

void
SBM::get_noninf_sample(vector<Edge> &sample, 
		       vector<uint32_t> &nodes)
{
  const vector<uint32_t> *zeros = _network.sparse_zeros(_start_node);
  NodeMap inf_map;
  for (uint32_t i = 0; i < zeros->size(); ++i)
    inf_map[(*zeros)[i]] = true;
  
  uint32_t limit = _k;
  while (sample.size() < limit) {
    uint32_t q = gsl_rng_uniform_int(_r, _n);
    if (q == _start_node)
      continue;
    
    yval_t y = get_y(_start_node, q);
    Edge e(_start_node, q);
    Network::order_edge(_env, e);
    if (y == 0 && edge_ok(e)) {
      NodeMap::iterator itr = inf_map.find(q);
      if (!itr->second)
	sample.push_back(e);
    }
  };
  info("noninf sample size = %ld\n", sample.size());
}

void
SBM::opt_process(vector<uint32_t> &nodes)
{
  _start_node = gsl_rng_uniform_int(_r, _n);
  _phit.zero(_start_node);

  tst("start node = %d\n", _start_node);

  double phi_scale = (double)_n / 2;
  double gamma_scale = 0.5;  // N / 2(N-1) * (1-epsilon)  ~ 1/2
  double lambda_scale = (double)_n / 2;
  
  // full inference only around the neighborhood _start_node
  vector<Edge> sample;
  get_inf_sample(sample, nodes);

  tst("got inf sample of size %d", sample.size());
  tst("sample = %s\n", edgelist_s(sample).c_str());

  if (sample.size() > 0) {
    tst("scale = %.5f\n", phi_scale);

    update_phit(_start_node, sample, gamma_scale, phi_scale);
    //update_phi2(_start_node);
    for (uint32_t i = 0; i < sample.size(); ++i) {
      uint32_t a = sample[i].first;
      uint32_t b = sample[i].second;
      uint32_t dst = (a == _start_node)? b : a;
      _phit.zero(dst);
      
      // todo: check uniq pairs in the set
      vector<Edge> v;
      Edge e(_start_node, dst);
      v.push_back(e);
      update_phit(dst, v, gamma_scale, phi_scale); 
      //update_phi2(dst);
    }
    update_phi(nodes);
    update_gammat_lambdat(sample, gamma_scale, lambda_scale);
    update_gamma_lambda();
  }
}


void
SBM::opt_process_noninf(vector<uint32_t> &nodes)
{
  _start_node = gsl_rng_uniform_int(_r, _n);
  double phi_scale = ((double)_n * (double)_n) / (2 * _inf_epsilon * _k);
  double gamma_scale = (double)_n / (2 * _inf_epsilon * _k);
  double lambda_scale = phi_scale;
  
  _phit.zero(_start_node);
  vector<Edge> sample;
  get_noninf_sample(sample, nodes);

  info("got non-inf sample of size %d", sample.size());

  if (sample.size() > 0) {
    update_phit(_start_node, sample, gamma_scale, phi_scale);
    //update_phi2(_start_node);
    for (uint32_t i = 0; i < sample.size(); ++i) {
      uint32_t a = sample[i].first;
      uint32_t b = sample[i].second;
      uint32_t dst = (a == _start_node)? b : a;
      _phit.zero(dst);
      
      // todo: check uniq pairs in the set
      vector<Edge> v;
      Edge e(_start_node, dst);
      v.push_back(e);
      update_phit(dst, v, gamma_scale, phi_scale); 
      //update_phi2(dst);
    }
    update_phi(nodes);
    update_gammat_lambdat(sample, gamma_scale, lambda_scale);
    update_gamma_lambda();
  }
}


double
SBM::heldout_likelihood()
{
  if (_env.accuracy)
    return .0;

  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _heldout_map.begin();
       i != _heldout_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t y = yd[p][q] & 0x01;
    bool seen = yd[p][q] & 0x80;
#else
    yval_t y = _network.y(p,q);
    bool seen = false; // TODO: fix heldout for sparse network
#endif

    assert (!seen);
    double u = edge_likelihood2(p,q,y);
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

  // Use hol @ network sparsity as stopping criteria
  double a = nshol;
  
  bool stop = false;
  int why = -1;
  if (_iter > 5000) {
    if (a > _prev_h && _prev_h != 0 && fabs((a - _prev_h) / _prev_h) < 0.00001) {
      stop = true;
      why = 0;
    } else if (a < _prev_h)
      _nh++;
    else if (a > _prev_h)
      _nh = 0;

    if (a > _max_h)
      _max_h = a;
    
    if (_nh > 3) {
      why = 1;
      stop = true;
    }
  }
  _prev_h = a;

  if (stop) {
    // precision goes here
    uint32_t v10 = 0, v100 = 0, v1000 = 0;
    compute_precision(v10, v100, v1000);
    
    double v = validation_likelihood();

    double t = 0;
#ifdef TRAINING_SAMPLE
    t = training_likelihood();
#endif

    FILE *f = fopen(Env::file_str("/max.txt").c_str(), "w");
    fprintf(f, "%d\t%d\t%.5f\t%.5f\t%.5f\t%.5f\t%d\t%d\t%d\t%d\n", 
	    _iter, duration(), 
	    a, t, v, _max_h,
	    v10, v100, v1000,
	    why);
    fclose(f);

    if (_env.use_validation_stop)
      exit(0);
  }
  return s / k;
}

double
SBM::validation_likelihood()
{
  if (_env.accuracy)
    return .0;

  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _validation_map.begin();
       i != _validation_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t y = yd[p][q] & 0x01;
    bool seen = yd[p][q] & 0x80;
#else
    yval_t y = _network.y(p,q);
    bool seen = false; // TODO: fix heldout for sparse network
#endif

    assert (!seen);
    double u = edge_likelihood2(p,q,y);
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
  fprintf(_vf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones, 
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones), 
	  nshol);
  fflush(_vf);
  
  return s / k;
}

void
SBM::compute_precision(uint32_t &c10, uint32_t &c100, 
		       uint32_t &c1000)
{
  char buf[128];
  sprintf(buf, "/hitcurve_%d.txt", _hitcurve_id);
  _hcf = fopen(Env::file_str(buf).c_str(), "w");
  if (!_hcf)  {
    lerr("cannot open hitcurve file:%s\n",  strerror(errno));
    exit(-1);
  }
  _hitcurve_id++;

  uint32_t k = 0;
  map<Edge, double> hol;
  D1Array<EdgeV> v(_precision_map.size());
  // compute likelihood for all pairs in precision set
  for (SampleMap::const_iterator i = _precision_map.begin();
       i != _precision_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

    double u1 = edge_likelihood2(p,q,1);
    double p1 = exp(u1);
    
    hol[e] = p1;
    v[k++] = EdgeV(e,p1);
  }
  
  // rank the pairs in desc order of likelihood
  v.sort_by_value();
  
  c10 = 0, c100 = 0, c1000 = 0;
  uint32_t step_size = 1000;
  uint32_t hits = 0;
  for (uint32_t i = 0; i < v.size(); ++i) {
    const Edge &e = v[i].first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);
    
#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t true_y = yd[p][q] & 0x01;
#else
    yval_t true_y = _network.y(p,q);
#endif

    if (true_y == 1) { // hit
      hits++;
      if (i < 10)
	c10++;
      else if (i < 100)
	c100++;
      else if (i < 1000)
	c1000++;
    }
    if ( i == 0 || (i+1) % step_size == 0) {
      fprintf(_hcf, "%d\t%d\n", i+1, hits);
      fflush(_hcf);
    }
  }
  c100 += c10;
  c1000 += c100;
  fprintf(_pf, "%d\t%d\t%d\t%d\t%d\n", _iter, duration(), c10, c100, c1000);
  fflush(_pf);
  fclose(_hcf);
}

uint32_t
SBM::most_likely_group(uint32_t p)
{
  const double ** const phid = _phi.const_data();
  
  double max_k = .0, max_p = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    if (phid[p][k] > max_p) {
      max_p = phid[p][k];
      max_k = k;
    }
  }
  return max_k;
}

void
SBM::gml()
{
  FILE *f = fopen(Env::file_str("/network.gml").c_str(), "w");
  fprintf(f, "graph\n[\n\tdirected 0\n");
  for (uint32_t i = 0; i < _n; ++i) {
    uint32_t g = most_likely_group(i);

    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);

    StrMapInv::const_iterator strt;
    if (_env.strid) {
      const StrMapInv &sm = _network.id2str();
      strt = sm.find(i);
    }
    
    assert (idt != m.end());
    fprintf(f, "\tnode\n\t[\n");
    fprintf(f, "\t\tid %d\n", i);
    fprintf(f, "\t\textid %d\n", idt->second);
    if (_env.strid)
      fprintf(f, "\t\tdescid %s\n", strt->second.c_str());
    fprintf(f, "\t\tgroup %d\n", g);
    fprintf(f, "\t\tdegree %d\n", _network.deg(i));
    fprintf(f, "\t]\n");
  }
  Array pp(_k);
  Array qp(_k);
  Array beta(_K);
  estimate_beta(beta);
  
  for (uint32_t i = 0; i < _n; ++i) {
    for (uint32_t j = 0; j < _n; ++j) {
      if (i < j && _network.y(i,j) != 0) {
	_phi.slice(0, i, pp);
	_phi.slice(0, j, qp);
	uint32_t max_k = 65536;
	double max = inner_prod_max2(pp, qp, beta, max_k);
	if (max < 0.9)
	  max_k = 65536;
	fprintf(f, "\tedge\n\t[\n");
	fprintf(f, "\t\tsource %d\n", i);
	fprintf(f, "\t\ttarget %d\n", j);
	fprintf(f, "\t\tcolor %d\n", max_k);
	//fprintf(f, "\t\tfilter %d\n", (i == 5330  || j == 5330) ? 1 : 0);
	fprintf(f, "\t]\n");
      }
    }
  }
  fprintf(f, "\t]\n");  
  fclose(f);
}

#ifdef GLOBALPHIS
double
SBM::approx_log_likelihood()
{
  const double ** const etad = _eta.const_data();
  const double * const alphad = _alpha.const_data();
  const double ** const elogbetad = _Elogbeta.const_data();
  const double * const elogpid = _Elogpi.const_data();
  const double ** const ld = _lambda.const_data();
  const double * const gd = _gamma.const_data();
  const double **phid = _phi.const_data();

  double s = .0, v = .0;
  for (uint32_t k = 0; k < _K; ++k) {
    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += gsl_sf_lngamma(etad[k][t]);
    s += gsl_sf_lngamma(_eta.sum(k)) - v;

    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += (etad[k][t] - 1) * elogbetad[k][t];
    s += v;

    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += gsl_sf_lngamma(ld[k][t]);
    s -= gsl_sf_lngamma(_lambda.sum(k)) - v;

    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += (ld[k][t] - 1) * elogbetad[k][t];
    s -= v;
  }

  for (uint32_t p = 0; p < _n; ++p)
    for (uint32_t q = 0; q < _n; ++q) {
      if (p >= q)
	continue;

      Edge e(p,q);
      Network::order_edge(_env, e);
      if (!edge_ok(e))
	continue;

      yval_t y = get_y(p,q);
      Array f(_k);
      
      double sum = .0;
      for (uint32_t k = 0; k < _k; ++k) {
	sum += phid[p][k] * phid[q][k];
	s += phid[p][k] * phid[q][k] * (y * elogbetad[k][0] + (1 - y) * elogbetad[k][1]);
      }
      s += (1 - sum) * (y * elogbetad[_k][0] + (1 - y) * elogbetad[_k][1]);
    }

  for (uint32_t n = 0; n < _n; ++n)
    for (uint32_t k = 0; k < _k; ++k) {
      s += phid[n][k] * elogpid[k];
      s -= phid[n][k] * log(phid[n][k]);
    }

  v = .0;
  for (uint32_t k = 0; k < _k; ++k)
    v += gsl_sf_lngamma(alphad[k]);
  s += gsl_sf_lngamma(_alpha.sum()) - v;

  for (uint32_t k = 0; k < _k; ++k)
    s += (alphad[k] - 1) * elogpid[k];
  
  v = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    double qq = gd[k];
    if (gd[k] < 1e-30)
      qq = 1e-30;
    v += gsl_sf_lngamma(qq);
  }
  s -= gsl_sf_lngamma(_gamma.sum()) - v;
  
  v = .0;
  for (uint32_t k = 0; k < _k; ++k)
    v += (gd[k] - 1) * elogpid[k];
  s -= v;

  info("approx. log likelihood = %f\n", s);
  fprintf(_lf, "%d\t%d\t%.5f\n", _iter, duration(), s);
  fflush(_lf);

  double thresh = 0.00001;
  double w = s;
  if (_env.accuracy && _iter > 1000) {
    if (w > _prev_w && _prev_w != 0 && fabs((w - _prev_w) / _prev_w) < thresh) {
      FILE *f = fopen(Env::file_str("/done.txt").c_str(), "w");
      fprintf(f, "%d\t%d\t%.5f\n", _iter, duration(), w); 
      fclose(f);
      if (_env.nmi) {
	char cmd[1024];
	sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s", 
		Env::file_str("/ground_truth.txt").c_str(),
		Env::file_str("/communities.txt").c_str(), 
		Env::file_str("/done.txt").c_str());
	system(cmd);
      }
      //exit(0);
    }
  }
  _prev_w = w;
  return s;
}
#endif

