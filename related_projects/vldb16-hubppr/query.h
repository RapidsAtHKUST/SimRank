#ifndef HUBPPR_QUERY_H
#define HUBPPR_QUERY_H

#include "algo.h"
#include "graph.h"
#include "heap.h"
#include "config.h"
// #include "threadpool.hpp"
// #include "thread_pool.hpp"

static Avg avg_bwd;
static Avg avg_hub;
static Avg avg_fwd;

extern iMap<int> hub_used_samples;
extern vector<iMap<int>> multi_hub_used_samples;

extern iMap<int> dest_nodes;
extern vector<iMap<int>> multi_dest_nodes;
// extern vector<vector<int> > fwd_idx_uncompressed;
// extern vector<int> fwd_idx_size;
// extern vector<vector<unordered_map<int, int> > > fwd_idx;
// extern vector<int> forward_node_order;
extern vector<int> fwd_idx;
extern map<int, pair<int64,int> > fwd_idx_ucp_pointers;
extern map<int, vector<int64> > fwd_idx_cp_pointers; 
extern vector<vector<int64>> fwd_idx_ptrs;
extern vector<int> fwd_idx_size;
extern iMap<int> fwd_idx_size_k;
extern iMap<int> statistic_hit_number;
extern vector<unsigned> global_seeds;

extern iMap<double> bwd_residuals;
extern iMap<double> bwd_reserves;

static void bippr_setting() {
    INFO("bippr setting");

    config.num_of_hubs = 0;

    config.fwd_method = NAIVE;
    config.bwd_method = NAIVE;
    
    //config.bwd_delta = config.epsilon * sqrt(config.delta*config.dbar / log(1.0 / config.pfail));
    
    //double fwd_rw_count = config.bwd_delta/config.delta/config.epsilon/config.epsilon*log(1/config.pfail);
    
    config.bwd_delta = calculate_bwd_delta_bippr_and_hubppr(config.delta, config.epsilon, config.dbar, config.pfail);

    double fwd_rw_count = calculate_fwd_count_bippr_and_hubppr(config.bwd_delta, config.delta, config.epsilon, config.pfail);

    INFO(fwd_rw_count);
    config.fwd_delta = 1.0 / fwd_rw_count;
}

static void hubppr_setting(const Graph &graph) {
    assert(config.space_consumption > 0);

    
    //config.bwd_delta = config.epsilon * sqrt(config.delta*config.dbar / log(1.0 / config.pfail));
    
    //double fwd_rw_count = config.bwd_delta/config.delta/config.epsilon/config.epsilon*log(1/config.pfail);

    config.bwd_delta = calculate_bwd_delta_bippr_and_hubppr(config.delta, config.epsilon, config.dbar, config.pfail);

    double fwd_rw_count = calculate_fwd_count_bippr_and_hubppr(config.bwd_delta, config.delta, config.epsilon, config.pfail);
    
    INFO(fwd_rw_count);
    config.fwd_delta = 1.0 / fwd_rw_count;

    config.num_of_hubs = 0;

    config.fwd_method = USE_ORACLE;
    load_forward_oracle(graph);

    INFO(config.num_of_hubs);
    assert(config.num_of_hubs > 0);

    config.bwd_method = USE_ORACLE;
    load_backward_oracle(graph);

}




static void bippr_topk_setting(const Graph& graph){
    INFO("bippr topk setting");

    config.epsilon /=2.0;
    
    config.bwd_delta = calculate_bwd_delta_bippr_and_hubppr(config.delta, config.epsilon, config.dbar, config.pfail);

    double fwd_rw_count = calculate_fwd_count_bippr_and_hubppr(config.bwd_delta, config.delta, config.epsilon, config.pfail);
    
    INFO(fwd_rw_count);
    config.fwd_delta = 1.0 / fwd_rw_count;
    
    config.fwd_method = NAIVE;
    config.bwd_method = NAIVE;
}


static void hubppr_topk_setting(int target_size, const Graph& graph){
    INFO("hubppr topk setting");

    config.num_of_hubs = 0;

    config.bwd_delta = sqrt(target_size)*calculate_bwd_delta_bippr_and_hubppr(config.delta, config.epsilon, config.dbar, config.pfail);
    config.compress_fwd = true;
    config.fwd_method = USE_ORACLE;
    load_forward_oracle(graph);
    INFO(config.num_of_hubs);
    assert(config.num_of_hubs > 0);
     
    config.bwd_method = USE_ORACLE;
    load_backward_oracle(graph);

    double fwd_rw_count = calculate_fwd_count_bippr_and_hubppr(config.bwd_delta, config.delta, config.epsilon, config.pfail);
    INFO(fwd_rw_count);
    config.fwd_delta = 1.0 / fwd_rw_count;
    
    config.fwd_method = USE_ORACLE;
    load_forward_oracle(graph);

    INFO(config.num_of_hubs);
    assert(config.num_of_hubs > 0);

    config.bwd_method = USE_ORACLE;
    load_backward_oracle(graph);
}


