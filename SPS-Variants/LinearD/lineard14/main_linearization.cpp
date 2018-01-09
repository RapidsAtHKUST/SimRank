//
// Created by yche on 1/2/18.
//

#include <chrono>

#include "linearD.h"

using namespace std;
using namespace std::chrono;

void test_lineD_all_pair(string data_name, double c, int T, int L, int R) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);

    auto start_ss = std::chrono::high_resolution_clock::now();
    LinearD lin(&g, data_name, c, T, L, R);
    auto end_ss = std::chrono::high_resolution_clock::now();
    cout << "pre-processing: " << float(duration_cast<microseconds>(end_ss - start_ss).count()) / (pow(10, 6))
         << " s\n" << endl;

    lin.all_pair();
    cout << lin.get_file_path_base() << endl;
    lin.save();
}

void test_single_source(string data_name, double c, int T, int L, int R, int i, int j = 1) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);

    auto start_ss = std::chrono::high_resolution_clock::now();
    LinearD lin(&g, data_name, c, T, L, R);
    auto end_ss = std::chrono::high_resolution_clock::now();
    cout << "pre-processing: " << float(duration_cast<microseconds>(end_ss - start_ss).count()) / (pow(10, 6))
         << " s\n" << endl;

    VectorXd tmp(num_vertices(g));

    auto tmp_start = std::chrono::high_resolution_clock::now();
    lin.single_source(i, tmp);
    cout << tmp(j) << endl;
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish single-pair computation "
         << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
}

void test_lineD_single_pair(string data_name, double c, int T, int L, int R, int i, int j) {
    // load graph
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    // init data
    auto start_ss = std::chrono::high_resolution_clock::now();
    LinearD lin(&g, data_name, c, T, L, R);
    auto end_ss = std::chrono::high_resolution_clock::now();
    cout << "pre-processing: " << float(duration_cast<microseconds>(end_ss - start_ss).count()) / (pow(10, 6))
         << " s\n" << endl;

    // 1st: single source solution
    VectorXd tmp(num_vertices(g));
    auto tmp_start = std::chrono::high_resolution_clock::now();
    lin.single_source(i, tmp);
    cout << tmp(j) << endl;
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish single-pair(via ss) computation "
         << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";

    // 2nd: single pair solution
    VectorXd lhs_vec(lin.n), rhs_vec(lin.n);
    tmp_start = std::chrono::high_resolution_clock::now();
    cout << lin.single_pair(i, j, lhs_vec, rhs_vec) << endl;
    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish single-pair(real sp) computation "
         << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
}

int main(int argc, char *argv[]) {
    int R = 100;
    int L = 3;
    double c = 0.6;

    int T = 10;

    test_lineD_single_pair(string(argv[1]), c, T, L, R, atoi(argv[2]), atoi(argv[3]));
}