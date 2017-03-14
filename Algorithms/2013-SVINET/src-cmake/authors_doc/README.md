SVINET implements sampling based algorithms that derive from
stochastic variational inference under the (assortative)
mixed-membership stochastic blockmodel. For details see the following
references:

Prem K. Gopalan, David M. Blei. Efficient discovery of overlapping communities 
in massive networks. To appear in the Proceedings of the National Academy of 
Sciences, 2013 (published ahead of print August 15, 2013, doi:10.1073/pnas.1221839110).

Article: http://www.pnas.org/content/early/2013/08/14/1221839110.full.pdf

SI: http://www.pnas.org/content/early/2013/08/14/1221839110/suppl/DCSupplemental

Installation
------------

Required libraries: gsl, gslblas, pthread

On Linux/Unix run

 ./configure
 make; make install
 
**SVINET has not been tested on Mac. Please run on Linux.**

The binary 'svinet' will be installed in /usr/local/bin unless a
different prefix is provided to configure. (See INSTALL.)

Tutorial
--------

1. Prepare your network data as a tab-separated file (e.g., network.txt)

2. Run the following command to find the overlapping communities: 

     svinet -file network.txt -n 10000 -k 75 -link-sampling

3. Run the following command to visualize the communities:
   
     cd output-dir; svinet -file ../network.txt  -n 10000 -k 75 -gml

In step 2, "-n" specifies the number of nodes, "-k" specifies the
number of communities and "-link-sampling" specifies the sampling
method.
   
Step 2 writes out the communities in communities.txt, the model fit in
gamma.txt and lambda.txt and the mixed-memberships in groups.txt.

Step 3 writes out a GML file (network.gml) that can be loaded into a
tool, such as Gephi, to visualize the communities. Note that each node
is colored by its most likely community in the visualization.

Some advanced tips
------------------

1. *Estimating the number of communities*

   Run the following command setting the number of communities equal
   to the number of nodes:

   svinet -file network.txt  -n 10000 -k 10000 -findk

   Estimate the number of communities using the following:

   wc -l n10000-k10000-mmsb-findk-uniform/communities.txt

   Specify this count as the number of communities in step 2 in the tutorial.

2. *Comparing communities to the ground truth*

   If you have a text file with ground truth community labels, you can
   specify it in step 2 above, to compute a normalized mutual
   information score between the true communities and the inferred
   communities. Run the command in step 2 as follows:

     svinet -file network.txt -n 10000 -k 75 -link-sampling -nmi community.txt

   Each line of the ground truth community file is a tab-separated
   list of integers including a node and its list of communities.

   node-id    list of communities the node is a member of

   Example:
   65	 17 22 43 54

   The above line says node with id 65 is a member of communities 17,
   22, 43, 54. The community ids are arbitrary. See
   detailed_readme.txt.

3. *Comparing communities to results from other methods*

   We recommend running svinet with two settings of the
   **link threshold** as follows:

     svinet -file network.txt -n 10000 -k 75 -link-sampling -link-thresh 0.5

     svinet -file network.txt -n 10000 -k 75 -link-sampling -link-thresh 0.9

   For further details, see detailed_readme.txt or please email the authors.

4. *Set an alternate prior on community strengths*

   The "fromdata" option sets a prior based on the network data on the
   community strengths and may result in better communities. By
   default, the prior on community strenths is a uniform distribution.

     svinet -file network.txt -n 10000 -k 75 -link-sampling -eta-type fromdata

5. *Other sampling methods*

   See detailed_readme.txt.


   


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/premgopalan/svinet/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

