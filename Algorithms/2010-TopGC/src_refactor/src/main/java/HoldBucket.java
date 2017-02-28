//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.Enumeration;
import java.util.Hashtable;

public class HoldBucket implements Comparable<HoldBucket> {
    double score;
    Hashtable<Integer, Integer> bin;

    public HoldBucket(double score, Hashtable<Integer, Integer> bin) {
        this.score = score;
        this.bin = bin;
    }

    public int compareTo(HoldBucket other) {
        if(this.score > other.score) {
            return 1;
        } else if(this.score < other.score) {
            return -1;
        } else if(this.bin.size() > other.bin.size()) {
            return 1;
        } else if(this.bin.size() < other.bin.size()) {
            return -1;
        } else {
            int thisCount = 0;
            int otherCount = 0;

            Enumeration keys;
            for(keys = this.bin.keys(); keys.hasMoreElements(); thisCount += ((Integer)this.bin.get(keys.nextElement())).intValue()) {
                ;
            }

            for(keys = other.bin.keys(); keys.hasMoreElements(); otherCount += ((Integer)other.bin.get(keys.nextElement())).intValue()) {
                ;
            }

            return thisCount > otherCount?1:(thisCount < otherCount?-1:0);
        }
    }
}
