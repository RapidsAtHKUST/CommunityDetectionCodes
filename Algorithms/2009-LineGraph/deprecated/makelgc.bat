@echo off
if defined PRGDIR goto :runtg
call c:\bin\setenv
:runtg
@echo --- Compiling LineGraphCreator in C++ from makelgc.bat
rem see http://homepages.gac.edu/~mc38/2001J/documentation/g++.html
rem or google g++ options to find out more
rem set TIMGRAPHDIR=%PRGDIR%\networks\timgraph
set INCLUDEDIR="C\:/MinGW/include/c++/3.4.5"
del *.o
del linegraphcreator.exe
g++.exe    -c -O2 -IC\:/MinGW/include/c++/3.4.5 -o TseGraph.o TseGraph.cpp
g++.exe    -c -O2 -IC\:/MinGW/include/c++/3.4.5 -o main.o main.cpp
g++.exe     -o linegraphcreator TseGraph.o main.o
