package conga;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.PrintStream;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.TreeSet;

public class CONGA {
    static String welcome1 = "***************************************";
    static String welcome2 = "* conga.CONGA v1.67  (c) Steve Gregory 2012 *";
    static boolean screen = true;
    static boolean show = true;
    static boolean debug = false;
    static String clustPrefix = "clusters-";
    static String splitPrefix = "split-";
    static String vertexPrefix = "vertex-";
    static String clusteringPrefix = "clustering-";
    static String dFile = "debug.txt";
    static PrintStream dStream;
    static int nSplit;
    static float[][] tempMatrix;
    static int tempMatrixSize = 0;
    static long timeBegin;
    static long timeEnd;
    static Random rand = null;
    static boolean useLatest = true;
    static float eBetweennessFirst;
    static LinkedList<Pair> newEdges = new LinkedList();
    static boolean csv = true;
    static boolean mat = false;
    static boolean statsOnly = false;

    public CONGA() {
    }

    public static void main(String[] var0) {
        boolean var1 = false;
        boolean var2 = false;
        int var3 = 0;
        int var4 = 0;
        int var5 = 0;
        int var6 = 0;
        int var7 = 1;
        int var8 = 0;
        int var9 = 0;
        int var10 = 2147483647;
        int var11 = 2147483647;
        int var12 = 0;
        double var13 = 0.0D;
        double var15 = 0.0D;
        boolean var17 = false;
        boolean var18 = false;
        String var19 = null;
        boolean var20 = false;
        int var21 = 1;
        HashSet var22 = new HashSet();
        String var23 = null;
        String var24 = null;
        String var25 = "";
        int var26 = Array.getLength(var0);
        if(var26 < 1) {
            printUsageAndExit();
        }

        String var27 = var0[0];
        int var29 = 1;

        while(true) {
            while(var29 < var26) {
                String var28 = var0[var29++];
                if(var28.equals("-n")) {
                    var9 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-f")) {
                    var23 = var0[var29++];
                } else if(var28.equals("-d")) {
                    var10 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-w")) {
                    var18 = true;
                    var19 = var0[var29++];
                } else if(var28.equals("-v")) {
                    var22.add(var0[var29++]);
                } else if(var28.equals("-GN")) {
                    var22.add("off");
                } else if(var28.equals("-e")) {
                    var25 = "e";
                } else if(var28.equals("-r")) {
                    var1 = true;
                } else if(var28.equals("-s")) {
                    show = false;
                } else if(var28.equals("-mem")) {
                    var2 = true;
                } else if(var28.equals("-vad")) {
                    var3 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-ov")) {
                    var4 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-m")) {
                    var5 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-mo")) {
                    var6 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-inc")) {
                    var7 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-dia")) {
                    var8 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-cd")) {
                    var15 = Double.parseDouble(var0[var29++]);
                } else if(var28.equals("-g")) {
                    var24 = var0[var29++];
                } else if(var28.equals("-h")) {
                    var22.add("reg");
                    var11 = Integer.parseInt(var0[var29++]);
                } else if(var28.equals("-sp")) {
                    var22.add("reg");
                    var21 = Integer.parseInt(var0[var29++]);
                    var12 = Integer.parseInt(var0[var29++]);
                    if(var12 > 9) {
                        var12 = 2147483647;
                    }

                    var13 = Double.parseDouble(var0[var29++]);
                } else if(var28.equals("-peacock")) {
                    var22.add("reg");
                    var13 = Double.parseDouble(var0[var29++]);
                    var12 = -1;
                } else if(var28.equals("-br")) {
                    var17 = true;
                } else if(var28.equals("-so")) {
                    statsOnly = true;
                } else if(var28.equals("-fuzzy")) {
                    var20 = true;
                } else if(var28.equals("-sd")) {
                    var22.add("deg");
                } else if(!var28.equals("-sb")) {
                    if(var28.equals("-random")) {
                        long var37 = Long.parseLong(var0[var29++]);
                        rand = new Random(var37);
                    } else {
                        printUsageAndExit();
                    }
                } else {
                    int var30 = Integer.parseInt(var0[var29++]);

                    for(int var33 = 4; var33 <= var30; ++var33) {
                        float var31 = sb_vs_vb(var33);
                        float var32 = (float)(var33 * (var33 - 1));
                        System.out.println(var33 + ": " + var31 / var32);
                    }

                    System.exit(1);
                }
            }

            String var38 = (new File(var27)).getName();
            String var39 = clusteringPrefix + var38;
            String var40 = clustPrefix + var38;
            String var41 = splitPrefix + var38;
            String var34 = vertexPrefix + var38;
            screen = true;

            try {
                clusterGraph1(var9, var23, var10, var22, var1, var2, var5, var6, var7, var4, var3, var8, var24, var27, var39, var40, var41, var34, var25, var11, var15, var21, var12, var13, var17, var18, var19, var20);
            } catch (Exception var36) {
                sOutput("clusterGraph1 error: " + var36.toString());
                var36.printStackTrace();
                System.exit(1);
            }

            return;
        }
    }

    public static Stats clusterGraph(int var0, HashSet<String> var1, String var2, String var3, String var4, String var5, String var6, int var7, double var8, int var10, int var11, double var12, boolean var14, boolean var15, String var16, boolean var17) {
        try {
            return clusterGraph1(var0, (String)null, 2147483647, var1, true, true, 0, 0, 1, 0, 0, 0, (String)null, var2, clusteringPrefix + "temp.txt", var3, var4, var5, var6, var7, var8, var10, var11, var12, var14, var15, var16, var17);
        } catch (Exception var19) {
            sOutput("clusterGraph error: " + var19.toString());
            var19.printStackTrace();
            return null;
        }
    }

    private static Stats clusterGraph1(int var0, String var1, int var2, HashSet<String> var3, boolean var4, boolean var5, int var6, int var7, int var8, int var9, int var10, int var11, String var12, String var13, String var14, String var15, String var16, String var17, String var18, int var19, double var20, int var22, int var23, double var24, boolean var26, boolean var27, String var28, boolean var29) throws Exception {
        float var30 = 0.0F;
        HashMap var32 = null;
        boolean var33 = var23 < 0;
        sOutput(welcome1);
        sOutput(welcome2);
        sOutput(welcome1);
        sOutput("Graph file = " + var13);
        timeBegin = (new Date()).getTime();
        if(var27) {
            if(!var18.equals("e")) {
                System.err.println("Weighted networks must use \"list of edges\" (-e) format");
                System.exit(1);
            }

            if(!var3.contains("off") && !var28.equals("min") && !var28.equals("max") && !var28.equals("mean") && Float.parseFloat(var28) <= 0.0F) {
                System.err.println("conga.Split weight must be positive, or min or mean or max");
                System.exit(1);
            }

            var32 = new HashMap();
        }

        HashMap var31;
        if(var18.equals("e")) {
            sOutput("Input format = list of edges [-e to change]");
            var31 = readGraphEdges(var13, var12, false, var32);
            var30 = getSplitWeight(var28, var32);
        } else {
            sOutput("Input format = conga.CONGA native format [-e to change]");
            var31 = readGraph(var13, var12, var2);
        }

        if(var12 == null) {
            sOutput("No filter file [-g to change]");
        } else {
            sOutput("Filter file = " + var12 + " [-g to change]");
        }

        Stats var34 = new Stats();
        var34.nEdges = checkGraph(var31);
        var34.initialGraphSize = var31.size();
        if(var0 == 0) {
            sOutput("Will not show any clustering [-n to change]");
        } else {
            sOutput("Will show clustering for " + var0 + " clusters [-n to change]");
            if(var1 != null) {
                sOutput("Will only show cluster containing " + var1 + " [-f to change]");
            } else {
                sOutput("Will show all clusters in clustering [-f to change]");
            }
        }

        showOption(var6, "modularity", "m");
        showOption(var7, "modularity (overlap)", "mo");
        showOption(var9, "overlap", "ov");
        showOption(var10, "vad", "vad");
        showOption(var11, "diameter", "dia");
        ClusterInfo var35 = null;
        if(var5 || var33) {
            var35 = new ClusterInfo();
        }

        if(var33) {
            sOutput("Splitting vertices using conga.CONGA");
            findClusters(var31, var14, var16, var17, var3, var19, var0, var34, var35, var22, var23, var24, var26, var32, var30);
        } else {
            if((new File(var14)).exists() && !var4 && !var5) {
                sOutput("Retrieving clustering previously computed for " + var13);
                sOutput("Make sure that " + var13 + " and filter file (if any) are unchanged");
            } else {
                sOutputN("Will find clusters using ");
                if(var3.isEmpty()) {
                    sOutputN("conga.CONGA");
                } else if(var3.size() == 1 && var3.contains("off")) {
                    sOutputN("GN");
                } else {
                    sOutputN(var3 + "");
                }

                sOutput(" algorithm [-GN to change]");
                sOutput("\n==================== Finding clusters ====================");
                findClusters(var31, var14, var16, var17, var3, var19, var0, var34, var35, var22, var23, var24, var26, var32, var30);
            }

            sOutput("\n==================== Results =============================");
            constructShowClusters(var14, var0, var1, var6, var7, var8, var9, var10, var11, var15, var34, var35, var20, var29);
        }

        long var36 = (new Date()).getTime();
        var34.time5 = var36 - timeEnd;
        var34.time = var36 - timeBegin;
        if(screen) {
            try {
                sOutput("\n==================== Statistics ==========================");
                var34.showStats("stats.txt");
            } catch (Exception var39) {
                System.out.println("stats error: ");
                System.out.println(var39.toString());
                throw new Exception();
            }
        }

        return var34;
    }

