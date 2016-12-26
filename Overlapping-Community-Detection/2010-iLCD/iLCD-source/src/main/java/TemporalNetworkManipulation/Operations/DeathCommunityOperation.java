package TemporalNetworkManipulation.Operations;

import TemporalNetworkManipulation.Community;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class DeathCommunityOperation extends Operation{

	public DeathCommunityOperation(Community c) {
		super(null, null, c, null, null);
		this.op = OperationType.DEATH;
		// TODO Auto-generated constructor stub
	}
	
}