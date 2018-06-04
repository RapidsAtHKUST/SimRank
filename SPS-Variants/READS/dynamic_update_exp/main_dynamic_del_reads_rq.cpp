//
// Created by yche on 1/25/18.
//

#include <iostream>
#include <fstream>
#include <chrono>

#include <boost/graph/adjacency_list.hpp>

#include "../ground_truth/graph_yche.h"
#include "../util/sample_num.h"
#include "../algorithm/readsrq.h"

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
    int r = 100;
    int rq = compute_reads_rq_num(eps, delta, c, r);
    int num_updates = 1000;

    cout << "sample num:" << r << ", on-line rand-walk:" << rq << endl;

    // 1st: generate edges
    cout << "begin generate edges..." << endl;
    std::set<std::pair<unsigned int, unsigned int>> del_edges;
    random_device rd;
    mt19937 gen(rd());

    auto num_v = num_vertices(g);

    vector<int> weights;
    weights.reserve(num_v);
    DirectedG::vertex_iterator v_it, v_end;
    tie(v_it, v_end) = vertices(g);
    for (; v_it != v_end; ++v_it) { weights.emplace_back(static_cast<int>(out_degree(*v_it, g))); }

    discrete_distribution<int> geometric_distribution(weights.begin(), weights.end());
    while (del_edges.size() < num_updates) {
        auto src_v = geometric_distribution(gen);
        uniform_int_distribution<int> distribution(0, weights[src_v] - 1);
        DirectedG::out_edge_iterator outi_iter, outi_end;
        tie(outi_iter, outi_end) = out_edges(src_v, g);
        auto dst_v = target(*(outi_iter + distribution(gen)), g);
        del_edges.emplace(src_v, dst_v);
    }

    vector<pair<unsigned int, unsigned int>> del_edge_vec{std::begin(del_edges), std::end(del_edges)};

    // construct the reads-rq algorithm object
    readsrq algorithm(data_name, n, r, rq, c, t);


    // 2nd: dynamic update statistics
    cout << "reads-d begin dynamic update..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &edge:del_edge_vec) {
        algorithm.delEdge(edge.first, edge.second);
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "total del time: " << elapsed.count() << endl;
    cout << "avg del time: " << elapsed.count() / num_updates << endl;
}