/*
lcelib, A high performance library for complex networks research
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


/**
 * Some metaprogramming for prefetching logic. The depth of possible 
 * prefetch stages is calculated recursively: be wary when using it, as
 * recursive definitions are easy to arrive to. 
 */


#ifdef GNU_PREFETCH

void lce_prefetch(const void * const loc) {
  __builtin_prefetch(loc,0,0);
}

template<typename T>
struct PrefetchDepth {
  enum {depth=0}
};

template<typename T, unsigned depth=0>
struct PrefetchPolicy {
  void prefetch(const T & tgt) {};
  void safe_prefetch(const T & tgt) {};
  enum {depth=0};
};

template<typename T, unsigned depth>
struct PrefetchPolicy<T*, depth> {
  /* We trust the compiler to eliminate unnecessarities: */
  void prefetch(const (T*) & tgt, size_t index=0) {
    PrefetchPolicy<T, depth-1>::prefetch(&(tgt[index]));
  }
  void safe_prefetch(const (T*) & tgt, size_t index=0) {
    if (tgt) PrefetchPolicy<T, depth-1>::safe_prefetch(&(tgt[index]));
  }
};

template<typename T>
struct PrefetchPolicy<T*,0> {
  void prefetch(const (T*) & tgt, size_t index=0) {
    lce_prefetch(&(tgt[index]));
  }
  void safe_prefetch(const (T*) & tgt, size_t index=0) {
    lce_prefetch(&(tgt[index]));
  }
};


#else 

void lce_prefetch(const void * const loc) {
  __builtin_prefetch(loc,0,0);
}

template<typename T, unsigned depth=0>
struct PrefetchPolicy {
  void prefetch(const T & tgt, size_t index=0) {};
};

#endif 

