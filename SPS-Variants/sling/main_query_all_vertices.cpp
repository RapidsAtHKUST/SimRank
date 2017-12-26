//
// Created by yche on 12/20/17.
//

//
// Created by yche on 11/19/17.
//

#include <iostream>

#include "sling.h"
#include "graph.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    // 1st: load graph
    Graph g;
    string file_path = argv[1];
    g.inputGraph(file_path);

    double c = atof(argv[2]);

    auto tmp_start = std::chrono::high_resolution_clock::now();
    Sling sling_algo(&g, c);
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish input graph " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";

    // build the index
    cout << "indexing..." << endl;

    tmp_start = std::chrono::high_resolution_clock::now();
//    sling_algo.calcD(0.005);
    sling_algo.calcD(0.002);
    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish calcD " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

    tmp_start = std::chrono::high_resolution_clock::now();
//    sling_algo.backward(0.000725);
    sling_algo.backward(0.00029);

    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish backward " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

//    int u = atoi(argv[3]);
//    int v = atoi(argv[4]);

    tmp_start = std::chrono::high_resolution_clock::now();
#pragma omp parallel for schedule(dynamic, 1)
//    for (auto u = 0; u < sling_algo.g->n; u++) {
    for (auto u = 0; u < 1000; u++) {
//        for (auto v = u; v < sling_algo.g->n; v++) {
        for (auto v = u; v < 1000; v++) {
            sling_algo.simrank(u, v);
        }
    }

    tmp_end = std::chrono::high_resolution_clock::now();

//    cout << result << std::endl;
    cout << "query time:"
         << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
}