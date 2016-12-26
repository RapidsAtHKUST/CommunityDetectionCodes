#ifndef _AARON_UTILS_H
#define _AARON_UTILS_H

using namespace std;
//#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

enum {FALSE=0, TRUE=1};
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define P(...) printf(__VA_ARGS__)
#define Perror(...) fprintf(stderr, __VA_ARGS__)
#define Pn(...) do { P(__VA_ARGS__); fputc('\n', stdout); } while (0)
#define Perrorn(...) do { Perror(__VA_ARGS__); fputc('\n', stderr); } while (0)
//#define PP(x) Pn("%s:%s", #x, show(x).c_str())
#define PP(x) cerr << #x << ":" << x << endl
#define PPLg(x) Pn("%s:%20.11Lg", #x, x)
static string thousandsSeparated(unsigned long x) {
	ostringstream s;
	if(x>=1000) {
		s << thousandsSeparated(x/1000) << ',';
		s << setfill('0');
	}
	s << setw(3) << x % 1000;
	x/=1000;
	return s.str();
}
#define PPdec(x) cerr << #x << ":" << thousandsSeparated(x) << endl
#define PPhex(x) cerr << #x << ":" << std::hex << std::setw(20) << x << std::dec << endl
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

string show(long x);
string show(const char * x);
string show(const string &x);

class runningAverage {
	long int total;
	long int n;
public:
	runningAverage();
	void operator() (long int i);
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

#endif
