//
// Created by cheyulin on 2/27/17.
//

#ifndef INC_2009_LFM_CC_H
#define INC_2009_LFM_CC_H

#include <deque>
#include <set>

using namespace std;

#include "cast.h"
#include "combinatorics.h"

bool they_are_mate(int a, int b, const deque<deque<int>> &member_list);

int common_neighbors(int a, int b, deque<set<int>> &en);

double compute_cc(deque<set<int>> &en, int i);

double compute_cc(deque<set<int>> &en);

double compute_tot_t(deque<set<int>> &en);

int choose_the_least(deque<set<int>> &en, deque<int> &A, int a, int &cn_a_o);

int cclu(deque<set<int>> &en, const deque<deque<int>> &member_list,
         const deque<deque<int>> &member_matrix, double ca);


#endif //INC_2009_LFM_CC_H
