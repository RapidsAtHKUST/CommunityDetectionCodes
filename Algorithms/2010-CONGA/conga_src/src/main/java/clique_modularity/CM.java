package clique_modularity;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.algorithm.BK.BK;
import clique_modularity.algorithm.KJ.KJ;
import clique_modularity.util.benchmark.GG;
import clique_modularity.util.community.CPP;
import clique_modularity.util.metric.Modularity;
import clique_modularity.util.metric.Omega;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Vector;

public class CM {
    public static ArrayList omegaIndex = new ArrayList();
    public static ArrayList modularities = new ArrayList();
    public static int numAlgorithms;

    public CM() {
    }

    public static void main(String[] var0) {
        int var1 = var0.length;
        String var3 = "";
        int var5 = 0;
        int var6 = 0;
        String var2 = var0[0];
        if(var1 == 5) {
            while(var6 < var1) {
                String var4 = var0[var6++];
                if(var4.equals("-m")) {
                    var3 = var0[var6++];
                    if(!var3.equals("clique_modularity.algorithm.BK.BK") && !var3.equals("clique_modularity.algorithm.KJ.KJ")) {
                        System.err.println("Usage: java " + var2 + " [-m clique_modularity.algorithm.BK.BK or clique_modularity.algorithm.KJ.KJ] [-c #]");
                        System.err.println("-m mean method: clique_modularity.algorithm.BK.BK or clique_modularity.algorithm.KJ.KJ");
                        System.err.println("-c mean the number of communities");
                        System.exit(1);
                    }
                } else if(var4.equals("-c")) {
                    var5 = Integer.parseInt(var0[var6++]);
                }
            }
        } else {
            System.err.println("Usage: java " + var2 + " [-m clique_modularity.algorithm.BK.BK or clique_modularity.algorithm.KJ.KJ] [-c #]");
            System.err.println("-m mean method: clique_modularity.algorithm.BK.BK or clique_modularity.algorithm.KJ.KJ");
            System.err.println("-c mean the number of communities");
            System.exit(1);
        }

        realGraph(var2, var3, var5);
    }

    private static void realGraph(String var0, String var1, int var2) {
        int var4 = var2;
        String var5 = "cluster-temp(gn).txt";
        new HashSet();
        String var7 = (new File(var0)).getName();
        String var8 = "num-" + var7;
        String var9 = "vertex-" + var7;
        int var10 = CPP.preprocess(var0, var8, var9);

        try {
            if(var1.equals("clique_modularity.algorithm.BK.BK")) {
                BK.communityResults(var8, (String)null, var10, var4);
            } else {
                KJ.communityResults(var8, (String)null, var10, var4);
            }

            String var11 = "";
            var5 = "mergedCommunities.txt";
            var11 = "ClustersOutput.txt";
            if(var1.equals("clique_modularity.algorithm.BK.BK")) {
                System.out.println("clique_modularity.CM-clique_modularity.algorithm.BK.BK Result");
            } else {
                System.out.println("clique_modularity.CM-clique_modularity.algorithm.KJ.KJ Result");
            }

            modularityResult(var8, var5);
            CPP.process(var5, var9, var11);
        } catch (FileNotFoundException var12) {
            var12.printStackTrace();
        }

    }

    private static void benchmarkGraph(int var0, int var1, int var2, double var3, double var5, double var7, int var9, int var10, int var11, int var12) {
        String var14 = "groups-temp.txt";
        String var15 = "temp.txt";
        String var16 = "temp.txt";
        String var17 = "cluster-temp(gn).txt";
        int var18 = var0;

        int var19;
        try {
            var19 = GG.gen_benchmark(var0, var1, var2, var3, var5, var7, var9, var10, var15, var14);

            for(int var20 = 0; var20 < 2; ++var20) {
                BK.communityResults(var16, (String)null, var18, var19);
            }
        } catch (FileNotFoundException var25) {
            var25.printStackTrace();
        }

        for(int var13 = 0; var13 < var12; ++var13) {
            try {
                System.out.println("--------------------Generating random graph " + var13 + "--------------------");
                var19 = GG.gen_benchmark(var0, var1, var2, var3, var5, var7, var9, var10, var15, var14);
                System.out.println("--------------------clique_modularity.algorithm.BK.BK algorithm--------------------");
                BK.communityResults(var16, (String)null, var18, var19);
                System.out.println("--------------------clique_modularity.algorithm.KJ.KJ algorithm--------------------");
                KJ.communityResults(var16, (String)null, var18, var19);
                System.out.println("--------------------clique_modularity.util.metric.Omega Index-------------------");
                var17 = "mergeCommunities.txt";
                omegaResult(var14, var17, var18);
                var17 = "MCQresults.txt";
                omegaResult(var14, var17, var18);
            } catch (FileNotFoundException var24) {
                var24.printStackTrace();
            }
        }

    }

