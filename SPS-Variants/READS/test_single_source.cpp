//
// Created by yche on 12/17/17.
//

#include <iostream>
#include <chrono>

#include "reads.h"

using namespace std;
using namespace std::chrono;

// usage and example:
// g++ reads*.h reads*.cpp timer.h -O3 -w -std=c++11 test.cpp -I . && ./a.out hp.data 3133
int main(int argc, char **argv) {
    int n = atoi(argv[2]);
//    int r = 100;
    int r = 1236;
    double c = 0.6;
    int t = 10;
//    int rq = 10;
    double *ansVal = new double[n];

    reads i1(argv[1], n, r, c, t);

    int vertex = atoi(argv[3]);
    int v = atoi(argv[4]);

    auto tmp_start = high_resolution_clock::now();
    i1.queryAll(vertex, ansVal);
    auto tmp_end = high_resolution_clock::now();
    cout << "query time:"
         << duration_cast<microseconds>(tmp_end - tmp_start).count() << " us\n";
    cout << ansVal[v] << "\n";

    delete[] ansVal;
}