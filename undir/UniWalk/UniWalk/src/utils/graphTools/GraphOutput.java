package utils.graphTools;



/**
 * @author Aapo Kyrola, akyrola@cs.cmu.edu, akyrola@twitter.com
 */
public interface GraphOutput {

    void addEdges(int[] from, int[] to);

    void finishUp();

}
