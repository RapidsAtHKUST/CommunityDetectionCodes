package clique_modularity.util.benchmark;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.util.Pair;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.InputStream;
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
import java.util.Random;
import java.util.TreeMap;
import java.util.TreeSet;

public class GG {
    static String dir = "tests/";
    static String groupsPrefix = "groups-";
    static String edgesPrefix = "edges-";
    static boolean csv = false;

    public GG() {
    }

    public static void main(String[] var0) {
        int var2 = 0;
        int var3 = 0;
        int var4 = 0;
        double var5 = 1.0D;
        double var7 = 1.0D;
        double var9 = 1.0D;
        double var11 = 0.0D;
        double var13 = 0.0D;
        double var15 = 0.0D;
        double var17 = 0.0D;
        int var19 = 0;
        int var20 = 0;
        double var21 = 0.0D;
        double var23 = 0.0D;
        boolean var25 = true;
        boolean var26 = false;
        boolean var27 = false;
        double var28 = -1.0D;
        boolean var30 = false;
        boolean var31 = false;
        int var32 = 0;
        int var33 = 0;
        boolean var34 = false;
        int var35 = 0;
        int var36 = 0;
        double var37 = 0.0D;
        double var39 = 0.0D;
        double var41 = 0.0D;
        boolean var43 = false;
        boolean var44 = false;
        int var45 = Array.getLength(var0);
        int var47 = 0;

        while(var47 < var45) {
            String var46 = var0[var47++];
            if(var46.equals("-n")) {
                var2 = Integer.parseInt(var0[var47++]);
                System.out.println("Number of individuals: " + var2);
            } else if(var46.equals("-n2")) {
                var3 = Integer.parseInt(var0[var47++]);
                System.out.println("Number of individuals2: " + var3);
            } else if(var46.equals("-g")) {
                var4 = Integer.parseInt(var0[var47++]);
                System.out.println("Number of groups: " + var4);
            } else if(var46.equals("-r")) {
                var5 = Double.parseDouble(var0[var47++]);
                System.out.println("Overlap: " + var5);
            } else if(var46.equals("-r2")) {
                var7 = Double.parseDouble(var0[var47++]);
                System.out.println("Overlap2: " + var7);
            } else if(var46.equals("-pi")) {
                var9 = Double.parseDouble(var0[var47++]);
                System.out.println("Probability of intragroup edges: " + var9);
            } else if(var46.equals("-po")) {
                var11 = Double.parseDouble(var0[var47++]);
                System.out.println("Probability of intergroup edges: " + var11);
            } else if(var46.equals("-zo")) {
                var13 = Double.parseDouble(var0[var47++]);
                System.out.println("Number of intergroup edges/vertex: " + var13);
            } else if(var46.equals("-z")) {
                var17 = Double.parseDouble(var0[var47++]);
                System.out.println("Number of edges/vertex: " + var17);
            } else if(var46.equals("-u")) {
                var19 = Integer.parseInt(var0[var47++]);
                System.out.println("Number not in any group: " + var19);
            } else if(var46.equals("-m0")) {
                var20 = Integer.parseInt(var0[var47++]);
                System.out.println("m0: " + var20);
            } else if(var46.equals("-zm")) {
                var21 = Double.parseDouble(var0[var47++]);
                System.out.println("zm: " + var21);
            } else if(var46.equals("-zn")) {
                var23 = Double.parseDouble(var0[var47++]);
                System.out.println("zn: " + var23);
            } else if(var46.equals("-flat")) {
                var25 = true;
                System.out.println("Flat");
            } else if(var46.equals("-pref")) {
                var26 = true;
                System.out.println("Preferential attachment");
            } else if(var46.equals("-dia")) {
                var27 = true;
                System.out.println("Computing community diameter");
            } else if(var46.equals("-npref")) {
                var28 = Double.parseDouble(var0[var47++]);
                System.out.println("New preference: " + var28);
            } else if(var46.equals("-conn")) {
                var32 = Integer.parseInt(var0[var47++]);
                System.out.println("Tolerance (disconnected): " + var32);
            } else if(var46.equals("-conn2")) {
                var33 = Integer.parseInt(var0[var47++]);
                System.out.println("Tolerance (disconnected) 2: " + var33);
            } else if(var46.equals("-old")) {
                var30 = true;
                System.out.println("Old makeGroups");
            } else if(var46.equals("-stats")) {
                var31 = true;
                System.out.println("Showing statistics");
            } else if(var46.equals("-benchmark")) {
                var34 = true;
                var2 = Integer.parseInt(var0[var47++]);
                var35 = Integer.parseInt(var0[var47++]);
                var36 = Integer.parseInt(var0[var47++]);
                var37 = Double.parseDouble(var0[var47++]);
                var39 = Double.parseDouble(var0[var47++]);
                var41 = Double.parseDouble(var0[var47++]);
            } else {
                printUsageAndExit();
            }
        }

        if(!var34 && (var2 < var4 || var4 < 1 || var5 < 1.0D || var7 < 1.0D)) {
            printUsageAndExit();
        }

        double var48 = (double)var2 * Math.pow(1.0D - var5 / (double)var4, var5);
        double var50 = (double)var2 - var48 - 1.0D;
        if(var17 > 0.0D || var13 > 0.0D) {
            var15 = var17 - var13;
            var9 = var15 / var50;
            var11 = var13 / var48;
            var23 = var15 / 2.0D;
            var21 = var13 / 2.0D + var23;
        }

        if(!var34 && (var26 && ((double)var20 <= var15 || Math.floor(var21) < Math.ceil(var23) || var20 < 1 || var23 < 1.0D || var21 < 1.0D) || !var26 && (var9 < 0.0D || var9 > 1.0D || var11 < 0.0D || var11 > 1.0D))) {
            printUsageAndExit();
        }

        Random var52 = new Random();
        String var53;
        if(var34) {
            var53 = "temp.txt";
        } else {
            var53 = randName(var2, var4, var5, var9, var11, var52);
        }

        String var54 = dir + groupsPrefix + var53;
        String var55 = "";
        String var56 = dir + var53;
        String var57 = "";
        String var58 = "";
        String var59 = "";
        String var60 = "";
        String var61 = "";
        if(var3 > 0) {
            var54 = dir + groupsPrefix + "1-" + var53;
            var55 = dir + groupsPrefix + "2-" + var53;
            var57 = dir + "inv-" + var53;
            var58 = dir + "1-" + var53;
            var59 = dir + "2-" + var53;
            var60 = dir + "1W-" + var53;
            var61 = dir + "2W-" + var53;
        }

        if(var34) {
            var4 = gen_benchmark(var2, var35, var36, var37, var39, var41, 0, 0, var56, var54);
            System.out.println("Number of groups = " + var4);
        } else {
            GGResult var62 = generate(var2, var3, var4, var5, var7, var9, var11, var19, var54, var55, var56, var57, var58, var59, var60, var61, var25, var26, var32, var33, var20, var21, var23, var27, var28, var30, var31);
            System.out.println("Min group size = " + var62.minGroup);
            System.out.println("Community diameter = " + var62.minDiameter + " " + var62.averageDiameter + " " + var62.maxDiameter);
        }

    }

