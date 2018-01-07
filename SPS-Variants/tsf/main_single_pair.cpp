//
// Created by yche on 12/18/17.
//

#include <chrono>
#include <iostream>

#include "input_output.h"
#include "util/pretty_print.h"

#include "yche_tsf.h"

using namespace std::chrono;

int main(int argc, char *argv[]) {
    auto decayFactor = 0.6;
    auto stepNum = 11;
    auto sampleNum = 100;
    auto sampleQueryNum = 43;

    vector<int> graph_src_vec;
    vector<int> graph_dst_vec;

    string input_file_path = argv[1];
    int u = atoi(argv[2]);
    int v = atoi(argv[3]);
    InitGraph(input_file_path, graph_src_vec, graph_dst_vec);

    auto yche_tfs = YcheTSF(sampleNum, sampleQueryNum, stepNum, decayFactor, graph_src_vec, graph_dst_vec);

    auto tmp_start = high_resolution_clock::now();
    cout << yche_tfs.querySinglePair(u, v) << endl;
    auto tmp_end = high_resolution_clock::now();
    cout << "query computation time:"
         << duration_cast<milliseconds>(tmp_end - tmp_start).count() << " ms\n";
}
