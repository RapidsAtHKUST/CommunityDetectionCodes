#CommunityDetectionCodes
The repository collects and refactors some overlapping Community detection algorithms. Major content is survey, algorithms' implementations, 
submodules, benchmarks, scripts, datasets.

Recommend ide are from jetbrains, namely clion, pycharm and intellij for c++, python and java.

##Survey
Some Non-Overlapping Community Detections Algorithms could be found in [NonOverlappingCodes](NonOverlappingCodes).

A community-detection survey in [Survey](Survey), you can have a look, if interested. 

##Algorithms

In each algorithm, there is a `ReadMe.md`, which gives brief introduction of corresponding information of the algorithm and 
current refactoring status. Category information are extracted, 
based on Xie's 2013 Survey paper [Overlapping Community Detection in Networks: The State-of-the-Art
and Comparative Study](http://dl.acm.org/citation.cfm?id=2501657).

All c++ projects are built with `cmake`, java projects are built with `maven`, python projects 
are not specified how to build. 

Algorithm | Category | Language | Dependency | Status
--- | --- | --- | --- | ---
[2009-CIS](2009-Connected-Iterative-Scan) | seed expansion | c++ |  | build success
[2009-EAGLE](2009-EAGLE) | seed expansion | c++ | igraph, boost | build success
[2010-LinkComm](2010-LinkCommunity) | link partition | python|  |
[2010-iLCD](2010-iLCD) | seed expansion | java | args4j, trove4j | build success
[2010-CONGA](2010-CONGA) | dynamics | java | | jars available
[2011-GCE](2011-GCE) | seed expansion | c++ | boost | build success
[2011-OSLOM](2011-OSLOM-v2) | seed expansion | c++ |  |
[2011-MOSES](2011-MOSES) | fuzzy detection | c++ | boost | 
[2011-SLPA](2011-SLPA) | dynamics | c++, java, python | networkx, numpy |
[2012-CliquePercolation](2012-Fast-Clique-Percolation) | clique percolation | python, c++ | networkx |
[2012-DEMON](2012-DEMON) | seed expansion | python | networkx | python okay
[2013-SeedExpansion](2013-Seed-Set-Expansion) | page-rank | c++, matlab | graclus, matlabgl | 
[2014-HRGrow](2014-Heat-Kernel) | matrix-exponential | c++, matlab, python | pylibbvg | python okay
[2015-LEMON](2015-LEMON) | seed expansion | python | pulp | python okay

##Submodules(Dependencies)

Detailed information is in [SubModules](SubModules).

Submodule | Implementation Language
--- | ---
[igraph](https://github.com/igraph/igraph) | c
[Graclus](https://github.com/GraphProcessor/Graclus) | c++

##Benchmarks

Content | Detail
--- | ---
[2009-LFM-Benchmark](2009-LFM-Benchmark) | LFM Benchmark to generate five types of graphs

##Scripts

Some file processing utility python scripts are put in [Scripts](Scripts).

##DataSets

Detailed information is in [Datasets](Datasets).

##Attention
These codes are all research codes, which I found through the internet. Please do not use them in production environment.
