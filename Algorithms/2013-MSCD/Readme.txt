================================================
== Fast Multi-Scale Community Detection Tools ==
================================================

Version 0.11 (Release 3)
16th July 2012

Erwan Le Martelot
e.le-martelot@imperial.ac.uk

Implementation of the  method presented in "Fast Multi-Scale Detection of Relevant Communities" (see http://arxiv.org/abs/1204.1002) and associated set of tools.
Should you have any problem compiling or using the tool, or should you find a bug, feel free to contact me.


Version History
===============

0.11(b)	7th June 2012
	- Slight modifications in the LFK and HSLSW code
	- Addition of LFK2, a multi-threaded LFK algorithm

0.1	4th April 2012
	- Initial release


COMPILATION
===========

Four binaries can be generated:
- mscd      : the community detection tool
- conv      : file format conversion tool (for graph and community files)
- analyse   : community analysis tool (number of communities, normalised mutual information)
- speedtest : speed testing tool to test the performance of the algorithms

Under OSX:
Four Xcode project files are provided, one for each binary.

Other platforms:
To compile the binaries, type 'make'.
To remove the object files, type 'make clean'.

Notes:
- The sources are located in the 'src' directory.
- To add multithreading support to stability optimisation, uncomment in 'config.h' the line with the macro USE_MULTITHREADING. If using make, also uncomment in the makefile the required flags to enable C++11 (required for STL threads).
- To add fast matrix operations support using Armadillo (http://arma.sourceforge.net) to the stability optimisation implementation using matrices, uncomment in 'config.h' the line with the macro USE_ARMADILLO. If using make, also uncomment in the makefile the line with the include directory for Armadillo as well as the library flags.


USING THE TOOLS
===============

For all tools the available file formats are:
2 Graph formats:
- edges : edge list format ("source target weight" with weight being optional)
- net   : Pajek graph format
3 Community set formats:
- coms  : community lists (lists of nodes of community i on line i)
- clu   : Pajek community format
- dat   : Lancichinetti and Fortunato network generator benchmark file format


# mscd

Multi-scale community detection tool. In this version, 8 algorithms are available. See the relevant publications for information about the criteria.
Based on global criteria:
- Reichardt and Bornholdt’s (RB),
- Arenas et al.’s (AFG),
- Stability optimisation (SO),
- Stability optimisation using matrices (SOM),
- Ronhovde and Nussinov’s (RN).
Based on local criteria:
- Lancichinetti et al.’s (LFK),
- Huang et al.’s (HSLSW),
- Lancichinetti et al.’s multi-threaded (LFK2).

The usage is: mscd -g graph -c cext -a alg params [-d] [-w] [-p expar] [-v level] [-h]
-g: specify input graph file
-c: community file format extension
-a: community detection algorithms followed by scale parameter values (e.g.[1,2], [1:1:10])
-d: specifies that the graph is directed (default: undirected)
-p: optional extra parameters for the given algorithm
-w: write (dump) communities to disk as the algorithm runs (to free their associated memory)
-v: verbose level (default=0, max=2)
-h: display this message

To run the tool, a network, an algorithm and scales must be given.
Ex: To run mscd on network.edges with RB on scales from 10 down to 0 with a sampling step of 0.1, dumping the found communities to disk, saving communities in coms format, no verbose, type:
  mscd -g network.edges -w -c coms -v 0 -a RB [10:-0.1:0]

The optional extra parameters must be given in order as a list with ',' between values and no space. The parameters are:
- for SO:
  1. edge threshold (for walks of length>1 computed edges below this value are discarded) (default: 0)
  2. memory saving mode (for walks of length>1 the number of networks kept in memory) (default: 0, i.e. off)
    Ex: mscd -g network.edges -w -c coms -p 0.01,1 -a SO [0:0.1:5]
- for SOM:
  1. memory saving mode (for walks of length>1 the number of networks kept in memory) (default: 0, i.e. off)
    Ex: mscd -g network.edges -w -c coms -p 1 -a SOM [0:0.1:5]
- for LFK:
  1. Merging threshold beyond which communities are merged. Value must be in [0,1]. (default 0.5)
  2. When growing a community, maximum number of neighbours from the sorted list of candidate nodes that can fail to join before the failures makes the growth stop. (default: infinite / value 0)
  3. When growing a community, maximum number of iterations through all nodes during which nodes can be removed. (default: infinite / value 0)
  Ex: To merge communities with 30% of overlapping nodes
      mscd -g network.edges -w -c coms -p 0.3 -a LFK [1:-0.01:0]
- for HSLSW:
  1. Merging threshold beyond which communities are merged. Value must be in [0,1]. (default 0.5)
- for LFK2:
  1. Merging threshold beyond which communities are merged. Value must be in [0,1]. (default 0.5)
  2. Maximum number of additional threads the program can use (default: number of cores available / value 0)
  3. Maximum number of seeds to start growing communities. (default: infinite / value 0)
  4. Recursive level of seed neighbours not to use as seeds must be 0, 1 or 2. (default 1) (i.e. with 1, all the neighbours of a seed cannot be added as a seed)

For test examples, some networks are given in the 'examples' folder.


# analyse

This tool analyses the results of the mscd tool. It analyses the community files and extracts the number of communities for each scale, the normalised mutual information (NMI) between successive community sets and the NMI compared to reference communities if given.

The usage is: analyse -i pfxname ext -p params -n nb_nodes -s steps -c crefs [-g]
-i: input files prefix
-p: scale parameter list
-n: number of nodes in the graph
-s: number of successive communities to consider for NMI analysis (list of values)
-c: specifies reference communities to compare NMI with (list of file names)
-g: uses the generalised NMI instead of NMI (required for overlapping communities)
-h: display this message

For example if you analysed 'network.edges' with mscd, a list of community set (communities found at each scale value) files with the prefix 'network' have been generated. Note that the scale parameter list must be the same as use for mscd. Say the 'coms' format was used for the communities. The number of nodes in the graph is required for some internal computation.
The number of communities for NMI analysis is the number of successive community sets that are compared with each other. For instance to evaluate the averaged NMI between each community set and to the 2 following ones, and also the 4 following ones, type:
  analyse -i network coms -p [0:0.1:5] -n 10000 -s 2 4
Then NMI can be used to compare each set with reference communities. For 2 given communities, type:
  analyse -i network coms -p [0:0.1:5] -n 10000 -c network_c1.coms network_c2.coms

The analyse tool generates a file with extension 'nc' that contains the number of communities in each set. The tool generates for each NMI analysis an associated file with the list of NMI values.


# conv

This tools converts graph and community file formats.

The usage is: conv {-g|-c} -i in -o out
-g: convert a graph file
-c: convert a community file
-i: input file
-o: output file
-d: specifies that graph is directed when converting a graph (default: undirected)
-h: display this message


# speedtest

This tool performs repeated runs on an algorithm on a network to measure the average speed performance. It was design for experiment purposes and can be used to replicate them.

The usage is: speedtest -n nbtests -g graph -a alg params [-d] [-p expar] [-h]
-n: specify the number of tests to run
-g: specify input graph file
-a: community detection algorithms followed by scale parameter values (e.g.[1,2], [1:1:10])
-d: specifies that the graph is directed (default: undirected)
-p: optional extra parameters for the given algorithm
-h: display this message


NOTES
=====

This set of tools was designed as a framework where algorithms and file readers/writers can be easily integrated. A registry pattern is implemented as well as abstract classes for the algorithm, the graph and community readers and writers. Should you implement a new class that you wish to add to the framework, the '.cpp' file should contain the line:

static bool registered = toolkit::Registry<MSCDAlgorithm>::GetInstance().Register(new MSCD_HSLSW);

The template MSCDAlgorithm is the abstract algorithm class you should implement for writing an algorithm. For graph readers it is GraphReader, for graph writer GraphWriter, community reader CommunityReader, and community writer CommunityWriter. The parameter of the register function in an instance of your new class, here MSCD_HSLSW in the example.

When implementing an algorithm you must define the functions 'Run' and 'GetName'. The GetName function is the name that is used as a parameter when using the tool (e.g. RB, AFG, SO, etc). The readers must have FromFile function defined, and the writers the function ToFile. All need the GetName function to be defined where the name stands for the extension of the files they handle.
