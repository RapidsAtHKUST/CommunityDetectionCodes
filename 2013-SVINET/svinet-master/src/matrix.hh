#ifndef MATRIX_HH
#define MATRIX_HH

#include <list>
#include <utility>

#include <assert.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

typedef std::pair<uint32_t, double> KV;
class Edge: public std::pair<uint32_t, uint32_t> {
public:
  typedef std::pair<uint32_t, uint32_t>  inherited;
  Edge(): inherited() { }
  Edge(uint32_t a, uint32_t b): inherited(a,b) { }
  bool ftos; // first to second
};
typedef std::pair<Edge, double> EdgeV;

using namespace std;

#define ROW_LIMIT 512
#define COL_LIMIT 32
#define CELL_LIMIT 32

static int cmpdouble(const void *p1, const void *p2);
static int cmpuint32(const void *p1, const void *p2);
static int cmppairval(const void *p1, const void *p2);
static int cmppairedgeval(const void *p1, const void *p2);

template <class T>
class D1Array {
public:
  D1Array(uint32_t m, bool zero = true);
  D1Array(const D1Array<T> &a);
  ~D1Array();

  uint32_t n() const { return _n; }
  uint32_t size() const { return _n; }

  T sum() const;
  double mean() const;
  double stdev(double mean) const;
  double zscore(double v) const;
  double logsum() const;
  bool dim_equal(const D1Array<T> &a) const;
  int first_positive_idx() const;

  T *data() { return _data; }
  const T * const data() const {return (const T *)_data;}
  const T * const const_data() const {return (const T *)_data;}

  D1Array &scale(T s);
  D1Array &abs();
  D1Array &exp();
  double max(uint32_t &idx) const;

  void set_elements(T v);
  void zero();
  void copy_from(const D1Array<T> &a);
  void add_to(const D1Array<T> &a);
  void lognormalize() { }
  void lognormalize(double log_sum) { }
  void lognormalize(double s, std::list<uint16_t> &v);
  void normalize();

  void sort();
  void sort_by_value();

  D1Array &operator*=(T);          // V * x
  D1Array &operator+=(const D1Array<T> &); // V + A
  D1Array &operator-=(const D1Array<T> &); // V - A

  T &operator[](uint32_t p);
  T operator[](uint32_t p) const;

  //const T at(uint32_t p) const { return assert(p < _n); _data[p];}

  string s() const;

private:
  uint32_t _n;
  T *_data;
};
typedef D1Array<uint32_t> uArray;
typedef D1Array<double> Array;

template<class T> inline
D1Array<T>::D1Array(uint32_t n, bool)
  :_n(n)
{
  _data = new T[n];
}

template<> inline
D1Array<std::vector<uint32_t> *>::D1Array(uint32_t n, bool zero)
  :_n(n)
{
  typedef std::vector<uint32_t> * X;
  _data = new X[n];
  if (zero) {
    for (uint32_t i = 0; i < n; ++i)
      _data[i] = NULL;
  }
}

template<> inline
D1Array<double>::D1Array(uint32_t n, bool zero)
  :_n(n)
{
  _data = new double[n];
  if (zero)
    for (uint32_t i = 0; i < n; ++i)
      _data[i] = 0;
}

template<> inline
D1Array<uint32_t>::D1Array(uint32_t n, bool zero)
  :_n(n)
{
  _data = new uint32_t[n];
  if (zero)
    for (uint32_t i = 0; i < n; ++i)
      _data[i] = 0;
}

template<> inline
D1Array<uint64_t>::D1Array(uint32_t n, bool zero)
  :_n(n)
{
  _data = new uint64_t[n];
  if (zero)
    for (uint32_t i = 0; i < n; ++i)
      _data[i] = 0;
}


template<class T> inline
D1Array<T>::D1Array(const D1Array<T> &a)
{
  copy_from(a);
}

template<class T> inline
D1Array<T>::~D1Array()
{
  delete[] _data;
}

template<class T> inline void
D1Array<T>::set_elements(T v)
{
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = v;
}

