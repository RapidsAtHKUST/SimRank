//
// Created by yche on 1/2/18.
//

#include <chrono>

#include "cloud_walker.h"

using namespace std;
using namespace std::chrono;

void test_cloud_walker_sp(string data_name, int u, int v,
                          double c = 0.6, int T = 10, int L = 3, int R = 100, int R_prime = 10000) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    srand(static_cast<unsigned int>(time(nullptr))); // random number generator
    std::chrono::duration<double> elapsed{};

    // 1st: indexing
    auto start = std::chrono::high_resolution_clock::now();
    CloudWalker cw(&g, data_name, c, T, L, R, R_prime);
    auto pre_time = std::chrono::high_resolution_clock::now();
    cout << "indexing time:" << float(duration_cast<microseconds>(pre_time - start).count()) / (pow(10, 6)) << " s\n";

    // allocate memory in advance
    MatrixXd pos_dist_v(T + 1, cw.n);
    MatrixXd pos_dist_u(T + 1, cw.n);

    // 2nd: query
    auto tmp_start = std::chrono::high_resolution_clock::now();
    cout << cw.mcsp(u, v, pos_dist_u, pos_dist_v) << endl;
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish single-pair(real sp) computation "
         << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
}

int main(int argc, char *argv[]) {
    double c = 0.6;
    int L = 3;
    int R = 100;

    int T = 10;
    int R_prime = 10000;

    test_cloud_walker_sp(string(argv[1]), atoi(argv[2]), atoi(argv[3]), c, T, L, R, R_prime);
}