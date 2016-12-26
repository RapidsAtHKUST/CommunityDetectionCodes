all: format network

format: Format.cc
	g++ -fpic -c Format.cc -o Format.o -std=c++11
	g++ -shared -o libFormat.so Format.o

network: Network.h TemporalNetwork.h TemporalNetwork.cc
	g++ -fpic -c LibNetwork.cc -o Network.o -std=c++11
	g++ -shared -o libNetwork.so Network.o ../Files/files.o