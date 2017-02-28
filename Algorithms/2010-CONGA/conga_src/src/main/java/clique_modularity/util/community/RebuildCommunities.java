package clique_modularity.util.community;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.input_output.ReadEdges;
import clique_modularity.util.Tree;
import clique_modularity.util.metric.Modularity;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

public class RebuildCommunities {
    private static int nEdgesMod;

    public RebuildCommunities() {
    }

    public static ArrayList<HashSet<Integer>> rebuildCommunities(int var0, int var1, ArrayList<HashSet<Integer>> var2, String var3) throws FileNotFoundException {
        boolean var4 = false;
        double var5 = 1.0D;
        int var7 = var2.size();
        int[][] var24 = new int[var2.size()][var2.size()];
        ArrayList var25 = new ArrayList();
        ArrayList var26 = new ArrayList();
        ReadEdges var27 = new ReadEdges();
        ArrayList var28 = var27.readGraph(var3, var1);
        double var22 = modularity(var28, var1, var2, var24, var25, var26);
        if (var4) {
            System.out.println(var7 + ": " + var22);
        }

        CommPairSet var30 = new CommPairSet(var7);

        int var8;
        double var17;
        Iterator var19;
        for (var8 = 0; var8 < var25.size(); ++var8) {
            var19 = ((HashSet) var25.get(var8)).iterator();

            while (var19.hasNext()) {
                int var9 = ((Integer) var19.next()).intValue();
                if (var8 > var9) {
                    var17 = compareCliques_mod_fast(var8, var9, var24, var26, nEdgesMod);
                    var30.add(new CommPair(var8, var9, var17));
                }
            }
        }

        ArrayList var31 = new ArrayList();

        for (var8 = 0; var8 < var2.size(); ++var8) {
            var31.add(new Tree((HashSet) var2.get(var8)));
        }

        for (int var14 = var7; var14 > var0; --var14) {
            CommPair var29 = var30.best();
            int var20 = var29.comm1;
            int var21 = var29.comm2;
            double var15 = var29.modInc;
            if (var21 != -1) {
                ((Tree) var31.get(var21)).add((Tree) var31.get(var20));
                var31.set(var20, (Object) null);
                var22 = updateMod(var21, var20, var24, var25, var26, var22, nEdgesMod);
                if (var4) {
                    System.out.println(var14 - 1 + ": " + var22);
                }

                var30.remove(var20);
                var30.remove(var21);
                var19 = ((HashSet) var25.get(var21)).iterator();

                while (var19.hasNext()) {
                    var8 = ((Integer) var19.next()).intValue();
                    var17 = compareCliques_mod_fast(var8, var21, var24, var26, nEdgesMod);
                    if (var8 > var21) {
                        var30.add(new CommPair(var8, var21, var17));
                    } else if (var8 < var21) {
                        var30.add(new CommPair(var21, var8, var17));
                    }
                }
            }
        }

        ArrayList var32 = new ArrayList();

        for (var8 = 0; var8 < var2.size(); ++var8) {
            if (var31.get(var8) != null) {
                HashSet var33 = new HashSet();
                collect((Tree) var31.get(var8), var33);
                var32.add(var33);
            }
        }

        return var32;
    }

    private static double compareCliques_mod_fast(int var0, int var1, int[][] var2, ArrayList<Double> var3, int var4) {
        return ((double) var2[var0][var1] - ((Double) var3.get(var0)).doubleValue() * ((Double) var3.get(var1)).doubleValue() / (double) (var4 + var4)) / (double) var4;
    }

    private static void collect(Tree var0, HashSet<Integer> var1) {
        if (var0 != null) {
            if (var0.isLeaf) {
                var1.addAll(var0.set);
            } else {
                collect(var0.left, var1);
                collect(var0.right, var1);
            }

        }
    }

