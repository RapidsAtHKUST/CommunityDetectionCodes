package clique_modularity.util;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.HashSet;

public class Tree {
    public boolean isLeaf;
    public HashSet<Integer> set;
    public Tree left;
    public Tree right;

    public Tree(HashSet<Integer> var1) {
        this.isLeaf = true;
        this.set = var1;
    }

    public Tree(Tree var1, Tree var2) {
        this.isLeaf = false;
        this.left = var1;
        this.right = var2;
    }

    public void add(Tree var1) {
        if (this.isLeaf) {
            this.left = new Tree(this.set);
        } else {
            this.left = new Tree(this.left, this.right);
        }

        this.right = var1;
        this.isLeaf = false;
    }
}
