#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <random>

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

    Graph(string data_folder, string action, bool init_global_ppr = true){
        if(config.action == "generate-p2p-query"){
            this->data_folder = data_folder;
            init_nm(); 
        }else{
            Graph(data_folder, init_global_ppr);
        }
    }

    Graph(string data_folder, bool init_global_ppr = true) {
        INFO("sub constructor");
        this->data_folder = data_folder;
        this->alpha = ALPHA_DEFAULT;
        init_graph();
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
            g[t1].push_back(t2);
            gr[t2].push_back(t1);
        }
    }


    double get_avg_degree() const {
        return double(m) / double(n);
    }


};


class GlobalPR {
public:
    GlobalPR(string alias) {
        read_global_ppr(alias);
    }

    GlobalPR(Config &config, const Graph& graph){
        init_order(config, graph);
    }

    vector<int> global_ppr;
    vector<pair<int, int>> global_ppr_with_index;

    vector<int> global_bwd_hubs;

    // rank[nodeid] <--> slow_rank(nodeid)
    vector<int> global_ppr_rank;

    double get_pr(int x) {
        return double(global_ppr[x]) / double(total_cnt);
    }

    /*
     * return rank of the nodeid
     */
    int slow_rank(int nodeid) {
        for (int i = 0; i < (int) global_ppr_with_index.size(); ++i) {
            if (global_ppr_with_index[i].second == nodeid)
                return i;
        }
        ASSERT(false);
        return 0;
    }

    int fast_rank(int nodeid) {
        return global_ppr_rank[nodeid];
    }


    void read_global_ppr(string alias) {
        string path = parent_folder + "global_pr" + FILESEP + alias + "_globalppr.csv";
        assert_file_exist("globalpr", path);
        ifstream cin(path);
        int t, x, y;
        while (cin >> t >> x >> y) {
            total_cnt = y;
            global_ppr.push_back(x);
            global_ppr_with_index.push_back(MP(x, t));
        }
        sort(global_ppr_with_index.begin(), global_ppr_with_index.end(), greater<pair<int, int>>());

        for (int i = 0; i < (int) global_ppr_with_index.size(); ++i)
            global_ppr_rank.push_back(0);
        for (int i = 0; i < (int) global_ppr_with_index.size(); ++i) {
            global_ppr_rank[global_ppr_with_index[i].second] = i;
//                return i;
        }
    }


    void init_order(Config &config, const Graph& graph){
        string input_path = config.prefix + "global_pr" + FILESEP;
        input_path +=config.graph_alias+".forward.order";
        assert_file_exist("globalpr", input_path);
        ifstream order_file(input_path);

        for(int i=0; i<graph.n; i++){
            global_ppr.push_back(0);
        }

        int v;
        int lines=0;
        int hit_count, benefit, total_rw;
        while(order_file>>v){
            order_file>>hit_count;

            global_ppr[v]= hit_count;
            global_ppr_with_index.push_back(MP(hit_count, v));
            
            order_file>>benefit;//discard this benefit
            order_file>>total_rw;

            total_cnt = total_rw;
            
            lines++;
        }

        assert(lines == graph.n);

        //sort(global_ppr_with_index.begin(), global_ppr_with_index.end(), greater<pair<int, int>>());
        for (int i = 0; i < (int) global_ppr_with_index.size(); ++i)
            global_ppr_rank.push_back(0);
        for (int i = 0; i < (int) global_ppr_with_index.size(); ++i) {
            global_ppr_rank[global_ppr_with_index[i].second] = i;
//                return i;
        }

        string bwd_hub_path = config.prefix + "global_pr" + FILESEP;
        bwd_hub_path += config.graph_alias+".backward.order";
        assert_file_exist("globalpr", bwd_hub_path);
        ifstream bwd_order_file(bwd_hub_path);

        global_bwd_hubs.resize(graph.n);
        unordered_map<int, bool> hub_mark;
        int i=0;
        while(bwd_order_file>>v){
            global_bwd_hubs[i] = v;
            hub_mark[v] = true;
            i++;
        }

        for(int u=0; u<graph.n; u++){
            if(hub_mark[u]!=true){
                global_bwd_hubs[i]=u;
                i++;
            }
        }

    }


