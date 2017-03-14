//
// Created by cheyulin on 2/27/17.
//

#ifndef INC_2009_LFM_MUTUAL_H
#define INC_2009_LFM_MUTUAL_H

#include "util/common/histograms.h"

int overlap_grouping(deque<deque<int>> ten, int unique);

double mutual(deque<deque<int>> en, deque<deque<int>> ten);

double H(double a);

double H(deque<double> &p);

double H_x_given_y(deque<deque<int>> &en, deque<deque<int>> &ten, int dim);

double mutual2(deque<deque<int>> en, deque<deque<int>> ten);

double H_x_given_y3(deque<deque<int>> &en, deque<deque<int>> &ten, int dim);

double mutual3(deque<deque<int>> en, deque<deque<int>> ten);

#endif //INC_2009_LFM_MUTUAL_H
