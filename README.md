# CommunityDetectionCodes
Locality-Based Overlapping Community Detection Algorithms.

## Algorithms

Algorithm | Implementation Language | Dependency
--- | --- | ---
[2009-Connected-Iterative-Scan](2009-Connected-Iterative-Scan) | c++ |
[2009-EAGLE](2009-EAGLE) | c++ | igraph, boost
[2010-LinkComm](2010-LinkCommunity) | python| 
[2010-iLCD](2010-iLCD) | java | args4j, trove4j
[2011-GCE](2011-GCE) | c++ | boost
[2011-OSLOM-v2](2011-OSLOM-v2) | c++ |
[2012-DEMON](2012-DEMON) | python | networkx
[2013-Seed-Set-Expansion](2013-Seed-Set-Expansion) | c++, matlab | graclus, matlabgl
[2014-Heat-Kernel](2014-Heat-Kernel) | c++, matlab, python | pylibbvg 
[2015-LEMON](2015-LEMON) | python | pulp

## Datasets

Dataset | Size | Nodes Number | Undirected Edge Number
--- | --- | --- | --- 
[collaboration_edge_list](Datasets/collaboration_edge_list.txt) | small | nodes num:9875 | edges num:25973
[football_edge_list.txt](Datasets/football_edge_list.txt) | small | nodes num:115 | edges num:613
[karate_edge_list.txt](Datasets/karate_edge_list.txt) | small | nodes num:34 | edges num:78
[epinions_edge_list.txt](Datasets/epinions_edge_list.txt) | medium | nodes num:75879 | edges num:405740
[facebook_edge_list.txt](Datasets/facebook_edge_list.txt) | medium | nodes num:4039 | edges num:88234
[slashdot0811_edge_list.txt](Datasets/slashdot0811_edge_list.txt) | medium | nodes num:77360 | edges num:546487
[slashdot0902_edge_list.txt](Datasets/slashdot0902_edge_list.txt) | medium | nodes num:82168 | edges num:582533
[wiki_vote_edge_list.txt](Datasets/wiki_vote_edge_list.txt) | medium | nodes num:7115 | edges num:100762


## Submodules

Submodule | Implementation Language
--- | ---
[igraph](SubModules/igraph) | c

## Attention
These codes are all research codes, which I found through the internet. Please do not use them in production enviroment.
