#ifndef __TKDE17_SIM_H__
#define __TKDE17_SIM_H__

#include <ctime>
#include <cmath>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <queue>
#include <random>
#include <algorithm>
#include <iterator>
#include <exception>

#include <boost/format.hpp>

#include <Eigen/Dense>
#include <Eigen/src/Core/Dot.h>
#include <Eigen/Sparse>

#include "util/graph_ywangby.h"
#include "extern_include.h"

using namespace Eigen;
using namespace std;
typedef Triplet<float> Trip;

void tkde17_sim(DirectedG &g, float c, float epsilon);

int tkde17_sim();

struct LinearSystemSim {
    size_t maxIters = 100;
    float epsilon;
    float c;
    size_t n; // size of the grpah matrix
    double cpu_time;
    double mem_size;
    string g_name; // the graph name
    MatrixXf sim; // the simrank matrix

    LinearSystemSim() {} // the default constructor
    LinearSystemSim(string name, const DirectedG &g, float c_, float epsilon_);

    // the major function for solving the linear system
    void run(const DirectedG &g);

    void Ax(SparseMatrix<float> &P, MatrixXf &x, MatrixXf &result);

    string get_file_path() { // file path to save and load
        mkdir_if_not_exist(TKDE17_RESULT_DIR);
        return TKDE17_RESULT_DIR + str(format("%s-%.3f-%.6f") % g_name % c % epsilon);
    }

    void save();

    void load();
};

#endif
