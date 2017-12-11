package dao;

import java.util.List;

import conf.MyConfiguration;

public class DaoTest {
 
	/**
	 * @param args
	 */
	private static Dao dao;
	public static void main(String[] args) {
		dao= Dao.getInstance();
		boolean flag = dao.insertOneEdge("edges", 100000, 100000);
		
	}

}
