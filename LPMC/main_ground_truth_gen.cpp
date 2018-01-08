//
// Created by yche on 1/7/18.
//

#include "simrank.h"

int main(int args, char *argv[]) {
    double c = 0.6;
    double epsilon = 0.00001;

    DirectedG g;
    load_graph(get_edge_list_path(string(argv[1])), g);
    TruthSim ts(string(argv[1]), g, c, epsilon);
    ts.run(g);
    ts.save();
    cout << "finish gen..." << endl;
}