package clique_modularity;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

public class WriteCliques {
    public WriteCliques() {
    }

    public static void saveGraphfile(ArrayList<HashSet<Integer>> var0, String var1) {
        int var5 = var0.size();

        try {
            PrintStream var6 = new PrintStream(new FileOutputStream(var1));

            for(int var3 = 0; var3 < var5; ++var3) {
                Iterator var2 = ((HashSet)var0.get(var3)).iterator();

                while(var2.hasNext()) {
                    int var4 = ((Integer)var2.next()).intValue();
                    var6.print(var4 + " ");
                }

                var6.println();
            }
        } catch (Exception var7) {
            System.out.println("Error " + var7.toString());
            var7.printStackTrace();
            System.exit(1);
        }

    }
}
