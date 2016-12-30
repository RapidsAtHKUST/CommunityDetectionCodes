package TemporalNetworkManipulation;

public class MyDate implements Comparable{

	public enum dateType {YYYYMMDD,YYYYMMDDHHMMSS,NONE};

	private long date ;
	private dateType thisDateType;

	
	public MyDate(String date, dateType type){
		thisDateType = type;
		switch(type){
			case YYYYMMDD :
			case YYYYMMDDHHMMSS :
				this.date = Long.valueOf(date);
			break;
			case NONE : 
				this.date = Long.valueOf(date);
			
		}
	}
	
	public MyDate(MyDate currentTime) {
		this.date = currentTime.date;
		this.thisDateType = currentTime.thisDateType;
}

	public String writeDate(dateType type){
		switch(type){
		case YYYYMMDD :
		case YYYYMMDDHHMMSS :
		case NONE :
			return String.valueOf(date);
		}
		
		System.err.println("sorry, this output date format is not handled yet.");
		System.exit(-1);
		return "";
	}
	
	public boolean before(MyDate toCompare){
		return this.date<toCompare.date;
	}
	
	public boolean after(MyDate toCompare){
		return this.date>toCompare.date;
	}
	
	public boolean sameDateAs(MyDate toCompare){
		return this.date==toCompare.date;

	}
	
	public static MyDate beginningOfTheUniverse(){
		MyDate d = new MyDate("-1",dateType.NONE);
		return  d;
	}
	
	public MyDate increment() {
		if(this.thisDateType!=MyDate.dateType.NONE){
			System.err.println("Error : trying to incremente time while on real time mode");
			System.exit(-1);
		}
		MyDate newDate = new MyDate(this.writeDate(this.thisDateType),this.thisDateType);
		newDate.date++;
		return newDate;
	}

	@Override
	public int compareTo(Object o) {
		return ((int)this.date-(int)((MyDate)o).date);
	}
}
