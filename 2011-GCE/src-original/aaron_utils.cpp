#include "aaron_utils.hpp"
int testForError(FILE * x) { return x==NULL; }
int testForError(int x) { return x==-1; }

string show(long x) { ostringstream s; s << x; return s.str(); }
string show(const char * x) { ostringstream s; s << x; return s.str(); }
string show(const string &s) { return s; }

runningAverage::runningAverage() : total(0) , n(0) {}
void runningAverage::operator() (long int i) {
	total+=i;
	if(total<0) Die("total overflowed %ld", i);
	n++;
}
string runningAverage::operator() (void) const {
	ostringstream o;
	o << "Average Time (" << n << " points)" << ": " << (double)total/(double)n;
	return o.str();
}

DummyOutputStream& DummyOutputStream::operator << (int) { return *this; }
DummyOutputStream& DummyOutputStream::operator << (const char*) { return *this; }
DummyOutputStream dummyOutputStream;
