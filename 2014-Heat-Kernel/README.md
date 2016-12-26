---
title: "Heat kernel diffusion for local clustering"
layout: project
---

hkgrow: heat-kernel based local clustering
==========================================

### Kyle Kloster
### David F. Gleich

_These are research codes and may not work for you._

Download
--------

* [hkgrow.tar.gz](hkgrow.tar.gz) (2014-08-05)

Demo
--------
	compile % compile the mex files
	G = load_graph('four_clusters');
	n = size(G,1); seed = randi(n); % set seed node
	[set conductance cut volume] = hkgrow(G,seed);
    
Reusable codes
--------------

* `hkgrow_mex.cpp` C++ MEX code for computing a set of best conductance via seeded heat kernel with input graph "A" and input parameters "seeds, eps, t".
*  `hkgrow.m` Matlab script that calls hkgrow_mex, for more convenient selection of parameters.
*  `hkgrow1.m` Version of hkgrow.m used in experiments that require specific parameter choices.
*  `test_hkgrow_mex.cpp`,  `test_hkgrow.m` test the MEX code and M-script on a small example graph.


Codes from others
-----------------

* `pprgrow.m` from personal page rank clustering

Results from the paper
----------------------

NOTE: `hkgrow_mex.cpp` is the most up to date version of the algorithm and code. The file `hkgrow_mex_kdd.cpp` preserves the actual code used in producing the results given in the paper. It contained a slight error: the stopping tolerance, epsilon, was mistakenly scaled by *(e^t-1)/t* instead of *e^t*. This difference in stopping tolerance could potentially have skewed our time results to be slightly longer for HK than they should have been, and possibly skewed the resulting conductances to have been slightly lower than they should have been.


Before reproducing any figures, you must compile all mex code by running `compile.m` in the directories `hkgrow`, `hkgrow/ppr`, and `hkgrow/experiments/chirikov`.

To reproduce **Figure 1**, run:

		experiments/pathweights_figure.m

To reproduce **Figures 3** and **4**, run:

		experiments/chirikov/chirikov_example.m

To reproduce **Figure 5**, first check all required datasets are in the folder `hkgrow/data`, then generate the data by running (in this order):

		experiments/experimenthkgrow.m
		experiments/experimentpprgrow.m
		experiments/trials_ljournal.m
		experiments/trials_twitter.m
		experiments/trials_friendster.m
		experiments/trials_pprgrow.m
		
Finally, generate the plots by executing
		
		plotting/runtimes.m
		plotting/conductances.m
		plotting/plotruntimes.m
		plotting/plotconductances.m
		
To reproduce **Figure 6**, first generate the data by running

		experiments/heavytwitter.m
		experiments/pprheavytwitter.m

then produce the plots by running

		plotting/twitcondvsclust.m

To reproduce **Table 2**, first obtain the ground-truth community data for the datasets. Their sources are given in our paper. Before running experiments, the datasets need to be symmetrized using

		symmatfriend.m
		symmattwiter.m

Then, to produce the data in Table 2, read the output from running the experiments in the directory `hkgrow/experiments/ground-truth`,

		amazoncommunity.m
		dblpcommunity.m
		friendstercommunity.m
		ljournalcommunity.m
		orkutcommunity.m						
		youtubecommunity.m		
		
Obtaining Datasets
------------------

We provide references for the datasets in our paper. Please contact Kyle Kloster at `kkloste at purdue dot edu` if you have trouble obtaining or preparing any of the datasets (for example, the graphs must be symmetrized and stored in the '.smat' format.)

