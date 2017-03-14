#if !defined(WSARRAY_INCLUDED)
#define WSARRAY_INCLUDED
// this class is a container of int and double;
// it needs a preallocation when constructing the class;
// then you need to use the function push_back() to insert the numbers; this is the only way
// once you did, you can freeze the numbers; multiple entries are neglected; the preallocation can be reduced; you cannot use push_back anymore
// find returns the position of the integer you are looking for;
// posweightof returns a pair position-weight
// to access the integers use l[] and w[]
#include <map>
#include <iostream>

using namespace std;

class wsarray {
public:
    wsarray(int a);

    ~wsarray();

    int find(int);

    pair<int, pair<int, double> > posweightof(int x);

    int size() { return position; };

    void freeze();

    void push_back(int, int, double);

    pair<int, double> *w;
    int *l;

private:
    int _size_;
    int position;
};

void prints(wsarray &a);

void prints(wsarray &a, ostream &pout);

void prints(wsarray *a, ostream &pout);

void prints(wsarray *a);

#endif
