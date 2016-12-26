# LEMON
The example implements a large scale overlapping community detection method 
based on **local expansion via minimum one norm**. The program adopts a local expansion method in order to identify the community members from a few exemplary seed members. The algorithm finds the community by seeking a sparse vector in the span of the local spectra such that the seeds are in its support. LEMON can achieve the highest detection accuracy among state-of-the-art proposals. The running time depends on the size of the community rather than that of the entire graph. 

Here we use Amazon dataset (obtained from [SNAP](http://snap.stanford.edu/data/com-Amazon.html) website) as an illustration. You may switch to other datasets with corresponding file format as well. Note that some parameters might need to be adjusted accordingly based on the properties of network under test.


```
@inproceedings{li2015uncovering,
  title={Uncovering the small community structure in large networks: a local spectral approach},
  author={Li, Yixuan and He, Kun and Bindel, David and Hopcroft, John E},
  booktitle={Proceedings of the 24th international conference on world wide web},
  pages={658--668},
  year={2015},
  organization={International World Wide Web Conferences Steering Committee}
}
```

Requirements
------------
* numpy
* scipy
* pulp ([https://pypi.python.org/pypi/PuLP](https://pypi.python.org/pypi/PuLP))

(may have to be independently installed) 

Dataset Information
--------
* amazon dataset (available at [http://snap.stanford.edu/data/com-Amazon.html](http://snap.stanford.edu/data/com-Amazon.html))
* ``936`` communities with ground truth size ``>= 20``.
* nodes are products; edges are co-purchase relationship
* nodes: ``334863``, edges: ``925872``
* maximum membership per node: ``49``
* average community size: ``39``

Usage
-----

######Example Usage######

    $cd LEMON
    $python LEMON.py -f ../example/amazon/graph -g  ../example/amazon/community --sd ../example/amazon/seed --out output.txt

######Command Options######

**-d**: delimiter of input graph and community files [*default: space*]

**-f**:  input network file [*default*: ``example/amazon/graph``]

  The format of a graph is edgelist, e.g::
  
        1 2
        1 3
        1 4
        ...
**-g**:  input ground truth community file [*default*: ``example/amazon/community``]

    The format of a ground truth community is a space delimited line of node IDs , e.g:
  
        1 4 8 14 20 21 22                         # community 1
        2 5 3 6 7 15 16 17 18 19                  # community 2
        9 10 11 12 13 23                          # community 3

**--sd**: initial seed set input file [*default*:``example/amazon/seed``]

    The format of seed set is a single line of space delimited node IDs, e.g:
    
        2 5

**--out**: output file path [*default*: ``output.txt``]

    The output includes the detected community and the similarity between the detected community and ground truth community 
    (quantified by F1 score), e.g:

        # detected community:
        [2,5,3,6,7,15,16,17,18,19]
        # F1 score: 1.0

**-c**: minimum community size [*default*: 20]

**-C**: maximum community size [*default*: 100]

**-e**: expand step [*default*: 6]


######To View Full Command List######

The full list of command line options is available with ``$python LEMON.py --help``

