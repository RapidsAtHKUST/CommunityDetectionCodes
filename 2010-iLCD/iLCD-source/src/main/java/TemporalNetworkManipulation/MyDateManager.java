package TemporalNetworkManipulation;

import TemporalNetworkManipulation.MyDate.dateType;

public class MyDateManager {

	
	private MyDate.dateType inputDateFormat ;
	private MyDate.dateType outputDateFormat ;
		
	
	public MyDateManager(String inputFormat){
		inputDateFormat = convertDateFormat(inputFormat);
		outputDateFormat = convertDateFormat("NONE");
	}
	
	public void setOutputDateFormat(String outputFormat){
		this.outputDateFormat = convertDateFormat(outputFormat);

	}
	
	public MyDate getDate(String date){
		return new MyDate(date, inputDateFormat);
		
	}
	
	/*public MyDate getDate(){
		MyDate toReturn =  new MyDate(String.valueOf(currentStep), inputDateFormat);
		currentStep++ ;
		return toReturn;
		
	}*/
	
	public String writeDate(MyDate aDate){
		return aDate.writeDate(outputDateFormat);
	}
	
	private MyDate.dateType convertDateFormat(String stringFormat){
		if(stringFormat.equals("YYYYMMDD"))
			 return dateType.YYYYMMDD;
		if(stringFormat.equals("YYYYMMDDHHMMSS"))
			return dateType.YYYYMMDDHHMMSS;
		if(stringFormat.equals("NONE"))
			return dateType.NONE;
		
		System.err.println("sorry, the date format : "+stringFormat+" is not yet supported");
		System.exit(-1);
		return null;
	}

	

}
