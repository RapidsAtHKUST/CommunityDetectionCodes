package tools;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;


public class ToolDate {
	
	private static Date initDate = new Date("01/01/93");
	
	public static long timInDaySince(Date d){
		return (d.getTime()-initDate.getTime())/1000/60/60/24;
	}
	
	public static long timInDayBetween(Date beginning, Date end){
		return (end.getTime()-beginning.getTime())/1000/60/60/24;
	}
	
	public static Date dateFromString(String date, String format){
		SimpleDateFormat formatter = new SimpleDateFormat(format);
		try {
			return formatter.parse(date);
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
}
