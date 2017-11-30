package lxctools;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;
import java.util.PriorityQueue;
/**
 *  fixed capacity Maximum PriorityQueue.
 * @author luoxiongcai
 *
 * @param <E>
 */
public class FixedMaxPQ<E extends Comparable<E>> implements Iterable<E>{
 
	private PriorityQueue<E> pq ;
	private final int capacity ;
	private final Comparator<E> comparator;
	public FixedMaxPQ(int capacity){
		this.capacity = capacity; 
		this.comparator = new Comparator<E>(){
			public int compare(E o1, E o2) {
				return o1.compareTo(o2);
			}
		};
		this.pq = new PriorityQueue<E>(capacity, this.comparator);
	}
	
	public void offer(E e){
		if (pq.size() < this.capacity){
			pq.offer(e);
			return;
		}else if (pq.peek().compareTo(e) < 0){
			pq.poll();
			pq.offer(e);
		}
		
	}
	
	@Override
	public Iterator<E> iterator() {
		return new FixedMaxPQIterator<E>(pq);
	}

	private static class FixedMaxPQIterator<E> implements Iterator<E>{
		private PriorityQueue<E> backup;
		
		public FixedMaxPQIterator(PriorityQueue<E> pq){
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
	 * get sorted data at descendant order.
	 * @return
	 */
	public List<E> sortedElement(){
		List<E> sorted = new ArrayList<E>(pq);
		Collections.sort(sorted,Collections.reverseOrder());
		return sorted;
	}

	public static void main(String[] args) {
		FixedMaxPQ<String> maxp = new FixedMaxPQ<String>(2);
		maxp.offer("luo");
		maxp.offer("liu");
		maxp.offer("zhang");
		for (String s:maxp){
			System.out.println(s);
		}
		for (String e : maxp.sortedElement())
			System.out.println(e);
	}
}
