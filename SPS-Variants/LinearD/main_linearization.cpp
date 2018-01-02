//
// Created by yche on 1/2/18.
//

#include <chrono>

#include "linearD.h"

void test_linearD(string data_name, double c, int T, int L, int R) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    srand(time(NULL)); // random number generator
    std::chrono::duration<double> elapsed;
    int sample_size = 1000;

    auto start = std::chrono::high_resolution_clock::now();
    LinearD lin(&g, data_name, c, T, L, R);
    auto pre_time = std::chrono::high_resolution_clock::now();
    size_t n = num_vertices(g);
    ofstream out(LINEAR_D_DIR + lin.g_name + string(".meta")); // the file to store the execute info
    elapsed = pre_time - start;
    out << elapsed.count() << endl; // record the pre-processing time
    out << n << endl; // number of vertices
    for (int i = 0; i < sample_size; i++) {
        if (i % 10 == 0) {
            cout << i << " th sample for" << lin.g_name << endl;
        }
        int source_node = rand() % n;
        VectorXd tmp(n);
        auto start_ss = std::chrono::high_resolution_clock::now();
        lin.single_source(source_node, tmp);
        auto end_ss = std::chrono::high_resolution_clock::now();
        elapsed = end_ss - start_ss;
        auto mem_size = lin.mem_size; // record the memeory size
        auto cpu_time = elapsed.count();
        out << cpu_time << " " << mem_size << endl;
    }
    out.close();

    // lin.mcap();
    // cout << lin.get_file_path_base() << endl;
    // cout << lin.sim << endl;
    // lin.save();
    cout << "return ... " << endl;
}

void test_lineD_all_pair(string data_name, double c, int T, int L, int R) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    LinearD lin(&g, data_name, c, T, L, R);
    lin.all_pair();
    cout << lin.get_file_path_base() << endl;
    lin.save();
}

int main(int argc, char *argv[]) {
    int R = 100;
    int L = 15;
    int T = 15;
    double c = 0.6;

    using namespace std::chrono;
    auto tmp_start = std::chrono::high_resolution_clock::now();
    test_lineD_all_pair("ca-GrQc", c, T, L, R);
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish backward " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

}