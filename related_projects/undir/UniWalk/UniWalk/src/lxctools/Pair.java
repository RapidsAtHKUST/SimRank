package lxctools;
/**
 * Pair for index and value
 * or key value
 * it's compared by the value.
 * @author luoxiongcai
 *
 * @param <E>
 * @param <T>
 */
public class Pair<K extends Comparable<K>,V extends Comparable<V>> implements Comparable<Pair<K,V>>{
	private K key;
	private V value;
	public Pair(K key, V value){
		this.key = key;
		this.value = value;
	}
	
	
	public K getKey() {
		return key;
	}



	public void setKey(K key) {
		this.key = key;
	}



	public V getValue() {
		return value;
	}



	public void setValue(V value) {
		this.value = value;
	}



	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((key == null) ? 0 : key.hashCode());
		result = prime * result + ((value == null) ? 0 : value.hashCode());
		return result;
	}



	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		Pair<K, V> other = (Pair<K,V>) obj;
		if (key == null) {
			if (other.key != null)
				return false;
		} else if (!key.equals(other.key))
			return false;
		if (value == null) {
			if (other.value != null)
				return false;
		} else if (!value.equals(other.value))
			return false;
		return true;
	}

	@Override
	public int compareTo(Pair<K, V> o) {
		return this.value.compareTo(o.value);
	}
	
	

	@Override
	public String toString() {
		return "Pair [key=" + key + ", value=" + value + "]";
	}


	/**
	 * @param args
	 */
	public static void main(String[] args) {
		
		Pair<Integer,Double> p1 = new Pair<Integer,Double>(1,3.4);
		Pair<Integer,Double> p2 = new Pair<Integer,Double>(1,3.04);
		System.out.println(p1.compareTo(p2));
	}




}