static void fastppr_setting(const Graph &graph) {

    INFO("fastppr setting");
    config.num_of_hubs = 0;

    config.fwd_method = NAIVE;
    config.bwd_method = NAIVE;


//    double delta = 1.0 / double(graph.n);
//    double pfail = 1.0 / double(graph.n);

    double dbar = double(graph.m) / double(graph.n); // average degree
    double epsr = sqrt(dbar * config.delta);

    config.bwd_delta = 0.2 * 6 / config.epsilon / config.epsilon * epsr;

    double fwd_rw_count = 35.0 / config.epsilon / config.epsilon * epsr / config.delta;

    config.fwd_delta = 1.0 / fwd_rw_count;
}

static void montecarlo_setting(const Graph &graph) {
    INFO("montecarlo setting");

    config.num_of_hubs = 0;

    config.fwd_method = NAIVE;
    config.bwd_method = NAIVE;


//    double delta = 1.0 / double(graph.n);
//    double pfail = 1.0 / double(graph.n);

    // largest backward delta, so backward do nothing
    config.bwd_delta = 1;

    double fwd_rw_count = 3*log(2/config.pfail)/config.epsilon/config.epsilon/config.delta;
    //double fwd_rw_count = config.bwd_delta / config.delta / config.epsilon / config.epsilon * log(1 / config.pfail);
    config.fwd_delta = 1.0 / fwd_rw_count;
}



static void exact_topk_setting() {
    INFO("exact topk setting");
       config.num_of_hubs = 0;

    config.fwd_method = NAIVE;
    config.bwd_method = NAIVE;

    // largest backward delta, so backward do nothing
    config.bwd_delta = 1;

    double fwd_rw_count = 100*1/config.delta;
    //double fwd_rw_count = config.bwd_delta / config.delta / config.epsilon / config.epsilon * log(1 / config.pfail);
    config.fwd_delta = 1.0 / fwd_rw_count;
    INFO(fwd_rw_count);
    INFO(config.fwd_delta);
    INFO(config.bwd_delta);
}

static void bipproracle_setting() {
    INFO("bipproracle setting");

    config.fwd_method = FULL_PRECOMPUTE;
    config.bwd_method = FULL_PRECOMPUTE;
}




static double compute_ppr_topk(int start, double reserve, unordered_map<int, double>& map_rsd, int64 num_rw){
    Timer tm(101, "computing ppr");
    // map<int, double> &pi = rtn.first;
    // map<int, double> &residual = rtn.second;
    double ans = 0;
    // int total_num =0;

    if(dest_nodes.occur.m_num < map_rsd.size()){ //iterate on smaller-size list
        // for (auto &item:fwd) {
        for(int i=0; i<dest_nodes.occur.m_num; i++){
            int node = dest_nodes.occur[i];
            int count = dest_nodes[node];
            // total_num+=count;
            if (map_rsd.find(node)!=map_rsd.end()) {
                ans += map_rsd[node]*count;
            }
        }
    }
    else{
        for (auto &item: map_rsd) {
            int node = item.first;
            double resi = item.second;
            if (dest_nodes.exist(node)) {
                ans += dest_nodes[node]*resi;
            }
            // total_num += dest_nodes[node];
        }
    }

    ans/=num_rw;
    ans += reserve;

    return ans;
}

static double ppr(int start) {
    Timer tm(101, "computing ppr");
    // map<int, double> &pi = rtn.first;
    // map<int, double> &residual = rtn.second;
    double ans = 0;
    static int64 total_num = 1/config.fwd_delta;

    if(dest_nodes.occur.m_num < bwd_residuals.occur.m_num){ //iterate on smaller-size list
        // for (auto &item:fwd) {
        for(int i=0; i<dest_nodes.occur.m_num; i++){
            int node = dest_nodes.occur[i];
            int count = dest_nodes[node];
            // total_num+=count;
            if (bwd_residuals.exist(node)) {
                ans += bwd_residuals[node]*count;
            }
        }
    }
    else{
        for (int i=0; i<bwd_residuals.occur.m_num; i++) {
            int node = bwd_residuals.occur[i];
            double resi = bwd_residuals[node];
            if (dest_nodes.exist(node)) {
                ans += dest_nodes[node]*resi;
            }
            // total_num += dest_nodes[node];
        }
    }
    // bwd_residuals.clean();

    ans/=total_num;
    if(bwd_reserves.exist(start))
        ans += bwd_reserves[start];
    
    // bwd_reserves.clean();

    return ans;
}

