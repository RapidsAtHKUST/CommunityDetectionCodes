//
// Created by cheyulin on 2/27/17.
//

#ifndef INC_2009_LFM_DEQUE_NUMERIC_H
#define INC_2009_LFM_DEQUE_NUMERIC_H

#include <cmath>
#include <set>
#include <deque>

bool compare(deque<double> &one, deque<double> &two);

double Euclidean_norm(const deque<double> &a);

int Euclidean_normalize(deque<double> &a);

double scalar_product(deque<double> &a, deque<double> &b);

int orthogonalize(deque<double> &a, deque <deque<double>> &M);

int matrix_time_vector(deque <deque<double>> &Q, deque<double> &v, deque<double> &new_s);

void set_to_deque(const set<int> &s, deque<int> &a);

void set_to_deque(const set<double> &s, deque<double> &a);

void deque_to_set(const deque<double> &a, set<double> &s);

void deque_to_set(const deque<int> &a, set<int> &s);

double norm_one(const deque<double> &a);

int normalize_one(deque<double> &a);

#endif //INC_2009_LFM_DEQUE_NUMERIC_H