    public static int gen_benchmark(int var0, int var1, int var2, double var3, double var5, double var7, int var9, int var10, String var11, String var12) {
        String var13 = "parameters.dat";
        String var14 = "network.dat";
        String var15 = "community.dat";

        try {
            PrintStream var16 = new PrintStream(new FileOutputStream(var13));
            var16.println(var0);
            var16.println(var1);
            var16.println(var2);
            var16.println(var3);
            var16.println(var5);
            var16.println(var7);
            if(var9 > 0) {
                var16.println(var9);
                var16.println(var10);
            }

            var16.close();
            run_command("benchmark", true);
            convert_bm_network(var14, var11);
            return convert_bm_community(var15, var12);
        } catch (Exception var18) {
            System.out.println("Are you sure benchmark code is in current directory?");
            System.out.println("Error " + var18.toString());
            var18.printStackTrace();
            System.exit(1);
            return 0;
        }
    }

    private static long run_command(String var0, boolean var1) {
        long var2 = (new Date()).getTime();

        try {
            Process var4 = Runtime.getRuntime().exec(var0);
            InputStream var5 = var4.getInputStream();

            int var6;
            while((var6 = var5.read()) != -1) {
                if(var1) {
                    System.out.write(var6);
                }
            }

            var5.close();
            var4.waitFor();
        } catch (Exception var7) {
            System.out.println("Command error: " + var7.toString());
            System.exit(1);
        }

        return (new Date()).getTime() - var2;
    }

    public static void convert_bm_network(String var0, String var1) {
        try {
            BufferedReader var6 = new BufferedReader(new FileReader(var0));
            PrintStream var7 = new PrintStream(new FileOutputStream(var1));

            String var5;
            while((var5 = var6.readLine()) != null) {
                String[] var4 = var5.split("\t");
                int var2 = Integer.parseInt(var4[0]);
                int var3 = Integer.parseInt(var4[1]);
                var7.println(var2 - 1 + "\t" + (var3 - 1));
            }

            var6.close();
            var7.close();
        } catch (Exception var8) {
            System.out.println("Error " + var8.toString());
            var8.printStackTrace();
            System.exit(1);
        }

    }

    public static int convert_bm_community(String var0, String var1) {
        int var4 = 0;
        ArrayList var8 = new ArrayList();

        try {
            BufferedReader var9 = new BufferedReader(new FileReader(var0));

            int var3;
            String var6;
            while((var6 = var9.readLine()) != null) {
                String[] var5 = var6.split("\t");
                int var2 = Integer.parseInt(var5[0]) - 1;
                var3 = Integer.parseInt(var5[1]);

                while(var8.size() <= var3) {
                    var8.add(new TreeSet());
                }

                ((TreeSet)var8.get(var3)).add(Integer.valueOf(var2));
            }

            var9.close();
            PrintStream var10 = new PrintStream(new FileOutputStream(var1));

            for(var3 = 1; var3 < var8.size(); ++var3) {
                if(((TreeSet)var8.get(var3)).size() > 0) {
                    Iterator var7 = ((TreeSet)var8.get(var3)).iterator();

                    while(var7.hasNext()) {
                        var10.print(var7.next() + " ");
                    }

                    var10.println();
                    ++var4;
                }
            }

            var10.close();
        } catch (Exception var11) {
            System.out.println("Error " + var11.toString());
            var11.printStackTrace();
            System.exit(1);
        }

        return var4;
    }

    public static GGResult generate(int var0, int var1, int var2, double var3, double var5, double var7, double var9, int var11, String var12, String var13, String var14, String var15, String var16, String var17, String var18, String var19, boolean var20, boolean var21, int var22, int var23, int var24, double var25, double var27, boolean var29, double var30, boolean var32, boolean var33) {
        GGResult var34;
        do {
            var34 = gen(var0, var1, var2, var3, var5, var7, var9, var11, var12, var13, var14, var15, var16, var17, var18, var19, var20, var21, var22, var23, var24, var25, var27, var29, var30, var32, var33);
        } while(var34.minGroup < 1);

        return var34;
    }

    private static GGResult gen(int var0, int var1, int var2, double var3, double var5, double var7, double var9, int var11, String var12, String var13, String var14, String var15, String var16, String var17, String var18, String var19, boolean var20, boolean var21, int var22, int var23, int var24, double var25, double var27, boolean var29, double var30, boolean var32, boolean var33) {
        GGResult var34 = new GGResult(0, 0.0D, 0.0D, 0.0D);
        Random var35 = new Random();
        ArrayList var38 = null;
        long var45 = (new Date()).getTime();

        ArrayList var37;
        do {
            var37 = makeGroups(var0, var2, var3, var11, var35, var20);
        } while(var37 == null);

        saveGroups(var37, var2 + var11, var12);
        if(var1 > 0) {
            do {
                var38 = makeGroups(var1, var2, var5, var11, var35, var20);
            } while(var38 == null);

            saveGroups(var38, var2, var13);
        }

        HashSet var36;
        ArrayList var39;
        if(var1 > 0) {
            var39 = makeEdgesBi(var37, var38, var0, var1, var3, var5, var7, var9, var35, true, var30, var32);
            ArrayList var40 = invert(var39, var1);
            ArrayList var41 = project(var39, var40);
            ArrayList var42 = project(var40, var39);
            ArrayList var43 = weightedProject(var39, var40);
            ArrayList var44 = weightedProject(var40, var39);
            saveGraph(var39, var0, -1, (HashSet)null, var14, true);
            saveGraph(var40, var1, -1, (HashSet)null, var15, true);
            if(var33) {
                showStatsBi("Bipartite", var39, var37, var38, var0, var1, var3, var5, var7, var9, true);
                showStatsBi("Inverted", var40, var38, var37, var1, var0, var5, var3, var7, var9, true);
                showStatsBi("Mode 1", var41, var37, var37, var0, var0, var3, var3, 0.0D, 0.0D, false);
                showStatsBi("Mode 2", var42, var38, var38, var1, var1, var3, var3, 0.0D, 0.0D, false);
            }

            var36 = componentOf(0, var41);
            var34 = checkSaveGraph(var41, var0, var22, var36, var16);
            if(var34.minGroup < 0) {
                return var34;
            }

            saveWGraph(var43, var0, var22, var36, var18);
            var36 = componentOf(0, var42);
            var34 = checkSaveGraph(var42, var1, var23, var36, var17);
            if(var34.minGroup < 0) {
                return var34;
            }

            saveWGraph(var44, var1, var23, var36, var19);
            var34.nV = nVertices(var41);
            var34.nV2 = nVertices(var42);
        } else if(var21) {
            do {
                var39 = makeEdgesPA(var37, var2, var0, var24, var25, var27, var35);
            } while(componentOf(0, var39).size() != var0);
        } else {
            var39 = makeEdges(var37, var0, var3, var7, var9, var35, var30, var32);
            if(var33) {
                showStatsBi("Unipartite", var39, var37, var37, var0, var0, var3, var3, 0.0D, 0.0D, false);
            }

            var36 = componentOf(0, var39);
            var34 = checkSaveGraph(var39, var0, var22, var36, var14);
            if(var34.minGroup < 0) {
                return var34;
            }

            if(var29) {
                var34 = averageDiameter(var37, var39);
            }

            if(csv) {
                String var48 = var14.substring(0, var14.lastIndexOf(".")) + ".csv";
                saveGraphCSV(var39, var0, var22, componentOf(0, var39), var48);
            }
        }

        var34.minGroup = minSize(var37, var2);
        return var34;
    }