static double ppr_compress(int start) {
    Timer tm(101, "computing ppr");
    // map<int, double> &pi = rtn.first;
    // map<int, double> &residual = rtn.second;
    double ans = 0;
    static int64 total_num =1/config.fwd_delta;

    if(dest_nodes.occur.m_num < bwd_residuals.occur.m_num){ //iterate on smaller-size list
        for (int i=0; i<dest_nodes.occur.m_num; i++) {
            int node = dest_nodes.occur[i];
            int count = dest_nodes[node];
            // total_num+=count;
            if (bwd_residuals.exist(node)) {
                ans += bwd_residuals[node]*count;
            }
        }
    }
    else{
        for (int i=0; i<bwd_residuals.occur.m_num; i++) {
            int node = bwd_residuals.occur[i];
            double resi = bwd_residuals[node];
            if (dest_nodes.exist(node)) {
                ans += dest_nodes[node]*resi;
                // total_num += dest_nodes[node];
            }
        }
    }

    int node;
    int occur;
    int remaining;
    int64 last_beg_ptr;
    int64 end_ptr;
    int hub_node;
    int blocked_num;
    //hub_used_samples.occur.Sort();

    for(int xxx=0; xxx<hub_used_samples.occur.m_num; xxx++){
        hub_node = hub_used_samples.occur[xxx];
        vector<int64> &hub_vec = fwd_idx_ptrs[hub_node];
        last_beg_ptr = hub_vec[hub_vec.size()-2];
        end_ptr = hub_vec[hub_vec.size()-1];

        remaining = hub_used_samples[hub_node];
        //INFO(remaining, k);

        if(remaining > fwd_idx_size_k[hub_node]){
            //INFO("nodes larger than threshold");
            // for(auto ent: hub_vec[hub_vec.size()-1]){
            for(int64 ptr=last_beg_ptr; ptr<end_ptr; ptr+=2){
                node = fwd_idx[ptr];
                occur = fwd_idx[ptr+1];
                if (bwd_residuals.exist(node))
                    ans += bwd_residuals[ node ]*occur;
                // total_num+= occur;
                // selected_num+=occur;
                remaining-=occur;
            }
        }

        for(int i=0; i< hub_vec.size()-2; i++){
            int bit_pos = 1<<i;
            //INFO(i, bit_pos&remaining);
            if(bit_pos & remaining){
                for(int64 ptr=hub_vec[i]; ptr<hub_vec[i+1]; ptr+=2){
                    node = fwd_idx[ptr];
                    occur = fwd_idx[ptr+1];
                    if (bwd_residuals.exist(node))
                        ans += bwd_residuals[node]*occur;
                    // total_num+= occur;
                }
            }
        }
    }

    // bwd_residuals.clean();

    ans/=total_num;
    if(bwd_reserves.exist(start))
        ans += bwd_reserves[start];
    
    // bwd_reserves.clean();

    return ans;
}

int current_target=0;

static double query(int source, int target, const Graph &graph) {
    Counter c(1);
    //INFO("query ", source, target);
    // pi and residual in format id, value, means pi(id, t) = value, residual(id, t) = value
    current_target = target;
    sample_bwd(target,graph);
    sample_fwd(source, graph); // a list of ending nodes
    
    if(config.algo == "hubppr")
        return ppr_compress(source);
    else
        return ppr(source);
}

static double compute_ppr(int start){
    Timer tm(101, "computing ppr");
    // map<int, double> &pi = rtn.first;
    // map<int, double> &residual = rtn.second;
    double ans = 0;
    static int64 total_num = 1/config.fwd_delta;

    omp_set_num_threads(multi_dest_nodes.size());
#pragma omp parallel for reduction(+:ans)
    for(int i=0; i< multi_dest_nodes.size(); i++){
        unsigned tid = omp_get_thread_num();
        if(multi_dest_nodes[tid].occur.m_num < bwd_residuals.occur.m_num){
            for (int i=0; i<multi_dest_nodes[tid].occur.m_num; i++) {
                int node = multi_dest_nodes[tid].occur[i];
                int count = multi_dest_nodes[tid][node];
                // total_num+=count;
                if (bwd_residuals.exist(node)) {
                    ans += bwd_residuals[node]*count;
                }
            }
        }
        else{
            for (int i=0; i<bwd_residuals.occur.m_num; i++) {
                int node = bwd_residuals.occur[i];
                double resi = bwd_residuals[node];
                if (multi_dest_nodes[tid].exist(node)) {
                    ans += multi_dest_nodes[tid][node]*resi;
                    // total_num += multi_dest_nodes[tid][node];
                }
            } 
        }
        multi_dest_nodes[tid].clean();
    }

    if(config.compress_fwd){
        //merge hubs hit
        for(int i=1; i<multi_hub_used_samples.size(); i++){
            for(int j=0; j<multi_hub_used_samples[i].occur.m_num; j++){
                int hub=multi_hub_used_samples[i].occur[j];
                int count=multi_hub_used_samples[i][hub];
                if(multi_hub_used_samples[0].notexist(hub))
                    multi_hub_used_samples[0].insert(hub, count);
                else
                    multi_hub_used_samples[0][hub] += count;
            }
            multi_hub_used_samples[i].clean();
        }

        int node;
        int occur;
        multi_hub_used_samples[0].occur.Sort();

        omp_set_num_threads(config.num_thread);
#pragma omp parallel for reduction(+:ans)
        for(int xxx=0; xxx<multi_hub_used_samples[0].occur.m_num; xxx++){
            int hub_node = multi_hub_used_samples[0].occur[xxx];
            int blocked_num = multi_hub_used_samples[0][hub_node];

            // if(statistic_hit_number.notexist(hub_node))
            //     statistic_hit_number.insert(hub_node, blocked_num);
            // else
            //     statistic_hit_number[hub_node]+=blocked_num;

            vector<int64> &hub_vec = fwd_idx_cp_pointers[hub_node];
            int64 last_beg_ptr = hub_vec[hub_vec.size()-2];
            int64 end_ptr = hub_vec[hub_vec.size()-1];
            int k = fwd_idx_size_k[hub_node];
            int remaining = blocked_num;

            if(remaining > k){
                int selected_num=0;
                for(int64 ptr=last_beg_ptr; ptr<end_ptr; ptr+=2){
                    node = fwd_idx[ptr];
                    occur = fwd_idx[ptr+1];
                    if (bwd_residuals.exist(node)) {
                        ans += bwd_reserves[ node ]*occur;
                    }
                    // total_num+= occur;
                    selected_num+=occur;
                }
                remaining -= selected_num;
            }


            for(int i=0; i< hub_vec.size()-2; i++){
                int bit_pos = 1<<i;
                if(bit_pos & remaining){
                    for(int64 ptr=hub_vec[i]; ptr<hub_vec[i+1]; ptr+=2){
                        node = fwd_idx[ptr];
                        occur = fwd_idx[ptr+1];
                        if (bwd_residuals.exist(node)) {
                            ans += bwd_reserves[node]*occur;
                        }
                        // total_num += occur;
                    }
                }
            }
        }

        multi_hub_used_samples[0].clean();
        // hub_used_samples.clean();
    }

    // bwd_residuals.clean();

    ans/=total_num;
    if(bwd_reserves.exist(start))
        ans += bwd_reserves[start];
    
    // bwd_reserves.clean();

    return ans;
}



