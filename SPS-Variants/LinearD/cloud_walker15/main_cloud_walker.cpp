//
// Created by yche on 1/2/18.
//

#include <chrono>

#include "cloud_walker.h"

void test_cloud_walker(string data_name, double c = 0.6, int T = 10, int L = 3, int R = 100, int R_prime = 10000) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    srand(static_cast<unsigned int>(time(nullptr))); // random number generator
    std::chrono::duration<double> elapsed{};
    int sample_size = 1000;

    // 1st: indexing
    auto start = std::chrono::high_resolution_clock::now();
    CloudWalker cw(&g, data_name, c, T, L, R, R_prime);
    auto pre_time = std::chrono::high_resolution_clock::now();

    // output to log file
    size_t n = num_vertices(g);
    ofstream out(CLOUD_WALKER_DIR + cw.g_name + string(".meta")); // the file to store the execute info
    elapsed = pre_time - start;
    out << elapsed.count() << endl; // record the pre-processing time
    out << n << endl; // number of vertices

    // 2nd: query
    for (int i = 0; i < sample_size; i++) {
        auto source_node = static_cast<int>(rand() % n);
        VectorXd tmp(n);
        auto start_ss = std::chrono::high_resolution_clock::now();
        cw.mcss(source_node, tmp);
        auto end_ss = std::chrono::high_resolution_clock::now();
        elapsed = end_ss - start_ss;
        auto mem_size = cw.mem_size; // record the memory size
        auto cpu_time = elapsed.count();
        out << cpu_time << " " << mem_size << endl;
    }
    out.close();
}

int main(int argc, char *argv[]) {
    test_cloud_walker("ca-GrQc");
}
