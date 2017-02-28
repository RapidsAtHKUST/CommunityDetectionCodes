//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Vector;

public class TopGC {
    boolean directed;
    int cd;
    String graphFileName;
    int wordLength;
    String outputFile;
    int k;
    boolean unweighted;
    boolean normalizedClusterScore = true;
    boolean normalLinear = true;
    int maxClusterSize;
    int minClusterSize;
    int mostProm;
    int importantNeighbors;
    double overlapThreshold;
    int trials;
    double averageNumLinks;
    int m;
    int numWords;
    Integer[][] ordering;
    Integer[][] LSHordering;
    int numNodes;
    int threads;
    Look3 look;

    public static void main(String[] argv) throws FileNotFoundException, IOException {
        new TopGC(argv);
    }

    public TopGC(String[] argv) {
        this.importantNeighbors = (int)Math.floor((double)(this.maxClusterSize / 2));
        this.numNodes = 0;
        this.checkArgsFirst(argv);

        try {
            this.look = new Look3(this.graphFileName);
            this.numNodes = this.look.getNNodes();
            if(this.directed) {
                this.averageNumLinks = (double)this.look.getNEdges() * 1.0D / (double)(this.numNodes * 2);
            } else {
                this.averageNumLinks = (double)this.look.getNEdges() * 1.0D / (double)(this.numNodes * 4);
            }

            this.setDefaultArgs();
            this.checkArgs(argv);
            this.look.setCache(this.mostProm);
            int[] e = new int[this.threads];
            MinHeap[] minHeaps = new MinHeap[this.threads];
            int sum2 = 0;

            int end;
            for(int threadsHolder = 0; threadsHolder < e.length; ++threadsHolder) {
                MinHeap start = new MinHeap();

                for(end = 0; end < this.mostProm; ++end) {
                    start.insert(new IntAndDouble(-1, -1.0D));
                }

                minHeaps[threadsHolder] = start;
                if(threadsHolder < e.length - 1) {
                    e[threadsHolder] = (int)Math.floor((double)(this.numNodes / this.threads));
                    sum2 += e[threadsHolder];
                }
            }

            System.out.println("First Scan. Finding top " + this.mostProm + " most " + "promising nodes.");
            e[e.length - 1] = this.numNodes - sum2;
            Thread[] var32 = new Thread[this.threads];
            int var33 = 0;
            end = e[0];

            int bestHeap;
            for(bestHeap = 0; bestHeap < this.threads; ++bestHeap) {
                var32[bestHeap] = new Thread(new FindMostProm(var33, end, bestHeap, this.graphFileName, minHeaps[bestHeap], this.numNodes, this.importantNeighbors, this.m, this.numWords, this.wordLength, this.unweighted, this.trials, this.minClusterSize, e[bestHeap], 3 * this.mostProm));
                var32[bestHeap].start();
                if(bestHeap + 1 < this.threads) {
                    var33 = end;
                    end += e[bestHeap + 1];
                    if(end > this.numNodes) {
                        end = this.numNodes;
                    }
                }
            }

            for(bestHeap = 0; bestHeap < this.threads; ++bestHeap) {
                while(var32[bestHeap].isAlive()) {
                    Thread.currentThread();
                    Thread.sleep(300L);
                }
            }

            var32 = (Thread[])null;
            MinHeap var34 = new MinHeap();

            int bestNodes;
            for(bestNodes = 0; bestNodes < this.mostProm; ++bestNodes) {
                var34.insert(new IntAndDouble(-1, -1.0D));
            }

            for(bestNodes = 0; bestNodes < minHeaps.length; ++bestNodes) {
                MinHeap lsh = minHeaps[bestNodes];

                while(lsh.size() > 0) {
                    IntAndDouble overlapRemove = lsh.removeMin();
                    if(var34.peek().weight < overlapRemove.weight) {
                        var34.removeMin();
                        var34.insert(overlapRemove);
                    }
                }
            }

            HashSet var35 = new HashSet();

            while(var34.size() > 0) {
                IntAndDouble var36 = var34.removeMin();
                if(var36.node >= 0) {
                    var35.add(Integer.valueOf(var36.node));
                }
            }

            System.out.println("Finished. Found most promising nodes.");
            System.out.println("Starting Hashing");
            LSH var37 = new LSH(var35, this.graphFileName, this.m, this.numWords, this.wordLength, this.trials, this.k, this.threads, this.numNodes, this.minClusterSize, this.mostProm);
            System.out.println("Finished. Processing. (Removing duplicate clusters and scoring).");
            double var38 = 0.3D;
            var37.buckets = this.removeDupes(var37.buckets);
            BufferedWriter tempWrite2 = new BufferedWriter(new FileWriter(this.outputFile));
            Hashtable smallGraph = new Hashtable();
            Hashtable neededLinks = new Hashtable();
            ListIterator li = var37.buckets.listIterator();
            int a = 0;

            int cc;
            int sizeSet;
            HashSet var53;
            while(li.hasNext()) {
                ++a;
                Hashtable keys1 = (Hashtable)li.next();
                Enumeration li3;
                if(keys1.size() > this.maxClusterSize) {
                    Hashtable finalClusters = new Hashtable();
                    ArrayList li2 = new ArrayList();
                    li3 = keys1.keys();

                    while(li3.hasMoreElements()) {
                        int outputClusters = ((Integer)li3.nextElement()).intValue();
                        li2.add(new TempDouble2(outputClusters, Double.valueOf((double)((Integer)keys1.get(Integer.valueOf(outputClusters))).intValue()).doubleValue()));
                    }

                    Collections.sort(li2, Collections.reverseOrder());
                    Iterator var48 = li2.iterator();

                    for(cc = 0; var48.hasNext() && cc < this.maxClusterSize; ++cc) {
                        TempDouble2 average = (TempDouble2)var48.next();
                        sizeSet = (int)Math.floor(average.weight);
                        finalClusters.put(Integer.valueOf(average.nodeNum), Integer.valueOf(sizeSet));
                    }

                    keys1 = finalClusters;
                }

                Enumeration var40 = keys1.keys();

                while(var40.hasMoreElements()) {
                    int var43 = ((Integer)var40.nextElement()).intValue();
                    li3 = keys1.keys();

                    while(li3.hasMoreElements()) {
                        cc = ((Integer)li3.nextElement()).intValue();
                        if(var43 != cc) {
                            if(!neededLinks.containsKey(Integer.valueOf(var43))) {
                                var53 = new HashSet();
                                var53.add(Integer.valueOf(cc));
                                neededLinks.put(Integer.valueOf(var43), var53);
                            } else {
                                ((HashSet)neededLinks.get(Integer.valueOf(var43))).add(Integer.valueOf(cc));
                            }

                            if(!neededLinks.containsKey(Integer.valueOf(cc))) {
                                var53 = new HashSet();
                                var53.add(Integer.valueOf(var43));
                                neededLinks.put(Integer.valueOf(cc), var53);
                            } else {
                                ((HashSet)neededLinks.get(Integer.valueOf(cc))).add(Integer.valueOf(var43));
                            }
                        }
                    }
                }
            }

            Enumeration var39 = neededLinks.keys();

            int var55;
            while(var39.hasMoreElements()) {
                int var41 = ((Integer)var39.nextElement()).intValue();
                HashSet var44 = (HashSet)neededLinks.get(Integer.valueOf(var41));
                int[] var46 = this.look.look(var41);
                ArrayList var50 = new ArrayList();
                cc = 0;

                for(var55 = 0; var55 + 1 < var46.length; var55 += 2) {
                    if(var44.contains(Integer.valueOf(var46[var55]))) {
                        int[] var57 = new int[]{var46[var55], var46[var55 + 1]};
                        var50.add(new HoldInt(var57));
                    }

                    if(var46[var55 + 1] > cc) {
                        cc = var46[var55 + 1];
                    }
                }

                Iterator var58 = var50.iterator();

                while(var58.hasNext()) {
                    HoldInt var59 = (HoldInt)var58.next();
                    if(smallGraph.containsKey(Integer.valueOf(var41))) {
                        ((Hashtable)smallGraph.get(Integer.valueOf(var41))).put(Integer.valueOf(var59.m[0]), Double.valueOf(1.0D * (double)var59.m[1] / (double)cc));
                    } else {
                        Hashtable averageSize = new Hashtable();
                        averageSize.put(Integer.valueOf(var59.m[0]), Double.valueOf(1.0D * (double)var59.m[1] / (double)cc));
                        smallGraph.put(Integer.valueOf(var41), averageSize);
                    }
                }
            }

            LinkedList var42 = new LinkedList();
            ListIterator var45 = var37.buckets.listIterator();

            while(true) {
                LinkedList var51;
                String var65;
                while(var45.hasNext()) {
                    Hashtable var47 = (Hashtable)var45.next();
                    if(var47.size() >= this.minClusterSize && var47.size() <= this.maxClusterSize) {
                        String var52 = this.makeLine(var47);
                        new LSH_Cluster(var52, var47, smallGraph, this.normalizedClusterScore);
                        var42.add(new LSH_Cluster(var52, var47, smallGraph, this.normalizedClusterScore));
                    } else if(var47.size() > this.maxClusterSize) {
                        var51 = new LinkedList();
                        Enumeration var54 = var47.keys();

                        while(var54.hasMoreElements()) {
                            var55 = ((Integer)var54.nextElement()).intValue();
                            var51.add(new TempDouble2(var55, 1.0D * (double)((Integer)var47.get(Integer.valueOf(var55))).intValue()));
                        }

                        Collections.sort(var51, Collections.reverseOrder());
                        var53 = new HashSet();
                        sizeSet = var47.size() - this.minClusterSize;
                        if(sizeSet > 5) {
                            sizeSet = 5;
                        }

                        LinkedList var60 = new LinkedList();
                        HashSet clusterBase = new HashSet();
                        ListIterator count = var51.listIterator();

                        for(int cluster = 0; count.hasNext() && cluster < this.maxClusterSize; ++cluster) {
                            int liney = ((TempDouble2)count.next()).nodeNum;
                            var53.add(Integer.valueOf(liney));
                            if(cluster < this.maxClusterSize - sizeSet) {
                                clusterBase.add(Integer.valueOf(liney));
                            } else {
                                var60.add(Integer.valueOf(liney));
                            }
                        }

                        var65 = this.makeLine(var53);
                        var42.add(new LSH_Cluster(var65, var53, smallGraph, this.normalizedClusterScore));
                    }
                }

                Collections.sort(var42, Collections.reverseOrder());
                ListIterator var49 = var42.listIterator();
                var51 = new LinkedList();
                byte var56 = 0;
                double var61 = 0.0D;
                double var62 = 0.0D;
                int var63 = 0;

                while(var49.hasNext() && var56 < this.k) {
                    LSH_Cluster var64 = (LSH_Cluster)var49.next();
                    if(!this.overlapCluster(var64, var51, this.overlapThreshold) && var64.score > 0.0D) {
                        var65 = var64.line;
                        var62 += (double)(var65.split("\t").length - 1);
                        var61 += var64.score;
                        tempWrite2.write(var64.line);
                        var51.add(var64.nodes);
                        ++var63;
                    }
                }

                tempWrite2.close();
                System.out.println("Finished.");
                System.out.print("Found " + var63 + " Clusters");
                if(var63 == 0) {
                    System.out.println("Found no clusters. Perhaps try decreasing -min (minimum cluster size), ");
                    System.out.println("-l (wordLength), or increasing -p (# of nodes to keep in pruning).");
                }

                System.out.println("Output written to file " + this.outputFile);
                break;
            }
        } catch (Exception var31) {
            System.out.println(var31);
        }

    }

