//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.Random;

public class LSH {
    ArrayList<Hashtable<Integer, Integer>> buckets;
    Hashtable<String, Hashtable<Integer, Integer>> LSHtable = new Hashtable();
    Hashtable<String, Integer> LSHtable_numNeighbors = new Hashtable();
    Hashtable<Integer, LinkedList<TempDouble2>> cutGraph = new Hashtable();
    int m;
    int numWords;
    int wordLength;
    int minClusterSize;
    int numNodes;
    int maxBuckets = 4000;
    Integer[][] ordering;
    Integer[][] LSHordering;
    long[] a;
    long[] b;
    int[] p;
    Look3 look;
    String graphFileName;
    String topLine;

    public LSH(HashSet<Integer> bestNodes, String graphFileName, int m2, int numWords2, int wordLength2, int trials, int topK, int threads, int numNodes, int minClusterSize, int mostProm) {
        this.m = m2;
        this.numWords = numWords2;
        this.wordLength = wordLength2;
        this.minClusterSize = minClusterSize;
        this.numNodes = numNodes;

        try {
            this.look = new Look3(graphFileName);
            this.look.setCache(mostProm);
        } catch (Exception var23) {
            System.out.println(var23);
        }

        this.graphFileName = graphFileName;

        try {
            this.setLSH2();
            int e = numNodes / threads;
            int cutKey = e;
            int start = 0;
            int num2 = 0;
            Thread[] threadsHolder = new Thread[threads];

            int mult;
            for(mult = 0; mult < threads; ++mult) {
                threadsHolder[mult] = new Thread(new LSHThread(start, cutKey, graphFileName, trials, this.ordering, this.LSHordering, this.LSHtable, this.LSHtable_numNeighbors, this.m, this.numWords, this.wordLength, mult, bestNodes, minClusterSize, this.a, this.b, this.p, mostProm));
                threadsHolder[mult].start();
                ++num2;
                start = cutKey + 1;
                cutKey += e;
                if(cutKey >= numNodes || mult + 2 >= threads) {
                    cutKey = numNodes;
                }
            }

            for(mult = 0; mult < threads; ++mult) {
                while(threadsHolder[mult].isAlive()) {
                    Thread.currentThread();
                    Thread.sleep(300L);
                }
            }

            System.out.println("Stored LSH values into buckets. Now going through.");
            int numUniqueBinsFound = 0;
            boolean foundNumBins = false;
            boolean over = false;

            int numToFind;
            double var25;
            for(var25 = 1.0D; !over && numUniqueBinsFound < 2 * topK && var25 >= 1.0D; var25 -= 0.1D) {
                numUniqueBinsFound = this.findBins3(var25);
                System.out.println("Trying mult " + var25 + " gives " + numUniqueBinsFound + " bins");
                System.out.println(this.buckets.size() + " buckets.");
                if(this.buckets.size() > 2 * topK && this.buckets.size() > this.maxBuckets) {
                    over = true;
                    System.out.println("Too many bins in relation to K. Getting rid of the weakest scoring");
                    numToFind = this.maxBuckets;
                    if(this.maxBuckets < 2 * topK) {
                        numToFind = 2 * topK;
                    }

                    this.findLessBins(var25, numToFind);
                }
            }

            over = true;
            if(numUniqueBinsFound < 2 * topK && !over) {
                for(var25 = 0.09D; numUniqueBinsFound < 2 * topK && var25 >= 0.01D; var25 -= 0.01D) {
                    numUniqueBinsFound = this.findBins3(var25);
                    System.out.println("Trying mult " + var25 + " gives " + numUniqueBinsFound + " bins");
                    if(this.buckets.size() > 2 * topK && this.buckets.size() > this.maxBuckets) {
                        over = true;
                        System.out.println("Too many bins in relation to K. Getting rid of the weakest scoring");
                        numToFind = this.maxBuckets;
                        if(this.maxBuckets < 2 * topK) {
                            numToFind = 2 * topK;
                        }

                        this.findLessBins(var25, numToFind);
                    }
                }
            }

            System.out.println("Done with LSH.");
            System.out.println("catch");
        } catch (Exception var24) {
            System.out.println(var24);
        }

    }

    public void setLSH() {
        Random generator = new Random();
        this.ordering = new Integer[this.m][this.numNodes];
        ArrayList shuffleDeck = new ArrayList();

        int z;
        for(z = 0; z < this.numNodes; ++z) {
            shuffleDeck.add(Integer.valueOf(z));
        }

        for(z = 0; z < this.m; ++z) {
            Collections.shuffle(shuffleDeck);
            this.ordering[z] = (Integer[])shuffleDeck.toArray(new Integer[this.numNodes]);
        }

        this.LSHordering = new Integer[this.numWords][this.wordLength];

        for(z = 0; z < this.numWords; ++z) {
            for(int i = 0; i < this.wordLength; ++i) {
                this.LSHordering[z][i] = Integer.valueOf(generator.nextInt(this.m));
            }
        }

    }

