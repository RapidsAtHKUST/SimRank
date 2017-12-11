package lxctools;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

public class FixedHashMap implements Iterable<Pair<Integer,Double>>{

	private HashMap<Integer, Double> maps;
	private int capacity;
	public FixedHashMap(int capacity){
		maps = new HashMap<Integer, Double>();
		this.capacity = capacity;
	}
	@Override
	public Iterator<Pair<Integer, Double>> iterator() {
		return new FixedIterator(maps);
	}
	
	public void add(int key, double value){
		Double old = maps.get(key);
		if (old != null){
			maps.put(key, value+old);
		} else if (maps.size() < capacity){
			maps.put(key, value);
		}
	}
	
	// songjs
	public int size(){
		return maps.size();
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("size: "+maps.size()+"");
		for (Pair<Integer,Double> pair : this){
			sb.append("\t"+pair.getKey()+"->"+pair.getValue());
		}
		sb.append("\r\n");
		return sb.toString();
	}


	private class FixedIterator implements Iterator<Pair<Integer, Double>>{
		private List<Entry<Integer,Double>> entrys;
		private int ptr = 0;
		
		public FixedIterator(HashMap<Integer, Double> maps){
			entrys = new ArrayList<Entry<Integer,Double>>(maps.entrySet());
			Collections.sort(entrys, new Comparator<Entry<Integer,Double>>(){

				@Override
				public int compare(Entry<Integer, Double> o1,
						Entry<Integer, Double> o2) {
					return o2.getValue().compareTo(o1.getValue());
				}
				
			});
		}
		@Override
		public boolean hasNext() {
			return ptr < entrys.size();
		}

		@Override
		public Pair<Integer, Double> next() {
			Entry<Integer,Double> entry = entrys.get(ptr++);
			return new Pair<Integer, Double>(entry.getKey(), entry.getValue());
		}

		@Override
		public void remove() {
			
		}
		
	}
	
	
	
	public static void main(String[] args) {
		FixedHashMap m = new FixedHashMap(3);
		m.add(4, 7.0);
		m.add(1, 2.0);
		m.add(2, 3.0);
		m.add(3, 5.0);
		m.add(1, 2.0);
		
		for (Pair<Integer, Double> p : m){
			System.out.println(p.getKey()+"\t"+p.getValue());
		}
	}

}
