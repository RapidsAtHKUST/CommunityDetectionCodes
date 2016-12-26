package Outputs;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.MyDate;
import TemporalNetworkManipulation.MyDateManager;
import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Node;



/**this class is to print static communities. It prints only the communities alive at the end of the process.
 * the communities are printed in a way allowing overlap : each line correspond to a community. 
 * Each line contains all the nodes of the community. Standard format as used by several static algorithms
 *
 * @author remy cazabet
 *
 */
public class OutputStatic implements OutputHandlerInterface{

	
	private String separator = "	" ; //tab
	PrintWriter fileout=null;
	Network theNetwork;
	
	
	
	private boolean printFusionnedCommunities =false;
	
	public OutputStatic(MyDateManager dateManager, Network theNetwork) {
		this.theNetwork = theNetwork;
	}

	public void initialise(String outputFile){
		
		try {
			fileout = new PrintWriter(new BufferedWriter (new FileWriter(outputFile+".com")));
		} catch (IOException e) {
			System.out.println("problem to write in :  "+ outputFile);
			e.printStackTrace();
		}
	}
	
	public void terminate(){
		
		for(Community c : theNetwork.getCommunities()){
			String lineCom = "";
			int i = 1;
			for(Node n : c.getComponents()){
				lineCom+=n.getName();
				//if it's not the last element, add a separator
				if(i<c.getComponents().size()){
					lineCom+=separator;
				}
				i++;
			}
			
			this.printLine(lineCom);
		}
		fileout.close();
	}
	
	public void printLine(String line){

		fileout.println(line);
	}
	
	
	
	
	public void handleGrowth(Node n, Community c){
		
	}
	
	public void handleGrowth(Community initialCommunity, Community resultingCommunity){
		
	}

	public void handleBirth(Community c) {
		

		
	}

	public void handleDeath(Community c) {
		
	}

	/*public void handleContraction(Community tempSavedCom, Community community) {
		ArrayList<Node> removedNodes = new ArrayList<Node>(tempSavedCom.getComponents());
		removedNodes.removeAll(community.getComponents());
		for(Node n : removedNodes){
			this.handleContraction(n, community);
		}
		
	}*/
	
	public void handleContraction(Node n, Community c) {
		//this.printLine("-nc"+separator+n.getName()+separator+c.getID());		
	}
	
	public void handleNewEdge(Node n, Node n2) {
		//this.printLine("+"+separator+n.getName()+separator+n2.getName());		
	}
	
	public void handleRemoveEdge(Node n, Node n2) {
		//this.printLine("-"+separator+n.getName()+separator+n2.getName());		
	}

	public void handleFusion(Community resultingCommunity, Community suppressedCommunity) {
		
		
	}
	public void handleDateChange(String date) {
		
	}
	
}
