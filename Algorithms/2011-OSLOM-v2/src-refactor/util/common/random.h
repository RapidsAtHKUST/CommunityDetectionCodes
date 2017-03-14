#if !defined(RANDOM_INCLUDED)

#include <fstream>
#include <iostream>
#include <deque>
#include <map>
#include <set>
#include <algorithm>


using namespace std;

#define RANDOM_INCLUDED
#define R2_IM1 2147483563
#define R2_IM2 2147483399
#define R2_AM (1.0/R2_IM1)
#define R2_IMM1 (R2_IM1-1)
#define R2_IA1 40014
#define R2_IA2 40692
#define R2_IQ1 53668
#define R2_IQ2 52774
#define R2_IR1 12211
#define R2_IR2 3791
#define R2_NTAB 32
#define R2_NDIV (1+R2_IMM1/R2_NTAB)
#define R2_EPS 1.2e-7
#define R2_RNMX (1.0-R2_EPS)

double ran2(long *idum);

double ran4(bool t, long s);

double ran4();

void srand4(void);

void srand5(int rank);

int irand(int n);

void srand_file(void);

int configuration_model(deque<set<int>> &en, deque<int> &degrees);

#endif
