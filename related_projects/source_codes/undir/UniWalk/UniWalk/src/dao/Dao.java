package dao;

import java.util.ArrayList;
import java.util.List;

public class Dao extends BaseDao {

	private static Dao dao;
 
	static {
		dao = new Dao();
	}
 
	public static Dao getInstance() {
		return dao;
	}
	/****************************************** query *********************************/
	
	public int count(String tableName){
		String sql = " SELECT COUNT(*) FROM " + tableName ;
		return (Integer) selectOnlyValue(sql);
	}
	
	public List<List<Object>> getEdges(String tableName){
		String sql = " SELECT * FROM " + tableName;
		List<List<Object>> edges =  selectSomeNote(sql);
		return edges;
	}
	
	/****************************************** insert *********************************/
	public boolean insertOneEdge(String tableName, int src, int des){
		String sql = "INSERT INTO " + tableName + " VALUES(" + src +"," + des + ")" ;
		return longHaul(sql);
	}
	
	
	public boolean insertOneSim(String tableName,int src, int des, double sim){
		String sql = "INSERT INTO " + tableName+" VALUES(" + src +"," + des +"," +sim + ")" ;
		return longHaul(sql);
	}
	
	/**
	 * batch insert edges. 
	 * @param edges 
	 * @return
	 */
//	public boolean batchInsertEdges(List<Integer[]> edges){
//		
//	}
	
	/**
	 * you should cast from double to int for ids.
	 * @param tableName
	 * @param edges
	 * @return
	 */
//	public boolean batchInsertSims(String tableName, List<List<Double>> edges){
//		
//	}
	
	
	/****************************************** delete *********************************/
	
	public boolean clear(String tableName){
		String sql = "Truncate TABLE " + tableName;
		return longHaul(sql);
	}
	
	/****************************************** modify *********************************/
	
	/****************************************** alter table *********************************/
	 

}
