#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <sys/stat.h>

#include <cstdint>
#include <cstdio>
#include <chrono>  // for high_resolution_clock

#include <iostream>
#include <functional>
#include <random>
#include <algorithm>
#include <utility>
#include <fstream>

#include <boost/format.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/multi_array.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <sparsepp/spp.h>
#include <google/dense_hash_map>

#include <util/graph_yche.h>
#include "util/file_serialization.h"
#include "util/stat.h"
#include "util/sparse_matrix_utils.h"
#include "random_utils.h"

using namespace boost;
using namespace std;
using namespace Eigen;
using spp::sparse_hash_map;
using spp::sparse_hash_set;
using google::dense_hash_map;

//inline bool file_exists(const std::string &name) {
//    struct stat buffer;
//    return (stat(name.c_str(), &buffer) == 0);
//}
//
//inline string get_edge_list_path(string s) {
//    // get file location of edgelist for graph s
//    return "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + s + ".txt";
//}

// types definition
typedef boost::adjacency_list<vecS, vecS, bidirectionalS> DirectedG;

// graph function
extern void load_graph(string path, DirectedG &g);

extern void show_graph(DirectedG &g);

extern string get_edge_list_path(string s);

extern void indegree_mat(const DirectedG &g, SparseMatrix<float> &P);// column normalized adjacency matrix

// sample in-neighbor
extern int sample_in_neighbor(int a, DirectedG &g);

#endif