static vector<pair<int, int> > p2p_query;

static void generate_p2p_query(const Graph& graph){
    assert(config.query_size > 0);
    string path = parent_folder + "query" + FILESEP + "p2p" + FILESEP;
    if(config.target_sample == UNIFORM) 
        path += config.graph_alias + ".query.uniform";
    else
        path += config.graph_alias + ".query.globalpr";

    if(file_exists_test(path)){
        cerr<<"p2p query already exists"<<endl;
        //return;
    }
    INFO(path);
    ofstream fout_query(path);
    iMap<int> node_marker;
    node_marker.initialize(graph.n);
    for (int cnt = 0; cnt < config.query_size; cnt++) {
        int sample_source, sample_target = 0;

        sample_source = lrand() % graph.n;
        
        if (config.target_sample == UNIFORM)
            sample_target = lrand() % graph.n;
        else if (config.target_sample == GLOBAL_PAGERANK){
            sample_target = gpr->sample_by_pr();
            while(node_marker.exist(sample_target)){
                sample_target = gpr->sample_by_pr();
            }
            node_marker.insert(sample_target, 1);
        }
        else{
            INFO("wrong query type");
        }

        fout_query<<sample_source<<" "<<sample_target<<endl;
    }
    fout_query.close();
}

static void load_p2p_query(const Graph& graph){
    Timer timer100(100, "total query time");
    string path = parent_folder + "query" + FILESEP + "p2p" + FILESEP;
    if(config.target_sample == UNIFORM) 
        path += config.graph_alias + ".query.uniform";
    else
        path += config.graph_alias + ".query.globalpr";

    ASSERTMSG(file_exists_test(path), path.c_str());

    ifstream fin_query(path);
    int sample_source, sample_target;
    while(fin_query>>sample_source){
        fin_query>>sample_target;
        p2p_query.push_back(make_pair(sample_source, sample_target));
    }
    fin_query.close();
}
extern iMap<int> component;
static void query(const Graph &graph) {

    config.use_bwd_index = false;

    if (config.algo.size() == 0) {
        cerr << "NO algorithm" << endl;
        exit(1);
    }
    // config.algo in ['hubppr', 'bippr']

    //gpr = new GlobalPR(config, graph);

    if (config.algo == "bippr") {
        bippr_setting();
    }
    else if (config.algo == "bipproracle") {
        gpr = new GlobalPR(config, graph);
        bipproracle_setting();
    }
    else if (config.algo == "hubppr") {
        gpr = new GlobalPR(config, graph);
        hubppr_setting(graph);
    }
    else if (config.algo == "fastppr") {
        fastppr_setting(graph);
    }
    else if (config.algo == "montecarlo") {
        montecarlo_setting(graph);
    }
    else {
        cerr << "config.algo not recognized" << endl;
        exit(1);

    };
    load_p2p_query(graph);
    INFO(config.fwd_delta);
    INFO(config.bwd_delta);

    double (*query_fptr)(int, int, const Graph&);
    query_fptr = &query;

    if(p2p_query.size() == 0){
        for (int cnt = 0; cnt < config.query_size ; cnt++) {
            Timer timer1(1);
            int sample_source = lrand() % graph.n;
            int sample_target = 0;
            if (config.target_sample == UNIFORM)
                sample_target = lrand() % graph.n;
            else
                sample_target = gpr->sample_by_pr();

            Timer timer2(2); // to test same as 1

            // call query
            query_fptr(sample_source, sample_target, graph);
            result.finished_queries = cnt;
            if (Timer::used(1) > config.query_seconds) {
                break;
            }
        }
    }
    else{
        int cnt=0;
        for(auto query_pair : p2p_query){
            Timer timer1(1);
            double result = query_fptr(query_pair.first, query_pair.second, graph);
            cnt++;
            if(cnt >=config.query_size) break;
        }
        result.finished_queries = cnt;

        INFO(Timer::used(1));
    }

    result.n = graph.n;
    result.m = graph.m;

    result.hub_label_size = avg_hub.avg * graph.n;
    result.fwd_label_size = avg_fwd.avg * graph.n;
    result.bwd_label_size = avg_bwd.avg * graph.n;


    result.total_size = result.hub_label_size + result.fwd_label_size + result.bwd_label_size;
    result.time_spent = Timer::used(1);

    INFO(result.finished_queries);

    cout << (Timer::used(1) - Timer::used(88) - Timer::used(87)) / result.finished_queries * 1000 <<
    " (ms) per query" << endl;
}




