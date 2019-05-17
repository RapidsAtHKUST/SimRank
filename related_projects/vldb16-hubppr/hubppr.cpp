#define _CRT_SECURE_NO_DEPRECATE
#define HEAD_INFO

#include "mylib.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <map>
#include <stdlib.h>
#include <set>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "graph.h"
#include "config.h"
#include "algo.h"
#include "query.h"
#include "build_oracle.h"

#include <boost/progress.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 

#include <chrono>

GlobalPR *gpr;

using namespace std::chrono;

using namespace boost;
using namespace boost::property_tree;

using namespace std;


string get_time_path() {
    using namespace boost::posix_time;
    auto tm = second_clock::local_time();
#ifdef WIN32
    return  "../../execution/" + to_iso_string(tm);
#else
    return parent_folder+FILESEP+"execution/" + to_iso_string(tm);
#endif
}

#include <boost/program_options.hpp>

namespace po = boost::program_options;

// vector<vector<int> > fwd_idx_uncompressed;
// vector<vector<unordered_map<int,int> > > fwd_idx;

//continuous memory, 
//uncompressed version: e.g., dest-id,dest-id,dest-id...; 
//compressed_version: e.g., hub-id,occur,hub-id,occur...
vector<int> fwd_idx;

//pointers to uncompressed fwd_idx, nodeid:start-pointer
map<int, pair<int64, int> > fwd_idx_ucp_pointers;
//pointers to compressed fwd_idx, nodeid:{ start-pointer, start-pointer, start-pointer,...,end-pointer }
map<int, vector<int64> > fwd_idx_cp_pointers;
vector<vector<int64>> fwd_idx_ptrs;

// vector<int> fwd_idx_size;
vector<int> fwd_idx_size;
iMap<int> fwd_idx_size_k;

// vector<int> forward_node_order;
iMap<int> statistic_hit_number;

iMap<int> hub_used_samples;
vector<iMap<int>> multi_hub_used_samples;
iMap<int> dest_nodes;
vector<iMap<int>> multi_dest_nodes;

vector<unsigned> global_seeds;
vector<int> sample_number;
iMap<double> bwd_residuals;
iMap<double> bwd_reserves;
iMap<int> component;

using namespace std;

