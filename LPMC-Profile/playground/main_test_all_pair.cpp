//
// Created by yche on 12/20/17.
//
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "../bprw.h"

#include "omp.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;

void test_bp(string data_name, double c, double epsilon, double delta) {
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);

    size_t n = num_vertices(g);

    cout << n << endl;
    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        auto bprw = BackPush(data_name, g, c, epsilon, delta);
#pragma omp for schedule(dynamic, 1)
        for (auto i = 0u; i < 100; i++) {
            for (auto j = i; j < n; j++) {
                auto q = pair<uint32_t, uint32_t>(i, j);
                bprw.query_one2one(q);
            }
        }
    };

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time

}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

//    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_bp(data_name, c, epsilon, delta);
}