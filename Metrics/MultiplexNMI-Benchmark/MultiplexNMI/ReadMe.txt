
Compile:
g++ nmi_multiplex.cpp -o nmi -O3 -Wall


Run:
./nmi file1 file2
--------------------------------

file1 and file2 are in the format:
layerId NodeId CommunityId


THE MULTIPLEX NMI IS PRINTED AFTER THE STRING: "Multiplex NMI:"

Examples:
./nmi clu1 clu2
