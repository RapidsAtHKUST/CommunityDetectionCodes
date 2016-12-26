package TemporalNetworkManipulation.Operations;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class ContractCommunityOperation extends Operation{

	public ContractCommunityOperation(Community c, Node n) {
		super(n, null, c, null, null);
		this.c1 = c;
		this.node1=n;
		this.op = OperationType.CONTRACTION;
		// TODO Auto-generated constructor stub
	}
	
	
}