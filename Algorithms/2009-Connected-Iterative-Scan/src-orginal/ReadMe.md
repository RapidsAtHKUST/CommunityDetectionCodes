#Connected Iterative Scan 
Connected Iterative Scan is also known at times as Locally Optimal Sets.

##Building
To make the program, just type `make clean` followed by `make` or  `make all` in the top directory after expanding the files.

##Input
The program takes a single network as input. The format should be an egde list. 
So, an input file should have one edge on each line. Each line should have 
   
```zsh    
    [vertex_ID_1] <delimiter> [vertex_ID_2] <delimiter> [edge_weight]
```

The default delimiter is a pipe symbol ("|"), which can be changed at run time. 
Unfortunately, whitespace is not a possible delimiter at this time. 

##Running

To run, simply give the command 

```zsh
./cis -i {network_file} -o {output_file} [options]
```         

Other options are:

```zsh   
	-l Lambda value for the algorithm (default is 0)

	-dl Delimiter to use for the input file (default is `|`)

	-s Feed a seed file to the algorithm. In this file, each
	   line should be a single seed, consisting of a delimited
	   list of vertexIDs. The default ( no file ) uses each
	   vertex of the network as a seed.

	-sdl Delimiter to use for the seed file (default is `|`)

	-odl Delimiter to use for the output file (default is `|`)
``` 
	
##Output

The output file would be similar to the seed file. Each line is a single detected community. 
Each line consists of a delimited list of vertexIDs. The delimiter used is `|` by default.

##Simple Demo
In the demo file, there are some small example files. The `network.dat` file holds a small example network. 
The `network.cl` file is the results of running
  
```zsh   
./cis -i demo/network.dat -o demo/network.cl -l 0.5
```

The `network_seeded.cl` file holds the results of seeding the algorithm with the seed in `seeds.dat` 
(there`s only one - the  file name is really a misnomer).

```zsh
./cis -i demo/network.dat -o demo/network_seeded.cl -l 0.5 -s demo/seeds.dat
```
