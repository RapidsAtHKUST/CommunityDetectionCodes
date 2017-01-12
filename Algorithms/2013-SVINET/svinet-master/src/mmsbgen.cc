#include "mmsbgen.hh"

MMSBGen::MMSBGen(Env &env, Network &network, bool ppc)
  : _env(env), _network(network), _ppc(ppc),
    _n(env.n), _k(env.k), _t(env.t),
    _gen_y(1,1), //XXX
    _gen_ones(0),
    _alpha(_k), _eta(_k,_t),
    _pi(_n,_k),
    _beta(_k), _gamma(_n,_k), _lambda(_k,_t),
    _curr_id(0),
    _Elogpi(_n,_k), _Elogbeta(_k,_t), _Elogf(_k),
    _pcomp(env, _curr_id, _n, _k, _t, 0, 0, 0,
	   _Elogpi, _Elogbeta, _Elogf, true),
    _legal(0), _illegal(0),
    _lc_ppc_logpe(_k, _env.ppc_ndraws),
    _lc_obs_logpe(_k, _env.ppc_ndraws),
    _zscores_pe(_k),
    _lc_ppc_size(_k, _env.ppc_ndraws),
    _lc_obs_size(_k, _env.ppc_ndraws),
    _zscores_size(_k),
    _communities(_k),
    _lc_obs_avgdeg(_k, _env.ppc_ndraws),
    _lc_obs_maxdeg(_k, _env.ppc_ndraws),
    _bridgeness(_n)
{
  assert (!(ppc && _env.disjoint));
  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);
  if (_ppc) {
    unlink("ppc");
    setup_dir("ppc");
  }

  if (env.undirected)
    _total_pairs = _n * (_n - 1) / 2;
  else
    _total_pairs = _n * (_n - 1);
}

void
MMSBGen::gen(double alpha)
{
  double *alphad = _alpha.data();
  for (uint32_t i = 0; i < _k; ++i)
    alphad[i] = alpha;

  double **etad = _eta.data();
  for (uint32_t i = 0; i < _k; ++i) {
    etad[i][0] = _env.eta0_gen;
    etad[i][1] = _env.eta1_gen;
  }

  // draw pi from alpha
  double **pid = _pi.data();
  for (uint32_t i = 0; i < _n; ++i)
    gsl_ran_dirichlet(_r, _k, alphad, pid[i]);

  // draw beta from eta
  for (uint32_t k = 0; k < _k; ++k)
    _beta[k] = gsl_ran_beta(_r, etad[k][0], etad[k][1]);

  debug("alpha = %s\n", _alpha.s().c_str());
  debug("pi = %s\n", _pi.s().c_str());
  debug("beta = %s\n", _beta.s().c_str());

  draw_and_save(0);
  save_model();
}

int
MMSBGen::load_model()
{
  fprintf(stderr, "+ Loading model\n");
  double **gd = _gamma.data();
  FILE *gammaf = fopen("gamma.txt", "r");
  if (!gammaf)
    return -1;
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
    memset(line, 0, sz);
  }
  assert (n == _n);
  fclose(gammaf);
  memset(line, 0, sz);

  double **ld = _lambda.data();
  FILE *lambdaf = fopen("lambda.txt", "r");
  if (!lambdaf)
    return -1;
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
	  fprintf(stderr, "error parsing gamma file\n");
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
    debug("read %d lines\n", n);
    memset(line, 0, sz);
  }
  assert (k == _k);
  fclose(lambdaf);
  free(line);

  return 0;
}

void
MMSBGen::ppc()
{
  _onesf = fopen("ppc/ppc-ones.txt", "w");
  assert(_onesf);
  assert(_ppc);
  assert (load_model() >= 0);
  save_model();

  obs_data();

  // replications
  for (uint32_t i = 0; i < _env.ppc_ndraws; ++i) {
    fprintf(stdout, "ppc draw: %d / %d\n", i, _env.ppc_ndraws);
    create_id_dir(i);
    //estimate_all();
    draw_all();
    draw_and_save(i);
    compute_and_log_groups();
    ppc_ones();
    ppc_avg_deg();
    ppc_local_clustering_coeff();
    ppc_per_community();
    reset();
  }
  get_lc_zscores();
  fclose(_onesf);
}

