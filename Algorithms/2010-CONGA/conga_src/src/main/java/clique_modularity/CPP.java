package clique_modularity;//
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
        if(var1 < 3) {
            printUsageAndExit();
        }

        String var2 = var0[0];
        String var3 = var0[1];
        String var4 = var0[2];
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
            }
        }

        if(!var0[1].equals("none")) {
            process(var5, var3, var4);
        }

    }

    public static int preprocess(String var0, String var1, String var2) {
        ArrayList var3 = new ArrayList();
        HashMap var4 = CONGA.readGraphEdges(var0, (String)null, false, (HashMap)null);
        ArrayList var5 = new ArrayList();
        CONGA.compactGraph(var4, var5, (HashMap)null, (List)null, var3);
        CONGA.writeSplitGraph(var5, (List)null, var3, var1, var2);
        return var5.size();
    }

    public static void process(String var0, String var1, String var2) {
        ArrayList var8 = new ArrayList();
        HashSet var9 = new HashSet();

        try {
            BufferedReader var10 = new BufferedReader(new FileReader(var1));

            String var5;
            while((var5 = var10.readLine()) != null) {
                var8.add(CONGA.rootName(var5));
            }

            var10.close();
            var10 = new BufferedReader(new FileReader(var0));
            PrintStream var11 = new PrintStream(new FileOutputStream(var2));

            while((var5 = var10.readLine()) != null) {
                String[] var6 = var5.split(" ");
                var9.clear();
                byte var4;
                if(var6[0].endsWith(":")) {
                    var4 = 1;
                } else {
                    var4 = 0;
                }

                for(int var3 = var4; var3 < var6.length; ++var3) {
                    var9.add(var8.get(Integer.parseInt(var6[var3])));
                }

                Iterator var7 = var9.iterator();

                while(var7.hasNext()) {
                    var11.print((String)var7.next() + " ");
                }

                var11.println();
            }

            var10.close();
            var11.close();
        } catch (Exception var12) {
            System.out.println(var12.toString());
            var12.printStackTrace();
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
        boolean var8 = true;
        File var9 = new File(var1);
        var9.delete();

        try {
            BufferedReader var10 = new BufferedReader(new FileReader(var0));
            PrintStream var11 = new PrintStream(new FileOutputStream(var1, true));

            while(true) {
                String var5;
                do {
                    if((var5 = var10.readLine()) == null) {
                        var10.close();
                        var11.close();
                        return;
                    }
                } while(!var5.startsWith("community "));

                int var3 = var5.indexOf("{");
                int var4 = var5.indexOf("}");
                var5 = var5.substring(var3 + 1, var4);
                String[] var7 = var5.split(", ");
                String var6 = "";

                for(int var2 = 0; var2 < var7.length; ++var2) {
                    var11.print(var6 + var7[var2]);
                    var6 = " ";
                }

                var11.println();
            }
        } catch (Exception var12) {
            System.out.println("Walktrap error: " + var12.toString());
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
