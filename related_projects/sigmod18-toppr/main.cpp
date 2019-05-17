#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "ppr.h"
#include <unordered_set>    
#include <cstdlib>
#include <cstring>


void usage() {
    cerr << "AdaptivePPR [-d <dataset>] [-k topk (default 500)] [-n node_count (default 20)] [-r error_rate (default 1)] [-err error_eps (default 0)]" << endl;
}

int check_inc(int i, int max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}

bool maxCmp(const pair<int, double>& a, const pair<int, double>& b){
    return a.second > b.second;
}

vector<int> getRealTopK(int s, int k, string target_filename, int vert){
    stringstream ss;
    ss << "ppr-answer/" << target_filename << "/" << s << ".txt";
    string infile = ss.str();
    ifstream real(infile);
    vector<int> realList;
    vector<double> simList;
    for(int i = 0; i < vert; i++){
        int tempId; 
        double tempSim;
        real >> tempId >> tempSim;
        if(i >= k && tempSim < simList[k-1]){
           break; 
        } 
        realList.push_back(tempId);
        simList.push_back(tempSim);
    }
    real.close();
    return realList;
}

unordered_map<int, double> getRealTopKMap(int s, int k, string target_filename, int vert){
    unordered_map<int, double> answer_map;
    stringstream ss;
    ss << "ppr-answer/" << target_filename << "/" << s << ".txt";
    string infile = ss.str();
    ifstream real(infile);
    double k_Sim = 0;
    for(int i = 0; i < vert; i++){
        int tempId;
        double tempSim;
        real >> tempId >> tempSim;
        if(i == k - 1){
            k_Sim = tempSim;
        }
        if(i >= k && tempSim < k_Sim){
            break;
        }
        answer_map[tempId] = tempSim;
    }
    real.close();
    return answer_map;
}

int main(int argc, char *argv[]){
    int i = 1;
    char *endptr;
    string filename;
    double eps = 0.05;                    //forward adaptive parameter
    int k = 500;                   //parameter k
    double alpha = 0.2;            //decay factor
    int node_count = 20;           //query node size
    double error_rate = 1;         //precision parameter
    double error_eps = 0.0000000001;          //min error
    string algo = "TopPPR";
    if(argc < 7){
        usage();
        exit(1);
    }
    while (i < argc) {
        if (!strcmp(argv[i], "-d")) {
            i = check_inc(i, argc);
            filename = argv[i];
        } 
        else if (!strcmp(argv[i], "-algo")) {
            i = check_inc(i, argc);
            algo = argv[i];
        }
        else if (!strcmp(argv[i], "-k")) {
            i = check_inc(i, argc);
            k = strtod(argv[i], &endptr);
            if ((k < 0) && endptr) {
                cerr << "Invalid k argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-n")) {
            i = check_inc(i, argc);
            node_count = strtod(argv[i], &endptr);
            if ((node_count < 0) && endptr) {
                cerr << "Invalid node_count argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-r")) {
            i = check_inc(i, argc);
            error_rate = strtod(argv[i], &endptr);
            if (((error_rate < 0) || (error_rate > 1)) && endptr) {
                cerr << "Invalid error_rate argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-err")) {
            i = check_inc(i, argc);
            error_eps = strtod(argv[i], &endptr);
            if (((error_eps < 0) || (error_eps > 1)) && endptr) {
                cerr << "Invalid error_eps argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-a")) {
            i = check_inc(i, argc);
            alpha = strtod(argv[i], &endptr);
            if (((alpha < 0) || (alpha > 1)) && endptr) {
                cerr << "Invalid alpha argument" << endl;
                exit(1);
            }
        }
        else {
            usage();
            exit(1);
        }
        i++;
    }
    
    PPR ppr = PPR(filename, error_rate, error_eps, k, alpha);
    if(algo == "GEN_QUERY"){
        ofstream outFile("dataset/" + filename + ".query");
        ppr.generateQueryNode(node_count, outFile);
        outFile.close(); 
    }
    else if(algo == "GEN_GROUND_TRUTH"){
        string queryname = "dataset/" + filename + ".query";
        if(!ppr.is_file_exist(queryname)){
            cout << "please generate query file first" << endl;
        }
        else
            ppr.PowerMethodMulti(100, node_count, 10);/*  多线程PowerMethparameter: iteration loops, node size, thread num */
    }
    else{
        string queryname = "dataset/" + filename + ".query";
        if(!ppr.is_file_exist(queryname)){
            cout << "please generate query file first" << endl;
            return 0;
        }
        if(error_rate != 1)
            eps = 0.5;
        ifstream nodes_file("dataset/" + filename + ".query");
        vector<int> test_nodes;
        while(!nodes_file.eof()){
            int temp_node;
            nodes_file >> temp_node;
            test_nodes.push_back(temp_node);
        }
        cout << "read done!" << endl;
        // int kkk[11] = {1,2,4,8,16,32,64,128,256,512,1024};
        for(int para = 0; para < 1; para++){
            //k = kkk[para];
            //cout<<"k="<<k<<endl;
            if(k > ppr.g.n){
                break;
            }
            int realCount = 0;
            int totalQuery = test_nodes.size();
            for(int t = 0; t < node_count; t++){
                if(realCount == totalQuery){
                    cout << "too many query node than query file" << endl;
                    return 0;
                }
                int test_node = test_nodes[realCount++];
                //cout << "node: " << test_node << " " << eps << " " << k << endl;
                stringstream ss;
                ss << "ppr-answer/" << filename << "/" << test_node << ".txt";
                string infile = ss.str();
                if(!ppr.is_file_exist(infile)){
                    cout << "node:" << test_node << " groundtruth file not found, please generate groundtruth first" << endl;
                    return 0;
                }
                vector<int> realList = ppr.getRealTopK(test_node, k+1);
                unordered_map<int, double> realMap = ppr.getRealTopKMap(test_node, k+1);
                if(ppr.g.getOutSize(test_node) == 0){
                    t--;
                    node_count--;
                    continue;
                }
                if(realMap[realList[k - 1]] < 0.0000000001){
                    t--;
                    node_count--;
                    cout << "Top K value is too small" << endl;
                    continue;
                }
                if(realMap[realList[k - 1]] > 0.0000000001 && realMap[realList[k - 1]] - realMap[realList[k]] < 0.0000000001){
                    t--;
                    node_count--;
                    cout << "gap too small!" << endl;
                    continue;
                }
                double real_eps = eps * 800 * sqrt(1/ (double) ppr.g.m / (double) ppr.g.n / log(ppr.g.n)) / (double) log(k) / max(0.0001, pow(error_rate, 3));
                //TopPPR算法
                vector<pair<int, double> > topkppr = ppr.TopPPR(test_node, real_eps, k);     
            }
            cout << "avg precision: " << ppr.avg_pre / (double) node_count << endl;
            cout << "avg time: " << ppr.avg_time / (double) node_count << endl;
            ppr.avg_time = 0;   
            ppr.avg_pre = 0; 
        }
        for(int i = 0; i < argc; i++){
            cout << argv[i] << " ";
        }
        cout << endl;
    }
    return 0;
};
