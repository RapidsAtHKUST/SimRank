#include "reads.h"
#include "readsd.h"
#include "readsrq.h"

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
//querying one-to-all simrank value from node o
    i1.queryAll(0, ansVal);

    readsd i2(argv[1], n, r, c, t);
//querying one-to-all simrank value from node o
    i2.queryAll(0, ansVal);

    readsrq i3(argv[1], n, r, rq, c, t);
//querying one-to-all simrank value from node o
    i3.queryAll(0, ansVal);


    delete[] ansVal;

}