    public void setDefaultArgs() {
        this.cd = 50;
        this.k = 50;
        this.directed = false;
        this.maxClusterSize = 20;
        this.minClusterSize = 5;
        this.overlapThreshold = 0.2D;
        this.trials = this.maxClusterSize;
        this.threads = 1;
        this.mostProm = this.maxClusterSize * this.k;
        if(this.mostProm > this.numNodes) {
            this.mostProm = this.numNodes;
        }

        if((double)(this.mostProm / this.numNodes) < 0.3D) {
            this.mostProm = (int)Math.floor((double)this.numNodes * 0.3D);
        }

        if(this.mostProm > '썐') {
            this.mostProm = '썐';
        }

        this.m = (int)Math.ceil(2.0D * this.averageNumLinks);
        if(this.m > this.maxClusterSize) {
            this.m = this.maxClusterSize;
        }

        this.numWords = (int)Math.floor((double)(2 * this.m));
        this.wordLength = (int)Math.floor(this.averageNumLinks - 1.0D);
        if(this.wordLength > this.maxClusterSize) {
            this.wordLength = this.maxClusterSize;
        }

        if(this.wordLength < 3) {
            this.wordLength = 3;
        }

    }

    public boolean overlapCluster(LSH_Cluster cluster, LinkedList<HashSet<Integer>> outputClusters, double overlapThreshold) {
        boolean result = false;
        ListIterator li = outputClusters.listIterator();

        while(li.hasNext()) {
            HashSet cluster2 = (HashSet)li.next();
            HashSet cluster1 = cluster.nodes;
            Iterator nodes = cluster2.iterator();
            int intersect = 0;

            while(nodes.hasNext()) {
                if(cluster1.contains(nodes.next())) {
                    ++intersect;
                }
            }

            int smallestSize = cluster2.size();
            if(cluster1.size() < smallestSize) {
                smallestSize = cluster1.size();
            }

            if(1.0D * (double)intersect / (double)smallestSize > overlapThreshold) {
                result = true;
            }
        }

        return result;
    }

