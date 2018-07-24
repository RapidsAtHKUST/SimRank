This is the source code of the VLDB18 paper "ProbeSim: Scalable Single Source and Top-k SimRank Computations on Dynamic Graphs".
For any question of the code, please feel free to contact me. (Yu Liu, foreverhellas@163.com)
How to compile:
sfmt_dir=directory of SFMT
g++ -march=core2 -pthread -std=c++11 -DSFMT_MEXP=607 -I $sfmt_dir -O3 -o ProbeSim sfmt_dir/SFMT.c main.cpp
How to run:
./ProbeSim [-g <graph name>] [-e <eps, default 0.1>] [-q <query set>] [-d <output dir>]