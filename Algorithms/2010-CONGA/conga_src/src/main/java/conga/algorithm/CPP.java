package conga.algorithm;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import conga.CONGA;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.PrintStream;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.TreeSet;
import java.util.Vector;

public class CPP {
    public CPP() {
    }

    public static void main(String[] var0) {
        int var1 = Array.getLength(var0);
        String var2;
        String var3;
        String var4;
        if(var0[0].equals("-m")) {
            var2 = var0[1];
            var3 = var2.substring(0, var2.lastIndexOf(".")) + "-num.txt";
            var4 = var2.substring(0, var2.lastIndexOf(".")) + "-ver.txt";
            preprocess(var2, var3, var4, false, false, false, false);
            System.exit(1);
        }

        if(var0[0].equals("-md")) {
            var2 = var0[1];
            var3 = var2.substring(0, var2.lastIndexOf(".")) + "-num.txt";
            var4 = var2.substring(0, var2.lastIndexOf(".")) + "-ver.txt";
            preprocessDir(var2, var3, var4);
            System.exit(1);
        }

        if(var0[0].equals("-a")) {
            var2 = var0[1];
            var3 = var0[2];
            alspac(var2, var3);
            System.exit(1);
        }

        if(var1 < 3) {
            printUsageAndExit();
        }

        var2 = var0[0];
        var3 = var0[1];
        var4 = var0[2];
        String var5 = var2;
        if(var1 >= 4) {
            var5 = "$cpp$clusters$.txt";
            (new File(var5)).delete();
            if(var0[3].equals("-cnm")) {
                convert_cnm(var2, var5);
            } else if(var0[3].equals("-wt")) {
                convert_wakita_clusters(var2, var5, Integer.parseInt(var0[4]));
            } else if(var0[3].equals("-pl")) {
                convert_pons_clusters(var2, var5);
            } else if(var0[3].equals("-bgll")) {
                convert_blondel_clusters(var2, var5);
            } else if(var0[3].equals("-fc")) {
                convert_fc_clusters(var2, var5);
            } else if(var0[3].equals("-nmf")) {
                convert_nmf_clusters(var2, var5);
            } else if(var0[3].equals("-eagle")) {
                convert_eagle_clusters(var2, var5);
            } else if(var0[3].equals("-im")) {
                convert_im_clusters(var2, var5);
            }
        }

        if(!var0[1].equals("none")) {
            process(var5, var3, var4);
        }

    }

    public static void preprocessDir(String var0, String var1, String var2) {
        int var9 = 0;
        int var10 = 0;
        ArrayList var11 = new ArrayList();
        HashMap var12 = new HashMap();
        HashMap var13 = new HashMap();

        try {
            BufferedReader var14 = new BufferedReader(new FileReader(var0));
            PrintStream var15 = new PrintStream(new FileOutputStream(var1));

            String var3;
            while((var3 = var14.readLine()) != null) {
                ++var9;
                String[] var6 = var3.split("\t");
                if(Array.getLength(var6) != 2) {
                    System.err.println("Ignored: " + var3);
                    ++var10;
                } else {
                    String var4 = var6[0];
                    String var5 = var6[1];
                    if(!var12.containsKey(var4)) {
                        var12.put(var4, Integer.valueOf(var11.size()));
                        var11.add(var4);
                    }

                    if(!var12.containsKey(var5)) {
                        var12.put(var5, Integer.valueOf(var11.size()));
                        var11.add(var5);
                    }

                    int var7 = ((Integer)var12.get(var4)).intValue();
                    int var8 = ((Integer)var12.get(var5)).intValue();
                    if(var7 == var8) {
                        System.err.println("Ignored self-edge: " + (String)var11.get(var7) + "/" + (String)var11.get(var7));
                        ++var10;
                    } else {
                        if(!var13.containsKey(Integer.valueOf(var7))) {
                            var13.put(Integer.valueOf(var7), new HashSet());
                        }

                        if(((HashSet)var13.get(Integer.valueOf(var7))).add(Integer.valueOf(var8))) {
                            var15.println(var7 + "\t" + var8);
                        } else {
                            System.err.println("Ignored duplicate edge: " + (String)var11.get(var7) + "/" + (String)var11.get(var8));
                            ++var10;
                        }
                    }
                }
            }

            var14.close();
            var15.close();
            var15 = new PrintStream(new FileOutputStream(var2));
            Iterator var16 = var11.iterator();

            while(var16.hasNext()) {
                String var17 = (String)var16.next();
                var15.println(var17);
            }

            var15.close();
            System.out.println(var9 + " edges read");
            System.out.println(var10 + " of these were ignored");
            System.out.println(var11.size() + " vertices");
        } catch (Exception var18) {
            System.out.println("Line " + var9);
            System.out.println(var18.toString());
            var18.printStackTrace();
            System.exit(1);
        }

    }

