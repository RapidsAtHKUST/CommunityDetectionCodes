//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Random;

public class FindMostProm implements Runnable {
    int start;
    int end;
    int threadNum;
    MinHeap minHeap;
    int numNodes;
    int importantNeighbors;
    long[] a;
    long[] b;
    int[] p;
    Integer[][] ordering;
    Integer[][] LSHordering;
    int m;
    int numWords;
    int wordLength;
    boolean unweighted;
    int trials;
    int minClusterSize;
    int totalToDo;
    String graphFileName;
    Look3 look;
    int mostProm;
    int cacheHits = 0;
    int cacheMisses = 0;
    int mCacheHits = 0;
    int mCacheMisses = 0;
    long time_2 = 0L;
    long time_3 = 0L;
    long time_4 = 0L;
    Hashtable<Integer, HashSet<String>> cache = new Hashtable();
    Hashtable<Integer, HoldInt> cacheM = new Hashtable();
    Hashtable<Integer, HashSet<String>> cache2 = new Hashtable();
    Hashtable<Integer, HoldInt> cacheM2 = new Hashtable();

    public FindMostProm(int start, int end, int threadNum, String graphFileName, MinHeap minHeap, int numNodes, int importantNeighbors, int m, int numWords, int wordLength, boolean unweighted, int trials, int minClusterSize, int totalToDo, int mostProm) {
        this.start = start;
        this.end = end;
        this.threadNum = threadNum;
        this.minHeap = minHeap;
        this.numNodes = numNodes;
        this.importantNeighbors = importantNeighbors;
        this.m = m;
        this.numWords = numWords;
        this.wordLength = wordLength;
        this.unweighted = unweighted;
        this.trials = trials;
        this.minClusterSize = minClusterSize;
        this.totalToDo = totalToDo;
        this.graphFileName = graphFileName;
        this.mostProm = mostProm;

        try {
            this.look = new Look3(graphFileName);
            this.look.setCache(mostProm);
        } catch (Exception var17) {
            System.out.println(var17);
        }

    }

    public void run() {
        long time2 = System.currentTimeMillis();

        try {
            this.setLSH2();
            double e = 0.0D;
            boolean lookCalled = false;
            boolean hashedNodeNum = false;
            int nodesFinished = 0;

            for(int currentNode = this.start; currentNode < this.end && currentNode < this.look.getNNodes(); ++currentNode) {
                long time_temp = System.currentTimeMillis();
                if(nodesFinished % 5000 == 0) {
                    System.out.println("Thread " + this.threadNum + " " + nodesFinished + " / " + this.totalToDo);
                }

                ++nodesFinished;
                ArrayList neighbors = new ArrayList();
                HashSet topNeighbors = new HashSet();
                double largest = 0.0D;
                int[] neighArray = this.look.look(currentNode);

                int neighborsNeighbors;
                double score;
                for(neighborsNeighbors = 0; neighborsNeighbors + 1 < neighArray.length; neighborsNeighbors += 2) {
                    int iter = neighArray[neighborsNeighbors];
                    score = Double.valueOf((double)neighArray[neighborsNeighbors + 1]).doubleValue();
                    neighbors.add(new TempDouble2(iter, score));
                    if(this.look.hasWeights() && score > largest) {
                        largest = score;
                    }
                }

                Collections.sort(neighbors, Collections.reverseOrder());

                for(neighborsNeighbors = 0; neighborsNeighbors < neighbors.size() && this.look.hasWeights(); ++neighborsNeighbors) {
                    ((TempDouble2)neighbors.get(neighborsNeighbors)).weight /= largest;
                }

                for(neighborsNeighbors = 0; neighborsNeighbors < this.importantNeighbors && neighbors.size() > neighborsNeighbors; ++neighborsNeighbors) {
                    topNeighbors.add(Integer.valueOf(((TempDouble2)neighbors.get(neighborsNeighbors)).nodeNum));
                }

                Hashtable var24 = new Hashtable();
                Iterator var23 = topNeighbors.iterator();

                while(var23.hasNext()) {
                    int var25 = ((Integer)var23.next()).intValue();
                    var24.put(Integer.valueOf(var25), new ArrayList());
                    int[] results = this.look.look(var25);
                    largest = 0.0D;

                    for(int tt = 0; tt + 1 < results.length; tt += 2) {
                        ((ArrayList)var24.get(Integer.valueOf(var25))).add(new TempDouble2(results[tt], Double.valueOf((double)results[tt + 1]).doubleValue()));
                        if(this.look.hasWeights() && (double)results[tt + 1] > largest) {
                            largest = (double)results[tt + 1];
                        }
                    }

                    ArrayList var26 = (ArrayList)var24.get(Integer.valueOf(var25));

                    for(int k = 0; this.look.hasWeights() && k < var26.size(); ++k) {
                        ((TempDouble2)var26.get(k)).weight /= largest;
                    }
                }

                time_temp = System.currentTimeMillis();
                score = this.getScore2(currentNode, neighbors, var24);
                double var27 = this.minHeap.peek().weight;
                if(this.minHeap.peek().weight < score) {
                    this.minHeap.removeMin();
                    this.minHeap.insert(new IntAndDouble(currentNode, score));
                }
            }

            this.look = null;
        } catch (Exception var22) {
            System.out.println(var22);
        }

        System.out.println("Total time: " + (System.currentTimeMillis() - time2));
        System.out.println("Total time_2: " + this.time_2);
        System.out.println("Total time_3: " + this.time_3);
        System.out.println("Cache Hits: " + this.cacheHits + " Misses: " + this.cacheMisses + " / " + (double)this.cacheHits * 1.0D / (double)(this.cacheHits + this.cacheMisses));
        System.out.println("M cache hits: " + this.mCacheHits + " misses: " + this.mCacheMisses);
    }

