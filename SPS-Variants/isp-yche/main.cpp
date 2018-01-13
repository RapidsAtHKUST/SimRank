#include <iostream>
#include <chrono>

#include "ISP.h"

int main(int argc, char *argv[]) {
    string path_str = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/" + string(argv[1]) + ".txt";
    double c = 0.6;
    int u = atoi(argv[2]);
    int v = atoi(argv[3]);
    ISP my_algo(path_str);

    double h = 0.0001;
    auto tmp_start = std::chrono::high_resolution_clock::now();
    cout << my_algo.ComputeSim(u, v, c, 9, h) << endl;
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "query time:"
         << float(std::chrono::duration_cast<std::chrono::microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

    return 0;
}