    public static int preprocess(String var0, String var1, String var2, boolean var3, boolean var4, boolean var5, boolean var6) {
        ArrayList var7 = new ArrayList();
        HashMap var8 = null;
        ArrayList var9 = null;
        if(var5) {
            var8 = new HashMap();
            var9 = new ArrayList();
        }

        HashMap var10 = CONGA.readGraphEdges(var0, (String)null, var6, var8);
        ArrayList var11 = new ArrayList();
        CONGA.compactGraph(var10, var11, var8, var9, var7);
        CONGA.writeSplitGraph(var11, var9, var7, var1, var2, var3, var4);
        return var11.size();
    }

    public static void alspac(String var0, String var1) {
        int var3 = 0;

        try {
            BufferedReader var10 = new BufferedReader(new FileReader(var0));

            String var4;
            PrintStream var11;
            for(var11 = new PrintStream(new FileOutputStream(var1)); (var4 = var10.readLine()) != null; var11.println()) {
                ++var3;
                String[] var8 = var4.split("\t");
                var11.print(var8[1] + var8[0] + "\t");
                if(var8[4].equals("Y") && !"".equals(var8[5]) && !"".equals(var8[6])) {
                    var11.print(var8[6] + var8[5]);
                } else if(var8[4].equals("Y")) {
                    System.err.println("Missing ALSPAC ID on line " + var3);
                    var11.print(var8[3]);
                } else {
                    var11.print(var8[3]);
                }
            }

            var10.close();
            var11.close();
        } catch (Exception var12) {
            System.out.println("Line " + var3);
            System.out.println(var12.toString());
            var12.printStackTrace();
            System.exit(1);
        }

    }

    public static void process(String var0, String var1, String var2) {
        ArrayList var12 = new ArrayList();
        HashSet var13 = new HashSet();

        try {
            BufferedReader var14 = new BufferedReader(new FileReader(var1));

            String var5;
            while((var5 = var14.readLine()) != null) {
                var12.add(var5);
            }

            var14.close();
            var14 = new BufferedReader(new FileReader(var0));
            PrintStream var15 = new PrintStream(new FileOutputStream(var2));

            while(true) {
                do {
                    if((var5 = var14.readLine()) == null) {
                        var14.close();
                        var15.close();
                        return;
                    }
                } while(var5.startsWith("#"));

                String[] var9 = var5.split(" ");
                var13.clear();
                byte var4;
                if(var9[0].endsWith(":")) {
                    var4 = 1;
                } else {
                    var4 = 0;
                }

                for(int var3 = var4; var3 < var9.length; ++var3) {
                    String var7 = var9[var3];
                    String[] var10 = var7.split(":");
                    var7 = var10[0];
                    String var6 = (String)var12.get(Integer.parseInt(var7));
                    var15.print(var6);
                    if(Array.getLength(var10) > 1) {
                        var15.print(":" + var10[1]);
                    }

                    var15.print(" ");
                }

                var15.println();
            }
        } catch (Exception var16) {
            System.out.println(var16.toString());
            var16.printStackTrace();
            System.exit(1);
        }
    }

