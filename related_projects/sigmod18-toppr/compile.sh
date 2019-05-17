g++ -march=core2 -ffast-math -use_fast_math -pthread -std=c++11 -DSFMT_MEXP=607 -I SFMT-src-1.4.1/ -O3 -o TopPPR SFMT.c main.cpp 
