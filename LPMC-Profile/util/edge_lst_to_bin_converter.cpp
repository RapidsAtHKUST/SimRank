//
// Created by yche on 4/5/18.
//
#include <iostream>
#include <fstream>

#include "graph_yche.h"

using namespace std;

int main(int argc, char *argv[]) {
    string edge_lst_path = get_edge_list_path(string(argv[1]));
    auto edge_lst = GetEdgeList(edge_lst_path);
    WriteArrToFile(get_bin_list_path(string(argv[1])), edge_lst);
}