    public static void convert_wakita_clusters(String var0, String var1, int var2) {
        boolean var3 = true;
        int var7 = 0;
        int var9 = 0;
        HashMap var13 = new HashMap();
        File var14 = new File(var1);
        var14.delete();

        try {
            BufferedReader var15 = new BufferedReader(new FileReader(var0));

            int var8;
            label63:
            do {
                String[] var10;
                do {
                    String var4;
                    if((var4 = var15.readLine()) == null) {
                        break label63;
                    }

                    var10 = var4.trim().split(" ");
                } while(!var10[0].endsWith(":"));

                int var5 = Integer.parseInt(var10[1].substring(0, var10[1].indexOf("("))) - 1;
                int var6 = Integer.parseInt(var10[3].substring(0, var10[3].indexOf("("))) - 1;
                var7 = Integer.parseInt(var10[5]) - 1;
                if(var3) {
                    var3 = false;

                    for(var8 = 0; var8 < var7; ++var8) {
                        TreeSet var12 = new TreeSet();
                        var12.add(Integer.valueOf(var8));
                        var13.put(Integer.valueOf(var8), var12);
                    }

                    var9 = var7;
                }

                var13.put(Integer.valueOf(var7), var13.get(Integer.valueOf(var5)));
                ((TreeSet)var13.get(Integer.valueOf(var7))).addAll((Collection)var13.get(Integer.valueOf(var6)));
                var13.remove(Integer.valueOf(var5));
                var13.remove(Integer.valueOf(var6));
                --var9;
            } while(var9 != var2);

            PrintStream var16 = new PrintStream(new FileOutputStream(var1, true));

            for(var8 = 0; var8 <= var7; ++var8) {
                if(var13.get(Integer.valueOf(var8)) != null) {
                    Iterator var11 = ((TreeSet)var13.get(Integer.valueOf(var8))).iterator();

                    while(var11.hasNext()) {
                        var16.print(var11.next() + " ");
                    }

                    var16.println();
                }
            }

            var15.close();
            var16.close();
        } catch (Exception var17) {
            System.out.println("Wakita error: " + var17.toString());
        }

    }

    public static void convert_cnm_clusters(String var0, String var1) {
        String var2 = var0.substring(0, var0.lastIndexOf(46));
        String var3 = var2 + "-fc_a.groups";
        convert_cnm(var3, var1);
    }

    public static void convert_cnm_network(String var0, String var1) {
        File var4 = new File(var1);
        var4.delete();

        try {
            BufferedReader var5 = new BufferedReader(new FileReader(var0));
            PrintStream var6 = new PrintStream(new FileOutputStream(var1, true));

            String var2;
            while((var2 = var5.readLine()) != null) {
                String[] var3 = var2.split("\t");
                var6.println(var3[0] + "\t" + var3[1] + "\t" + (int)(1000.0F * Float.parseFloat(var3[2])));
            }

            var5.close();
            var6.close();
        } catch (Exception var7) {
            System.out.println("CNM error: " + var7.toString());
        }

    }

    private static void convert_cnm(String var0, String var1) {
        boolean var3 = true;
        File var4 = new File(var1);
        var4.delete();

        try {
            BufferedReader var5 = new BufferedReader(new FileReader(var0));
            PrintStream var6 = new PrintStream(new FileOutputStream(var1, true));

            String var2;
            while((var2 = var5.readLine()) != null) {
                if(var2.startsWith("GROUP")) {
                    if(var3) {
                        var3 = false;
                    } else {
                        var6.println();
                    }
                } else {
                    var6.print(var2 + " ");
                }
            }

            var6.println();
            var5.close();
            var6.close();
        } catch (Exception var7) {
            System.out.println("CNM error: " + var7.toString());
        }

    }

    public static void convert_pons_clusters(String var0, String var1) {
        File var8 = new File(var1);
        var8.delete();

        try {
            BufferedReader var9 = new BufferedReader(new FileReader(var0));
            PrintStream var10 = new PrintStream(new FileOutputStream(var1, true));

            while(true) {
                String var5;
                do {
                    if((var5 = var9.readLine()) == null) {
                        var9.close();
                        var10.close();
                        return;
                    }
                } while(!var5.startsWith("community "));

                int var3 = var5.indexOf("{");
                int var4 = var5.indexOf("}");
                var5 = var5.substring(var3 + 1, var4);
                String[] var7 = var5.split(", ");
                String var6 = "";

                for(int var2 = 0; var2 < var7.length; ++var2) {
                    var10.print(var6 + var7[var2]);
                    var6 = " ";
                }

                var10.println();
            }
        } catch (Exception var11) {
            System.out.println("Walktrap error: " + var11.toString());
        }
    }

