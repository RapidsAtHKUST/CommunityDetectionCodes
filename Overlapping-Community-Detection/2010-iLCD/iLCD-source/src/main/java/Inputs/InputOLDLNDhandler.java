package Inputs;

import tools.DebugMode;
import tools.TextFileHandler;
import TemporalNetworkManipulation.MyDate;
import TemporalNetworkManipulation.Network;
import TemporalNetworkManipulation.Operations.AddEdgeOperation;
import TemporalNetworkManipulation.Operations.BirthCommunityOperation;
import TemporalNetworkManipulation.Operations.DeathCommunityOperation;
import TemporalNetworkManipulation.Operations.FusionCommunityOperation;
import TemporalNetworkManipulation.Operations.GrowthCommunityOperation;
import TemporalNetworkManipulation.Operations.Operation;
import TemporalNetworkManipulation.Operations.Operation.OperationType;
import TemporalNetworkManipulation.Operations.RemoveEdgeOperation;
import TemporalNetworkManipulation.Operations.ContractCommunityOperation;

public class InputOLDLNDhandler extends InputHandlerInterface{

	public InputOLDLNDhandler(Network net) {
		super(net);
		// TODO Auto-generated constructor stub
	}

	private static String lndSeparator = "	";//tab

	public Operation operationFromLine(String line){
		
		String el1;
		String el2=null;
		String operationString;
		Operation.OperationType operation = null;
		Operation op = null;
		
		DebugMode.printDebug("processing the oldLND file");
		
			if(TextFileHandler.splitLine(line, lndSeparator).size()!=4){
				System.err.println("line : "+line+" is incorrectly formated");
				System.exit(-1);
			}
			
			//handling the date
			String date = TextFileHandler.splitLine(line, lndSeparator).get(0);
			MyDate theDate = this.theNetwork.dateManager.getDate(date);
			if(!theNetwork.getCurrentTime().sameDateAs(theDate)){
				theNetwork.changeTime(theDate);
			}
			
			
			operationString  = TextFileHandler.splitLine(line, lndSeparator).get(1);
			el1  = TextFileHandler.splitLine(line, lndSeparator).get(2);
			//if(TextFileHandler.splitLine(line, lndSeparator).size()>2)
			el2  = TextFileHandler.splitLine(line, lndSeparator).get(3);
			
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
		return op;
	}
}
