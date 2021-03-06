//
// Created by yche on 12/22/17.
//

#ifndef SPS_GRAPH_YCHE_H
#define SPS_GRAPH_YCHE_H

#include <sys/stat.h>

#include <cstdint>

#include <string>
#include <vector>
#include <random>

#include "log.h"
#include "random_utils.h"

#ifdef SFMT

#include "sfmt_based_rand.h"

#endif

using namespace std;

class GraphYche {
    // bidirectional csr (compressed sparse row representation)
public:
    int n, m;
    vector<int> off_in;
    vector<int> in_deg_arr;
    vector<int> neighbors_in;

    vector<int> off_out;
    vector<int> out_deg_arr;
    vector<int> neighbors_out;
private:
    vector<pair<int, int>> GetEdgeList(string &file_path);

    void LoadGraph(vector<pair<int, int>> &edge_lst);

    bool BinarySearch(uint32_t offset_beg, uint32_t offset_end, int val);

public:
    explicit GraphYche(string &graph_path);

    int in_degree(int u);

    int out_degree(int u);

    bool exists_edge(int src, int dst);

};

// utility function
inline bool file_exists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

inline string get_edge_list_path(string s) {
    // get file location of edgelist for graph s
    return "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + s + ".txt";
}


inline string get_bin_list_path(string s) {
    // get file location of edgelist for graphs
    return "/homes/ywangby/workspace/LinsysSimRank/datasets/bin_edge_list/" + s + ".bin";
}

inline string get_bin_list_path_from_txt(string path) {
    auto tmp = path.replace(path.begin() + 48, path.begin() + 57, "bin_edge_list");
    auto tmp2 = tmp.replace(tmp.end() - 3, tmp.end(), "bin");
    return tmp2;
}

#if !defined(SFMT)
extern int sample_in_neighbor(int a, GraphYche &g);
#else

inline extern int sample_in_neighbor(int a, GraphYche &g, SFMTRand &sfmt_rand_gen) {
    auto in_deg = g.in_deg_arr[a];
    if (in_deg > 0) {
        return g.neighbors_in[select_randomly_sfmt(g.off_in[a], g.off_in[a + 1], sfmt_rand_gen)];
    } else {
        return -1;
    }
}

#endif

#endif //SPS_GRAPH_YCHE_H
