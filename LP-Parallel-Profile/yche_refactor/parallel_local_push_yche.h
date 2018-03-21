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

extern double cal_rmax(double c, double epsilon); // the r_max for general case

struct FLPTask {
    int b_;
    float residual_;

    FLPTask(int b, float residual) {
        b_ = b;
        residual_ = residual;
    }
};

struct RLPTask {
    int b;
    float residual;
    bool is_singleton;
};

struct LP {
public:
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
    int mem_size;           // the memory usage

    size_t max_q_size;
public:
    LP(GraphYche &, string, double c, double epsilon, size_t);

    virtual string get_file_path_base() { return string(); } // get file path of local push data

    void init_PR(); // init or load the data of P and R
public:
    virtual double how_much_residual_to_push(GraphYche &g, NodePair &np) {}

    virtual void push(NodePair &pab, double inc) {};

    virtual void push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {}

    virtual void local_push(GraphYche &g); // empty function for local push
public:
    virtual double query_P(int a, int b) { return 0; }

    double query_R(int a, int b);
};

/*local push using reduced system*/
struct PRLP : LP {
    std::unordered_map<int, vector<RLPTask>> tmp_task_hash_table;
public:
    PRLP(GraphYche &g, string name, double c_, double epsilon, size_t n_);

    double query_P(int a, int b) override;

public:
    double how_much_residual_to_push(GraphYche &g, NodePair &np) override;

    void push(NodePair &pab, double) override;

    void push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) override;

    void local_push(GraphYche &g);
};

/* local push using full system*/
struct PFLP : LP {
    vector<vector<int>> thread_local_expansion_set_lst;

    vector<vector<int>> expansion_pair_lst;

public:
    PFLP(GraphYche &g, string name, double c_, double epsilon, size_t n_);

    double query_P(int a, int b) override;

public:
    double how_much_residual_to_push(GraphYche &g, NodePair &np) override;

    void push(NodePair &pab, double) override;

    void push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) override;

    void local_push(GraphYche &g); // empty function for local push
};

#endif