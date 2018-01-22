//
// Created by yche on 1/22/18.
//

#include <random>

#include "local_push.h"

using namespace std;
using namespace std::chrono;

void exp_dynamic(string data_name, double c, double epsilon, int num_updates = 1000) {
    // load graph and available result
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    cout << "begin rlp constructing" << endl;
    Reduced_LocalPush rlp(g, data_name, c, epsilon, n);

    // 1st: generate edges
    cout << "begin generate edges..." << endl;
    vector<pair<unsigned int, unsigned int>> ins_edges;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(0, n - 1);
    while (ins_edges.size() < num_updates) {
        auto e1 = distribution(gen) % n;
        auto e2 = distribution(gen) % n;
        if (!boost::edge(e1, e2, g).second) {
            ins_edges.emplace_back(e1, e2);
        }
    }

    // 2nd: dynamic updates
    cout << "begin dynamic update..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &edge:ins_edges) {
        // add to g
        add_edge(edge.first, edge.second, g);
        // update P and R
        rlp.insert(edge.first, edge.second, g);
    }
    cout << "begin local push..." << endl;
    auto middle = std::chrono::high_resolution_clock::now();
    rlp.local_push(g);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << duration_cast<microseconds>(middle - start).count() / pow(10, 6) << endl;

    // statistics
    cout << "data: " << data_name << endl;
    cout << "number of updates: " << num_updates << endl;
    cout << "total time: " << elapsed.count() << endl;
    cout << "avg time: " << elapsed.count() / num_updates << endl;
}

int main(int argc, char *argv[]) {
    string data_name = argv[1];
    int num_updates = atoi(argv[2]);
    double c = 0.6;
    double eps = 0.116040;
    exp_dynamic(data_name, c, eps, num_updates);
}