void
MMSBGen::get_lc_stats()
{
  assert (load_model() >= 0);
  //save_model();
  estimate_all();
  lc_current_draw2();
  printf("+ Computing link communities\n");
  process_link_communities2();
  printf("+ Computing bridgeness and influence\n");
  bridgeness();
  //compute_modularity();
  community_degrees();
}

void
MMSBGen::compute_modularity()
{
  FILE *f = fopen("obs_modularity.txt", "w");
  const Array &deg = _network.deg();

#ifndef SPARSE_NETWORK  
  const yval_t ** const yd = _network.y().const_data();
#endif

  double q = .0;
  double qmax = .0;
  uint32_t ones = _network.ones();
  double assor = .0;
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _n; ++j) {
#ifndef SPARSE_NETWORK
      yval_t y = yd[i][j] & 0x01;
#else
      yval_t y = _network.y(i,j);
#endif
      double v = ((double)deg[i] * deg[j]) / (2 * ones);
      double w = _pi.dot(i, j);
      q += (y - v) * w;
      qmax += v * w;
    }
  
  qmax = 2 * ones - qmax;
  assor = q / qmax;
  q = q / (2*_network.ones());
  fprintf(f, "%.5f\t%.5f\n", q, assor);
  fclose(f);
}

void
MMSBGen::bridgeness()
{
  Array b(_n);
  const double ** const pid = _pi.const_data();
  FILE *f = fopen(Env::file_str("/node_bridgeness.txt").c_str(), "w");
  for (uint32_t i = 0; i < _n; ++i) {
    double v = .0;
    for (uint32_t k = 0; k < _k; ++k)
      v += (pid[i][k] - ((double)1.0 / _k))*(pid[i][k] - ((double)1.0 / _k));
    v = (1 - sqrt(v * ((double)_k) / (_k - 1))) * _network.deg(i);
    _bridgeness[i] = v;

    uint32_t g = most_likely_group(i);

    double max = .0, avg = .0;
    uint32_t node = 0;
    _communities[g].deg_stats(avg, max, node);

    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);
    if (idt != m.end()) {
      uint32_t id = (*idt).second;
      fprintf(f, "%d\t%d\t%.5f\t%d\t%d\t%d\t%.5f\t%.5f\t%d\n", 
	      i, id, v, _communities[g].deg(i), _network.deg(i), 
	      _communities[g].nodes(), avg, max, g);
    }
  }
  fclose(f);
}

void
MMSBGen::community_degrees()
{
  FILE *f = fopen(Env::file_str("/node_influence.txt").c_str(), "w");
  FILE *g = fopen(Env::file_str("/number_of_memberships.txt").c_str(), "w");
  for (uint32_t i = 0; i < _n; ++i) {
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);
    if (idt != m.end()) {
      uint32_t id = idt->second;
      fprintf(f, "%d\t%d\t", i, id);
      uint32_t s = 0;
      for (uint32_t k = 0; k < _k; ++k) {
	uint32_t v = _communities[k].deg(i);
	if (v > 0)
	  s++;
	fprintf(f, "%d\t", _communities[k].deg(i));
      }
      fprintf(f, "\n");
      fprintf(g, "%d\t%d\t%d\n", i, id, s);
    }
  }
  fclose(f);
  fclose(g);
}

void
MMSBGen::get_lc_zscores()
{
  FILE *f = fopen("ppc/lc_zscores_pe.txt", "w");
  FILE *g = fopen("ppc/lc_zscores_size.txt", "w");
  FILE *h = fopen("ppc/lc_obs_avgdeg.txt", "w");
  FILE *o = fopen("ppc/lc_obs_maxdeg.txt", "w");

  for (uint32_t k = 0; k < _k; ++k) {
    Array ppc_logpe(_env.ppc_ndraws);
    _lc_ppc_logpe.slice(0, k, ppc_logpe);
    Array obs_logpe(_env.ppc_ndraws);
    _lc_obs_logpe.slice(0, k, obs_logpe);
    _zscores_pe[k] =  ppc_logpe.zscore(obs_logpe.mean());
    fprintf(f, "%d\t%.5f\n", k, _zscores_pe[k]);

    Array ppc_size(_env.ppc_ndraws);
    _lc_ppc_size.slice(0, k, ppc_size);
    Array obs_size(_env.ppc_ndraws);
    _lc_obs_size.slice(0, k, obs_size);
    _zscores_size[k] =  ppc_size.zscore(obs_size.mean());
    fprintf(g, "%d\t%.5f\n", k, _zscores_size[k]);

    Array avgdeg(_env.ppc_ndraws);
    _lc_obs_avgdeg.slice(0, k, avgdeg);
    double m = avgdeg.mean();
    fprintf(h, "%d\t%.5f\n", k, m);

    Array maxdeg(_env.ppc_ndraws);
    _lc_obs_maxdeg.slice(0, k, maxdeg);
    m = maxdeg.mean();
    fprintf(o, "%d\t%.5f\n", k, m);
  }
  fclose(o);
  fclose(h);
  fclose(g);
  fclose(f);
}