    public double getScore(int currentNode, Hashtable<Integer, Double> neighbors, Hashtable<Integer, Hashtable<Integer, Double>> neighborsNeighbors) {
        double sum = 0.0D;

        double otherWeight;
        for(Enumeration otherNodeIter = neighborsNeighbors.keys(); otherNodeIter.hasMoreElements(); sum += otherWeight) {
            int otherNode = ((Integer)otherNodeIter.nextElement()).intValue();
            Hashtable otherNeighbors = (Hashtable)neighborsNeighbors.get(Integer.valueOf(otherNode));
            otherWeight = ((Double)neighbors.get(Integer.valueOf(otherNode))).doubleValue();
            HashSet temp = new HashSet();
            Enumeration keys = otherNeighbors.keys();

            while(keys.hasMoreElements()) {
                int intersectionIter = ((Integer)keys.nextElement()).intValue();
                if(neighbors.containsKey(Integer.valueOf(intersectionIter))) {
                    temp.add(new IntAndDouble(intersectionIter, ((Double)otherNeighbors.get(Integer.valueOf(intersectionIter))).doubleValue()));
                }
            }

            double average;
            for(Iterator intersectionIter1 = temp.iterator(); intersectionIter1.hasNext(); otherWeight += average) {
                IntAndDouble intersection = (IntAndDouble)intersectionIter1.next();
                average = (intersection.weight + ((Double)neighbors.get(Integer.valueOf(intersection.node))).doubleValue()) / 2.0D;
            }
        }

        return sum;
    }