    private static void pruneGraph(ArrayList<HashSet<Integer>> var0, HashSet<Integer> var1, HashSet<Integer> var2) {
        for(int var3 = 0; var3 < var0.size(); ++var3) {
            if(var1.contains(Integer.valueOf(var3))) {
                Iterator var4 = ((HashSet)var0.get(var3)).iterator();

                while(var4.hasNext()) {
                    if(!var2.contains(var4.next())) {
                        var4.remove();
                    }
                }
            } else {
                var0.set(var3, new HashSet());
            }
        }

    }

    private static void showStatsBi(String var0, ArrayList<HashSet<Integer>> var1, ArrayList<TreeSet<Integer>> var2, ArrayList<TreeSet<Integer>> var3, int var4, int var5, double var6, double var8, double var10, double var12, boolean var14) {
        int var18 = 0;
        int var19 = 0;
        int var20 = 0;
        int var21 = 0;
        double var22 = 0.0D;
        double var24 = 0.0D;
        int var26 = var2.size();

        int var15;
        for(var15 = 0; var15 < var1.size(); ++var15) {
            HashSet var27 = (HashSet)var1.get(var15);
            var19 += var27.size();
            Iterator var28 = var27.iterator();
            if(var27.size() >= 1) {
                ++var18;
            }

            while(var28.hasNext()) {
                if(sameGroup(var15, ((Integer)var28.next()).intValue(), var2, var3)) {
                    ++var20;
                } else {
                    ++var21;
                }
            }
        }

        for(var15 = 0; var15 < var26; ++var15) {
            int var16;
            int var17;
            if(var14) {
                var16 = ((TreeSet)var2.get(var15)).size() * ((TreeSet)var3.get(var15)).size();
                var17 = ((TreeSet)var2.get(var15)).size() * (var5 - ((TreeSet)var3.get(var15)).size());
            } else {
                var16 = ((TreeSet)var2.get(var15)).size() * (((TreeSet)var2.get(var15)).size() - 1);
                var17 = ((TreeSet)var2.get(var15)).size() * (var4 - ((TreeSet)var2.get(var15)).size());
            }

            var22 += (double)var16;
            var24 += (double)var17;
        }

        if(!var14) {
            var20 /= 2;
            var21 /= 2;
            var22 /= 2.0D;
            var24 /= 2.0D;
        }

        System.out.println(var0);
        System.out.println("Size:   " + var18);
        System.out.println("Degree: " + (double)var19 / (double)var18);
        if(var6 == 1.0D && var8 == 1.0D) {
            System.out.println("Measured pin=" + String.format("%.3f", new Object[]{Double.valueOf((double)var20 / var22)}) + " pout=" + String.format("%.3f", new Object[]{Double.valueOf((double)var21 / var24)}));
            if(var14) {
                double var29 = (double)(var5 / var26);
                double var31 = 1.0D - Math.pow(1.0D - var10 * var10, var29) * Math.pow(1.0D - var12 * var12, (double)var5 - var29);
                double var33 = 1.0D - Math.pow(1.0D - var10 * var12, var29 + var29) * Math.pow(1.0D - var12 * var12, (double)var5 - var29 - var29);
                System.out.println("Predicted for mode 1: pin=" + String.format("%.3f", new Object[]{Double.valueOf(var31)}) + " pout=" + String.format("%.3f", new Object[]{Double.valueOf(var33)}));
                double var35 = (double)(var4 / var26);
                double var37 = 1.0D - Math.pow(1.0D - var10 * var10, var35) * Math.pow(1.0D - var12 * var12, (double)var4 - var35);
                double var39 = 1.0D - Math.pow(1.0D - var10 * var12, var35 + var35) * Math.pow(1.0D - var12 * var12, (double)var4 - var35 - var35);
                System.out.println("Predicted for mode 2: pin=" + String.format("%.3f", new Object[]{Double.valueOf(var37)}) + " pout=" + String.format("%.3f", new Object[]{Double.valueOf(var39)}));
            }
        }

        System.out.println();
    }

    private static int nVertices(ArrayList<HashSet<Integer>> var0) {
        int var2 = 0;

        for(int var1 = 0; var1 < var0.size(); ++var1) {
            if(((HashSet)var0.get(var1)).size() >= 1) {
                ++var2;
            }
        }

        return var2;
    }

    private static boolean sameGroup(int var0, int var1, ArrayList<TreeSet<Integer>> var2, ArrayList<TreeSet<Integer>> var3) {
        for(int var4 = 0; var4 < var2.size(); ++var4) {
            if(((TreeSet)var2.get(var4)).contains(Integer.valueOf(var0)) && ((TreeSet)var3.get(var4)).contains(Integer.valueOf(var1))) {
                return true;
            }
        }

        return false;
    }

    private static GGResult checkSaveGraph(ArrayList<HashSet<Integer>> var0, int var1, int var2, HashSet<Integer> var3, String var4) {
        int var5 = var3.size();
        if(var2 >= 0 && var5 < var1 - var2) {
            System.out.println("component size = " + var5);
            var5 = -var5;
        }

        saveGraph(var0, var1, var2, var3, var4, false);
        return new GGResult(var5, 0.0D, 0.0D, 0.0D);
    }

