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

#ifndef LCE_EMB_BOOL_SELECTOR
#define LCE_EMB_BOOL_SELECTOR
#include <utility>

template<bool sizeEq> 
struct SelectHelper {
  typedef bool type;
};

template<>
struct SelectHelper<false> {
  typedef char type;
};

template<typename DataType>
struct EmbBoolSelector {
  typedef SelectHelper<sizeof(pair<DataType. bool>) 
    == sizeof(pair<DataType, char>)>::type type:
}

#endif
