//
// Created by yche on 12/20/17.
//
#include <cstdlib>
#include <ctime>
#include <chrono>

#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>


#include "omp.h"
#include "../yche_refactor/bprw_yche.h"

using namespace std;
using namespace boost::program_options;

void test_bp(string data_name, double c, double epsilon, double delta, int x, int y) {
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    string path = get_edge_list_path(data_name);
    GraphYche g(path);

    BackPush bprw(data_name, g, c, epsilon, delta);
    size_t n = static_cast<size_t>(g.n);
    NodePair q{x, y};
    // for(int i = 0; i < 100;i++){
    auto start = std::chrono::high_resolution_clock::now();

    double result = bprw.query_one2one(q);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << q << ": " << result << endl;


    cout << format("sim: %s:%s") % q % result << endl;

    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time

}

void test_bp(string data_name, double c, double epsilon, double delta) {
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);

    cout << n << endl;
    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        auto bprw = BackPush(data_name, g, c, epsilon, delta);
#pragma omp for schedule(dynamic, 1)
//        for (auto i = 0u; i < 100; i++) {
        for (auto i = 0u; i < n; i++) {
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