    public static ArrayList<HashSet<Integer>> invert(ArrayList<HashSet<Integer>> var0, int var1) {
        ArrayList var4 = new ArrayList();

        int var2;
        for(var2 = 0; var2 < var1; ++var2) {
            var4.add(new HashSet());
        }

        for(var2 = 0; var2 < var0.size(); ++var2) {
            Iterator var3 = ((HashSet)var0.get(var2)).iterator();

            while(var3.hasNext()) {
                ((HashSet)var4.get(((Integer)var3.next()).intValue())).add(Integer.valueOf(var2));
            }
        }

        return var4;
    }

    public static ArrayList<HashSet<Integer>> project(ArrayList<HashSet<Integer>> var0, ArrayList<HashSet<Integer>> var1) {
        ArrayList var2 = new ArrayList();

        for(int var4 = 0; var4 < var0.size(); ++var4) {
            HashSet var3 = new HashSet();
            Iterator var5 = ((HashSet)var0.get(var4)).iterator();

            while(var5.hasNext()) {
                var3.addAll((Collection)var1.get(((Integer)var5.next()).intValue()));
            }

            var3.remove(Integer.valueOf(var4));
            var2.add(var3);
        }

        return var2;
    }

    public static ArrayList<HashMap<Integer, Integer>> weightedProject(ArrayList<HashSet<Integer>> var0, ArrayList<HashSet<Integer>> var1) {
        ArrayList var2 = new ArrayList();

        for(int var4 = 0; var4 < var0.size(); ++var4) {
            HashMap var3 = new HashMap();
            Iterator var6 = ((HashSet)var0.get(var4)).iterator();

            while(var6.hasNext()) {
                Iterator var7 = ((HashSet)var1.get(((Integer)var6.next()).intValue())).iterator();

                while(var7.hasNext()) {
                    int var5 = ((Integer)var7.next()).intValue();
                    if(var4 < var5) {
                        if(var3.containsKey(Integer.valueOf(var5))) {
                            var3.put(Integer.valueOf(var5), Integer.valueOf(((Integer)var3.get(Integer.valueOf(var5))).intValue() + 1));
                        } else {
                            var3.put(Integer.valueOf(var5), Integer.valueOf(1));
                        }
                    }
                }
            }

            var2.add(var3);
        }

        return var2;
    }

    private static GGResult averageDiameter(ArrayList<TreeSet<Integer>> var0, ArrayList<HashSet<Integer>> var1) {
        int var4 = 2147483647;
        int var5 = -2147483648;
        double var6 = 0.0D;

        for(int var2 = 0; var2 < var0.size(); ++var2) {
            TreeSet var8 = (TreeSet)var0.get(var2);
            int var3 = diameter(var8, var1);
            var6 += (double)var3;
            if(var3 < var4) {
                var4 = var3;
            }

            if(var3 > var5) {
                var5 = var3;
            }
        }

        return new GGResult(0, (double)var4, var6 / (double)var0.size(), (double)var5);
    }

    private static int diameter(TreeSet<Integer> var0, ArrayList<HashSet<Integer>> var1) {
        int var4 = 0;
        Iterator var5 = var0.iterator();

        while(var5.hasNext()) {
            int var2 = ((Integer)var5.next()).intValue();
            int var3 = maxDist(var2, var0, var1);
            if(var3 > var4) {
                var4 = var3;
            }
        }

        return var4;
    }

    private static int maxDist(int var0, TreeSet<Integer> var1, ArrayList<HashSet<Integer>> var2) {
        HashSet var5 = new HashSet();
        ArrayList var7 = new ArrayList();
        var5.add(Integer.valueOf(var0));
        var7.add(Integer.valueOf(var0));
        int var9 = 1;

        int var10;
        for(var10 = 0; !var7.isEmpty(); ++var9) {
            ArrayList var8 = new ArrayList();

            do {
                int var3 = ((Integer)var7.remove(var7.size() - 1)).intValue();
                Iterator var6 = ((HashSet)var2.get(var3)).iterator();

                while(var6.hasNext()) {
                    int var4 = ((Integer)var6.next()).intValue();
                    if(var5.add(Integer.valueOf(var4))) {
                        var8.add(Integer.valueOf(var4));
                        if(var1.contains(Integer.valueOf(var4))) {
                            var10 = var9;
                        }
                    }
                }
            } while(!var7.isEmpty());

            var7 = var8;
        }

        return var10;
    }

    private static void saveGroups(ArrayList<TreeSet<Integer>> var0, int var1, String var2) {
        try {
            PrintStream var6 = new PrintStream(new FileOutputStream(var2));

            for(int var3 = 0; var3 < var1; ++var3) {
                int var5 = ((TreeSet)var0.get(var3)).size();
                Integer[] var7 = new Integer[var5];
                ((TreeSet)var0.get(var3)).toArray(var7);
                String[] var8 = new String[var5];

                int var4;
                for(var4 = 0; var4 < var5; ++var4) {
                    var8[var4] = var7[var4] + " ";
                }

                Arrays.sort(var8);

                for(var4 = 0; var4 < var5; ++var4) {
                    var6.print(var8[var4]);
                }

                var6.println();
            }

            var6.close();
        } catch (Exception var9) {
            System.out.println("Error " + var9.toString());
            var9.printStackTrace();
            System.exit(1);
        }

    }

    private static void saveGraph(ArrayList<HashSet<Integer>> var0, int var1, int var2, HashSet<Integer> var3, String var4, boolean var5) {
        int var8 = 0;

        try {
            PrintStream var10 = new PrintStream(new FileOutputStream(var4));

            label42:
            for(int var6 = 0; var6 < var1; ++var6) {
                Iterator var9 = ((HashSet)var0.get(var6)).iterator();

                while(true) {
                    int var7;
                    do {
                        do {
                            if(!var9.hasNext()) {
                                continue label42;
                            }

                            var7 = ((Integer)var9.next()).intValue();
                        } while(!var5 && var6 >= var7);
                    } while(var2 >= 0 && !var3.contains(Integer.valueOf(var6)));

                    var10.println(var6 + "\t" + var7);
                    ++var8;
                }
            }

            var10.close();
        } catch (Exception var11) {
            System.out.println("Error " + var11.toString());
            var11.printStackTrace();
            System.exit(1);
        }

    }