template<class T> inline void
D1Array<T>::zero()
{
  set_elements(.0);
}

template<> inline void
D1Array<KV>::zero()
{
  for (uint32_t i = 0; i < _n; ++i) {
    _data[i].first = 0;  
    _data[i].second = 0;  
  }
}

template<class T> inline void
D1Array<T>::copy_from(const D1Array<T> &a)
{
  assert (dim_equal(a));
  const T * const d = a.data();
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = d[i];
}

template<> inline void 
D1Array<double>::sort()  // desc
{
  qsort(_data, _n, sizeof(double), cmpdouble);
}

template<> inline void 
D1Array<uint32_t>::sort() // asc XXX
{
  qsort(_data, _n, sizeof(uint32_t), cmpuint32);
}

template<> inline void 
D1Array<KV>::sort_by_value()
{
  qsort(_data, _n, sizeof(KV), cmppairval);
}

template<> inline void 
D1Array<EdgeV>::sort_by_value()
{
  qsort(_data, _n, sizeof(EdgeV), cmppairedgeval);
}

static int
cmpdouble(const void *p1, const void *p2)
{
  double u = *(const double *)p1;
  double v = *(const double *)p2;
  return u < v;
}

static int
cmpuint32(const void *p1, const void *p2)
{
  uint32_t u = *(const uint32_t *)p1;
  uint32_t v = *(const uint32_t *)p2;
  return u > v;
}

static int
cmppairval(const void *p1, const void *p2)
{
  const KV &u = *(const KV *)p1;
  const KV &v = *(const KV *)p2;
  return u.second < v.second;
}

static int
cmppairedgeval(const void *p1, const void *p2)
{
  const EdgeV &u = *(const EdgeV *)p1;
  const EdgeV &v = *(const EdgeV *)p2;
  return u.second < v.second;
}

template<class T> inline void
D1Array<T>::add_to(const D1Array<T> &a)
{
  assert (dim_equal(a));
  const T * const d = a.data();
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] += d[i];
}

template<class T> inline bool
D1Array<T>::dim_equal(const D1Array<T> &a) const
{
  return a.n() == _n;
}

template<class T> inline T
D1Array<T>::sum() const
{
  T s = .0;
  for (uint32_t i = 0; i < _n; ++i)
    s += _data[i];
  return s;
}

template<class T> inline double
D1Array<T>::mean() const
{
  assert(_n > 0);
  return (double)sum() / _n;
}

template<class T> inline double
D1Array<T>::stdev(double m) const
{
  double l = .0;
  for (uint32_t i = 0; i < _n; ++i) {
    double p = _data[i];
    l += p * p;
  }
  return sqrt((l / _n) -  (m*m));
}

template<class T> inline double
D1Array<T>::zscore(double v) const
{
  double m = mean();
  double sd = stdev(m);
  if (sd == .0) {
    fprintf(stderr, "Warning: stdev is 0!\n");
    return .33333;
  }
  return (v - m) / sd;
}

template<class T> inline double
D1Array<T>::logsum() const
{
  // assume array is log(u), return log(sum(u))
  assert (_n > 0);
  if (_n == 1)
    return _data[0];
  T r = _data[0];
  for (uint32_t i = 1; i < _n; ++i)
    if (_data[i] < r)
      r = r + log(1 + ::exp(_data[i] - r));
    else
      r = _data[i] + log(1 + ::exp(r - _data[i]));
  return r;
}

template<> inline void
D1Array<double>::lognormalize()
{
  double s = logsum();
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = ::exp(_data[i] - s);
}

template<> inline void
D1Array<double>::lognormalize(double s)
{
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = ::exp(_data[i] - s);
}


template<> inline void
D1Array<double>::lognormalize(double s, std::list<uint16_t> &v)
{
  for (list<uint16_t>::const_iterator t = v.begin(); t != v.end(); ++t) {
    uint32_t k = *t;
    //printf("lognormalize: %f, %f\n", _data[k] - s, s);
    _data[k] = ::exp(_data[k] - s);
  }
}



