package clique_modularity;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TreeSet;

public class MCQ {
    static ARRAY_FIX QMAX;
    static ARRAY_FIX Q;
    static ARRAY_FIX[] C;
    static int UPPER_BOUND = 2147483647;
    static long t1 = 0L;
    static long t2 = 0L;
    static long t3 = 0L;
    static long t4 = 0L;

    public MCQ() {
    }

    public static void main(String[] var0) {
        HashSet var1 = mcq(var0[0]);
        System.out.print("Max Clique elements are ");
        System.out.println(var1);
        System.out.println("Maximum clique size = " + var1.size());
    }

    public static HashSet<Integer> mcq(String var0) {
        ArrayList var1 = new ArrayList();
        int var2 = readGraph(var0, var1);
        return mcq(var1, var2 + 1, 2147483647);
    }

    public static HashSet<Integer> mcq(ArrayList<HashSet<Integer>> var0, int var1, int var2) {
        ARRAY2 var3 = convert_V(var1);
        boolean[][] var4 = convert_e(var0);
        return mcq(var0, var3, var4, var1, var2, (TreeSet) null);
    }

    public static HashSet<Integer> mcq(ArrayList<HashSet<Integer>> var0, ARRAY2 var1, boolean[][] var2, int var3, int var4, TreeSet<ELEMENT> var5) {
        UPPER_BOUND = var4;
        QMAX = new ARRAY_FIX(var3);
        Q = new ARRAY_FIX(var3);
        C = new ARRAY_FIX[var3];
        MCQ(var0, var1, var2, var3, var5);
        HashSet var8 = new HashSet();

        for (int var9 = 0; var9 < QMAX.size; ++var9) {
            var8.add(Integer.valueOf(QMAX.ele[var9]));
        }

        return var8;
    }

    private static int readGraph(String var0, ArrayList<HashSet<Integer>> var1) {
        String[] var7 = null;
        int var8 = 0;

        try {
            BufferedReader var9 = new BufferedReader(new FileReader(var0));

            String var6;
            while ((var6 = var9.readLine()) != null) {
                var7 = var6.split(" ");
                int var2 = Integer.parseInt(var7[0]);
                int var3 = Integer.parseInt(var7[1]);
                int var4;
                int var5;
                if (var2 < var3) {
                    var4 = var2;
                    var5 = var3;
                } else {
                    var4 = var3;
                    var5 = var2;
                }

                while (var1.size() <= var4) {
                    var1.add(new HashSet());
                }

                ((HashSet) var1.get(var4)).add(Integer.valueOf(var5));
                if (var5 > var8) {
                    var8 = var5;
                }
            }

            var9.close();
        } catch (Exception var10) {
            System.out.println("Read error: " + var10.toString());
            var10.printStackTrace();
            System.exit(1);
        }

        return var8;
    }

    public static ARRAY2 convert_V(int var0) {
        ARRAY2 var1 = new ARRAY2();
        var1.size = var0;
        var1.ele = new ELEMENT[var1.size];

        for (int var2 = 0; var2 < var0; var1.ele[var2].vertex = var2++) {
            var1.ele[var2] = new ELEMENT();
        }

        return var1;
    }

    public static boolean[][] convert_e(ArrayList<HashSet<Integer>> var0) {
        int var1 = var0.size();
        boolean[][] var2 = new boolean[var1][var1];

        int var4;
        for (int var3 = 0; var3 < var1; ++var3) {
            for (Iterator var5 = ((HashSet) var0.get(var3)).iterator(); var5.hasNext(); var2[var3][var4] = true) {
                var4 = ((Integer) var5.next()).intValue();
            }
        }

        return var2;
    }

    private static long MCQ(ArrayList<HashSet<Integer>> var0, ARRAY2 var1, boolean[][] var2, int var3, TreeSet<ELEMENT> var4) {
        Q.size = 0;
        QMAX.size = 0;
        var1.size = var3;
        t1 = (new Date()).getTime();
        vSort(var1, var4, var0);
        int var6 = ((HashSet) var0.get(var1.ele[0].vertex)).size();

        int var5;
        for (var5 = 0; var5 < var6; ++var5) {
            var1.ele[var5].degree = var5 + 1;
        }

        for (var5 = var6; var5 < var1.size; ++var5) {
            var1.ele[var5].degree = var6 + 1;
        }

        EXPAND(var1, var2, var3);
        long var7 = (new Date()).getTime();
        return var7 - t1;
    }

    private static void vSort(ARRAY2 var0, TreeSet<ELEMENT> var1, ArrayList<HashSet<Integer>> var2) {
        if (var1 == null) {
            var1 = initVerts(var0, var2, new int[var2.size()]);
        }

        int var3 = 0;

        ELEMENT var4;
        for (Iterator var5 = var1.descendingIterator(); var5.hasNext(); var0.ele[var3++].vertex = var4.vertex) {
            var4 = (ELEMENT) var5.next();
        }

    }

