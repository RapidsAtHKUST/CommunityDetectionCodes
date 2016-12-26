package TemporalNetworkManipulation;
import gnu.trove.map.hash.THashMap;
import gnu.trove.set.hash.THashSet;

import java.util.ArrayList;
import java.util.HashMap;

import Outputs.OutputHandlerInterface;
import TemporalNetworkManipulation.Operations.DeathCommunityOperation;
import TemporalNetworkManipulation.Operations.Operation;
import TemporalNetworkManipulation.Operations.ContractCommunityOperation;

import tools.DebugMode;



public class Network {

	private MyDate currentDate = MyDate.beginningOfTheUniverse();
	
	public MyDateManager dateManager;
	public OutputHandlerInterface usedOutput = null;
	private HashMap<String, Node> nodes = new HashMap<String,Node>();
	private HashMap<Integer,Community> communities = new HashMap<Integer,Community>();
	
	public Network(MyDateManager dateManager, OutputHandlerInterface outputHandler){
		this.dateManager=dateManager;
		this.usedOutput = outputHandler;
	}
	//return a node from its name. If the node does not exist, create it
	public Node getNode(String nodeName){
		if(!nodes.containsKey(nodeName))
			nodes.put(nodeName, new Node(nodeName));
			
		return nodes.get(nodeName);
	}

	Community c;
	Operation newOp;
	
	//modify the network, plus call the output to "display" the modification
	public void doOperation(Operation currentOperation) {
		switch(currentOperation.op){
		case ADD :
			usedOutput.handleNewEdge(currentOperation.getNode1(), currentOperation.getNode2());
			DebugMode.printDebug("operation is : "+currentOperation.op+" "+currentOperation.getNode1().getName()+" "+currentOperation.getNode2().getName());
			currentOperation.getNode1().addNeighbor(currentOperation.getNode2());
			break;
		case REMOVE :
			usedOutput.handleRemoveEdge(currentOperation.getNode1(), currentOperation.getNode2());
			currentOperation.getNode1().removeNeighbor(currentOperation.getNode2());
			break;
		case BIRTH : 
			c = currentOperation.getCommunity1();
			c.setBirthDate(this.getCurrentTime());
			addCommunity(c);
			usedOutput.handleBirth(c);
			break;
		case DEATH :
			c = currentOperation.getCommunity1();
			
			ArrayList<Node> tempList = new ArrayList<Node>(c.getComponents());
			for(Node n : tempList){
				newOp = new ContractCommunityOperation(c, n);
				this.doOperation(newOp);
			}
			
			this.removeCommunity(c);
			usedOutput.handleDeath(c);
			
			break;
		case FUSION : 
			newOp = new DeathCommunityOperation(currentOperation.getCommunity2());
			this.doOperation(newOp);
			
			usedOutput.handleFusion(currentOperation.getCommunity1(), currentOperation.getCommunity2());
			break;
		case GROWTH : 
			usedOutput.handleGrowth(currentOperation.getNode1(), currentOperation.getCommunity1());
			currentOperation.getCommunity1().addNodeToCommunity(currentOperation.getNode1());
			break;
		case CONTRACTION : 
			usedOutput.handleContraction(currentOperation.getNode1(), currentOperation.getCommunity1());
			currentOperation.getCommunity1().removeNodeFromCommunity(currentOperation.getNode1());
			break;
		}
		
	}

	public void changeTime(MyDate newDate) {
		if(!newDate.after(this.currentDate)){
			System.err.println("Error : problem with the dates : "+newDate+" is not after "+this.currentDate);
			System.exit(-1);
		}
		
		this.currentDate = newDate;
	}

	public MyDate getCurrentTime() {
		return this.currentDate;
	}


	public void createCommunity(Integer c){
		if(this.communities.containsKey(c)){
			System.err.println("Error : trying to create an existing community : "+c);
			System.exit(-1);
		}
		this.addCommunity(new Community(c));

	}
	
	public Community getCommunity(Integer com){
		if(!this.communities.containsKey(com)){
			System.err.println("Error : trying to retrieve an unexisting community : "+com);
			System.exit(-1);
		}
		return this.communities.get(com);
	}
	public void removeCommunity(Community c) {
		if(!this.communities.containsKey(c.getID())){
			System.err.println("Error : trying to remove unexisting community : "+c.getID());
			System.exit(-1);
		}
		this.communities.remove(c.getID());
		
	}
	
	public void addCommunity(Community c){
		if(this.communities.containsKey(c.getID())){
			System.err.println("Error : trying to add already present community : "+c.getID());
			System.exit(-1);
		}
		this.communities.put(c.getID(),c);
	}

	public THashSet<Community> getCommunities(){
		return new THashSet<Community>(this.communities.values());
	}
}