    private static void saveWGraph(ArrayList<HashMap<Integer, Integer>> var0, int var1, int var2, HashSet<Integer> var3, String var4) {
        int var7 = 0;

        try {
            PrintStream var9 = new PrintStream(new FileOutputStream(var4));

            label39:
            for(int var5 = 0; var5 < var1; ++var5) {
                Iterator var8 = ((HashMap)var0.get(var5)).keySet().iterator();

                while(true) {
                    int var6;
                    do {
                        do {
                            if(!var8.hasNext()) {
                                continue label39;
                            }

                            var6 = ((Integer)var8.next()).intValue();
                        } while(var5 >= var6);
                    } while(var2 >= 0 && !var3.contains(Integer.valueOf(var5)));

                    var9.println(var5 + "\t" + var6 + "\t" + ((HashMap)var0.get(var5)).get(Integer.valueOf(var6)));
                    ++var7;
                }
            }

            var9.close();
        } catch (Exception var10) {
            System.out.println("Error " + var10.toString());
            var10.printStackTrace();
            System.exit(1);
        }

    }

    private static void saveGraphCSV(ArrayList<HashSet<Integer>> var0, int var1, int var2, HashSet<Integer> var3, String var4) {
        int var8 = 0;

        try {
            PrintStream var10 = new PrintStream(new FileOutputStream(var4));

            for(int var6 = 0; var6 < var1; ++var6) {
                var10.print(var6 + 1);
                TreeSet var5 = new TreeSet((Collection)var0.get(var6));

                for(Iterator var9 = var5.iterator(); var9.hasNext(); ++var8) {
                    int var7 = ((Integer)var9.next()).intValue();
                    var10.print("," + (var7 + 1));
                }

                var10.println();
            }

            var10.close();
        } catch (Exception var11) {
            System.out.println("Error " + var11.toString());
            var11.printStackTrace();
            System.exit(1);
        }

    }

    private static String randName(int var0, int var1, double var2, double var4, double var6, Random var8) {
        int var9 = (int)(100.0D * var2);
        int var10 = (int)(100.0D * var4);
        int var11 = (int)(100.0D * var6);
        char var12 = (char)(97 + var8.nextInt(26));
        char var13 = (char)(97 + var8.nextInt(26));
        char var14 = (char)(97 + var8.nextInt(26));
        return "temp.txt";
    }

    private static int minSize(ArrayList<TreeSet<Integer>> var0, int var1) {
        int var4 = 2147483647;

        for(int var2 = 0; var2 < var1; ++var2) {
            int var3 = ((TreeSet)var0.get(var2)).size();
            if(var3 < var4) {
                var4 = var3;
            }
        }

        return var4;
    }

    private static ArrayList<HashSet<Integer>> makeArrayList(HashMap<Integer, HashSet<Integer>> var0, int var1) {
        ArrayList var4 = new ArrayList(var1);

        int var2;
        for(var2 = 0; var2 < var1; ++var2) {
            var4.add(new HashSet());
        }

        for(var2 = 0; var2 < var1; ++var2) {
            for(int var3 = var2 + 1; var3 < var1; ++var3) {
                if(((HashSet)var0.get(Integer.valueOf(var2))).contains(Integer.valueOf(var3))) {
                    ((HashSet)var4.get(var2)).add(Integer.valueOf(var3));
                    ((HashSet)var4.get(var3)).add(Integer.valueOf(var2));
                }
            }
        }

        return var4;
    }

    private static ArrayList<HashSet<Integer>> makeEdgesPA(ArrayList<TreeSet<Integer>> var0, int var1, int var2, int var3, double var4, double var6, Random var8) {
        ArrayList var24 = new ArrayList();
        ArrayList var25 = new ArrayList();
        ArrayList var26 = new ArrayList();
        ArrayList var27 = new ArrayList();

        int var9;
        for(var9 = 0; var9 < var2; ++var9) {
            var25.add(new HashSet());
        }

        while(var27.size() < var2) {
            var27.add(Integer.valueOf(0));
        }

        while(var26.size() < var2) {
            var26.add((Object)null);
        }

        int var10;
        int var13;
        int var17;
        for(var17 = 0; var17 < var1; ++var17) {
            Iterator var23 = ((TreeSet)var0.get(var17)).iterator();

            while(var23.hasNext()) {
                var27.set(((Integer)var23.next()).intValue(), Integer.valueOf(var17));
            }

            var26.set(var17, new ArrayList());
            var23 = ((TreeSet)var0.get(var17)).iterator();

            for(var10 = 0; var10 < var3; ++var10) {
                var13 = ((Integer)var23.next()).intValue();
                ((ArrayList)var26.get(var17)).add(Integer.valueOf(var13));
            }
        }

        int var18 = (int)((double)var3 * var6);

        int var11;
        ArrayList var20;
        for(var17 = 0; var17 < var1; ++var17) {
            var20 = (ArrayList)var26.get(var17);
            var24.clear();

            for(var9 = 0; var9 < var3 - 1; ++var9) {
                for(var10 = var9 + 1; var10 < var3; ++var10) {
                    var24.add(new Pair(((Integer)var20.get(var9)).intValue(), ((Integer)var20.get(var10)).intValue()));
                }
            }

            for(var11 = 0; var11 < var18; ++var11) {
                var9 = var8.nextInt(var24.size());
                Pair var19 = (Pair)var24.remove(var9);
                makeEdge(var19.value1, var19.value2, var25, false);
            }
        }

        HashSet var22;
        for(var17 = 0; var17 < var1; ++var17) {
            var20 = (ArrayList)var26.get(var17);

            for(int var12 = 0; var12 < var3; ++var12) {
                var9 = ((Integer)var20.get(var12)).intValue();
                int var16 = randInt(var4 - var6, var8);
                var22 = new HashSet();

                for(var10 = 0; var10 < var16; ++var10) {
                    do {
                        var11 = var8.nextInt(var1 - 1);
                        if(var17 <= var11) {
                            ++var11;
                        }

                        var13 = findInitVertex(var9, var11, var26, var25, var8);
                    } while(!var22.add(Integer.valueOf(var13)));
                }

                makeEdges(var9, var22, var25);
            }
        }

        for(var9 = 0; var9 < var2; ++var9) {
            if(((HashSet)var25.get(var9)).isEmpty()) {
                var17 = ((Integer)var27.get(var9)).intValue();
                int var14 = randInt(var4, var8);
                int var15 = randInt(var6, var8);
                var22 = new HashSet();

                for(var10 = 0; var10 < var15; ++var10) {
                    do {
                        var13 = findVertex(var9, var17, var0, var25, var8);
                    } while(!var22.add(Integer.valueOf(var13)));
                }

                for(var10 = 0; var10 < var14 - var15; ++var10) {
                    var11 = var8.nextInt(var1 - 1);
                    if(var17 <= var11) {
                        ++var11;
                    }

                    do {
                        var13 = findVertex(var9, var11, var0, var25, var8);
                    } while(!var22.add(Integer.valueOf(var13)));
                }

                makeEdges(var9, var22, var25);
            }
        }

        return var25;
    }

