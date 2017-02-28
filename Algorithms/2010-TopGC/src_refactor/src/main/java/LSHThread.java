//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.Collections;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Random;

public class LSHThread implements Runnable {
    boolean sorted = true;
    boolean scale = true;
    boolean scale2 = true;
    int start;
    int end;
    int[] nodes;
    int trials;
    Hashtable<Integer, LinkedList<TempDouble2>> cutGraph;
    Integer[][] ordering;
    Integer[][] LSHordering;
    Hashtable<String, Integer> LSHtable_numNeighbors;
    Hashtable<String, Hashtable<Integer, Integer>> LSHtable;
    HashSet<Integer> bestNodes;
    int minClusterSize;
    int m;
    int numWords;
    int wordLength;
    int threadNum;
    Look3 look;
    String topLine;
    long[] a;
    long[] b;
    int[] p;

    public LSHThread(int start2, int cutKey, String graphFileName, int trials2, Integer[][] ordering2, Integer[][] LSHordering, Hashtable<String, Hashtable<Integer, Integer>> LSHtable, Hashtable<String, Integer> LSHtable_numNeighbors, int m2, int numWords2, int wordLength2, int threadNum, HashSet<Integer> bestNodes, int minClusterSize, long[] a, long[] b, int[] p, int mostProm) {
        this.start = start2;
        this.end = cutKey;

        try {
            this.look = new Look3(graphFileName);
            this.look.setCache(mostProm);
        } catch (Exception var20) {
            System.out.println(var20);
        }

        this.trials = trials2;
        this.ordering = ordering2;
        this.LSHordering = LSHordering;
        this.LSHtable = LSHtable;
        this.LSHtable_numNeighbors = LSHtable_numNeighbors;
        this.m = m2;
        this.numWords = numWords2;
        this.wordLength = wordLength2;
        this.threadNum = threadNum;
        this.bestNodes = bestNodes;
        this.minClusterSize = minClusterSize;
        this.a = a;
        this.b = b;
        this.p = p;
    }

    public void run() {
        try {
            int e = 0;

            for(int currentNode = this.start; currentNode <= this.end && currentNode < this.look.getNNodes(); ++currentNode) {
                if(e % 5000 == 0) {
                    System.out.println("LSH thread " + this.threadNum + " " + e + " / " + (this.end - this.start));
                }

                ++e;
                if(this.bestNodes.contains(Integer.valueOf(currentNode))) {
                    LinkedList neighbors = this.getNeighbors(this.look, currentNode);
                    if(neighbors.size() > 1) {
                        HashSet numNeighborsCutoffs = new HashSet();
                        if(this.look.hasWeights()) {
                            numNeighborsCutoffs = this.getNumNeighborsCutoffs(neighbors, this.trials);
                        } else {
                            numNeighborsCutoffs.add(Integer.valueOf(neighbors.size() - 1));
                        }

                        LinkedList numNeighborsCutoffsSorted = new LinkedList();
                        if(this.look.hasWeights()) {
                            Iterator ee4 = numNeighborsCutoffs.iterator();

                            while(ee4.hasNext()) {
                                numNeighborsCutoffsSorted.add((Integer)ee4.next());
                            }

                            Collections.sort(numNeighborsCutoffsSorted);
                            ee4 = numNeighborsCutoffsSorted.iterator();

                            while(ee4.hasNext()) {
                                int topN = ((Integer)ee4.next()).intValue();
                                this.makeAndStoreWords(topN, neighbors, currentNode, this.ordering, this.LSHordering);
                            }
                        } else {
                            this.makeAndStoreWords(neighbors.size(), neighbors, currentNode, this.ordering, this.LSHordering);
                        }
                    }
                }
            }
        } catch (Exception var8) {
            System.out.println(var8);
        }

    }

