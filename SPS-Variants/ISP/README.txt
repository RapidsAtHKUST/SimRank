The main function is in src\exec\computeSimrank.java

It needs two parameters:
Config.stopRea (h in our paper) - small threshold to discard small enough score
Config.depth (l in our paper) - max length of path in consideration

In computeSimrank.java, you should also specify the following files:

String nodeFile 
String edgeFile 
String queryFile 
String resultFile  