    private static int randInt(double var0, Random var2) {
        int var3 = (int)Math.floor(var0);
        if(randBoolean(var0 - (double)var3, var2)) {
            ++var3;
        }

        return var3;
    }

    private static int findInitVertex(int var0, int var1, ArrayList<ArrayList<Integer>> var2, ArrayList<HashSet<Integer>> var3, Random var4) {
        ArrayList var8 = (ArrayList)var2.get(var1);
        HashSet var9 = (HashSet)var3.get(var0);
        int var7 = var8.size();
        int var5 = var4.nextInt(var8.size());

        int var6;
        do {
            var6 = ((Integer)var8.get((var5 + var7) % var8.size())).intValue();
            --var7;
        } while(var7 >= 0 && (var6 == var0 || var9.contains(Integer.valueOf(var6))));

        return var6;
    }

    private static int findVertex(int var0, int var1, ArrayList<TreeSet<Integer>> var2, ArrayList<HashSet<Integer>> var3, Random var4) {
        int var7 = 0;
        HashSet var10 = (HashSet)var3.get(var0);
        ArrayList var11 = new ArrayList();
        ArrayList var12 = new ArrayList();
        TreeSet var13 = (TreeSet)var2.get(var1);

        int var5;
        int var6;
        for(Iterator var14 = var13.iterator(); var14.hasNext(); var7 += var5) {
            var6 = ((Integer)var14.next()).intValue();
            var11.add(Integer.valueOf(var6));
            var5 = ((HashSet)var3.get(var6)).size();
            var12.add(Integer.valueOf(var5));
        }

        do {
            int var8 = var4.nextInt(var7);
            var6 = -1;

            for(int var9 = 0; var9 < var13.size(); ++var9) {
                var8 -= ((Integer)var12.get(var9)).intValue();
                if(var8 < 0) {
                    var6 = ((Integer)var11.get(var9)).intValue();
                    break;
                }
            }
        } while(var10.contains(Integer.valueOf(var6)) || var0 == var6);

        return var6;
    }

    private static ArrayList<HashSet<Integer>> makeEdgesOld(HashMap<Integer, HashSet<Integer>> var0, int var1, double var2, double var4, Random var6) {
        ArrayList var11 = new ArrayList();

        int var7;
        for(var7 = 0; var7 < var1; ++var7) {
            var11.add(new HashSet());
        }

        int var8;
        if(var4 == 0.0D) {
            Iterator var9 = var0.keySet().iterator();

            while(var9.hasNext()) {
                var7 = ((Integer)var9.next()).intValue();
                Iterator var10 = ((HashSet)var0.get(Integer.valueOf(var7))).iterator();

                while(var10.hasNext()) {
                    var8 = ((Integer)var10.next()).intValue();
                    if(var7 < var8) {
                        makeEdge(var2, var7, var8, var11, var6, false);
                    }
                }
            }
        } else {
            for(var7 = 0; var7 < var1; ++var7) {
                for(var8 = var7 + 1; var8 < var1; ++var8) {
                    if(var0.containsKey(Integer.valueOf(var7)) && ((HashSet)var0.get(Integer.valueOf(var7))).contains(Integer.valueOf(var8))) {
                        makeEdge(var2, var7, var8, var11, var6, false);
                    } else {
                        makeEdge(var4, var7, var8, var11, var6, false);
                    }
                }
            }
        }

        return var11;
    }

    private static ArrayList<HashSet<Integer>> makeEdges(ArrayList<TreeSet<Integer>> var0, int var1, double var2, double var4, double var6, Random var8, double var9, boolean var11) {
        return makeEdgesBi(var0, var0, var1, var1, var2, var2, var4, var6, var8, false, var9, var11);
    }

    private static ArrayList<HashSet<Integer>> makeEdgesBi(ArrayList<TreeSet<Integer>> var0, ArrayList<TreeSet<Integer>> var1, int var2, int var3, double var4, double var6, double var8, double var10, Random var12, boolean var13, double var14, boolean var16) {
        ArrayList var26 = new ArrayList();
        double var27 = (double)var2 * var4 / (double)var0.size();
        double var29 = (double)var3 * var6 / (double)var1.size();
        long var31;
        long var33;
        if(var13) {
            var31 = Math.round(var10 * var27 * ((double)var3 - var29));
            var33 = Math.round(var8 * var27 * var29);
        } else {
            var31 = Math.round(var10 * var27 * ((double)var2 - var27) / 2.0D);
            var33 = Math.round(var8 * var27 * (var27 - 1.0D) / 2.0D);
        }

        int var17;
        for(var17 = 0; var17 < var2; ++var17) {
            var26.add(new HashSet());
        }

        for(int var21 = 0; var21 < var0.size(); ++var21) {
            TreeSet var24 = (TreeSet)var0.get(var21);
            TreeSet var25 = (TreeSet)var1.get(var21);
            Integer[] var35 = new Integer[var24.size()];
            var24.toArray(var35);
            int var18;
            int var20;
            if(var16) {
                Iterator var22 = var24.iterator();

                label67:
                while(var22.hasNext()) {
                    var17 = ((Integer)var22.next()).intValue();
                    Iterator var23 = var25.iterator();

                    while(true) {
                        do {
                            if(!var23.hasNext()) {
                                continue label67;
                            }

                            var18 = ((Integer)var23.next()).intValue();
                        } while(var17 >= var18 && !var13);

                        makeEdge(var8, var17, var18, var26, var12, var13);
                    }
                }
            } else {
                Integer[] var36 = new Integer[var25.size()];
                var25.toArray(var36);
                ArrayList var37 = new ArrayList();
                ArrayList var38 = new ArrayList();
                if(var14 > 0.0D) {
                    for(var20 = 0; var20 < var36.length; ++var20) {
                        var37.add(var36[var20]);
                        var38.add(Integer.valueOf(0));
                    }
                }

                int var39 = 0;
                if(var13 && var14 > -1.0D) {
                    if((long)var25.size() <= var33) {
                        var39 = var25.size();

                        for(var20 = 0; var20 < var39; ++var20) {
                            makeEdge(var35[inVertex(var35, var12)].intValue(), var36[var20].intValue(), var26, true);
                            var38.set(var20, Integer.valueOf(((Integer)var38.get(var20)).intValue() + 1));
                        }
                    } else {
                        System.out.println("Error: initial edges");
                        System.exit(1);
                    }
                }

                for(var20 = var39; (long)var20 < var33; ++var20) {
                    do {
                        var17 = inVertex(var35, var12);
                        if(var14 > -1.0D) {
                            var18 = findVertex1(var17, var14, var25, var26, var37, var38, var20, var12);
                        } else {
                            var18 = inVertex(var36, var12);
                        }
                    } while(!makeEdge(var35[var17].intValue(), var36[var18].intValue(), var26, var13));

                    if(var14 > 0.0D) {
                        var38.set(var18, Integer.valueOf(((Integer)var38.get(var18)).intValue() + 1));
                    }
                }
            }

            for(var20 = 0; (long)var20 < var31; ++var20) {
                do {
                    var17 = inVertex(var35, var12);
                    var18 = outVertex(var3, var25, var12);
                } while(!makeEdge(var35[var17].intValue(), var18, var26, var13));
            }
        }

        return var26;
    }

