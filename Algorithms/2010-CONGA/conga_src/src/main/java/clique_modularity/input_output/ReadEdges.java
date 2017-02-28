package clique_modularity.input_output;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Scanner;

public class ReadEdges {
    public ReadEdges() {
    }

    public ArrayList<HashSet<Integer>> readGraph(String var1, int var2) throws FileNotFoundException {
        ArrayList var6 = new ArrayList();

        for (int var3 = 0; var3 < var2; ++var3) {
            var6.add(new HashSet());
        }

        File var7 = new File(var1);
        if (var7 == null) {
            throw new IllegalArgumentException("File should not be null.");
        } else if (!var7.exists()) {
            throw new IllegalArgumentException("File does not exist: " + var7);
        } else {
            Scanner var8 = new Scanner(var7);

            Scanner var10;
            try {
                for (; var8.hasNextLine(); var10.close()) {
                    String var9 = var8.nextLine();
                    var10 = new Scanner(var9);
                    if (var10.hasNext()) {
                        int var11 = Integer.parseInt(var10.next());
                        int var12 = Integer.parseInt(var10.next());
                        ((HashSet) var6.get(var11)).add(Integer.valueOf(var12));
                        ((HashSet) var6.get(var12)).add(Integer.valueOf(var11));
                    }
                }
            } finally {
                var8.close();
            }

            return var6;
        }
    }
}
