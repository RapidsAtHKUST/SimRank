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
    size_t n = static_cast<size_t>(g.n);
    NodePair q{x, y};

    FLPMC flpmc(data_name, g, c, epsilon, delta, 100);
    double result = flpmc.query_one2one(q);
    cout << result << endl;

    // ground truth
    TruthSim ts(data_name, g, c, epsilon);
    cout << format("ground truth: %s") % ts.sim(x, y) << endl;
    cout << format("error: %s") % (ts.sim(q.first, q.second) - result) << endl;
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