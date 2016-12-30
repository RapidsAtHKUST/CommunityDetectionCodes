package TemporalNetworkManipulation;

import java.util.ArrayList;
import java.util.Collection;
import gnu.trove.set.hash.THashSet;




public class Node {
	
	
	private String name ;
	private THashSet<Node> neighbors = new THashSet<Node>();
	private THashSet<Community> communities = new THashSet<Community>();
	
	
	public Node(String nodeName) {
		this.name = nodeName;
	}
	
	public THashSet<Community> getCommunities(){
		return this.communities;
	}

	protected void addCommunity(Community c) {
		if(communities.contains(c)){
			System.err.println("problem : multiple adding of the same node to same community :"+this.name+" to "+c.getID());
			System.exit(-1);
		}
		communities.add(c);
		
	}

	public String getName() {
		return this.name;
	}

	protected void removeCommunity(Community c) {
		if(!communities.contains(c)){
			System.err.println("problem : try to remove a community from a node which do not belong to it :"+this.name+" to "+c.getID());
			System.exit(-1);
		}
		communities.remove(c);		
	}

	public THashSet<Node> getNeighbors() {
		return this.neighbors;
	}
	
	public THashSet<Node> getNeighborsInCommunity(Community c){
		THashSet<Node> n = new THashSet<Node>(c.getComponents());
		n.retainAll(this.neighbors);
		
		return n;
	}

	protected void addNeighbor(Node node2) {
		if(this.neighbors.contains(node2)){
			System.err.println("problem : try to connect two already connected nodes :"+this.name+" to "+node2.name);
			System.exit(-1);
		}
		this.neighbors.add(node2);
		node2.neighbors.add(this);
		
	}
	
	protected void removeNeighbor(Node node2) {
		if(!this.neighbors.contains(node2)){
			System.err.println("problem : try to disconnect two non connected nodes :"+this.name+" to "+node2.name);
			System.exit(-1);
		}
		this.neighbors.remove(node2);
		node2.neighbors.remove(this);
	}
}