    public void checkArgsFirst(String[] argv) {
        boolean foundInput = false;
        boolean cont = true;

        for(int i = 0; cont && i < argv.length; ++i) {
            if(argv[i].equals("-i")) {
                foundInput = true;
                ++i;
                this.graphFileName = argv[i];
            } else if(argv[i].equals("-cd")) {
                ++i;
                this.cd = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-d")) {
                this.directed = true;
            } else if(argv[i].equals("-l")) {
                ++i;
                this.wordLength = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-k")) {
                ++i;
                this.k = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-max")) {
                ++i;
                this.maxClusterSize = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-min")) {
                ++i;
                this.minClusterSize = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-p")) {
                ++i;
                this.mostProm = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-lambda")) {
                ++i;
                this.overlapThreshold = Double.valueOf(argv[i]).doubleValue();
            } else if(argv[i].equals("-trials")) {
                ++i;
                this.trials = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-m")) {
                ++i;
                this.m = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-w")) {
                ++i;
                this.numWords = Integer.valueOf(argv[i]).intValue();
            } else {
                cont = false;
            }
        }

        if(!foundInput || !cont) {
            System.out.println("Usage: java -jar TopGC.jar -i inputGraph [-p mostProm] [-max maxClusterSize] [-min minClusterSize] [-lambda overlapThreshold] [-l wordLength] [-m m] [-w numWords] [-trials trials]");
            System.exit(1);
        }

    }