double reverse_local_update_heap_hitting_forward(int t, const Graph &graph, int source, unordered_map<int, int> &fwd_sample) {
    // return the estimated value
    static BinaryHeap<double, greater<double> > heap(graph.n, greater<double>());
    static map<int, double> exist;

    double myeps = config.bwd_delta;

    exist.clear();

    heap.clear();

    heap.insert(t, 1);
//    cerr << "init" << endl;
    while (heap.size()) {
        //cerr << "heapsize " << heap.size() << endl;
//        heap.display();
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;
        if (residual < myeps)
            break;
        heap.delete_top();
        if (gpr->fast_rank(v) * gpr->fast_rank(v) < graph.n) {
            // brutely through away this node
            continue;
        }
        exist[v] += residual * config.alpha;
        for (int next : graph.gr[v]) {
//            cerr << "next " << next << endl;
            int cnt = (int) graph.g[next].size();
            double delta = ((1 - config.alpha) * residual) / cnt;
            if (heap.has_idx(next))
                heap.modify(next, heap.get_value(next) + delta);
            else
                heap.insert(next, delta);
        }
//        heap.modify(v, 0);
    }
    result.count_exist += exist.size();
    result.count_residual += heap.size();
    map<int, double> residual;
    while (heap.size()) {
        auto top = heap.extract_top();
        residual[top.second] = top.first;
        heap.delete_top();
    }
    double result = exist[source];

    for (auto item: residual) {
        int node = item.first;
        double prob = item.second;
        if (fwd_sample.find(node) != fwd_sample.end()) {
            result += prob * fwd_sample[node];
        }
    }

    return result;
}


unordered_map<int, double> lower_bounds;
iMap<double> upper_bounds;
std::list<int> candidate_list;
unordered_map<int, int> node_to_order;
vector<double> source_reserves;
vector<unordered_map<int, double>> residual_maps;
vector< vector<double> > iter_rmax;
iMap<double> iter_ppr;
vector<double> m_omega;
unordered_map<int64, int> j_log;
vector<pair<int, double>> low_up_ratio;

void set_bound_by_martingale(int t_size, int rw_num, int iteration_num, int source_node, int node, const Graph& graph){
    static double new_pfail = config.pfail/2.0/t_size/log2((unsigned long long)graph.n*config.alpha*graph.n*t_size);
    static double pfail_star = log(new_pfail);
    static int64 omega = ceil(1.0/config.fwd_delta);

    double m_omega=0;
    {
        Timer tm(20);
        if(dest_nodes.occur.m_num < residual_maps[node_to_order[node]].size()){ //iterate on smaller-size list
            for(int i=0; i<dest_nodes.occur.m_num; i++){
                int dest = dest_nodes.occur[i];
                int count = dest_nodes[dest];
                if (residual_maps[node_to_order[node]].find(dest)!=residual_maps[node_to_order[node]].end()) {
                    m_omega += residual_maps[node_to_order[node]][dest]*count;
                }
            }
        }
        else{
            for (auto &item: residual_maps[node_to_order[node]]) {
                int dest = item.first;
                double resi = item.second;
                if (dest_nodes.exist(dest)) {
                    m_omega += dest_nodes[dest]*resi;
                }
            }
        }
    }

    //compute lambda
    double b = 0;
    // int iter_prime = 0;
    b = (2*rw_num-1)*pow(iter_rmax[iteration_num][node_to_order[node]]/2.0, 2);

    double lambda = sqrt(pow(1.0*iter_rmax[iteration_num][node_to_order[node]]*pfail_star/3, 2) - 2*b*pfail_star)-1.0*iter_rmax[iteration_num][node_to_order[node]]*pfail_star/3;

    // INFO(source_reserves[node_to_order[node]], lambda, m_omega);
    
    //compute lower bound & upper bound
    upper_bounds[node] = min(upper_bounds[node], source_reserves[node_to_order[node]]+(m_omega+lambda)/(2*rw_num-1) );
    assert(lower_bounds[node]<1);
    lower_bounds[node] = max(lower_bounds[node], source_reserves[node_to_order[node]]+(m_omega-lambda)/(2*rw_num-1) );
    assert(upper_bounds[node]>0);
}

