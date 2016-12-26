package TemporalNetworkManipulation.Operations;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class FusionCommunityOperation extends Operation{

	public FusionCommunityOperation(Community absorb, Community mergedIn) {
		super(null, null, absorb, mergedIn, null);
		this.op = OperationType.FUSION;
		// TODO Auto-generated constructor stub
	}
	
}