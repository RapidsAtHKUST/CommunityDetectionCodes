package conga.util;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class Tree {
    public int label;
    public Tree parent;
    public Tree left;
    public Tree right;

    public Tree(int var1) {
        this.label = var1;
        this.parent = null;
        this.left = null;
        this.right = null;
    }

    public Tree(Tree var1) {
        this.label = var1.label;
        this.parent = var1;
        this.left = var1.left;
        this.right = var1.right;
    }
}
