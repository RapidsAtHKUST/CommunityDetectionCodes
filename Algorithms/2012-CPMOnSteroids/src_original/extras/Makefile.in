CC = gcc
CFLAGS =-O2 -Wall -D_FILE_OFFSET_BITS=64
LDFLAGS =

IGRAPH_INCDIR_1 = /usr/local/include/igraph 
IGRAPH_INCDIR_2 = /usr/include/igraph 
IGRAPH_INCDIR_3 = /opt/local/include/igraph
IGRAPH_LIB = igraph

all : maximal_cliques

maximal_cliques: maximal_cliques.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(IGRAPH_INCDIR_1) -I$(IGRAPH_INCDIR_2) -I$(IGRAPH_INCDIR_3) $< -ligraph -o $@	

clean:
	rm -rf maximal_cliques