    public void checkArgs(String[] argv) {
        String endVary = "";
        boolean foundInput = false;
        boolean cont = true;

        for(int i = 0; cont && i < argv.length; ++i) {
            if(argv[i].equals("-i")) {
                foundInput = true;
                ++i;
                this.graphFileName = argv[i];
            } else if(argv[i].equals("-cd")) {
                ++i;
                this.cd = Integer.valueOf(argv[i]).intValue();
            } else if(argv[i].equals("-d")) {
                this.directed = true;
                endVary = endVary + "_directed";
            } else if(argv[i].equals("-l")) {
                ++i;
                this.wordLength = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_l" + this.wordLength;
            } else if(argv[i].equals("-k")) {
                ++i;
                this.k = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_k" + this.k;
            } else if(argv[i].equals("-max")) {
                ++i;
                this.maxClusterSize = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_max" + this.maxClusterSize;
            } else if(argv[i].equals("-min")) {
                ++i;
                this.minClusterSize = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_min" + this.minClusterSize;
            } else if(argv[i].equals("-p")) {
                ++i;
                this.mostProm = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_p" + this.mostProm;
            } else if(argv[i].equals("-lambda")) {
                ++i;
                this.overlapThreshold = Double.valueOf(argv[i]).doubleValue();
                endVary = endVary + "_lambda" + this.overlapThreshold;
            } else if(argv[i].equals("-trials")) {
                ++i;
                this.trials = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_trials" + this.trials;
            } else if(argv[i].equals("-m")) {
                ++i;
                this.m = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_m" + this.m;
            } else if(argv[i].equals("-w")) {
                ++i;
                this.numWords = Integer.valueOf(argv[i]).intValue();
                endVary = endVary + "_w" + this.numWords;
            } else {
                cont = false;
            }
        }

        if(foundInput && cont) {
            System.out.println("=================================");
            System.out.print("nodes = " + this.look.getNNodes() + " unique edges = ");
            if(this.directed) {
                System.out.println(String.valueOf(this.look.getNEdges() / 2));
            } else {
                System.out.println(String.valueOf(this.look.getNEdges() / 4));
            }

            System.out.println("Average # of edges per node on this");
            if(this.directed) {
                System.out.print("directed, ");
            } else {
                System.out.print("undirected, ");
            }

            if(this.look.hasWeights()) {
                System.out.print("weighted ");
            } else {
                System.out.print("unweighted ");
            }

            if(this.directed) {
                System.out.println("graph is " + this.averageNumLinks);
            } else {
                System.out.println("graph is " + this.averageNumLinks);
            }

            System.out.println("input= " + this.graphFileName + " max Cluster Size= " + this.maxClusterSize + " min Cluster Size= " + this.minClusterSize);
            System.out.println("p= " + this.mostProm + " lambda= " + this.overlapThreshold + " m= " + this.m + " l= " + this.wordLength + " w= " + this.numWords + " trials= " + this.trials);
            System.out.println("Using " + this.threads + " threads.");
            System.out.println("=================================");
        } else {
            System.out.println("Usage: java -jar TopGC.jar -i inputGraph [-p mostProm] [-max maxClusterSize] [-min minClusterSize] [-lambda overlapThreshold] [-l wordLength] [-m m] [-w numWords] [-trials trials]");
            System.exit(1);
        }

        this.outputFile = this.graphFileName + ".clusters" + endVary;
    }

    public String makeLine(Hashtable<Integer, Integer> cluster) {
        Enumeration li2 = cluster.keys();
        LinkedList line = new LinkedList();
        String line2 = "";

        while(li2.hasMoreElements()) {
            line.add(String.valueOf(li2.nextElement()));
        }

        Collections.sort(line);

        for(ListIterator li3 = line.listIterator(); li3.hasNext(); line2 = line2 + (String)li3.next() + "\t") {
            ;
        }

        line2 = line2 + "\n";
        return line2;
    }

    public String makeLine(HashSet<Integer> cluster) {
        Iterator li2 = cluster.iterator();
        LinkedList line = new LinkedList();
        String line2 = "";

        while(li2.hasNext()) {
            line.add(String.valueOf(li2.next()));
        }

        Collections.sort(line);

        for(ListIterator li3 = line.listIterator(); li3.hasNext(); line2 = line2 + (String)li3.next() + "\t") {
            ;
        }

        line2 = line2 + "\n";
        return line2;
    }

    public ArrayList<Hashtable<Integer, Integer>> removeDupes(ArrayList<Hashtable<Integer, Integer>> buckets) {
        ArrayList results = new ArrayList();
        Hashtable table = new Hashtable();

        for(int keys = 0; keys < buckets.size(); ++keys) {
            Hashtable bucket1 = (Hashtable)buckets.get(keys);
            HashSet key = new HashSet();
            Enumeration en = bucket1.keys();

            while(en.hasMoreElements()) {
                key.add((Integer)en.nextElement());
            }

            if(table.containsKey(key)) {
                Hashtable Oldtable = (Hashtable)table.get(key);
                en = bucket1.keys();

                while(en.hasMoreElements()) {
                    Integer tempKey = (Integer)en.nextElement();
                    Integer tempVal = Integer.valueOf(((Integer)Oldtable.get(tempKey)).intValue() + ((Integer)bucket1.get(tempKey)).intValue());
                    Oldtable.put(tempKey, tempVal);
                }
            } else {
                table.put(key, bucket1);
            }
        }

        Enumeration var11 = table.keys();

        while(var11.hasMoreElements()) {
            results.add((Hashtable)table.get(var11.nextElement()));
        }

        return results;
    }

