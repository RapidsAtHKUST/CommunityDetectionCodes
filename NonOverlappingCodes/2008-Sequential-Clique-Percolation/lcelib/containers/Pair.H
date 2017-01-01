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

#ifndef LCE_PAIR
#define LCE_PAIR
#include<cstring>
#include<memory>

/**
 * An utility pair class for containers. Partial specializations for void
 * members exist: in order to simplify the design, they return the non-void
 * member if a reference for the void one is called. For safety, only 
 * const references are given out this way, however.
 * 
 * Includes typedefs in order to avoid refs to void. 
 */

template<typename FirstType, typename SecondType>
class Pair {
protected:
  FirstType _first;
  SecondType _second;
  
public:
  typedef FirstType & first_reference;
  typedef SecondType & second_reference;
  typedef const FirstType & const_first_reference;
  typedef const SecondType & const_second_reference;
  FirstType & first() {return _first;}
  SecondType & second() {return _second;}
  const FirstType & first() const {return _first;}
  const SecondType & second() const {return _second;}

  void removeSecond() {_second.~SecondType();}
  void clearSecond() {
    SecondType temp=SecondType();
    memcpy(&(_second),
	   &temp,
	   sizeof(SecondType));
  }
};

template<typename FirstType>
class Pair<FirstType, void> {
  FirstType _first;

public:
  typedef FirstType & first_reference;
  typedef const FirstType & second_reference;
  typedef const FirstType & const_first_reference;
  typedef const FirstType & const_second_reference;
  FirstType & first() {return _first;}
  const FirstType & first() const {return _first;}
  const FirstType & second() const {return _first;}

  void removeSecond() {}
};

template<typename SecondType>
class Pair<void, SecondType> {
  SecondType _second;

public:
  typedef const SecondType & first_reference;
  typedef SecondType & second_reference;
  typedef const SecondType & const_first_reference;
  typedef const SecondType & const_second_reference;
  SecondType & second() {return _second;}
  const SecondType & second() const {return _second;}
  const SecondType & first() const {return _second;}

  void removeSecond() {_second.~SecondType();}
  
  void clearSecond() {
    SecondType temp=SecondType();
    memcpy(&(_second),
	   &temp,
	   sizeof(SecondType));
  }
};
  

#endif