void topk_hubppr_martingale(const Graph &graph, int source_node, vector<int>& targets, const int k, vector< pair<int, double> >& results){
    static int64 the_omega = 2*config.bwd_delta*log(2*k/config.pfail)/config.epsilon/config.epsilon/config.delta;
    static double bwd_cost_div = 1.0*graph.m/graph.n/config.alpha/1.0;
    static int64 omega = ceil(1.0/config.fwd_delta);
    static int exp_max_iter_time = ceil(log2(omega));

    check_end_nodes(source_node, targets);
        
    lower_bounds.clear();
    upper_bounds.clean();
    candidate_list.clear();
    node_to_order.clear();

    source_reserves.clear();
    source_reserves.resize(targets.size());

    residual_maps.clear();
    residual_maps.resize(targets.size());

    iter_rmax.clear();
    iter_rmax.resize(exp_max_iter_time);

    dest_nodes.clean();

    low_up_ratio.clear();
    low_up_ratio.resize(targets.size());

    int64 fwd_cost =0, backward_cost =0;

    for(int i=0; i< targets.size();i++){
        candidate_list.push_back(targets[i]);
        lower_bounds[targets[i]] = 1./graph.n;
        upper_bounds[targets[i]] = 1;
        low_up_ratio[i] = MP(targets[i], lower_bounds[targets[i]]/upper_bounds[targets[i]]);

        node_to_order[targets[i]] = i;
        iter_rmax[0].push_back(1);
        residual_maps[i][targets[i]] = 1;
        source_reserves[i] = 0;
    }

    int iteration_num=1;
    while(candidate_list.size()>k){
        if(iteration_num>=iter_rmax.size()){
            iter_rmax.push_back(iter_rmax[iteration_num-1]); //copy the rmax of last iteration to current iteration
        }
        else{
            iter_rmax[iteration_num] = iter_rmax[iteration_num-1];
        }

        double max_rmax = *(std::max_element(iter_rmax[iteration_num].begin(), iter_rmax[iteration_num].end()));
        if( max_rmax >= config.bwd_delta ){ //if rmax for each target is less than minimum rmax setting, no need to backward push
            if( 1 == iteration_num){ //first initial round, bwd push from all targets
                for(int t: candidate_list){
                    iter_rmax[iteration_num][node_to_order[t]] = max(iter_rmax[iteration_num-1][node_to_order[t]]/2, config.bwd_delta);
                    //continue bwd push until the reverse of each node < rmax_t
                    int push_count = reverse_local_update_topk(source_node, t, graph, source_reserves[node_to_order[t]], residual_maps[node_to_order[t]], iter_rmax[iteration_num][node_to_order[t]]);
                    backward_cost+=(bwd_cost_div*(1/iter_rmax[iteration_num][node_to_order[t]]-1/iter_rmax[iteration_num-1][node_to_order[t]])); //bwd push based on previous residuals
                    // backward_cost+=push_count;
                }
            }
            else{
                static vector<pair<int, double>> sort_low_up_ratio;
                sort_low_up_ratio.clear();
                sort_low_up_ratio.resize(low_up_ratio.size());
                partial_sort_copy(low_up_ratio.begin(), low_up_ratio.end(), sort_low_up_ratio.begin(), sort_low_up_ratio.end(),[](pair<int, double> const& l, pair<int, double> const& r){return l.second < r.second;});
                int iter=0;
                int count=0;
                while( fwd_cost >= backward_cost || iter<sort_low_up_ratio.size() ) {
                    int t = sort_low_up_ratio[iter].first;
                    if(iter_rmax[iteration_num-1][node_to_order[t]] > config.bwd_delta){ //when reach lowest rmax, no need bwd anymore
                        iter_rmax[iteration_num][node_to_order[t]] = max(iter_rmax[iteration_num-1][node_to_order[t]]/2, config.bwd_delta);
                        //continue bwd push until the reverse of each node < rmax_t
                        int push_count = reverse_local_update_topk(source_node, t, graph, source_reserves[node_to_order[t]], residual_maps[node_to_order[t]], iter_rmax[iteration_num][node_to_order[t]]);
                        backward_cost+=(bwd_cost_div*(1/iter_rmax[iteration_num][node_to_order[t]]-1/iter_rmax[iteration_num-1][node_to_order[t]])); //bwd push based on previous residuals
                        // backward_cost+=push_count;
                        count++;
                    }
                    iter++;
                    if(iter>=sort_low_up_ratio.size()){ //stop while one round iteration finished
                        break;
                    }
                }
            }
        }

        int64 rw_num = pow(2, iteration_num);

        generate_fwd_randwalk_topk_martingale(source_node, graph, rw_num);
        fwd_cost +=rw_num*1.0/config.alpha;

        // compute lower bound & upper bound
        for(int node: candidate_list){
            set_bound_by_martingale(targets.size(), rw_num, iteration_num, source_node, node, graph);
            // INFO(lower_bounds[node], upper_bounds[node]);
        }

        results.clear();
        results.resize(k);
        partial_sort_copy(lower_bounds.begin(), lower_bounds.end(), results.begin(), results.end(), [](pair<int, double> const& l, pair<int, double> const& r){return l.second > r.second;});

        if( lower_bounds[results[k-1].first]*(1+config.epsilon) >= upper_bounds[results[k-1].first] ){
            //top-k nodes all satisify the constraint
            INFO("return correctly", iteration_num);
            // for(int i=0;i<results.size();i++){
            //     int node = results[i].first;
            //     INFO(node, lower_bounds[node], upper_bounds[node]);
            //     INFO(node, source_reserves[node_to_order[node]], iter_rmax[iteration_num][node_to_order[node]]);
            // }
            return;
        }

        if(rw_num>=the_omega){
            for(int t: targets){
                if(iter_rmax[iteration_num][node_to_order[t]]<=config.bwd_delta){
                    //return k random nodes
                    INFO("return abnormally");
                    return;
                }
            }
        }

        //eliminate all nodes t' from candidate_list if UB(t') >= LB(t_k)
        int t_k = results[k-1].first;
        std::list<int>::iterator candi_iter = candidate_list.begin();
        while(candi_iter!=candidate_list.end()){
            if(upper_bounds[*candi_iter]<=lower_bounds[t_k]){
                //remove invalid t from candidate_list
                upper_bounds[*candi_iter]=1;
                lower_bounds[*candi_iter]=0;
                candi_iter = candidate_list.erase(candi_iter);
            }
            else
                candi_iter++;
        }

        //compute lower_bound:upper_bound for each candidate node
        low_up_ratio.clear();
        for(int node: candidate_list){
            low_up_ratio.push_back(MP(node, lower_bounds[node]/upper_bounds[node]));
        }

       iteration_num++;
    }
}