    private static float getSplitWeight(String var0, HashMap<String, HashMap<String, Float>> var1) {
        float var2 = 0.0F;
        float var4 = 3.4028235E38F;
        float var5 = 0.0F;
        float var6 = 0.0F;
        int var7 = 0;
        if(var1 == null) {
            var2 = 0.0F;
        } else if(!var0.equals("min") && !var0.equals("max") && !var0.equals("mean")) {
            var2 = Float.parseFloat(var0);
        } else {
            Iterator var9 = var1.keySet().iterator();

            while(var9.hasNext()) {
                HashMap var8 = (HashMap)var1.get(var9.next());

                for(Iterator var10 = var8.keySet().iterator(); var10.hasNext(); ++var7) {
                    float var3 = ((Float)var8.get(var10.next())).floatValue();
                    if(var3 < var4) {
                        var4 = var3;
                    }

                    if(var3 > var5) {
                        var5 = var3;
                    }

                    var6 += var3;
                }
            }

            if(var0.equals("min")) {
                var2 = var4;
            } else if(var0.equals("max")) {
                var2 = var5;
            } else if(var0.equals("mean")) {
                var2 = var6 / (float)var7;
            }
        }

        return var2;
    }

    private static void showOption(int var0, String var1, String var2) {
        sOutputN("Will ");
        if(var0 == 0) {
            sOutputN("not ");
        }

        sOutput("show " + var1 + " of each clustering [-" + var2 + " to change]");
    }

    private static boolean sameCluster(String var0, String var1, HashSet<HashSet<String>> var2) {
        Iterator var4 = var2.iterator();

        HashSet var3;
        do {
            if(!var4.hasNext()) {
                return false;
            }

            var3 = (HashSet)var4.next();
        } while(!var3.contains(var0) || !var3.contains(var1));

        return true;
    }

    private static int checkGraph(HashMap<String, HashSet<String>> var0) throws Exception {
        int var1 = 0;
        int var2 = 0;
        new ArrayList();
        Iterator var6 = var0.keySet().iterator();

        while(var6.hasNext()) {
            String var3 = (String)var6.next();
            ++var2;

            for(Iterator var7 = ((HashSet)var0.get(var3)).iterator(); var7.hasNext(); ++var1) {
                String var4 = (String)var7.next();
                if(var0.get(var4) == null || !((HashSet)var0.get(var4)).contains(var3)) {
                    throw new Exception("Asymmetric graph: " + var3 + "/" + var4);
                }
            }
        }

        int var8 = var1 / 2;
        if(screen) {
            sOutput("Finished reading graph: " + var2 + " vertices, " + var8 + " edges");
        }

        return var8;
    }

    private static void statsGraph(List<HashSet<Integer>> var0) {
        int var2 = -2147483648;
        int var3 = 2147483647;
        int var4 = 0;
        int var5 = 0;
        ArrayList var8 = new ArrayList();

        for(int var6 = 0; var6 < var0.size(); ++var6) {
            ++var4;
            int var1 = ((HashSet)var0.get(var6)).size();
            if(var1 < var3) {
                var3 = var1;
            }

            if(var1 > var2) {
                var2 = var1;
            }

            int var7;
            for(Iterator var9 = ((HashSet)var0.get(var6)).iterator(); var9.hasNext(); var7 = ((Integer)var9.next()).intValue()) {
                ;
            }

            var5 += var1;

            while(var8.size() <= var1) {
                var8.add(Integer.valueOf(0));
            }

            var8.set(var1, Integer.valueOf(((Integer)var8.get(var1)).intValue() + 1));
        }

        if(screen) {
            System.out.print("n=" + var4 + " m=" + var5 / 2 + " Degree: " + var3 + "-" + String.format("%.3f", new Object[]{Double.valueOf((double)var5 / (double)var4)}) + "-" + var2 + ":");

            for(int var10 = 0; var10 < var8.size(); ++var10) {
                System.out.print(" " + var8.get(var10));
            }
        }

    }

    public static HashMap<String, HashSet<String>> readGraphEdges(String var0, String var1, boolean var2, HashMap<String, HashMap<String, Float>> var3) {
        HashMap var10 = new HashMap();
        HashSet var11 = new HashSet();

        try {
            BufferedReader var4;
            String var5;
            if(var1 != null && (new File(var1)).exists()) {
                var4 = new BufferedReader(new FileReader(var1));

                while((var5 = var4.readLine()) != null) {
                    if(!var5.equals("")) {
                        var11.add(var5);
                    }
                }

                var4.close();
            }

            if((new File(var0)).exists()) {
                var4 = new BufferedReader(new FileReader(var0));
                int var12 = 0;

                while((var5 = var4.readLine()) != null) {
                    ++var12;
                    if(!var5.equals("") && !var5.startsWith("#")) {
                        String[] var8 = var5.split("[ \t]");
                        if(Array.getLength(var8) < 2) {
                            System.err.println("Missing vertices on line " + var12 + " of " + var0);
                            System.exit(1);
                        }

                        String var6 = var8[0];
                        String var7 = var8[1];
                        if(!var11.contains(var6) && !var11.contains(var7)) {
                            if(var6.equals(var7)) {
                                sOutput("Ignoring self edge: " + var6 + "/" + var7);
                            } else {
                                if(var10.get(var6) == null) {
                                    var10.put(var6, new HashSet());
                                }

                                if(!((HashSet)var10.get(var6)).add(var7)) {
                                    sOutput("Duplicate edge: " + var6 + "/" + var7);
                                }

                                if(!var2) {
                                    if(var10.get(var7) == null) {
                                        var10.put(var7, new HashSet());
                                    }

                                    ((HashSet)var10.get(var7)).add(var6);
                                }

                                if(var3 != null) {
                                    if(Array.getLength(var8) < 3) {
                                        System.err.println("Missing weight on line " + var12 + " of " + var0);
                                        System.exit(1);
                                    }

                                    float var9 = Float.parseFloat(var8[2]);
                                    if(var6.compareTo(var7) < 0) {
                                        addWeight(var6, var7, var9, var3, var12, var0);
                                    } else {
                                        addWeight(var7, var6, var9, var3, var12, var0);
                                    }
                                }
                            }
                        }
                    }
                }

                var4.close();
            } else {
                System.out.println("Graph file " + var0 + " not found");
                System.exit(1);
            }
        } catch (Exception var13) {
            System.out.println("readGraphEdges error: ");
            System.out.println(var13.toString());
            System.exit(1);
        }

        return var10;
    }

    public static void addWeight(String var0, String var1, float var2, HashMap<String, HashMap<String, Float>> var3, int var4, String var5) {
        if(!var3.containsKey(var0)) {
            var3.put(var0, new HashMap());
        }

        if(((HashMap)var3.get(var0)).containsKey(var1)) {
            if(((Float)((HashMap)var3.get(var0)).get(var1)).floatValue() != var2) {
                System.err.println("Wrong weight on line " + var4 + " of " + var5);
                System.exit(1);
            }
        } else {
            ((HashMap)var3.get(var0)).put(var1, Float.valueOf(var2));
        }

    }

    public static HashMap<String, HashSet<String>> readGraph(String var0, String var1, int var2) {
        String var6 = null;
        String[] var7 = null;
        HashMap var8 = new HashMap();
        HashSet var9 = new HashSet();

        try {
            BufferedReader var3;
            String var5;
            if(var1 != null && (new File(var1)).exists()) {
                var3 = new BufferedReader(new FileReader(var1));

                while((var5 = var3.readLine()) != null) {
                    if(!var5.equals("")) {
                        var9.add(var5);
                    }
                }

                var3.close();
            }

            if((new File(var0)).exists()) {
                var3 = new BufferedReader(new FileReader(var0));

                label64:
                while(true) {
                    while(true) {
                        do {
                            if((var5 = var3.readLine()) == null) {
                                break label64;
                            }
                        } while(var5.equals(""));

                        var7 = var5.split(" ");
                        int var4;
                        if(var7[0].equals("--")) {
                            if(var6 == null) {
                                System.out.println("Bad graph file: " + var0);
                                System.exit(1);
                            }

                            if(Array.getLength(var7) >= 3) {
                                var4 = Integer.parseInt(var7[2]);
                            } else {
                                var4 = 0;
                            }

                            neighbour(var7[1], var6, var8, var4, var2, var9);
                        } else {
                            if(Array.getLength(var7) > 2 && !var7[2].equals("(complete)") && var7[2].startsWith("(")) {
                                break label64;
                            }

                            var6 = var7[0];
                            if(Array.getLength(var7) >= 3 && var7[2].equals("(complete)")) {
                                var4 = Integer.parseInt(var7[1]);
                            } else {
                                var4 = 0;
                            }

                            vertex(var6, var8, var4, var2, var9);
                        }
                    }
                }

                var3.close();
            } else {
                System.out.println("Graph file " + var0 + " not found");
                System.exit(1);
            }
        } catch (Exception var11) {
            System.out.println("readGraph error: ");
            System.out.println(var11.toString());
            System.exit(1);
        }

        return var8;
    }

