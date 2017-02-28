//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

class TempDouble2 implements Comparable<TempDouble2> {
    public double weight;
    public int nodeNum;
    public boolean allSame;

    public TempDouble2(int n, double w) {
        this.weight = w;
        this.nodeNum = n;
    }

    public int compareTo(TempDouble2 other) {
        return this.weight > other.weight?1:(this.weight == other.weight?0:-1);
    }
}
