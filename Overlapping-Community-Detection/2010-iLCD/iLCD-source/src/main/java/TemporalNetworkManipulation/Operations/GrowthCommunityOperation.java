package TemporalNetworkManipulation.Operations;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class GrowthCommunityOperation extends Operation{

	public GrowthCommunityOperation(Community c, Node n) {
		super(n, null, c, null, null);
		this.op = OperationType.GROWTH;
		// TODO Auto-generated constructor stub
	}
	
}