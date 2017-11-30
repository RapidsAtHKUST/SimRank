UniWalk
=======
Here, we design a Monte Carlo based method, UniWalk, to enable the fast top-k SimRank computation over large undirected graphs. UniWalk directly locates the top-k similar vertices for any single source vertex u via approximately R sampling paths originating from u only. Related work has been published in ICDE 2017 ("UniWalk: Unidirectional Random Walk Based Scalable SimRank Computation over Large Graph").

Furthermore, in order to overcome the "rectification issue" in the original UniWalk, we design a path enumeration strategy to gain higher precision, which computes SimRank with the path probabilities instead of path frequencies in sampling. We call it the optimized UniWalk here, which is shown in the "UniWalk/src/simrank/SingleRandomDev.java" file(the basic UniWalk is shown in "UniWalk/src/simrank/SingleRandomWalk.java").

### input graph format
```
src  tar
0    1
0    2
1    3
1    4
```
We have added one example graph in "data/real/crime.txt".(They are splited by "\t")

All the output answer will be saved in "output/"(UniWalk in single/, and Opt-UniWalk in singledev/).

### Normal-SimRank
The Normal SimRank is shown in "UniWalk/src/simrank/SimRank.java". With the default configuration("UniWalk/src/conf/Configuration.java"), we can easily get the “real” SimRank scores.

### Basic-UniWalk
The Basic UniWalk is shown in "UniWalk/src/simrank/SingleRandomWalk.java". After you have calculate the “real” SimRank scores, you can run this file to get the basic UniWalk answer, and the precision result compared with the “real” SimRank.


### Opt-UniWalk
The optimized UniWalk is shown in "UniWalk/src/simrank/SingleRandomDev.java". After you have calculate the "real" answer of SimRank, you can run this file to get the Opt-UniWalk results, and the precision compared with the “real” scores.

Note, "SingleRandomWalk_M.java and SingleRandomDev_M.java" are the methods with the M-candidate optimization. When you want to deal with large graphs, you should better use them in case of "Out of Memory" Error. "SingleRandomWalk_R.java and SingleRandomWalk_M_R" are for the optimization 'Path Sharing'.

### Distributed UniWalk on Giraph
To improve the scalability and lower the query response time for multiple sources, we devise a distributed version of UniWalk via a framework Giraph(1.0.0) with Hadoop(0.20.203). The main source codes are shown in "UniWalk/Giraph/". You can directly export all of this project with "Fat jar" into a file "uniwalk.jar", and then use a commond like 
```
$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/uniwalk.jar org.apache.giraph.GiraphRunner giraph.SingleWalkVertex -eif org.apache.giraph.io.formats.IntNullReverseTextEdgeInputFormat -eip /home/junshuai.sjs/simrank/input/crime.txt -ve giraph.RandOutEdges -of giraph.ioformat.SimilaritytVertexOutputFormat -op /home/junshuai.sjs/output/crime -w 8
```
to run it. For a simple test, you can also use "data/real/crime.txt" as the input graph.

### Reference
1.PaRMAT to create test all the Powerlaw and Random graphs. https://github.com/farkhor/PaRMAT/

2.We use Giraph 1.0.0 based on Hadoop 0.20.203. 

3.You should better use "Fat jar" on eclipse to pack this project.

4.In addition, we have already submitted a single version of UniWalk on Code Ocean: https://codeocean.com/algorithm/662d773f-f10c-4eff-aac5-3e2bb0467414/code, which will be automatically made public as soon as the linked article(UniWalk) is published, and it is another choose for readers to have a quick start.