    private static double modularity(ArrayList<HashSet<Integer>> var0, int var1, ArrayList<HashSet<Integer>> var2, int[][] var3, ArrayList<HashSet<Integer>> var4, ArrayList<Double> var5) throws FileNotFoundException {
        ArrayList var6 = new ArrayList();
        int var14 = 0;

        int var8;
        for (var8 = 0; var8 < var1; ++var8) {
            var6.add(Integer.valueOf(0));
        }

        int var9;
        Iterator var15;
        for (var8 = 0; var8 < var2.size(); ++var8) {
            var15 = ((HashSet) var2.get(var8)).iterator();

            while (var15.hasNext()) {
                var9 = ((Integer) var15.next()).intValue();
                var6.set(var9, Integer.valueOf(var8));
            }

            var4.add(new HashSet());
            var5.add(Double.valueOf(0.0D));
        }

        int var7 = var2.size();

        for (var8 = 0; var8 < var0.size(); ++var8) {
            var15 = ((HashSet) var0.get(var8)).iterator();

            while (var15.hasNext()) {
                var9 = ((Integer) var15.next()).intValue();
                if (var8 < var9) {
                    int var10 = ((Integer) var6.get(var8)).intValue();
                    int var11 = ((Integer) var6.get(var9)).intValue();
                    ++var3[var10][var11];
                    ++var3[var11][var10];
                    if (var10 != var11) {
                        ((HashSet) var4.get(var10)).add(Integer.valueOf(var11));
                        ((HashSet) var4.get(var11)).add(Integer.valueOf(var10));
                    }

                    var5.set(var10, Double.valueOf(((Double) var5.get(var10)).doubleValue() + 1.0D));
                    var5.set(var11, Double.valueOf(((Double) var5.get(var11)).doubleValue() + 1.0D));
                    ++var14;
                }
            }
        }

        nEdgesMod = var14;
        double var16 = (double) (var14 + var14);
        double var18 = 0.0D;

        for (var8 = 0; var8 < var7; ++var8) {
            double var12 = ((Double) var5.get(var8)).doubleValue();
            var18 += ((double) var3[var8][var8] - var12 * var12 / var16) / var16;
        }

        return var18;
    }

    public static void show(int[][] var0, ArrayList<HashSet<Integer>> var1, CommPairSet var2) {
        for (int var3 = 0; var3 < var0.length; ++var3) {
            for (int var4 = 0; var4 < var0.length; ++var4) {
                System.out.print(var0[var3][var4] + " ");
            }

            System.out.println();
        }

        System.out.println(var1);
        System.out.println(var2);
    }

    public static int detcomp(double var0, int var2, int var3, double var4, int var6, int var7) {
        return var0 < var4 ? -1 : (var0 > var4 ? 1 : (var2 < var6 ? -1 : (var2 > var6 ? 1 : (var3 < var7 ? -1 : (var3 > var7 ? 1 : 0)))));
    }

    private static double updateMod(int var0, int var1, int[][] var2, ArrayList<HashSet<Integer>> var3, ArrayList<Double> var4, double var5, int var7) {
        double var10 = ((double) var2[var1][var0] - ((Double) var4.get(var1)).doubleValue() * ((Double) var4.get(var0)).doubleValue() / (double) (var7 + var7)) / (double) var7;
        Iterator var12 = ((HashSet) var3.get(var1)).iterator();

        while (var12.hasNext()) {
            int var8 = ((Integer) var12.next()).intValue();
            var2[var0][var8] += var2[var1][var8];
            var2[var8][var0] += var2[var8][var1];
            if (var8 == var0) {
                var2[var0][var0] += var2[var1][var1];
            } else {
                ((HashSet) var3.get(var0)).add(Integer.valueOf(var8));
                ((HashSet) var3.get(var8)).add(Integer.valueOf(var0));
            }

            var2[var1][var8] = 0;
            var2[var8][var1] = 0;
            ((HashSet) var3.get(var8)).remove(Integer.valueOf(var1));
            var12.remove();
        }

        var4.set(var0, Double.valueOf(((Double) var4.get(var0)).doubleValue() + ((Double) var4.get(var1)).doubleValue()));
        var2[var1][var1] = 0;
        var4.set(var1, Double.valueOf(0.0D));
        return var5 + var10;
    }

