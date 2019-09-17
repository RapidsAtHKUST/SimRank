//
// Created by yche on 12/23/17.
//

#include <boost/program_options.hpp>

#include "../yche_refactor/flpmc_yche.h"
#include "../yche_refactor/simrank.h"

using namespace std;
using namespace boost::program_options;

void test_FLPMC(string data_name, double c, double epsilon, double delta, int x, int y) {
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    NodePair q{x, y};

    // 1st: init
    FLPMC flpmc(data_name, g, c, epsilon, delta, 100);

    // 2nd: query
    TruthSim ts(data_name, g, c, epsilon);
    auto max_err = 0.0;
    auto min_err = 1.0;
    auto failure_count = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (auto i = 0; i < 2000; i++) {
        double result = flpmc.query_one2one(q);
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
}

int main(int args, char *argv[]) {
    try {
        options_description desc{"Options"};
        double c = 0.6;
        double epsilon = 0.01;
        double delta = 0.01;
        string data_name;
        int x, y;
        desc.add_options()
                ("help,h", "Help Screen")
                ("DataName,d", value<string>(&data_name), "Graph Name")
                ("decay,c", value<double>(&c), "Decay Facrot c")
                ("x,x", value<int>(&x), "x: the query node")
                ("y,y", value<int>(&y), "y: the query node")
                ("epsilon,e", value<double>(&epsilon), "Error bound");
        variables_map vm;
        store(parse_command_line(args, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
        } else {
            test_FLPMC(data_name, c, epsilon, delta, x, y);
        }

    }
    catch (const error &ex) {
        cerr << ex.what() << endl;
    }
    return 0;
}