void
MMSBGen::obs_data()
{
  obs_ones();
  obs_avg_deg();
}

void
MMSBGen::ppc_per_community()
{
  lc_current_draw();
  process_link_communities();
}

void
MMSBGen::ppc_avg_deg() const
{
  FILE *avgdegf = fopen("ppc/ppc-avg-deg.txt", "a");
  FILE *maxdegf = fopen("ppc/ppc-max-deg.txt", "a");
  const yval_t **gen_yd = _gen_y.const_data();
  assert(avgdegf);
  assert(maxdegf);
  Array deg(_gen_y.m());
  for (uint32_t i = 0; i < _gen_y.m(); ++i)
    for (uint32_t j = 0; j < _gen_y.n(); ++j)
      if (i < j && gen_yd[i][j]) {
	deg[i]++;
	deg[j]++;
      }
  uint32_t max_deg = 0;
  uint32_t s = 0;
  for (uint32_t i = 0; i < _gen_y.m(); ++i)   {
    s += deg[i];
    if (deg[i] > max_deg)
      max_deg = deg[i];
  }
  fprintf(avgdegf, "%.5f\n", (double)s / _gen_y.m());
  fprintf(maxdegf, "%d\n", max_deg);
  fclose(avgdegf);
  fclose(maxdegf);
}

void
MMSBGen::ppc_local_clustering_coeff() const
{
  char cmd[1024];
  sprintf(cmd, "cd %s; /usr/local/bin/nstat -i:network.dat -d:F -p:dhCws -o:ppc > /dev/null 2>&1",
	  id_dir().c_str());
  if (system(cmd) < 0) {
    fprintf(stderr, "cmd %s returned with error %s\n", cmd, strerror(errno));
    exit(-1);
  }
  process("ppc/ppc-ccf.txt", "/ccf.ppc.tab", "%*[^:]:%s%*[^\n]");
  process("ppc/ppc-hop.txt", "/hop.ppc.tab", "%*[^:]:%s%*[^\n]");
  process("ppc/ppc-scc.txt", "/scc.ppc.tab", "%*[^s]s%*[^s]s%s%*[^\n]");
}

void
MMSBGen::process(string outfile, string infile, string pattern) const
{
  FILE *ccf = fopen(outfile.c_str(), "a");
  string ccfdir =  id_dir() + infile;
  FILE *f = fopen(ccfdir.c_str(), "r");
  assert(f);
  char buf[1024];
  float c = .0;
  char cs[512];
  while (fgets(buf, 1024, f)) {
    //printf("got %s\n", buf);
    if (sscanf(buf, pattern.c_str(), cs) > 0) {
      c = atof(cs);
      break;
    }
  }
  fprintf(ccf, "%.5f\n", c);
  fclose(f);
  fclose(ccf);
}

void
MMSBGen::lc_current_draw()
{
  MMSBInfer::set_dir_exp(_gamma, _Elogpi);
  MMSBInfer::set_dir_exp(_lambda, _Elogbeta);
  _Elogf.zero();
  _ppc_link_communities.clear();
  _obs_link_communities.clear();

  lc_current_draw_helper(true);
  lc_current_draw_helper(false);
}

void
MMSBGen::lc_current_draw2()
{
  MMSBInfer::set_dir_exp(_gamma, _Elogpi);
  MMSBInfer::set_dir_exp(_lambda, _Elogbeta);
  _Elogf.zero();
  _obs_link_communities.clear();
  lc_current_draw_helper(true);
}

