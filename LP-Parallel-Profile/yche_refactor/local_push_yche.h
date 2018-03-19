#ifndef __LOCAL_PUSH_H__
#define __LOCAL_PUSH_H__

#include <cstdio>
#include <cmath>

#include <iostream>
#include <utility>
#include <stack>
#include <queue>

#include <boost/graph/adjacency_list.hpp>
#include <boost/format.hpp>

#include <sparsepp/spp.h>

#include "../util/graph_yche.h"
#include "../util/sparse_matrix_utils.h"

using boost::format;
using PairMarker= sparse_hash_map<NodePair, bool>;

const string LOCAL_PUSH_DIR = "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/datasets/local_push/";

extern double cal_rmax(GraphYche &g, double c, double epsilon, double delta); //calculate r_,max

extern double cal_rmax(double c, double epsilon); // the r_max for general case

extern bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full);

/* base class of local push */
struct LocalPush {
public:
    /* data members */
    string g_name;                  // the name of graph data

    DensePairMap<float> P;          // the estimates
    DensePairMap<float> R;          // the residuals
    queue<NodePair> Q;              // the queue to hold invalid node pairs
    DensePairMap<bool> marker;

    double r_max;
    double c;
    size_t n;               // we need to define total number of nodes in advance
    unsigned int n_push;    // record number of push
    double cpu_time;        // cpu_time for local push
    double epsilon;         // the error bound
    int mem_size;           // the memory useage

public:
    // methods
    LocalPush() = default;

    // constructor, init members
    LocalPush(GraphYche &, string, double c, double epsilon, size_t);

    virtual string get_file_path_base() { return string(); } // get file path of local push data

    void init_PR(); // init or load the data of P and R
public:
    virtual double how_much_residual_to_push(GraphYche &g, NodePair &np) {}

    virtual void push(NodePair &pab, double inc) {};

    virtual void push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {}

    void local_push(GraphYche &g); // empty function for local push
public:
    void save();

    void load();

    virtual double query_P(int a, int b) { return 0; }

    double query_R(int a, int b);
};

/*local push using reduced system*/
struct Reduced_LocalPush : LocalPush {
    Reduced_LocalPush() = default;

    Reduced_LocalPush(GraphYche &g, string name, double c_, double epsilon, size_t n_);

    string get_file_path_base() override; // get file path of local push data
public:
    double how_much_residual_to_push(GraphYche &g, NodePair &np) override;

    void push(NodePair &pab, double) override;

    void push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) override;

public:
    double query_P(int a, int b) override;
};

/* local push using full system*/
struct Full_LocalPush : LocalPush {
    Full_LocalPush() = default;

    Full_LocalPush(GraphYche &g, string name, double c_, double epsilon, size_t n_);

    string get_file_path_base() override; // get file path of local push data

public:
    double how_much_residual_to_push(GraphYche &g, NodePair &np) override;

    void push(NodePair &pab, double) override;

    void push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) override;

public:
    double query_P(int a, int b) override;
};

#endif