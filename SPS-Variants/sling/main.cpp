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
    sling_algo.calcD(0.005);
    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish calcD " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";

    tmp_start = std::chrono::high_resolution_clock::now();
    sling_algo.backward(0.000725);
    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish backward " << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";

    int u = atoi(argv[3]);
    int v = atoi(argv[4]);

    tmp_start = std::chrono::high_resolution_clock::now();
    double result = sling_algo.simrank(u, v);
    tmp_end = std::chrono::high_resolution_clock::now();

    cout << result << std::endl;
    cout << "query time:"
         << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";
}