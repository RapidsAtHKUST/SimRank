package utils;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import conf.MyConfiguration;
import dao.Dao;
import lxctools.FixedCacheMap;
import lxctools.FixedHashMap;
import lxctools.FixedMaxPQ;
import lxctools.Pair;

/**
 * print tools
 * 
 * @author luoxiongcai
 * 
 */
public class Print {
	private static final Dao dao = Dao.getInstance();
	/**
	 * @param sim
	 * @param outPath
	 * @param topk
	 * @throws IOException
	 */
	public static void printByOrder(double[][] sim, String outPath, int topk, int testTopK)
			throws IOException {
		BufferedWriter out = new BufferedWriter(new FileWriter(outPath));
		BufferedWriter outsim = new BufferedWriter(new FileWriter(outPath
				+ ".sim.txt"));
		for (int v = 0; v < sim.length; v++) {
			FixedMaxPQ<Pair<Integer, Double>> maxpq = new FixedMaxPQ<Pair<Integer, Double>>(
					topk); 
			double max = 0;
			for (int i = 0; i < sim[0].length; i++) {
				max = Math.max(sim[v][i], max);
				maxpq.offer(new Pair<Integer, Double>(i, sim[v][i]));
			}
//			if (max < MyConfiguration.MIN){  // don't report too small results.
//				out.append(v + "\r\n");
//				outsim.append(v + "\r\n");
//				continue;
//			}
			out.append(v + "");
			outsim.append(v + "");
			for (Pair<Integer, Double> p : maxpq.sortedElement()) {
				out.append(MyConfiguration.SEPARATOR + p.getKey());
				outsim.append(MyConfiguration.SEPARATOR + p.getKey()
						+ MyConfiguration.SEPARATOR_KV
						+ String.format("%f", p.getValue()));
			}
			out.append("\r\n");
			outsim.append("\r\n");
		}
		out.close();
		outsim.close();
	}

	
	/**
	 * 
	 * @param sim
	 * @param outPath
	 * @param topk
	 * @throws IOException
	 */
	public static void printByOrder(FixedCacheMap[] sim,
			String outPath, int topk) throws IOException {
		BufferedWriter out = new BufferedWriter(new FileWriter(outPath));
		BufferedWriter outsim = new BufferedWriter(new FileWriter(outPath
				+ ".sim.txt"));
		for (int v = 0; v < sim.length; v++) {
//		for (int v = 0; v < 1000; v++) {	
//			System.out.println("v = " + v);
			int size = sim[v].size();
//			if (size == 0)
//				continue;
			out.append(v + "");
			outsim.append(v + "");
			int i = 0;
			
			for (Pair<Integer, Float> p : sim[v]) {
				if (i < size - topk){
					i++;
					continue;
				}
				int key = p.getKey();
				float value = p.getValue();
				out.append(MyConfiguration.SEPARATOR + key);
				outsim.append(MyConfiguration.SEPARATOR + key
						+ MyConfiguration.SEPARATOR_KV
						+ String.format("%f", value));
			}
			out.append("\r\n");
			outsim.append("\r\n");
		}
		out.close();
		outsim.close();
	}
	
	/**
	 * 
	 * @param sim
	 * @param outPath
	 * @param topk
	 * @throws IOException
	 */
	public static void printByOrder(FixedHashMap[] sim,
			String outPath, int topk) throws IOException {
		BufferedWriter out = new BufferedWriter(new FileWriter(outPath));
		BufferedWriter outsim = new BufferedWriter(new FileWriter(outPath
				+ ".sim.txt"));
		for (int v = 0; v < sim.length; v++) {

			out.append(v + "");
			outsim.append(v + "");
			int i = 0;
			for (Pair<Integer, Double> p : sim[v]) {
				if (i >= topk)
					break;
				out.append(MyConfiguration.SEPARATOR + p.getKey());
				outsim.append(MyConfiguration.SEPARATOR + p.getKey()
						+ MyConfiguration.SEPARATOR_KV
						+ String.format("%.6f", p.getValue()));
				i++;
			}
			out.append("\r\n");
			outsim.append("\r\n");
		}
		out.close();
		outsim.close();
	}

	/**
	 * save the results directly to the database;
	 * 
	 * @param tableName
	 * @param sim
	 * @param topk
	 */
	public static void saveToDatabase(String tableName, double[][] sim, int topk) {
		for (int v = 0; v < sim.length; v++) {
			FixedMaxPQ<Pair<Integer, Double>> maxpq = new FixedMaxPQ<Pair<Integer, Double>>(
					topk);
			for (int i = 0; i < sim[0].length; i++) {
				maxpq.offer(new Pair<Integer, Double>(i, sim[v][i]));
			}
			for (Pair<Integer, Double> p : maxpq.sortedElement()) {
				dao.insertOneSim(tableName, v, p.getKey(), p.getValue());
			}
		}
	}

	/**
	 * dump similarity file to the database.
	 * 
	 * @param inputFile
	 * @param tableName
	 * @throws IOException
	 */
	public static void simFileToDatabase(String inputFile, String tableName)
			throws IOException {
		BufferedReader input = new BufferedReader(new FileReader(inputFile));
		String line = null;
		while ((line = input.readLine()) != null) {
			String[] tokens = line.split(MyConfiguration.SEPARATOR);
			int src = Integer.valueOf(tokens[0]);
			for (int i = 1; i < tokens.length; i++) {
				String[] idsim = tokens[i].split(MyConfiguration.SEPARATOR_KV);
				dao.insertOneSim(tableName, src, Integer.valueOf(idsim[0]),
						Double.valueOf(idsim[1]));
			}
		}
		input.close();
	}

	/**
	 * dump edge file to the database;
	 * 
	 * @param inputFile
	 * @param tableName
	 * @throws NumberFormatException
	 * @throws IOException
	 */
	public static void edgeFileToDatabase(String inputFile, String tableName)
			throws NumberFormatException, IOException {
		System.out.println("deleting database: "+ tableName);
		dao.clear(tableName);
		System.out.println("writing to database...");
		BufferedReader input = new BufferedReader(new FileReader(inputFile));
		String line = null;
		while ((line = input.readLine()) != null) {
			String[] tokens = line.split(MyConfiguration.SEPARATOR);
			int src = Integer.valueOf(tokens[0]);
			int des = Integer.valueOf(tokens[1]);
			dao.insertOneEdge(tableName, src, des);
		}
		input.close();
		System.out.println("write to database done!");
	}

	public static void main(String[] args) throws NumberFormatException, IOException {
	}

}
