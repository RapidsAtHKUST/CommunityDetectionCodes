package tools;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;

public class DebugMode {

	public static boolean printDebugDialogs = false;
	public static boolean printBasicDialogs = false;
	public static void printDebug(String s){
		if(printDebugDialogs){
			PrintStream out;
			try {
				out = new PrintStream(System.out, true, "UTF-8");
			    out.println(s);

			} catch (UnsupportedEncodingException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
			//System.out.println(s);
	}
	
	public static void printBasic(String s){
		if(printBasicDialogs){
			PrintStream out;
			try {
				out = new PrintStream(System.out, true, "UTF-8");
			    out.println(s);

			} catch (UnsupportedEncodingException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
			//System.out.println(s);
	}

}
