#include "mmsborig.hh"

MMSBOrig::MMSBOrig(Env &env, Network &network)
  :_env(env), _network(network), 
   _n(env.n), _k(env.k), _t(_env.t),
   _gen_y(_n, _n), _gen_ones(0),
   _alpha(_k), _eta(_k,_t),
   _pi(_n,_k),
   _beta(_k,_k), _gamma(_n,_k), _lambda(_k,_t),
   _curr_id(0)
{
  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);
  if (_env.seed)
    gsl_rng_set(_r, _env.seed);

  if (env.undirected)
    _total_pairs = _n * (_n - 1) / 2;
  else
    _total_pairs = _n * (_n - 1);  
}

void
MMSBOrig::gen(double alpha)
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
  double **betad = _beta.data();
  for (uint32_t i = 0; i < _k; ++i)
    for (uint32_t j = 0; j < _k; ++j)
      if (i == j)
	betad[i][j] = gsl_ran_beta(_r, etad[i][0], etad[i][1]);
      else
	betad[i][j] = _env.epsilon;

  printf("beta = %s\n", _beta.s().c_str());

  draw_and_save(0);
}

void
MMSBOrig::draw_and_save(uint32_t id)
{
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _n; ++j) {
      if (i < j)
	draw_membership_indicators(i, j);
    }
  yval_t **gen_yd = _gen_y.data();
  fprintf(stdout, "\nWriting network (ones:%d)\n", _gen_ones);
  char buf[128];
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
MMSBOrig::draw_membership_indicators(uint32_t p, uint32_t q)
{
  uArray zp(_k);
  uArray zq(_k);

  const double **pid = _pi.const_data();
  yval_t **gen_yd = _gen_y.data();
  unsigned int *zpd = zp.data();
  unsigned int *zqd = zq.data();

  Array pp(_k);
  Array qp(_k);
  
  gsl_ran_multinomial(_r, _k, 1, pid[p], zpd);
  gsl_ran_multinomial(_r, _k, 1, pid[q], zqd);

  int pk = zp.first_positive_idx();
  int qk = zq.first_positive_idx();
  printf("%d,%d\n", pk, qk);
  assert (pk >= 0 && qk >= 0);

  const double ** const betad = _beta.const_data();

  double rate = betad[pk][qk];
  printf("rate = %.2f\n", rate);
  gen_yd[p][q] = gsl_ran_bernoulli(_r, rate);
  printf("y = %d\n", gen_yd[p][q]);
  if (_env.undirected)
    gen_yd[q][p] = gen_yd[p][q];
  if (gen_yd[p][q])
    _gen_ones++;
}
