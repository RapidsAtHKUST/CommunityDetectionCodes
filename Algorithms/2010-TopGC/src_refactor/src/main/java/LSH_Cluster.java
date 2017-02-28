//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;

public class LSH_Cluster implements Comparable<LSH_Cluster> {
    boolean normalizedClusterScore;
    HashSet<Integer> nodes;
    HashSet<String> nodes2;
    double score;
    String line;

    public LSH_Cluster(String l, Hashtable<Integer, Integer> n, Hashtable<Integer, Hashtable<Integer, Double>> smallGraph, boolean norm) {
        this.normalizedClusterScore = norm;
        this.line = l;
        this.nodes = new HashSet();
        Enumeration i = n.keys();

        while(i.hasMoreElements()) {
            this.nodes.add((Integer)i.nextElement());
        }

        this.score = this.getScore(smallGraph);
    }

    public LSH_Cluster(String l, HashSet<Integer> n, Hashtable<Integer, Hashtable<Integer, Double>> smallGraph, boolean norm) {
        this.normalizedClusterScore = norm;
        this.line = l;
        this.nodes = new HashSet();
        Iterator i = n.iterator();

        while(i.hasNext()) {
            this.nodes.add((Integer)i.next());
        }

        this.score = this.getScore(smallGraph);
    }

    public LSH_Cluster(String l, HashSet<String> n, Hashtable<String, Hashtable<String, Double>> smallGraph, int k, boolean norm) {
        this.normalizedClusterScore = norm;
        this.line = l;
        this.nodes2 = new HashSet();
        Iterator i = n.iterator();

        while(i.hasNext()) {
            this.nodes2.add((String)i.next());
        }

        this.score = this.getScore2(smallGraph);
    }

    public int compareTo(LSH_Cluster other) {
        return this.score > other.score?1:(this.score == other.score?0:-1);
    }

    public int size() {
        return this.nodes.size();
    }

    double getScore(Hashtable<Integer, Hashtable<Integer, Double>> smallGraph) {
        double sum = 0.0D;
        Iterator li = this.nodes.iterator();

        while(li.hasNext()) {
            int average = ((Integer)li.next()).intValue();
            Iterator li2 = this.nodes.iterator();

            while(li2.hasNext()) {
                int secondNode = ((Integer)li2.next()).intValue();
                if(average != secondNode) {
                    int first = average;
                    int second = secondNode;
                    if(!this.normalizedClusterScore && average > secondNode) {
                        second = average;
                        first = secondNode;
                    }

                    if(smallGraph.containsKey(Integer.valueOf(first)) && ((Hashtable)smallGraph.get(Integer.valueOf(first))).containsKey(Integer.valueOf(second))) {
                        sum += ((Double)((Hashtable)smallGraph.get(Integer.valueOf(first))).get(Integer.valueOf(second))).doubleValue();
                    }
                }
            }
        }

        double average1 = sum / (double)(this.nodes.size() * (this.nodes.size() - 1));
        return Math.sqrt((double)this.nodes.size()) * average1;
    }

    double getScore2(Hashtable<String, Hashtable<String, Double>> smallGraph) {
        double sum = 0.0D;
        Iterator li = this.nodes2.iterator();

        while(li.hasNext()) {
            String average = (String)li.next();
            Iterator li2 = this.nodes2.iterator();

            while(li2.hasNext()) {
                String secondNode = (String)li2.next();
                if(!average.equals(secondNode)) {
                    String first = average;
                    String second = secondNode;
                    if(!this.normalizedClusterScore && average.compareTo(secondNode) > 0) {
                        second = average;
                        first = secondNode;
                    }

                    if(smallGraph.containsKey(first) && ((Hashtable)smallGraph.get(first)).containsKey(second)) {
                        sum += ((Double)((Hashtable)smallGraph.get(first)).get(second)).doubleValue();
                    }
                }
            }
        }

        double average1 = sum / (double)(this.nodes2.size() * (this.nodes2.size() - 1));
        return Math.sqrt((double)this.nodes2.size()) * average1;
    }
}
