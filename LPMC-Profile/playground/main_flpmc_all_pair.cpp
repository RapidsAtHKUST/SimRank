//
// Created by yche on 12/23/17.
//

#include "../yche_refactor/flpmc_yche.h"
#include "../util/graph_yche.h"
#include "../yche_refactor/simrank.h"

void test_FLPMC(string data_name, double c, double epsilon, double delta) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    size_t n = static_cast<size_t>(g.n);

    auto flpmc = FLPMC(data_name, g, c, epsilon, delta, 100);

#ifdef GROUND_TRUTH
    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
#endif

    auto start = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
        auto my_flpmc = flpmc;

#ifdef GROUND_TRUTH
#pragma omp for reduction(max:max_err) schedule(dynamic, 1)
#else
#pragma omp for schedule(dynamic, 1)
#endif
//        for (auto i = 0u; i < 1000; i++) {
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
//            for (auto j = i; j < 1000; j++) {
                auto q = pair<uint32_t, uint32_t>(i, j);
#ifdef GROUND_TRUTH
                auto res = my_flpmc.query_one2one(q);
                max_err = max(max_err, abs(ts.sim(q.first, q.second) - res));
                if (abs(ts.sim(q.first, q.second) - res) > 0.01) {
#pragma omp critical
                    cout << i << ", " << j << "," << ts.sim(q.first, q.second) << "," << res << endl;
                }
#else
                my_flpmc.query_one2one(q);
#endif
            }
        }
    };

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
//    double delta = 0.000001;
    double delta = 0.01;

    test_FLPMC(data_name, c, epsilon, delta);
}