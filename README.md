#Community Detection Survey 
##Introduction
The repository collects and refactors some overlapping Community detection algorithms. Major content is survey, algorithms' implementations, 
graph input benchmarks, submodules, scripts.

Recommend ide are from jetbrains, namely clion, pycharm and intellij for c++, python and java.

Some Non-Overlapping Community Detections Algorithms could be found in [NonOverlappingCodes](NonOverlappingCodes).

A community-detection survey in [Survey](Survey), you can have a look, if interested. 

##Graph Benchmarks

###Synthetic Tool

Content | Detail | Status
--- | --- | ---
[2009-LFR-Benchmark](Benchmark/2009-LFR-Benchmark) | LFR Benchmark to generate five types of graphs | build success, some files unused

###Real-World DataSets(Edge List)

Detailed information is in [Datasets](Datasets).

##Quality-Evaluation Metrics
###Without-Ground-Truth

Evaluation Metric Name | Implementation | Heuristic
--- | --- | ---
Link-Belonging-Based Modularity | [link_belong_modularity.py](Metrics/metrics/link_belong_modularity.py) | compare to random graph

###With-Ground-Truth

Evaluation Metric Name | Implementation | Heuristic
--- | --- | ---
Overlap-NMI | [overlap_nmi.py](Metrics/metrics/overlap_nmi.py) | info-theory entropy-measure
Omega-Idx | [omega_idx.py](Metrics/metrics/omega_idx.py) | unadjusted compared to expected


##Algorithms

In each algorithm, there is a `ReadMe.md`, which gives brief introduction of corresponding information of the algorithm and 
current refactoring status. Category information are extracted, 
based on Xie's 2013 Survey paper [Overlapping Community Detection in Networks: The State-of-the-Art
and Comparative Study](http://dl.acm.org/citation.cfm?id=2501657).

All c++ projects are built with `cmake`, java projects are built with `maven`, python projects 
are not specified how to build. 

Algorithm | Category | Language | Dependency | Status
--- | --- | --- | --- | ---
[2008-CPM](Algorithms/2008-CliquePercolation) | clique percolation | c++, python | | 
[2009-CIS](Algorithms/2009-Connected-Iterative-Scan) | seed expansion | c++ |  | build success
[2009-EAGLE](Algorithms/2009-EAGLE) | seed expansion | c++ | igraph, boost | build success
[2010-LinkComm](Algorithms/2010-LinkCommunity) | link partition | python| optparse | python okay
[2010-iLCD](Algorithms/2010-iLCD) | seed expansion | java | args4j, trove4j | build success
[2010-CONGA](Algorithms/2010-CONGA) | dynamics | java | | build success
[2010-TopGC](Algorithms/2010-TopGC) | stochastic inference | java | | build success
[2011-GCE](Algorithms/2011-GCE) | seed expansion | c++ | boost | build success
[2011-OSLOM](Algorithms/2011-OSLOM-v2) | seed expansion | c++ |  |
[2011-MOSES](Algorithms/2011-MOSES) | fuzzy detection | c++ | boost | build success
[2011-SLPA](Algorithms/2011-SLPA) | dynamics | c++, java, python | networkx, numpy | build success for java
[2012-FastCPM](Algorithms/2012-Fast-Clique-Percolation) | clique percolation | python, c++ | networkx | build success
[2012-ParCPM](Algorithms/2012-CPMOnSteroids) | clique percolation | c | igraph | build success
[2012-DEMON](Algorithms/2012-DEMON) | seed expansion | python | networkx | python okay
[2013-SVINET](Algorithms/2013-SVINET) | stochastic inference | c++ | gsl, pthread | build success
[2013-SeedExpansion](Algorithms/2013-Seed-Set-Expansion) | page-rank | c++, matlab | graclus, matlabgl | 
[2014-HRGrow](Algorithms/2014-Heat-Kernel) | matrix-exponential | c++, matlab, python | pylibbvg | python okay
[2015-LEMON](Algorithms/2015-LEMON) | seed expansion | python | pulp | python okay

##Submodules(Dependencies)

Detailed information is in [SubModules](SubModules).

Submodule | Implementation Language | Detail
--- | --- | ---
[igraph](https://github.com/igraph/igraph) | c | also provide python wrapper, graph utilities 
[graclus](https://github.com/GraphProcessor/Graclus) | c++ | graph partition algorithm
[lcelib](https://github.com/CxAalto/lcelib) | c++ | graph utilities by [CxAalto](http://complex.cs.aalto.fi/)

##Scripts

Some file processing utility python scripts are put in [Scripts](Scripts).

##Attention
These codes are all research codes, which I found through the internet. Please do not use them in production environment.
