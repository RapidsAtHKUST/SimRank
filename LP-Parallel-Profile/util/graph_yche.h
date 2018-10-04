//
// Created by yche on 12/22/17.
//

#ifndef SPS_GRAPH_YCHE_H
#define SPS_GRAPH_YCHE_H

#include <sys/stat.h>

#include <cstdint>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>

#include "../util/log.h"

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
template<typename T>
void WriteArrToFile(string file_path, vector<T> &arr) {
    std::ofstream ofs(file_path, ios::binary);
    auto arr_size = static_cast<int64_t>(arr.size());
    cout << arr_size << endl;
    auto constexpr TYPE_SIZE = sizeof(T);
    ofs.write(reinterpret_cast<const char *>(&arr_size), sizeof(int64_t));
    ofs.write(reinterpret_cast<const char *>(&arr.front()), arr_size * TYPE_SIZE);
}

template<typename T>
void ReadFileToArr(string file_path, vector<T> &arr) {
    ifstream ifs(file_path, ios::binary);
    int64_t arr_size;
    auto constexpr TYPE_SIZE = sizeof(T);
    ifs.read(reinterpret_cast<char *>(&arr_size), TYPE_SIZE);
    arr.resize(arr_size);
    log_info("edge#: %lld", arr_size);
    log_info("edge type size: %d", TYPE_SIZE);

    ifs.read(reinterpret_cast<char *>(&arr.front()), arr_size * TYPE_SIZE);
    log_info("finish reading edge list");
}

// utility function
inline bool file_exists(std::string name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

inline string get_edge_list_path(string s) {
    // get file location of edgelist for graphs
    return string(DATA_ROOT) + "/edge_list/" + s + ".txt";
}

inline string get_bin_list_path(string s) {
    // get file location of edgelist for graphs
    return string(DATA_ROOT) + "/bin_edge_list/" + s + ".bin";
}

inline string get_bin_list_path_from_txt(string path) {
    auto start_pos = string(DATA_ROOT).length() + 1;
    auto tmp = path.replace(path.begin() + start_pos, path.begin() + start_pos + 9, "bin_edge_list");
    auto tmp2 = tmp.replace(tmp.end() - 3, tmp.end(), "bin");
    cout << tmp2 << endl;
    return tmp2;
}

#endif //SPS_GRAPH_YCHE_H
