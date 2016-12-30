package Inputs;

import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Operations.Operation;

public abstract class InputHandlerInterface {

	protected Network theNetwork;
	InputHandlerInterface(Network net){
		theNetwork = net;
	}
	public abstract Operation operationFromLine(String line);
}
