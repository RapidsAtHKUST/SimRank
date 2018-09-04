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
#include <fstream>
#include <iostream>

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
    void LoadGraph(vector<pair<int, int>> &edge_lst);

    bool BinarySearch(uint32_t offset_beg, uint32_t offset_end, int val);

public:
    explicit GraphYche(string &graph_path);

    int in_degree(int u);

    int out_degree(int u);

    bool exists_edge(int src, int dst);
};

vector<pair<int, int>> GetEdgeList(string &file_path);

template<typename T>
void WriteArrToFile(string file_path, vector<T> &arr) {
    std::ofstream ofs(file_path, ios::binary);
    auto arr_size = static_cast<int>(arr.size());
    cout << arr_size << endl;
    auto constexpr TYPE_SIZE = sizeof(T);
    ofs.write(reinterpret_cast<const char *>(&arr_size), TYPE_SIZE);
    ofs.write(reinterpret_cast<const char *>(&arr.front()), arr_size * TYPE_SIZE);
}

template<typename T>
void ReadFileToArr(string file_path, vector<T> &arr) {
    ifstream ifs(file_path, ios::binary);
    int arr_size;
    auto constexpr TYPE_SIZE = sizeof(T);
    ifs.read(reinterpret_cast<char *>(&arr_size), TYPE_SIZE);
    arr.resize(arr_size);
    ifs.read(reinterpret_cast<char *>(&arr.front()), arr_size * TYPE_SIZE);
}

// utility function
inline bool file_exists(std::string name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

inline string get_edge_list_path(string s) {
    // get file location of edgelist for graphs
    return "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/edge_list/" + s + ".txt";
}

inline string get_bin_list_path(string s) {
    // get file location of edgelist for graphs
    // return "/homes/ywangby/workspace/LinsysSimRank/datasets/bin_edge_list/" + s + ".bin";
    return "/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC/build/edge_list/" + s + ".bin";
}

inline string get_bin_list_path_from_txt(string path) {
    auto tmp = path.replace(path.begin() + 48, path.begin() + 57, "bin_edge_list");
    auto tmp2 = tmp.replace(tmp.end() - 3, tmp.end(), "bin");
    return tmp2;
}

template<typename Iter, typename RandomGenerator>
inline Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
    std::uniform_int_distribution<> dis(0, (end - start) - 1);
    start += dis(g);
    return start;
}

template<typename Iter>
inline Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
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