    public ArrayList<Hashtable<Integer, Integer>> removeOverlap3(double overlapRemove, ArrayList<Hashtable<Integer, Integer>> buckets) {
        Hashtable finalClusts = new Hashtable();
        ArrayList compareClusts = new ArrayList();

        Enumeration keys;
        for(int done = 0; done < buckets.size(); ++done) {
            Hashtable level = (Hashtable)buckets.get(done);
            HashSet result = new HashSet();
            keys = level.keys();

            while(keys.hasMoreElements()) {
                result.add((Integer)keys.nextElement());
            }

            TopGC.TempCluster key = new TopGC.TempCluster(done, result, (TopGC.TempCluster)null);
            finalClusts.put(result, key);
            key = new TopGC.TempCluster(done, (HashSet)result.clone(), (TopGC.TempCluster)null);
            compareClusts.add(key);
        }

        boolean var18 = false;

        TopGC.TempCluster keyClust;
        ArrayList var20;
        for(int var19 = 0; !var18; ++var19) {
            var18 = true;
            var20 = new ArrayList();
            HashSet var21 = new HashSet();

            int var22;
            for(var22 = 0; var22 < compareClusts.size() - 1; ++var22) {
                if(!var21.contains(Integer.valueOf(var22))) {
                    keyClust = (TopGC.TempCluster)compareClusts.get(var22);

                    for(int j = var22 + 1; j < compareClusts.size(); ++j) {
                        if(!var21.contains(Integer.valueOf(j)) && !var21.contains(Integer.valueOf(var22))) {
                            TopGC.TempCluster cluster2 = (TopGC.TempCluster)compareClusts.get(j);
                            if((keyClust.level == var19 || cluster2.level == var19) && this.overlapTooMuch(keyClust.nodes, cluster2.nodes, overlapRemove)) {
                                var21.add(Integer.valueOf(var22));
                                var21.add(Integer.valueOf(j));
                                HashSet newNodes = keyClust.nodes;
                                newNodes.addAll(cluster2.nodes);
                                Iterator temp;
                                int finalCombined;
                                if(finalClusts.containsKey(newNodes)) {
                                    TopGC.TempCluster var24 = (TopGC.TempCluster)finalClusts.get(newNodes);
                                    temp = keyClust.clusterNums.iterator();

                                    while(temp.hasNext()) {
                                        finalCombined = ((Integer)temp.next()).intValue();
                                        if(finalCombined >= buckets.size()) {
                                            System.out.println("ERROR 1! Trying to put in a " + finalCombined + " into buckets of size " + buckets.size());
                                        } else {
                                            var24.clusterNums.add(Integer.valueOf(finalCombined));
                                        }
                                    }

                                    temp = cluster2.clusterNums.iterator();

                                    while(temp.hasNext()) {
                                        finalCombined = ((Integer)temp.next()).intValue();
                                        if(finalCombined >= buckets.size()) {
                                            System.out.println("ERROR 2! Trying to put in a " + finalCombined + " into buckets of size " + buckets.size());
                                        } else {
                                            var24.clusterNums.add(Integer.valueOf(finalCombined));
                                        }
                                    }
                                } else {
                                    var18 = false;
                                    HashSet newClusterNums = new HashSet();
                                    temp = keyClust.clusterNums.iterator();

                                    while(temp.hasNext()) {
                                        finalCombined = ((Integer)temp.next()).intValue();
                                        if(finalCombined >= buckets.size()) {
                                            System.out.println("ERROR 3! Trying to put in a " + finalCombined + " into buckets of size " + buckets.size());
                                        } else {
                                            newClusterNums.add(Integer.valueOf(finalCombined));
                                        }
                                    }

                                    temp = cluster2.clusterNums.iterator();

                                    while(temp.hasNext()) {
                                        finalCombined = ((Integer)temp.next()).intValue();
                                        if(finalCombined >= buckets.size()) {
                                            System.out.println("ERROR 4! Trying to put in a " + finalCombined + " into buckets of size " + buckets.size());
                                        } else {
                                            newClusterNums.add(Integer.valueOf(finalCombined));
                                        }
                                    }

                                    TopGC.TempCluster var25 = new TopGC.TempCluster(newNodes, newClusterNums, (TopGC.TempCluster)null);
                                    var25.level = var19;
                                    var20.add(var25);
                                    var25 = new TopGC.TempCluster((HashSet)newNodes.clone(), (HashSet)newClusterNums.clone(), (TopGC.TempCluster)null);
                                    finalClusts.put(var25.nodes, var25);
                                }
                            }
                        }
                    }
                }
            }

            for(var22 = 0; var22 < compareClusts.size(); ++var22) {
                if(!var21.contains(Integer.valueOf(var22))) {
                    var20.add((TopGC.TempCluster)compareClusts.get(var22));
                }
            }

            compareClusts = null;
            compareClusts = var20;
        }

        var20 = new ArrayList();
        keys = finalClusts.keys();

        while(keys.hasMoreElements()) {
            HashSet var23 = (HashSet)keys.nextElement();
            keyClust = (TopGC.TempCluster)finalClusts.get(var23);
            var20.add(this.convertClust(var23, keyClust, buckets));
        }

        System.out.println("Done.");
        return var20;
    }

