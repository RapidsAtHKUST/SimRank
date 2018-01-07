//
// Created by yche on 12/18/17.
//


#include <chrono>
#include <iostream>

#include "TSF.h"
#include "../input_output.h"
#include "../util/pretty_print.h"

using namespace std::chrono;

int main(int argc, char *argv[]) {
    auto decayFactor = 0.6;

    auto hasIndex = false;
    auto isFm = true;
    auto numIter = 10;
    auto sampleNum = 100;
    auto sampleQueryNum = 100;

    auto usDisk = 0;
    auto *index_path = const_cast<char *>("test_output.bin");

    vector<int> graph_src_vec;
    vector<int> graph_dst_vec;

    string input_file_path = argv[1];
    InitGraph(input_file_path, graph_src_vec, graph_dst_vec);

#ifdef DEBUG
    cout << graph_src_vec << endl;
    cout << graph_dst_vec << endl;
#endif
    auto srm = new TSF(numIter, sampleNum, decayFactor, sampleQueryNum,
                       &graph_src_vec.front(), &graph_dst_vec.front(),
                       static_cast<int>(graph_src_vec.size() - 1),
                       usDisk, index_path, hasIndex, isFm);

    auto tmp_start = high_resolution_clock::now();
    srm->run(0, 20);
    auto tmp_end = high_resolution_clock::now();
    cout << "query computation time:"
         << duration_cast<milliseconds>(tmp_end - tmp_start).count() << " ms\n";
}