template<> inline void
D1Array<double>::normalize()
{
  double s = sum();
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = _data[i] / s;
}

template<class T> inline D1Array<T> &
D1Array<T>::scale(T s)
{
  assert (s > .0);
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] *= s;
  return *this;
}

template<class T> inline D1Array<T> &
D1Array<T>::abs()
{
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = fabs(_data[i]);
  return *this;
}

template<class T> inline D1Array<T> &
D1Array<T>::exp()
{
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] = ::exp(_data[i]);
  return *this;
}

template<class T> inline int
D1Array<T>::first_positive_idx() const
{
  for (uint32_t i = 0; i < _n; ++i)
    if (_data[i] > .0)
      return i;
  return -1;
}

template<class T> inline D1Array<T> &
D1Array<T>::operator*=(T u)
{
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] *= u;
  return *this;
}

template<class T> inline D1Array<T> &
D1Array<T>::operator+=(const D1Array<T> &a)
{
  assert (dim_equal(a));
  const T * const d = a.data();
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] += d[i];
  return *this;
}

template<class T> inline D1Array<T> &
D1Array<T>::operator-=(const D1Array<T> &a)
{
  assert (dim_equal(a));
  const T * const d = a.data();
  for (uint32_t i = 0; i < _n; ++i)
    _data[i] -= d[i];
  return *this;
}

template<class T> inline void
sub(const D1Array<T> &a, const D1Array<T> &b, D1Array<T> &c)
{
  assert (a.dim_equal(b));
  const T * const ad = a.data();  
  const T * const bd = b.data();
  T *cd = c.data();
  for (uint32_t i = 0; i < a.n(); ++i)
    cd[i] = ad[i] - bd[i];
}

template<class T> inline void
prod(const D1Array<T> &a, T v, D1Array<T> &b)
{
  assert (a.dim_equal(b));
  const T * const ad = a.data();  
  T *bd = b.data();
  for (uint32_t i = 0; i < a.n(); ++i)
    bd[i] = ad[i] * v;
}


template<class T> inline double
dot(const D1Array<T> &a, const D1Array<T> &b)
{
  assert (a.dim_equal(b));
  double u = .0;
  for (uint32_t i = 0; i < a.n(); ++i)
      u += a[i] * b[i];
  assert (u >= .0);
  return u;
}

template<class T> inline double
dotverb(const D1Array<T> &a, const D1Array<T> &b)
{
  assert (a.dim_equal(b));
  double u = .0;
  for (uint32_t i = 0; i < a.n(); ++i) {
    if (a[i] < 1e-10 || b[i] < 1e-10)
      u += 1e-20;
    else
      u += a[i] * b[i];
    if (u < 1e-30)
      u = 1e-30;
  }
  return u;
}

template<class T> inline double
inner_prod_max(const D1Array<T> &a, const D1Array<T> &b, const D1Array<T> &c, 
	       uint32_t &idx)
{
  assert (a.dim_equal(b));
  double u = .0;
  idx = 0;
  double s = .0;
  for (uint32_t i = 0; i < a.n(); ++i) {
    double v = a[i] * b[i] * c[i];
    s += v;
    if  (v > u) {
      u = v;
      idx = i;
    }
  }
  return u / s;
}

template<class T> inline void
inner_prod(const D1Array<T> &a, const D1Array<T> &b, const D1Array<T> &c)
{
  assert (a.dim_equal(b));
  assert (a.dim_equal(c));

  for (uint32_t i = 0; i < a.n(); ++i)
    c[i] = a[i] * b[i];
}


template<class T> inline double
inner_prod_max2(const D1Array<T> &a, const D1Array<T> &b, const D1Array<T> &c, 
		uint32_t &idx)
{
  assert (a.dim_equal(b));
  assert (c.n() == a.n() + 1);
  double u = .0;
  idx = 0;
  double s = .0;
  double sum = .0;
  for (uint32_t i = 0; i < a.n(); ++i) {
    double v = a[i] * b[i] * c[i];
    sum += a[i] * b[i];
    s += v;
    if  (v > u) {
      u = v;
      idx = i;
    }
  }
  s += (1 - sum) * c[c.n()-1];
  double similar = (1 - sum) * c[c.n()-1] / s;
  double dissimilar = u / s;
  if (similar > dissimilar) {
    // retain idx
    return similar;
  } else {
    idx = c.n() - 1;
    return dissimilar;
  }
}


