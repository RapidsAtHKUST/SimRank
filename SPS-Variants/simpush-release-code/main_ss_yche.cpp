//
// Created by shijm on 17/8/19.
//

#include "SimRank.h"
#include "util/stat.h"
#include "util/util.h"
#include "util/log.h"

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
    auto clock_start = clock();

    Timer global_timer;
    int i = 1;
    char *endptr;
    string filelabel;
    int query_node = -1;
    double c = 0.6;

    while (i < argc) {
        if (!strcmp(argv[i], "-f")) {
            i = check_inc(i, argc);
            filelabel = argv[i];
        } else if (!strcmp(argv[i], "-qn")) {
            i = check_inc(i, argc);
            query_node = strtod(argv[i], &endptr);
            if ((query_node < 0) && endptr) {
                cerr << "Invalid query_node argument" << endl;
                exit(1);
            }
        } else {
            usage();
            exit(1);
        }
        i++;
    }

    Graph g;
    g.loadGraphBinEL(filelabel);
    if (query_node == -1 || query_node > g.n)
        query_node = min(50, g.n);
    cout << "query node: " << query_node << endl;

    int REP = 3;
    cout << "REPEAT: " << REP << endl;
//    vector<string> epsilonVec = {"0.05", "0.02", "0.01", "0.005", "0.002"};
    vector<string> epsilonVec = {"0.02"};
    for (int j = 0; j < epsilonVec.size(); ++j) {
        string epsStr = epsilonVec[j];
        double eps_h = stod(epsilonVec[j]) * (1.0 - sqrt(c)) / 3.0 / sqrt(c);
        cout << "eps: " << epsStr << endl;
        SimRank sim = SimRank(&g, filelabel, eps_h, epsStr, c);
        log_info("PreProcessing Time: %.6lfs, Mem Usage: %s", global_timer.elapsed(),
                 FormatWithCommas(getValue()).c_str());
        auto clock_end = clock();
        auto clock_time = static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC;
        log_info("Clock Time: %.6lfs, Mem Usage: %s", clock_time,
                 FormatWithCommas(getValue()).c_str());
        for (int repeat = 0; repeat < REP; ++repeat) {
            Timer timer;
            int queryNid = query_node;
            //reset for next query.
            memset(sim.result, 0, sizeof(double) * g.n);
            sim.result[queryNid] = 1.0;
            sim.singleSource(queryNid);
//                if (repeat == REP - 1) {
//                    sim.writeSingleSourceResult(queryNid);
//                }
            sim.reset();
            log_info("Query Time: %.6lfs, Mem Usage: %s KB", timer.elapsed(), FormatWithCommas(getValue()).c_str());
        }
        cout << "result written to result/" << endl;
    }
}
