#include <iostream>

#include "ISP.h"

int main(int argc, char *argv[]) {
    string path_str = argv[1];
    double c = atof(argv[2]);
    ISP my_algo(path_str);

    cout << my_algo.ComputeSim(1, 2, c, 200) << endl;
    cout << my_algo.ComputeSim(1, 3, c, 200) << endl;
    return 0;
}