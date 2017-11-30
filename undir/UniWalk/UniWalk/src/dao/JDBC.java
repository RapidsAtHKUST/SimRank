package dao;

import java.sql.*;

public class JDBC {
 
	private static final String DRIVERCLASS = "com.microsoft.sqlserver.jdbc.SQLServerDriver";
	private static final String URL = "jdbc:sqlserver://localhost:1433;DatabaseName=simrank";

	private static final String USERNAME = "sa";

	private static final String PASSWORD ="1638"; 

	private static final ThreadLocal<Connection> threadLocal = new ThreadLocal<Connection>();

	static {
		try {
			Class.forName(DRIVERCLASS).newInstance();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static Connection getConnection() {
		Connection conn = threadLocal.get();
		if (conn == null) {
			try {
				conn = DriverManager.getConnection(URL, USERNAME, PASSWORD);
				threadLocal.set(conn);
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}
		return conn;
	}

	public static boolean closeConnection() { 
		boolean isClosed = true;
		Connection conn = threadLocal.get(); 
		threadLocal.set(null); 
		if (conn != null) { 
			try {
				conn.close(); 
			} catch (SQLException e) {
				isClosed = false;
				e.printStackTrace();
			}
		}
		return isClosed;
	}

}