    private static int findVertex1(int var0, double var1, TreeSet<Integer> var3, ArrayList<HashSet<Integer>> var4, ArrayList<Integer> var5, ArrayList<Integer> var6, int var7, Random var8) {
        HashSet var10 = (HashSet)var4.get(var0);
        double var11 = var8.nextDouble() * ((double)var7 + (double)var3.size() * (var1 - 1.0D));
        boolean var13 = true;

        int var9;
        for(var9 = 0; var9 < var3.size(); ++var9) {
            var11 -= (double)(((Integer)var6.get(var9)).intValue() - 1) + var1;
            if(var11 < 0.0D) {
                break;
            }
        }

        return var9;
    }

    private static int inVertex(Integer[] var0, Random var1) {
        int var2 = var1.nextInt(var0.length);
        return var2;
    }

    private static int outVertex(int var0, TreeSet<Integer> var1, Random var2) {
        int var3;
        do {
            var3 = var2.nextInt(var0);
        } while(var1.contains(Integer.valueOf(var3)));

        return var3;
    }

    private static void makeEdges(int var0, HashSet<Integer> var1, ArrayList<HashSet<Integer>> var2) {
        Iterator var3 = var1.iterator();

        while(var3.hasNext()) {
            makeEdge(var0, ((Integer)var3.next()).intValue(), var2, false);
        }

    }

    private static boolean makeEdge(double var0, int var2, int var3, ArrayList<HashSet<Integer>> var4, Random var5, boolean var6) {
        if(randBoolean(var0, var5)) {
            makeEdge(var2, var3, var4, var6);
            return true;
        } else {
            return false;
        }
    }

    private static boolean makeEdge(int var0, int var1, ArrayList<HashSet<Integer>> var2, boolean var3) {
        boolean var4;
        if(var3) {
            var4 = ((HashSet)var2.get(var0)).add(Integer.valueOf(var1));
        } else if(var0 >= var1) {
            var4 = false;
        } else {
            var4 = ((HashSet)var2.get(var0)).add(Integer.valueOf(var1)) && ((HashSet)var2.get(var1)).add(Integer.valueOf(var0));
        }

        return var4;
    }

    private static HashMap<Integer, HashSet<Integer>> makeEdgesPref(HashMap<Integer, HashSet<Integer>> var0, int var1, int var2, Random var3) {
        ArrayList var13 = new ArrayList();
        ArrayList var14 = new ArrayList();
        HashMap var15 = new HashMap();

        int var7;
        for(var7 = 0; var7 < var1; ++var7) {
            var15.put(Integer.valueOf(var7), new HashSet());
        }

        while(var2 > 0) {
            int var12 = var3.nextInt(var1);
            var13.addAll((Collection)var0.get(Integer.valueOf(var12)));
            int var5 = 0;

            int var8;
            int var9;
            for(var9 = 0; var9 < var13.size(); ++var9) {
                int var6 = ((Integer)var13.get(var9)).intValue();
                int var4 = 0;

                for(var7 = 0; var7 < var6; ++var7) {
                    if(((HashSet)var15.get(Integer.valueOf(var7))).contains(Integer.valueOf(var6))) {
                        ++var4;
                    }
                }

                for(var8 = var6 + 1; var8 < var1; ++var8) {
                    if(((HashSet)var15.get(Integer.valueOf(var6))).contains(Integer.valueOf(var8))) {
                        ++var4;
                    }
                }

                var14.add(Integer.valueOf(var4));
                var5 += var4;
            }

            int var11;
            if(var5 == 0) {
                var11 = ((Integer)var13.get(var3.nextInt(var13.size()))).intValue();
            } else {
                int var10 = var3.nextInt(var5);
                var5 = 0;
                var11 = -1;

                for(var9 = 0; var9 < var13.size(); ++var9) {
                    var5 += ((Integer)var14.get(var9)).intValue();
                    if(var10 < var5) {
                        var11 = ((Integer)var13.get(var9)).intValue();
                        break;
                    }
                }
            }

            var13.clear();
            var14.clear();
            if(var11 < 0) {
                System.out.println("Error in choosing vertex");
                System.exit(1);
            }

            if(var11 < var12) {
                var7 = var11;
                var8 = var12;
            } else {
                var7 = var12;
                var8 = var11;
            }

            if(!((HashSet)var15.get(Integer.valueOf(var7))).contains(Integer.valueOf(var8))) {
                ((HashSet)var15.get(Integer.valueOf(var7))).add(Integer.valueOf(var8));
                ((HashSet)var15.get(Integer.valueOf(var8))).add(Integer.valueOf(var7));
                --var2;
            }
        }

        return var15;
    }

    private static boolean[][] makeEdgesFlat(HashSet<Pair> var0, int var1, int var2, double var3, Random var5) {
        int[] var12 = new int[var1];
        int[] var13 = new int[var1];
        boolean[][] var14 = new boolean[var1][var1];
        int var15 = 0;

        int var7;
        for(var7 = 0; var7 < var1; ++var7) {
            var13[var7] = randIntFromDouble(var3, var5);
            var12[var7] = var2 - var13[var7];
            var15 += var12[var7] + var13[var7];
            System.out.println("nin = " + var12[var7] + ", nout = " + var13[var7]);
        }

        var15 /= 2;
        System.out.println(var15);

        for(int var11 = 0; var11 < var15; ++var11) {
            while(true) {
                var7 = var5.nextInt(var1);
                int var8 = var5.nextInt(var1);
                int var9;
                int var10;
                if(var7 < var8) {
                    var9 = var7;
                    var10 = var8;
                } else {
                    var9 = var8;
                    var10 = var7;
                }

                if(var9 != var10 && !var14[var9][var10]) {
                    Pair var6 = new Pair(var9, var10);
                    if(var0.contains(var6) && var12[var9] > 0 && var12[var10] > 0) {
                        var14[var9][var10] = true;
                        --var12[var9];
                        --var12[var10];
                        System.out.println("intra edge " + var9 + " " + var10);
                        break;
                    }

                    if(!var0.contains(var6) && var13[var9] > 0 && var13[var10] > 0) {
                        var14[var9][var10] = true;
                        --var13[var9];
                        --var13[var10];
                        System.out.println("inter edge " + var9 + " " + var10);
                        break;
                    }
                }
            }
        }

        return var14;
    }

