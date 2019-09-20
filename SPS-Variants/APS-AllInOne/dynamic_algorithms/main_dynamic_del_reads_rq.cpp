//
// Created by yche on 1/25/18.
//

#include <iostream>
#include <fstream>
#include <chrono>

#include <boost/graph/adjacency_list.hpp>

#include "util/graph_yche.h"
#include "util/stat.h"

#include "reads/sample_num.h"
#include "reads/readsrq.h"

using namespace std;
using namespace boost;
using namespace std::chrono;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS> DirectedG;

extern void load_graph(string path, DirectedG &g);

extern vector<pair<unsigned int, unsigned int>> GenerateDelEdges(int num_updates, DirectedG &g);

int main(int argc, char *argv[]) {
    FILE *log_f = nullptr;
    if (argc >= 4) {
        log_f = fopen(argv[3], "a+");
        log_set_fp(log_f);
    }
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
    int num_updates = atoi(argv[2]);

    cout << "sample num:" << r << ", on-line rand-walk:" << rq << endl;

    // 1st: generate edges
    auto del_edge_vec = GenerateDelEdges(num_updates, g);

    // construct the reads17-rq reads17 object
    readsrq algorithm(data_name, n, r, rq, c, t);

    // 2nd: dynamic update statistics
    cout << "reads17-d begin dynamic update..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto &edge:del_edge_vec) {
        algorithm.delEdge(edge.first, edge.second);
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    cout << "total del time: " << elapsed.count() << endl;
    cout << "avg del time: " << elapsed.count() / num_updates << endl;

    log_info("Mem Size: %.9lf MB", getValue() / 1024.0);
    log_info("Update Time: %.9lfs", elapsed.count());
    if (log_f != nullptr) {
        log_info("Flush File and Close...");
        fflush(log_f);
        fclose(log_f);
    }
}