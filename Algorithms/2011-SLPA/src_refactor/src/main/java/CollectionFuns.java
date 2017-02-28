//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;

public class CollectionFuns {
    public CollectionFuns() {
    }

    public static double roundTwoDecimals(double d) {
        DecimalFormat twoDForm = new DecimalFormat("#.##");
        return Double.valueOf(twoDForm.format(d)).doubleValue();
    }

    public static String Ascii2Str(int i) {
        return Character.toString((char)i);
    }

    public static String sortString(String str) {
        char[] content = str.toCharArray();
        Arrays.sort(content);
        String sorted = new String(content);
        return sorted;
    }

    public static String intArrayToString(int[] narray) {
        StringBuffer result = new StringBuffer();

        for(int i = 0; i < narray.length; ++i) {
            result.append(narray[i]);
        }

        return result.toString();
    }

    public static int[] String2intArray(String str) {
        int[] result = new int[str.length()];

        for(int i = 0; i < str.length(); ++i) {
            result[i] = str.charAt(i) - 49 + 1;
        }

        return result;
    }

    public static int[] int2intArray(Integer aInt) {
        String s = aInt.toString();
        return String2intArray(s);
    }

    public static int intArray2int(int[] intarray) {
        String s = intArrayToString(intarray);
        return Integer.parseInt(s);
    }

    public void printAHashtablebyKey(Hashtable ht) {
        ArrayList keys = new ArrayList(ht.keySet());
        Collections.sort(keys);
        Iterator it = keys.iterator();

        while(it.hasNext()) {
            System.out.println("Key=" + it.next());
        }

    }

    public static void doGC(int n) {
        Runtime rt = Runtime.getRuntime();

        for(int i = 1; i <= n; ++i) {
            rt.gc();
        }

    }

    public static long getSizeofUsedHeap() {
        Runtime rt = Runtime.getRuntime();
        return rt.totalMemory() - rt.freeMemory();
    }

    public static long getSizeofAvailableHeap() {
        Runtime rt = Runtime.getRuntime();
        return rt.maxMemory() - rt.totalMemory() + rt.freeMemory();
    }

    public static void getTotalHeapSize() {
        Runtime rt = Runtime.getRuntime();
        System.out.println("total heap:" + rt.totalMemory() / 1048576L + "MB");
    }

    public static void addArray2ToArray1_Double(ArrayList<Double> al1, ArrayList<Double> al2) {
        for(int i = 0; i < al1.size(); ++i) {
            al1.set(i, Double.valueOf(((Double)al1.get(i)).doubleValue() + ((Double)al2.get(i)).doubleValue()));
        }

    }

    public static void AvgArray_Double(ArrayList<Double> al, Double times) {
        for(int i = 0; i < al.size(); ++i) {
            al.set(i, Double.valueOf(((Double)al.get(i)).doubleValue() / times.doubleValue()));
        }

    }

    public static double sumArray_Double(ArrayList<Double> al) {
        double sum = 0.0D;

        for(int i = 0; i < al.size(); ++i) {
            sum += ((Double)al.get(i)).doubleValue();
        }

        return sum;
    }

    public static void ExpandArrayByLastE_Double(ArrayList<Double> al, Double explen) {
        for(int i = al.size(); (double)i < explen.doubleValue(); ++i) {
            al.add((Double)al.get(al.size() - 1));
        }

    }

    public static void FindMaxBetween2Array_Double(ArrayList<Double> al1, ArrayList<Double> al2) {
        for(int i = 0; i < al1.size(); ++i) {
            if(((Double)al1.get(i)).doubleValue() < ((Double)al2.get(i)).doubleValue()) {
                al1.set(i, (Double)al2.get(i));
            }
        }

    }

    public static void FindMinBetween2Array_Double(ArrayList<Double> al1, ArrayList<Double> al2) {
        for(int i = 0; i < al1.size(); ++i) {
            if(((Double)al1.get(i)).doubleValue() > ((Double)al2.get(i)).doubleValue()) {
                al1.set(i, (Double)al2.get(i));
            }
        }

    }

    public static Set unionSet(Set s1, Set s2) {
        HashSet set = new HashSet();
        set.addAll(s1);
        set.addAll(s2);
        return set;
    }

    public static Set interSet(Set s1, Set s2) {
        HashSet set = new HashSet();
        set.addAll(s1);
        set.retainAll(s2);
        return set;
    }

    public static Set diffSet(Set s1, Set s2) {
        HashSet set = new HashSet();
        set.addAll(s1);
        set.removeAll(s2);
        return set;
    }

    public static double round2nDecimal(double v, double p) {
        int precision = (int)Math.pow(10.0D, p);
        v = Math.floor(v * (double)precision + 0.5D) / (double)precision;
        return v;
    }

    public static void makeListUnique(ArrayList AList) {
        HashSet set = new HashSet();

        for(int i = 0; i < AList.size(); ++i) {
            set.add(AList.get(i));
        }

        AList.clear();
        AList.addAll(set);
    }

    public static Long Lookup_Hashtable_getputID_Str_Long(Hashtable<String, Long> ht, String key) {
        if(!ht.containsKey(key)) {
            ht.put(key, Long.valueOf((long)ht.size() - 0L));
        }

        return (Long)ht.get(key);
    }

    public static void main(String[] args) {
    }
}
