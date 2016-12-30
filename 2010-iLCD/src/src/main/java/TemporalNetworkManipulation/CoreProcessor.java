package TemporalNetworkManipulation;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;

import Inputs.InputHandlerInterface;
import TemporalNetworkManipulation.Interface.PossibleInputFormat;
import TemporalNetworkManipulation.Operations.Operation;
import TemporalNetworkManipulation.Operations.Operation.OperationType;



import tools.DebugMode;
import tools.TextFileHandler;


public class CoreProcessor {

	private Network theNetwork;
	protected OperationsHandler operationHandler;
	private InputHandlerInterface inputHandler;
	
	private BufferedReader reader = null;
	
	
	public CoreProcessor(Network mainNetwork) {
		this.theNetwork = mainNetwork;
	}

	public void setOperationHandler(OperationsHandler opHandler){
		this.operationHandler = opHandler;
	}
	
	public void setInputHandler(InputHandlerInterface inputHandler){
		this.inputHandler=inputHandler;
	}
	
	public void initializeReading(String file){	
		try {
			 reader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
		} catch (Exception e) {
			System.out.println("file doesn't exist "+ file);
			e.printStackTrace();
		}
	}
	
	//return true if the line contains an operation, false if it's a date change
	public void readALine(String line){
		Operation currentOperation = this.getOperation(line);
		if(currentOperation!=null){
			theNetwork.doOperation(currentOperation);
			if(operationHandler!=null)
				operationHandler.handle(currentOperation);
		}
	}
	
	public void readOperationStream(String file){
		this.initializeReading(file);
		String line="";
		try{
			DebugMode.printDebug("reading the operation stream");
			int countLines = 0;
			while ((line=reader.readLine())!=null){
				if(countLines%1000==0)
					DebugMode.printBasic("----- processing operation "+countLines +"------");
				countLines++;
				readALine(line);
			}
		}
		catch (Exception e) {
			System.out.println("problem with line "+ line);
			e.printStackTrace();
		}
	}
	
	
	
	public Operation getOperation(String line){

		return inputHandler.operationFromLine(line);
	}
	
/*	public void setTypeOfInputFile(FileType tof){
		if(typeOfInputFile == FileType.UNKNOWN)
			this.typeOfInputFile=tof;
		else{
			System.err.println("error, type of file already defined");
			System.exit(-1);
		}
	}*/
	
}
