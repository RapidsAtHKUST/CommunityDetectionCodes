//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Map.Entry;

public class NODE {
    public long ID;
    public String name;
    public int numNbs;
    public ArrayList<Long> nbList;
    public HashSet<Long> nbSet;
    public ArrayList<Long> WQueue;
    public Hashtable<Long, Integer> WQHistgram;
    public ArrayList<Entry<Long, Integer>> WQHistMapEntryList;
    public Hashtable<Long, Double> foundcomTable;
    public Hashtable<Long, Double> truecomTable;
    public int numNbs_Out;
    public int numNbs_In;
    public ArrayList<Long> nbList_Out = new ArrayList();
    public HashSet<Long> nbSet_Out = new HashSet();
    public ArrayList<Long> nbList_In = new ArrayList();
    public HashSet<Long> nbSet_In = new HashSet();
    public ArrayList<Double> nbweightList_In = new ArrayList();
    public ArrayList<Double> nbweightList_In_norm = new ArrayList();
    public Hashtable<Long, Integer> WQHistgram_r = new Hashtable();
    public int status = 0;
    public int t = 0;
    public static final Comparator<NODE> ComNodeIDInc = new Comparator<NODE>() {
        public int compare(NODE v1, NODE v2) {
            Long i1 = Long.valueOf(v1.ID);
            Long i2 = Long.valueOf(v2.ID);
            return i1.compareTo(i2);
        }
    };

    public NODE(Long ID) {
        this.ID = ID.longValue();
        this.nbList = new ArrayList();
        this.nbSet = new HashSet();
        this.WQueue = new ArrayList();
        this.WQHistMapEntryList = new ArrayList();
        this.foundcomTable = new Hashtable();
        this.truecomTable = new Hashtable();
    }

    public static void main(String[] args) {
    }
}
