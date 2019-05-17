#ifndef __CONFIG_H__
#define __CONFIG_H__


#ifdef WIN32
#define FILESEP "\\"
#else
#define FILESEP "/"
#endif

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/progress.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost;
using namespace boost::property_tree;

const int TIMER_TOPK = 81;
const int TIMER_TOPK_ELEGANT = 82;
const int TIMER_SELECT_FORWARD_HUB = 83;
const int TIMER_SELECT_BWDWARD_HUB = 83;

const double ALPHA_DEFAULT = 0.2;

const int FWD = 0;
const int HUB = 1;

const int NUM_OF_QUERY = 20;

const string LARGE_DEGREE = "LARGE_DEGREE";
const string LARGE_GLOBAL_PR = "LARGE_GLOBAL_PR";

const string BUILD_ORACLE = "build-oracle";
const string QUERY = "query";
const string TOPK = "topk";
const string GEN_TOPK_QUERY = "generate-topk-query";
const string GEN_P2P_QUERY = "generate-p2p-query";
const string SELECT_FORWARD_HUB = "select-forward-hub";
const string SELECT_BACKWARD_HUB = "select-backward-hub";

typedef pair<map<int, double>, map<int, double>> Bwdidx;
//              pi               residual
typedef pair<double, Bwdidx> BwdidxWithResidual;
static map<int, vector<BwdidxWithResidual>> idx;

static double bwd_prune_ratio = 0.75;

static double calculate_bwd_delta_bippr_and_hubppr(double delta, double epsilon, double dbar, double pfail){
    // return sqrt(delta) * epsilon * sqrt(dbar / log(2 / pfail)/(2+epsilon))*bwd_prune_ratio;
    return epsilon * sqrt(delta*dbar / 3.0 / log(2 / pfail))*bwd_prune_ratio;
}

static double calculate_fwd_count_bippr_and_hubppr( double bwd_delta, double delta, double epsilon, double pfail){
    // return (2+epsilon)*bwd_delta / delta / epsilon / epsilon * log(2 / pfail);
    return 3*bwd_delta / delta / epsilon / epsilon * log(2 / pfail);
}



#ifdef WIN32
const string parent_folder = "../../";
#else
const string parent_folder = string("./") + FILESEP;
#endif


const string NAIVE = "naive";
const string USE_ORACLE = "use-oracle";
const string FULL_PRECOMPUTE = "full-precompute";
const string UNIFORM = "uniform";
const string GLOBAL_PAGERANK = "global-pagerank";
const string RANDOM_WALK = "random-walk";

class Config {
public:
    string graph_alias;
    string graph_location;

    string action = ""; // query/generate index, etc..

    string prefix = "d:\\dropbox\\research\\data\\";

    string get_graph_folder() {
        return prefix + graph_alias + FILESEP;
    }

    bool multithread = false;
    unsigned num_thread = 1;

    string fwd_method = "naive";
    // naive, usehub, full-precompute
    string bwd_method = "naive";
    // naive, usehub, full-precompute

    string target_sample = UNIFORM; // uniform, global-pagerank
    bool target_set_all = false;

    double fwd_delta; // 1/omega  omega = # of random walk
    double bwd_delta; // identical to r_max

    double fwd_cost_ratio = 1; //the time consumption ratio of fwd to bwd

    double bwd_prune_ratio = 5;
    bool bwd_prune = false;
    bool no_forward = false;

    int query_size = 1000;
    bool no_backward = false;
    bool test_forward = false;

    string node_score_method = LARGE_GLOBAL_PR;

    // how much space hubppr can use to store oracle 2x means it can use 2x space of original graph, i.e, 4*m of numbers
    // because we need to store outgoing and incoming edges for each node, so we need to store 2*m numbers in memory to represent the graph
    double space_consumption = 0;


    double pfail = 0;
    double dbar = 0;
    double epsilon = 0;
    double delta = 0;

