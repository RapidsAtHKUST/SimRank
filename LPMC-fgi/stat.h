#ifndef __STAT_H__
#define __STAT_H__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
// #include "graph.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <chrono>

#include <util/graph_yche.h>
#include <util/sfmt_based_rand.h>

using namespace std;
using namespace Eigen;
typedef GraphYche DirectedG;


using NodePair= pair<unsigned int, unsigned int>;

extern int parseLine(char *line);

extern int getValue();

extern double random_01(); // uniform distribution of 0-1

extern int random_int(int lower, int upper);// generate random number between [lower, upper]

struct LinearRegression{
    // simple implementation multiple linear regression 
    VectorXd W;
    int N; // number of samples
    int P; // number of features
    LinearRegression(int _N, int _P){
        N = _N;
        P = _P;
    }
    void fit(vector<vector<double>> & X, vector<double> &Y); // fit the data and compute the parameters
    double predict(vector<double> &); // predict the y given x
};


// help function for argsort, in decreasing order
template <typename T>
void  sort_indexes(const vector<T> &v, vector<size_t>& idx) {

  // initialize original index locations
  idx.resize(v.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

}

// help function of sample a (1-c) walk return the indicator 0/1
extern int sample_an_1c_walk(NodePair np, DirectedG &g, int length, SFMTRand & rand_gen);

extern void display_seperate_line();

#endif

