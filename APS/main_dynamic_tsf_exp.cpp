//
// Created by yche on 1/22/18.
//
#include <random>
#include "graph.h"

int main(int argc, char *argv[]) {
    // load reversed graph
    DirectedG g;
    string data_name = argv[1];
    int num_updates = atoi(argv[2]);
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    // construct 100 one-way-graphs
    auto sampleNum = 100;
    vector<vector<int>> owg_arr;
    owg_arr.resize(sampleNum);

    random_device rd;
    mt19937 gen(rd());

    for (int sid = 0; sid < sampleNum; ++sid) {
        owg_arr[sid] = vector<int>(n, -1);
        for (int i = 0; i < n; ++i) {
            auto &one_way_graph = owg_arr[sid];
            auto in_deg = in_degree(i, g);
            if (in_deg != 0) {
                uniform_int_distribution<int> distribution(0, in_deg);
                DirectedG::in_edge_iterator in_edge_iter, in_edge_end;
                tie(in_edge_iter, in_edge_end) = in_edges(i, g);
                one_way_graph[i] = static_cast<int>(source(*(in_edge_iter + distribution(gen)), g));
            }
        }
    }

    vector<pair<unsigned int, unsigned int>> ins_edges;
    uniform_int_distribution<int> distribution(0, static_cast<int>(n - 1));
    while (ins_edges.size() < num_updates) {
        auto e1 = distribution(gen) % n;
        auto e2 = distribution(gen) % n;
        if (!boost::edge(e1, e2, g).second && boost::in_degree(e1, g) > 0 && boost::in_degree(e2, g) > 0) {
            ins_edges.emplace_back(e1, e2);
        }
    }
    cout << "tsf begin dynamic update..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &edge:ins_edges) {
        // add to g
        add_edge(edge.first, edge.second, g);
        for (int sid = 0; sid < sampleNum; ++sid) {
            auto &one_way_graph = owg_arr[sid];
            DirectedG::in_edge_iterator in_edge_iter, in_edge_end;
            tie(in_edge_iter, in_edge_end) = in_edges(edge.second, g);
            auto in_deg = in_degree(edge.second, g);
            uniform_int_distribution<int> distribution(0, in_deg);
            one_way_graph[edge.second] = static_cast<int>(source(*(in_edge_iter + distribution(gen)), g));
        }
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "total time: " << elapsed.count() << endl;
    cout << "avg time: " << elapsed.count() / num_updates << endl;
}