void
MMSBGen::lc_current_draw_helper(bool obs)
{
  Array pp(_k);
  Array qp(_k);
  //yval_t **gen_yd = _gen_y.data();
  uint32_t unlikely = 0, bad = 0;
  for (uint32_t i = 0; i < _n; ++i) {
    const vector<uint32_t> *edges = _network.get_edges(i);
    for (uint32_t m = 0; m < edges->size(); ++m) {
      uint32_t j = (*edges)[m];
      if (i < j) {
	Array beta(_k);
	estimate_beta(beta);
	_pi.slice(0, i, pp);
	_pi.slice(0, j, qp);
	uint32_t max_k = 65536;
	double max = inner_prod_max(pp, qp, beta, max_k);
	
	assert (max_k < _k);
	if (max < 0.5) {
	  //printf("max = %.2f\n", max);
	  Array pg(_k);
	  Array qg(_k);
	  _gamma.slice(0, i, pg);
	  _gamma.slice(0, j, qg);
	  unlikely++;
	} else {
	  EdgeList &v =
	    obs ? _obs_link_communities[max_k] : _ppc_link_communities[max_k];
	  Edge e(i,j);
	  Network::order_edge(_env, e);
	  v.push_back(e);
	}
      }
    }
  }
  if (obs)
    info("obs unlikely:%d, bad:%d\n", unlikely, bad);
  else
    info("ppc unlikely:%d, bad:%d\n", unlikely, bad);
}

void
MMSBGen::process_link_communities2()
{
  std::map<uint32_t, double> logl_by_degree;
  for(std::map<uint32_t, EdgeList>::const_iterator i = _obs_link_communities.begin();
      i != _obs_link_communities.end(); ++i) {

    uint32_t color = i->first;
    const EdgeList &vobs = i->second;

    for (uint32_t j = 0; j < vobs.size(); ++j) {
      const Edge &e = vobs[j];
      _communities[color].add_edge(_env, e);
    }
  }
  // most influential node in community k
  FILE *f = fopen(Env::file_str("/community_stats.txt").c_str(), "w");
  for (uint32_t k =0; k < _k; ++k) {
    double avg = 0 , max = 0;
    uint32_t node = 0;
    _communities[k].deg_stats(avg, max, node);
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(node);
    assert (idt != m.end());
    fprintf(f, "%d\t%.5f\t%.5f\t%d\t%d\n", k, avg, max, node, (*idt).second);
  }
  fclose(f);
}


void
MMSBGen::process_link_communities()
{
  std::map<uint32_t, double> logl_by_degree;
  for(std::map<uint32_t, EdgeList>::const_iterator i = _obs_link_communities.begin();
      i != _obs_link_communities.end(); ++i) {
    
    uint32_t color = i->first;
    const EdgeList &vobs = i->second;
    const EdgeList &vppc = _ppc_link_communities[color];

#ifdef DEBUG_GEN
    char buf1[512];
    sprintf(buf1, "ppc/obs_deg_logl_%d.txt", color);
    char buf2[512];
    sprintf(buf2, "ppc/ppc_deg_logl_%d.txt", color);
    FILE *g = fopen(buf1, "a");
    FILE *h = fopen(buf2, "a");
#endif

    _illegal = 0; 
    _legal = 0;

    double sobs = 0;
    uint32_t sobs_k = 0;
    for (uint32_t j = 0; j < vobs.size(); ++j) {
      const Edge &e = vobs[j];
      _communities[color].add_edge(_env, e);

      if (_network.relevant_node_ppc(e.first) && _network.relevant_node_ppc(e.second)) {
	double v = edge_likelihood(e.first, e.second, color);
#ifdef DEBUG_GEN
	fprintf(g, "%d\t%d\t%d\t%d\t%d\t%.3f\n", 
		e.first, e.second, _network.deg(e.first), 
		_network.deg(e.second), std::max(_network.deg(e.first), _network.deg(e.second)), v);
#endif
	sobs += v;
	sobs_k++;
      }
    }
#ifdef DEBUG_GEN
    fclose(g);
#endif

    printf("OBS: illegal:%d, legal:%d, ratio:%.3f\n", 
	   _illegal, _legal, (double)_illegal / _legal);
    _illegal = 0;
    _legal = 0;


    double sppc = 0;
    uint32_t sppc_k = 0;
    for (uint32_t j = 0; j < vppc.size(); ++j) {
      const Edge &e = vppc[j];
      double v = edge_likelihood(e.first, e.second, color);
      sppc += v;
      sppc_k++;
#ifdef DEBUG_GEN
      fprintf(h, "%d\t%d\t%d\t%d\t%d\t%.3f\n", 
	      e.first, e.second, _network.deg(e.first), 
	      _network.deg(e.second), std::max(_network.deg(e.first), _network.deg(e.second)), v);
#endif
    }
#ifdef DEBUG_GEN
    fclose(h);
#endif

    

    double obs_logpe = sobs / sobs_k;
    double ppc_logpe = sppc / sppc_k;

    double **lc_ppc_logped = _lc_ppc_logpe.data();
    double **lc_obs_logped = _lc_obs_logpe.data();

    lc_ppc_logped[color][_curr_id] = ppc_logpe;
    lc_obs_logped[color][_curr_id] = obs_logpe;

    double **lc_obs_avgdeg = _lc_obs_avgdeg.data();
    double **lc_obs_maxdeg = _lc_obs_maxdeg.data();

    double avg = .0, max = .0;
    uint32_t n = 65536;
    _communities[color].deg_stats(avg, max, n);
    assert (n != 65536);

    lc_obs_avgdeg[color][_curr_id] = avg;
    lc_obs_maxdeg[color][_curr_id] = max;

    printf("PPC: illegal:%d, legal:%d, ratio:%.3f\n", 
	   _illegal, _legal, (double)_illegal / _legal);

    char buf[512];
    sprintf(buf, "ppc/lc_%d.txt", color);
    
    FILE *f = fopen(buf, "a");
    //fprintf(f, "%d\t%ld\t%ld\t%.5f\t%.5f\n", color, 
    //vobs.size(), vppc.size(), sobs / vobs.size(), sppc / vppc.size());

    fprintf(f, "%d\t%d\t%d\t%.5f\t%.5f\n", color, 
	    sobs_k, sppc_k, obs_logpe, ppc_logpe);
    fclose(f);
  }
}

