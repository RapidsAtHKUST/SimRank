//
// Created by yche on 12/22/17.
//

#ifndef SPS_GRAPH_YCHE_H
#define SPS_GRAPH_YCHE_H

#include <cstdint>

#include <string>
#include <vector>

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

public:
    explicit GraphYche(string &graph_path);

    int in_degree(int u);

    int out_degree(int u);
};

#endif //SPS_GRAPH_YCHE_H
