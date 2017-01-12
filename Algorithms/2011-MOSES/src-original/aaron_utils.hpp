#ifndef _AARON_UTILS_H
#define _AARON_UTILS_H

using namespace std;
//#include <string>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <map>
#include "Range.hpp"

typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned int uint;

enum {FALSE=0, TRUE=1};
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define P(...) printf(__VA_ARGS__)
#define Pf(f, ...) fprintf(f, __VA_ARGS__)
#define Perror(...) fprintf(stderr, __VA_ARGS__)
#define Pn(...) do { P(__VA_ARGS__); fputc('\n', stdout); } while (0)
#define Perrorn(...) do { Perror(__VA_ARGS__); fputc('\n', stderr); } while (0)
//#define PP(x) Pn("%s:%s", #x, show(x).c_str())
#define PP(x) cout << #x << ":" << x << endl
#define PPt(x) cout << #x << ":" << x << '\t'
#define PPnn(x) cout << #x << ":" << x
#define PP2(x,y) cout << #x << ',' << #y << ":\t" << x << ',' << y << endl
#define PP3(x,y,z) cout << #x << ',' << #y << ',' << #z << ":\t" << x << ',' << y << ',' << z << endl
#define PP4(w,x,y,z) cout << #w << ',' << #x << ',' << #y << ',' << #z << ":\t" << w << ',' << x << ',' << y << ',' << z << endl
#define PPLg(x) Pn("%s:%20.11Lg", #x, x)
string thousandsSeparated(uint64 x);
#define PPdec(x) cout << #x << ":" << thousandsSeparated(x) << endl
#define PPhex(x) cout << #x << ":" << std::hex << std::setw(20) << x << std::dec << endl
#define Print(x)  P("%s", show(x).c_str())
#define Printn(x) P("%s\n", show(x).c_str())
enum {
	EXIT_UNSPECIFIED=1  // other (unspecified) error. Try to avoid using this
	, EXIT_FILEOPEN     // child process failed because it couldn't open the requested file
	, EXIT_CHILD     // other error in the child process, e.g. execl failed
	};
#define Die(...) ( ({ Perrorn(__VA_ARGS__); exit(EXIT_UNSPECIFIED); 0; }) )
#define orDie(x ,errorCond ,msg) ({ typeof(x) y = (x); y!=errorCond || Die(msg); y; })
#define pipe_orDie(filedes)            orDie(pipe(filedes) ,-1 ,"Couldn't pipe(2)")
#define fork_orDie()                   orDie(fork()        ,-1 ,"Couldn't fork(2)")
#define orDie1(x) orDie(x, -1, "died @ " __FILE__ ":" TOSTRING(__LINE__) "\t(" #x ")")
#define orExitChild(x) ({ typeof(x) y = (x); if (testForError(y)) exit(EXIT_CHILD); y; })

int testForError(FILE * x);
int testForError(int x);

string show(int64 x);
string show(const char * x);
string show(const string &x);

class runningAverage {
	int64 total;
	int64 n;
public:
	runningAverage();
	void operator() (int64 i);
	string operator() (void) const;
} ;
#define unless(x) if(!(x))

template <class T1, class T2>
struct RefPair {
	T1& e1;
	T2& e2;
	RefPair(T1 &in1, T2 &in2) : e1(in1), e2(in2) { }
	void operator= (pair<T1, T2> source) { e1 = source.first; e2 = source.second; }
};
template <class T1, class T2>
RefPair<T1, T2> make_refpair(T1 &e1, T2 &e2) {
	RefPair<T2, T2> rf(e1,e2);
	return rf;
}

struct DummyOutputStream {
	DummyOutputStream& operator << (int);
	DummyOutputStream& operator << (const char*);
};
extern DummyOutputStream dummyOutputStream;

struct StopWatch { // TODO: put the stopWatch in another file?
	struct timeval tp;
	struct timeval last_laptime;
	StopWatch() {
		gettimeofday(&tp, NULL);
		last_laptime = tp;
	}
	void laptime(void) { laptime(""); }
	void laptime(const string &tag) {
		struct timeval tp_new;
		gettimeofday(&tp_new, NULL);
		double sinceStart   = (double)(tp_new.tv_sec - tp.tv_sec) + (double)(tp_new.tv_usec - tp.tv_usec) / 1000000;
		double sinceLastLap = (double)(tp_new.tv_sec - last_laptime.tv_sec) + (double)(tp_new.tv_usec - last_laptime.tv_usec) / 1000000;
			Pn("                                              <StopWatch> %.3f (+%.3f) %s", sinceStart, sinceLastLap, tag.c_str());
		last_laptime = tp_new;
	}
};