    public double getScore2(int currentNode, ArrayList<TempDouble2> neighbors, Hashtable<Integer, ArrayList<TempDouble2>> neighborsNeighbors) {
        double result = -1.0D;
        if(neighbors.size() > 0) {
            try {
                Hashtable e = new Hashtable();
                double minHashes;
                Iterator var28;
                if(!this.cache.containsKey(Integer.valueOf(currentNode)) && !this.cache2.containsKey(Integer.valueOf(currentNode))) {
                    ++this.cacheMisses;
                    result = 0.0D;
                    ArrayList var19 = neighbors;
                    new HashSet();
                    HashSet var20 = this.getNumNeighborsCutoffs(neighbors, this.trials);
                    LinkedList var24 = new LinkedList();
                    if(this.unweighted) {
                        this.makeAndStoreWords(neighbors.size(), neighbors, currentNode, this.ordering, this.LSHordering, e, true);
                    } else {
                        var28 = var20.iterator();

                        while(var28.hasNext()) {
                            var24.add((Integer)var28.next());
                        }

                        Collections.sort(var24);

                        for(var28 = var24.iterator(); var28.hasNext(); this.time_2 = (long)((double)this.time_2 + minHashes)) {
                            int key = ((Integer)var28.next()).intValue();
                            double bucket = (double)System.currentTimeMillis();
                            this.makeAndStoreWords(key, var19, currentNode, this.ordering, this.LSHordering, e, true);
                            minHashes = (double)System.currentTimeMillis() - bucket;
                        }
                    }
                } else {
                    ++this.cacheHits;
                    HashSet neigh;
                    if(this.cache.containsKey(Integer.valueOf(currentNode))) {
                        neigh = (HashSet)this.cache.get(Integer.valueOf(currentNode));
                    } else {
                        neigh = (HashSet)this.cache2.get(Integer.valueOf(currentNode));
                    }

                    Iterator keys = neigh.iterator();

                    while(keys.hasNext()) {
                        String topScore = (String)keys.next();
                        Hashtable numNeighborsCutoffs;
                        if(e.containsKey(topScore)) {
                            numNeighborsCutoffs = (Hashtable)e.get(topScore);
                            if(!numNeighborsCutoffs.containsKey(Integer.valueOf(currentNode))) {
                                numNeighborsCutoffs.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                            }
                        } else {
                            numNeighborsCutoffs = new Hashtable();
                            numNeighborsCutoffs.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                            e.put(topScore, numNeighborsCutoffs);
                        }
                    }
                }

                Enumeration var21 = neighborsNeighbors.keys();

                while(true) {
                    int intersect;
                    String var31;
                    Hashtable var33;
                    while(var21.hasMoreElements()) {
                        Integer var22 = (Integer)var21.nextElement();
                        if(this.cache.containsKey(var22)) {
                            ++this.cacheHits;
                            HashSet var26 = (HashSet)this.cache.get(var22);
                            var28 = var26.iterator();

                            while(var28.hasNext()) {
                                var31 = (String)var28.next();
                                if(e.containsKey(var31)) {
                                    var33 = (Hashtable)e.get(var31);
                                    if(!var33.containsKey(Integer.valueOf(currentNode))) {
                                        var33.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                                    }
                                }
                            }
                        } else {
                            ++this.cacheMisses;
                            new HashSet();
                            new LinkedList();
                            HashSet var29 = this.getNumNeighborsCutoffs(neighbors, this.trials);
                            ArrayList var25 = (ArrayList)neighborsNeighbors.get(var22);
                            var29 = this.getNumNeighborsCutoffs(var25, this.trials);
                            LinkedList var30 = new LinkedList();
                            if(this.unweighted) {
                                this.makeAndStoreWords(var25.size(), var25, var22.intValue(), this.ordering, this.LSHordering, e, false);
                            } else {
                                Iterator var32 = var29.iterator();

                                while(var32.hasNext()) {
                                    var30.add((Integer)var32.next());
                                }

                                Collections.sort(var30);

                                double time4;
                                for(var32 = var30.iterator(); var32.hasNext(); this.time_2 = (long)((double)this.time_2 + time4)) {
                                    intersect = ((Integer)var32.next()).intValue();
                                    minHashes = (double)System.currentTimeMillis();
                                    this.makeAndStoreWords(intersect, var25, var22.intValue(), this.ordering, this.LSHordering, e, false);
                                    time4 = (double)System.currentTimeMillis() - minHashes;
                                }
                            }
                        }
                    }

                    Enumeration var23 = e.keys();
                    double var27 = 0.0D;

                    while(true) {
                        int score;
                        String[] var34;
                        double var35;
                        do {
                            if(!var23.hasMoreElements()) {
                                if(result == 0.0D) {
                                    var23 = e.keys();

                                    label98:
                                    while(true) {
                                        do {
                                            if(!var23.hasMoreElements()) {
                                                break label98;
                                            }

                                            var31 = (String)var23.nextElement();
                                        } while(((Hashtable)e.get(var31)).size() < 2);

                                        var33 = (Hashtable)e.get(var31);
                                        intersect = 0;
                                        var34 = var31.split(",");

                                        for(score = 0; score < var34.length; ++score) {
                                            if(var33.containsKey(Integer.valueOf(var34[score]))) {
                                                ++intersect;
                                            }
                                        }

                                        var35 = (double)intersect * 1.0D / (double)var33.size() * Math.sqrt((double)var33.size());
                                        if(var35 > var27) {
                                            var27 = var35;
                                        }
                                    }
                                }

                                result = var27;
                                return result;
                            }

                            var31 = (String)var23.nextElement();
                        } while(((Hashtable)e.get(var31)).size() <= 2);

                        var33 = (Hashtable)e.get(var31);
                        intersect = 0;
                        var34 = var31.split(",");

                        for(score = 0; score < var34.length; ++score) {
                            if(var33.containsKey(Integer.valueOf(var34[score]))) {
                                ++intersect;
                            }
                        }

                        var35 = (double)intersect * 1.0D / (double)var33.size() * Math.sqrt((double)var33.size());
                        if(var35 > var27) {
                            var27 = var35;
                        }
                    }
                }
            } catch (Exception var18) {
                System.out.println(var18);
            }
        }

        return result;
    }

