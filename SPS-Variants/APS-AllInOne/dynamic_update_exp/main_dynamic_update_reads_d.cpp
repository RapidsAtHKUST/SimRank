//
// Created by yche on 1/25/18.
//

#include <iostream>
#include <fstream>
#include <chrono>

#include <boost/graph/adjacency_list.hpp>

#include "../ground_truth/graph_yche.h"
#include "../util/sample_num.h"
#include "../reads/readsd.h"

using namespace std;
using namespace boost;
using namespace std::chrono;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS> DirectedG;

void load_graph(string path, DirectedG &g) {
    // load graph from edge_list file
    if (file_exists(path)) {
        int a, b;
        cout << "loading " << path << endl;;
        ifstream edgeFile(path, ios::in);
        while (edgeFile >> a >> b) {
            add_edge(a, b, g);
        }
        edgeFile.close();
        return;
    } else {
        cout << "file doesn't exit" << endl;
        return;
    }
}

int main(int argc, char *argv[]) {
    DirectedG g;
    string data_name = argv[1];
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);

    double eps = 0.01;
    double delta = 0.01;
    double c = 0.6;
    int t = 10;
    int r = compute_reads_sample_num(eps, delta, c);
    cout << "sample num:" << r << endl;

    // 1st: generate edges
    int num_updates = 1000;
    cout << "begin generate edges..." << endl;
    vector<pair<unsigned int, unsigned int>> ins_edges;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(0, static_cast<int>(n - 1));
    while (ins_edges.size() < num_updates) {
        auto e1 = distribution(gen) % n;
        auto e2 = distribution(gen) % n;
        if (!boost::edge(e1, e2, g).second && boost::in_degree(e1, g) > 0 && boost::in_degree(e2, g) > 0) {
            ins_edges.emplace_back(e1, e2);
        }
    }

    // construct the reads-d reads object
    readsd algorithm(data_name, n, r, c, t);

    // 2nd: dynamic update statistics
    cout << "reads-d begin dynamic update..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &edge:ins_edges) {
        algorithm.insEdge(edge.first, edge.second);
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "total time: " << elapsed.count() << endl;
    cout << "avg time: " << elapsed.count() / num_updates << endl;
}