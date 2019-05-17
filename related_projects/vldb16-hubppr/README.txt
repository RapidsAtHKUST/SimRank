# HubPPR

Lib requirement: Boost version>=boost1.55 (tested on Ubuntu)



The following manual assumes that you are in the code repository.

Pre-compile: type cmake in terminal, 
$cmake . 

Compile: type make in terminal, i.e.,
$make

Allow the test scripts to run 
$chmod +x *.sh 


Dataset format: 
Assume that we have a folder ./dataset which stores all the testing datatsets.
Each dataset will include an own folder in this directory. For instance, the dblp dataset will be in a folder named ./dataset/dblp/, and includes two files: attribute.txt, graph.txt.
The ./dataset/dblp/attribute.txt file includes the information of the node and edge:
----begin of file-----
n=613586
m=3980318
----end of file-----

The dataset/dblp/graph.txt file includes the information of each edge in the form of <source target>. For instance, the first three lines of dataset/dblp/graph.txt includes:
----begin of file-----
0 1
0 2
0 3
...

----end of file-----
indicating that there are three edges starting from node 0, and ending at 1, 2, and 3 respectively

##Generate query
./generate-query.sh dataset-name dataset-dir
For instance, to generate query for dblp dataset, we then run the script as ./generate-query.sh dblp ./dataset/


## Hub selection
./select-hub.sh dataset-name  dataset-dir
For instance, to geenrate hubs for dblp dataset, we then run the script as ./select-hub.sh dblp ./dataset/


## Build index for HubPPR

./build-index.sh dataset-name dataset-dir

##Run p2p query with HubPPR 
./run-hubppr.sh dataset-name dataset-dir

##Run top-k query with HubPPR
./topk-hubppr.sh dataset-name dataset-dir k_size T_size

For instance, to test on dblp with k=8 and target set size |T| = 400, we run the following command:
./topk-hubppr.sh dblp ./dataset 8 400





