//
// Created by yche on 12/13/17.
//
#include <iostream>

#include "../pretty_print.h"

#include "SerialNaiveSim.h"

using namespace std;

int main(int argc, char *argv[]) {
    string path_str = argv[1];
    SerialNaiveSim my_algo(path_str);

    auto &my_mat = my_algo.ComputeSim(100, 0.00001, 0.5);
    cout << my_mat << endl;
}
