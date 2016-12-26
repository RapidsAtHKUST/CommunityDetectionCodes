package Outputs;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.MyDate;
import TemporalNetworkManipulation.MyDateManager;
import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Node;



/**display all actions on communities in ctnf format.
 * The ctnf format is defined in my PhD Thesis.
 * easy to read : 
 * +c : new community
 * -c : death of a community
 * +nc : adding a node to a community
 * -nc : removing a node from a community
 * = merging two communities
 * Complex operations, such as fusion of communities with nodes being absorbed, are represented as
 * several simpler operations accuring at the same time (represented as #time)
 *
 * @author remy cazabet
 *
 */
public class OutputCTNF implements OutputHandlerInterface{

	private String separator = "	" ; //tab
	PrintWriter fileout=null;
	Network theNetwork;
	
	private MyDateManager dateManager;
	
	private MyDate lastPrintedDate = MyDate.beginningOfTheUniverse();
	
	public OutputCTNF(MyDateManager dateManager, Network theNetwork) {
		this.dateManager = dateManager;
		this.theNetwork = theNetwork;
	}

	public void initialise(String outputFile){
		
		try {
			fileout = new PrintWriter(new BufferedWriter (new FileWriter(outputFile+".ctnf")));
		} catch (IOException e) {
			System.out.println("problem to write in :  "+ outputFile);
			e.printStackTrace();
		}
	}
	
	public void terminate(){
		fileout.close();
	}
	
	public void printLine(String line){
		if(theNetwork.getCurrentTime().after(lastPrintedDate)){
			lastPrintedDate=theNetwork.getCurrentTime();
			fileout.println("#"+dateManager.writeDate(lastPrintedDate));
		}
		fileout.println(line);
	}
	
	
	
	
	public void handleGrowth(Node n, Community c){
		this.printLine("+nc"+separator+n.getName()+separator+c.getID());
	}
	
	public void handleGrowth(Community initialCommunity, Community resultingCommunity){
		ArrayList<Node> addedNodes = new ArrayList<Node>(resultingCommunity.getComponents());
		addedNodes.removeAll(initialCommunity.getComponents());
		for(Node n : addedNodes){
			this.handleGrowth(n, resultingCommunity);
		}
		
	}

	public void handleBirth(Community c) {
		this.printLine("+c"+separator+c.getID());
		for(Node n : c.getComponents())
			this.handleGrowth(n, c);
	}

	public void handleDeath(Community c) {
		for(Node n : c.getComponents())
			this.handleContraction(n, c);
		
		this.printLine("-c"+separator+c.getID());		
	}

	/*public void handleContraction(Community tempSavedCom, Community community) {
		ArrayList<Node> removedNodes = new ArrayList<Node>(tempSavedCom.getComponents());
		removedNodes.removeAll(community.getComponents());
		for(Node n : removedNodes){
			this.handleContraction(n, community);
		}
		
	}*/
	
	public void handleContraction(Node n, Community c) {
		this.printLine("-nc"+separator+n.getName()+separator+c.getID());		
	}
	
	public void handleNewEdge(Node n, Node n2) {
		this.printLine("+"+separator+n.getName()+separator+n2.getName());		
	}
	
	public void handleRemoveEdge(Node n, Node n2) {
		this.printLine("-"+separator+n.getName()+separator+n2.getName());		
	}

	public void handleFusion(Community resultingCommunity, Community suppressedCommunity) {
		this.printLine("="+separator+resultingCommunity.getID()+separator+suppressedCommunity.getID());
		//this.handleDeath(oldVersionKeepedCom);
		//this.handleGrowth(oldVersionKeepedCom, resultingCommunity);
	}
	public void handleDateChange(String date) {
		this.printLine(date);
		
	}
}