    private static void vertex(String var0, HashMap<String, HashSet<String>> var1, int var2, int var3, HashSet<String> var4) {
        if(var2 <= var3 && !var4.contains(var0)) {
            var1.put(var0, new HashSet());
        }

    }

    private static void neighbour(String var0, String var1, HashMap<String, HashSet<String>> var2, int var3, int var4, HashSet<String> var5) {
        HashSet var6 = (HashSet)var2.get(var1);
        if(var6 != null) {
            if(var0.equals(var1)) {
                System.out.println("Ignoring self edge: " + var0 + "/" + var1);
            }

            if(var3 <= var4 && !var5.contains(var0) && !var0.equals(var1)) {
                var6.add(var0);
            }
        }

    }

    public static void compactGraph(HashMap<String, HashSet<String>> var0, List<HashSet<Integer>> var1, HashMap<String, HashMap<String, Float>> var2, List<HashMap<Integer, Float>> var3, List<String> var4) {
        Set var13 = var0.keySet();
        HashMap var14 = new HashMap();
        int var9 = 0;

        String var5;
        Iterator var11;
        for(var11 = var13.iterator(); var11.hasNext(); ++var9) {
            var5 = (String)var11.next();
            var4.add(var5);
            var14.put(var5, Integer.valueOf(var9));
            var1.add(new HashSet());
        }

        int var7;
        if(var3 != null) {
            for(var7 = 0; var7 < var9; ++var7) {
                var3.add(new HashMap());
            }
        }

        var11 = var13.iterator();

        while(var11.hasNext()) {
            var5 = (String)var11.next();
            var7 = ((Integer)var14.get(var5)).intValue();
            Iterator var12 = ((HashSet)var0.get(var5)).iterator();

            while(var12.hasNext()) {
                String var6 = (String)var12.next();
                int var8;
                if(var14.containsKey(var6)) {
                    var8 = ((Integer)var14.get(var6)).intValue();
                } else {
                    var4.add(var6);
                    var14.put(var6, Integer.valueOf(var9));
                    var8 = var9++;
                }

                ((HashSet)var1.get(var7)).add(Integer.valueOf(var8));
                if(var3 != null) {
                    float var10;
                    if(var5.compareTo(var6) < 0) {
                        var10 = ((Float)((HashMap)var2.get(var5)).get(var6)).floatValue();
                    } else {
                        var10 = ((Float)((HashMap)var2.get(var6)).get(var5)).floatValue();
                    }

                    if(var7 < var8) {
                        ((HashMap)var3.get(var7)).put(Integer.valueOf(var8), Float.valueOf(var10));
                    } else {
                        ((HashMap)var3.get(var8)).put(Integer.valueOf(var7), Float.valueOf(var10));
                    }
                }
            }
        }

    }

    private static void findClusters(HashMap<String, HashSet<String>> var0, String var1, String var2, String var3, HashSet<String> var4, int var5, int var6, Stats var7, ClusterInfo var8, int var9, int var10, double var11, boolean var13, HashMap<String, HashMap<String, Float>> var14, float var15) throws Exception {
        ArrayList var16 = new ArrayList();
        ArrayList var17 = new ArrayList();
        ArrayList var18 = null;
        if(var14 != null) {
            var18 = new ArrayList();
        }

        compactGraph(var0, var16, var14, var18, var17);
        if(var8 != null) {
            var8.graphSize = var16.size();
            var8.vertexName = var17;
        }

        try {
            if(var5 > 0) {
                cluster(var16, var18, var17, var1, var4, var5, var6, var7, var8, var9, var10, var11, var13, var15);
            }

            if(var10 < 0) {
                writeSplitGraph(var16, var18, var17, var2, var3, csv, mat);
            }
        } catch (Exception var20) {
            System.out.println("cluster error: ");
            System.out.println(var20.toString());
            var20.printStackTrace();
            System.exit(1);
        }

    }

    public static void writeSplitGraph(List<HashSet<Integer>> var0, List<HashMap<Integer, Float>> var1, List<String> var2, String var3, String var4, boolean var5, boolean var6) {
        float var9 = 0.0F;

        try {
            PrintStream var11 = new PrintStream(new FileOutputStream(var3));

            int var7;
            int var8;
            Iterator var10;
            for(var7 = 0; var7 < var0.size(); ++var7) {
                var10 = ((HashSet)var0.get(var7)).iterator();

                while(var10.hasNext()) {
                    var8 = ((Integer)var10.next()).intValue();
                    if(var7 < var8) {
                        var11.print(var7 + "\t" + var8);
                        if(var1 == null) {
                            var11.println();
                        } else {
                            var11.println("\t" + ((HashMap)var1.get(var7)).get(Integer.valueOf(var8)));
                        }
                    }
                }
            }

            var11.close();
            String var12;
            if(var5) {
                var12 = var3.substring(0, var3.lastIndexOf(".")) + ".csv";
                var11 = new PrintStream(new FileOutputStream(var12));
                var7 = 0;

                while(true) {
                    if(var7 >= var0.size()) {
                        var11.close();
                        break;
                    }

                    var10 = (new TreeSet((Collection)var0.get(var7))).iterator();
                    var11.print(var7 + 1);

                    while(var10.hasNext()) {
                        var8 = ((Integer)var10.next()).intValue();
                        var11.print("," + (var8 + 1));
                    }

                    var11.println();
                    ++var7;
                }
            }

            if(var6) {
                var12 = var3.substring(0, var3.lastIndexOf(".")) + ".mtx";
                var11 = new PrintStream(new FileOutputStream(var12));
                float[] var13 = new float[var0.size()];
                var7 = 0;

                while(true) {
                    if(var7 >= var0.size()) {
                        var11.close();
                        break;
                    }

                    for(var8 = 0; var8 < var0.size(); ++var8) {
                        var13[var8] = 0.0F;
                    }

                    for(var10 = ((HashSet)var0.get(var7)).iterator(); var10.hasNext(); var13[var8] = var9) {
                        var8 = ((Integer)var10.next()).intValue();
                        if(var1 == null) {
                            var9 = 1.0F;
                        } else if(var7 < var8) {
                            var9 = ((Float)((HashMap)var1.get(var7)).get(Integer.valueOf(var8))).floatValue();
                        } else {
                            var9 = ((Float)((HashMap)var1.get(var8)).get(Integer.valueOf(var7))).floatValue();
                        }
                    }

                    for(var8 = 0; var8 < var0.size(); ++var8) {
                        if(var13[var8] == 0.0F) {
                            var11.print("0 ");
                        } else if(var13[var8] == 1.0F) {
                            var11.print("1 ");
                        } else {
                            var11.print(var13[var8] + " ");
                        }
                    }

                    var11.println();
                    ++var7;
                }
            }

            var11 = new PrintStream(new FileOutputStream(var4));

            for(var7 = 0; var7 < var0.size(); ++var7) {
                var11.println((String)var2.get(var7));
            }

            var11.close();
        } catch (Exception var14) {
            System.out.println("writeSplitGraph error: ");
            System.out.println(var14.toString());
            var14.printStackTrace();
            System.exit(1);
        }

    }

    private static float sb_vs_vb(int var0) {
        ArrayList var1 = new ArrayList();
        HashSet var2 = new HashSet();
        ArrayList var3 = new ArrayList();
        ArrayList var4 = new ArrayList();
        ArrayList var5 = new ArrayList();
        ArrayList var6 = new ArrayList();
        HashSet var7 = new HashSet();
        HashSet var8 = new HashSet();
        var8.add(Integer.valueOf(0));
        var1.add(new HashSet());

        for(int var9 = 1; var9 <= var0; ++var9) {
            var1.add(var8);
            ((HashSet)var1.get(0)).add(Integer.valueOf(var9));
        }

        Between var10 = new Between(var1, var7, (List)null);
        var2.add(Integer.valueOf(0));
        setSizes(var1.size(), var10, var3, var4, var5, var6);
        updateComps(var2, var10, var1, 2147483647, var7, false, var3, var4, var5, var6);
        return var10.bestVertex().betweenness;
    }

