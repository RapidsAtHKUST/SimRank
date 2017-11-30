package lxctools;

import java.util.Arrays;

public class LxcArrays {
	public static <T>  void reverse(T [] array){
		assert(array != null);
		for (int i =0 ; i < array.length / 2; i++){
			swap(array, i, array.length - 1 - i);
		}
	}
	
	public static <T>void swap(T[] A, int src, int des){
		T temp = A[src];
		A[src] = A[des];
		A[des] = temp;
	}
	
	public static  void reverse(int [] array){
		assert(array != null);
		for (int i =0 ; i < array.length / 2; i++){
			swap(array, i, array.length - 1 - i);
		}
	}
	
	public static void swap(int[] A, int src, int des){
		int temp = A[src];
		A[src] = A[des];
		A[des] = temp;
	}
	
	public static  void reverse(double [] array){
		assert(array != null);
		for (int i =0 ; i < array.length / 2; i++){
			swap(array, i, array.length - 1 - i);
		}
	}
	
	public static void swap(double[] A, int src, int des){
		double temp = A[src];
		A[src] = A[des];
		A[des] = temp;
	}
	
	/**
	 * A should be sorted in ascending order.
	 * find the index of the first element, which is larger or equal to the given target.
	 * if target is larger than any element, than you will get A.length as the result.
	 * @param A
	 * @param target
	 * @return
	 */
	public static int insertPoint(double[] A, double target){
		int L = 0, R = A.length - 1;
		while (L < R){
			int mid = (L + R) / 2;
			if (A[mid] < target)
				L = mid + 1;
			else R = mid;
		}
		return A[L] < target ? L + 1: L;
	}
	
	
	public static void main(String[] args) {
		double[] A = {0.2,0.4,0.4,0.4,0.6,1};
		int r = insertPoint(A, 0.5);
		System.out.println(r);
	}
 
}