int main(int argc, char *argv[]) {
    ios::sync_with_stdio(false);
    program_start(argc, argv);

    // this is main entry
    Saver::init();
    config.graph_alias = "nethept";
    for (int i = 0; i < argc; i++) {
        string help_str = ""
                "hubppr build-oracle --algo <algo> [options]\n"
                "hubppr query --algo <algo> [options]\n"
                "hubppr topk --algo <algo> [options]\n"
                "hubppr select-forward-hub --algo <algo> [options]\n"
                "hubppr select-backward-hub --algo <algo> [options]\n"
                "hubppr generate-topk-query --algo <algo> [options]\n"
                "hubppr generate-p2p-query --algo <algo> [options]\n\n"
                "hubppr\n"
                "\n"
                "algo: \n"
                "  bippr\n"
                "  fastppr\n"
                "  montecarlo\n"
                "  hubppr\n"
                "options: \n"
                "  --target_size <target-size>\n"
                "  --k_size <k-size>\n"
                "  --compress_fwd\n"
                "  --prefix <prefix>\n"
                "  --dataset <dataset>\n"
                "   --query_size\n"
                "  --space_consumption <space>                    The space it uses\n"
                "  --target_sample <uniform|global-pagerank>      target sample\n"
                "  --force_rebuild                                force rebuild the oracle\n";
        if (string(argv[i]) == "--help") {
            cout << help_str << endl;
            exit(0);
        }
    }

    config.action = argv[1];
    cout << "action: " << config.action << endl;

    // init graph first
    for (int i = 0; i < argc; i++) {
        if (string(argv[i]) == "--prefix") {
            config.prefix = argv[i + 1];
        }
        if (string(argv[i]) == "--dataset") {
            config.graph_alias = argv[i + 1];
        }
    }


    config.graph_location = config.get_graph_folder();

    bool load_order = true;

    if(config.action == "select-forward-hub" || config.action == "generate-p2p-query")
        load_order = false;

    Graph graph(config.graph_location, load_order);
    INFO("load graph finish");


    INFO(graph.n, graph.m);
    for (int i = 0; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--algo") {
    		config.algo = string(argv[i + 1]);
        }
        else if (arg == "--target_sample") {
            config.target_sample = string(argv[i + 1]);
            INFO(config.target_sample);
            assert(config.target_sample == UNIFORM || config.target_sample == GLOBAL_PAGERANK || config.target_sample == RANDOM_WALK);
        }
        else if (arg == "--epsilon") {
            config.epsilon = atof(argv[i + 1]);
            INFO(config.epsilon);
        }
        else if (arg == "--space_consumption") {
            config.space_consumption = atof(argv[i + 1]);
            INFO(config.space_consumption);
        }
        else if (arg == "--target_size") {
            config.target_size = atoi(argv[i + 1]);
        }
        else if (arg == "--k_size"){
            config.k_size = atoi(argv[i+1]);
        }
        else if (arg == "--query_size"){
            config.query_size = atoi(argv[i+1]);
        }
        else if (arg == "--force_rebuild") {
            config.force_rebuild = true;
        }
        else if (arg == "--prefix" || arg == "--dataset") {
        }
        else if (arg.substr(0, 2) == "--") {
            cerr << "command not recognize " << arg << endl;
            exit(1);
        }
    }

    vector<string> possibleAlgo = {"bippr", "hubppr", "bipproracle",  "montecarlo", "fastppr"};
    auto f = find(possibleAlgo.begin(), possibleAlgo.end(), config.algo);
    assert (f != possibleAlgo.end());
    
    INFO(config.action);
    if( config.algo == "hubppr" && (config.action == "build-oracle" || config.action == "query")){
        gpr = new GlobalPR(config, graph);
    }

    init_parameter(config, graph);
   
    INFO("finished initing parameters");

    if (config.action == BUILD_ORACLE) {
        config.multithread = true;
        config.compress_fwd = true;
        hub_used_samples.initialize(graph.n);
        dest_nodes.initialize(graph.n);
        bwd_residuals.initialize(graph.n);
        bwd_reserves.initialize(graph.n);
        component.initialize(graph.n);
        tune_forward_backward_ratio(graph);
        build_oracle_entry(graph);
    }
    else if (config.action == QUERY) {
        config.compress_fwd = true;
        hub_used_samples.initialize(graph.n);
        dest_nodes.initialize(graph.n);
        bwd_residuals.initialize(graph.n);
        bwd_reserves.initialize(graph.n);
        component.initialize(graph.n);
        query(graph);
    }
    else if (config.action == TOPK) {
        config.compress_fwd = true;
        hub_used_samples.initialize(graph.n);
        dest_nodes.initialize(graph.n);
        bwd_residuals.initialize(graph.n);
        bwd_reserves.initialize(graph.n);
        component.initialize(graph.n);
        topk(graph);
    }
    else if (config.action == GEN_TOPK_QUERY){
        generate_topk_query(graph);
    }
    else if (config.action == GEN_P2P_QUERY){
        generate_p2p_query(graph);
    }
    else if (config.action == SELECT_FORWARD_HUB){
        select_forward_hub(graph);
    }
    else if (config.action == SELECT_BACKWARD_HUB){
        bippr_setting();
        bwd_residuals.initialize(graph.n);
        bwd_reserves.initialize(graph.n);
        select_backward_hub(graph);
    }
    else {
        cerr << "sub command not regoznized" << endl;
        exit(1);
    }

    auto args = combine_args(argc, argv);
    Saver::save_json(config, result, args);
    program_stop();
    return 0;
}
