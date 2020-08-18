//
// Created by shijm on 17/8/19.
//

#include "SimRank.h"

void usage() {
    cerr << "./simpush" << endl
         << "-f <graphlabel>" << endl
         << "[-qn <querynum> (default 50)]" << endl;
}

//Check parameters
int check_inc(int i, int max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}

int main(int argc, char **argv) {
    int i = 1;
    char *endptr;
    string filelabel;
    int querynum = -1;
    double c = 0.6;

    while (i < argc) {
        if (!strcmp(argv[i], "-f")) {
            i = check_inc(i, argc);
            filelabel = argv[i];
        } else if (!strcmp(argv[i], "-qn")) {
            i = check_inc(i, argc);
            querynum = strtod(argv[i], &endptr);
            if ((querynum < 0) && endptr) {
                cerr << "Invalid querynum argument" << endl;
                exit(1);
            }
        } else {
            usage();
            exit(1);
        }
        i++;
    }

    Graph g;
    g.loadGraph(filelabel);
    if (querynum == -1 || querynum > g.n)
        querynum = min(50, g.n);
    cout << "query num: " << querynum << endl;

    int REP = 3;
    cout << "REPEAT: " << REP << endl;
    vector<string> epsilonVec = {"0.05", "0.02", "0.01", "0.005", "0.002"};
    for (int j = 0; j < epsilonVec.size(); ++j) {
        string epsStr = epsilonVec[j];
        double eps_h = stod(epsilonVec[j]) * (1.0 - sqrt(c)) / 3.0 / sqrt(c);
        cout << "eps: " << epsStr << endl;
        SimRank sim = SimRank(&g, filelabel, eps_h, epsStr, c);
        for (int repeat = 0; repeat < REP; ++repeat) {
            string queryname = "query/" + filelabel + ".query";
            ifstream fquery;
            fquery.open(queryname.data());
            double totaltime = 0;
            for (int i = 0; i < querynum; i++) {
                int queryNid;
                fquery >> queryNid;
                if (queryNid >= sim.vert)
                    cout << queryNid << " is larger than n=" << sim.vert << endl;
                //reset for next query.
                memset(sim.result, 0, sizeof(double) * g.n);
                sim.result[queryNid] = 1.0;

                auto start = std::chrono::high_resolution_clock::now();
                sim.singleSource(queryNid);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> fp_ms = end - start;
                totaltime += fp_ms.count();
                if (repeat == REP - 1) {
                    sim.writeSingleSourceResult(queryNid);
                }
            }
            cout << totaltime / (double) querynum << " ms" << endl;
            fquery.close();
            sim.reset();
        }
        cout << "result written to result/" << endl;
    }
}
