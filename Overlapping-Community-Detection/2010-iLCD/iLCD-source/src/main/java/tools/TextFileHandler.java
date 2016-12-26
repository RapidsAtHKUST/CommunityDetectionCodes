package tools;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Vector;

public class TextFileHandler {

	//load a file where each line contains 2 values, the first one will be a key and the other one the value.
	//you can specifie the separator you want.
	//if reverse = true, the order is "definition - id"
	public static HashMap<String,String> loadDictionary(String fileName,String separator,boolean reverse){
		HashMap<String,String> dico = new HashMap<String, String>();
		String line="";
		BufferedReader br = null;
		
		try {
			 br = new BufferedReader(new InputStreamReader(new FileInputStream(fileName)));
		} catch (Exception e) {
			System.out.println("file doesn't exist "+ fileName);
			e.printStackTrace();
		}
		
		try{
			while ((line=br.readLine())!=null){
				
				if(line.contains(separator)){
					String word = line.substring(0, line.lastIndexOf(separator));
					String definition = line.substring(line.lastIndexOf(separator)+1);
					if(reverse){
						String temp = word;
						word = definition;
						definition = temp;
					}
					dico.put(word, definition);
				}
			}
			
		} catch (Exception e) {
			System.out.println("problem with line "+ line);
			e.printStackTrace();
		}
		return dico;
	}
	
	public static String readFirstLine(String fileName){
		BufferedReader br = null;
	
		try {
			 br = new BufferedReader(new InputStreamReader(new FileInputStream(fileName)));
		} catch (Exception e) {
			System.out.println("file doesn't exist "+ fileName);
			e.printStackTrace();
		}
		String firstLine = null;
		try {
			firstLine = br.readLine();

		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("problem with first line");
		}
		return firstLine;
		
	}
	//read a file and return a list of string 
	public static ArrayList<String> loadFile(String fileName){
		BufferedReader br = null;
		ArrayList<String> lines = new ArrayList<String>();
		try {
			 br = new BufferedReader(new InputStreamReader(new FileInputStream(fileName)));
		} catch (Exception e) {
			System.out.println("file doesn't exist "+ fileName);
			e.printStackTrace();
		}
		String line="";

			try {
				while ((line=br.readLine())!=null){
					lines.add(line);
				}
			} catch (IOException e) {
				System.out.println("problem with line "+ line);
				e.printStackTrace();
			}
			return lines;
				
	}
	
	public static void printFile(String fileName,ArrayList<String> lines){
		PrintWriter fileout=null;
			try {
				fileout = new PrintWriter(new BufferedWriter (new FileWriter(fileName)));
			} catch (IOException e) {
				System.out.println("problem to write in :  "+ fileName);
				e.printStackTrace();
			}
			
			int count = 0;
			for(String line : lines){
				count++;
				fileout.write(line);
				if(count<lines.size())
					fileout.write("\n");
			}
			fileout.close();
	}
	
	//split a single line into several parts, according to the given separator
	public static Vector<String> splitLine(String line, String separator){
		String[] s = line.split(separator);
		Vector<String> result = new Vector<String>() ;
		for(String el : s)
			result.add(el);
		return result;
	}
	
	public static void main(String args[]){
		String dic = "/Users/quetzalcroak/Desktop/MARAMI_2010/MARAMI/expe/graphs/ds/ds_v_ND_NR_v2.ncol.lemme";
		String verbs = "/Users/quetzalcroak/Desktop/testTrucaDRoite/CompareCommunities/previousNetwork/verbs.ncol/verbs.ncol";
		HashMap<String,String> dico = loadDictionary(dic," ",true);
		ArrayList<String> edges = loadFile(verbs);
		ArrayList<String> toPrint = new ArrayList<String>();
		
		for(String line : edges){
			Vector<String> els = splitLine(line, " ");
			String resultLine = "";
			int countEls = 0;
			for(String el : els){

				countEls++;
				String def = dico.get(el);
				if(def ==null)
					System.err.println("error : don't find -"+el+"- in dictionary");
				String temp = dico.get(el);
				temp = temp.replace(' ', '_');
				System.out.println(temp);

				resultLine += temp;
				if(countEls<els.size())
					resultLine+="	";
			}
			//System.out.println(resultLine);

			toPrint.add(resultLine);
		}
		
		printFile("/Users/quetzalcroak/Desktop/testTrucaDRoite/CompareCommunities/verbs.ncol",toPrint);
	}
	
	public static String cleanNodeName(String nodeName){
		//nodeName = nodeName.replace('	', '–');
		nodeName = nodeName.replace(' ', '_');
		nodeName = nodeName.replace('"', '_');
		return nodeName;
	}
}
