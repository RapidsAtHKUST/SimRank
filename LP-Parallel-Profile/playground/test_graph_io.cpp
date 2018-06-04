//
// Created by yche on 12/23/17.
//

#include <iostream>

#include "../util/graph_yche.h"
#include "../util/pretty_print.h"

int main(int argc, char *argv[]) {
    auto my_path = string(argv[1]);
    auto graph = GraphYche(my_path);

    cout << "n:" << graph.n << ",m:" << graph.m << endl << endl;
    cout << graph.off_in << endl;
    cout << graph.in_deg_arr << endl;
    cout << graph.neighbors_in << endl;

    cout << endl;
    cout << graph.off_out << endl;
    cout << graph.out_deg_arr << endl;
    cout << graph.neighbors_out << endl;
}