    public LinkedList<TempDouble2> getNeighbors(Look3 look, int currentNode) {
        LinkedList cutNeighbors2 = new LinkedList();
        LinkedList cutNeighbors = new LinkedList();
        double maxScore = 0.0D;

        try {
            int[] e = look.look(currentNode);

            for(int li = 0; li + 1 < e.length; li += 2) {
                cutNeighbors.add(new TempDouble2(e[li], Double.valueOf((double)e[li + 1]).doubleValue()));
                if((double)e[li + 1] > maxScore) {
                    maxScore = (double)e[li + 1];
                }
            }

            ListIterator li1 = cutNeighbors.listIterator();

            while(li1.hasNext()) {
                TempDouble2 node = (TempDouble2)li1.next();
                if(look.hasWeights()) {
                    cutNeighbors2.add(new TempDouble2(node.nodeNum, node.weight / maxScore));
                } else {
                    cutNeighbors2.add(new TempDouble2(node.nodeNum, 1.0D));
                }
            }

            cutNeighbors = null;
            Collections.sort(cutNeighbors2, Collections.reverseOrder());
        } catch (Exception var10) {
            System.out.println(var10);
        }

        return cutNeighbors2;
    }

    public HashSet<Integer> getNumNeighborsCutoffs(LinkedList<TempDouble2> neighbors, int trials) {
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
        if(!this.scale2) {
            while(!found && index < neighbors.length) {
                sumSoFar += neighbors[index].weight;
                if(sumSoFar >= guess) {
                    found = true;
                } else {
                    ++index;
                }
            }
        } else {
            while(!found && index < neighbors.length) {
                double weight = neighbors[index].weight;
                if(weight < guess) {
                    found = true;
                } else {
                    ++index;
                }
            }
        }

        return index;
    }

    public void makeAndStoreWords(int topN, LinkedList<TempDouble2> neighbors, int currentNode, Integer[][] ordering, Integer[][] LSHordering) throws Exception {
        int[] minHash = new int[this.m];

        int numKeptNeighbors;
        int i;
        for(numKeptNeighbors = 0; numKeptNeighbors < this.m; ++numKeptNeighbors) {
            minHash[numKeptNeighbors] = currentNode;
            int minHashValue = (int)(Math.floor((double)(this.a[numKeptNeighbors] * (long)currentNode + this.b[numKeptNeighbors])) % (double)this.p[numKeptNeighbors]);
            ListIterator ttemp2 = neighbors.listIterator();
            i = 0;

            while(ttemp2.hasNext() && i < topN) {
                ++i;
                int LSHword = ((TempDouble2)ttemp2.next()).nodeNum;
                int letters = (int)(Math.floor((double)(this.a[numKeptNeighbors] * (long)LSHword + this.b[numKeptNeighbors])) % (double)this.p[numKeptNeighbors]);
                if(letters < minHashValue) {
                    minHashValue = letters;
                    minHash[numKeptNeighbors] = LSHword;
                }
            }
        }

        numKeptNeighbors = topN - 1;
        new HashSet();

        for(i = 0; i < this.numWords; ++i) {
            String var17 = "";
            HashSet var18 = new HashSet();

            for(int iter2 = 0; iter2 < this.wordLength; ++iter2) {
                var18.add(Integer.valueOf(minHash[LSHordering[i][iter2].intValue()]));
            }

            Iterator var19 = var18.iterator();
            LinkedList sorted = new LinkedList();

            while(var19.hasNext()) {
                sorted.add((Integer)var19.next());
            }

            Collections.sort(sorted);

            for(ListIterator var20 = sorted.listIterator(); var20.hasNext(); var17 = var17 + var20.next() + ",") {
                ;
            }

            Hashtable ttemp;
            if(this.LSHtable.containsKey(var17)) {
                ttemp = (Hashtable)this.LSHtable.get(var17);
                if(!ttemp.containsKey(Integer.valueOf(currentNode))) {
                    ttemp.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                    int currentLength = ((Integer)this.LSHtable_numNeighbors.get(var17)).intValue();
                    currentLength += numKeptNeighbors;
                    this.LSHtable_numNeighbors.put(var17, Integer.valueOf(currentLength));
                } else {
                    ttemp.put(Integer.valueOf(currentNode), Integer.valueOf(((Integer)ttemp.get(Integer.valueOf(currentNode))).intValue() + 1));
                }
            } else {
                ttemp = new Hashtable();
                ttemp.put(Integer.valueOf(currentNode), Integer.valueOf(1));
                this.LSHtable.put(var17, ttemp);
                this.LSHtable_numNeighbors.put(var17, Integer.valueOf(numKeptNeighbors));
            }
        }

    }
}
