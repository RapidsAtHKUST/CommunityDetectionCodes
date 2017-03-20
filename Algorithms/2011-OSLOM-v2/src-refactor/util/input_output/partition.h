//
// Created by cheyulin on 3/14/17.
//
#ifndef INC_2011_OSLOM_PARTITION_H
#define INC_2011_OSLOM_PARTITION_H

#include <algorithm>
#include <deque>
#include <fstream>
#include <map>

#include <util/common/cast.h>

int get_partition_from_file(string s, deque<deque<int>> &M);

int get_partition_from_file_list(string s, deque<deque<int>> &ten);

int get_partition_from_file_tp_format(string S, deque<deque<int> > &M, bool anyway);

int get_partition_from_file_tp_format(string S, deque<deque<int> > &M);

int get_partition_from_file(string s, deque<deque<int> > &M, int min);

#endif //INC_2011_OSLOM_PARTITION_H
