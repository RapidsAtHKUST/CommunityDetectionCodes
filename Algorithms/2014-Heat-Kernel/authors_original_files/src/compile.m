if ismac
mex -O -largeArrayDims hkgrow_mex.cpp
mex -O -largeArrayDims hkgrow_mex_kdd.cpp
else
mex -O CXXFLAGS="\$CXXFLAGS -std=c++0x" -largeArrayDims hkgrow_mex.cpp
mex -O CXXFLAGS="\$CXXFLAGS -std=c++0x" -largeArrayDims hkgrow_mex_kdd.cpp
end