#define assertEQint(x,y) do { unless((x)==(y)) { PP(x); PP(y); } assert((x)==(y)); } while(0)
#define DYINGWORDS(x) for (int klsdjfslkfj = (x) ? 0 : 1; klsdjfslkfj!=0; klsdjfslkfj--, ({ assert (x); }) )
#define UNUSED __attribute__ ((__unused__))

template<class K, class V> class maxOnlymap : public map<K, V> {
public:
	pair<typename map<K,V>::iterator,bool> insert ( const typename map<K,V>::value_type& x ) {
		if(this->empty()) {
			return map<K,V>::insert(x);
		}
		if(x.first <= this->begin()->first ) {
			return make_pair(this->end(), true); // ignore it.
		}
		this->clear();
		return map<K,V>::insert(x);
	}
};

struct Timer {
	struct timeval start;
	string _s;
	Timer() {
		gettimeofday(&start, NULL);
	}
	Timer(const string &s): _s(s) {
		gettimeofday(&start, NULL);
	}
	double age() const {
		struct timeval end;
		gettimeofday(&end, NULL);
		double diff = end.tv_sec - start.tv_sec + 1e-6 * (end.tv_usec - start.tv_usec);
		return diff;
	}
	~Timer() {
		double diff = this->age();
		P("                                                                          "); cout << "Timer " << _s << ": " << diff << " s" << endl;
	}
};

namespace amd {

istream *zcatThis(const char *gzippedFile);
bool fileExists(const char *);

struct DebugCounter {
	int count;
	int freq;
	std::string s;
	DebugCounter(const std::string &_s, int _freq = 1000);
	void operator++ ();
};

template<class T>
void Histogram(T r) {
	typedef typename tr1::remove_const<typename tr1::remove_reference<typename T::value_type>::type>::type V;
	map<V, int> freqs;
	size_t total = 0;
	while(!r.empty()) {
		freqs[r.front()]++;
		r.popFront();
		++total;
	}
	pair<V, int> p;
	cout << endl << "\t x\t freq" << endl;
	forEach (p, mk_range(freqs)) {
		cout << "\t" << p.first << "\t" << p.second  << endl;
	}
	cout << "\t total\t " << total << endl;
	cout << endl;
}
template<class T1, class T2>
void TwoWayTable(T1 r1, T2 r2) {
	typedef typename tr1::remove_const<typename tr1::remove_reference<typename T1::value_type>::type>::type V1;
	typedef typename tr1::remove_const<typename tr1::remove_reference<typename T2::value_type>::type>::type V2;
	map<V1, int> xs;
	map<V2, int> ys;
	map<pair<V1,V2>, int> freqs;
	size_t total = 0;
	while(!r1.empty() && !r2.empty()) {
		freqs[make_pair(r1.front(), r2.front())]++;
		xs[r1.front()]++;
		ys[r2.front()]++;
		r1.popFront();
		r2.popFront();
		++total;
	}
	pair<V1, int> x;
	pair<V2, int> y;
	forEach(y, mk_range(ys)) {
		forEach(x, mk_range(xs)) {
			cout << setw(10) << freqs[make_pair(x.first,y.first)];
			cout << " " << x.first << "," << setw(2) << y.first;
		}
		cout << " sub:" << setw(10) << y.second << endl;
	}
	forEach(x, mk_range(xs)) {
			cout << setw(10) << x.second;
			cout << "  @ " << x.first;
	}
	cout << "\t total\t " << total << endl;
	cout << endl;

	forEach(y, mk_range(ys)) {
		forEach(x, mk_range(xs)) {
			double expected = double(x.second) / double (total) * double(y.second);// / double (total)
			cout << setw(10) << freqs[make_pair(x.first,y.first)] / expected;
			cout << " " << x.first << "," << setw(2) << y.first;
		}
		cout << "   sub:" << setw(10) << double(y.second)/total << endl;
	}
	forEach(x, mk_range(xs)) {
			cout << setw(10) << double(x.second)/total;
			cout << "  @ " << x.first;
	}

	cout << endl;
	cout << endl;
}

template<class C>
typename C::value_type constAt(const C &c, int i) {
	return c.at(i);
}

} // namespace amd

#define VERYCLOSE(a,b) (1e-10 > fabs((a)-(b)))

#define printfstring(...) ({ char str[1000]; sprintf(str, __VA_ARGS__) ; (std::string (str)); })
#define printfCstring(...) (printfstring(__VA_ARGS__).c_str())

#endif