template<class T> inline double
D1Array<T>::max(uint32_t &idx) const
{
  double maxv = .0;
  for (uint32_t i = 0; i < n(); ++i) {
    if (_data[i] > maxv) {
      maxv = _data[i];
      idx = i;
    }
  }
  return maxv;
}

template<> inline double
D1Array<KV>::max(uint32_t &idx) const
{
  double max = .0;
  for (uint32_t i = 0; i < n(); ++i) {
    if (_data[i].second > max) {
      max = _data[i].second;
      idx = _data[i].first;
    }
  }
  return max;
}

template<class T> inline T &
D1Array<T>::operator[](uint32_t p)
{
  assert (p < _n);
  return _data[p];
}

template<class T> inline T
D1Array<T>::operator[](uint32_t p) const
{
  assert (p < _n);
  return _data[p];
}

template<> inline string
D1Array<double>::s() const
{
  ostringstream sa;
  uint32_t n = _n > ROW_LIMIT ? ROW_LIMIT : _n;
  sa << "\n[ ";
  for (uint32_t i = 0; i < n; ++i) {
    double u = _data[i];
    if (u < 1e-05 && u > .0)
      u = .0;
    sa << "(" << i << ":" << u << ") ";
  }
  if (_n > n)
    sa << "...";
  sa << "]";
  return sa.str();
}

template<> inline string
D1Array<uint32_t>::s() const
{
  ostringstream sa;
  uint32_t n = _n > ROW_LIMIT ? ROW_LIMIT : _n;
  sa << "\n[ ";
  for (uint32_t i = 0; i < n; ++i)
    sa << _data[i] << " ";
  if (_n > n)
    sa << "...";
  sa << "]";
  return sa.str();
}

template<> inline string
D1Array<KV>::s() const
{
  ostringstream sa;
  uint32_t n = _n > ROW_LIMIT ? ROW_LIMIT : _n;
  sa << "\n[ ";
  for (uint32_t i = 0; i < n; ++i)
    sa << "(" << _data[i].first << ", " << _data[i].second << ")";
  if (_n > n)
    sa << "...";
  sa << "]";
  return sa.str();
}


template <class T>
class D2Array {
public:
  D2Array(uint32_t m, uint32_t n, bool zero=true);
  D2Array(const D2Array<T> &a);
  ~D2Array();

  uint32_t m() const { return _m; }
  uint32_t n() const { return _n; }

  T at(uint32_t m, uint32_t n) const;

  // T must support operator+()
  void add(uint32_t m, uint32_t n, T val);
  void set(uint32_t m, uint32_t n, T val);

  void lognormalize();
  double logsum() const;

  T **data() { return _data; }
  const T ** const data() const {return (const T **)_data;}
  const T ** const const_data() const {return (const T **)_data;}

  D2Array &operator*=(T);
  D2Array &operator+=(const D2Array<T> &);

  double abs_mean() const;

  // expensive
  void slice(uint32_t dim, uint32_t p, D1Array<T> &v) const;

  bool dim_equal(const D2Array<T> &a) const;
  T sum(uint32_t p) const;
  
  void set_elements(T v);
  void set_elements(uint32_t m, const Array &v);
  double dot(uint32_t i, uint32_t j);
  void zero();
  void zero(uint32_t a);
  int copy_from(const D2Array<T> &a);
  int add_to(const D2Array<T> &a);
  void add_slice(uint32_t p, const D1Array<T> &u);
  void sub_slice(uint32_t p, const D1Array<T> &u);

  D2Array<T> & scale(T s);
  D2Array<T> & scale(uint32_t p, T s);

  void reset(D2Array<T> &u);
  void reset();
  void swap(D2Array<T> &u);

  string s() const;

private:
  uint32_t _m;
  uint32_t _n;
  T **_data;
};

