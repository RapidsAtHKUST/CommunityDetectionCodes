cd ./GraclusSeeds/graclus1.2/matlab
graclus_compile_script
cd ../../..
mex -largeArrayDims -O triangleclusters_mex.cc
mex -O CXXFLAGS="\$CXXFLAGS -std=c++0x" -largeArrayDims pprgrow_mex.cc
mex -O CXXFLAGS="\$CXXFLAGS -std=c++0x" -largeArrayDims vpprgrow_mex.cc
mex -O CXXFLAGS="\$CXXFLAGS -std=c++0x" -largeArrayDims cutcond_mex.cc 