    public void setLSH2() {
        Random generator = new Random();
        this.a = new long[this.m];
        this.b = new long[this.m];
        this.p = new int[this.m];
        Prime prime = new Prime();

        int z;
        for(z = 0; z < this.m; ++z) {
            this.a[z] = Long.valueOf((long)generator.nextInt(2 * this.numNodes)).longValue();
            this.b[z] = Long.valueOf((long)generator.nextInt(2 * this.numNodes)).longValue();
            this.p[z] = prime.getPrime(this.numNodes + generator.nextInt(this.numNodes));
        }

        this.LSHordering = new Integer[this.numWords][this.wordLength];

        for(z = 0; z < this.numWords; ++z) {
            for(int i = 0; i < this.wordLength; ++i) {
                this.LSHordering[z][i] = Integer.valueOf(generator.nextInt(this.m));
            }
        }

    }

    public int findLessBins(double mult, int numToFind) {
        System.out.println("trying to find " + numToFind);
        int numUniqueBinsFound = 0;
        int tempCountBuckets = 0;
        HashSet foundSoFar = new HashSet();
        MaxHeap tempBucket = new MaxHeap(numToFind);
        this.buckets = new ArrayList();
        Enumeration e3 = this.LSHtable.keys();

        while(true) {
            String i;
            Hashtable nodes;
            double var22;
            do {
                String[] split;
                do {
                    if(!e3.hasMoreElements()) {
                        for(int var21 = 0; var21 < numToFind && tempBucket.size() > 0; ++var21) {
                            this.buckets.add(tempBucket.removeMin().bin);
                        }

                        tempBucket = null;
                        System.out.println("Have " + this.buckets.size() + " buckets now.");
                        return numUniqueBinsFound;
                    }

                    i = (String)e3.nextElement();
                    split = i.split(",");
                    nodes = (Hashtable)this.LSHtable.get(i);
                    Enumeration iter = nodes.keys();
                } while((this.minClusterSize < 5 || nodes.size() <= 2) && (this.minClusterSize >= 5 || nodes.size() < 2));

                int intersect = 0;

                for(int temp3 = 0; temp3 < split.length; ++temp3) {
                    if(nodes.containsKey(Integer.valueOf(split[temp3]))) {
                        ++intersect;
                    }
                }

                var22 = (double)nodes.size() * 1.0D / (double)intersect;
            } while(var22 < mult);

            ++tempCountBuckets;
            Hashtable temp = (Hashtable)nodes.clone();
            String[] temp2 = i.split(",");

            for(int unique = 0; unique < temp2.length; ++unique) {
                if(!temp2[unique].equals("") && !temp.containsKey(Integer.valueOf(temp2[unique]))) {
                    temp.put(Integer.valueOf(temp2[unique]), Integer.valueOf(0));
                }
            }

            tempBucket.insert(new HoldBucket(var22, temp));
            boolean var23 = true;

            int key2;
            for(Enumeration it = nodes.keys(); it.hasMoreElements(); foundSoFar.add(Integer.valueOf(key2))) {
                key2 = ((Integer)it.nextElement()).intValue();
                if(foundSoFar.contains(Integer.valueOf(key2))) {
                    var23 = false;
                }
            }

            if(var23) {
                ++numUniqueBinsFound;
            }
        }
    }

    public int findBins3(double mult) {
        int numUniqueBinsFound = 0;
        int tempCountBuckets = 0;
        HashSet foundSoFar = new HashSet();
        this.buckets = new ArrayList();
        Enumeration e3 = this.LSHtable.keys();

        while(true) {
            String key;
            Hashtable nodes;
            double var19;
            do {
                String[] split;
                do {
                    if(!e3.hasMoreElements()) {
                        return numUniqueBinsFound;
                    }

                    key = (String)e3.nextElement();
                    split = key.split(",");
                    nodes = (Hashtable)this.LSHtable.get(key);
                    Enumeration iter = nodes.keys();
                } while((this.minClusterSize < 5 || nodes.size() <= 2) && (this.minClusterSize >= 5 || nodes.size() < 2));

                int intersect = 0;

                for(int temp3 = 0; temp3 < split.length; ++temp3) {
                    if(nodes.containsKey(Integer.valueOf(split[temp3]))) {
                        ++intersect;
                    }
                }

                var19 = (double)nodes.size() * 1.0D / (double)intersect;
            } while(var19 < mult);

            ++tempCountBuckets;
            Hashtable temp = (Hashtable)nodes.clone();
            String[] temp2 = key.split(",");

            for(int unique = 0; unique < temp2.length; ++unique) {
                if(!temp2[unique].equals("") && !temp.containsKey(Integer.valueOf(temp2[unique]))) {
                    temp.put(Integer.valueOf(temp2[unique]), Integer.valueOf(0));
                }
            }

            this.buckets.add(temp);
            boolean var20 = true;

            int key2;
            for(Enumeration it = nodes.keys(); it.hasMoreElements(); foundSoFar.add(Integer.valueOf(key2))) {
                key2 = ((Integer)it.nextElement()).intValue();
                if(foundSoFar.contains(Integer.valueOf(key2))) {
                    var20 = false;
                }
            }

            if(var20) {
                ++numUniqueBinsFound;
            }
        }
    }

