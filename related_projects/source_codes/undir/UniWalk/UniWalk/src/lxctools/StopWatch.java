package lxctools;
/**
 * a single-thread stop watch;
 * @author luoxiongcai
 *
 */
public class StopWatch {
	private static long stime;
	
	public static void start(){
		stime = System.currentTimeMillis();
	}
	
	public static void say(String msg){
		System.out.println(msg+"\t used time: "+(System.currentTimeMillis() - stime)/1000.0+"s");
	}
	
	public static void main(String[] args) {
		
		StopWatch.start();
		StopWatch.say("hello");
	}

}
