#include <iostream>

#include "ISP.h"

int main(int argc, char *argv[]) {
    string path_str = argv[1];
    double c = atof(argv[2]);
    int u = atoi(argv[3]);
    int v = atoi(argv[4]);
    ISP my_algo(path_str);

//    cout << my_algo.ComputeSim(1, 2, c, 200) << endl;
//    cout << my_algo.ComputeSim(1, 3, c, 200) << endl;
    double h = 0.000001;
    cout << my_algo.ComputeSim(u, v, c, 50, h) << endl;
    return 0;
}