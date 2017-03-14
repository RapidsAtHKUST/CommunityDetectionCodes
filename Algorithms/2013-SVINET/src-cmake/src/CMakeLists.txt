cmake_minimum_required(VERSION 2.8)
project(2013-svinet)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        fastamm.cc fastamm2.cc fastinit.cc linksampling.cc log.cc
        mmsbgen.cc mmsbinfer.cc mmsbinferorig.cc mmsborig.cc
        network.cc sbm.cc thread.cc
        main.cc)

find_package(Threads)

add_executable(2013-svinet ${SOURCE_FILES})
target_link_libraries(2013-svinet ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(2013-svinet gslcblas)
target_link_libraries(2013-svinet gsl)
