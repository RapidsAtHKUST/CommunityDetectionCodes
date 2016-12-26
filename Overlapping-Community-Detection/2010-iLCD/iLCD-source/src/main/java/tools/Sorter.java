package tools;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

public class Sorter<T> {

	private HashMap<T,Element<T>> listToCompare = new HashMap<T,Element<T>>();
	
	public Sorter(){
		
	}
	
	//set the value if there was no key before. Otherwise, add the value to the previous one
	public void add(T el, Double val){
		if(!listToCompare.containsKey(el)){
			Element temp = new Element();
			temp.el=el;
			temp.val=val;
			
			listToCompare.put(el, temp);
		}
		else
			modVal(el,listToCompare.get(el).val+val);
	}
	
//	public void sort(){
		
	//}
	
	//sorted in ascending order
	public ArrayList<T> getSortedValues(){
		ArrayList<Element<T>> toSort = new ArrayList<Element<T>>(listToCompare.values());
		Collections.sort(toSort);
		ArrayList<T> toReturn = new ArrayList<T>();
		for(Element<T> el : toSort)
			toReturn.add(el.el);
		
		return toReturn;
		
	}
	
	public ArrayList<T> getElts(){
		
		return new ArrayList<T>(listToCompare.keySet());
		
	}
	
	public class Element<T> implements Comparable{
		private T el;
		private Double val;
		
		@Override
		public int compareTo(Object o){
			
			return val.compareTo(((Element<T>)o).val);
			
		}
	}
	
	public void modVal(T el, Double val){
		//Element temp = new Element();
		//temp.el=el;
		listToCompare.get(el).val=val;
	}
	
	public T getHigher(){
		ArrayList<Element<T>> toSort = new ArrayList<Element<T>>(listToCompare.values());
		Collections.sort(toSort);
		return toSort.get(toSort.size()-1).el;
	}
	
}