double
MMSBGen::edge_likelihood(uint32_t p, uint32_t q, uint32_t color) const
{
  const double **pid = _pi.const_data();
  double v = pid[p][color] * pid[q][color] * gsl_ran_bernoulli_pdf(1, _beta[color]);

#ifdef DEBUG_GEN
  if (v > .0) {
    char buf[512];
    sprintf(buf, "ppc/el_%d.txt", color);
    FILE *f = fopen(buf, "a");
    fprintf(f, "pid[%d]:%f\tpid[%d]:%f\tbeta[%d]:%f -> %f\n",
	    p,pid[p][color],q,pid[q][color],color,_beta[color], log(v));
    fclose(f);
  }
#endif

  if (v > .0) {
    _legal++;
    return log(v);
  } else {
    printf("Warning: edge_likelihood .0 found!\n");
    _illegal++;
    return .0;
  }
}

void
MMSBGen::ppc_ones() const
{
  FILE *f = fopen("ppc/ppc-ones.txt", "a");
  fprintf(f, "%.5f\n", (double)_gen_ones / _total_pairs);
  fclose(f);
}

void
MMSBGen::obs_ones() const
{
  FILE *f = fopen("obs-ones.txt", "a");
  fprintf(f, "%.5f\n", (double)_network.ones() / _total_pairs);
  fclose(f);
}

void
MMSBGen::obs_avg_deg() const
{
  FILE *avgf = fopen("obs-avg-deg.txt", "w");
  FILE *maxf = fopen("obs-max-deg.txt", "w");
  uint32_t max = 0;
  double avg = .0;
  _network.deg_stats(max, avg);
  fprintf(avgf, "%.5f\n", avg);
  fprintf(maxf, "%d\n", max);
  fclose(avgf);
  fclose(maxf);
}

void
MMSBGen::draw_and_save(uint32_t id)
{
  fprintf(stdout, "Generating network of size %d x %d\n", _n, _n);
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _n; ++j) {
      if (i < j)
	draw_membership_indicators(i, j);
#ifndef DEBUG_GEN
      if (j == 0 && i % 10 == 0) {
	printf("\r%.5f %%", ((double)(i*_n*100)) / (_n * _n));
	fflush(stdout);
      }
#endif
    }

  yval_t **gen_yd = _gen_y.data();
  fprintf(stdout, "\nWriting network (ones:%d)\n", _gen_ones);
  char buf[128];
  if (_ppc)
    sprintf(buf,"%s", (id_dir() + string("network.dat")).c_str());
  else
    sprintf(buf,"%s", Env::file_str("/network_gen.dat").c_str());

  FILE *f = fopen(buf, "w");
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _n; ++j) {
      if (i < j) {
	yval_t y = gen_yd[i][j];
	if (y)
	  fprintf(f, "%d\t%d\n", i, j);
      }
    }
  fclose(f);
  fprintf(stdout, "done\n");
}

