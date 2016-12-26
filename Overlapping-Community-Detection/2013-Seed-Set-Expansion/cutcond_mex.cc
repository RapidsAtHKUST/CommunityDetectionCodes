/**
 * @file greedyclustergrow.cc
 * Greedily grow a cluster to minimize conductance
 */

#include <vector>
#include <queue>
#include <utility> // for pair sorting

#ifdef __APPLE__
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#define tr1ns std::tr1
#else
#include <unordered_set>
#include <unordered_map>
#define __STDC_UTF_16__ 1
#define tr1ns std
#endif

#include <mex.h>


double sparse_array_volume(const mxArray* mat)
{
    mwIndex *aj = mxGetIr(mat), *ai = mxGetJc(mat);
    mwSize n = mxGetM(mat);
    double vol = 0;
    mwIndex last = aj[n+1];
    for (mwIndex i=0; i<n; ++i) {
        for (mwIndex nzi=ai[i]; nzi<ai[i+1]; ++nzi) {
            if (i != aj[i]) {
                vol += 1.;
            }
        }
    }

    return vol;
}

void cluster_conductance(const mxArray* mat, const std::vector<size_t >& cluster, 
    double Gfullvol, double* fcond, double* fcut,
    double* fvol)
{
    // we only handle symmetric matrices
    mwIndex *aj = mxGetIr(mat), *ai = mxGetJc(mat);
    mwSize n = mxGetM(mat);

    tr1ns::unordered_set<mwIndex> ind;

    // index the starting set
    for (size_t i=0; i<cluster.size(); ++i) {
        mwIndex v = cluster[i];
        ind.insert(v);    
    }

    // compute the initial cut/volume
    // along with the boundary queue
    double cut = 0;
    double vol = 0;
   
    for (size_t i=0; i<cluster.size(); ++i) {
        mwIndex v = cluster[i];
        for (mwIndex nzi=ai[v]; nzi<ai[v+1]; ++nzi) {   
            mwIndex x = aj[nzi];
            if (v == x) { 
                continue;
            }
            vol += 1.;

            if (ind.count(x) == 0) {
                cut += 1.;
            } 
        }
    }

    *fcut = cut;
    *fvol = vol;
    *fcond = cut/std::min(vol,Gfullvol-vol);
}

void copy_array_to_index_vector(const mxArray* v, std::vector<mwIndex>& vec)
{
    mxAssert(mxIsDouble(v), "array type is not double");
    size_t n = mxGetNumberOfElements(v);
    double *p = mxGetPr(v);

    vec.resize(n);
    
    for (size_t i=0; i<n; ++i) {
        double elem = p[i];
        mxAssert(elem >= 1, "Only positive integer elements allowed");
        vec[i] = (mwIndex)elem - 1;
    }
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    mxAssert(nrhs >= 2 && nrhs <= 3, "2 or 3 inputs required.");

    const mxArray* mat = prhs[0];
    const mxArray* set = prhs[1];

    mxAssert(mxIsSparse(mat), "Input matrix is not sparse");
    mxAssert(mxGetM(mat) == mxGetN(mat), "Input matrix not square");

    mxArray* cond = mxCreateDoubleMatrix(1,1,mxREAL);
    mxArray* cut = mxCreateDoubleMatrix(1,1,mxREAL);
    mxArray* vol = mxCreateDoubleMatrix(1,1,mxREAL);
    
    if (nlhs > 0) { plhs[0] = cond; }
    if (nlhs > 1) { plhs[1] = cut; }
    if (nlhs > 2) { plhs[2] = vol; }

    mxAssert(nlhs <= 3, "Too many output arguments");

    std::vector< size_t > cluster;
    copy_array_to_index_vector( set, cluster );

    double Gvol = 0;
    if (nrhs >= 3) {
        Gvol = mxGetScalar(prhs[2]);
    } else {
        Gvol = sparse_array_volume(mat);
    }

    cluster_conductance(mat, cluster, Gvol, 
        mxGetPr(cond), mxGetPr(cut), mxGetPr(vol));
}