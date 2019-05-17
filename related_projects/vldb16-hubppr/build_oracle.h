#ifndef HUBPPR_BUILD_BACKWARD_ORACLE_H
#define HUBPPR_BUILD_BACKWARD_ORACLE_H

#include "mylib.h"
#include "graph.h"
#include "config.h"
#include "algo.h"

#include <mutex>
#include <atomic>
#include <future>
#include <algorithm>
extern iMap<int> component;
static Bwdidx generate_full_backward_oracle(int nodeid, const Graph &graph, ofstream &fout) {
    // return the space consumption
    // return 1 means use same size as dataset size (which is n+2m)

    // used in bippr, every node will generate its backward snapshot with no limit

    double residual = 1;
//    INFO(nodeid, residual);
    Bwdidx rtn = reverse_local_update_heap_thread_for_build_oracle(nodeid, graph, residual);

    if (fout.is_open()) {
        fout << nodeid << endl;
        fout << residual << endl;
        fout << rtn.first.size() << endl;
        for (auto v: rtn.first)
            fout << v.first << " " << v.second << " ";
        fout << endl;
        fout << rtn.second.size() << endl;
        for (auto v: rtn.second)
            fout << v.first << " " << v.second << " ";
        fout << endl;
    }
    return rtn;
}

static void build_forward_oracle_hubppr(const Graph& graph){
    static int64 space(2*config.space_consumption * graph.m*config.fwd_cost_ratio);
    // INFO("conmpress fwd:", config.compress_fwd);

    fwd_idx.reserve(2*space);
    static int64 num_tries = int64(1 / config.fwd_delta );
        
    for (int i = 0; i < graph.n; ++i) {
        if(space >0){
            //select a hub
            int nodeid = gpr->get_nodeid_by_globalpr(i);
            int64 increased_size =0;
            //generate destination multiset for the selected hub by running random walk
            auto t = generate_fwd_label_compressed_binary_multi_set(i, nodeid, graph, num_tries);

            // fwd_idx_cp_pointers[nodeid].reserve(t.size()+1);
            fwd_idx_cp_pointers[nodeid].resize(t.size()+1);
            int j=0;
            for(auto &m_set: t){
                // fwd_idx_cp_pointers[nodeid].push_back(fwd_idx.size()); // push start-pointer
                fwd_idx_cp_pointers[nodeid][j] = fwd_idx.size();
                j++;
                increased_size += m_set.size();
                for(auto &ele: m_set){
                    fwd_idx.push_back(ele.first);  //push node
                    fwd_idx.push_back(ele.second);  //push node occur
                }
            }

            // fwd_idx_cp_pointers[nodeid].push_back(fwd_idx.size()-1);
            fwd_idx_cp_pointers[nodeid][j] = fwd_idx.size()-1;
            space -= increased_size;
        }

        //select hubs and generate multiset for them until memory is exhausted
        if(space <= 0 || i == graph.n -1 )
            break;
    }
}

static void build_backward_oracle_hubppr(const Graph &graph) {
    assert(config.bwd_delta > 0);
    // used for hubppr, make sure that the size is smaller than config.space_consumption * m
    int64 space(2*config.space_consumption * graph.m*(1-config.fwd_cost_ratio));
    INFO("space for bwd oracle", space);
    // bool stop=false;

    int num_idx_per_node = log(config.bwd_delta/2)/log(config.alpha/2);

    for (int i = 0; i < graph.n; i++) {
        int nodeid = gpr->get_nodeid_by_globalpr(i);
        // int nodeid = gpr->get_bwd_hub_by_order(i);

        if (space <= 0)// || stop==true)
            return;
        
        idx[nodeid].reserve(num_idx_per_node);
        int64 hub_space=0;
        for (double residual = config.alpha; residual > config.bwd_delta / 2; residual /= 2) {
            Bwdidx rtn = reverse_local_update_heap_thread_for_build_oracle(nodeid, graph, residual);
            // Bwdidx rtn = reverse_local_update_linear(nodeid, graph, residual);
            // if(residual == config.alpha && space-2*hub_space<0){
            //     stop = true;
            //     return;              
            // }

            if (idx[nodeid].size()==0 || rtn.second.size() < idx[nodeid].back().second.second.size() / 2) {
                hub_space+=(rtn.first.size() + rtn.second.size());
                idx[nodeid].push_back(MP(residual, rtn));
            }
        }
        space-=hub_space;
    }
}

