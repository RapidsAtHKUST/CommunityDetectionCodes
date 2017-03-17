//
// Created by cheyulin on 3/14/17.
//
#include "wsarray.h"


void prints(wsarray &a) {
    for (int i = 0; i < a.size(); i++)
        cout << a.l[i] << "\t" << a.w[i].first << " " << a.w[i].second << endl;
    cout << endl;
}

void prints(wsarray &a, ostream &pout) {
    for (int i = 0; i < a.size(); i++)
        cout << a.l[i] << "\t" << a.w[i].first << " " << a.w[i].second << endl;
    pout << endl;
}

void prints(wsarray *a, ostream &pout) {
    for (int i = 0; i < a->size(); i++)
        cout << a->l[i] << "\t" << a->w[i].first << " " << a->w[i].second << endl;
    pout << endl;
}

void prints(wsarray *a) {
    prints(a, cout);
}

wsarray::wsarray(int a) {
    position = 0;
    _size_ = a;
    l = new int[_size_];
    w = new pair<int, double>[_size_];
}

wsarray::~wsarray() {
    delete[] l;
    l = NULL;
    delete[] w;
    w = NULL;
}

pair<int, pair<int, double> > wsarray::posweightof(int x) {
    int i = find(x);
    if (i == -1)
        return (make_pair(-1, make_pair(0, 0.)));
    return (make_pair(i, w[i]));
}

int wsarray::find(int a) {
    int one = 0;
    int two = position - 1;
    if (position == 0)
        return -1;
    if (a < l[one] || a > l[two])
        return -1;
    if (a == l[one])
        return one;
    if (a == l[two])
        return two;
    while (two - one > 1) {
        int middle = (two - one) / 2 + one;
        if (a == l[middle])
            return middle;
        if (a > l[middle])
            one = middle;
        else
            two = middle;
    }
    return -1;
}

void wsarray::push_back(int a, int bb, double b) {
    l[position] = a;
    w[position++] = make_pair(bb, b);
}

void wsarray::freeze() {
    map<int, pair<int, double> > M;
    for (int i = 0; i < position; i++) {
        /*	//this is to sum up multiple entries
        map<int, double>::iterator itf=M.find(l[i]);
        if (itf==M.end())
            M.insert(make_pair(l[i], w[i]));
        else
            itf->second+=w[i];
        //*/
        M.insert(make_pair(l[i], make_pair(w[i].first, w[i].second)));
    }
    if (_size_ != int(M.size())) {
        delete[] l;
        l = NULL;
        delete[] w;
        w = NULL;
        _size_ = M.size();
        position = M.size();
        l = new int[_size_];
        w = new pair<int, double>[_size_];
    }
    int poi = 0;
    for (map<int, pair<int, double> >::iterator itm = M.begin(); itm != M.end(); itm++) {
        l[poi] = itm->first;
        w[poi] = itm->second;
        poi++;
    }
}