template<class T> inline
D2Array<T>::D2Array(uint32_t m, uint32_t n, bool zero):
  _m(m), _n(n)
{
  _data = new T*[m];
  for (uint32_t i = 0; i < m; ++i) {
    _data[i] = new T[n];
    if (zero)
      memset(_data[i], 0, sizeof(T)*n);
  }
}

template<class T> inline
D2Array<T>::~D2Array()
{
  for (uint32_t i = 0; i < _m; ++i)
    delete[] _data[i];
  delete[] _data;
}

template<class T> inline
D2Array<T>::D2Array(const D2Array<T> &a)
{
  copy_from(a);
}

template<class T> inline T
D2Array<T>::at(uint32_t m, uint32_t n) const 
{
  assert (m < _m && n < _n);
  return _data[m][n];
}

template<class T> inline void
D2Array<T>::add(uint32_t m, uint32_t n, T val)
{
  assert (m < _m && n < _n);
  _data[m][n] += val;
}

template<class T> inline void
D2Array<T>::set(uint32_t m, uint32_t n, T val)
{
  assert (m < _m && n < _n);
  _data[m][n] = val;
}


template<class T> inline void
D2Array<T>::set_elements(T v)
{
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] = v;
}

template<class T> inline void
D2Array<T>::set_elements(uint32_t m, const Array &v)
{
  assert (v.size() == _n);
  for (uint32_t j = 0; j < _n; ++j)
    _data[m][j] = v[j];
}

template<class T> inline void
D2Array<T>::reset(D2Array<T> &u)
{
  for (uint32_t i = 0; i < _m; ++i)
    delete[] _data[i];
  delete[] _data;
  _data = u.data();
  u.reset();
}

template<class T> inline void
D2Array<T>::swap(D2Array<T> &u)
{
  T **d = _data;
  _data = u.data();
  u._data = d;
}

template<class T> inline void
D2Array<T>::reset()
{
  _data = new T*[_m];
  for (uint32_t i = 0; i < _m; ++i)
    _data[i] = new T[_n];
  // note: random init
}

template<class T> inline void
D2Array<T>::zero()
{
  set_elements(.0);
}

template<class T> inline void
D2Array<T>::zero(uint32_t a)
{
  for (uint32_t j = 0; j < _n; ++j)
    _data[a][j] = 0;
}

template<class T> inline D2Array<T> &
D2Array<T>::scale(T s)
{
  assert (s > .0);
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] *= s;
  return *this;
}

template<class T> inline D2Array<T> &
D2Array<T>::scale(uint32_t p, T s)
{
  assert (s > .0);
  assert (p < _m);
  for (uint32_t j = 0; j < _n; ++j)
    _data[p][j] *= s;
  return *this;
}

template<class T> inline int
D2Array<T>::copy_from(const D2Array<T> &a)
{
  if (!dim_equal(a))
    return -1;
  const T ** const d = a.data();
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] = d[i][j];
  return 0;
}

template<class T> inline int
D2Array<T>::add_to(const D2Array<T> &a)
{
  if (!dim_equal(a))
    return -1;
  const T ** const d = a.data();
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] += d[i][j];
  return 0;
}

template<class T> inline bool
D2Array<T>::dim_equal(const D2Array<T> &a) const
{
  return a.m() == _m && a.n() == _n;
}

template<class T> inline T
D2Array<T>::sum(uint32_t p) const
{
  T s = .0;
  for (uint32_t j = 0; j < _n; ++j)
    s += _data[p][j];
  return s;
}

template<class T> inline void
D2Array<T>::add_slice(uint32_t p, const D1Array<T> &u)
{
  assert (_n == u.n());
  const T * const ud = u.data();  
  for (uint32_t i = 0; i < u.n(); ++i)
    _data[p][i] += ud[i];
}

template<class T> inline void
D2Array<T>::sub_slice(uint32_t p, const D1Array<T> &u)
{
  assert (_n == u.n());
  const T * const ud = u.data();  
  for (uint32_t i = 0; i < u.n(); ++i)
    _data[p][i] -= ud[i];
}