    public int findBins2(double mult) {
        int numUniqueBinsFound = 0;
        int tempCountBuckets = 0;
        HashSet foundSoFar = new HashSet();
        this.buckets = new ArrayList();
        Enumeration e3 = this.LSHtable.keys();

        while(true) {
            String key;
            Hashtable nodes;
            int intersect;
            do {
                String[] split;
                do {
                    if(!e3.hasMoreElements()) {
                        return numUniqueBinsFound;
                    }

                    key = (String)e3.nextElement();
                    split = key.split(",");
                    nodes = (Hashtable)this.LSHtable.get(key);
                    Enumeration iter = nodes.keys();
                } while((this.minClusterSize < 5 || nodes.size() <= 2) && (this.minClusterSize >= 5 || nodes.size() < 2));

                intersect = 0;

                for(int temp = 0; temp < split.length; ++temp) {
                    if(nodes.containsKey(Integer.valueOf(split[temp]))) {
                        ++intersect;
                    }
                }
            } while((double)nodes.size() * 1.0D / (double)intersect < mult);

            ++tempCountBuckets;
            Hashtable var17 = (Hashtable)nodes.clone();
            String[] temp2 = key.split(",");

            for(int unique = 0; unique < temp2.length; ++unique) {
                if(!temp2[unique].equals("") && !var17.containsKey(Integer.valueOf(temp2[unique]))) {
                    var17.put(Integer.valueOf(temp2[unique]), Integer.valueOf(0));
                }
            }

            this.buckets.add(var17);
            boolean var18 = true;

            int key2;
            for(Enumeration it = nodes.keys(); it.hasMoreElements(); foundSoFar.add(Integer.valueOf(key2))) {
                key2 = ((Integer)it.nextElement()).intValue();
                if(foundSoFar.contains(Integer.valueOf(key2))) {
                    var18 = false;
                }
            }

            if(var18) {
                ++numUniqueBinsFound;
            }
        }
    }

    public int findBins(double mult) {
        int numUniqueBinsFound = 0;

        try {
            int e = 0;
            HashSet foundSoFar = new HashSet();
            this.buckets = new ArrayList();
            Enumeration e3 = this.LSHtable.keys();

            while(true) {
                String key;
                Hashtable nodes;
                double cutoffNumber;
                do {
                    do {
                        if(!e3.hasMoreElements()) {
                            return numUniqueBinsFound;
                        }

                        key = (String)e3.nextElement();
                        String[] split = key.split(",");
                        nodes = (Hashtable)this.LSHtable.get(key);
                        Enumeration iter = nodes.keys();
                    } while((this.minClusterSize < 5 || nodes.size() <= 2) && (this.minClusterSize >= 5 || nodes.size() < 2));

                    double averageNumNeighbors = (double)(((Integer)this.LSHtable_numNeighbors.get(key)).intValue() / nodes.size());
                    cutoffNumber = this.getCutoff(mult, averageNumNeighbors);
                } while((double)nodes.size() < cutoffNumber);

                ++e;
                Hashtable temp = (Hashtable)nodes.clone();
                String[] temp2 = key.split(",");

                for(int unique = 0; unique < temp2.length; ++unique) {
                    if(!temp2[unique].equals("") && !temp.containsKey(Integer.valueOf(temp2[unique]))) {
                        temp.put(Integer.valueOf(temp2[unique]), Integer.valueOf(0));
                    }
                }

                this.buckets.add(temp);
                boolean var21 = true;

                int key2;
                for(Enumeration it = nodes.keys(); it.hasMoreElements(); foundSoFar.add(Integer.valueOf(key2))) {
                    key2 = ((Integer)it.nextElement()).intValue();
                    if(foundSoFar.contains(Integer.valueOf(key2))) {
                        var21 = false;
                    }
                }

                if(var21) {
                    ++numUniqueBinsFound;
                }
            }
        } catch (Exception var20) {
            System.out.println(var20);
            return numUniqueBinsFound;
        }
    }

    public double getCutoff(double mult, double averageNumNeighbors) {
        double result = Math.floor(mult * averageNumNeighbors);
        if(result < 3.0D) {
            result = 3.0D;
        }

        return result;
    }

    public void clearMem() {
        this.LSHtable = null;
        this.LSHtable_numNeighbors = null;
        this.cutGraph = null;
    }
}
