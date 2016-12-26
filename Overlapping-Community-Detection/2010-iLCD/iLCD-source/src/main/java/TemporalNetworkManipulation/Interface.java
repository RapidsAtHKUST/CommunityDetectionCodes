package TemporalNetworkManipulation;

import java.io.File;

import Inputs.InputCTNFhandler;
import Inputs.InputNCOLhandler;
import Inputs.InputOLDLNDhandler;
import Outputs.OutputCTNF;
import Outputs.OutputMultipleHandler;
import Outputs.OutputStatic;
import Outputs.OutputSummary;

public class Interface {

	public Network mainNetwork ;
	public OutputMultipleHandler myOutputs = new OutputMultipleHandler();

	
	private MyDateManager dateManager;
	private PossibleInputFormat inputFileType;
	
	private String inputFile;
	private CoreProcessor mainCoreProcessor;
	
	public static enum PossibleOutputFormat{CTNF,SUMMARY,STATIC}
	public enum PossibleInputFormat {UNKNOWN, NCOL, TNF, CTNF, LND};
	
	
	public Interface(String inputFile, String inputDateF){
		//---------determine inputfile type and initialise date manager---------
		inputFileType = getFileType(inputFile);
		this.inputFile = inputFile;
		
		
		dateManager = new MyDateManager(inputDateF);

		//----------------------------------------------------------------------
		//-------create the network structure------------------
		mainNetwork = new Network(dateManager,myOutputs);
		
		//--------initialise the core processor, inputs, etc...
		mainCoreProcessor = new CoreProcessor(mainNetwork);
		
		
		switch(inputFileType){
			case NCOL :
				 mainCoreProcessor.setInputHandler(new InputNCOLhandler(mainNetwork));
				break;
			case TNF : 
				mainCoreProcessor.setInputHandler(new InputCTNFhandler(mainNetwork));
				break;
			case LND :
				mainCoreProcessor.setInputHandler(new InputOLDLNDhandler(mainNetwork));
		}
		//----------------------------------------------------------------------
		
		//mainCoreProcessor.setTypeOfInputFile(inputFileType);
		
	}
	
	public void addOutput(PossibleOutputFormat format, String outputFile, String outputFormat){
		mainNetwork.dateManager.setOutputDateFormat(outputFormat);
		
		switch(format){
		case CTNF :
			OutputCTNF ctnf = new OutputCTNF(dateManager, mainNetwork);
			ctnf.initialise(outputFile);
			this.myOutputs.addOutputHandler(ctnf);
			
			break;
		case SUMMARY :
			OutputSummary summary = new OutputSummary(dateManager, mainNetwork);
			summary.initialise(outputFile);
			this.myOutputs.addOutputHandler(summary);
			break;
		
		case STATIC :
			OutputStatic staticN = new OutputStatic(dateManager, mainNetwork);
			staticN.initialise(outputFile);
			this.myOutputs.addOutputHandler(staticN);
			break;
		}
		
	}
	
	public void setOperationHandler(OperationsHandler operationHandler){
		if(mainCoreProcessor.operationHandler!=null){
			System.err.println("Error : you can't spcify more than one operation handler");
			System.exit(-1);
		}
		mainCoreProcessor.setOperationHandler(operationHandler);
	}
	
	public void readOperationStream() {
		if(mainCoreProcessor==null){
			System.err.println("Error : you probably forgot to specify your operationHandler");
			System.exit(-1);
		}
		 mainCoreProcessor.readOperationStream(inputFile);
	}
	
	//return the date after the processing of the operation
	public String processALine(String line) {
		if(mainCoreProcessor==null){
			System.err.println("Error : you probably forgot to specify your operationHandler");
			System.exit(-1);
		}
		mainCoreProcessor.readALine(line);
		return this.getCurrentDate();

	}
	
	public void terminate() {
		myOutputs.terminate();
		
	}
	
	public static PossibleInputFormat getFileType(String file){
		File f = new File(file);
		String fileName = f.getName();
		String extension = fileName.substring(fileName.lastIndexOf(".")+1,fileName.length());
		if(extension.equals("ncol")){
			return PossibleInputFormat.NCOL;
		}
		
		if(extension.equals("ctnf") || extension.equals("tnf")){
			return PossibleInputFormat.TNF;
		}
		
		if(extension.equals("lnd")){
			return PossibleInputFormat.LND;
		}
		System.err.println("Sorry, the type of entry file : -"+extension +"- is not yet supported. Please check which types of files are supported");
		System.exit(-1);
		return PossibleInputFormat.UNKNOWN;
	}
	
	public String getCurrentDate(){
		return dateManager.writeDate(mainNetwork.getCurrentTime());
	}
	
	public PossibleInputFormat getInputFileType(){
		return this.inputFileType;
	}
}
