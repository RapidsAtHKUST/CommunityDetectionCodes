//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.RandomAccessFile;

public class Look3 {
    private static int CACHESIZE = 1023;
    private RandomAccessFile fp;
    private int nNodes;
    private int nEdges;
    private boolean hasWeights;
    private int edgeStart;
    private int[][] cache;
    private int[] cachekeys;
    private static long cachetries = 0L;
    private static long cachehits = 0L;
    private static long cacheupds = 0L;
    private static long cachecollisions = 0L;

    public Look3(String filename) throws Exception {
        this.fp = new RandomAccessFile(filename, "r");
        this.nNodes = this.fp.readInt();
        this.nEdges = this.fp.readInt();
        this.hasWeights = this.fp.readInt() != 0;
        this.edgeStart = (this.nNodes + 4) * 4;
    }

    public void setCache(int size) {
        CACHESIZE = size;
        this.cache = new int[CACHESIZE][];
        this.cachekeys = new int[CACHESIZE];

        for(int i = 0; i < CACHESIZE; ++i) {
            this.cachekeys[i] = -1;
        }

    }

    public int[] look(int node) throws Exception {
        if(node >= this.nNodes) {
            throw new Exception("node " + node + " > nNodes " + this.nNodes);
        } else {
            int[] edges = this.cacheLook(node);
            if(edges != null) {
                return edges;
            } else {
                long nodeAddr = (long)(12 + node * 4);
                this.fp.seek(nodeAddr);
                int startEdge = this.fp.readInt();
                int endEdge = this.fp.readInt();
                if(startEdge >= 0 && endEdge >= startEdge && endEdge <= this.nEdges) {
                    int nEdgeOut = endEdge - startEdge;
                    if(this.hasWeights) {
                        nEdgeOut /= 2;
                    }

                    edges = new int[2 * nEdgeOut];
                    if(nEdgeOut > 0) {
                        long edgeAddr = (long)(this.edgeStart + startEdge * 4);
                        this.fp.seek(edgeAddr);
                        int edgeIdx = 0;

                        for(int edge = 0; edge < nEdgeOut; ++edge) {
                            edges[edgeIdx++] = this.fp.readInt();
                            if(this.hasWeights) {
                                edges[edgeIdx++] = this.fp.readInt();
                            } else {
                                edges[edgeIdx++] = 1;
                            }
                        }
                    }

                    this.addCache(node, edges);
                    return edges;
                } else {
                    throw new Exception("Bad edge node=" + node + " nodeAddr=o" + Integer.toOctalString((int)nodeAddr) + " range " + startEdge + " to " + endEdge + ", nEdges=" + this.nEdges);
                }
            }
        }
    }

    public int getNEdges() {
        return this.nEdges;
    }

    public int getNNodes() {
        return this.nNodes;
    }

    public boolean hasWeights() {
        return this.hasWeights;
    }

    private int[] cacheLook(int key) {
        ++cachetries;
        int hash = key % CACHESIZE;
        if(this.cachekeys[hash] == key) {
            ++cachehits;
            return this.cache[hash];
        } else {
            return null;
        }
    }

    public void addCache(int key, int[] entry) {
        ++cacheupds;
        int hash = key % CACHESIZE;
        this.cache[hash] = entry;
        if(this.cachekeys[hash] != -1) {
            ++cachecollisions;
        }

        this.cachekeys[hash] = key;
    }

    public static void printCacheStats() {
        System.out.println("cache hits " + cachehits + "/" + cachetries + " (" + 100.0D * (double)cachehits / (double)cachetries + "%)");
        System.out.println("cache collisions " + cachecollisions + "/" + cacheupds + " (" + 100.0D * (double)cachecollisions / (double)cacheupds + "%)");
    }

    public static void main(String[] args) {
        Look3 l = null;

        try {
            l = new Look3("/home/jlm/soci");
        } catch (Exception var10) {
            var10.printStackTrace();
        }

        while(true) {
            while(true) {
                try {
                    System.out.println("Node:");
                    BufferedReader e = new BufferedReader(new InputStreamReader(System.in));
                    String line = e.readLine();
                    int node = Integer.parseInt(line);
                    int[] temp = l.look(node);
                    int[] var9 = temp;
                    int var8 = temp.length;

                    for(int var7 = 0; var7 < var8; ++var7) {
                        int s = var9[var7];
                        System.out.println(s);
                    }

                    printCacheStats();
                } catch (Exception var11) {
                    System.out.println(var11);
                    var11.printStackTrace();
                }
            }
        }
    }
}