    public static void convert_lfm_clusters(String var0, String var1) {
        boolean var6 = false;
        File var7 = new File(var1);
        var7.delete();

        try {
            BufferedReader var8 = new BufferedReader(new FileReader(var0));
            PrintStream var9 = new PrintStream(new FileOutputStream(var1, true));

            String var3;
            while((var3 = var8.readLine()) != null && !var3.startsWith("collection 0")) {
                ;
            }

            while(true) {
                while((var3 = var8.readLine()) != null && !var3.startsWith("-----")) {
                    if(var6) {
                        var6 = false;
                        String[] var5 = var3.split("\t");
                        String var4 = "";

                        for(int var2 = 0; var2 < var5.length; ++var2) {
                            var9.print(var4 + var5[var2]);
                            var4 = " ";
                        }

                        var9.println();
                    } else if(var3.startsWith("weak = ")) {
                        var6 = true;
                    }
                }

                var8.close();
                var9.close();
                break;
            }
        } catch (Exception var10) {
            System.out.println("LFM error: " + var10.toString());
        }

    }

    public static void convert_fc_clusters(String var0, String var1) {
        int var4 = 0;
        ArrayList var9 = new ArrayList();

        try {
            BufferedReader var10 = new BufferedReader(new FileReader(var0));

            while(true) {
                while(true) {
                    int var2;
                    int var3;
                    float var6;
                    String var7;
                    do {
                        if((var7 = var10.readLine()) == null) {
                            var10.close();
                            PrintStream var11 = new PrintStream(new FileOutputStream(var1));

                            for(var2 = 0; var2 < var4; ++var2) {
                                Iterator var5 = ((HashMap)var9.get(var2)).keySet().iterator();

                                while(var5.hasNext()) {
                                    var3 = ((Integer)var5.next()).intValue();
                                    var6 = ((Float)((HashMap)var9.get(var2)).get(Integer.valueOf(var3))).floatValue();
                                    var11.print(var3 + ":" + var6 + " ");
                                }

                                var11.println();
                            }

                            var11.close();
                            return;
                        }
                    } while(var7.startsWith("%"));

                    String[] var8 = var7.split("\t");
                    if(var8[0].equals("conga.util.graph.Vertex")) {
                        for(var2 = 1; var8[var2].startsWith("u_"); ++var2) {
                            var9.add(new HashMap());
                        }

                        var4 = var2 - 1;
                    } else {
                        var3 = Integer.parseInt(var8[0]);

                        for(var2 = 0; var2 < var4; ++var2) {
                            var6 = Float.parseFloat(var8[var2 + 1]);
                            if(var6 > 0.0F) {
                                ((HashMap)var9.get(var2)).put(Integer.valueOf(var3), Float.valueOf(var6));
                            }
                        }
                    }
                }
            }
        } catch (Exception var12) {
            System.out.println("fuzzyclusters error: " + var12.toString());
        }
    }

    public static void convert_nmf_clusters(String var0, String var1) {
        int var3 = 0;
        int var4 = 0;
        ArrayList var10 = new ArrayList();

        try {
            int var2;
            float var7;
            String var8;
            BufferedReader var11;
            for(var11 = new BufferedReader(new FileReader(var0)); (var8 = var11.readLine()) != null; ++var3) {
                String[] var9 = var8.split(",");
                var4 = Array.getLength(var9);

                for(var2 = 0; var2 < var4; ++var2) {
                    if(var3 == 0) {
                        var10.add(new HashMap());
                    }

                    var7 = Float.parseFloat(var9[var2]);
                    if((double)var7 > 0.001D) {
                        ((HashMap)var10.get(var2)).put(Integer.valueOf(var3), Float.valueOf(var7));
                    }
                }
            }

            var11.close();
            PrintStream var12 = new PrintStream(new FileOutputStream(var1));

            for(var2 = 0; var2 < var4; ++var2) {
                HashMap var6 = (HashMap)var10.get(var2);
                if(!var6.isEmpty()) {
                    Iterator var5 = var6.keySet().iterator();

                    while(var5.hasNext()) {
                        var3 = ((Integer)var5.next()).intValue();
                        var7 = ((Float)((HashMap)var10.get(var2)).get(Integer.valueOf(var3))).floatValue();
                        var12.print(var3 + ":" + var7 + " ");
                    }

                    var12.println();
                }
            }

            var12.close();
        } catch (Exception var13) {
            System.out.println("nmf error: " + var13.toString());
        }

    }