    public void makeAndStoreWords(int topN, ArrayList<TempDouble2> neighbors, int currentNode, Integer[][] ordering, Integer[][] LSHordering, Hashtable<String, Hashtable<Integer, Integer>> LSHtable, boolean first) throws Exception {
        HashSet cacheWords = new HashSet();
        int[] minHash;
        int numKeptNeighbors;
        int i;
        int iter2;
        if(!this.cacheM.containsKey(Integer.valueOf(currentNode)) && !this.cacheM2.containsKey(Integer.valueOf(currentNode))) {
            ++this.mCacheMisses;
            minHash = new int[this.m / 2];

            for(i = 0; i < this.m / 2; ++i) {
                minHash[i] = currentNode;
                numKeptNeighbors = (int)(Math.floor((double)(this.a[i] * (long)currentNode + this.b[i])) % (double)this.p[i]);
                ListIterator LSHword = neighbors.listIterator();
                int letters = 0;

                while(LSHword.hasNext() && letters < topN) {
                    ++letters;
                    iter2 = ((TempDouble2)LSHword.next()).nodeNum;
                    int sorted = (int)(Math.floor((double)(this.a[i] * (long)iter2 + this.b[i])) % (double)this.p[i]);
                    if(sorted < numKeptNeighbors) {
                        numKeptNeighbors = sorted;
                        minHash[i] = iter2;
                    }
                }
            }

            if(this.cache.size() == this.mostProm / 2) {
                if(this.cacheM.size() < this.mostProm / 2) {
                    this.cacheM.put(Integer.valueOf(currentNode), new HoldInt(minHash));
                } else if(this.cacheM2.size() < this.mostProm / 2) {
                    this.cacheM2.put(Integer.valueOf(currentNode), new HoldInt(minHash));
                } else {
                    this.cacheM = this.cacheM2;
                    this.cacheM2 = new Hashtable();
                    this.cacheM2.put(Integer.valueOf(currentNode), new HoldInt(minHash));
                }
            }
        } else {
            if(this.cacheM.containsKey(Integer.valueOf(currentNode))) {
                minHash = ((HoldInt)this.cacheM.get(Integer.valueOf(currentNode))).m;
            } else {
                minHash = ((HoldInt)this.cacheM2.get(Integer.valueOf(currentNode))).m;
            }

            ++this.mCacheHits;
        }

        numKeptNeighbors = topN - 1;

        for(i = 0; i < this.numWords / 2; ++i) {
            String var17 = "";
            HashSet var18 = new HashSet();

            for(iter2 = 0; iter2 < this.wordLength / 2; ++iter2) {
                var18.add(Integer.valueOf(minHash[LSHordering[i][iter2].intValue()]));
            }

            Iterator var19 = var18.iterator();
            LinkedList var20 = new LinkedList();

            while(var19.hasNext()) {
                var20.add((Integer)var19.next());
            }

            Collections.sort(var20);

            for(ListIterator var21 = var20.listIterator(); var21.hasNext(); var17 = var17 + var21.next() + ",") {
                ;
            }

            cacheWords.add(var17);
            Hashtable ttemp;
            if(LSHtable.containsKey(var17)) {
                ttemp = (Hashtable)LSHtable.get(var17);
                if(!ttemp.containsKey(Integer.valueOf(currentNode))) {
                    ttemp.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                }
            } else if(first) {
                ttemp = new Hashtable();
                ttemp.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                LSHtable.put(var17, ttemp);
            }
        }

        if(this.cache.size() < this.mostProm / 2) {
            this.cache.put(Integer.valueOf(currentNode), cacheWords);
        } else if(this.cache2.size() < this.mostProm / 2) {
            this.cache2.put(Integer.valueOf(currentNode), cacheWords);
        } else {
            this.cache = this.cache2;
            this.cache2 = new Hashtable();
            this.cache2.put(Integer.valueOf(currentNode), cacheWords);
        }

    }