void topk_bippr(const Graph &graph, int s, vector<int>& targets, const int k, vector< pair<int, double> >& results){
    static unordered_map<int, double> pprs;
    static unordered_map<int, double> map_reserves;
    static unordered_map<int, double> map_residuals;

    pprs.clear();

    sample_fwd(s, graph);
    for (int t: targets) {
        sample_bwd(t, graph); 
        pprs[t] =  ppr(s);
    }

    results.clear();
    results.resize(k);
    partial_sort_copy(pprs.begin(), pprs.end(), results.begin(), results.end(),[](pair<int, double> const& l, pair<int, double> const& r){return l.second > r.second;});
}



static void BFS(int start, const Graph& graph, vector<int>& targets, int k){
    std::queue<int> q;
    unordered_map<int, bool> marks;
    q.push(start);
    marks[start] = true;

    while(!q.empty()&&targets.size()<k){
        int u = q.front();
        // INFO(u, graph.n);
        q.pop();

        for(int next: graph.g[u]){
            if(marks[next]==true||next==0)
                continue;
            q.push(next);
            targets.push_back(next);
            if(targets.size()==k)
                return;
            marks[next]=true;
        }
    }
}

static void generate_topk_query(const Graph& graph){
    assert(config.query_size>0);
    assert(config.target_size>0);

    string path = parent_folder + "query" + FILESEP + "topk" + FILESEP;

    path += config.graph_alias + ".T"+to_str(config.target_size)+".query";
    ofstream fout_query(path);


    int T = config.target_size;
    fout_query<<T<<" "<<config.query_size<<endl;
    //generate config.query_size number of queries with target size T

    int cnt=0;
    while(true){
        int s = rand_double() * graph.n;
        if(graph.g[s].size()<1)
            continue;

        vector<int> targets;
        // choose destinations by BFS
        BFS(s, graph, targets, config.target_size);
        if(targets.size()<config.target_size){
            continue;
        }

        // for (int i = 0; i < config.target_size; i++) {
        fout_query<<s<<endl;
        for(int t: targets){
            // int t = lrand() % graph.n;
            fout_query<<t<<" "<<endl;
        }

        cnt++;
        if(cnt>=config.query_size)
            break;
    }
}



static vector<pair<int, vector<int> > > topk_queries;

static vector< pair<int, vector<pair<double, int> > > > topk_answers;

static void load_topk_query(const Graph& graph){
    INFO("loading topk queries");
    string path = parent_folder + "query" + FILESEP + "topk" + FILESEP;
    path += config.graph_alias + ".T"+to_str(config.target_size) + ".query";
    ASSERTMSG(file_exists_test(path), path.c_str());

    ifstream fin_query(path);

    int T, query_num;
    fin_query>>T;
    fin_query>>query_num;
    for(int i=0; i<query_num; i++){
        int source_sample;
        int target_node;
        fin_query>>source_sample;
        vector<int> target_set_sample;
        for(int j=0; j<T; j++){
            fin_query>>target_node;
            target_set_sample.push_back(target_node);
        }
        INFO(target_set_sample.size());
        topk_queries.push_back(make_pair(source_sample, target_set_sample));
    }

    INFO("finisehd loading topk queries");
}



