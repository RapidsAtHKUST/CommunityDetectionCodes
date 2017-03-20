#if !defined(CAST_INCLUDED)
#define CAST_INCLUDED

#include <cmath>
#include <deque>
#include <string>

using namespace std;

bool cast_string_to_double(string &b, double &h);

double cast_string_to_double(string &b);

int cast_int(double u);

int cast_string_to_char(string &file_name, char *b);

bool cast_string_to_doubles(string &b, deque<double> &v);

bool cast_string_to_doubles(string &b, deque<int> &v);

bool separate_strings(string &b, deque<string> &v);

double approx(double a, int digits);

#endif