    public LinkedList<TempDouble2> convertToList(Hashtable<Integer, Double> neighbors) {
        LinkedList results = new LinkedList();
        double largest = 0.0D;

        int i;
        double t;
        for(Enumeration e = neighbors.keys(); e.hasMoreElements(); results.add(new TempDouble2(i, t))) {
            i = ((Integer)e.nextElement()).intValue();
            t = ((Double)neighbors.get(Integer.valueOf(i))).doubleValue();
            if(t > largest) {
                largest = t;
            }
        }

        TempDouble2 t1;
        for(Iterator i1 = results.iterator(); i1.hasNext(); t1.weight /= largest) {
            t1 = (TempDouble2)i1.next();
        }

        Collections.sort(results, Collections.reverseOrder());
        return results;
    }

    public HashSet<Integer> getNumNeighborsCutoffs(ArrayList<TempDouble2> neighbors, int trials) {
        TempDouble2[] neighbors2 = (TempDouble2[])neighbors.toArray(new TempDouble2[neighbors.size()]);
        HashSet result = new HashSet();
        Random generator = new Random();

        for(int i = 0; i < trials; ++i) {
            double guess = generator.nextDouble();
            int index = this.findIndex(guess, neighbors2);
            if(this.minClusterSize >= 5 && index >= 2) {
                result.add(Integer.valueOf(index));
            } else if(index >= 1) {
                result.add(Integer.valueOf(index));
            }
        }

        if(neighbors.size() >= 4) {
            result.add(Integer.valueOf(4));
        }

        return result;
    }

