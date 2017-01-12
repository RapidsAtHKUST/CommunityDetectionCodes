#if !defined(RANDOM_INCLUDED)
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




double ran2(long *idum) {
	int j;
	long k;
	static long idum2=123456789;
	static long iy=0;
	static long iv[R2_NTAB];
	double temp;

	if(*idum<=0 || !iy){
		if(-(*idum)<1) *idum=1*(*idum);
		else *idum=-(*idum);
		idum2=(*idum);
		for(j=R2_NTAB+7;j>=0;j--){
			k=(*idum)/R2_IQ1;
			*idum=R2_IA1*(*idum-k*R2_IQ1)-k*R2_IR1;
			if(*idum<0) *idum+=R2_IM1;
			if(j<R2_NTAB) iv[j]=*idum;
		}
		iy=iv[0];
	}
	k=(*idum)/R2_IQ1;
	*idum=R2_IA1*(*idum-k*R2_IQ1)-k*R2_IR1;
	if(*idum<0) *idum+=R2_IM1;
	k=(idum2)/R2_IQ2;
	idum2=R2_IA2*(idum2-k*R2_IQ2)-k*R2_IR2;
	if (idum2 < 0) idum2 += R2_IM2;
	j=iy/R2_NDIV;
	iy=iv[j]-idum2;
	iv[j]=*idum;
	if(iy<1) iy+=R2_IMM1;
	if((temp=R2_AM*iy)>R2_RNMX) return R2_RNMX;
	else return temp;
}



double ran4(bool t, long s) {
	
	double r=0;
	
	
	static long seed_=1;
	
	if(t)
		r=ran2(&seed_);
	else
		seed_=s;
	

	return r;
}


double ran4() {
	
	return ran4(true, 0);
}


void srand4(void) {
	
	long s=(long)time(NULL);
	ran4(false, s);
	
	
	
}

void srand5(int rank) {
	
	long s=(long)(rank);
	ran4(false, s);
	
}



int irand(int n) {

	return (int(ran4()*(n+1)));
	
}


void srand_file(void) {

	ifstream in("time_seed.dat");
	int seed;
	
	if (!in.is_open())
		seed=21111983;
	else
		in>>seed;
	
	if (seed < 1 || seed>R2_IM2)
		seed=1;
	
	
	srand5(seed);
	ofstream out("time_seed.dat");
	out<<seed+1<<endl;
	

}

#endif
