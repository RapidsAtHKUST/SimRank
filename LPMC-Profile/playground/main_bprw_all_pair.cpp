//
// Created by yche on 12/20/17.
//
#include <cstdlib>
#include <chrono>

#include <iostream>

#include <boost/program_options.hpp>

#include "../yche_refactor/bprw_yche.h"
#include "../yche_refactor/simrank.h"

using namespace std;
using namespace boost::program_options;

void test_bp(string data_name, double c, double epsilon, double delta) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);

    cout << n << endl;


#ifdef GROUND_TRUTH
    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
#endif

    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        auto bprw = BackPush(data_name, g, c, epsilon, delta);

#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp for schedule(dynamic, 1)
#endif

#if defined(ALL_PAIR)
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
#else
                for (auto i = 0u; i < 1000; i++) {
                    for (auto j = i; j < 1000; j++) {
#endif
                auto q = pair<uint32_t, uint32_t>(i, j);
#ifdef GROUND_TRUTH
                auto res = bprw.query_one2one(q);
                // left: local, right: global or local ???
                max_err = max(max_err, abs(ts.sim(q.first, q.second) - res));
                if (abs(ts.sim(q.first, q.second) - res) > 0.01) {
#pragma omp critical
                    cout << i << ", " << j << "," << ts.sim(q.first, q.second) << "," << res << endl;
                }
#else
                bprw.query_one2one(q);
#endif
            }
        }
//#pragma omp critical
//        cout << bprw.heap.R.bucket_count() << ", " << bprw.heap.R.load_factor() << endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    test_bp(data_name, c, epsilon, delta);
}