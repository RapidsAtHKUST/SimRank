#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <sys/stat.h>

#include <cstdint>
#include <cstdio>
#include <chrono>  // for high_resolution_clock

#include <random>

#include <boost/graph/adjacency_list.hpp>

#include "../util/stat.h"
#include "../util/sfmt_based_rand.h"
#include "../util/file_serialization.h"
#include "../util/sparse_matrix_utils.h"

// utility function
inline bool file_exists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

// graph data
extern string EDGE_LIST_PATH;
extern vector<string> DATA_NAME;

// types definition
using DirectedG = boost::adjacency_list<vecS, vecS, bidirectionalS>;

// graph functions
extern void load_graph(string path, DirectedG &g);

extern void show_graph(DirectedG &g);

extern string get_edge_list_path(string s);

extern void indegree_mat(const DirectedG &g, SparseMatrix<float> &P);// column normalized adjacency matrix

// sampling related
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

#if !defined(SFMT)
extern int sample_in_neighbor(int a, DirectedG &g);
#else

extern int sample_in_neighbor(int a, DirectedG &g, SFMTRand &sfmt_rand_gen);

#endif

#endif