    private static void cluster(List<HashSet<Integer>> var0, List<HashMap<Integer, Float>> var1, List<String> var2, String var3, HashSet<String> var4, int var5, int var6, Stats var7, ClusterInfo var8, int var9, int var10, double var11, boolean var13, float var14) throws Exception {
        boolean var19 = var10 != 0;
        boolean var20 = var10 < 0;
        PrintStream var21 = null;
        TreeSet var22 = null;
        TreeSet var23 = null;
        TreeSet var24 = null;
        HashSet var25 = new HashSet();
        HashSet var26 = new HashSet();
        ArrayList var27 = new ArrayList();
        boolean var35 = false;
        nSplit = 0;
        int var40 = var0.size();
        var7.clustering = true;
        Between var41 = new Between(var0, var4, var1);
        if(debug) {
            dStream = new PrintStream(new FileOutputStream(dFile));
        }

        int var31;
        if(var8 == null) {
            var21 = new PrintStream(new FileOutputStream(var3));
            var21.println(var40);

            for(var31 = 0; var31 < var40; ++var31) {
                var21.println((String)var2.get(var31));
            }
        }

        for(var31 = 0; var31 < var40; ++var31) {
            var26.add(Integer.valueOf(var31));
            Iterator var30 = ((HashSet)var0.get(var31)).iterator();

            while(var30.hasNext()) {
                int var32 = ((Integer)var30.next()).intValue();
                if(var31 < var32) {
                    cEdge(var31, var32, var21, var8);
                }
            }
        }

        if(var8 == null) {
            var21.println();
        }

        while(!var26.isEmpty()) {
            var31 = ((Integer)var26.iterator().next()).intValue();
            var22 = componentOf(var31, var0);
            var26.removeAll(var22);
            var27.add(Integer.valueOf(var22.size()));
            var25.add(Integer.valueOf(var31));
        }

        if(var6 > 0 && var6 < var27.size()) {
            System.out.println("Number of clusters must not be less than " + var27.size() + " (the number of components)");
            System.exit(1);
        }

        long var42 = (new Date()).getTime();
        var7.time1 = var42 - timeBegin;
        ArrayList var44 = new ArrayList();
        ArrayList var45 = new ArrayList();
        ArrayList var46 = new ArrayList();
        ArrayList var47 = new ArrayList();
        setSizes(var40, var41, var44, var45, var46, var47);
        updateComps(var25, var41, var0, var5, var4, var13, var44, var45, var46, var47);
        if(statsOnly) {
            ArrayList var48 = var41.getSplitBetweenness(var2);
            ArrayList var49 = var41.getVertexBetweenness(var22, var0, var2);

            for(var31 = 0; var31 < var48.size(); ++var31) {
                System.out.println(var31 + "\t" + (String)var2.get(var31) + "\t" + ((HashSet)var0.get(var31)).size() + "\t" + var49.get(var31) + "\t" + var48.get(var31));
            }

            System.exit(1);
        }

        long var53 = (new Date()).getTime();
        var7.time2 = var53 - var42;
        boolean var15 = var4.contains("reg");
        boolean var16 = var4.contains("deg");
        byte var50 = 1;
        Edge var38 = var41.bestEdge();
        float var36 = wEdgeBetweenness(var38, var1);
        eBetweennessFirst = var36;

        while(true) {
            while(var38 != null) {
                boolean var17 = false;
                Vertex var39 = var41.bestVertex();
                float var37 = wVertexBetweenness(var39, var14);
                var36 = wEdgeBetweenness(var38, var1);
                if(updatePhase(var50, var37, var36, var19, var11, var0, var1, var2, var9, var13, var53, var7, var14)) {
                    if(var20) {
                        var38 = null;
                        continue;
                    }

                    var50 = 2;
                    var5 = var10;
                    if(var15) {
                        var25 = new HashSet();
                        var25.add(Integer.valueOf(var39.vertex));
                        var41.initComp(componentOf(var39.vertex, var0), var0);
                        updateComps(var25, var41, var0, var10, var4, var13, var44, var45, var46, var47);
                    }
                }

                List var28;
                int var33;
                int var34;
                if(!var19 && detLess(var36, var37) || var19 && var50 == 1) {
                    var33 = var39.vertex;
                    if(var15) {
                        var23 = new TreeSet();
                        var28 = regionOf(var33, var5, var0, var23);
                        if(smallRegion(var28, var23, var0)) {
                            compBetweenness(var41, var0, var44, var45, var46, var47, var5, var23, true, -1);
                            var17 = true;
                        }
                    }

                    setSizes(var0.size() + 1, var41, var44, var45, var46, var47);
                    var34 = splitVertex(var33, var39.split, var41, var0, var1, var2, var17, var23, var7);
                    showStep(1, var33, var34, var37, var23, var17, var2, var16, var0);
                } else {
                    var33 = var38.vertex1;
                    var34 = var38.vertex2;
                    if(var15) {
                        var23 = new TreeSet();
                        var24 = new TreeSet();
                        var28 = regionOf(var33, var5 - 1, var0, var23);
                        List var29 = regionOf(var34, var5 - 1, var0, var24);
                        var23.addAll(var24);
                        if(smallRegion(var28, var23, var0) || smallRegion(var29, var23, var0)) {
                            compBetweenness(var41, var0, var44, var45, var46, var47, var5, var23, true, -1);
                            var17 = true;
                        }
                    }

                    removeEdge(var33, var34, var41, var0, var1, var17, var7);
                    showStep(2, var33, var34, var36, var23, var17, var2, var16, var0);
                }

                update2Comps(var33, var34, var41, var0, var5, var17, var23, var21, var4, var13, var44, var45, var46, var47, var8);
                if(var19 && var50 == 1) {
                    newEdges.addLast(new Pair(var33, var34));
                }

                var38 = var41.bestEdge();
                ++var7.nBetweenPhase;
            }

            long var51 = (new Date()).getTime();
            var7.time3 = var51 - var53;
            var40 = var0.size();
            if(var8 == null) {
                var21.println("end");
                var21.println(var40);

                for(var31 = 0; var31 < var40; ++var31) {
                    var21.println((String)var2.get(var31));
                }

                var21.close();
            }

            if(debug) {
                dStream.close();
            }

            var7.finalGraphSize = var40;
            var7.compSizes = var27;
            timeEnd = (new Date()).getTime();
            var7.time4 = timeEnd - var51;
            return;
        }
    }

    private static float wEdgeBetweenness(Edge var0, List<HashMap<Integer, Float>> var1) {
        if(var1 == null) {
            return var0.betweenness;
        } else {
            int var2 = var0.vertex1;
            int var3 = var0.vertex2;
            float var4;
            if(var2 < var3) {
                var4 = ((Float)((HashMap)var1.get(var2)).get(Integer.valueOf(var3))).floatValue();
            } else {
                var4 = ((Float)((HashMap)var1.get(var3)).get(Integer.valueOf(var2))).floatValue();
            }

            return var0.betweenness / var4;
        }
    }

    private static float wVertexBetweenness(Vertex var0, float var1) {
        return var1 == 0.0F?var0.betweenness:var0.betweenness / var1;
    }

    private static boolean updatePhase(int var0, float var1, float var2, boolean var3, double var4, List<HashSet<Integer>> var6, List<HashMap<Integer, Float>> var7, List<String> var8, int var9, boolean var10, long var11, Stats var13, float var14) {
        boolean var15 = false;
        if(var3 && var0 == 1) {
            float var16;
            if(useLatest) {
                var16 = var2;
            } else {
                var16 = eBetweennessFirst;
            }

            if(var10 && (double)var1 < var4 || !var10 && (double)var1 < var4 * (double)var16) {
                var15 = true;
                if(var9 >= 1) {
                    addNewEdges(var6, var7, var8, var9, var14);
                }

                var13.time6 = (new Date()).getTime() - var11;
            }
        }

        return var15;
    }

    private static void addNewEdges(List<HashSet<Integer>> var0, List<HashMap<Integer, Float>> var1, List<String> var2, int var3, float var4) {
        int var6 = 0;
        HashMap var8 = new HashMap();
        HashMap var9 = new HashMap();

        int var7;
        while(!newEdges.isEmpty()) {
            Pair var5 = (Pair)newEdges.removeFirst();
            var7 = var5.value2;
            if(var3 != 1 && var3 != 11) {
                if(var3 == 2 || var3 == 3) {
                    if(var8.containsKey(Integer.valueOf(var5.value1))) {
                        var6 = ((Integer)var8.get(Integer.valueOf(var5.value1))).intValue();
                    } else {
                        var6 = var5.value1;
                    }

                    var8.put(Integer.valueOf(var7), Integer.valueOf(var6));
                    if(!var9.containsKey(Integer.valueOf(var6))) {
                        var9.put(Integer.valueOf(var6), new HashSet());
                        ((HashSet)var9.get(Integer.valueOf(var6))).add(Integer.valueOf(var6));
                    }

                    ((HashSet)var9.get(Integer.valueOf(var6))).add(Integer.valueOf(var7));
                }
            } else {
                var6 = var5.value1;
            }

            if(var3 != 3) {
                ((HashSet)var0.get(var6)).add(Integer.valueOf(var7));
                ((HashSet)var0.get(var7)).add(Integer.valueOf(var6));
                if(var1 != null) {
                    if(var6 < var7) {
                        ((HashMap)var1.get(var6)).put(Integer.valueOf(var7), Float.valueOf(var4));
                    } else {
                        ((HashMap)var1.get(var7)).put(Integer.valueOf(var6), Float.valueOf(var4));
                    }
                }
            }
        }

        if(var3 == 3) {
            Iterator var13 = var9.keySet().iterator();

            while(var13.hasNext()) {
                HashSet var10 = (HashSet)var9.get(var13.next());
                Iterator var11 = var10.iterator();

                while(var11.hasNext()) {
                    var6 = ((Integer)var11.next()).intValue();
                    Iterator var12 = var10.iterator();

                    while(var12.hasNext()) {
                        var7 = ((Integer)var12.next()).intValue();
                        if(var6 != var7) {
                            ((HashSet)var0.get(var6)).add(Integer.valueOf(var7));
                        }
                    }
                }
            }
        }

    }