template<class T> inline double
D2Array<T>::logsum() const
{
  // assume array is log(u), return log(sum(u))
  assert (_n > 0);
  if (_n == 1)
    return _data[0][0];
  T r = _data[0][0];
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j) {
      if (i == 0 && j == 0)
	continue;
      if (_data[i][j] < r)
	r = r + log(1 + ::exp(_data[i][j] - r));
      else
	r = _data[i][j] + log(1 + ::exp(r - _data[i][j]));
    }
  return r;
}

template<> inline void
D2Array<double>::lognormalize()
{
  double s = logsum();
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] = ::exp(_data[i][j] - s);
}

template<class T> inline D2Array<T>&
D2Array<T>::operator*=(T v)
{
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] *= v;
  return *this;
}

template<class T> inline D2Array<T>&
D2Array<T>::operator+=(const D2Array<T> &u)
{
  assert (dim_equal(u));
  const T ** const ud = u.data();
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] += ud[i][j];  
  return *this;
}

template<class T> inline string 
D2Array<T>::s() const
{
  ostringstream sa;
  uint32_t m = (_m > ROW_LIMIT) ? ROW_LIMIT : _m;
  uint32_t n = (_n > COL_LIMIT) ? COL_LIMIT : _n;
  sa << "\n[ ";
  for (uint32_t i = 0; i < m; ++i) {
    for (uint32_t j = 0; j < n; ++j) {
      double u = _data[i][j];
      if (u < 1e-05 && u > .0)
	u = .0;

      if (i > 0 && j == 0)
	sa << "  " << u << " ";
      else
	sa << u << " ";
    }
    if (_n > n)
      sa << "..\n";
    sa << "\n";
  }
  if (m > _m)
    sa << "...\n";
  sa << "]";
  return sa.str();
}

template<> inline string 
D2Array<KV>::s() const
{
  ostringstream sa;
  uint32_t m = (_m > ROW_LIMIT) ? ROW_LIMIT : _m;
  uint32_t n = (_n > COL_LIMIT) ? COL_LIMIT : _n;
  sa << "\n[ ";
  for (uint32_t i = 0; i < m; ++i) {
    for (uint32_t j = 0; j < n; ++j) {
      double v = _data[i][j].first;
      double u = _data[i][j].second;
      if (u < 1e-05 && u > .0)
	u = .0;

      if (i > 0 && j == 0)
	sa << "  " << v << ":" << u << " ";
      else
	sa << v << ":" << u << " ";
    }
    if (_n > n)
      sa << "..\n";
    sa << "\n";
  }
  if (m > _m)
    sa << "...\n";
  sa << "]";
  return sa.str();
}

template<class T> inline double
D2Array<T>::dot(uint32_t i, uint32_t j)
{
  assert (i < _m && j < _m);
  double u = .0;
  for (uint32_t k = 0; k < _n; ++k)
    u += _data[i][k] * _data[j][k];
  return u;
}

template<class T> inline void
D2Array<T>::slice(uint32_t const_dim, uint32_t p, D1Array<T> &v) const
{
  assert (const_dim < 2);
  if (const_dim == 0) {
    assert (p < _m && v.n() == _n);
    for (uint32_t q = 0; q < _n; ++q)
      v[q] = _data[p][q];
  } else if (const_dim == 1) {
    assert (p < _n && v.n() == _m);
    for (uint32_t q = 0; q < _m; ++q)
      v[q] = _data[q][p];
  }
}

template<class T> inline void
sub(const D2Array<T> &a, const D2Array<T> &b, D2Array<T> &c)
{
  assert (a.dim_equal(b));
  const T ** const ad = a.data();
  const T ** const bd = b.data();
  T **cd = c.data();
  for (uint32_t i = 0; i < a.m(); ++i)
    for (uint32_t j = 0; j < a.n(); ++j)
      cd[i][j] = ad[i][j] - bd[i][j];
}


template<class T> inline double
D2Array<T>::abs_mean() const
{
  double s = .0;
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      s += fabs(_data[i][j]);
  return s / (_m * _n);
}

