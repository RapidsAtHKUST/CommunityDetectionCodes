package TemporalNetworkManipulation.Operations;

import java.util.HashSet;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class BirthCommunityOperation extends Operation{

	private static int currentID = 0;

	public BirthCommunityOperation(Community c) {
		super(null, null, null, null, null);
		this.c1=c;
		this.op = OperationType.BIRTH;
		//this.nodes = initialNodes;
		// TODO Auto-generated constructor stub
	}
	
}