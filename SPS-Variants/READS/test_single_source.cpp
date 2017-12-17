//
// Created by yche on 12/17/17.
//

#include <iostream>

#include "reads.h"

using namespace std;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    int n = atoi(argv[2]);
    int r = 100;
    double c = 0.6;
    int t = 10;
    int rq = 10;
    double *ansVal = new double[n];

    reads i1(argv[1], n, r, c, t);

    int vertex = atoi(argv[3]);
    i1.queryAll(vertex, ansVal);
    for (auto i = 0; i < n; i++) {
        cout << ansVal[i] << "\n";
    }
    delete[] ansVal;
}