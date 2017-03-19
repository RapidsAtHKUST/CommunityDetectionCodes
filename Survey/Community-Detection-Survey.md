# Community Detection Related (Mainly Social Network)

## Basic Concepts

- Non-Overlapping-Empirical-Study, [Community Detection in Social Networks: An Indepth Benchmarking Study with a Procedure Oriented Framework](http://delivery.acm.org/10.1145/2800000/2794370/p998-wang.pdf) (Evaluation Framework, VLDB 2015)  
- Non-Overlapping-Benchmark, [Exploring the limits of community detection strategies in complex networks](http://www.nature.com/articles/srep02216?message-global=remove&WT.ec_id=SREP-639-20130801) (Summarization(Test on 17 algorithms), Scientific Report 2013)  
- Non-Overlapping-Benchmark, [Closed benchmarks for network community structure characterization](http://journals.aps.org/pre/pdf/10.1103/PhysRevE.85.026109) (Closed Benchmark(Mention RC benchmark), Physics Review 2012)  
- LFR-Benchmark, [Benchmark graphs for testing community detection algorithms](http://journals.aps.org/pre/pdf/10.1103/PhysRevE.78.046110) (LFR Graph Benchmark, Physics Review 2008)  

- Benchmarks  
	- 2002, GN benchmark(not appropriate for million to billion vertices)  
	- 2008, LFR graph benchmark  
	- 2010, Relaxed Caveman benchmark  
	- 2012, closed benchmark(mention RC benchmark)  
	- 2013, closed plus visualization for intermediate results  

- Social Network Input Graph Features  
	- scale-free(power law), small-world(small diameter)  

- Algorithm Applicability  
	- Density sensitivity  
	- Overlapping sensitivity  
	- Outliers sensitivity  
	- Different graph-features(e.g, DBLP , Twitter, Facebook, LiveJournal) sensitivity  

- Result-Quality Evaluation Metrics  
	- With ground truth : cross common fraction, jaccard index, normalized mutual information  
	- W/O ground truth : clustering coefficient, strength(intensity of detected communitites), modularity(compares result with a randomized one)  
	- Evaluation methods: 1) use closed benchmarks, 2) visualize hierarchically clustering results  

## Possible Research Points
1. Difference between directed graph and undirected graph
1. Evaluate prior detection methods(non-overlapping, overlapping, hierarchical) on metric and scalability (**WWW, ASONAM**)
1. Implement useful algorithms on **GraphX(Scala), PowerGraph(C++), Giraph(Java)** and estimate efficiency and analyze hwo to transparent sequential one into parallel environment (**SC, IPDPS**)  
1. Implement algorithms with GPU/MIC acceleration (**My Imagination**)  
1. Study fast local community search/query (**VLDB, SIGMOD**)  
1. Study Sampling large scale graph for approximated structure (**KDD**)  
1. Study index technique to help combine several algorithms for social network analysis (**My Imagination**)  
1. Visulize algorithm execuation procedures(i.e. how community form especially in hierarchical algorithms) (**My Imagination**)  

## Related Researcher
- **[Mark Newman](http://www-personal.umich.edu/~mejn/) (Michigan, Pioneer)** 
- **[Andrea Lancichinetti](https://sites.google.com/site/andrealancichinetti/) (Umeå University, LFR Benchmark)**  
- **[Peter Bickel](http://www.stat.berkeley.edu/~bickel/index.html)	(Berkeley, Spectral Algorithm, Model Selection)**  
- **[Jure Leskovec](http://cs.stanford.edu/people/jure/) (Stanford, Snap Software)**  
- **[Carlo Ratti](http://senseable.mit.edu/community_detection/) (MIT, Senseable Research)**  
- **[Jie Tang](http://keg.cs.tsinghua.edu.cn/jietang/) (Tsinghua)**  
- **[Jianyong Wang](http://dbgroup.cs.tsinghua.edu.cn/wangjy/) (Tsinghua)**  

## Good Survey
- **[Overall Survey](http://lab41.github.io/survey-community-detection/)** (Home Page : http://lab41.github.io/Circulo)  

## Prior Famous Algorithms & Evaluation Benchmark
- Reference  
	- (Refer to [Reihaneh Rabbany](https://scholar.google.com.hk/citations?user=Foh_c-QAAAAJ&hl=zh-CN&oi=ao)'s Github: https://github.com/rabbanyk/CommunityEvaluation)  
	- (Refer to [Andrea Lancichinetti](https://sites.google.com/site/andrealancichinetti/)'s Implementation)  

- Algorithms  
	- 2004, **Fast Modularity**, *[Aaron Clauset]*, Finding community structure in very large networks  
	- 2005, **Walker Trap**, *[Pascal Pons]*, Computing Communities in Large Networks Using Random Walks  
	- 2005, **Spectral Algorithm**, *[Luca Donetti]*, Improved spectral algorithm for the detection of network communities  
	- 2007, **Label Propogation**, *[Usha Nandini Raghavan]*, Near linear time algorithm to detect community structures in large-scale networks  
	- 2007, **Modularity Opt(Simulated Annealing)**, *[Marta Sales-Pardo]*, Extracting the hierarchical organization of complex systems  
	- 2008, **Louvain**, *[Vincent D Blondel]*, Fast unfolding of communities in large networks  
	- 2008, **Infomap**, *[Martin Rosvall]*, Maps of random walks on complex networks reveal community structure  
	- 2009, **Potts model**, *[Peter Ronhovde]*, Multiresolution community detection for megascale networks by information-based replica correlations  
	- 2010, **Link-Plus**, *[Yong-Yeol Ahn]*, Link communities reveal multiscale complexity in networks  
	- 2010, **Greedy Clique Expansion**, *[Conrad Lee]*, Detecting Highly Overlapping Community Structure by Greedy Clique Expansion  
	- 2010, **MOSES**, *[Aaron McDaid]*, Detecting higly overlapping communities with Model-based Overlapping Seed Expectation  
	- 2010, **Potts Model**, *[Peter Ronhovde]*, Local resolution-limit-free Potts model for community detection  
	- 2010, **COPRA(Label Propogation)**, *[Steve Gregory]*, Finding overlapping communities in networks by label propagation  
	- 2010, **Top Leader**. *[Reihaneh Rabbany Khorasgani]*, Top Leaders Community Detection Approach in Information Networks  
	- 2011, **(State-of-Art) OSOLOM**, *[Andrea Lancichinetti]*, Finding Statistically Significant Communities in Networks  
	- 2011, **Multi-Level-Infomap**, *[Martin Rosvall]*, Multilevel Compression of Random Walks on Networks Reveals Hierarchical Organization in Large Integrated Systems  
	- 2012, **Consensus Clustering**, *[Andrea Lancichinetti]*, Consensus clustering in complex networks  
	- 2012, **Community-Affiliation Graph Model**, *[Jaewon Yang]*, Community-Affiliation Graph Model for Overlapping Network Community Detection  
	- 2013, **(State-of-Art) Large Scale CAG, BigClam**, *[Jaewon Yang]*, Overlapping Community Detection at Scale: A Nonnegative Matrix Factorization Approach  
	- 2014, **Combo Optimization**, *[Stanislav Sobolevsky]*, General optimization technique for high-quality community detection in complex networks  

- Scalable Algorithms
	- 2009, **Propinquity dynamics**, *[Jianyong Wang]*, Parallel Community Detection on Large Networks with Propinquity Dynamics  
	- 2013, **Ensemble**, *[Michael Ovelg¨onne]*, Distributed Community Detection in Web-Scale Networks  
	- 2013, **Fast Algorithm for Modularity-based**, *[Hiroaki Shiokawa]*, Fast Algorithm for Modularity-based Graph Clustering  
	- 2014, **SCD**, *[Arnau Prat-Pérez]*, High Quality, Scalable and Parallel Community Detection for Large Real Graphs
	- 2014, **RelaxMap**,*[Seung-Hee Bae]*, Scalable Flow-Based Community Detection for Large-Scale Network Analysis  
	- 2015, **GossipMap**, *[Seung-Hee Bae]*, GossipMap: A Distributed Community Detection Algorithm for Billion-Edge Directed Graphs  

- Missing Algorithms(Need to find source codes)
	- **Maximal k-Mutual-Friends**, *[F.Zhao]*, Large scale cohesive subgraphs discovery for social network visual analysis(VLDB 2012)  
	- **Matrix Blocking Dense Subgraph Extract**, *[J.Chen]*, Dense subgraph extraction with application to community detection (TKDE 2012)  
	- **Skeleton Clustering**, *[D.Bortner]*, Progressive clustering of networks using structure-connected order of traversal(ICDE 2010), *[J.Huang]*, Revealing density-based clustering structure from the core-connected tree of a network(TKDE 2013)  

- Evaluations  
	- 2009, **NMI for overlapping Community Detection**, *[Andrea Lancichinetti]*, Detecting the overlapping and hierarchical community structure in complex networks  
	- 2010, **Snap empirical comparison**, *[Jure Leskovec]*, Empirical Comparison of Algorithms for Network Community Detection  
	- 2011, **NMI for overlapping Community Detection**, *[Aaron F. McDaid]*, Normalized Mutual Information to evaluate overlapping community finding algorithms  
	- 2012, **WCC(a metric based on triangle structures in a community)**, *[Arnau Prat-Pérez]*, Shaping Communities out of Triangles  
	- 2013, **Summarize LFR and RC benchmark**, *[Rodrigo Aldecoa]*, Exploring the limits of community detection strategies in complex networks (**Notes: Evaluate 17 algorithms**)  
	- 2013, **Effective Evaluation**, *[Conrad Lee]*, Community detection: effective evaluation on large social networks  
	- 2015, **rNMI**, *[Pan Zhang]*, Evaluating accuracy of community detection using the relative normalized mutual information  

- Optimization Techniques
	- 2014, **General Optimization**, *[Stanislav Sobolevsky]*, General optimization technique for high-quality community detection in complex networks  

## Codes
- Tools
    - General Library
        - **[Some Recommendations From CppReference](http://en.cppreference.com/w/cpp/links/libs)** (Boost.Graph, LEMON, OGDF, NGraph)  
        - **Networkx** has implemented **[many graph algorithms](http://networkx.github.io/documentation/networkx-1.10/reference/algorithms.html)**  
        - **[Snap](https://github.com/snap-stanford/snap)** (Infomap, Fast Newman, BIGCLAM, CESNA, CoDA, RoIX)  
        - **[igraph](https://github.com/igraph/igraph)** (Infomap, WalkTrap, Leading Eginvector)  
            - **Implementations**: edge_betweenness, leading_eigenvector, spinglass, fastgreedy , leading_eigenvector_naive, walktrap, infomap, multilevel, label_propagation, optimal_modularity  
            - **Usage Explaination**: https://github.com/rabbanyk/CommunityEvaluation/blob/master/src/algorithms/communityMining/external_methods/iGraph/communityMinerInterface.py  
    
    - BLAS
        - **[Boost BLAS](http://www.boost.org/doc/libs/1_60_0/libs/numeric/ublas/doc/index.html)** (Boost Library)  
        - **[Egien](http://eigen.tuxfamily.org/index.php?title=Main_Page)** (Cpp template library for linear algebra related algorithms)  
        - **[ArmAdillo](http://arma.sourceforge.net/)**(Primarily developed at Data61 (Australia) by Conrad Sanderson)  
- C/C++:
	- Benchmark  
        - [2009 Lancichi Benchmark](https://sites.google.com/site/santofortunato/inthepress2) (from GoogleSite)  
        - [2009 Lancichi Benchmark ReImpl](https://github.com/CarloNicolini/paco) (from Github)
        - [2013 Conrad Lee Benchmark](https://github.com/conradlee/network-community-benchmark) (from Github)  
    - Algorithm  
    	- [???? Multi-level Modularity Based on Betweenness Centrality](https://github.com/sidrakesh/Community-Detection-Betweenness) (from Github)  
    	- [2004 Clauset](https://github.com/ddvlamin/CommunityDetectionC) (from Github)  
		- [2005 Simulated Annealing](http://seeslab.info/downloads/network-c-libraries-rgraph/) (from RGraph Website)  
    	- [2010 linkcomm](https://github.com/bagrow/linkcomm) (from Github)  
    	- [2010 GCE](https://sites.google.com/site/greedycliqueexpansion/) (from GoogleSite)  
    	- [2011 SLPA](https://sites.google.com/site/communitydetectionslpa/) (from GoogleSite)  
    	- [2011 Dense Subgraph Extraction](https://github.com/sranshous/Graph-Community-Detection) (from Github)  
    	- [2011 Marvelot](http://www.elemartelot.org/index.php/programming/cd-code) (from Website)  
    	- [2012 Combo](http://senseable.mit.edu/community_detection/) (from MIT Senseable Lab's Homepage), [Combo And Others](https://github.com/sina-khorami/AI-community-detection) (from Github)  
    	- [2012 K-clique Percolation](https://sites.google.com/site/cliqueperccomp/) (from GoogleSite)  
    	- [2014 SCD](https://github.com/DAMA-UPC/SCD) (from Github), [SCD GPU implementation](https://github.com/Het-SCD/Het-SCD) (from Github)  
    	- [2014 Heterogeneous SCD Impl](https://github.com/stijnh/Par-CD) (from Github)  
    	- [2015 GossipMap](https://github.com/uwescience/GossipMap) (from Github), [2013 RelaxMap](https://github.com/uwescience/RelaxMap) (from Github)  

- Java:
    - [2013 Impl of Algorithms On Giraph, GraphX](https://github.com/Sotera/distributed-graph-analytics) (BSP impl,e.g., High Betweenness Set Extraction, Weakly Connected Components, Page Rank, Leaf Compression, and Louvain Modularity) (from Github)  
    - [2012 Reihaneh Rabbany, Top-leader Author, Compare differnt algorithms](https://github.com/rabbanyk/CommunityEvaluation) (5th-year Phd Student, Give integration of many sequential implementations) (from Github)  
    - [2012 An Application of Network Analysis Platform](https://github.com/sisusisu/noesis)(Noesis) (from Github)  

- Python:
	- [2013 Survey Circulo](https://github.com/Lab41/Circulo) (With Some Implementations and Refer to SNAP)  
    - [2007 Label Propagation](https://github.com/liyanghua/Label-Propagation) (Refer to Pyshics 2007) (from Github)  

## Recent Papers(With Codes)
- Algorithms
	- (**GossipMap, Use InfoTheory**) [GossipMap: a distributed community detection algorithm for billion-edge directed graphs](http://dl.acm.org/citation.cfm?id=2807668) (SC 2015)  
	- (**Scalable Community Detection**) [High quality, scalable and parallel community detection for large real graphs](http://www.dama.upc.edu/publications/fp546prat.pdf) (WWW 2014)  
	- (**Marvelot**) [Fast Multi-Scale Detection of Relevant Communities in Large-Scale Networks](http://comjnl.oxfordjournals.org/content/early/2013/01/22/comjnl.bxt002.full.pdf+html)(The Computer Journal 2013)  
	- (**SLPA**) [Towards Linear Time Overlapping Community Detection in Social Networks](http://arxiv.org/pdf/1202.2465.pdf) (Advances in Knowledge Discovery and Data Mining 2012)  
	- (**Dense Subgraph Extraction**) [Dense Subgraph Extraction with Applicationto Community Detection](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5677532) (TKDE 2012)  

- Evaluation
	- [Benchmarks for testing community detection algorithms on directed and weighted graphs with overlapping communities](http://journals.aps.org/pre/pdf/10.1103/PhysRevE.80.016118) (Physics Review 2009)  
	- [Community detection: effective evaluation on large social networks](http://comnet.oxfordjournals.org/content/2/1/19.full.pdf+html) (Journal of Complex Networks 2014)  

- Optimization Techniques
	- [General optimization technique for high-quality community detection in complex networks](http://journals.aps.org/pre/pdf/10.1103/PhysRevE.90.012811) (Physics Review 2014)  