static void build_oracle_bippr(const Graph &graph) {
    string target_file_bippr = config.prefix + FILESEP + config.graph_alias+FILESEP+ "bippr." + config.graph_alias + ".eps" + to_str(config.epsilon) + ".idx";
    string target_file_bippr_meta = replace(target_file_bippr, ".idx", ".meta");

    ofstream fout;
    assert(!fout.is_open());

    //forward
    int cnt_forward = 0;
    double size_forward = 0;
    double graph_size = double(graph.n + 2 * graph.m);
    while (true) {
        Timer tm(2, "generate forward oracle for bippr");
        int i = lrand() % graph.n;
        // auto fwd = sample_fwd(i, graph);
        sample_fwd(i, graph);
        if (Timer::used(2) > config.oracle_seconds)
            break;
        cnt_forward++;
        // for (auto item:fwd)
        for(int x=0; x<dest_nodes.occur.m_num; x++){
            int node = dest_nodes.occur[x];
            size_forward += dest_nodes[node];
        }
    }

    //backward
    int cnt_backward = 0;
    double size_backward = 0;
    while (true) {
        Timer tm(3, "generate backward oracle for bippr");
        int i = lrand() % graph.n;
        auto rtn = generate_full_backward_oracle(i, graph, fout);
        if (Timer::used(3) > config.oracle_seconds)
            break;
        cnt_backward++;
        size_backward += rtn.first.size() + rtn.second.size();
    }

    //output
    ofstream fout_meta(target_file_bippr_meta);
    fout_meta << "time used: " << Timer::used(2) << endl;

    double est_time_forward = (Timer::used(2)) / double(cnt_forward) * double(graph.n);
    double est_time_backward = Timer::used(3) / double(cnt_backward) * double(graph.n);
    double est_time = est_time_forward + est_time_backward;

    fout_meta << "estimate forward time: " << est_time_forward << endl;
    fout_meta << "estimate backward time: " << est_time_backward << endl;
    fout_meta << "estimate time: " << est_time << endl;

    double est_size = size_forward / double(cnt_forward) * double(graph.n);
    est_size += size_backward / double(cnt_backward) * double(graph.n);
    est_size /= graph_size;
    fout_meta << "estimate size: " << est_size << endl;

}

extern vector<int> sample_number;


static int paralle_level=1;
void eachcore_build_forward(int64 space, unsigned core_id,  const Graph& graph, vector<int>& eachcore_fwd_idx, unordered_map<int, vector<int64>>& eachcore_fwd_idx_pointers ){
    eachcore_fwd_idx.reserve(2*space);

    for (int i = 0; i < graph.n/paralle_level; ++i) {
        if(space >0){
            int nodeid = gpr->get_nodeid_by_globalpr(i*paralle_level + core_id);
            //assert(sample_number[nodeid] !=0);

            int64 num_tries = compute_num_of_tries(nodeid, graph.n);
            //int64(1.0/config.fwd_delta/graph.n*sample_number[nodeid] );
                
            int64 increased_size =0;
            if(graph.g[nodeid].size()==0){
                INFO("skip this node, no out-neighbor");
                continue;
            }

            auto t = generate_fwd_label_compressed_binary_multi_set_thread(i*paralle_level + core_id, nodeid, graph, num_tries, core_id);

            int j=0;
            eachcore_fwd_idx_pointers[nodeid].resize(t.size()+1);
            for(auto &m_set: t){
                // eachcore_fwd_idx_pointers[nodeid].push_back(eachcore_fwd_idx.size()); // push start-pointer
                eachcore_fwd_idx_pointers[nodeid][j] = eachcore_fwd_idx.size();
                j++;
                increased_size += m_set.size();
                for(auto &ele: m_set){
                    eachcore_fwd_idx.push_back(ele.first);  //push node
                    eachcore_fwd_idx.push_back(ele.second);  //push node occur
                }
            }

            // eachcore_fwd_idx_pointers[nodeid].push_back(eachcore_fwd_idx.size()-1);  
            eachcore_fwd_idx_pointers[nodeid][j] = eachcore_fwd_idx.size()-1;

            space -= increased_size;
        }

        if(space <= 0 || i >= graph.n/paralle_level -1)
            break;
    }
}


