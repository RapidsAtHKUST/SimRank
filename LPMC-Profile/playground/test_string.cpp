//
// Created by yche on 4/5/18.
//
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;

inline string get_edge_list_path(string s) {
    // get file location of edgelist for graphs
    return "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + s + ".txt";
}

inline string get_bin_list_path(string s) {
    // get file location of edgelist for graphs
    return "/homes/ywangby/workspace/LinsysSimRank/datasets/bin_edge_list/" + s + ".bin";
}

inline string get_bin_list_path_from_txt(string path) {
    string tmp = path.replace(path.begin() + 48, path.begin() + 57, "bin_edge_list");
    string tmp2 = tmp.replace(tmp.end() - 3, tmp.end(), "bin");
    return tmp2;
}

int main() {
    string a_string = get_edge_list_path("ca-GrQc");
    cout << get_bin_list_path_from_txt(a_string) << endl;
}
