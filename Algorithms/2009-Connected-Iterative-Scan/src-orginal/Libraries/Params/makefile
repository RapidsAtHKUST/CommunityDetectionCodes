params: Parameters.h Parameters.cc
	g++ -fpic -c Parameters.cc -o Params.o -std=c++11
	g++ -shared Params.o ../Files/files.o -o libParams.so 