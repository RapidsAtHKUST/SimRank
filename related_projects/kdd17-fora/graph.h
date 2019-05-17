#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "mylib.h"
#include "config.h"

class Graph {
public:

    vector<vector<int>> g;
    vector<vector<int>> gr;
    string data_folder;

    //vector<double> global_ppr;

    // node rank[100] = 0, means node 100 has first rank
    vector<int> node_rank;
    // node_score[0]
    vector<double> node_score;

    //node order 0 = [100, 34.5], most important node is 100 with score 34.5
    vector<pair<int, double>> node_order;
    vector<int> loc;


    // the tele ratio for random walk
    double alpha;

    static bool cmp(const pair<int, double> &t1, const pair<int, double> &t2) {
        return t1.second > t2.second;
    }

    int n;
    long long m;

    Graph(string data_folder) {
        INFO("sub constructor");
        this->data_folder = data_folder;
        this->alpha = ALPHA_DEFAULT;
        if(config.action != GEN_SS_QUERY)
            init_graph();
        else
            init_nm();
        cout << "init graph n: " << this->n << " m: " << this->m << endl;
    }


    void init_nm() {
        string attribute_file = data_folder + FILESEP + "attribute.txt";
        assert_file_exist("attribute file", attribute_file);
        ifstream attr(attribute_file);
        string line1, line2;
        char c;
        while (true) {
            attr >> c;
            if (c == '=') break;
        }
        attr >> n;
        while (true) {
            attr >> c;
            if (c == '=') break;
        }
        attr >> m;
    }

    void init_graph() {
        init_nm();
        g = vector<vector<int>>(n, vector<int>());
        gr = vector<vector<int>>(n, vector<int>());
        string graph_file = data_folder + FILESEP + "graph.txt";
        assert_file_exist("graph file", graph_file);
        FILE *fin = fopen(graph_file.c_str(), "r");
        int t1, t2;
        while (fscanf(fin, "%d%d", &t1, &t2) != EOF) {
            assert(t1 < n);
            assert(t2 < n);
            if(t1 == t2) continue;
            g[t1].push_back(t2);
            gr[t2].push_back(t1);
        }
    }


    double get_avg_degree() const {
        return double(m) / double(n);
    }


};



static void init_parameter(Config &config, const Graph &graph) {
    // init the bwd delta, fwd delta etc

    INFO("init parameters", graph.n);
    config.delta = 1.0 / graph.n;
    config.pfail = 1.0 / graph.n; 

    config.dbar = double(graph.m) / double(graph.n); // average degree


}



#endif
