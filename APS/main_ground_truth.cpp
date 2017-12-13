//
// Created by yche on 12/13/17.
//

#include <iostream>

#include <boost/program_options.hpp>

#include "simrank.h"

int main(int argc, char *argv[]) {
    string data_name = argv[1];
    int a = atoi(argv[2]);
    int b = atoi(argv[3]);
    DirectedG g;
    load_graph("./datasets/edge_list/" + data_name + ".txt", g);

    TruthSim ts(data_name, g, 0.6, 0.00001);
    cout << format("ground truth: %s") % ts.sim(a, b) << endl;
}