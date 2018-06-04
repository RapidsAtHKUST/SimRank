//
// Created by yche on 12/20/17.
//
#include <cstdlib>
#include <chrono>

#include <iostream>

#include <boost/program_options.hpp>

#include "../../yche_refactor/bprw_yche.h"
#include "../../yche_refactor/simrank.h"
#include "../../util/random_pair_generator.h"
#include "../../playground/pretty_print.h"

using namespace std;
using namespace boost::program_options;

void test_bp(string data_name, double c, double epsilon, double delta, int pair_num, int round) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);

    cout << n << endl;

#ifdef GROUND_TRUTH
    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
#endif
    auto sample_pairs = read_sample_pairs(data_name, pair_num, round);
    auto clock_start = clock();
    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        auto bprw = BackPush(data_name, g, c, epsilon, delta);

#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 100)
#else
#pragma omp for schedule(dynamic, 100)
#endif
        for (auto pair_i = 0; pair_i < pair_num; pair_i++) {
            auto q = pair<uint32_t, uint32_t>(sample_pairs[pair_i].first, sample_pairs[pair_i].second);

#ifdef GROUND_TRUTH
            auto res = bprw.query_one2one(q);
            max_err = max(max_err, abs(ts.sim(q.first, q.second) - res));
            if (abs(ts.sim(q.first, q.second) - res) > 0.01) {
#pragma omp critical
                cout << sample_pairs[pair_i].first << ", " << sample_pairs[pair_i].second << ","
                     << ts.sim(q.first, q.second) << "," << res << endl;
            }
#else
            bprw.query_one2one(q);
#endif
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
#ifdef GROUND_TRUTH
    cout << "max err:" << max_err << endl;
#endif
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = atof(argv[4]);
    double epsilon = atof(argv[5]);
    double delta = atof(argv[6]);

    int pair_num = atoi(argv[2]);
    int round_i = atoi(argv[3]);
    test_bp(data_name, c, epsilon, delta, pair_num, round_i);
}