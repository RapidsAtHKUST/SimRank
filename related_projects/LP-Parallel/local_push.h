#ifndef __LOCAL_PUSH_H__
#define __LOCAL_PUSH_H__

#include <cstdio>
#include <cmath>

#include <omp.h>

#include <iostream>
#include <utility>
#include <stack>
#include <queue>

#include <boost/graph/adjacency_list.hpp>
#include <boost/format.hpp>

#include <sparsepp/spp.h>

#include "graph.h"

using boost::format;
const string LOCAL_PUSH_DIR = "./datasets/local_push/";

extern double cal_rmax(double c, double epsilon); // the r_max for general case

/* base class of local push */
struct LocalPush {
    /* data members */
    string g_name; // the name of graph data

    DensePairMap<float> P; // the estimates
    DensePairMap<float> R; // the residuals
    queue<NodePair> Q; // the queue to hold invalid node pairs
    DensePairMap<bool> marker;

    double r_max;
    double c;
    size_t n;// we need to define total number of nodes in advance
    unsigned int n_push; // record number of push
    double cpu_time;// cpu_time for local push
    double epsilon; // the error bound
    int mem_size;// the memory useage

public:
    LocalPush() = default;

    LocalPush(DirectedG &, string, double c, double epsilon, size_t);

    virtual void local_push(DirectedG &g); // empty funciton for local push

    virtual double how_much_residual_to_push(DirectedG &g, NodePair &np) {}

    virtual void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) {}

    void save();

    void load();

    void init_PR(); // init or load the data of P and R

    virtual string get_file_path_base() { return string(); } // get file path of local push data

    void show(); // print values

    virtual void push(NodePair &pab, double inc) {}

private:
    virtual void update_residual(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {}

public:
    void insert(DirectedG::vertex_descriptor, DirectedG::vertex_descriptor, DirectedG &g);

    void remove(DirectedG::vertex_descriptor, DirectedG::vertex_descriptor, DirectedG &g);

public:
    virtual double query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {}

    double query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b);

};

/* local push using reduced system */
struct Reduced_LocalPush : LocalPush {
    Reduced_LocalPush() = default;

    Reduced_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_);

    double how_much_residual_to_push(DirectedG &g, NodePair &np) override;

    void push(NodePair &pab, double) override;

    // LocalPush(g, name, c_,r_max_, n_){} // invode the base constructor
    void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) override;

public:
    /* update the residual score of R[a,b], we can assum a<b*/
    void update_residual(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) override;

    string get_file_path_base() override; // get file path of local push data

    double query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) override;
};

/* local push using the full linear system */
struct Full_LocalPush : LocalPush {
    Full_LocalPush() = default;

    Full_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_);

    void push(NodePair &pab, double) override;

    double how_much_residual_to_push(DirectedG &g, NodePair &np) override;

    void push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) override;

    string get_file_path_base() override; // get file path of local push data

    double query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) override;
};

/* parallel local push for SimRank*/
struct Parallel_LocalPush : LocalPush {
    Parallel_LocalPush() = default;

    Parallel_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_);

    void local_push(DirectedG &g) override; // the parallel local push method
    omp_lock_t *P_lock{}; // locks for P
    omp_lock_t *R_lock{}; // locks for R
    omp_lock_t *M_lock{}; // locks for markers

    ~Parallel_LocalPush() {
        delete[] P_lock;
        delete[] R_lock;
        delete[] M_lock;
    }
};

#endif
