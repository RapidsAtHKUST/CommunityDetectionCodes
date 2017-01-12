/**
 * @file triangleclusters_mex.cc
 * Implement clustering coefficient calculations in C++ for future
 * greedy improvements
 */

#include <mex.h>

#include <vector>

void triangle_clusters(const mxArray* mat, mxArray* cond, mxArray* cut,
    mxArray* vol, mxArray* cc, mxArray* t, mxArray* tedgeind)
{
    // we only handle symmetric matrices
    mwIndex *aj = mxGetIr(mat), *ai = mxGetJc(mat);
    mwSize n = mxGetM(mat);

    double Gvol = 0.;
    double *teind = mxGetPr(tedgeind); /* edge in triangle indicator */
    for (mwIndex v=0; v<ai[n]; ++v) { teind[v] = 0.; }

    std::vector<int> ind(n, 0);
    for (mwIndex v = 0; v < n; ++v) {
        // index the neighbors
        for (mwIndex nzi=ai[v]; nzi<ai[v+1]; ++nzi) {
            ind[aj[nzi]] = 1;
        }
        ind[v] = 1;

        double d = (double)(ai[v+1]-ai[v]);
        double curvol = 0.;
        double curcut = 0.;
        double curt = 0.;

        // do two BFS steps
        for (mwIndex nzi=ai[v]; nzi<ai[v+1]; ++nzi) {
            mwIndex w = aj[nzi];
            if (v == w) { d-=1.; continue; }
            curvol += 1.;
            Gvol += 1.;
            for (mwIndex nzi2=ai[w]; nzi2<ai[w+1]; ++nzi2) {
                mwIndex x = aj[nzi2];
                if (x == w) { continue; }
                curvol += 1.;
                if (x==v) { continue; }
                if (ind[x]) {
                    curt += 1.;
                    teind[nzi2]++;
                    teind[nzi]++;
                } else {
                    curcut += 1.;
                }
            }
        }

        // assign the output
        mxGetPr(cut)[v] = curcut;
        mxGetPr(vol)[v] = curvol;
        if (d > 1.) {
            mxGetPr(cc)[v] = curt/(d*(d-1.));
        } else {
            mxGetPr(cc)[v] = 0.;
        }
        mxGetPr(t)[v] = curt/2.;

        // clear the index
        for (mwIndex nzi=ai[v]; nzi<ai[v+1]; ++nzi) {
            ind[aj[nzi]] = 0;
        }
        ind[v] = 0;
    }

    for (mwIndex v = 0; v < n; ++v) {
        mxGetPr(cond)[v] = mxGetPr(cut)[v]/std::min(mxGetPr(vol)[v],Gvol-mxGetPr(vol)[v]);
    }
}

void mxFullAssert(int flag, const char* str) {
  if (!flag) { 
    mexErrMsgTxt(str);
  }
}

void mexFunction(int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[])
{
    mxFullAssert(nrhs == 1, "One input required.");

    const mxArray* mat = prhs[0];
    mxFullAssert(mxIsSparse(mat), "Input matrix is not sparse");
    mxFullAssert(mxGetM(mat) == mxGetN(mat), "Input matrix not square");

    mwSize n=mxGetM(mat);
    mwIndex nnz=(mxGetJc(mat))[n]; 

    mxArray* cond = mxCreateDoubleMatrix(n,1,mxREAL);
    mxArray* cut = mxCreateDoubleMatrix(n,1,mxREAL);
    mxArray* vol = mxCreateDoubleMatrix(n,1,mxREAL);
    mxArray* cc = mxCreateDoubleMatrix(n,1,mxREAL);
    mxArray* t = mxCreateDoubleMatrix(n,1,mxREAL);
    mxArray* tedgeind = mxCreateDoubleMatrix(nnz,1,mxREAL);
    
    mxFullAssert(nlhs <= 6, "Too many output arguments");
    if (nlhs > 0) { plhs[0] = cond; }
    if (nlhs > 1) { plhs[1] = cut; }
    if (nlhs > 2) { plhs[2] = vol; }
    if (nlhs > 3) { plhs[3] = cc; }
    if (nlhs > 4) { plhs[4] = t; }
    if (nlhs > 5) { plhs[5] = tedgeind; }
    
    triangle_clusters(mat, cond, cut, vol, cc, t, tedgeind);
}