    /*
     * return i-th most important nodeid
     */
    int get_nodeid_by_globalpr(int i) {
        return global_ppr_with_index[i].second;
    }

    int get_bwd_hub_by_order(int i){
        return global_bwd_hubs[i];
    }

    int sample_by_pr() {
        static std::random_device rd;
        static std::mt19937 generator(rd());
        static discrete_distribution<int> distribution(global_ppr.begin(), global_ppr.end());
        int startnode = distribution(generator);
        return startnode;
    }

    int total_cnt;

};


class DiskGraph {
public:
    FILE *fedges;
    FILE *foffset;

    DiskGraph(string prefix, string alias, bool transpose = false) {
        string name;
        if (transpose) {

            name = prefix + FILESEP + alias + FILESEP + alias + "-t.edges";
            assert_file_exist("edges", name);
            fedges = fopen(name.c_str(), "rb");

            name = prefix + FILESEP + alias + FILESEP + alias + "-t.offset";
            assert_file_exist("offset", name);
            foffset = fopen(name.c_str(), "rb");

        }
        else {
            name = prefix + FILESEP + alias + FILESEP + alias + ".edges";
            assert_file_exist("edges", name);
            fedges = fopen(name.c_str(), "rb");

            name = prefix + FILESEP + alias + FILESEP + alias + ".offset";
            assert_file_exist("offset", name);
            foffset = fopen(name.c_str(), "rb");
        }
        init_nm(prefix, alias);
    }

    int n;
    long long m;

    void init_nm(string prefix, string alias) {
        string attribute_file = prefix + FILESEP + alias + FILESEP + "attribute.txt";
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

    int edge_count(int x) {
        if (x == 0) {
            int buf;
            fseek(foffset, x * 8, SEEK_SET);
            fread(&buf, sizeof buf, 1, foffset);
            return buf;
        }
        else {
            long long buf, buf2;
            fseek(foffset, (x - 1) * 8, SEEK_SET);
            fread(&buf, sizeof buf, 1, foffset);
            fread(&buf2, sizeof buf, 1, foffset);
            return int(buf2 - buf);
        }
    }

    long long get_start_pos(int x) {
        if (x == 0)
            return 0;
        else {
            long long buf;
            fseek(foffset, (x - 1) * 8, SEEK_SET);
            fread(&buf, sizeof buf, 1, foffset);
            return buf * 4;
        }

    }

    vector<int> edges(int x) {
        vector<int> t;
        long long start = get_start_pos(x);
        int len = edge_count(x);
        fseek(fedges, start, SEEK_SET);
        for (int i = 0; i < len; i++) {
            int buf;
            fread(&buf, sizeof buf, 1, fedges);
            t.push_back(buf);
        }
        return t;
    }

    double get_avg_degree() const {
        return double(m) / double(n);
    }


};

extern GlobalPR *gpr;

static void init_parameter(Config &config, const Graph &graph) {
    // init the bwd delta, fwd delta etc

//    double delta = 1.0 / double(graph.n);
//    double pfail = 1.0 / double(graph.n);
    INFO("init parameters", graph.n);
    config.delta = 1.0 / graph.n;
    config.pfail = 1.0 / graph.n; //0.001;
    INFO(config.delta);
    INFO(config.pfail);

    config.dbar = double(graph.m) / double(graph.n); // average degree

//    double dbar = double(graph.m) / double(graph.n); // average degree
//    config.bwd_delta = sqrt(config.delta) * config.epsilon * sqrt(dbar / log(2 / config.pfail);
//
//    double fwd_rw_count = config.bwd_delta / delta / config.epsilon / config.epsilon * log(1 / config.pfail;
//    config.fwd_delta = 1.0 / fwd_rw_count;


}



#endif
