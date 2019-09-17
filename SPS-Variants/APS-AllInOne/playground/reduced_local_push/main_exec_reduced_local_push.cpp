//
// Created by yche on 1/22/18.
//

#include <boost/program_options.hpp>

#include "../../yche_refactor/flpmc_yche.h"
#include "../../yche_refactor/simrank.h"

using namespace std;
using namespace boost::program_options;


int main(int argc, char *argv[]) {
    // 1st: compute reduce local push
    string g_name = argv[1];
    string path = get_edge_list_path(g_name);
    GraphYche g(path);
    double c = 0.6;
    double eps = 0.01;

    auto n = g.n;
    auto lp = new Reduced_LocalPush(g, g_name, c, eps, n);
    if (!lp_file_exists(g_name, c, eps, n, false)) { // test wether the local push index exists
        cout << "local push offline index doesn't exists.. " << endl;
        auto start_time = std::chrono::high_resolution_clock::now();
        lp->local_push(g);
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        cout << format("total indexing cost: %s s") % elapsed.count() << endl; // record the pre-processing time
        cout << format("building compete, saving to %s ") % lp->get_file_path_base() << endl;
        lp->save();
        cout << "saved." << endl;
    } else {
        cout << "offline index exists..loading " << endl;
        lp->load();
    }

    // 2nd: verify the correcness
    if (n < 10000) {
        TruthSim ts(g_name, g, c, eps);
        auto max_err = 0.0;

#pragma omp parallel for reduction(max:max_err) schedule(dynamic, 1)
        for (auto i = 0u; i < n; i++) {
            for (auto j = i; j < n; j++) {
                auto res = lp->query_P(i, j);
                max_err = max(max_err, abs(ts.sim(i, j) - res));
                if (abs(ts.sim(i, j) - res) > eps) {
#pragma omp critical
                    cout << i << ", " << j << "," << ts.sim(i, j) << "," << res << endl;
                }
            }
        }
        cout << "max err:" << max_err << endl;
    };
}