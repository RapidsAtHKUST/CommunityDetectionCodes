#######################################################################################
Acknowledgement
#######################################################################################
Howard (Hao) Lu, Washington State University, has spent considerable amount of time
and efforts in debugging, testing, and software development.

Daniel Chavarria-Miranda, Pacific Northwest National Laboratory, has provided 
substantial software and support in debugging and optimization.

Ananth Kalyanaraman, Washington State University, has provided considerable algorithmic
support.


#######################################################################################
Contact:
#######################################################################################
Mahantesh Halappanavar: hala@pnnl.gov (mhalappa@gmail.com)
Howard (Hao) Lu       : hao.lu@email.wsu.edu


#######################################################################################
Building the binaries:
#######################################################################################
1. Edit the Makefile to select the:
     a. compilers -- GCC or Intel (or others that you want), and
     b. driver files that you want to build

2. Build the binaries: $make

3. To run the drivers, see the options below.


#######################################################################################
Main binaries:
#######################################################################################

1. driverForGraphClustering : Run clustering algorithms
2. convertFileToBinary      : Store the input into binary format for fast reading
3. convertFileToEdgeList    : Store the input into edgelist format
4. driverForColoringExperiments : Run distance-1 coloring experiments


#######################################################################################
Runtime Options: Clustering
#######################################################################################