    private static void showStep(int var0, int var1, int var2, float var3, TreeSet<Integer> var4, boolean var5, List<String> var6, boolean var7, List<HashSet<Integer>> var8) {
        if(screen && show) {
            if(var7) {
                statsGraph(var8);
            } else {
                boolean var9 = true;
                String var13 = String.format("%.2f", new Object[]{Float.valueOf(var3)});
                String var10;
                String var11;
                if(var9) {
                    var10 = (String)var6.get(var1);
                    var11 = (String)var6.get(var2);
                } else {
                    var10 = String.valueOf(var1);
                    var11 = String.valueOf(var2);
                }

                String var12;
                if(var0 == 1) {
                    var12 = "conga.Split";
                } else {
                    var12 = "Remove";
                }

                sOutputN(var12 + " " + var10 + "/" + var11 + "  " + var13);
                if(var4 != null) {
                    if(var5) {
                        sOutputN("  s" + var4.size());
                    } else {
                        sOutputN("  r" + var4.size());
                    }
                }
            }

            sOutput("");
        }

    }

    private static void setSizes(int var0, Between var1, List<HashSet<Integer>> var2, List<Integer> var3, List<Integer> var4, List<Float> var5) {
        var1.setSize(var0);

        while(var2.size() < var0) {
            var2.add((Object)null);
        }

        while(var3.size() < var0) {
            var3.add((Object)null);
        }

        while(var4.size() < var0) {
            var4.add((Object)null);
        }

        while(var5.size() < var0) {
            var5.add((Object)null);
        }

    }

    private static boolean update2Comps(int var0, int var1, Between var2, List<HashSet<Integer>> var3, int var4, boolean var5, TreeSet<Integer> var6, PrintStream var7, HashSet<String> var8, boolean var9, List<HashSet<Integer>> var10, List<Integer> var11, List<Integer> var12, List<Float> var13, ClusterInfo var14) {
        boolean var15 = var6 != null;
        boolean var16 = false;
        if(!var15) {
            var6 = componentOf(var0, var3);
            var16 = !var6.contains(Integer.valueOf(var1));
        }

        if(!var5) {
            var2.initComp(var6, var3);
        }

        compBetweenness(var2, var3, var10, var11, var12, var13, var4, var6, var15, 1);
        compVertices(var6, var2, var3, var8, var9);
        if(var16) {
            var6 = componentOf(var1, var3);
            var2.initComp(var6, var3);
            compBetweenness(var2, var3, var10, var11, var12, var13, var4, var6, false, 1);
            compVertices(var6, var2, var3, var8, var9);
        }

        cSplit(var0, var1, var7, var14);
        return var16;
    }

    private static void updateComps(HashSet<Integer> var0, Between var1, List<HashSet<Integer>> var2, int var3, HashSet<String> var4, boolean var5, List<HashSet<Integer>> var6, List<Integer> var7, List<Integer> var8, List<Float> var9) {
        Iterator var11 = var0.iterator();

        while(var11.hasNext()) {
            TreeSet var10 = componentOf(((Integer)var11.next()).intValue(), var2);
            compBetweenness(var1, var2, var6, var7, var8, var9, var3, var10, false, 1);
            compVertices(var10, var1, var2, var4, var5);
        }

    }

    private static void compBetweenness(Between var0, List<HashSet<Integer>> var1, List<HashSet<Integer>> var2, List<Integer> var3, List<Integer> var4, List<Float> var5, int var6, TreeSet<Integer> var7, boolean var8, int var9) {
        int[] var10 = new int[var7.size()];
        int[] var11 = new int[var7.size()];
        Iterator var12 = var7.iterator();

        while(var12.hasNext()) {
            betweenness(((Integer)var12.next()).intValue(), var0, var1, var2, var3, var4, var5, var10, var11, var6, var7, var8, var9);
        }

    }

    private static void betweenness(int var0, Between var1, List<HashSet<Integer>> var2, List<HashSet<Integer>> var3, List<Integer> var4, List<Integer> var5, List<Float> var6, int[] var7, int[] var8, int var9, TreeSet<Integer> var10, boolean var11, int var12) {
        int var28 = 0;
        byte var29 = 0;
        int var30 = 0;
        if(!var11 && var9 < 2147483647) {
            var10 = regionOf(var0, var9, var2);
        }

        Iterator var26 = var10.iterator();

        int var13;
        while(var26.hasNext()) {
            var13 = ((Integer)var26.next()).intValue();
            var4.set(var13, Integer.valueOf(-1));
            var3.set(var13, new HashSet());
        }

        var4.set(var0, Integer.valueOf(0));
        var5.set(var0, Integer.valueOf(1));
        int var32 = var29 + 1;
        var7[var29] = var0;

        int var14;
        int var19;
        while(var28 != var32) {
            var13 = var7[var28++];
            int var18 = ((Integer)var4.get(var13)).intValue();
            var19 = ((Integer)var5.get(var13)).intValue();
            var26 = ((HashSet)var2.get(var13)).iterator();

            while(var26.hasNext()) {
                var14 = ((Integer)var26.next()).intValue();
                if(var10.contains(Integer.valueOf(var14))) {
                    int var20 = ((Integer)var4.get(var14)).intValue();
                    if(var20 == -1) {
                        if(var18 < var9) {
                            var4.set(var14, Integer.valueOf(var18 + 1));
                            var5.set(var14, Integer.valueOf(var19));
                            var7[var32++] = var14;
                            ((HashSet)var3.get(var14)).add(Integer.valueOf(var13));
                            var6.set(var14, Float.valueOf(1.0F));
                            var8[var30++] = var14;
                        }
                    } else if(var20 == var18 + 1) {
                        var5.set(var14, Integer.valueOf(((Integer)var5.get(var14)).intValue() + var19));
                        ((HashSet)var3.get(var14)).add(Integer.valueOf(var13));
                    }
                }
            }
        }

        label61:
        while(var30 != 0) {
            --var30;
            var14 = var8[var30];
            float var24 = ((Float)var6.get(var14)).floatValue();
            var26 = ((HashSet)var3.get(var14)).iterator();

            while(true) {
                PB var21;
                float var23;
                do {
                    float var22;
                    do {
                        if(!var26.hasNext()) {
                            continue label61;
                        }

                        var13 = ((Integer)var26.next()).intValue();
                        var19 = ((Integer)var5.get(var13)).intValue();
                        var22 = var24 * (float)var19 / (float)((Integer)var5.get(var14)).intValue();
                        var23 = var22 * (float)var12;
                        BP var10000;
                        if(var13 < var14) {
                            var10000 = (BP)((HashMap)var1.betweenV.get(var13)).get(Integer.valueOf(var14));
                            var10000.newB += var23;
                        } else {
                            var10000 = (BP)((HashMap)var1.betweenV.get(var14)).get(Integer.valueOf(var13));
                            var10000.newB += var23;
                        }
                    } while(var13 == var0);

                    var6.set(var13, Float.valueOf(((Float)var6.get(var13)).floatValue() + var22));
                    var21 = var1.getPB(var13);
                } while(var21 == null);

                int var16;
                int var17;
                float var25;
                for(Iterator var27 = ((HashSet)var3.get(var13)).iterator(); var27.hasNext(); var21.matrix[var16][var17] += var25) {
                    int var15 = ((Integer)var27.next()).intValue();
                    var25 = var23 * (float)((Integer)var5.get(var15)).intValue() / (float)var19;
                    var17 = ((Integer)var21.entryOf.get(Integer.valueOf(var14))).intValue();
                    var16 = ((Integer)var21.entryOf.get(Integer.valueOf(var15))).intValue();
                    var21.matrix[var17][var16] += var25;
                }
            }
        }

    }

    private static void compVertices(TreeSet<Integer> var0, Between var1, List<HashSet<Integer>> var2, HashSet<String> var3, boolean var4) {
        Iterator var7 = var0.iterator();

        while(var7.hasNext()) {
            int var5 = ((Integer)var7.next()).intValue();
            Iterator var8 = ((HashSet)var2.get(var5)).iterator();

            while(var8.hasNext()) {
                int var6 = ((Integer)var8.next()).intValue();
                if(var5 < var6 && var0.contains(Integer.valueOf(var6))) {
                    var1.putEdge(var5, var6);
                }
            }

            if(!var3.contains("off")) {
                compVertex(Integer.valueOf(var5), var1, var2, var4);
            }
        }

    }

    public static boolean detLess(float var0, float var1) {
        return detLess(var0, var1, 0, 0, 0, 0);
    }

    public static boolean detLess(float var0, float var1, int var2, int var3) {
        return detLess(var0, var1, 0, 0, var2, var3);
    }

    public static boolean detLess(float var0, float var1, int var2, int var3, int var4, int var5) {
        return var0 < var1 || var0 == var1 && var2 < var3 || var0 == var1 && var2 == var3 && var4 < var5;
    }

    private static void removeEdge(int var0, int var1, Between var2, List<HashSet<Integer>> var3, List<HashMap<Integer, Float>> var4, boolean var5, Stats var6) {
        ((HashSet)var3.get(var0)).remove(Integer.valueOf(var1));
        ((HashSet)var3.get(var1)).remove(Integer.valueOf(var0));
        if(var5) {
            if(var0 < var1) {
                var2.removeEdge(var0, var1);
            } else {
                var2.removeEdge(var1, var0);
            }
        }

        ++var6.nEdgesRemoved;
    }

