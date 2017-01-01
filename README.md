# CommunityDetectionCodes
Locality-Based Overlapping Community Detection Algorithms. 
And I recommend you to use jetbrains' ide namely clion, pycharm and intellij to 
read related codes.

##Algorithms

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

##Datasets

###Small

Dataset | Nodes Number | Undirected Edge Number
--- | --- | --- | --- 
[football_edge_list.txt](Datasets/football_edge_list.txt) | 115 | 613
[karate_edge_list.txt](Datasets/karate_edge_list.txt) | 34 | 78

##Medium

Dataset |  Nodes Number | Undirected Edge Number
--- | ---  | --- 
[collaboration_edge_list](Datasets/collaboration_edge_list.txt)  | 9,875 | 25,973
[epinions_edge_list.txt](Datasets/epinions_edge_list.txt)  | 75,879 | 405,740
[facebook_edge_list.txt](Datasets/facebook_edge_list.txt)  | 4,039 | 88,234
[slashdot0811_edge_list.txt](Datasets/slashdot0811_edge_list.txt)  | 77,360 | 546,487
[slashdot0902_edge_list.txt](Datasets/slashdot0902_edge_list.txt)  | 82,168 | 582,533
[wiki_vote_edge_list.txt](Datasets/wiki_vote_edge_list.txt)  | 7,115 | 100,762


##Submodules

Submodule | Implementation Language
--- | ---
[igraph](https://github.com/igraph/igraph) | c

##Scripts

Some file processing utility python scripts are put in [Scripts](Scripts).

##Attention
These codes are all research codes, which I found through the internet. Please do not use them in production enviroment.
