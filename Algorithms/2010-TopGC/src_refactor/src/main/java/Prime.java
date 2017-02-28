//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class Prime {
    public Prime() {
    }

    boolean isPrime(int n) {
        if(n <= 2) {
            return n == 2;
        } else if(n % 2 == 0) {
            return false;
        } else {
            int i = 3;

            for(int end = (int)Math.sqrt((double)n); i <= end; i += 2) {
                if(n % i == 0) {
                    return false;
                }
            }

            return true;
        }
    }

    int getPrime(int n) {
        while(!this.isPrime(n)) {
            ++n;
        }

        return n;
    }
}