    private static int splitVertex(int var0, Split var1, Between var2, List<HashSet<Integer>> var3, List<HashMap<Integer, Float>> var4, List<String> var5, boolean var6, TreeSet<Integer> var7, Stats var8) {
        HashSet var11 = var1.value1;
        HashSet var12 = var1.value2;
        int var14 = var3.size();
        var3.set(var0, var11);
        var3.add(var12);
        if(var4 != null) {
            var4.add(new HashMap());
        }

        Iterator var13 = var12.iterator();

        while(var13.hasNext()) {
            int var9 = ((Integer)var13.next()).intValue();
            HashSet var10 = (HashSet)var3.get(var9);
            var10.remove(Integer.valueOf(var0));
            var10.add(Integer.valueOf(var14));
            replaceWeight(var9, var0, var14, var4);
        }

        String var15 = rootName((String)var5.get(var0));
        var5.set(var0, nameVertex(var5, var11, var15));
        var5.add(nameVertex(var5, var12, var15));
        if(var6) {
            var2.splitVertex(var0, var14, var11, var12);
        }

        if(var7 != null) {
            var7.add(Integer.valueOf(var14));
        }

        var8.splitVertex(rootName((String)var5.get(var0)), var1);
        return var14;
    }

    private static void replaceWeight(int var0, int var1, int var2, List<HashMap<Integer, Float>> var3) {
        if(var3 != null) {
            float var4;
            if(var0 < var1) {
                var4 = ((Float)((HashMap)var3.get(var0)).get(Integer.valueOf(var1))).floatValue();
            } else {
                var4 = ((Float)((HashMap)var3.get(var1)).get(Integer.valueOf(var0))).floatValue();
            }

            if(var0 < var2) {
                ((HashMap)var3.get(var0)).put(Integer.valueOf(var2), Float.valueOf(var4));
            } else {
                ((HashMap)var3.get(var2)).put(Integer.valueOf(var0), Float.valueOf(var4));
            }
        }

    }

    private static String nameVertex(List<String> var0, HashSet<Integer> var1, String var2) {
        for(Iterator var3 = var1.iterator(); var3.hasNext(); var2 = var2.concat("." + rootName((String)var0.get(((Integer)var3.next()).intValue())))) {
            ;
        }

        return var2;
    }

    public static String rootName(String var0) {
        int var1 = var0.indexOf(".");
        return var1 == -1?var0:var0.substring(0, var1);
    }

    private static void compVertex(Integer var0, Between var1, List<HashSet<Integer>> var2, boolean var3) {
        ArrayList var4 = null;
        float var14 = 1.0F / 0.0;
        float var16 = 0.0F;
        int var17 = 0;
        boolean var18 = false;
        PB var19 = (PB)var1.pbV.get(var0.intValue());
        if(((HashSet)var2.get(var0.intValue())).size() >= 4) {
            int var11 = var19.entryOf.size();
            float[][] var20 = tempMatrixCopy(var19.matrix, var11);
            ArrayList var21 = new ArrayList();
            int var9;
            int var10;
            if(var3 || statsOnly) {
                for(var9 = 1; var9 < var11; ++var9) {
                    for(var10 = 0; var10 < var9; ++var10) {
                        var16 += var20[var9][var10];
                    }
                }
            }

            int var12;
            for(var12 = 0; var12 < var11; ++var12) {
                var21.add(new HashSet());
                ((HashSet)var21.get(var12)).add(Integer.valueOf(var19.vertexOf[var12]));
            }

            for(var12 = 0; var12 < var11 - 2; ++var12) {
                var14 = 1.0F / 0.0;
                var17 = 0;
                int var23 = 0;
                int var7 = 2147483647;
                int var8 = 2147483647;

                for(var9 = 1; var9 < var11; ++var9) {
                    for(var10 = 0; var10 < var9; ++var10) {
                        int var5;
                        int var6;
                        if(var19.vertexOf[var9] < var19.vertexOf[var10]) {
                            var5 = var9;
                            var6 = var10;
                        } else {
                            var5 = var10;
                            var6 = var9;
                        }

                        if(rand != null && (double)var20[var9][var10] >= -1.0E-4D) {
                            if(var20[var9][var10] < var14) {
                                var4 = new ArrayList();
                                var4.add(new Pair(var9, var10));
                            } else if(var20[var9][var10] == var14) {
                                var4.add(new Pair(var9, var10));
                            }
                        }

                        if((double)var20[var9][var10] >= -1.0E-4D && detLess(var20[var9][var10], var14, var19.vertexOf[var5], var7, var19.vertexOf[var6], var8)) {
                            var14 = var20[var9][var10];
                            var17 = var9;
                            var23 = var10;
                            var7 = var19.vertexOf[var5];
                            var8 = var19.vertexOf[var6];
                        }
                    }
                }

                if(rand != null) {
                    Pair var22 = (Pair)var4.get(rand.nextInt(var4.size()));
                    var17 = var22.value1;
                    var23 = var22.value2;
                }

                if(var19.vertexOf[var23] < var19.vertexOf[var17]) {
                    int var25 = var17;
                    var17 = var23;
                    var23 = var25;
                }

                for(var9 = 0; var9 < var11; ++var9) {
                    var20[var17][var9] += var20[var23][var9];
                    var20[var9][var17] += var20[var9][var23];
                    var20[var17][var17] = 0.0F;
                    var20[var23][var9] = -1.0F;
                    var20[var9][var23] = -1.0F;
                }

                ((HashSet)var21.get(var17)).addAll((Collection)var21.get(var23));
                ((HashSet)var21.get(var23)).clear();
            }

            for(var10 = 0; var10 < var11 && (var20[var17][var10] < 0.0F || var10 == var17); ++var10) {
                ;
            }

            Split var13 = new Split((HashSet)var21.get(var17), (HashSet)var21.get(var10));
            float var15 = var20[var17][var10];
            if(var3) {
                if(var16 == 0.0F) {
                    if(var15 != 0.0F) {
                        System.out.println("Div by 0");
                    }
                } else {
                    var15 /= var16;
                }
            }

            var1.putVertex(var0.intValue(), var15, var13, var16);
        } else {
            int var24 = ((HashSet)var2.get(var0.intValue())).size();
            if(var24 >= 4) {
                System.out.println("Not splittable " + var0 + " " + var24);
            }
        }

    }

    private static float[][] tempMatrixCopy(float[][] var0, int var1) {
        if(tempMatrixSize < var1) {
            tempMatrixSize = var1;
            tempMatrix = new float[tempMatrixSize][tempMatrixSize];
        }

        for(int var2 = 0; var2 < var1; ++var2) {
            for(int var3 = 0; var3 < var1; ++var3) {
                tempMatrix[var2][var3] = var0[var2][var3];
            }
        }

        return tempMatrix;
    }

    public static TreeSet<Integer> componentOf(int var0, List<HashSet<Integer>> var1) {
        return regionOf(var0, 2147483647, var1);
    }

    private static TreeSet<Integer> regionOf(int var0, int var1, List<HashSet<Integer>> var2) {
        TreeSet var3 = new TreeSet();
        regionOf(var0, var1, var2, var3);
        return var3;
    }

    private static List<Integer> regionOf(int var0, int var1, List<HashSet<Integer>> var2, TreeSet<Integer> var3) {
        ArrayList var8 = new ArrayList();
        var3.add(Integer.valueOf(var0));
        var8.add(Integer.valueOf(var0));

        for(int var6 = 0; var6 < var1 && !var8.isEmpty(); ++var6) {
            ArrayList var9 = new ArrayList();

            do {
                int var4 = ((Integer)var8.remove(var8.size() - 1)).intValue();
                Iterator var7 = ((HashSet)var2.get(var4)).iterator();

                while(var7.hasNext()) {
                    int var5 = ((Integer)var7.next()).intValue();
                    if(var3.add(Integer.valueOf(var5))) {
                        var9.add(Integer.valueOf(var5));
                    }
                }
            } while(!var8.isEmpty());

            var8 = var9;
        }

        return var8;
    }

    private static boolean smallRegion(List<Integer> var0, TreeSet<Integer> var1, List<HashSet<Integer>> var2) {
        label17:
        while(true) {
            if(!var0.isEmpty()) {
                int var3 = ((Integer)var0.remove(var0.size() - 1)).intValue();
                Iterator var5 = ((HashSet)var2.get(var3)).iterator();

                int var4;
                do {
                    if(!var5.hasNext()) {
                        continue label17;
                    }

                    var4 = ((Integer)var5.next()).intValue();
                } while(var1.contains(Integer.valueOf(var4)));

                return true;
            }

            return false;
        }
    }

