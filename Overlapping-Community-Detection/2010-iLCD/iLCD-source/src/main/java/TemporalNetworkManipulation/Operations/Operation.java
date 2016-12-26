package TemporalNetworkManipulation.Operations;

import java.util.HashSet;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;

public class Operation {
	
	//in the case of files where dates are not defined, allow to identify steps
	//private static long currentTime = -1;
	
	//in the case of files where dates are defined
	//static long operationTime = -1;
	
	//String timeStamp;
	Node node1;
	Node node2;
	Community c1;
	Community c2;
	Community c3;

	HashSet<Node> nodes;
	public OperationType op;
	
	public enum OperationType { ADD, REMOVE,BIRTH,DEATH,FUSION,GROWTH,CONTRACTION };
	
	//constructor 
	public Operation(Node n1, Node n2, Community c1, Community c2, Community c3){
	this.c1 = c1;
	this.c2 = c2;
	this.node1 = n1;
	this.node2 = n2;
	this.c3 = c3;
	}
	
	public Node getNode1(){
		return this.node1;
	}
	
	public Node getNode2(){
		return this.node2;
	}
	
	public Community getCommunity1(){
		return this.c1;
	}
	
	public Community getCommunity2(){
		return this.c2;
	}
	
	public Community getCommunity3(){
		return this.c3;
	}
	
	public HashSet<Node> getNodes(){
		return this.nodes;
	}
	
	
	/*public void initialiseCurrentStep(){
		if(currentStep==0)
			this.currentStep=0;
		else{
			System.err.println("problem, multiple initialisation of the current step");
			System.exit(-1);
		}
	}
	
	private void incrementStep(){
		if(currentStep==-1){
			System.err.println("problem, increment of a step not initialized");
			System.exit(-1);
		}
		this.currentStep++;
	}*/
	
	


}
