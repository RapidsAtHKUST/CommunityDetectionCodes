package TemporalNetworkManipulation.Operations;

import TemporalNetworkManipulation.Node;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class AddEdgeOperation extends Operation{

	public AddEdgeOperation(Node n1, Node n2) {
		super(n1, n2, null, null, null);
		this.op = OperationType.ADD;
		// TODO Auto-generated constructor stub
	}
	
}