template <class T>
class D3Array {
public:
  D3Array(int m, int n, int k);
  ~D3Array();

  uint32_t m() const { return _m; }
  uint32_t n() const { return _n; }
  uint32_t k() const { return _k; }

  T ***data() { return _data; }
  const T *** const data() const { return (const T ***)_data; }
  const T *** const const_data() const { return (const T ***)_data; }

  T at(uint32_t m, uint32_t n, uint32_t k) const;

  // T must support operator+()
  void add(uint32_t m, uint32_t n, uint32_t k, T val);
  void set(uint32_t m, uint32_t n, uint32_t k, T val);

  void copy_slice(uint32_t p, uint32_t q, const D1Array<T> &u);
  bool dim_equal(const D3Array<T> &a) const;
  void set_elements(T v);
  void zero();

  string s() const;

private:
  uint32_t _m;
  uint32_t _n;
  uint32_t _k;
  T ***_data;
};

template<class T> inline
D3Array<T>::D3Array(int m, int n, int k)
  :_m(m), _n(n), _k(k)
{
  _data = new T**[_m];
  for (uint32_t i = 0; i < _m; ++i) {
    _data[i] = new T*[_n];
    for (uint32_t j = 0; j < _n; ++j)
      _data[i][j] = new T[_k];
  }
}

template<class T> inline
D3Array<T>::~D3Array()
{
  for (uint32_t i = 0; i < _m; ++i) {
    for (uint32_t j = 0; j < _n; ++j)
      delete[] _data[i][j];
    delete[] _data[i];
  }
  delete[] _data;
}

template<class T> inline T
D3Array<T>::at(uint32_t m, uint32_t n, uint32_t k) const
{
  assert (m < _m && n < _n  && k < _k);
  return _data[m][n][k];
}

template<class T> inline void
D3Array<T>::add(uint32_t m, uint32_t n, uint32_t k, T val)
{
  assert (m < _m && n < _n && n < _k);
  _data[m][n][k] += val;
} 

template<class T> inline void
D3Array<T>::set(uint32_t m, uint32_t n, uint32_t k, T val)
{
  assert (m < _m && n < _n && n < _k);
  _data[m][n][k] = val;
} 

template<class T> inline void
D3Array<T>::copy_slice(uint32_t p, uint32_t q, const D1Array<T> &u)
{
  assert (_k == u.n());
  const T * const ud = u.data();  
  for (uint32_t i = 0; i < u.n(); ++i)
    _data[p][q][i] = ud[i];
}

template<class T> inline bool
D3Array<T>::dim_equal(const D3Array<T> &a) const
{
  return a.m() == _m && a.n() == _n && a.k() == _k;
}
template<class T> inline void
D3Array<T>::set_elements(T v)
{
  for (uint32_t i = 0; i < _m; ++i)
    for (uint32_t j = 0; j < _n; ++j)
      for (uint32_t k = 0; k < _k; ++k)
	_data[i][j][k] = v;
}

template<class T> inline void
D3Array<T>::zero()
{
  set_elements(.0);
}

template<class T> inline string 
D3Array<T>::s() const
{
  ostringstream sa;
  uint32_t m = (_m > ROW_LIMIT) ? ROW_LIMIT : _m;
  uint32_t n = (_n > COL_LIMIT) ? COL_LIMIT : _n;
  uint32_t t = (_k > CELL_LIMIT) ? CELL_LIMIT : _k;
  sa << "\n[ ";
  for (uint32_t i = 0; i < m; ++i) {
    for (uint32_t j = 0; j < n; ++j) {
      sa << "(";
      for (uint32_t k = 0; k < t; ++k) {
	if (k == t - 1)
	  sa << _data[i][j][k];
	else
	  sa << _data[i][j][k] << ",";
      }
      sa << ")";
    }
    if (_n > n)
      sa << "..\n";
    sa << "\n";
  }
  if (m > _m)
    sa << "...\n";
  sa << "]";
  return sa.str();
}

#endif
