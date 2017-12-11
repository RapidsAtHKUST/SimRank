package dao;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public class BaseDao {

	//query for multiple records
	protected List<List<Object>> selectSomeNote(String sql) {
		List<List<Object>> vector = new ArrayList<List<Object>>();
		Connection conn = JDBC.getConnection();
		try { 
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sql);
			int columnCount = rs.getMetaData().getColumnCount();
			int row = 1;
			while (rs.next()) {
				List<Object> rowV = new ArrayList<Object>();
				rowV.add(new Integer(row++));
				for (int column = 1; column <= columnCount; column++) {
					rowV.add(rs.getObject(column));
				}
				vector.add(rowV);
			}
			rs.close();
			stmt.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}finally{
			JDBC.closeConnection();
		}
		return vector;
	}

	//query for only one record.
	protected List<Object> selectOnlyNote(String sql) {
		List<Object> vector = new ArrayList<Object>();
		
		Connection conn = JDBC.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sql);
			int columnCount = rs.getMetaData().getColumnCount();
			while (rs.next()) {
				vector = new ArrayList<Object>();
				for (int column = 1; column <= columnCount; column++) {
					vector.add(rs.getObject(column));
				}
			}
			rs.close();
			stmt.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}finally{
			JDBC.closeConnection();
		}
		return vector;
	}

	//query for multiple values
	protected List<Object> selectSomeValue(String sql) {
		List<Object> vector = new ArrayList<Object>();
		Connection conn = JDBC.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sql);
			while (rs.next()) {
				vector.add(rs.getObject(1));
			}
			rs.close();
			stmt.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}finally{
			JDBC.closeConnection();
		}
		return vector;
	}

	// query for only a value
	protected Object selectOnlyValue(String sql) {
		Object value = null;
		Connection conn = JDBC.getConnection();
		try {
			Statement stmt = conn.createStatement();
			ResultSet rs = stmt.executeQuery(sql);
			while (rs.next()) {
				value = rs.getObject(1);
			}
			rs.close();
			stmt.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}finally{
			JDBC.closeConnection();
		}
		return value;
	}

	// insert or delete records, alter table and so on.
	public boolean longHaul(String sql) {
		boolean isLongHaul = true;
		Connection conn = JDBC.getConnection();
		try {
			conn.setAutoCommit(false);
			Statement stmt = conn.createStatement();
			stmt.executeUpdate(sql);
			stmt.close();
			conn.commit();
		} catch (SQLException e) {
			isLongHaul = false;
			try {
				conn.rollback();
			} catch (SQLException e1) {
				e1.printStackTrace();
			}
			e.printStackTrace();
		}finally{
			JDBC.closeConnection();
		}
		return isLongHaul;
	}

	
	// batch insert
//	public boolean batchInsert(String sql)
	
}
