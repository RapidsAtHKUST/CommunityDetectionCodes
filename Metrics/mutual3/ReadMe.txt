In order to compile, just type:

make

from terminal.

To execute the program, type:

./mutual file1 file2
--------------------------------
The program expects two files in the same folder named "file1" and "file2".
In each of these two files, you have to write one partition. 
The format is like this: write the labels of the nodes belonging to the same cluster in the same line.
Foe example:


1 2 3
4 5 

means there are two clusters: in one there are nodes 1, 2, 3 and in the other there are nodes 4 and 5.
--------------------------------

The program will print their mutual information (the new one!).

Try to type

./mutual one.dat two.dat

for an example.

--------------------------------
If you are using this program for your research please cite:
Detecting the overlapping and hierarchical community structure in complex networks, 
Andrea Lancichinetti et al 2009 New J. Phys. 11 033015