    public static void convert_eagle_clusters(String var0, String var1) {
        try {
            BufferedReader var5 = new BufferedReader(new FileReader(var0));
            PrintStream var6 = new PrintStream(new FileOutputStream(var1));
            var5.readLine();

            String var3;
            while((var3 = var5.readLine()) != null) {
                String[] var4 = var3.split("[ \t]");
                var6.print(var4[0]);

                for(int var2 = 1; var2 < Array.getLength(var4); ++var2) {
                    var6.print(" " + var4[var2]);
                }

                var6.println();
            }

            var5.close();
            var6.close();
        } catch (Exception var7) {
            System.out.println("eagle error: " + var7.toString());
        }

    }

    public static void convert_im_clusters(String var0, String var1) {
        int var4 = 0;
        String[] var7 = null;
        String[] var8 = null;
        HashSet var9 = null;
        ArrayList var10 = new ArrayList();

        try {
            BufferedReader var11 = new BufferedReader(new FileReader(var0));

            String var5;
            while((var5 = var11.readLine()) != null && !var5.equals("")) {
                if(!var5.startsWith("#")) {
                    var7 = var5.split(" ");
                    var8 = var7[0].split(":");
                    int var3 = Integer.parseInt(var8[0]);
                    String var6 = var7[2].substring(1, var7[2].length() - 1);
                    if(var3 != var4) {
                        if(var3 > 1) {
                            var10.add(var9);
                        }

                        var9 = new HashSet();
                    }

                    var9.add(var6);
                    var4 = var3;
                }
            }

            if(var9 != null) {
                var10.add(var9);
            }

            var11.close();
            PrintStream var12 = new PrintStream(new FileOutputStream(var1));
            Iterator var13 = var10.iterator();

            while(var13.hasNext()) {
                HashSet var14 = (HashSet)var13.next();
                Iterator var15 = var14.iterator();

                while(var15.hasNext()) {
                    String var16 = (String)var15.next();
                    var12.print(var16 + " ");
                }

                var12.println();
            }

            var12.close();
        } catch (Exception var17) {
            System.out.println("infomap error: " + var17.toString());
            System.exit(1);
        }

    }

    public static void convert_oslom_clusters(String var0, String var1) {
        try {
            PrintStream var3 = new PrintStream(new FileOutputStream(var1));
            BufferedReader var4 = new BufferedReader(new FileReader(var0));

            String var2;
            while((var2 = var4.readLine()) != null && !var2.equals("")) {
                if(!var2.startsWith("#")) {
                    var3.println(var2);
                }
            }

            var4.close();
            var3.close();
        } catch (Exception var5) {
            System.out.println("oslom error: " + var5.toString());
            System.exit(1);
        }

    }

    public static int convert_nmf_graph(String var0, String var1) {
        ArrayList var8 = new ArrayList();

        try {
            BufferedReader var9 = new BufferedReader(new FileReader(var0));

            while(true) {
                int var2;
                int var3;
                String var5;
                do {
                    do {
                        if((var5 = var9.readLine()) == null) {
                            var9.close();
                            PrintStream var10 = new PrintStream(new FileOutputStream(var1));

                            for(var2 = 0; var2 < var8.size(); ++var2) {
                                int[] var7 = new int[var8.size()];

                                for(Iterator var4 = ((HashSet)var8.get(var2)).iterator(); var4.hasNext(); var7[var3] = 1) {
                                    var3 = ((Integer)var4.next()).intValue();
                                }

                                for(var3 = 0; var3 < var8.size(); ++var3) {
                                    var10.print(var7[var3] + " ");
                                }

                                var10.println();
                            }

                            var10.close();
                            return var8.size();
                        }
                    } while(var5.equals(""));
                } while(var5.startsWith("#"));

                String[] var6 = var5.split("[ \t]");
                var2 = Integer.parseInt(var6[0]);
                var3 = Integer.parseInt(var6[1]);

                while(var8.size() <= var2) {
                    var8.add(new HashSet());
                }

                ((HashSet)var8.get(var2)).add(Integer.valueOf(var3));

                while(var8.size() <= var3) {
                    var8.add(new HashSet());
                }

                ((HashSet)var8.get(var3)).add(Integer.valueOf(var2));
            }
        } catch (Exception var11) {
            System.out.println("nmf error: " + var11.toString());
            return var8.size();
        }
    }

