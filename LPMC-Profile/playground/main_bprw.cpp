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

    // 2nd: query sim(x,y)
    auto start = std::chrono::high_resolution_clock::now();
    double result = bprw.query_one2one(q);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << "\n" << format("sim: %s:%s") % q % result << endl;
    cout << format("memory:%s KB") % getValue() << endl;
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time

    // 3rd: ground truth
    if (g.n < 10000) {
        cout << "\n";
        TruthSim ts(data_name, g, c, epsilon);
        cout << format("ground truth: %s") % ts.sim(x, y) << endl;
        cout << format("error: %s") % (ts.sim(q.first, q.second) - result) << endl;
    }
}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_bp(data_name, c, epsilon, delta, x, y);
}