    private static void constructShowClusters(String var0, int var1, String var2, int var3, int var4, int var5, int var6, int var7, int var8, String var9, Stats var10, ClusterInfo var11, double var12, boolean var14) throws Exception {
        String[] var16 = null;
        int var20 = 0;
        int var21 = 0;
        ArrayList var22 = null;
        ArrayList var23 = null;
        ArrayList var24 = null;
        ArrayList var25 = new ArrayList();
        int var19;
        if(var11 != null) {
            var24 = var11.edgesI;
            var23 = var11.steps;
            var22 = var11.vertexName;
            var21 = var22.size();
            var20 = var11.graphSize;

            for(var19 = 0; var19 < var20; ++var19) {
                var25.add(rootName((String)var22.get(var19)));
            }
        } else {
            try {
                if((new File(var0)).exists()) {
                    BufferedReader var26 = new BufferedReader(new FileReader(var0));
                    boolean var27 = false;
                    var20 = Integer.parseInt(var26.readLine());

                    for(var19 = 0; var19 < var20; ++var19) {
                        var25.add(var26.readLine());
                    }

                    var24 = new ArrayList();

                    String var15;
                    while((var15 = var26.readLine()) != null && !var15.equals("")) {
                        var16 = var15.split(" ");
                        int var17 = Integer.parseInt(var16[0]);
                        int var18 = Integer.parseInt(var16[1]);
                        var24.add(new Pair(var17, var18));
                    }

                    var23 = new ArrayList();
                    if(var15.equals("")) {
                        while((var15 = var26.readLine()) != null && !var15.equals("")) {
                            if(var15.equals("end")) {
                                var27 = true;
                                break;
                            }

                            var16 = var15.split(" ");
                            var23.add(new Pair(Integer.parseInt(var16[0]), Integer.parseInt(var16[1])));
                        }
                    }

                    if(!var27) {
                        var26.close();
                        System.out.println("Clustering file " + var0 + " incomplete.");
                        System.out.println("Delete and try again.");
                        throw new Exception();
                    }

                    var21 = Integer.parseInt(var26.readLine());
                    var22 = new ArrayList(var21);

                    for(var19 = 0; var19 < var21; ++var19) {
                        var22.add(var26.readLine());
                    }

                    var26.close();
                } else {
                    System.out.println("Clustering file " + var0 + " not found");
                    System.exit(1);
                }
            } catch (Exception var28) {
                System.out.println("Clustering file error: " + var28.toString());
                System.exit(1);
            }
        }

        if(var1 > var21) {
            System.out.println("Number of clusters must not be greater than " + var21);
            System.exit(1);
        }

        showClusters(var23, var24, var25, var1, var2, var3, var4, var5, var6, var7, var8, var20, var21, var22, var9, var10, var12, var14);
    }

    private static void showClusters(List<Pair> var0, List<Pair> var1, List<String> var2, int var3, String var4, int var5, int var6, int var7, int var8, int var9, int var10, int var11, int var12, List<String> var13, String var14, Stats var15, double var16, boolean var18) throws Exception {
        GResult var20 = null;
        double var28 = 0.0D;
        int var30 = 0;
        boolean var36 = false;
        int[] var40 = new int[var12];
        HashMap var41 = new HashMap();
        ArrayList var42 = new ArrayList();
        ArrayList var43 = new ArrayList();
        HashMap var44 = new HashMap();
        HashMap var45 = new HashMap();
        HashMap var48 = null;
        HashMap var49 = null;
        if(var10 > 0 || var16 > 0.0D || var6 > 0) {
            var49 = new HashMap();
        }

        int var54;
        for(var54 = 0; var54 < var1.size(); ++var54) {
            Pair var19 = (Pair)var1.get(var54);
            String var33 = (String)var2.get(var19.value1);
            String var34 = (String)var2.get(var19.value2);
            var42.add(new StrPair(var33, var34));
            if(var10 > 0 || var16 > 0.0D || var6 > 0) {
                if(!var49.containsKey(var33)) {
                    var49.put(var33, new HashSet());
                }

                if(!var49.containsKey(var34)) {
                    var49.put(var34, new HashSet());
                }

                ((HashSet)var49.get(var33)).add(var34);
                ((HashSet)var49.get(var34)).add(var33);
            }
        }

        for(var54 = 0; var54 < var12; ++var54) {
            var43.add(new Tree(var54));
        }

        var54 = var12;
        int var53 = var0.size() - 1;

        Tree var21;
        int var50;
        int var51;
        int var52;
        while(var16 == 0.0D && var54 > var3 || var16 > 0.0D && var28 < var16) {
            if(var53 < 0) {
                if(var3 == 0) {
                    break;
                }

                System.out.println("Number of clusters must not be less than " + var54);
                System.exit(1);
            }

            int var57 = ((Pair)var0.get(var53)).value1;
            int var56 = ((Pair)var0.get(var53)).value2;
            --var53;
            Tree var22 = rootOf((Tree)var43.get(var57));
            Tree var23 = rootOf((Tree)var43.get(var56));
            if(clusterId(var22) != clusterId(var23)) {
                var21 = makeTree(var22, var23);
                var43.set(var57, var21);
                var43.set(var56, var21);
                --var54;
                boolean var58 = var54 <= var6 && var54 % var7 == 0;
                boolean var59 = var54 <= var9 && var54 % var7 == 0;
                boolean var60 = var54 <= var5 && var54 % var7 == 0;
                boolean var61 = var54 <= var8 && var54 % var7 == 0;
                boolean var62 = var54 <= var10 && var54 % var7 == 0;
                if(var58 || var59 || var61 || var60 || var62) {
                    var36 = true;
                    sOutputN(var54 + ":");
                }

                if(var58 || var59 || var61 || var60 || var62 || var16 > 0.0D) {
                    for(var50 = 0; var50 < var12; ++var50) {
                        var51 = clusterId(rootOf((Tree)var43.get(var50)));
                        if(var41.containsKey(Integer.valueOf(var51))) {
                            var52 = ((Integer)var41.get(Integer.valueOf(var51))).intValue();
                        } else {
                            var52 = var41.size();
                            var41.put(Integer.valueOf(var51), Integer.valueOf(var52));
                        }

                        var40[var50] = var52;
                    }

                    var41.clear();
                }

                if(var58 || var59 || var61 || var62 || var16 > 0.0D) {
                    var48 = new HashMap();
                    var30 = makeClusters(var40, var48, var13, var49, var18);
                }

                if(var62 || var16 > 0.0D) {
                    var20 = averageDiameter(var48, var49);
                    var28 = var20.averageDiameter;
                }

                if(var60) {
                    sOutputN(" m=" + String.format("%.3f", new Object[]{Double.valueOf(modularity(var54, var40, var1))}));
                }

                if(var58) {
                    sOutputN(" mo=" + String.format("%.3f", new Object[]{Double.valueOf(ModOverlap.modOverlap(var48, var49, (double)var11, (double)var42.size()))}));
                }

                if(var61) {
                    double var26 = (double)var30 / (double)var11;
                    sOutputN(" ov=" + String.format("%.3f", new Object[]{Double.valueOf(var26)}));
                    var15.overlap = var26;
                }

                if(var59) {
                    sOutputN(" vad=" + String.format("%.3f", new Object[]{Double.valueOf(vad(var48, var42))}));
                }

                if(var62) {
                    sOutputN(" dia=" + String.format("%.3f %.3f %.3f", new Object[]{Double.valueOf(var20.minDiameter), Double.valueOf(var20.averageDiameter), Double.valueOf(var20.maxDiameter)}));
                }

                if(var36) {
                    sOutput("");
                    var36 = false;
                }
            }
        }

        var15.nClusters = var54;
        if(var3 > 0 || var28 > 0.0D) {
            try {
                PrintStream var64 = new PrintStream(new FileOutputStream(var14));
                var50 = 0;
                var53 = 0;
                TreeSet var65 = new TreeSet();
                int var66 = 0;

                Iterator var37;
                HashMap var46;
                for(var52 = 0; var53 < var54; ++var50) {
                    var46 = new HashMap();
                    var21 = rootOf((Tree)var43.get(var50));
                    if(var65.add(Integer.valueOf(var21.label))) {
                        ++var53;
                        HashSet var39 = flattenTree(var21);
                        String var32;
                        if(!var39.isEmpty()) {
                            for(var37 = var39.iterator(); var37.hasNext(); var45.put(var32, Double.valueOf(((Double)var45.get(var32)).doubleValue() + 1.0D))) {
                                var32 = rootName((String)var13.get(((Integer)var37.next()).intValue()));
                                if(!var46.containsKey(var32)) {
                                    var46.put(var32, Double.valueOf(0.0D));
                                }

                                var46.put(var32, Double.valueOf(((Double)var46.get(var32)).doubleValue() + 1.0D));
                                if(!var45.containsKey(var32)) {
                                    var45.put(var32, Double.valueOf(0.0D));
                                }
                            }
                        }

                        var44.put(Integer.valueOf(var52++), var46);
                    }
                }

                ModOverlap.normalize(var44, var45);
                var37 = var44.keySet().iterator();

                while(true) {
                    do {
                        if(!var37.hasNext()) {
                            sOutput("Total size = " + var66);
                            var64.close();
                            return;
                        }

                        var46 = (HashMap)var44.get(var37.next());
                    } while(var4 != null && !var46.containsKey(var4));

                    int var55 = var46.size();
                    var66 += var55;
                    String[] var67 = new String[var55];
                    var46.keySet().toArray(var67);
                    Arrays.sort(var67);
                    sOutput(var55 + "");

                    for(var51 = 0; var51 < var55; ++var51) {
                        var64.print(var67[var51]);
                        if(var18 && ((Double)var46.get(var67[var51])).doubleValue() != 1.0D) {
                            var64.print(":" + String.format("%.3f", new Object[]{var46.get(var67[var51])}));
                        }

                        var64.print(" ");
                    }

                    var64.println();
                }
            } catch (Exception var63) {
                System.out.println("Error: " + var63.toString());
                throw var63;
            }
        }
    }

