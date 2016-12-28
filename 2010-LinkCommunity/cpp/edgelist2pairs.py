#!/usr/bin/env python
# encoding: utf-8

# edgelist2pairs.py
# Jim Bagrow
# Last Modified: 2008-12-29

"""
Copyright 2008,2009,2010 James Bagrow


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""


"""
converts a general edgelist into a ".pairs" file,
where the nodes are represented as integers numbered
sequentially from 0 (or offset)

There should be exactly two nodes on each line, separated
by a delimiter:

nodei<delimiter>nodej<newline>
nodek<delimiter>nodel<newline>
...

does not skip commented lines, or anything fancy, and
will overwrite existing outfiles!
"""

import sys, os
from optparse import OptionParser

def getDelimiter(infile, expectedLength=2):
    """get delimiter by testing first line (only) of file
    """
    first_line = open(infile,'r').readline().strip()
    if len(first_line.split()) == expectedLength: # split on whitespace!
        for element in first_line.split():
            first_line = first_line.replace(element,"")
        return first_line[:len(first_line)/(expectedLength-1)]
    for delim in ",:;|_~<>.":
        if len(first_line.split(delim)) == expectedLength:
            return delim
    sys.exit("Delimiter could not be found, aborting...")


if __name__ == '__main__':
    parser = OptionParser(usage="usage: %prog [options] infile")
    parser.add_option("-o", "--outfile",  dest="outFile",
           help="write new edgelist to OUTFILE", metavar="OUTFILE")
    parser.add_option("-d", "--delimiter", dest="outDelim", default=" ",
           help="use DELIM in outfile, default \" \" (note that passing literals \
such as tabs might require hitting ctrl+V (bash), consult your shell)", metavar="DELIM")
    parser.add_option("-n", "--index-file", dest="indexFile",
           help="record integer -> node mapping to INDEX-FILE, same delimiter as infile.  Default INFILE.int2node", metavar="INDEX-FILE")
    parser.add_option("-s", "--start-from", dest="offset", default=0, type=int,
           help="integer to start mapping nodes from, default 0")
    #parser.add_option("-u", "--undirected", dest="offset", default=0, type=int,
    #      help="undirected help!")
    parser.add_option("-x", "--extension", dest="outExt", default=False,
           help="if no OUTFILE specified, record edgelist to INFILE.EXT, default \"pairs\"", metavar="EXT")
    parser.add_option("-q", "--quiet", action="store_false", dest="verbose", default=True,
           help="don't print anything, default VERBOSE")
    (options, args) = parser.parse_args()
    if len(args) == 0:
        parser.error("no infile specified")
    elif len(args) > 1:
        parser.error("incorrect number of arguments")
    else:
        inFile = args[0]
        if not os.path.exists(inFile):
            parser.error("infile does not exist")
        if options.outExt and options.outFile:
            parser.error("cannot specify outfile and out extension")
    
    # set up filenames:
    baseInFile, extInFile = os.path.splitext(inFile)
    if options.outExt and options.outExt[0] == ".":
        options.outExt = options.outExt[1:]
    if not options.outFile:
        options.outFile = baseInFile + ".pairs"
    if not options.indexFile:
        options.indexFile = baseInFile + ".int2node"
        
    inDelim  = getDelimiter(inFile)
    outDelim = options.outDelim
    
    node2integer = {}
    integer2node = {} # doubles the memory, but ok...
    index = options.offset
    set_edges = set([])
    
    # open the edgelist and start indexing/writing:
    if options.verbose: print "Opening %s and writing integers to %s..." % (inFile, options.outFile)
    f = open(options.outFile, 'w'); write = f.write
    for line in open(inFile, 'r'):
        node_i, node_j = line.strip().split(inDelim) # only works for unweighted edgelists
        if node_i >= node_j: node_i,node_j = node_j,node_i; # UNDIRECTED!
        if (node_i,node_j) in set_edges:
            continue # remove dupes
        set_edges.add((node_i,node_j))
        try:
            index_i = node2integer[node_i]
        except KeyError:
            index_i = index
            node2integer[node_i] = index
            integer2node[index] = node_i
            index += 1
        try:
            index_j = node2integer[node_j]
        except KeyError:
            index_j = index
            node2integer[node_j] = index
            integer2node[index] = node_j
            index += 1
            
        write("%i%s%i\n" % (index_i,outDelim,index_j) )
    f.close()
    if options.verbose: print "done"
    
    # now just write the file mapping original node names to integers
    if options.verbose: print "Writing integer -> node mapping to %s..." % options.indexFile
    f = open(options.indexFile, 'w'); write = f.write
    for index,node in integer2node.iteritems():
        write( "%i%s%s\n" % (index,inDelim,node)  )
    f.close()
    if options.verbose: print "done"
    
    