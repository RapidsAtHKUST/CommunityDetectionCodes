package CorePackage;
import implementable.Implementation;
import implementable.ImplementationRNHM;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.List;

import static org.kohsuke.args4j.ExampleMode.ALL;
import org.kohsuke.args4j.Argument;
import org.kohsuke.args4j.CmdLineException;
import org.kohsuke.args4j.CmdLineParser;
import org.kohsuke.args4j.Option;

import Outputs.OutputCTNF;
import TemporalNetworkManipulation.CoreProcessor;
import TemporalNetworkManipulation.Interface;
import TemporalNetworkManipulation.MyDateManager;
import TemporalNetworkManipulation.Network;

import communityDetectionPackage.ILCDMetaAlgorithm;

import tools.DebugMode;
import tools.TextFileHandler;

public class Main {

	static GregorianCalendar calendar = new java.util.GregorianCalendar();
	public static SimpleDateFormat myDateFormat = new SimpleDateFormat("yyyyMMdd");
	
	
	//---------input----------------------------------
	@Option(name="-i",usage="input file")
	private static String inputFile;
	//private static String inputFile;
	//private static String inputFile = "/Users/remynii/Desktop/testNetworkJapon2013/hiverStrictX2.lnd";

	//private static String inputFile = "/Users/remynii/Desktop/ALLTAGSOFVIDEOS.lnd";
	//private static String inputFile = "/Users/remynii/Desktop/netwokStep703.ncol";
	//private static String inputFile = "/Users/remy/Desktop/testNetworkJapon2013/karateSorted.ncol";

	
	//--------output-------------
	@Option(name="-o",usage="file to write the output in, without extention. If not specified, same name as input file")
	//private static String outputFile ;
	private static String outputFile;
	
	
	//@Option(name="-xml",usage="write complete xml description of interesting communities")
	// public static String textualXML = null; //"/Users/quetzalcroak/Desktop/fileiLCD2/detailXML.xml"

	
	//---------dialog---------------
	@Option(name="-debug",usage="display some debug dialogs. Default : false")
	public static boolean debugDialogs  = false;
	
	@Option(name="-verbose",usage="display basic dialogs (number of line processed...) Default : true")
	public static boolean basicDialogs  = true;
	
	
	//------------parameters-------------
	
	@Option(name="-s",usage="size of the minimal community. Default : 3")
	 public static int smallestCommunitySize = 3;
	
	@Option(name="-fRatio",usage="fusion of community ratio. Default : 0.5")
	 public static float fusionParameter = 0.5f;//0.75d;
	
	@Option(name="-bThreshold",usage="threshold of belonging. Default : 0.5 ")
	 public static float integrateParameter = 0.5f;//0.5d;
	
	
	//---------options---------------------
	//currently, static network will only change the way to print the result. Could also change the order of edges
	//to be more efficient.
	@Option(name="-staticN",usage="if true, the provided network will considered as static.")
	 public static boolean staticN = false;
	
	@Option(name="-inputDateF",usage="select your date formating. Default : YYYYMMDD. possibilities : YYYYMMDDHHMMSS, YYYYMMDD, NONE")
	 public static String inputDateF = "YYYYMMDD";
	
	@Option(name="-outputDateF",usage="select your date formating. Default : YYYYMMDD. possibilities : YYYYMMDDHHMMSS, YYYYMMDD, NONE")
	 public static String outputDateF = "YYYYMMDD";
	
	

	
	
	 @Argument
	    private List<String> arguments = new ArrayList<String>();

	 
	 public static void main(String args[]){
			
		 System.setProperty("file.encoding", "UTF-8");
		 	DebugMode.printBasic("----- starting initialisation ------");
			new Main().argumentsHandling(args);
	        
			//-------------initialise debug printing------------------------
			DebugMode.printDebugDialogs = debugDialogs;
			DebugMode.printBasicDialogs = basicDialogs;
			
			//creating the temporal network processing part
			Interface TNinterface = new Interface(inputFile, inputDateF);
			
			if(staticN)
				TNinterface.addOutput(Interface.PossibleOutputFormat.STATIC, outputFile, outputDateF);
			else{
				TNinterface.addOutput(Interface.PossibleOutputFormat.CTNF, outputFile, outputDateF);
				TNinterface.addOutput(Interface.PossibleOutputFormat.SUMMARY, outputFile, outputDateF);
			}

			
			//initializing the iLCD algorithm
			Implementation choosedImplementation = new ImplementationRNHM(smallestCommunitySize, integrateParameter, fusionParameter);
			ILCDMetaAlgorithm iLCD = new ILCDMetaAlgorithm(choosedImplementation, TNinterface.mainNetwork);
			
			TNinterface.setOperationHandler(iLCD);
			
			
		 	DebugMode.printBasic("----- starting flow reading ------");

		 	TNinterface.readOperationStream();
			
		 	DebugMode.printBasic("----- detection finished ------");

		 	TNinterface.terminate();
			
			
		}
	 
	 private void argumentsHandling(String args[]){
			CmdLineParser parser = new CmdLineParser(this);
			
			parser.setUsageWidth(80);

	        try {
	            // parse the arguments.
	            parser.parseArgument(args);

	            // you can parse additional arguments if you want.
	            // parser.parseArgument("more","args");

	            // after parsing arguments, you should check
	            // if enough arguments are given.
	           // if( arguments.isEmpty() )
	            if( inputFile == null )
	                throw new CmdLineException("You MUST specify an input file (-i)");
	            
	            if(outputFile==null)
	            	outputFile=inputFile;

	        } catch( CmdLineException e ) {
	            // if there's a problem in the command line,
	            // you'll get this exception. this will report
	            // an error message.
	            System.err.println(e.getMessage());
	            System.err.println("java SampleMain [options...] arguments...");
	            // print the list of available options
	            parser.printUsage(System.err);
	            System.err.println();

	            // print option sample. This is useful some time
	            System.err.println("  Example: java SampleMain"+parser.printExample(ALL));

	            System.exit(0);
	        }
		}


	
}