    private static double vad(HashMap<Integer, HashMap<String, Double>> var0, List<StrPair> var1) {
        double var5 = 0.0D;
        double var7 = 0.0D;
        Iterator var9 = var0.keySet().iterator();

        while(var9.hasNext()) {
            HashMap var4 = (HashMap)var0.get(var9.next());
            var5 += (double)var4.size();

            for(int var2 = 0; var2 < var1.size(); ++var2) {
                StrPair var3 = (StrPair)var1.get(var2);
                if(var4.containsKey(var3.value1) && var4.containsKey(var3.value2)) {
                    ++var7;
                }
            }
        }

        return (var7 + var7) / var5;
    }

    private static int makeClusters(int[] var0, HashMap<Integer, HashMap<String, Double>> var1, List<String> var2, HashMap<String, HashSet<String>> var3, boolean var4) {
        int var7 = 0;
        HashMap var10 = new HashMap();

        for(int var5 = 0; var5 < var0.length; ++var5) {
            int var6 = var0[var5];
            String var8 = rootName((String)var2.get(var5));
            if(!var1.containsKey(Integer.valueOf(var6))) {
                var1.put(Integer.valueOf(var6), new HashMap());
            }

            HashMap var9 = (HashMap)var1.get(Integer.valueOf(var6));
            if(!var9.containsKey(var8)) {
                var9.put(var8, Double.valueOf(1.0D));
                incWeight(var10, var8);
                ++var7;
            } else if(var4) {
                var9.put(var8, Double.valueOf(((Double)var9.get(var8)).doubleValue() + 1.0D));
                incWeight(var10, var8);
            }
        }

        ModOverlap.normalize(var1, var10);
        return var7;
    }

    private static void incWeight(HashMap<String, Double> var0, String var1) {
        if(!var0.containsKey(var1)) {
            var0.put(var1, Double.valueOf(0.0D));
        }

        var0.put(var1, Double.valueOf(((Double)var0.get(var1)).doubleValue() + 1.0D));
    }

    private static GResult averageDiameter(HashMap<Integer, HashMap<String, Double>> var0, HashMap<String, HashSet<String>> var1) {
        int var4 = 2147483647;
        int var5 = -2147483648;
        int var6 = 0;
        double var7 = 0.0D;
        Iterator var10 = var0.keySet().iterator();

        while(var10.hasNext()) {
            ++var6;
            HashMap var9 = (HashMap)var0.get(var10.next());
            int var3 = diameter(var9, var1);
            var7 += (double)var3;
            if(var3 < var4) {
                var4 = var3;
            }

            if(var3 > var5) {
                var5 = var3;
            }
        }

        return new GResult((double)var4, var7 / (double)var6, (double)var5);
    }

    private static int diameter(HashMap<String, Double> var0, HashMap<String, HashSet<String>> var1) {
        int var4 = 0;
        Iterator var5 = var0.keySet().iterator();

        while(var5.hasNext()) {
            String var2 = (String)var5.next();
            int var3 = maxDist(var2, var0, var1);
            if(var3 > var4) {
                var4 = var3;
            }
        }

        return var4;
    }

    private static int maxDist(String var0, HashMap<String, Double> var1, HashMap<String, HashSet<String>> var2) {
        HashSet var5 = new HashSet();
        ArrayList var7 = new ArrayList();
        var5.add(var0);
        var7.add(var0);
        int var9 = 1;

        int var10;
        for(var10 = 0; !var7.isEmpty(); ++var9) {
            ArrayList var8 = new ArrayList();

            do {
                String var3 = (String)var7.remove(var7.size() - 1);
                Iterator var6 = ((HashSet)var2.get(var3)).iterator();

                while(var6.hasNext()) {
                    String var4 = (String)var6.next();
                    if(var5.add(var4)) {
                        var8.add(var4);
                        if(var1.containsKey(var4)) {
                            var10 = var9;
                        }
                    }
                }
            } while(!var7.isEmpty());

            var7 = var8;
        }

        return var10;
    }

    private static double modularity(int var0, int[] var1, List<Pair> var2) {
        int var10 = 0;
        int[][] var12 = new int[var0][var0];

        int var3;
        int var4;
        for(var3 = 0; var3 < var0; ++var3) {
            for(var4 = 0; var4 < var0; ++var4) {
                var12[var3][var4] = 0;
            }
        }

        for(Iterator var13 = var2.iterator(); var13.hasNext(); ++var10) {
            Pair var11 = (Pair)var13.next();
            int var5 = var1[var11.value1];
            int var6 = var1[var11.value2];
            ++var12[var5][var6];
            ++var12[var6][var5];
        }

        double var14 = (double)(var10 + var10);
        double var16 = 0.0D;

        for(var3 = 0; var3 < var0; ++var3) {
            int var7 = 0;

            for(var4 = 0; var4 < var0; ++var4) {
                var7 += var12[var3][var4];
            }

            double var8 = (double)var7;
            var16 += ((double)var12[var3][var3] - var8 * var8 / var14) / var14;
        }

        return var16;
    }

    private static Tree rootOf(Tree var0) {
        while(var0.parent != null) {
            var0 = var0.parent;
        }

        return var0;
    }

    private static Tree makeTree(Tree var0, Tree var1) {
        int var2 = Math.min(var0.label, var1.label);
        Tree var3 = new Tree(var2);
        var3.left = var0;
        var3.right = var1;
        var0.parent = var3;
        var1.parent = var3;
        return var3;
    }

    private static int clusterId(Tree var0) {
        return var0.label;
    }

    private static void dispTree(Tree var0) {
        System.out.print("(" + var0.label);
        if(var0.left != null) {
            System.out.print(" ");
            dispTree(var0.left);
        }

        if(var0.right != null) {
            System.out.print(" ");
            dispTree(var0.right);
        }

        System.out.print(")");
    }

    private static HashSet<Integer> flattenTree(Tree var0) {
        HashSet var1 = new HashSet();
        flattenTree(var0, var1);
        return var1;
    }

    private static void flattenTree(Tree var0, HashSet<Integer> var1) {
        if(var0.left == null) {
            var1.add(Integer.valueOf(var0.label));
        } else {
            flattenTree(var0.left, var1);
            flattenTree(var0.right, var1);
        }

    }

    public static void printDegree(TreeSet<Integer> var0, List<HashSet<Integer>> var1, List<String> var2) {
        Iterator var4 = var0.iterator();

        while(var4.hasNext()) {
            int var3 = ((Integer)var4.next()).intValue();
            System.out.println((String)var2.get(var3) + "\t" + ((HashSet)var1.get(var3)).size());
        }

    }

    private static void printUsageAndExit() {
        System.err.println("Usage: java conga.CONGA <file> [-e] [-g f] [-n nC] [-s] [-cd t] [-f v] [-r]");
        System.err.println("                         [-mem] [-m c] [-mo c] [-vad c] [-ov c]");
        System.err.println("                         [-dia c] [-h h] [-GN] [-peacock s] [-w eW]");
        System.err.println("Options:");
        System.err.println("  -e   Network file format is list of edges. Default: native format.");
        System.err.println("  -g   Remove vertices named in filter file f.");
        System.err.println("  -n   Find clustering containing nC clusters. Default: 0.");
        System.err.println("  -s   Silent operation: don\'t display steps in algorithm.");
        System.err.println("  -cd  Find solution with mean cluster diameter t. Default: 0.");
        System.err.println("  -f   Find only vertex v\'s cluster. Default: find all clusters.");
        System.err.println("  -r   Recompute clusters even if clustering file exists.");
        System.err.println("  -mem Recompute clusters; don\'t create/use clustering file.");
        System.err.println("  -m   Show (Newman\'s) modularity of solutions with up to c clusters.");
        System.err.println("  -mo  Show (Nicosia\'s) modularity of solutions with up to c clusters.");
        System.err.println("  -vad Show vertex average degree of solutions with up to c clusters.");
        System.err.println("  -ov  Show overlap of solutions with up to c clusters.");
        System.err.println("  -dia Show cluster diameters of solutions with up to c clusters.");
        System.err.println("  -h   Use region with horizon h. Default: unlimited.");
        System.err.println("  -GN  Algorithm is Girvan & Newman. Default: conga.CONGA.");
        System.err.println("  -peacock  Peacock mode: split until max edge betweenness <= s. Default: off.");
        System.err.println("  -w   Include edge weights in computations. Default: unweighted.");
        System.exit(1);
    }

    private static void cSplit(int var0, int var1, PrintStream var2, ClusterInfo var3) {
        if(var3 != null) {
            var3.steps.add(new Pair(var0, var1));
        } else {
            var2.println(var0 + " " + var1);
        }

        ++nSplit;
    }

    private static void cEdge(int var0, int var1, PrintStream var2, ClusterInfo var3) {
        if(var3 != null) {
            var3.edgesI.add(new Pair(var0, var1));
        } else {
            var2.println(var0 + " " + var1);
        }

    }

    private static void dOutput(String var0) {
        if(debug) {
            dStream.println(var0);
        }

    }

    private static void sOutputN(String var0) {
        if(screen) {
            System.out.print(var0);
        }

    }

    private static void sOutput(String var0) {
        if(screen) {
            System.out.println(var0);
        }

    }
}
