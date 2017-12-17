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
    Sling sling_algo(&g, c);

    // build the index
    cout <<"indexing..."<<endl;
    sling_algo.calcD(0.005);
    cout <<"finish calcD"<<endl;
    sling_algo.backward(0.000725);
    cout <<"finish backward"<<endl;

    int u = atoi(argv[3]);
    int v = atoi(argv[4]);

    auto tmp_start = std::chrono::high_resolution_clock::now();
    double result = sling_algo.simrank(u, v);
    auto tmp_end = std::chrono::high_resolution_clock::now();

    cout << result << std::endl;
    cout << "query time:"
         << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";
}