    public static void convert_eagle_graph(String var0, String var1, String var2) {
        ArrayList var6 = new ArrayList();
        ArrayList var7 = new ArrayList();
        HashMap var8 = CONGA.readGraphEdges(var0, (String)null, false, (HashMap)null);
        CONGA.compactGraph(var8, var6, (HashMap)null, (List)null, var7);

        try {
            PrintStream var9 = new PrintStream(new FileOutputStream(var2));

            int var3;
            for(var3 = 0; var3 < var7.size(); ++var3) {
                var9.println((String)var7.get(var3));
            }

            var9.close();
            var9 = new PrintStream(new FileOutputStream(var1));
            var9.println(var7.size());

            for(var3 = 0; var3 < var6.size(); ++var3) {
                Iterator var5 = ((HashSet)var6.get(var3)).iterator();

                while(var5.hasNext()) {
                    int var4 = ((Integer)var5.next()).intValue();
                    if(var3 < var4) {
                        var9.println(var3 + 1 + "\t" + (var4 + 1));
                    }
                }
            }

            var9.close();
        } catch (Exception var10) {
            System.out.println("eagle error: " + var10.toString());
        }

    }

    public static void convert_im_graph(String var0, String var1) {
        ArrayList var8 = new ArrayList();
        ArrayList var9 = new ArrayList();
        ArrayList var10 = new ArrayList();
        int var11 = -1;

        try {
            BufferedReader var12 = new BufferedReader(new FileReader(var0));

            String var6;
            while((var6 = var12.readLine()) != null) {
                if(!var6.equals("") && !var6.startsWith("#")) {
                    String[] var7 = var6.split("[ \t]");
                    int var3 = Integer.parseInt(var7[0]);
                    int var4 = Integer.parseInt(var7[1]);
                    if(Array.getLength(var7) >= 3) {
                        var10.add(Float.valueOf(Float.parseFloat(var7[2])));
                    } else {
                        var10.add(Float.valueOf(1.0F));
                    }

                    var8.add(Integer.valueOf(var3));
                    var9.add(Integer.valueOf(var4));
                    if(var11 < var3) {
                        var11 = var3;
                    }

                    if(var11 < var4) {
                        var11 = var4;
                    }
                }
            }

            var12.close();
            PrintStream var13 = new PrintStream(new FileOutputStream(var1));
            var13.println("*Vertices " + (var11 + 1));

            int var2;
            for(var2 = 0; var2 <= var11; ++var2) {
                var13.println(var2 + 1 + " " + var2);
            }

            var13.println("*Edges");

            for(var2 = 0; var2 < var8.size(); ++var2) {
                if(((Integer)var8.get(var2)).intValue() < ((Integer)var9.get(var2)).intValue()) {
                    var13.println(((Integer)var8.get(var2)).intValue() + 1 + " " + (((Integer)var9.get(var2)).intValue() + 1) + " " + var10.get(var2));
                }
            }

            var13.close();
        } catch (Exception var14) {
            System.out.println("infomap error: " + var14.toString());
        }

    }

    public static boolean convert_blondel_clusters(String var0, String var1) {
        Vector var7 = new Vector();
        File var8 = new File(var1);
        var8.delete();

        try {
            int var2;
            String var4;
            String[] var6;
            BufferedReader var9;
            for(var9 = new BufferedReader(new FileReader(var0)); (var4 = var9.readLine()) != null; var7.add(Integer.valueOf(Integer.parseInt(var6[1])))) {
                var6 = var4.split(" ");
                var2 = Integer.parseInt(var6[0]);
                if(var2 != var7.size()) {
                    System.out.println("Mismatch " + var2 + " " + var7.size());
                    System.exit(1);
                }
            }

            var9.close();
            if(var7.size() == 0) {
                return false;
            }

            int var3 = 0;
            PrintStream var10 = new PrintStream(new FileOutputStream(var1, true));

            while(true) {
                String var5 = "";

                for(var2 = 0; var2 < var7.size(); ++var2) {
                    if(((Integer)var7.get(var2)).intValue() == var3) {
                        var10.print(var5 + var2);
                        var5 = " ";
                    }
                }

                if(var5.equals("")) {
                    var10.close();
                    break;
                }

                ++var3;
                var10.println();
            }
        } catch (Exception var11) {
            System.out.println("Blondel error: " + var11.toString());
        }

        return true;
    }

    private static void printUsageAndExit() {
        System.err.println("Usage: java " + CPP.class.getName() + " <clusterNumsFile> <vertexFile> <clustersFile>");
        System.exit(1);
    }
}