void eachcore_build_backward(int64 space, unsigned core_id, const Graph &graph, map<int, vector<BwdidxWithResidual>>& idx_each_core){
    // static bool stop = false;
    // static atomic<int> counter(0);
    // static atomic<int64> space(config.space_consumption * graph.m);

    int num_idx_per_node = log(config.bwd_delta/2)/log(config.alpha/2);

    for(int i=0; i<graph.n/paralle_level; i++){
        if (space <= 0 )// || stop == true){
            return;

        int order = i*paralle_level + core_id;
        int nodeid = gpr->get_nodeid_by_globalpr(order);
        // int nodeid = gpr->get_bwd_hub_by_order(order);

        idx_each_core[nodeid].reserve(num_idx_per_node);
        int64 hub_space=0;
        for (double residual = config.alpha; residual > config.bwd_delta / 2; residual /= 2) {
            // Bwdidx rtn = reverse_local_update_linear_thread(nodeid, graph, residual);
            Bwdidx rtn = reverse_local_update_heap_thread_for_build_oracle(nodeid, graph, residual);

            // if(residual == config.alpha && space<0){//space-2*hub_space<0){
            //     stop = true;
            //     return;              
            // }

            if (idx_each_core[nodeid].empty() || rtn.second.size() < idx_each_core[nodeid].back().second.second.size() / 2) {
                hub_space += (rtn.first.size() + rtn.second.size());
                idx_each_core[nodeid].push_back(MP(residual, rtn));
            }
        }
        space-=hub_space;
    }
}

static void multicore_build_oracle_hubppr(const Graph &graph) {
    // build oracle for hubppr
    assert(config.space_consumption > 0);
    unsigned NUM_CORES = std::thread::hardware_concurrency();
    if(NUM_CORES > 32) NUM_CORES = 32;
    assert(NUM_CORES >= 2);

    string fwd_idx_path = get_fwd_idx_file_name();//replace(oracle_path, ".idx", ".meta");
    string fwd_idx_info_path = get_fwd_idx_info_file_name();
    string bwd_idx_path = get_bwd_idx_file_name();//replace(meta_path_forward, ".fwd.", ".bwd.");
    string bwd_idx_info_path = get_bwd_idx_info_file_name();

    paralle_level = NUM_CORES - 1;

    //forward
    INFO("construting forward oracle");
    if (!config.force_rebuild && file_exists_test(fwd_idx_path) ) {
        cerr << "forward oracle already exist " << endl;
    }
    else{
        Timer tm(2, "forward oracle construction");

        int64 space(2*config.space_consumption * graph.m*config.fwd_cost_ratio);
        int64 space_per_core = space/paralle_level;
        int64 space_for_first_core = space_per_core + space%paralle_level;

        std::vector< std::future<void> > futures(NUM_CORES-1);
        std::vector< vector<int> > eachcore_fwd_idx(NUM_CORES-1);
        std::vector< unordered_map<int, vector<int64>> > eachcore_fwd_idx_pointers(NUM_CORES-1); 
        
        for(unsigned i=0; i< NUM_CORES-1; i++){
            if(i==0)
                futures[i] = std::async( std::launch::async, eachcore_build_forward, space_for_first_core, i, std::ref(graph), std::ref(eachcore_fwd_idx[i]), std::ref(eachcore_fwd_idx_pointers[i]) );
            else
                futures[i] = std::async( std::launch::async, eachcore_build_forward, space_per_core, i, std::ref(graph), std::ref(eachcore_fwd_idx[i]), std::ref(eachcore_fwd_idx_pointers[i]) );
        }
        std::for_each( futures.begin(), futures.end(), std::mem_fn(&std::future<void>::wait));

        {
            Timer t(9, "merge fwd idx from each core");
            //merge fwd idx from each core into one
            for(unsigned i=0; i<eachcore_fwd_idx.size(); i++){
                for(auto &a_fwd_pointers: eachcore_fwd_idx_pointers[i]){
                    std::transform(a_fwd_pointers.second.begin(), a_fwd_pointers.second.end(), a_fwd_pointers.second.begin(), std::bind2nd(std::plus<int64>(), fwd_idx.size()));
                }

                fwd_idx_cp_pointers.insert(eachcore_fwd_idx_pointers[i].begin(), eachcore_fwd_idx_pointers[i].end());

                fwd_idx.insert( fwd_idx.end(), eachcore_fwd_idx[i].begin(), eachcore_fwd_idx[i].end() );
                eachcore_fwd_idx[i].clear();
                eachcore_fwd_idx_pointers[i].clear();
            }
        }

        {
            Timer t(11, "serialize fwd oracle");
            fwd_idx.shrink_to_fit();
            serialize_fwd_idx();
        }
    }

    //backward
    INFO("construting backward oracle");
    if ( !config.force_rebuild && file_exists_test(bwd_idx_path) ) {
        cerr << "backward oracle already exist " << endl;
    }
    else{
        Timer tm(3, "backward oracle construction");

        int64 space(2*config.space_consumption * graph.m*(1-config.fwd_cost_ratio));
        int64 space_per_core = space/paralle_level;
        int64 space_for_first_core = space_per_core + space%paralle_level;

        std::vector< std::future<void> > futures(NUM_CORES-1);
        std::vector< map<int, vector<BwdidxWithResidual>> > all_bwd_idx(NUM_CORES-1);

        for(unsigned i=0; i< NUM_CORES-1; i++){
            if(i==0)
                futures[i] = std::async( std::launch::async, eachcore_build_backward, space_for_first_core, i, std::ref(graph), std::ref(all_bwd_idx[i]) );
            else
                futures[i] = std::async( std::launch::async, eachcore_build_backward, space_per_core, i, std::ref(graph), std::ref(all_bwd_idx[i]) );
        }
        std::for_each( futures.begin(), futures.end(), std::mem_fn(&std::future<void>::wait));

        {
            //merge bwd idx from each core into one
            Timer t(10, "merge bwd idx from each core");
            for(auto &bwd_idx: all_bwd_idx){
                idx.insert(bwd_idx.begin(), bwd_idx.end());
                // bwd_idx.clear();
            }
            all_bwd_idx.clear();
        }

        {
            Timer t(12, "serialize bwd oracle");
            serialize_bwd_idx();
        }
    }
}

