/*************************************************
Copyright:
Author: Yu Liu, Xiaodong He, Zhewei Wei
Date: 2017-12-25
Libraries Required:
	SFMT-1.4.1: A random number generator, which can be downloaded here:
	www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT
**************************************************/

#include <cstdio>

#include <iostream>
#include <algorithm>

#include "util.h"
#include "evaluate.h"
#include "pooling.h"

#ifndef EVALUATION

#include "SimStruct.h"

void usage() {
    cerr << "./ProbeSim [-g <graph name>] [-e <eps, default 0.1>] [-q <query set>] [-d <output dir>]" << endl;
}

int check_inc(int i, int max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}

#endif

using namespace std;

int main(int argc, char **argv) {
#ifdef EVALUATION
    /*
    cout << "graph file: " << argv[1] << endl;
    cout << "query file: " << argv[2] << endl;
    cout << "# query: " << argv[3] << endl;
    cout << "# result dir: " << argv[4] << endl;
    vector<string> dirs;
    for(int i = 0; i < atoi(argv[4]); i++)
        dirs.push_back(argv[4+i+1]);
    cout << "polled result dir: " << argv[4+1+atoi(argv[4])] << endl;
    int k = 50;
    Graph g = Graph(argv[1]);
    Poll poll(g);
    poll.poll(argv[2], atoi(argv[3]), k, dirs, argv[4+1+atoi(argv[4])]);
    */
    evaluateTopK(argv[1], argv[2], argv[3], 50);
    return 0;
#endif

    int i = 1;
    string file;
    string qf;
    string output_dir;
    vector<int> queryNode;
    double eps = 0.1;

    while (i < argc) {
        if (!strcmp(argv[i], "-g")) {
            i = check_inc(i, argc);
            file = argv[i];
        } else if (!strcmp(argv[i], "-q")) {
            i = check_inc(i, argc);
            qf = argv[i];
        } else if (!strcmp(argv[i], "-d")) {
            i = check_inc(i, argc);
            output_dir = argv[i];
        } else if (!strcmp(argv[i], "-e")) {
            i = check_inc(i, argc);
            eps = strtod(argv[i], nullptr);
        } else {
            usage();
            exit(1);
        }
        i++;
    }
    readfile(qf, queryNode);

    SimStruct sim = SimStruct(file, eps);
    double total_t = 0;
    for (int i = 0; i < queryNode.size(); i++) {
        int nodeId = queryNode[i];
        cout << "node: " << nodeId << "\t";
        double t = sim.batch(nodeId, argv[5]);
        cout << t << "s" << endl;
        total_t += t;
    }
    cout << "average probe time: " << total_t / (double) queryNode.size() << "s" << endl;
    return 0;
}