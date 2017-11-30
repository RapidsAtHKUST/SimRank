package lxctools;

import java.util.HashMap;
import java.util.Iterator;

/**
 * cache ,but delete the <k,v> with the minimum  v, not by LRU.
 * be careful! when use iterator, the queue will be empty. because we use delMin not copy!
 * besides! it's in ascend order by value!
 * @author luoxiongcai
 *
 */

public class FixedCacheMap implements Iterable<Pair<Integer,Float>> {
    private int NMAX;        // maximum number of elements on PQ
    private int N;           // number of elements on PQ
    private HashMap<Integer, Integer> key2Index; // "Short" to save storage.
    private int[] keys;        // binary heap using 1-based indexing  keys[i] = k, values[i] = v 
    private float[] values;    
    /**
     * Initializes an empty indexed priority queue with indices between 0 and NMAX-1.
     * @param NMAX the keys on the priority queue are index from 0 to NMAX-1
     * @throws java.lang.IllegalArgumentException if NMAX < 0
     */
    public FixedCacheMap(int NMAX) {
        this.NMAX = NMAX;
        values = new float[NMAX + 1];    
        keys   = new int[NMAX + 1];
        key2Index = new HashMap<Integer, Integer>();
    }

    public void put(int key, float value){
    	Integer index = key2Index.get(key);
    	if (index != null){
    		values[index.intValue()] += value;
    		sink(index);
    	} else if (N < NMAX){ // not full
    		N++;
    		keys[N] = key;
    		values[N] = value;
    		key2Index.put(key, (Integer) N);
    		swim(N);
    	} else if (value > getMinValue()){ // substitue the minvalue and sink.
    		key2Index.remove(keys[1]);
    		keys[1] = key;
    		values[1] = value;
    		key2Index.put(key, (Integer)1);
    		sink(1);
    	}
    }
    
    public int size(){
    	return N;
    }
    
    public boolean isEmpty(){
    	return N == 0;
    }
    

    private void sink(int index){
    	while (2 * index <= N){
    		int j = 2 * index;
    		if ( j < N && greater(j, j + 1)) j++;
    		if (!greater(index, j)) break;
    		exch(index, j);
    		index = j;
    	}
    }
    

    
    private void swim(int index){
    	while ( index > 1 && greater(index / 2, index)){
    		exch(index, index / 2);
    		index  = index / 2;
    	}
    }
    

    
    private boolean greater(int index1 , int index2){
    	return values[index1] > values[index2];
    }
    
    private void exch(int index1, int index2){
    	//do not forget to modify key2Index.
    	key2Index.put(keys[index1], (Integer)index2);
    	key2Index.put(keys[index2], (Integer)index1);
    	int tempK = keys[index1];
    	keys[index1] = keys[index2];
    	keys[index2] = tempK;
    	float tempV = values[index1];
    	values[index1] = values[index2];
    	values[index2] = tempV;
    	
    	
    }
    
    private float getMinValue(){
    	return values[1];
    }
    
    private Pair<Integer,Float> delMin(){
    	Pair<Integer,Float> min = new Pair<Integer,Float>(keys[1],values[1]);
    	key2Index.remove(keys[1]);
    	exch(1, N--);
    	sink(1);
    	return min;
    }

	@Override
	public Iterator<Pair<Integer, Float>> iterator() {
		return new FixedCacheMapIte();
	}
	
	private class FixedCacheMapIte implements Iterator<Pair<Integer, Float>>{
		
		@Override
		public boolean hasNext() {
			return N > 0;
		}

		@Override
		public Pair<Integer, Float> next() {
			return delMin();
		}

		@Override
		public void remove() {
		}
	} 
	
	public static void main(String[] args){
		
		FixedCacheMap fc = new FixedCacheMap(3);
		fc.put(1, 0.5f);
		fc.put(2, 3f);
		fc.put(3, 0.1f);
		fc.put(4, 8f);
		fc.put(4, 8f);
		fc.put(1, 0.6f);
		fc.put(5, 1f);
		System.out.println(fc.size());
		for (Pair<Integer, Float> p : fc){
			System.out.println(p.getKey()+"\t"+p.getValue());
		}
		System.out.println(fc.size());
	}
 
}