    public int intersect(HashSet<Integer> compare1, HashSet<Integer> compare2) {
        Iterator iter = compare1.iterator();
        int intersect = 0;

        while(iter.hasNext()) {
            if(compare2.contains(iter.next())) {
                ++intersect;
            }
        }

        return intersect;
    }

    public boolean overlapTooMuch(HashSet<Integer> compare1, HashSet<Integer> compare2, double overlap) {
        Iterator iter = compare1.iterator();
        int intersect = 0;

        while(iter.hasNext()) {
            if(compare2.contains(iter.next())) {
                ++intersect;
            }
        }

        int smallSize = compare1.size();
        if(compare2.size() < smallSize) {
            smallSize = compare2.size();
        }

        if(1.0D * (double)intersect / (double)smallSize >= overlap && compare1.size() + compare2.size() - intersect <= this.maxClusterSize) {
            return true;
        } else {
            return false;
        }
    }

    public Hashtable<Integer, Integer> convertClust(HashSet<Integer> nodes, TopGC.TempCluster nodesClust, ArrayList<Hashtable<Integer, Integer>> buckets) {
        Hashtable result = new Hashtable();
        Iterator iter = nodes.iterator();

        while(iter.hasNext()) {
            result.put((Integer)iter.next(), Integer.valueOf(0));
        }

        HashSet clusters = nodesClust.clusterNums;
        iter = clusters.iterator();

        while(iter.hasNext()) {
            Hashtable values = (Hashtable)buckets.get(((Integer)iter.next()).intValue());
            Enumeration keys = values.keys();

            while(keys.hasMoreElements()) {
                Integer key = (Integer)keys.nextElement();
                result.put(key, Integer.valueOf(((Integer)result.get(key)).intValue() + ((Integer)values.get(key)).intValue()));
            }
        }

        return result;
    }

    public boolean shouldCombine(double overlapRemove, TopGC.TempCluster bucket1, TopGC.TempCluster bucket2, HashSet<Integer> nodes1, HashSet<Integer> nodes2, int iterNum) {
        boolean result = false;
        if(bucket1.level == iterNum - 1 || bucket2.level == iterNum - 1) {
            HashSet smallSet;
            HashSet largeSet;
            if(bucket1.clusterNums.size() <= bucket2.clusterNums.size()) {
                smallSet = bucket1.clusterNums;
                largeSet = bucket2.clusterNums;
            } else {
                smallSet = bucket2.clusterNums;
                largeSet = bucket1.clusterNums;
            }

            boolean contained = true;
            Iterator iter = smallSet.iterator();

            while(iter.hasNext() && contained) {
                if(!largeSet.contains(iter.next())) {
                    contained = false;
                }
            }

            if(!contained) {
                Iterator ii = nodes1.iterator();
                int intersect = 0;

                while(ii.hasNext()) {
                    if(nodes2.contains(ii.next())) {
                        ++intersect;
                    }
                }

                int minBucket = nodes1.size();
                if(minBucket > nodes2.size()) {
                    minBucket = nodes2.size();
                }

                if(intersect >= 3) {
                    double var16 = 1.0D * (double)intersect / (double)minBucket;
                }

                if(intersect == nodes1.size() && intersect == nodes2.size()) {
                    result = true;
                } else if(1.0D * (double)intersect / (double)minBucket >= overlapRemove) {
                    result = true;
                }
            }
        }

        return result;
    }

    public boolean removeOverlap(double overlapRemove, ArrayList<Hashtable<Integer, Integer>> buckets) {
        boolean done = true;

        for(int i = 0; i < buckets.size() - 1; ++i) {
            HashSet remove = new HashSet();
            Hashtable bucket1 = (Hashtable)buckets.get(i);

            int num;
            for(int ii = i + 1; ii < buckets.size(); ++ii) {
                if(!remove.contains(Integer.valueOf(ii))) {
                    Hashtable remove_sorted = (Hashtable)buckets.get(ii);
                    Enumeration li = bucket1.keys();
                    num = 0;

                    while(li.hasMoreElements()) {
                        if(remove_sorted.containsKey(li.nextElement())) {
                            ++num;
                        }
                    }

                    int minBucket = bucket1.size();
                    if(minBucket > remove_sorted.size()) {
                        minBucket = remove_sorted.size();
                    }

                    if(num >= 3) {
                        double var13 = 1.0D * (double)num / (double)minBucket;
                    }

                    if(num == bucket1.size() && num == remove_sorted.size()) {
                        done = false;
                        remove.add(Integer.valueOf(ii));
                    } else if(1.0D * (double)num / (double)minBucket >= overlapRemove) {
                        done = false;
                        remove.add(Integer.valueOf(ii));
                        this.combineBuckets(bucket1, remove_sorted);
                    }
                }
            }

            Iterator var15 = remove.iterator();
            LinkedList var16 = new LinkedList();

            while(var15.hasNext()) {
                var16.add((Integer)var15.next());
            }

            Collections.sort(var16, Collections.reverseOrder());
            ListIterator var17 = var16.listIterator();

            while(var17.hasNext()) {
                num = ((Integer)var17.next()).intValue();
                buckets.remove(num);
            }
        }

        return done;
    }

