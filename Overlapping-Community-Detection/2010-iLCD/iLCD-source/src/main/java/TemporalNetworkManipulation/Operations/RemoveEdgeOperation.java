package TemporalNetworkManipulation.Operations;

import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class RemoveEdgeOperation extends Operation{

	public RemoveEdgeOperation(Node n1, Node n2) {
		super(n1, n2, null, null, null);
		this.op = OperationType.REMOVE;
		// TODO Auto-generated constructor stub
	}
	
}