static void load_topk_query_answer(const Graph& graph){
    INFO("loading topk queries");

    string path = parent_folder + "query" + FILESEP + "topk" + FILESEP;
    path += config.graph_alias +".T"+to_str(config.target_size)+ ".query.answer.exact";

    ASSERTMSG(file_exists_test(path), path.c_str());

    ifstream fin_query(path);

    int T, query_num;
    fin_query>>T;
    fin_query>>query_num;
    INFO(T, query_num);
    for(int i=0; i<query_num; i++){
        int source_sample;
        int target_node;
        double ppr_target_node;
        fin_query>>source_sample;

        vector<pair<double, int> > target_set_sample_answer;
        for(int j=0; j<T; j++){
            fin_query>> ppr_target_node;
            fin_query>>target_node;
            target_set_sample_answer.push_back(make_pair(ppr_target_node, target_node));
        }
        topk_answers.push_back(make_pair(source_sample, target_set_sample_answer));
    }

    INFO("finisehd loading topk answers");
}


static void topk(const Graph &graph) {
    

    assert(config.target_size > 5);
    INFO(config.target_size);
    INFO(config.epsilon);

    INFO(config.k_size);

    load_topk_query(graph);
    //load_topk_query_answer(graph);

  
    if(config.algo == "exact"){
        exact_topk_setting();
    }else if(config.algo == "bippr"){
        gpr = new GlobalPR(config, graph);
        bippr_topk_setting(graph);
    }
    else if(config.algo == "hubppr"){
        gpr = new GlobalPR(config, graph);
        hubppr_topk_setting(config.target_size, graph);
    }

    ofstream fout_answer;
    string path = parent_folder + "query" + FILESEP + "topk" + FILESEP;
    path += config.graph_alias +".T"+to_str(config.target_size)+ ".query.answer.exact";

    if(config.algo == "exact"){
        if(file_exists_test(path)){
            cerr<<"exact topk answer file already exists"<<endl;
            return;
        }else{
            fout_answer.open(path.c_str());
            fout_answer<<config.target_size<<" "<<topk_queries.size()<<endl;
        }
    }
    
    static Avg precision_avg;
    if(config.algo == "hubppr"){
        INFO("Start computing log of number of randwalks");
        int64 max_rw_num = ceil(1/config.fwd_delta);
        INFO(max_rw_num);
        int max_iter_times = ceil(log2(max_rw_num));
        INFO(max_iter_times);
        upper_bounds.initialize(graph.n);
        iter_ppr.initialize(graph.n);
        // iter_rmax.resize(max_iter_times);
    }

    INFO("start querying");
    int cnt=0;
    vector<int> target_set_sample;

    for (; cnt < 20; cnt++){//topk_queries.size(); cnt++) {
        vector< pair<int, double> > pprs;
        {
            int s = topk_queries[cnt].first;
            target_set_sample  = topk_queries[cnt].second;
            if(config.algo == "exact"){
                // auto fwd = sample_fwd(s, graph);
                {
                    Timer timer(TIMER_TOPK);
                    sample_fwd(s, graph);
                    for(int i=0; i<target_set_sample.size(); i++){
                        // pprs.push_back(fwd[target_set_sample[i]]* config.fwd_delta);
                        pprs.push_back(MP(target_set_sample[i], dest_nodes[target_set_sample[i]]* config.fwd_delta));
                    }
                }
                fout_answer<<s<<endl;
                for(auto ppr: pprs){
                    fout_answer<<ppr.second<<" "<<ppr.first<<" "<<endl;
                }
            }
            if (config.algo == "bippr" ) {
                {
                    Timer timer(TIMER_TOPK);
                    topk_bippr(graph, s, target_set_sample, config.k_size, pprs);
                }

                INFO(s);
                for(auto ppr: pprs){
                    INFO(ppr.first, ppr.second);
                }
            }            
            else if (config.algo == "hubppr"){
                {
                    Timer timer(TIMER_TOPK);
                    topk_hubppr_martingale(graph, s, target_set_sample, config.k_size, pprs);
                }
                INFO(s);
                for(auto ppr: pprs){
                    INFO(ppr.first, ppr.second);
                }
            }
        }
    }
    fout_answer.close();
    INFO(precision_avg.avg);
    INFO(config.target_size);
    cout << "topk ppr time for per query: " << Timer::used(TIMER_TOPK) / cnt *1000<< " ms" << endl;
}

void tune_forward_backward_ratio(const Graph& graph){
     bippr_setting();
     clock_t fwd_time=0;
     clock_t bwd_time = 0;
     srand(time(NULL));
     INFO("start tuning fwd-bwd ratio...");
     for (int cnt = 0; cnt < 1000; cnt++) {
        int sample_source = lrand() % graph.n;
        int sample_target = 0;
        if (config.target_sample == UNIFORM)
            sample_target = lrand() % graph.n;
        else
            sample_target = gpr->sample_by_pr();

        //cout<<sample_source<<" "<<sample_target<<endl;
        clock_t start = clock();
        sample_fwd(sample_source, graph); // a list of ending nodes
        clock_t end = clock();
        fwd_time += (end-start);

        start = clock();
        // sample_bwd(sample_target, graph);
        reverse_local_update_linear(sample_target, graph);
        end = clock();
        bwd_time +=(end-start);
    }
    config.fwd_cost_ratio = fwd_time*1.0/(bwd_time+fwd_time);

    dest_nodes.clean();
    hub_used_samples.clean();
    bwd_reserves.clean();
    bwd_residuals.clean();

    INFO(config.fwd_cost_ratio);
}

#endif //HUBPPR_QUERY_H