    private static void omegaResult(String var0, String var1, int var2) {
        Vector var3 = new Vector();
        Vector var4 = new Vector();
        int var5 = Omega.readCount(var3, var0, var2);
        int var6 = Omega.readCount(var4, var1, var2);
        double var7 = Omega.evaluateOmega(var3, var4, (long)var2, var5, var6);
        omegaIndex.add(Double.valueOf(var7));
        System.out.println(var0 + ", " + var1 + ", size " + var2);
        System.out.println("clique_modularity.util.metric.Omega is " + var7);
    }

    private static double omegaBGLL(String var0, String var1, int var2) {
        Vector var3 = new Vector();
        Vector var4 = new Vector();
        int var5 = Omega.readCount(var3, var0, var2);
        int var6 = Omega.readCount(var4, var1, var2);
        double var7 = Omega.evaluateOmega(var3, var4, (long)var2, var5, var6);
        return var7;
    }

    private static void modularityResult(String var0, String var1) {
        double var2 = Modularity.modularity(var0, var1);
        modularities.add(Double.valueOf(var2));
        System.out.println("clique_modularity.util.metric.Modularity is " + var2);
    }

    private static void printOmega() {
        for(int var0 = 0; var0 < omegaIndex.size(); ++var0) {
            System.out.println(omegaIndex.get(var0));
        }

        System.out.println();
    }

    private static void avgOmegaOthers(int var0) {
        for(int var1 = 0; var1 < 5; ++var1) {
            int var2 = 0;

            double var4;
            for(var4 = 0.0D; var2 < omegaIndex.size(); var2 += 5) {
                var4 += ((Double)omegaIndex.get(var1 + var2)).doubleValue();
            }

            double var6 = var4 / (double)var0;
            if(var1 == 0) {
                System.out.println("The clique_modularity.util.metric.Omega Index of CNM: " + var6);
            } else if(var1 == 1) {
                System.out.println("The clique_modularity.util.metric.Omega Index of PL: " + var6);
            } else if(var1 == 2) {
                System.out.println("The clique_modularity.util.metric.Omega Index of WT: " + var6);
            } else if(var1 == 3) {
                System.out.println("The clique_modularity.util.metric.Omega Index of clique_modularity.CM-clique_modularity.algorithm.BK.BK: " + var6);
            } else if(var1 == 4) {
                System.out.println("The clique_modularity.util.metric.Omega Index of clique_modularity.CM-clique_modularity.algorithm.KJ.KJ: " + var6);
            }
        }

    }

    private static void avgOmegaCM(int var0) {
        for(int var1 = 0; var1 < 2; ++var1) {
            int var2 = 0;

            double var3;
            for(var3 = 0.0D; var2 < omegaIndex.size(); var2 += 2) {
                var3 += ((Double)omegaIndex.get(var1 + var2)).doubleValue();
            }

            double var5 = var3 / (double)var0;
            if(var1 == 0) {
                System.out.println("The clique_modularity.util.metric.Omega Index of clique_modularity.CM-clique_modularity.algorithm.BK.BK: " + var5);
            } else {
                System.out.println("The clique_modularity.util.metric.Omega Index of clique_modularity.CM-clique_modularity.algorithm.KJ.KJ: " + var5);
            }
        }

    }
}
