cmake_minimum_required(VERSION 2.8)
project(heat_kernel_growth)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -g")


add_executable(hk_grow hkgrow_mex_kdd.cpp main.cpp)