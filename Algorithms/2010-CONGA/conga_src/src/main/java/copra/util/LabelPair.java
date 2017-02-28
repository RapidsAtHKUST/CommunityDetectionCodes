package copra.util;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class LabelPair {
    public float value;
    public int pos;

    public LabelPair(float var1, int var2) {
        this.value = var1;
        this.pos = var2;
    }

    public String toString() {
        return this.pos + "/" + this.value;
    }
}
