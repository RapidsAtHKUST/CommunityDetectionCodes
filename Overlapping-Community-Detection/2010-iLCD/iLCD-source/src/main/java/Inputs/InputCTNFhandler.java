package Inputs;

import tools.DebugMode;
import tools.TextFileHandler;
import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Operations.AddEdgeOperation;
import TemporalNetworkManipulation.Operations.BirthCommunityOperation;
import TemporalNetworkManipulation.Operations.DeathCommunityOperation;
import TemporalNetworkManipulation.Operations.FusionCommunityOperation;
import TemporalNetworkManipulation.Operations.GrowthCommunityOperation;
import TemporalNetworkManipulation.Operations.Operation;
import TemporalNetworkManipulation.Operations.RemoveEdgeOperation;
import TemporalNetworkManipulation.Operations.ContractCommunityOperation;
import TemporalNetworkManipulation.Operations.Operation.OperationType;

public class InputCTNFhandler extends InputHandlerInterface{

	public InputCTNFhandler(Network net) {
		super(net);
		// TODO Auto-generated constructor stub
	}

	private static String ctnfSeparator = "	";//tab

	public Operation operationFromLine(String line){
		
		String el1;
		String el2=null;
		String operationString;
		Operation.OperationType operation = null;
		Operation op = null;
		
		DebugMode.printDebug("processing the TNF file");
		if(line.charAt(0)=='#'){
			//change time : update with given time
			theNetwork.changeTime(theNetwork.dateManager.getDate(line.substring(1, line.length())));
		}
		else{
			if(TextFileHandler.splitLine(line, ctnfSeparator).size()>3){
				System.err.println("line : "+line+" is incorrectly formated");
				System.exit(-1);
			}
			operationString  = TextFileHandler.splitLine(line, ctnfSeparator).get(0);
			el1  = TextFileHandler.splitLine(line, ctnfSeparator).get(1);
			if(TextFileHandler.splitLine(line, ctnfSeparator).size()>2)
				el2  = TextFileHandler.splitLine(line, ctnfSeparator).get(2);
			
			//--------handling network modifications : edges adding and removal---------
			if(operationString.equals("-")){
				operation = Operation.OperationType.REMOVE;
				op = new RemoveEdgeOperation(theNetwork.getNode(el1),theNetwork.getNode(el2));
	
			}
			if(operationString.equals("+")){
				operation = Operation.OperationType.ADD;
				op = new AddEdgeOperation(theNetwork.getNode(el1),theNetwork.getNode(el2));
			}
			
			
			//-----------handling communities operations----------
			
			//community creation
			/*if(operationString.equals("+c")){
					operation = OperationType.BIRTH;
					theNetwork.createCommunity(Integer.parseInt(el1));
					op = new BirthCommunityOperation(theNetwork.getCommunity(Integer.parseInt(el1)));
			}*/
			
			//community death
			if(operationString.equals("-c")){
				operation = OperationType.DEATH;
				op = new DeathCommunityOperation(theNetwork.getCommunity(Integer.parseInt(el1)));
			}
			//community growth
			if(operationString.equals("+nc")){
				operation = OperationType.GROWTH;
				DebugMode.printDebug("inputProcessor : +NC "+ el1+" "+el2);
				op = new GrowthCommunityOperation(theNetwork.getCommunity(Integer.parseInt(el2)),theNetwork.getNode(el1));
			}					
			//community shrink
			if(operationString.equals("-nc")){
				operation = OperationType.CONTRACTION;
				op = new ContractCommunityOperation(theNetwork.getCommunity(Integer.parseInt(el2)),theNetwork.getNode(el1));
			}
			//community fusion
			if(operationString.equals("=")){
				operation = OperationType.FUSION;
				op = new FusionCommunityOperation(theNetwork.getCommunity(Integer.parseInt(el1)),theNetwork.getCommunity(Integer.parseInt(el2)));
			}
			
	
			if(operation==null){
				System.err.println("error in entry file, unknown operator : "+ operationString+" ");
				System.exit(-1);
			}
		}
		return op;
	}
}
