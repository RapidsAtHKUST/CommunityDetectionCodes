all:
	cd Libraries/Files; make;
	cd Libraries/Params; make;
	cd Libraries/Network; make;
	make cis

cis: main.cc 
	g++ main.cc Libraries/Network/TemporalNetwork.cc -o cis -LLibraries/Params -lParams -LLibraries/Files -lfiles -LLibraries/Network -lNetwork -std=c++11 -g -Wall -Wno-sign-compare

clean:
	rm Libraries/Params/*.*o
	rm Libraries/Files/*.*o
	rm Libraries/Network/*.*o
	rm cis