    private static int randIntFromDouble(double var0, Random var2) {
        int var3 = (int)var0;
        if(randBoolean(var0 - (double)var3, var2)) {
            ++var3;
        }

        return var3;
    }

    private static boolean randBoolean(double var0, Random var2) {
        return var2.nextDouble() < var0;
    }

    private static HashMap<Integer, HashSet<Integer>> makeSameGroup(int var0, int var1, ArrayList<TreeSet<Integer>> var2) {
        HashMap var8 = new HashMap();

        for(int var5 = 0; var5 < var1; ++var5) {
            Iterator var6 = ((TreeSet)var2.get(var5)).iterator();

            while(var6.hasNext()) {
                int var3 = ((Integer)var6.next()).intValue();

                int var4;
                for(Iterator var7 = ((TreeSet)var2.get(var5)).iterator(); var7.hasNext(); ((HashSet)var8.get(Integer.valueOf(var4))).add(Integer.valueOf(var3))) {
                    var4 = ((Integer)var7.next()).intValue();
                    if(!var8.containsKey(Integer.valueOf(var3))) {
                        var8.put(Integer.valueOf(var3), new HashSet());
                    }

                    ((HashSet)var8.get(Integer.valueOf(var3))).add(Integer.valueOf(var4));
                    if(!var8.containsKey(Integer.valueOf(var4))) {
                        var8.put(Integer.valueOf(var4), new HashSet());
                    }
                }
            }
        }

        return var8;
    }

    private static ArrayList<TreeSet<Integer>> makeGroups(int var0, int var1, double var2, int var4, Random var5, boolean var6) {
        boolean var19 = true;
        ArrayList var20 = new ArrayList(var1);

        int var12;
        for(var12 = 0; var12 < var1; ++var12) {
            var20.add(new TreeSet());
        }

        int var8;
        TreeSet var17;
        for(var8 = 0; var8 < var4; ++var8) {
            var17 = new TreeSet();
            var17.add(Integer.valueOf(var0 - var8 - 1));
            var20.add(var17);
        }

        var0 -= var4;
        var12 = -1;

        int var9;
        int var16;
        for(var9 = 0; var9 < var0; ++var9) {
            var16 = individualName(var9);
            if(var6) {
                var12 = (var12 + 1) % var1;
            } else {
                var12 = var5.nextInt(var1);
            }

            ((TreeSet)var20.get(var12)).add(Integer.valueOf(var16));
        }

        int var13 = (int)((double)var0 * (var2 - 1.0D));
        int var21 = (int)var2;
        if((double)var21 == var2) {
            --var21;
        }

        TreeMap var22 = new TreeMap();

        int var11;
        for(var9 = 0; var9 < var0; ++var9) {
            for(var8 = 0; var8 < var21; ++var8) {
                Integer var7 = (Integer)var22.get(Integer.valueOf(var9));
                if(var7 == null) {
                    var11 = 1;
                } else {
                    var11 = var7.intValue();
                    ++var11;
                }

                var22.put(Integer.valueOf(var9), Integer.valueOf(var11));
            }
        }

        for(int var14 = 0; var14 < var13; ++var14) {
            if(var6) {
                var12 = (var12 + 1) % var1;
            } else {
                var12 = var5.nextInt(var1);
            }

            var17 = (TreeSet)var20.get(var12);
            if(var6) {
                Iterator var18 = var22.keySet().iterator();
                var19 = false;

                while(var18.hasNext()) {
                    int var15 = ((Integer)var18.next()).intValue();
                    var11 = ((Integer)var22.get(Integer.valueOf(var15))).intValue();
                    if(!var17.contains(Integer.valueOf(individualName(var15)))) {
                        var19 = true;
                        break;
                    }
                }

                if(!var19) {
                    break;
                }
            }

            do {
                do {
                    var9 = var5.nextInt(var0);
                    var16 = individualName(var9);
                } while(var17.contains(Integer.valueOf(var16)));
            } while(var6 && !var22.containsKey(Integer.valueOf(var9)));

            var17.add(Integer.valueOf(var16));
            if(var6) {
                var11 = Integer.valueOf(var9).intValue();
                int var10 = ((Integer)var22.get(Integer.valueOf(var11))).intValue();
                if(var10 == 1) {
                    var22.remove(Integer.valueOf(var11));
                } else {
                    var22.put(Integer.valueOf(var11), Integer.valueOf(var10 - 1));
                }
            }
        }

        if(!var19) {
            var20 = null;
        }

        return var20;
    }

    private static int individualName(int var0) {
        return var0;
    }

    private static void printBoolean(boolean[][] var0, int var1) {
        for(int var2 = 0; var2 < var1; ++var2) {
            for(int var3 = 0; var3 < var1; ++var3) {
                if(var0[var2][var3]) {
                    System.out.print("1");
                } else {
                    System.out.print("0");
                }
            }

            System.out.println();
        }

    }

    private static HashSet<Integer> componentOf(int var0, ArrayList<HashSet<Integer>> var1) {
        LinkedList var5 = new LinkedList();
        HashSet var6 = new HashSet();
        var6.add(Integer.valueOf(var0));
        var5.addLast(Integer.valueOf(var0));

        while(!var5.isEmpty()) {
            int var2 = ((Integer)var5.removeFirst()).intValue();
            Iterator var4 = ((HashSet)var1.get(var2)).iterator();

            while(var4.hasNext()) {
                int var3 = ((Integer)var4.next()).intValue();
                if(!var6.contains(Integer.valueOf(var3))) {
                    var6.add(Integer.valueOf(var3));
                    var5.addLast(Integer.valueOf(var3));
                }
            }
        }

        return var6;
    }

    private static void printUsageAndExit() {
        System.err.println("Usage: java " + GG.class.getName() + " [-n #] [-g #] [-r #] [-pi #] [-po #]");
        System.err.println("Options:");
        System.err.println("  -n Number of individuals (>= nGroups)");
        System.err.println("  -g Number of groups (>= 1)");
        System.err.println("  -r Mean # of groups per individual (>= 1)");
        System.err.println("  -pi Probability of intragroup edges (0 - 1)");
        System.err.println("  -po Probability of intergroup edges (0 - 1)");
        System.exit(1);
    }
}