    static double compareCliques_missing(int var0, int var1, ArrayList<HashSet<Integer>> var2, ArrayList<HashSet<Integer>> var3, ArrayList<Integer> var4, ArrayList<Integer> var5, int var6, int var7) {
        double var8 = (double) var6 / (double) var7;
        int var10 = ((HashSet) var3.get(var0)).size();
        int var11 = ((HashSet) var3.get(var1)).size();
        int var12 = var10 + var11;
        var7 += var12 * (var12 - 1) / 2 - var10 * (var10 - 1) / 2 - var11 * (var11 - 1) / 2;
        var6 += edgeCount((HashSet) var3.get(var0), (HashSet) var3.get(var1), var2);
        double var13 = (double) var6 / (double) var7;
        return var13 - var8;
    }

    static double compareCliques_missing2(int var0, int var1, ArrayList<HashSet<Integer>> var2, ArrayList<HashSet<Integer>> var3, ArrayList<Integer> var4, ArrayList<Integer> var5, int var6, int var7) {
        int var8 = ((HashSet) var3.get(var0)).size();
        int var9 = ((HashSet) var3.get(var1)).size();
        int var10 = var8 + var9;
        int var11 = var10 * (var10 - 1) / 2 - var8 * (var8 - 1) / 2 - var9 * (var9 - 1) / 2;
        int var12 = edgeCount((HashSet) var3.get(var0), (HashSet) var3.get(var1), var2);
        return (double) var12 / (double) var11;
    }

    static double compareCliques_edgeCount(int var0, int var1, ArrayList<HashSet<Integer>> var2, ArrayList<HashSet<Integer>> var3) {
        int var5 = ((HashSet) var3.get(var0)).size();
        int var6 = ((HashSet) var3.get(var1)).size();
        int var4 = edgeCount((HashSet) var3.get(var0), (HashSet) var3.get(var1), var2);
        return (double) var4 / (double) (var5 * var6);
    }

    static int edgeCount(HashSet<Integer> var0, HashSet<Integer> var1, ArrayList<HashSet<Integer>> var2) {
        int var3 = 0;
        Iterator var6 = var0.iterator();

        while (var6.hasNext()) {
            int var4 = ((Integer) var6.next()).intValue();
            Iterator var7 = var1.iterator();

            while (var7.hasNext()) {
                int var5 = ((Integer) var7.next()).intValue();
                if (((HashSet) var2.get(var4)).contains(Integer.valueOf(var5))) {
                    ++var3;
                }
            }
        }

        return var3;
    }

    static double compareCliques_mod(int var0, int var1, ArrayList<HashSet<Integer>> var2, ArrayList<HashSet<Integer>> var3, String var4, double var5) {
        if (edgeCount((HashSet) var3.get(var0), (HashSet) var3.get(var1), var2) == 0) {
            return -1.0D / 0.0;
        } else {
            double var7 = getMod(var4, var3, var0, var1);
            return var7 - var5;
        }
    }

    static double getMod(String var0, ArrayList<HashSet<Integer>> var1, int var2, int var3) {
        String var5 = "tempClusters.txt";

        try {
            PrintStream var6 = new PrintStream(new FileOutputStream(var5));

            for (int var4 = 0; var4 < var1.size(); ++var4) {
                if (var2 == var3 || var4 != var3) {
                    writeCom((HashSet) var1.get(var4), var6);
                }

                if (var2 != var3 && var4 == var2) {
                    writeCom((HashSet) var1.get(var3), var6);
                }

                if (var2 == var3 || var4 != var3) {
                    var6.println();
                }
            }

            var6.close();
        } catch (Exception var7) {
            System.out.println("Error: " + var7.toString());
            System.exit(1);
        }

        return Modularity.modularity(var0, var5);
    }

    static void writeCom(HashSet<Integer> var0, PrintStream var1) {
        Iterator var2 = var0.iterator();

        while (var2.hasNext()) {
            var1.print(var2.next() + " ");
        }

    }
}
