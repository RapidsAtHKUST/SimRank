//
// Created by yche on 1/22/18.
//

#include <boost/program_options.hpp>

#include "../yche_refactor/simrank.h"
#include "../yche_refactor/local_push_yche.h"

using namespace std;
using namespace boost::program_options;

int main(int argc, char *argv[]) {
    // 1st: compute reduce local push
    string g_name = argv[1];
    double eps = atof(argv[2]);
    string path = get_edge_list_path(g_name);
    GraphYche g(path);
    double c = 0.6;
//    double eps = 0.0001;

    auto n = g.n;
#ifdef FLP
    auto lp = new Full_LocalPush(g, g_name, c, eps, n);
#else
    auto lp = new Reduced_LocalPush(g, g_name, c, eps, n);
#endif
    auto start_time = std::chrono::high_resolution_clock::now();
    lp->local_push(g);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    cout << format("computation time: %s s") % elapsed.count() << endl; // record the pre-processing time
    cout << format("push num: %s") % lp->n_push << endl;
    cout << format("max queue size #: %s") % (lp->n_push / lp->max_q_size) << endl;
    cout << format("mem size: %s KB") % getValue() << endl;

    cout << format("P size: %s") % lp->P.size() << endl;
    cout << format("R size: %s") % lp->R.size() << endl;
    cout << format("avg push merge #: %s") % (lp->n_push / lp->R.size()) << endl;

    // 2nd: verify the correcness
    if (n < 10000) {
        TruthSim ts(g_name, g, c, eps);
        auto max_err = 0.0;
        double err = 0;

#pragma omp parallel for reduction(max:max_err), reduction(+:err) schedule(dynamic, 1)
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
                auto res = lp->query_P(i, j);
                max_err = max(max_err, abs(ts.sim(i, j) - res));
                err += abs(ts.sim(i, j) - res);

                if (abs(ts.sim(i, j) - res) > eps + pow(10, -6)) {
#pragma omp critical
                    cout << i << ", " << j << "," << ts.sim(i, j) << "," << res << endl;
                }
            }
        }
        cout << "max err:" << max_err << endl;
        cout << "mean err:" << (err / ((n + 1) * n / 2)) << endl;
    };
}