    double query_seconds; // how many seconds query run
    double oracle_seconds = 10; // how many seconds query run

    double target_size; // how many seconds query run
    int k_size;


    string algo; // bippr, bipproracle, hubppr,  naive, naiveindex, it, itindex


    string global_ppr_path() {
        return prefix + "global_pr" + FILESEP + graph_alias + "_globalppr.csv";
    }

    double alpha = ALPHA_DEFAULT;
    int num_of_hubs;
    int num_of_fwd_hubs;
    int num_of_fwd_rw;
    bool compress_fwd = false;

    ptree get_data() {
        ptree data;
        data.put("graph_alias", graph_alias);
        data.put("action", action);

        data.put("alpha", alpha);

        data.put("num_of_hubs", num_of_hubs);
        data.put("fwd_delta", fwd_delta);
        data.put("bwd_delta", bwd_delta);
        data.put("num_of_fwd_rw", num_of_fwd_rw);


        data.put("pfail", pfail);
        data.put("epsilon", epsilon);
        data.put("delta", delta);


        data.put("bwd_prune", bwd_prune);

        data.put("bwd_prune_ratio", bwd_prune_ratio);


        data.put("compress_fwd", compress_fwd);

        data.put("node_score_method", node_score_method);

        data.put("algo", algo);
        return data;

    }

    bool use_bwd_index = false;
    bool force_rebuild = false;
};

class Result {
public:
    int n;
    long long m;
    double fwd_label_size;
    double hub_label_size;
    double bwd_label_size;

    int count_exist = 0;
    int count_residual = 0;
    int count_large_residual = 0;
    unordered_map<int, double> max_residual;
    unordered_map<int, vector<pair<int, pair<double, double>>>> largeincome_residual;

    int num_nodes_larger_than_indegree_threshold;

    double total_size;

    int64 num_randwalk=0;

    int num_block_by_hub=0;
    int num_use_backward_oracle=0;
    int count_propagate;
    int count_propagate_edge;
    double time_spent;
    int finished_queries;

    ptree get_data() {
        ptree data;
        data.put("n", n);
        data.put("m", m);

        data.put("num_block_by_hub", num_block_by_hub);
        data.put("num_use_backward_oracle", num_use_backward_oracle);
        //data.put("hub_label_size_x", hub_label_size / double(m));
        //data.put("fwd_label_size_x", fwd_label_size / double(m));
        //data.put("bwd_label_size_x", bwd_label_size / double(m));

        data.put("total_size_x", total_size / double(m));
        data.put("time_spent", time_spent);
        data.put("finished_query", finished_queries);
        return data;
    }

};

extern Config config;
extern Result result;

bool exists_test(const std::string &name);

void assert_file_exist(string desc, string name);

namespace Saver {
    static string get_current_time_str() {
        time_t rawtime;
        struct tm *timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
        std::string str(buffer);

        return str;

    }

    static string get_time_path() {
        using namespace boost::posix_time;
        auto tm = second_clock::local_time();
        return parent_folder+FILESEP+"execution/" + to_iso_string(tm);
    }

    static ptree combine;

    static void init() {
        combine.put("start_time", get_current_time_str());
    }


    static void save_json(Config &config, Result &result, vector<string> args) {
        ofstream fout(get_time_path() + "_result.json");
        string command_line = "";
        for (int i = 1; i < args.size(); i++) {
            command_line += " " + args[i];
        }
        combine.put("end_time", get_current_time_str());
        combine.put("command_line", command_line);
        combine.put_child("config", config.get_data());
        combine.put_child("result", result.get_data());
        ptree timer;
        for (int i = 0; i < (int) Timer::timeUsed.size(); i++) {
            if (Timer::timeUsed[i] > 0) {
                timer.put(to_str(i), Timer::timeUsed[i] / TIMES_PER_SEC);
            }
        }
        combine.put_child("timer", timer);

        write_json(fout, combine, true);
    }
};

#endif
