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
import TemporalNetworkManipulation.Node;




public class OutputMultipleHandler implements OutputHandlerInterface{

	
	private String separator = "	" ; //tab
	PrintWriter fileout=null;
	//Network theNetwork;
	
	private MyDate lastPrintedDate = MyDate.beginningOfTheUniverse();
	
	private ArrayList<OutputHandlerInterface> outputsClasses = new ArrayList<OutputHandlerInterface>();
	
	public OutputMultipleHandler() {
	}

	public void addOutputHandler(OutputHandlerInterface oh){
		this.outputsClasses.add(oh);
	}
	public void initialise(String outputFile){
		for(OutputHandlerInterface oh : outputsClasses){
			oh.initialise(outputFile);
		}
	}
	
	public void terminate(){
		for(OutputHandlerInterface oh : outputsClasses){
			oh.terminate();
		}
	}
	
	public void handleGrowth(Node n, Community c){
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleGrowth(n,c);
		}
	}
	
	public void handleGrowth(Community initialCommunity, Community resultingCommunity){
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleGrowth(initialCommunity, resultingCommunity);
		}
		
	}

	public void handleBirth(Community c) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleBirth(c);
		}

		
	}

	public void handleDeath(Community c) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleDeath(c);
		}
	}

	/*public void handleContraction(Community tempSavedCom, Community community) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleContraction(tempSavedCom, community);
		}
		
	}*/
	
	public void handleContraction(Node n, Community c) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleContraction(n,c);
		}	}
	
	public void handleNewEdge(Node n, Node n2) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleNewEdge(n,n2);
		}	}
	
	public void handleRemoveEdge(Node n, Node n2) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleRemoveEdge(n,n2);
		}	}

	public void handleFusion(Community resultingCommunity, Community suppressedCommunity) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleFusion(resultingCommunity, suppressedCommunity);
		}
		
	}
	public void handleDateChange(String date) {
		for(OutputHandlerInterface oh : outputsClasses){
			oh.handleDateChange(date);
		}		
	}
}
