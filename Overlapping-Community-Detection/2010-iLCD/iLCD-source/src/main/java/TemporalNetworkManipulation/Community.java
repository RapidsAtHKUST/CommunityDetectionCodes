package TemporalNetworkManipulation;
import gnu.trove.impl.hash.THash;
import gnu.trove.set.hash.THashSet;

import java.util.ArrayList;
import java.util.HashSet;

import tools.DebugMode;



public class Community implements Comparable<Community>{
	private int ID;
	private MyDate birth; //a number if no dates, or a date in format YYYYMMDDHHMMSS
	private MyDate death;
	private THashSet<Node> components = new THashSet<Node>();
	
	private static int countIds = 0;

	
	//constructor used to make a local copy of a community
	public Community(Community c) {
		this.components = new THashSet<Node>(c.components);
	}

	public Community(){
		this.ID=countIds;
		countIds++;
	}
	
	public Community(int ID){
		this.ID=ID;
	}

	private void addNode(Node node1) {
		if(components.contains(node1)){
			System.err.println("problem : multiple adding of the same node to same community :"+node1.getName()+" to "+this.ID);
			System.exit(-1);
		}
		components.add(node1);
			
		
	}

	//fonction in charge of adding properly a node to a community
	public void addNodeToCommunity(Node n){
		this.addNode(n);
		n.addCommunity(this);
	}
		
	//fonction in charge of removing properly a node to a community
	protected void removeNodeFromCommunity(Node n){
		DebugMode.printDebug(" operation is  : REMOVE "+n.getName()+" from "+this.getID());
		this.removeNode(n);
		n.removeCommunity(this);
	}

	protected void removeNode(Node n) {
		if(!components.contains(n)){
			System.err.println("problem : trying to remove a node which is not in a community :"+n.getName()+" from "+this.ID);
			System.exit(-1);
		}
		components.remove(n);
			
		}

	public int getID() {
		return this.ID;
	}


	public boolean isDifferent(Community otherCom) {
		HashSet<Node> compareElements = new HashSet<Node>(this.components);
		
		if(this.components.size()<otherCom.components.size())
			return false;
		compareElements.removeAll(otherCom.components);
		if(!compareElements.isEmpty())
			return false;
		
/*		compareElements = new HashSet<Node>(otherCom.components);
		compareElements.retainAll(this.components);
		if(!compareElements.isEmpty())
			return false;*/
		
		return true;
		
					
					
	}
	
	protected void setBirthDate(MyDate date){
		this.birth = date;
	}


	public THashSet<Node> getComponents() {
		return this.components;
	}

	public boolean youngerThan(Community c2) {
		return this.birth.before(c2.birth);
	}

	@Override
	public int compareTo(Community o) {
		return this.birth.compareTo(o.birth);
	}

	public THashSet<Node> getCommonNodes(Community cTested) {
		THashSet<Node> common = new THashSet<Node>(this.getComponents());
		common.retainAll(cTested.getComponents());
		return common;
	}
	
	
}