void
MMSBGen::estimate_all()
{
  const double ** const gd = _gamma.const_data();
  double **pid = _pi.data();
  for (uint32_t n = 0; n < _n; ++n) {
    double s = .0;
    for (uint32_t k = 0; k < _k; ++k)
      s += gd[n][k];
    assert(s);

    for (uint32_t k = 0; k < _k; ++k)
      pid[n][k] = gd[n][k] / s;

#if 0
    double sum = .0;
    for (uint32_t k = 0; k < _k; ++k) {
      pid[n][k] = gd[n][k] / s;
      if (pid[n][k] < 1e-4)
	pid[n][k] = .0;
      sum += pid[n][k];
    }
    for (uint32_t k = 0; k < _k; ++k)
      pid[n][k] = pid[n][k] / sum;
#endif
  }

  for (uint32_t k = 0; k < _k; ++k)
    _beta[k] = estimate_bernoulli_rate(k);
}

void
MMSBGen::draw_all()
{
  const double ** const gd = _gamma.const_data();
  double **pid = _pi.data();
  for (uint32_t n = 0; n < _n; ++n)
    gsl_ran_dirichlet(_r, _k, gd[n], pid[n]);

  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _k; ++k)
    _beta[k] = gsl_ran_beta(_r, ld[k][0], ld[k][1]);
  printf("drawn beta = %s\n", _beta.s().c_str());
}

void
MMSBGen::reset()
{
  _gen_y.zero(); // XXX expensive
  _pi.zero();
  _beta.zero();
  _gen_ones = 0;
  _illegal = 0;
  _legal = 0;
  for (uint32_t i = 0; i < _communities.n(); ++i)
    _communities[i].reset();
}


