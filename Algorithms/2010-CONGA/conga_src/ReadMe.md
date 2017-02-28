#Info
##Author 

http://gregory.org/research/networks/


##Usage of CM (clique_modularity), 2009

> This page provides the software developed for the paper Detecting Communities in Networks by Merging Cliques.

 
```zsh
java -cp CM.jar clique_modularity.CM <networkFile> -m <method> -c <nComm>
```


> where <networkFile> is the file containing the network, 
in "list of edges" format, <method> is the clique-finding algorithm to use ("clique_modularity.algorithm.BK.BK" or "clique_modularity.algorithm.KJ.KJ"), 
and <nComm> is the number of communities required.
The program prints the modularity of the solution and outputs the solution to a file named "ClustersOutput.txt".


##Usage of CONGA/CONGO/Peacock, 2009

> The latest version of the program is available here. It implements the CONGA/CONGO/Peacock algorithms, 
as well as the GN algorithm, on which CONGA is based. 
It has also been recently extended to handle weighted networks, and fuzzy overlapping communities.

- [conga-guide.pdf](../conga-guide.pdf)


##Usage of COPRA, 2010

> This page provides the COPRA software developed for the paper Finding overlapping communities in networks by label propagation.

- [copra-guide.pdf](../copra-guide.pdf)

##Compilation via MVN

```zsh
mvn compile
```
