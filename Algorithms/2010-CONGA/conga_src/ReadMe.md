#Info
##Author 

http://gregory.org/research/networks/

##Software Usage of clique_modularity.CM

 
```zsh
java -cp clique_modularity.CM.jar clique_modularity.CM <networkFile> -m <method> -c <nComm>
```


> where <networkFile> is the file containing the network, 
in "list of edges" format, <method> is the clique-finding algorithm to use ("clique_modularity.BK" or "clique_modularity.KJ"), 
and <nComm> is the number of communities required.
The program prints the modularity of the solution and outputs the solution to a file named "ClustersOutput.txt".