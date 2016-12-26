package tools;

import java.util.ArrayList;
import java.util.HashMap;

import tools.Sorter.Element;

public class SortEdges {
	static HashMap<String,Integer> degreeNodes = new HashMap<String, Integer>();
	
	
	public static void sort(String input, String output){
		
		ArrayList<String> lines = TextFileHandler.loadFile(input);

		
		Sorter<String> mySorter = new Sorter<String>();
	
		for(String s : lines){
			String source = TextFileHandler.splitLine(s, "	").firstElement();
			String destination = TextFileHandler.splitLine(s, "	").lastElement();
			if(!degreeNodes.containsKey(source))
				degreeNodes.put(source,0);
			if(!degreeNodes.containsKey(destination))
				degreeNodes.put(destination,0);
			
			degreeNodes.put(source, degreeNodes.get(source)+1);
			degreeNodes.put(destination, degreeNodes.get(destination)+1);
	
			mySorter.add(s, 0d);
		}
		
		for(String l : mySorter.getElts()){
			String source = l.split("	")[0];
			String destination = l.split("	")[1];
			
			mySorter.modVal(l, (double)degreeNodes.get(source)+degreeNodes.get(destination));
			
		}
		
		
		ArrayList<String> toPrint = mySorter.getSortedValues();
		
		TextFileHandler.printFile(output, toPrint);
	}
	
	public static void main(String args[]){
		String in = "/Users/quetzalcroak/Desktop/testTrucaDRoite/CompareCommunities/previousNetwork/verbsFR.ncol/verbsFR.ncol";
		String out = "/Users/quetzalcroak/Desktop/testTrucaDRoite/CompareCommunities/previousNetwork/verbsFR.ncol/verbsFRsorted.ncol";
		sort(in,out);
		
	}
}
