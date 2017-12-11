package lxctools;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.PriorityQueue;

/**
 *  fixed capacity Minimum PriorityQueue.
 * @author luoxiongcai
 *
 * @param <E>
 */
public class FixedMinPQ<E extends Comparable<E>> implements Iterable<E> {

	private PriorityQueue<E> pq ;
	private final int capacity ;
	private final Comparator<E> comparator;
	public FixedMinPQ(int capacity){
		this.capacity = capacity; 
		this.comparator = new Comparator<E>(){
			public int compare(E o1, E o2) {
				return o2.compareTo(o1);
			}
		};
		this.pq = new PriorityQueue<E>(capacity, this.comparator);
	}
	
	public void offer(E e){
		if (pq.size() < this.capacity){
			pq.offer(e);
			return;
		}else if (pq.peek().compareTo(e) > 0){
			pq.poll();
			pq.offer(e);
		}
		
	}
	
	@Override
	public Iterator<E> iterator() {
		return new FixedMinPQIterator<E>(pq);
	}

	private static class FixedMinPQIterator<E> implements Iterator<E>{
		private PriorityQueue<E> backup;
		
		public FixedMinPQIterator(PriorityQueue<E> pq){
			this.backup = new PriorityQueue<E>(pq);
		}
		@Override
		public boolean hasNext() {
			return !backup.isEmpty();
		}

		@Override
		public E next() {
			return backup.poll();
		}

		@Override
		public void remove() {
			
		}
		
	}
	
	/**
	 * get sorted data.
	 * @return
	 */
	public List<E> sortedElement(){
		List<E> sorted = new ArrayList<E>(pq);
		Collections.sort(sorted);
		return sorted;
	}

	public static void main(String[] args) {
		FixedMinPQ<String> minp = new FixedMinPQ<String>(2);
		minp.offer("luo");
		minp.offer("liu");
		minp.offer("zhang");
		for (String s:minp){
			System.out.println(s);
		}
		
		for (String s : minp.sortedElement()){
			System.out.println(s);
		}
	}

}
