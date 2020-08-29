//
// Created by yche on 12/20/17.
//

#include <iostream>
#include <unordered_map>

#include <util/timer.h>
#include "ground_truth/simrank.h"

#include "sling.h"
#include "ground_truth/stat.h"

using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    Timer timer;
    // 1st: load graph
    Graph g;
    string file_name = argv[1];

    double c = atof(argv[2]);
    double eps = atof(argv[3]);
    double delta = atof(argv[4]);
    int u = atoi(argv[5]);

    // compute theta and eps_d
    double half_eps = eps / 2;
    double eps_d = (1 - c) * half_eps;
    double denominator = (2 * sqrt(c)) / (1 - sqrt(c)) / (1 - c);
    double theta = half_eps / (denominator);
    cout << "eps_d:" << eps_d << ", theta:" << theta << endl;

    unordered_map<string, string> file_mapping{
            {"friendster",  "fs.ebin"},
            {"fr",          "fs.ebin"},
            {"fs",          "fs.ebin"},
            {"livejournal", "lj.e.bin"},
            {"lj",          "lj.e.bin"},
            {"twitter",     "tw.e.bin"},
            {"tw",          "tw.e.bin"},
            {"uk-2007-05",  "uk.e.bin"},
            {"uk",          "uk.e.bin"},
            {"comlj",        "com-lj.ud.bin"}
    };
    string file_path = string(DATA_ROOT) + "/" + file_mapping[file_name];
    g.inputGraph2(file_path);

    // 2nd: construct sling algorithm, indexing
    auto tmp_start = std::chrono::high_resolution_clock::now();
    Sling sling_algo(&g, file_name, c, eps_d, theta);

    // setup the failure probability for calD
    sling_algo.failure_probability = delta;
    cout << "delta(fail-prob):" << sling_algo.failure_probability << endl;
    auto tmp_end = std::chrono::high_resolution_clock::now();
    cout << "finish input graph: " << duration_cast<milliseconds>(tmp_end - tmp_start).count() << " ms\n";

    tmp_start = std::chrono::high_resolution_clock::now();
    auto clock_start_cal_d = clock();
    sling_algo.calcD(eps_d);
    auto clock_end_cal_d = clock();
    tmp_end = std::chrono::high_resolution_clock::now();
    cout << "total calD cpu time:" << static_cast<double>(clock_end_cal_d - clock_start_cal_d) / CLOCKS_PER_SEC << "s"
         << endl;

    cout << "finish calcD " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6)) << " s\n";
    tmp_start = std::chrono::high_resolution_clock::now();
    sling_algo.backward(theta);
    log_info("Pre-Processing: %.6lfs, Mem Usage: %s", timer.elapsed_and_reset(), FormatWithCommas(getValue()).c_str());
    tmp_end = std::chrono::high_resolution_clock::now();

    cout << "finish backward " << float(duration_cast<microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
         << " s\n";

    auto start = std::chrono::high_resolution_clock::now();
    auto clock_start = clock();

    // 3rd: querying pairs
    sling_algo.simrank(u);
    log_info("Query Time: %.6lfs, Mem Usage: %s", timer.elapsed(), FormatWithCommas(getValue()).c_str());
    auto end = std::chrono::high_resolution_clock::now();
    auto clock_end = clock();
    cout << "total query cpu time:" << static_cast<double>(clock_end - clock_start) / CLOCKS_PER_SEC << "s" << endl;
    std::chrono::duration<double> elapsed = end - start;
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
}