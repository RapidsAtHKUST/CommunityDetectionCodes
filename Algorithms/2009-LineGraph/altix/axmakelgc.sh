module load intel-suite
icpc -xP -mcmodel=large -i-dynamic -o linegraphcreator TseGraph.cpp main.cpp
