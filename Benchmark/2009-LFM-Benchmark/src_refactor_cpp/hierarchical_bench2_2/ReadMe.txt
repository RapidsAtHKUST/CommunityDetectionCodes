
This program is an implementation of the algorithm described in the paper "Directed, weighted and overlapping benchmark graphs for community detection algorithms", written by Andrea Lancichinetti and Santo Fortunato. In particular, this program is to produce binary networks with overlapping nodes and hierarchies.
Each feedback is very welcome. If you have found a bug or have problems, or want to give advises, please contact us:

andrea.lancichinetti@isi.it
fortunato@isi.it


Turin, 9 February 2011
---------------------------------------------------------------





-------------------- How to compile -----------------------------------
In order to compile, type:

make

-------------------- How to run the program ---------------------------


To run the program, type:

./benchmark [FLAG] [P]


[FLAG]		[P]

1.	-N              [number of nodes]
2.	-k              [average degree]
3.	-maxk           [maximum degree]
4.	-t1             [minus exponent for the degree sequence]


5.	-minc           [minimum for the micro community sizes]
6.	-maxc           [maximum for the micro community sizes]
7.	-on             [number of overlapping nodes (micro communities only)]
8.	-om             [number of memberships of the overlapping nodes (micro communities only)]

9.	-t2             [minus exponent for the community size distribution]

10.	-minC           [minimum for the macro community size]
11.	-maxC           [maximum for the macro community size]


12.	-mu1            [mixing parameter for the macro communities (see below)]
13.	-mu2            [mixing parameter for the micro communities (see below)]
----------------------



The flags from 5. to 8. concern the micro-communities. -t2 refers to micro and macro community distribution.

-mu1 is the fraction of links between nodes belonging to different macro-communities.
-mu2 is the fraction of links between nodes belonging to the same macro but not micro community.

More specifically, each node i has k(i) stubs: mu1 * k(i) of these stubs link to nodes of different macro-communities, and mu2 * k(i) link to nodes belonging to the same macro but not micro community. So, (1 -mu2 -mu1) *k(i) stubs link to nodes belonging to the same micro and macro community. 



-------------------- Examples ---------------------------

Example1:
./hbenchmark -N 10000 -k 20 -maxk 50 -mu2 0.3 -minc 20 -maxc 50 -minC 100 -maxC 1000 -mu1 0.1
Example2:
./hbenchmark -f flags.dat



-------------------- Output ---------------------------

Please note that the community size distribution can be modified by the program to satisfy several constraints (a warning will be displayed).

The program will produce three files:

1) network.dat contains the list of edges (nodes are labelled from 1 to the number of nodes; the edges are ordered and repeated twice, i.e. source-target and target-source).

2) community_first_level.dat contains a list of the nodes and their membership for the micro-communities

3) community_second_level.dat is the same thing for the macro-communities




-------------------- Seed for the random number generator ---------------------------
 

-In the file time_seed.dat you can edit the seed which generates the random numbers. After reading, the program will increase this number by 1 (this is done to generate different networks running the program again and again). If the file is erased, it will be produced by the program again.