    public void combineBuckets(Hashtable<Integer, Integer> bucket1, Hashtable<Integer, Integer> bucket2) {
        Enumeration e = bucket2.keys();

        while(e.hasMoreElements()) {
            int key2 = ((Integer)e.nextElement()).intValue();
            int count2 = ((Integer)bucket2.get(Integer.valueOf(key2))).intValue();
            if(bucket1.containsKey(Integer.valueOf(key2))) {
                bucket1.put(Integer.valueOf(key2), Integer.valueOf(((Integer)bucket1.get(Integer.valueOf(key2))).intValue() + count2));
            } else {
                bucket1.put(Integer.valueOf(key2), Integer.valueOf(count2));
            }
        }

    }

    public String[] split(String str, char delim) {
        Vector strsVec = new Vector(0, 1);

        String tmp;
        for(tmp = str; tmp.indexOf(delim) != -1; tmp = tmp.substring(tmp.indexOf(delim) + 1, tmp.length())) {
            if(tmp.substring(0, tmp.indexOf(delim)).length() > 0) {
                strsVec.addElement(new String(tmp.substring(0, tmp.indexOf(delim))));
            }
        }

        strsVec.addElement(new String(tmp));
        String[] strs = new String[strsVec.capacity()];

        for(int s = 0; s < strsVec.capacity(); ++s) {
            strs[s] = (String)strsVec.elementAt(s);
        }

        return strs;
    }

    private class IterateSet {
        ArrayList<Integer> nodes = new ArrayList();
        int[] choose;
        boolean notDone = true;

        public IterateSet(LinkedList<Integer> var1) {
            Iterator i = nodes2.iterator();

            while(i.hasNext()) {
                this.nodes.add((Integer)i.next());
            }

            this.choose = new int[this.nodes.size()];
        }

        public boolean almostDone() {
            boolean result = true;

            for(int i = 0; i < this.choose.length && result; ++i) {
                if(this.choose[i] != 1) {
                    result = false;
                }
            }

            return result;
        }

        public boolean hasNext() {
            return this.notDone;
        }

        public HashSet<Integer> next() {
            HashSet result = new HashSet();

            for(int i = 0; i < this.choose.length; ++i) {
                if(this.choose[i] == 1) {
                    result.add((Integer)this.nodes.get(i));
                }
            }

            this.incr();
            return result;
        }

        public void incr() {
            boolean done = false;
            if(!this.almostDone()) {
                for(int i = this.choose.length - 1; i >= 0 && !done; --i) {
                    if(this.choose[i] == 0) {
                        done = true;
                        this.choose[i] = 1;
                    } else {
                        this.choose[i] = 0;
                    }
                }
            } else {
                this.notDone = false;
            }

        }
    }

    private class TempCluster {
        int level;
        HashSet<Integer> clusterNums;
        HashSet<Integer> nodes;

        private TempCluster(int var1, HashSet<Integer> num) {
            this.clusterNums = new HashSet();
            this.clusterNums.add(Integer.valueOf(num));
            this.nodes = nodes;
            this.level = 0;
        }

        private TempCluster(HashSet<Integer> var1, HashSet<Integer> nodes) {
            this.clusterNums = new HashSet();
            this.clusterNums = clusterNums;
            this.nodes = nodes;
        }

        private TempCluster(TopGC.TempCluster clust1, TopGC.TempCluster clust2, int iterNum) {
            this.clusterNums = new HashSet();
            this.clusterNums = clust1.clusterNums;
            Iterator iter = clust2.clusterNums.iterator();

            while(iter.hasNext()) {
                this.clusterNums.add((Integer)iter.next());
            }

            this.nodes = clust1.nodes;
            this.nodes.addAll(clust2.nodes);
        }
    }
}
