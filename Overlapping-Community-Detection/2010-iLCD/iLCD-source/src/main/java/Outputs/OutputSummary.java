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



/**this class is to display only a summary of the communities.
 * display final communities and communities who died without being merged (we can also display communities merged
 * with a parameter)
 * Time of the communities is displayed
 *
 * @author remy cazabet
 *
 */
public class OutputSummary implements OutputHandlerInterface{

	
	private String separator = "	" ; //tab
	PrintWriter fileout=null;
	Network theNetwork;
	
	private MyDateManager dateManager;
	//private MyDate lastPrintedDate = MyDate.beginningOfTheUniverse();
	
	private HashMap<Integer,CommunityToPrint> communitiesToPrint = new HashMap<Integer, CommunityToPrint>();
	
	
	private boolean printFusionnedCommunities =false;
	
	public OutputSummary(MyDateManager dateManager, Network theNetwork) {
		this.dateManager = dateManager;
		this.theNetwork = theNetwork;
	}

	public void initialise(String outputFile){
		
		try {
			fileout = new PrintWriter(new BufferedWriter (new FileWriter(outputFile+".tcom")));
		} catch (IOException e) {
			System.out.println("problem to write in :  "+ outputFile);
			e.printStackTrace();
		}
	}
	
	public void terminate(){
		ArrayList<CommunityToPrint> ctps = new ArrayList<OutputSummary.CommunityToPrint>(communitiesToPrint.values());
		Collections.sort(ctps);
		
		//first, print all communities which are still alive
		for(CommunityToPrint ctp: ctps){
			if(ctp.deathDate==null){
				printACommunity(ctp);
			}
		}
		
		this.printLine("#dead communities:");
		
		for(CommunityToPrint ctp: ctps){
			if(ctp.deathDate!=null){
				printACommunity(ctp);
			}
		}
		fileout.close();
	}
	
	private void printACommunity(CommunityToPrint ctp){
		String deathDate = "-";
		if(ctp.deathDate!=null){
			deathDate = dateManager.writeDate(ctp.deathDate);
		}
		
		if(ctp.fusion.equals("") || this.printFusionnedCommunities){
			String lineCtps = ctp.id+":"+separator+dateManager.writeDate(ctp.birthDate)+":"+deathDate+separator;
			for(Node n : ctp.nodes){
				lineCtps+=n.getName()+separator;
			}
			lineCtps+=ctp.fusion;
			this.printLine(lineCtps);
		}
	}
	public void printLine(String line){
/*		if(theNetwork.getCurrentTime().after(lastPrintedDate)){
			lastPrintedDate=theNetwork.getCurrentTime();
			fileout.println("#"+dateManager.writeDate(lastPrintedDate));
		}*/
		fileout.println(line);
	}
	
	
	
	
	public void handleGrowth(Node n, Community c){
		CommunityToPrint ctp = communitiesToPrint.get(c.getID());
		if(!ctp.nodes.contains(n)){
			ctp.nodes.add(n);
		}
	}
	
	public void handleGrowth(Community initialCommunity, Community resultingCommunity){
		ArrayList<Node> addedNodes = new ArrayList<Node>(resultingCommunity.getComponents());
		addedNodes.removeAll(initialCommunity.getComponents());
		for(Node n : addedNodes){
			this.handleGrowth(n, resultingCommunity);
		}
		
	}

	public void handleBirth(Community c) {
		CommunityToPrint ctp = new CommunityToPrint();
		ctp.id=String.valueOf(c.getID());
		ctp.birthDate = new MyDate(theNetwork.getCurrentTime());
		
		this.communitiesToPrint.put(c.getID(), ctp);
		
		
		for(Node n : c.getComponents())
			this.handleGrowth(n, c);
		

		
	}

	public void handleDeath(Community c) {
		CommunityToPrint ctp = communitiesToPrint.get(c.getID());

		ctp.deathDate = new MyDate(theNetwork.getCurrentTime());
		
		//do not print community which are born and merged immediately
		if(ctp.deathDate.sameDateAs(ctp.birthDate))
			this.communitiesToPrint.remove(Integer.parseInt(ctp.id));
		
		
		for(Node n : c.getComponents())
			this.handleContraction(n, c);	
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
		if(communitiesToPrint.containsKey(suppressedCommunity.getID())){
			CommunityToPrint ctp = communitiesToPrint.get(suppressedCommunity.getID());
			ctp.fusion = " -> "+resultingCommunity.getID();
		}
		
	}
	public void handleDateChange(String date) {
		//this.printLine(date);
		
	}
	
	private class CommunityToPrint implements Comparable<CommunityToPrint>{
		MyDate birthDate = null ;
		MyDate deathDate= null;
		String id;
		String fusion ="";
		ArrayList<Node> nodes = new ArrayList<Node>();
		@Override
		public int compareTo(CommunityToPrint arg0) {
			return birthDate.compareTo(arg0.birthDate);
		}
		
		
	}
}
