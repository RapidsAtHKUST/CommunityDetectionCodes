package Outputs;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;




public interface OutputHandlerInterface {

	
	

	public void initialise(String outputFile);

	
	public void terminate();
	
	
	
	
	public void handleGrowth(Node n, Community c);
	
	public void handleGrowth(Community initialCommunity, Community resultingCommunity);

	public void handleBirth(Community c);

	public void handleDeath(Community c);

	//public void handleContraction(Community tempSavedCom, Community community);
	
	public void handleContraction(Node n, Community c);
	
	public void handleNewEdge(Node n, Node n2);
	
	public void handleRemoveEdge(Node n, Node n2);

	public void handleFusion(Community resultingCommunity, Community suppressedCommunity);
	public void handleDateChange(String date);
}
