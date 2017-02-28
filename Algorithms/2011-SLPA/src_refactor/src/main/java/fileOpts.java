//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Iterator;

public class fileOpts {
    public fileOpts() {
    }

    public static String extractFileName(String fullName) {
        int endIndex = fullName.lastIndexOf(".");
        String name;
        if(endIndex == -1) {
            name = fullName;
        } else {
            name = fullName.substring(0, endIndex);
        }

        return name;
    }

    public static String extractFileName_FullPath(String fullName) {
        int endIndex = fullName.lastIndexOf(".");
        int startIndx = fullName.lastIndexOf("/");
        if(startIndx == -1) {
            startIndx = fullName.lastIndexOf("\\");
            if(startIndx == -1) {
                startIndx = 0;
            } else {
                ++startIndx;
            }
        } else {
            ++startIndx;
        }

        String name;
        if(endIndex == -1) {
            name = fullName;
        } else {
            name = fullName.substring(startIndx, endIndex);
        }

        return name;
    }

    public static ArrayList<String> readFromTxt(String filename) {
        ArrayList contents = new ArrayList();
        File file = new File(filename);
        BufferedReader reader = null;

        try {
            reader = new BufferedReader(new FileReader(file));
            String e = null;

            while((e = reader.readLine()) != null) {
                contents.add(e);
            }
        } catch (FileNotFoundException var15) {
            var15.printStackTrace();
        } catch (IOException var16) {
            var16.printStackTrace();
        } finally {
            try {
                if(reader != null) {
                    reader.close();
                }
            } catch (IOException var14) {
                var14.printStackTrace();
            }

        }

        System.out.println(file.getAbsolutePath());
        return contents;
    }

    public static void writeToTxt(String filename, boolean append, ArrayList<String> data) {
        BufferedWriter bufferedWriter = null;

        try {
            bufferedWriter = new BufferedWriter(new FileWriter(filename, append));
            Iterator ex = data.iterator();

            while(ex.hasNext()) {
                String oneLine = (String)ex.next();
                bufferedWriter.write(oneLine);
                bufferedWriter.newLine();
            }
        } catch (FileNotFoundException var16) {
            var16.printStackTrace();
        } catch (IOException var17) {
            var17.printStackTrace();
        } finally {
            try {
                if(bufferedWriter != null) {
                    bufferedWriter.flush();
                    bufferedWriter.close();
                }
            } catch (IOException var15) {
                var15.printStackTrace();
            }

        }

    }

    public static boolean isFileExist(String name) {
        File f = new File(name);
        return f.exists();
    }

    public static void createDirectory(String strDirectoy) {
        if(!isFileExist(strDirectoy)) {
            boolean success = (new File(strDirectoy)).mkdir();
            if(success) {
                System.out.println("Directory: " + strDirectoy + " created");
            }
        }

    }

    public static void createEmptyTxt(String filename) {
        boolean append = false;
        ArrayList data = new ArrayList();
        writeToTxt(filename, append, data);
    }

    public static void writeObj2File(Object oriObj, String fileName) {
        try {
            ObjectOutputStream output = new ObjectOutputStream(new FileOutputStream(fileName));
            output.writeObject(oriObj);
            output.flush();
            output.close();
        } catch (IOException var4) {
            var4.printStackTrace();
        }

    }

    public static Object readObjfromFile(String fileName) {
        Object copyObj = null;

        try {
            ObjectInputStream input = new ObjectInputStream(new FileInputStream(fileName));
            copyObj = input.readObject();
            input.close();
        } catch (IOException var4) {
            var4.printStackTrace();
        } catch (ClassNotFoundException var5) {
            var5.printStackTrace();
        }

        return copyObj;
    }

    public static byte[] writeObj2ByteArray(Object oriObj) {
        ByteArrayOutputStream bos = null;

        try {
            bos = new ByteArrayOutputStream();
            ObjectOutputStream output = new ObjectOutputStream(bos);
            output.writeObject(oriObj);
            output.flush();
            output.close();
        } catch (IOException var5) {
            var5.printStackTrace();
        }

        return bos.toByteArray();
    }

    public static Object readObjfromwByteArray(byte[] buf) {
        Object copyObj = null;

        try {
            ObjectInputStream input = new ObjectInputStream(new ByteArrayInputStream(buf));
            copyObj = input.readObject();
            input.close();
        } catch (IOException var4) {
            var4.printStackTrace();
        } catch (ClassNotFoundException var5) {
            var5.printStackTrace();
        }

        return copyObj;
    }

    public static void main(String[] args) {
    }
}