skynet214% ./driverForGraphClustering 
Problem name not specified.  Exiting.
***************************************************************************************
Basic usage: Driver <Options> FileName
***************************************************************************************
Input Options: 
***************************************************************************************
File-type  : -f <1-7>   -- default=7
File-Type  : (1) Matrix-Market  (2) DIMACS#9 (3) Pajek (each edge once) (4) Pajek (twice) 
           : (5) Metis (DIMACS#10) (6) Simple edge list twice (7) Binary format 
--------------------------------------------------------------------------------------
Strong scaling : -s         -- default=false
VF             : -v         -- default=false
Output         : -o         -- default=false
Coloring       : -c         -- default=false
--------------------------------------------------------------------------------------
Min-size       : -m <value> -- default=100000
C-threshold    : -d <value> -- default=0.0001
Threshold      : -t <value> -- default=0.000001
***************************************************************************************


#######################################################################################
Option Details:
#######################################################################################

1. "-s" : Turns scaling on -- this will run the algorithm with different number of threads starting from two to the maximum available or set through the OMP_NUM_THREADS environment variable.

2. "-v" : Turns vertex following on -- this will remove all isolated vertices and merge degree-one nodes to their only neighbor. In case of isolated edges, the vertex with larger index will merge with the vertex with smaller index. The graph is rebuilt and used subsequently. This is a one-time operation.

3. "-o" : Turns priting final clustering onto a file with "_clustInfo" suffix to the input file. The file contains the cluster-id for each vertex (the line number is the implicit id for a vertex.

4. "-c" : Turns distance-1 vertex coloring on. This option will be enabled only if the number of vertices in the graph is of a certain size specified with "-m" option (default value is 100,000 vertices).

5. "-m <integer>" : The minimum size for coloring to be enabled (default value is 100,000 vertices).

6. "-d <double>" : The threshold value with distance-1 vertex coloring (default value is 0.0001).

7. "-t <double>" : The threshold value without coloring (default value is 0.000001).


#######################################################################################
Example:
#######################################################################################

$export OMP_NUM_THREADS=64
$numactl --interleave=all skynet217% ./driverForGraphClustering karate.graph -f 5 -v -c -o -m 10

This command uses the input file specified in Metis (DIMACS#10) format, turn the coloring abd vertex following (VF) options on. 

Please browse through our paper for details on different options. 


#######################################################################################
Information from complete run
#######################################################################################

skynet217% ./driverForGraphClustering karate.graph -f 5 -v -c -o -m 10
********************************************
Input Parameters: 
********************************************
Input File: karate.graph
File type : 5
Threshold  : 1e-06
C-threshold: 0.0001
Min-size   : 10
--------------------------------------------
Coloring   : TRUE
Strong scaling : FALSE
VF         : TRUE
Output     : TRUE
********************************************
Within displayGraphCharacteristics()
*******************************************
General Graph: Characteristics :
*******************************************
Number of vertices   :  34
Number of edges      :  78
Maximum out-degree is:  17
Average out-degree is:  4.588235
Expected value of X^2:  35.647059
Variance is          :  14.595156
Standard deviation   :  3.820361
Isolated vertices    :  0 (0.00%)
Degree-one vertices  :  1 (2.94%)
Density              :  6.747405%
*******************************************
Vertex following is enabled.
Time to determine number of vertices (numNode) to fix: 0.000021
Graph will be modified -- 1 vertices need to be fixed.
Within renumberClustersContiguously()
Time to renumber clusters: 0.000026
Within buildNewGraphVF(): # of unique clusters= 33
Actual number of threads: 16 
Time to initialize: 0.000
NE_out= 77   NE_self= 1
These should match: 155 == 155
Time to count edges: 0.000
Time to build the graph: 0.000
Total time: 0.000
Graph after modifications:
Within displayGraphCharacteristics()
*******************************************
General Graph: Characteristics :
*******************************************
Number of vertices   :  33
Number of edges      :  78
Maximum out-degree is:  17
Average out-degree is:  4.696970
Expected value of X^2:  36.696970
Variance is          :  14.635445
Standard deviation   :  3.825630
Isolated vertices    :  0 (0.00%)
Degree-one vertices  :  0 (0.00%)
Density              :  7.162534%
*******************************************
Within algoDistanceOneVertexColoringOpt()
Actual number of threads: 16 (requested: 16)
Vertices: 33  Edges: 78
Within generateRandomNumbers() -- Number of threads: 16
Each thread will add 2 edges
Results from parallel coloring:
***********************************************
** Iteration : 0 
Time taken for Coloring:  0.000095 sec.
Conflicts          : 0 
Time for detection : 0.000010 sec
***********************************************
Total number of colors used: 4 
Number of conflicts overall: 0 
Number of rounds           : 1 
Total Time                 : 0.000105 sec
***********************************************
Check - SUCCESS: No conflicts exist

===============================
Phase 1
===============================
Within algoLouvainWithDistOneColoring()
Actual number of threads: 16 (requested: 16)
Time to initialize: 0.000
========================================================================================================
Itr      E_xx            A_x2           Curr-Mod         Time-1(s)       Time-2(s)        T/Itr(s)
========================================================================================================
1        56      3238    0.225920        0.000   0.000           0.000
2        84      4898    0.337196        0.000   0.000           0.000
3        86      4982    0.346565        0.000   0.000           0.000
4        86      4982    0.346565        0.000   0.000           0.000
========================================================================================================
Total time for 4 iterations is: 0.001064
========================================================================================================
Within renumberClustersContiguously()
Time to renumber clusters: 0.000003
Within buildNextLevelGraphOpt(): # of unique clusters= 7
Actual number of threads: 16 (requested: 16)
Time to initialize: 0.000
Time to count edges: 0.000
Time to build the graph: 0.000
Total time: 0.000
===============================
Phase 2
===============================
Within parallelLouvianMethod()
Actual number of threads: 16 (requested: 16)
Time to initialize: 0.000
========================================================================================================
Itr      E_xx            A_x2           Curr-Mod         Time-1(s)       Time-2(s)        T/Itr(s)
========================================================================================================
1        86      4982    0.346565        0.000   0.000           0.000
2        114     7592    0.418803        0.000   0.000           0.000
3        114     7592    0.418803        0.000   0.000           0.000
========================================================================================================
Total time for 3 iterations is: 0.000185
========================================================================================================
Within renumberClustersContiguously()
Time to renumber clusters: 0.000002
Within buildNextLevelGraphOpt(): # of unique clusters= 4
Actual number of threads: 16 (requested: 16)
Time to initialize: 0.000
Time to count edges: 0.000
Time to build the graph: 0.000
Total time: 0.000
===============================
Phase 3
===============================
Within parallelLouvianMethod()
Actual number of threads: 16 (requested: 16)
Time to initialize: 0.000
========================================================================================================
Itr      E_xx            A_x2           Curr-Mod         Time-1(s)       Time-2(s)        T/Itr(s)
========================================================================================================
1        114     7592    0.418803        0.000   0.000           0.000
2        114     7592    0.418803        0.000   0.000           0.000
========================================================================================================
Total time for 2 iterations is: 0.000107
========================================================================================================
Within renumberClustersContiguously()
Time to renumber clusters: 0.000001
********************************************
*********    Compact Summary   *************
********************************************
Total number of phases         : 3
Total number of iterations     : 9
Total time for clustering      : 0.001356
Total time for building phases : 0.000190
Total time for coloring        : 0.000105
********************************************
TOTAL TIME                     : 0.001651
********************************************
Cluster information will be stored in file: karate.graph_clustInfo
skynet218% 