    public int findIndex(double guess, TempDouble2[] neighbors) {
        int index = 0;
        double sumSoFar = 0.0D;
        boolean found = false;

        while(!found && index < neighbors.length) {
            double weight = neighbors[index].weight;
            if(weight < guess) {
                found = true;
            } else {
                ++index;
            }
        }

        return index;
    }

    public void setLSH() {
        Random generator = new Random();
        this.ordering = new Integer[this.m / 2][this.numNodes];
        ArrayList shuffleDeck = new ArrayList();

        int z;
        for(z = 0; z < this.numNodes; ++z) {
            shuffleDeck.add(Integer.valueOf(z));
        }

        for(z = 0; z < this.m / 2; ++z) {
            Collections.shuffle(shuffleDeck);
            this.ordering[z] = (Integer[])shuffleDeck.toArray(new Integer[this.numNodes]);
        }

        this.LSHordering = new Integer[this.numWords / 2][this.wordLength / 2];

        for(z = 0; z < this.numWords / 2; ++z) {
            for(int i = 0; i < this.wordLength / 2; ++i) {
                this.LSHordering[z][i] = Integer.valueOf(generator.nextInt(this.m / 2));
            }
        }

    }

    public void setLSH2() {
        Random generator = new Random();
        this.a = new long[this.m / 2];
        this.b = new long[this.m / 2];
        this.p = new int[this.m / 2];
        Prime prime = new Prime();

        int z;
        for(z = 0; z < this.m / 2; ++z) {
            this.a[z] = Long.valueOf((long)generator.nextInt(2 * this.numNodes)).longValue();
            this.b[z] = Long.valueOf((long)generator.nextInt(2 * this.numNodes)).longValue();
            this.p[z] = prime.getPrime(this.numNodes + generator.nextInt(this.numNodes));
        }

        this.LSHordering = new Integer[this.numWords / 2][this.wordLength / 2];

        for(z = 0; z < this.numWords / 2; ++z) {
            for(int i = 0; i < this.wordLength / 2; ++i) {
                this.LSHordering[z][i] = Integer.valueOf(generator.nextInt(this.m / 2));
            }
        }

    }
}