static void build_oracle_hubppr(const Graph &graph) {
    // build oracle for hubppr
    assert(config.space_consumption > 0);

    string fwd_idx_path = get_fwd_idx_file_name();//replace(oracle_path, ".idx", ".meta");
    string fwd_idx_info_path = get_fwd_idx_info_file_name();
    string bwd_idx_path = get_bwd_idx_file_name();//replace(meta_path_forward, ".fwd.", ".bwd.");
    string bwd_idx_info_path = get_bwd_idx_info_file_name();

    //forward
    INFO("construting forward oracle");
    if (!config.force_rebuild && file_exists_test(fwd_idx_path) && file_exists_test(fwd_idx_info_path) ) {
        cerr << "forward oracle already exist " << endl;
    }
    else{
        Timer timer(2, "build forward oracle");
        build_forward_oracle_hubppr(graph);
        {
            Timer t(11, "serialize fwd oracle");
            fwd_idx.shrink_to_fit();
            serialize_fwd_idx();
        }
    }

    //backward
    INFO("construting backward oracle");
    if ( !config.force_rebuild && file_exists_test(bwd_idx_path) && file_exists_test(bwd_idx_info_path) ) {
        cerr << "backward oracle already exist " << endl;
    }
    else{
        Timer tm(3, "generate backward oracle for hubppr");
        build_backward_oracle_hubppr(graph);
        {
            Timer t(12, "serialize bwd oracle");
            serialize_bwd_idx();
        }
    }
}



static void build_oracle_entry(const Graph &graph) {
    config.action = "gen_bwd_oracle";
    INFO("build oracle", config.graph_alias);

    calculate_sample_number(graph);
    config.bwd_delta = calculate_bwd_delta_bippr_and_hubppr(config.delta, config.epsilon, config.dbar, config.pfail);

    double fwd_rw_count = calculate_fwd_count_bippr_and_hubppr(config.bwd_delta, config.delta, config.epsilon, config.pfail);
    config.fwd_delta = 1.0 / fwd_rw_count;

    INFO(fwd_rw_count);




    INFO(config.fwd_delta);
    INFO(config.bwd_delta);
    assert(config.bwd_delta > 0);
    assert(config.fwd_delta > 0);

    // build for hubppr
    if (config.algo == "hubppr") {
        if(config.multithread == true)
            multicore_build_oracle_hubppr(graph);
        else
            build_oracle_hubppr(graph);

    }
    else if (config.algo == "bipproracle") {
        build_oracle_bippr(graph);
    }
    else {
        cerr << "config.algo in build oracle cannot recognized" << config.algo << endl;
        exit(1);
    }
}

#endif //HUBPPR_BUILD_BACKWARD_ORACLE_H
