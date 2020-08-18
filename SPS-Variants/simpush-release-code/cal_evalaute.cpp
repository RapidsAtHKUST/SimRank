#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <set>
#include <iomanip>      // std::setprecision

using namespace std;

double calPrecision(vector<int> topK1, vector<int> realList, int kactual, int kparameter) {
    set<int> truth(realList.begin(), realList.end());
    int size = realList.size();
    int hitCount = 0;
    for (int l = 0; l < topK1.size(); ++l) {
        if (truth.find(topK1[l]) != truth.end()) {
            hitCount++;
        }
    }
    return min(1.0, (hitCount + (kparameter - kactual)) / (double) kparameter);
}

int main(int argc, char **argv) {
//    cerr << "./eval simpush <graphLabel> <eps> <numQuery> <k>" << endl;
    string algoname = argv[1];//simpush
    string graphLabel = argv[2];//dblp
    int num_queries = 50;//NUM of queries
    int topKparameter = 50; // the parameter k.
    char *cptr;
    num_queries = strtod(argv[4], &cptr);
    topKparameter = strtod(argv[5], &cptr);
    string epsilon;
    epsilon = argv[3];

    cout << graphLabel;
    cout << "; numQ: " << num_queries;
    cout << "; topK: " << topKparameter;
    cout << "; eps: " << epsilon << endl;

    double avg_avg_error = 0, avg_precision = 0;

    //load query nodes
    vector<int> query_set;
    //TODO make this a parameter
    string queryfile = "query/" + graphLabel + ".query";
    ifstream query_file(queryfile);
    int query_node;
    while (query_file >> query_node) {
        query_set.push_back(query_node);
    }

    for (int i = 0; i < num_queries; ++i) {
        int u = query_set[i];
        stringstream ss_gt;
        ss_gt << "groundtruth/" << graphLabel << "/" << u << "+1e-07_gt.txt";
//        cout << ss_gt.str() << endl;
        ifstream gtin(ss_gt.str());

        vector<pair<double, int> > gtanswers;
        vector<int> topk_gt_Nodes;
        int gt_tempNode;
        double gt_tempSim;
        //read all ground truth in a file
        while (gtin >> gt_tempNode >> gt_tempSim) {
            if (gt_tempSim < 0.000000001) break;
            gtanswers.push_back(make_pair(gt_tempSim, gt_tempNode));
        }
        if (gtanswers.size() == 1) {//itself
            gtanswers.clear();
        }
        sort(gtanswers.begin(), gtanswers.end(), greater<pair<double, int> >());

        int topkNum = topKparameter;// the actuall top K that may be modified below
        if (gtanswers.size() < topkNum) topkNum = gtanswers.size();
        for (int x = 0; x < topkNum; x++) {
            topk_gt_Nodes.push_back(gtanswers[x].second);
        }
        for (int y = topkNum; y < gtanswers.size() - topkNum; y++) {
            if (gtanswers[y].first != gtanswers[topkNum - 1].first) {
                break;
            } else {
                topk_gt_Nodes.push_back(gtanswers[y].second);
            }
        }

        stringstream ss_algo;
        ss_algo << "result/" << graphLabel << "_" << u << "_" << epsilon
                    << "_online.txt";
        ifstream algoin(ss_algo.str());
        vector<pair<double, int> > algoanswers;
        unordered_map<int, double> algoNidScores;
        int algo_tempNode;
        double algo_tempSim;
        while (algoin >> algo_tempNode >> algo_tempSim) {
            algoanswers.push_back(make_pair(algo_tempSim, algo_tempNode));
            algoNidScores[algo_tempNode] = algo_tempSim;
        }
        sort(algoanswers.begin(), algoanswers.end(), greater<pair<double, int> >());
        topkNum = min(topkNum, (int) algoanswers.size());

        vector<int> topk_algo_Nodes;
        //vector<double> topk_algo_values;
        for (int x = 0; x < topkNum; x++) {
            topk_algo_Nodes.push_back(algoanswers[x].second);
        }

        double max_err = 0, avg_err = 0, precision = 0;
        double tmp_err;
        for (int j = 0; j < topkNum; j++) {
            tmp_err = abs(gtanswers[j].first - algoNidScores[gtanswers[j].second]);
            if (max_err < tmp_err) {
                max_err = tmp_err;
            }
            avg_err += tmp_err;
        }
        avg_avg_error += avg_err / topKparameter;

        if (gtanswers.size() > 0 && gtanswers[topkNum - 1].first > 0) {
            precision = calPrecision(topk_gt_Nodes, topk_algo_Nodes, topkNum, topKparameter);
        } else
            precision = 1;
        avg_precision += precision;
    }
    avg_avg_error /= (double) num_queries;
    avg_precision /= (double) num_queries;
//    cout << "avg_avg_error=" << avg_avg_error << endl;
//    cout << "avg_precision=" << avg_precision << endl;
    cout << "avg error: " << avg_avg_error << endl;
    cout << "avg precision: " << avg_precision << endl;
    return 0;
}
