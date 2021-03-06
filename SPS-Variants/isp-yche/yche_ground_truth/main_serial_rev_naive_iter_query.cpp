//
// Created by yche on 12/13/17.
//

#include <iostream>

#include "../pretty_print.h"

#include "SerialRevNaiveSim.h"

using namespace std;

int main(int argc, char *argv[]) {
    string path_str = argv[1];
    double c = atof(argv[2]);
    SerialRevNaiveSim my_algo(path_str);
    int u = atoi(argv[3]);
    int v = atoi(argv[4]);

    auto &my_mat = my_algo.ComputeSim(100, 0.00001, c);
//    cout << my_mat << endl;
    cout << my_mat[u * sqrt(my_mat.size()) + v];
}