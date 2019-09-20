//
// Created by yche on 1/25/18.
//

#include <iostream>
#include <chrono>

#include <boost/graph/adjacency_list.hpp>

#include "util/graph_yche.h"
#include "util/stat.h"

#include "reads/sample_num.h"
#include "reads/readsd.h"

using namespace std;
using namespace boost;
using namespace std::chrono;
typedef boost::adjacency_list<vecS, vecS, bidirectionalS> DirectedG;

extern void load_graph(string path, DirectedG &g);

extern vector<pair<unsigned int, unsigned int>> GenerateInsEdges(int num_updates, DirectedG &g);

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
    int r = compute_reads_sample_num(eps, delta, c);
    cout << "sample num:" << r << endl;

    // 1st: generate edges
    int num_updates = atoi(argv[2]);
    cout << "begin generate edges..." << endl;
    auto ins_edges = GenerateInsEdges(num_updates, g);

    // construct the reads17-d reads17 object
    readsd algorithm(data_name, n, r, c, t);

    // 2nd: dynamic update statistics
    cout << "reads17-d begin dynamic update..." << endl;
    Timer timer;
    for (auto &edge:ins_edges) {
        algorithm.insEdge(edge.first, edge.second);
    }
    cout << "total time: " << timer.elapsed() << endl;
    cout << "avg time: " << timer.elapsed() / num_updates << endl;

    log_info("Mem Size: %.9lf MB", getValue() / 1024.0);
    log_info("Update Time: %.9lfs", timer.elapsed());
    if (log_f != nullptr) {
        log_info("Flush File and Close...");
        fflush(log_f);
        fclose(log_f);
    }
}