    public static TreeSet<ELEMENT> initVerts(ARRAY2 var0, ArrayList<HashSet<Integer>> var1, int[] var2) {
        TreeSet<ELEMENT> var3 = new TreeSet<ELEMENT>(new Comparator<ELEMENT>() {
            public int compare(ELEMENT var1, ELEMENT var2) {
                return var1.vertex == var2.vertex ? 0 : (var1.degree >= var2.degree && (var1.degree != var2.degree || var1.vertex >= var2.vertex) ? 1 : -1);
            }
        });

        for (int var5 = 0; var5 < var0.size; ++var5) {
            ELEMENT var4 = new ELEMENT();
            var4.vertex = var0.ele[var5].vertex;
            var4.degree = ((HashSet) var1.get(var0.ele[var5].vertex)).size();
            var3.add(var4);
            var2[var4.vertex] = var4.degree;
        }

        return var3;
    }

    private static void EXPAND(ARRAY2 var0, boolean[][] var1, int var2) {
        for (ARRAY2 var4 = new ARRAY2(); var0.size != 0; --var0.size) {
            int var3 = var0.ele[var0.size - 1].vertex;
            if (Q.size + var0.ele[var0.size - 1].degree <= QMAX.size) {
                return;
            }

            Q.ele[Q.size++] = var3;
            var4.ele = new ELEMENT[var0.size];
            if (CUT2(var3, var0, var4, var1)) {
                COLOR_SORT(var4, var1, var2);
                EXPAND(var4, var1, var2);
            } else if (Q.size > QMAX.size) {
                COPY(QMAX, Q);
            }

            if (QMAX.size >= UPPER_BOUND) {
                return;
            }

            --Q.size;
        }

    }

    private static boolean CUT2(int var0, ARRAY2 var1, ARRAY2 var2, boolean[][] var3) {
        var2.size = 0;

        for (int var4 = 0; var4 < var1.size - 1; ++var4) {
            if (var3[var0][var1.ele[var4].vertex]) {
                var2.ele[var2.size] = new ELEMENT();
                var2.ele[var2.size].vertex = var1.ele[var4].vertex;
                ++var2.size;
            }
        }

        return var2.size != 0;
    }

    private static void COLOR_SORT(ARRAY2 var0, boolean[][] var1, int var2) {
        int var8 = 1;
        int var7 = QMAX.size - Q.size + 1;
        C[1] = new ARRAY_FIX(var2);
        C[2] = new ARRAY_FIX(var2);
        C[1].size = 0;
        C[2].size = 0;
        int var3 = 0;

        int var4;
        int var5;
        for (var4 = 0; var3 < var0.size; ++var3) {
            int var6 = var0.ele[var3].vertex;
            var5 = 1;

            while (true) {
                if (C[var5] == null) {
                    C[var5] = new ARRAY_FIX(var2);
                }

                if (!CUT1(var6, C[var5], var1)) {
                    if (var5 > var8) {
                        var8 = var5;
                        if (C[var5 + 1] == null) {
                            C[var5 + 1] = new ARRAY_FIX(var2);
                        }

                        C[var5 + 1].size = 0;
                    }

                    int[] var10000 = C[var5].ele;
                    ARRAY_FIX var10001 = C[var5];
                    ARRAY_FIX var10002 = C[var5];
                    int var9 = var10001.size;
                    var10002.size = C[var5].size + 1;
                    var10000[var9] = var0.ele[var3].vertex;
                    if (var5 < var7) {
                        var0.ele[var4++].vertex = var0.ele[var3].vertex;
                    }
                    break;
                }

                ++var5;
            }
        }

        if (var7 <= 0) {
            var7 = 1;
        }

        for (var5 = var7; var5 <= var8; ++var5) {
            for (var3 = 0; var3 < C[var5].size; ++var3) {
                var0.ele[var4].vertex = C[var5].ele[var3];
                var0.ele[var4++].degree = var5;
            }
        }

    }

    private static boolean CUT1(int var0, ARRAY_FIX var1, boolean[][] var2) {
        int var3;
        for (var3 = 0; var3 < var1.size && !var2[var0][var1.ele[var3]]; ++var3) {
            ;
        }

        return var3 != var1.size;
    }

    private static void COPY(ARRAY_FIX var0, ARRAY_FIX var1) {
        for (int var2 = 0; var2 < var1.size; ++var2) {
            var0.ele[var2] = var1.ele[var2];
        }

        var0.size = var1.size;
    }
}
