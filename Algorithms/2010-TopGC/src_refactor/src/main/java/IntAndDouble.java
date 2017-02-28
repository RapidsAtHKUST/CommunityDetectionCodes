//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class IntAndDouble implements Comparable<IntAndDouble> {
    int node;
    double weight;

    public IntAndDouble(int i, double d) {
        this.node = i;
        this.weight = d;
    }

    public int compareTo(IntAndDouble other) {
        return other.weight == this.weight?0:(other.weight > this.weight?1:-1);
    }

    public String toString() {
        return String.valueOf(this.weight);
    }
}
