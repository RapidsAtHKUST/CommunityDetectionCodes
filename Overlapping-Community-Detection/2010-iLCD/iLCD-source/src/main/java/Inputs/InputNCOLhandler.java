package Inputs;

import tools.DebugMode;
import tools.TextFileHandler;
import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Operations.AddEdgeOperation;
import TemporalNetworkManipulation.Operations.Operation;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class InputNCOLhandler extends InputHandlerInterface{

	public InputNCOLhandler(Network net) {
		super(net);
		// TODO Auto-generated constructor stub
	}

	private static String ncolSeparator = "	";//tab

	public Operation operationFromLine(String line){
		
			String el1;
			String el2=null;
			Operation op = null;
		//change time : each operation occurs at a different time
		theNetwork.changeTime(theNetwork.getCurrentTime().increment());
		if(TextFileHandler.splitLine(line, ncolSeparator).size()!=2){
			System.err.println("line : "+line+" is incorrectly formated");
			System.exit(-1);
		}
		el1 = TextFileHandler.splitLine(line, ncolSeparator).firstElement();
		el2 = TextFileHandler.splitLine(line, ncolSeparator).lastElement();
		
		op = new AddEdgeOperation(theNetwork.getNode(el1), theNetwork.getNode(el2));

		
		return op;
	}
}
