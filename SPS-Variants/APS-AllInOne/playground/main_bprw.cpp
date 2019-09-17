#include <cstdlib>
#include <ctime>

#include <chrono>
#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "../yche_refactor/bprw_yche.h"
#include "../yche_refactor/simrank.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;

void test_bp(string data_name, double c, double epsilon, double delta, int x, int y) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);

    // 1st: init
    BackPush bprw(data_name, g, c, epsilon, delta);
    size_t n = static_cast<size_t>(g.n);
    NodePair q{x, y};

    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
    auto min_err = 1.0;
    // 2nd: query sim(x,y)
    auto failure_count = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < 2000; i++) {
        double result = bprw.query_one2one(q);
        auto cur_err = abs(result - ts.sim(x, y));
        max_err = max(max_err, cur_err);
        min_err = min(min_err, cur_err);
        if (cur_err > 0.01) {
            cout << result << " ," << ts.sim(x, y) << endl;
            failure_count++;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << "failure count:" << failure_count << endl;
    cout << "max err:" << max_err << endl;
    cout << "min err:" << min_err << endl;
    cout << format("memory:%s KB") % getValue() << endl;
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_bp(data_name, c, epsilon, delta, x, y);
}