void // refactor
MMSBGen::estimate_pi(uint32_t p, Array &pi_p) const
{
  const double ** const gd = _gamma.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[p][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    pi_p[k] = gd[p][k] / s;
}

double // refactor
MMSBGen::estimate_bernoulli_rate(uint32_t k) const
{
  const double ** const ld = _lambda.data();
  double s = .0;
  for (uint32_t t = 0; t < _t; ++t)
    s += ld[k][t];
  assert(s);
  debug("total (%d) = %f", k, s);
  debug("lambda  = %s", _lambda.s().c_str());
  return ld[k][0] / s;
}

void
MMSBGen::save_model()
{
  fprintf(stderr, "+ saving gamma\n");
  string gfname = _ppc ? "gamma-ppc.txt" : Env::file_str("/pi-gen.txt");
  FILE *gammaf = fopen(gfname.c_str(), "w");
  const double ** const gd = _ppc ? _gamma.const_data() : _pi.const_data();
  for (uint32_t i = 0; i < _n; ++i) {
    bool skip = false;
    if (_ppc) {
      const IDMap &m = _network.seq2id();
      IDMap::const_iterator idt = m.find(i);
      if (idt != m.end())  {
	fprintf(gammaf,"%d\t", i);
	fprintf(gammaf,"%d\t", (*idt).second);
      } else {
	skip = true;
	printf("Warning: seq %d not found in seq2id map\n", i);
      }
    } else
      fprintf(gammaf,"%d\t%d\t", i,i);
    if (!skip)
      for (uint32_t k = 0; k < _k; ++k) {
	if (k == _k - 1)
	  fprintf(gammaf,"%.5f\n", gd[i][k]);
	else
	  fprintf(gammaf,"%.5f\t", gd[i][k]);
      }
  }
  fclose(gammaf);

  fprintf(stderr, "+ saving lambda\n");
  string lfname = _ppc ? "lambda-ppc.txt" : Env::file_str("/beta-gen.txt");

  if (_ppc) {
    FILE *lambdaf = fopen(lfname.c_str(), "w");
    assert(lambdaf);
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
  } else {
    FILE *betaf = fopen(lfname.c_str(), "w");
    for (uint32_t k = 0; k < _k; ++k) {
      fprintf(betaf,"%d\t",k);
      fprintf(betaf,"%.5f\n", _beta[k]);
    }
    fclose(betaf);
  }
  if (!_ppc) // for ppc, this is computed for each drawn network
    compute_and_log_groups();
}

void
MMSBGen::compute_and_log_groups() const
{
  fprintf(stdout, "compute_and_log_groups start\n");
  string gfname = _ppc ? (id_dir() + "groups-ppc.txt") : Env::file_str("/groups.txt");
  string sfname = _ppc ? (id_dir() + "summary-ppc.txt") : Env::file_str("/summary.txt");
  FILE *groupsf = fopen(gfname.c_str(), "w");
  FILE *summaryf = fopen(sfname.c_str(), "a");
  assert(groupsf);
  assert(summaryf);

  char buf[32];
  ostringstream sa;
  Array groups(_n);
  const double **pid = _pi.const_data();
  for (uint32_t i = 0; i < _n; ++i) {
    sa << i << "\t";
    uint32_t id = i;
    sa << id << "\t";
    double max = .0;
    for (uint32_t j = 0; j < _k; ++j) {
      memset(buf, 0, 32);
      sprintf(buf,"%.5f", pid[i][j]);
      sa << buf << "\t";
      if (pid[i][j] > max) {
	max = pid[i][j];
	groups[i] = j;
      }
    }
    sa << groups[i] << "\n";
  }
  fprintf(groupsf, "%s", sa.str().c_str());

  D1Array<int> s(_k);
  for (uint32_t i = 0; i < _n; ++i)
    s[groups[i]]++;
  for (uint32_t i = 0; i < _k; ++i)
    fprintf(summaryf, "%d\t", s[i]);

  fprintf(summaryf,"\n");
  fflush(groupsf);
  fflush(summaryf);
  fclose(groupsf);
  fclose(summaryf);
  fprintf(stdout, "compute_and_log_groups done\n");
}

int
MMSBGen::setup_dir(string dirname)
{
  struct stat dirstat;
  if (stat(dirname.c_str(), &dirstat) != 0) {
    if (errno == ENOENT) {
      mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      if (stat(dirname.c_str(), &dirstat) != 0) {
	fprintf(stderr, "Warning: could not create dir %s: %s\n",
		dirname.c_str(), strerror(errno));
	return -1;
      }
    } else {
      fprintf(stderr, "Warning: could not stat dir %s\n", dirname.c_str());
      return -1;
    }
  }
  return 0;
}

void
MMSBGen::gml()
{
  get_lc_stats();
  FILE *f = fopen(Env::file_str("/network.gml").c_str(), "w");
  fprintf(f, "graph\n[\n\tdirected 0\n");
  for (uint32_t i = 0; i < _n; ++i) {
    uint32_t g = most_likely_group(i);

    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);
    assert (idt != m.end());
    fprintf(f, "\tnode\n\t[\n");
    fprintf(f, "\t\tid %d\n", i);
    fprintf(f, "\t\textid %d\n", idt->second);
    fprintf(f, "\t\tgroup %d\n", g);
    fprintf(f, "\t\tbridgeness %.5f\n", _bridgeness[i]);
    fprintf(f, "\t\tinfluence %d\n", _communities[g].deg(i));
    fprintf(f, "\t\tdegree %d\n", _network.deg(i));
    //fprintf(f, "\t\tfilter %d\n", (_network.y(5330, i) == 1 || i == 5330) ? 1 : 0);
    fprintf(f, "\t]\n");
  }
  Array pp(_k);
  Array qp(_k);
  for (uint32_t i = 0; i < _n; ++i) {
    for (uint32_t j = 0; j < _n; ++j) {
      if (i < j && _network.y(i,j) != 0) {
	  _pi.slice(0, i, pp);
	  _pi.slice(0, j, qp);
	  Array beta(_k);
	  estimate_beta(beta);
	  uint32_t max_k = 65536;
	  double max = inner_prod_max(pp, qp, beta, max_k);
	  if (max < 0.9) {
	    max_k = 65536;
	    continue;
	  }

	  fprintf(f, "\tedge\n\t[\n");
	  fprintf(f, "\t\tsource %d\n", i);
	  fprintf(f, "\t\ttarget %d\n", j);
	  fprintf(f, "\t\tcolor %d\n", max_k);
	  //fprintf(f, "\t\tfilter %d\n", (i == 5330  || j == 5330) ? 1 : 0);
	  fprintf(f, "\t]\n");
      }
    }
  }
  fprintf(f, "]\n");
  fclose(f);
  printf("+ Done writing GML file. Visualize the communities using a tool such as Gephi.\n");
  fflush(stdout);
}

