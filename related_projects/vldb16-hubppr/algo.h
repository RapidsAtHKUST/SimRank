#ifndef __ALGO_H__
#define __ALGO_H__

#include "graph.h"
#include "heap.h"
#include "config.h"
#include <tuple>



extern vector<int> fwd_idx;
extern map<int, pair<int64,int> > fwd_idx_ucp_pointers;
extern map<int, vector<int64> > fwd_idx_cp_pointers; 
extern vector<vector<int64>> fwd_idx_ptrs;
extern vector<int> fwd_idx_size;
extern vector<int> sample_number;
extern iMap<int> fwd_idx_size_k;
extern iMap<int> statistic_hit_number;
extern vector<unsigned> global_seeds;
extern iMap<int> hub_used_samples;
extern vector<iMap<int>> multi_hub_used_samples;
extern iMap<int> dest_nodes;
extern vector<iMap<int>> multi_dest_nodes;
extern iMap<double> bwd_residuals;
extern iMap<double> bwd_reserves;

class Avg {
public:
    int cnt = 0;
    double avg = 0;

    double update(double t) {
        cnt++;
        avg = (avg * (cnt - 1) + t) / cnt;
        return avg;
    };


};

static inline unsigned long lrand() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, INT_MAX);
    return dis(gen);
}

static inline double rand_double() {
    // return (double)(lrand()) / (double)RAND_MAX;
    static size_t seed = static_cast<unsigned int>(std::time(0));
    static boost::random::mt19937 engine(seed);
    static boost::function<double()> randu =  boost::bind(boost::random::uniform_real_distribution<>(0, 1), engine);
    return randu();
}

static inline double drand(){
    return ((double)rand()/(double)RAND_MAX);
}

static inline double drand_r(unsigned core_id) {
    // return static_cast<double> (rand()) / static_cast<double>(RAND_MAX);
    // seed = global_seed;
    return ((double)rand_r(&global_seeds[core_id])/(double)RAND_MAX);
}

static inline double drand_u(unsigned core_id){
    static thread_local size_t seed = core_id + static_cast<unsigned int>(std::time(0));
    static thread_local boost::random::mt19937 engine(seed);
    static thread_local boost::function<double()> randu =  boost::bind(boost::random::uniform_real_distribution<>(0, 1), engine);
    return randu();
}

static double compute_num_of_tries( int nodeid, int n) {
    double threshold = 1.0/config.fwd_delta / config.bwd_prune_ratio / config.bwd_prune_ratio;

    return min(1.0/config.fwd_delta/n*sample_number[nodeid], threshold);

}


inline string get_fwd_idx_file_name(){
    string prefix = config.prefix + FILESEP + config.graph_alias+FILESEP;
    prefix += config.graph_alias + ".eps-" + to_str(config.epsilon);
    prefix += ".space-" + to_str(config.space_consumption);

    string suffix;
    if(config.compress_fwd)
        suffix += ".compress.fwd.idx";
    else
        suffix += ".fwd.idx";

    string file_name = prefix + suffix;
    return file_name;
}

inline string get_fwd_idx_info_file_name(){
    string idx_file = get_fwd_idx_file_name();
    return replace(idx_file, "fwd.idx", "fwd.info");
}

inline string get_bwd_idx_file_name(){
    string prefix = config.prefix + FILESEP + config.graph_alias+FILESEP;
    prefix += config.graph_alias + ".eps-" + to_str(config.epsilon);
    prefix += ".space-" + to_str(config.space_consumption);

    string suffix;
    if(config.compress_fwd)
        suffix += ".compress.bwd.idx";
    else
        suffix += ".bwd.idx";

    string file_name = prefix + suffix;
    return file_name;
}

inline string get_bwd_idx_info_file_name(){
    string idx_file = get_bwd_idx_file_name();
    return replace(idx_file, "bwd.idx", "bwd.info");
}

inline void serialize_fwd_idx(){
    std::ofstream ofs(get_fwd_idx_file_name());
    boost::archive::binary_oarchive oa(ofs);
    oa << fwd_idx;

    string ofs_rwn_str = get_fwd_idx_file_name()+".rwn";
    std::ofstream ofs_rwn( ofs_rwn_str);
    boost::archive::binary_oarchive oa_rwn(ofs_rwn);
    oa_rwn << sample_number;
    
    std::ofstream info_ofs(get_fwd_idx_info_file_name());
    boost::archive::binary_oarchive info_oa(info_ofs);
    if(config.compress_fwd)
        info_oa << fwd_idx_cp_pointers;
    else
        info_oa << fwd_idx_ucp_pointers;
}

inline void deserialize_fwd_idx(){
    string file_name = get_fwd_idx_file_name();
    assert_file_exist("index file", file_name);
    std::ifstream ifs(file_name);
    boost::archive::binary_iarchive ia(ifs);
    ia >> fwd_idx;
    
    string rwn_file_name = get_fwd_idx_file_name()+".rwn";
    assert_file_exist("rwn file", rwn_file_name);
    std::ifstream ofs_rwn(rwn_file_name);
    boost::archive::binary_iarchive ia_rwn(ofs_rwn);

    ia_rwn >> sample_number;


    string info_file = get_fwd_idx_info_file_name();
    assert_file_exist("info file", info_file);
    std::ifstream info_ofs(info_file);
    boost::archive::binary_iarchive info_ia(info_ofs);
    if(config.compress_fwd)
        info_ia >> fwd_idx_cp_pointers;
    else
        info_ia >> fwd_idx_ucp_pointers;
}

inline void serialize_bwd_idx(){
    std::ofstream ofs(get_bwd_idx_file_name());
    boost::archive::binary_oarchive oa(ofs);
    oa << idx;
}

inline void deserialize_bwd_idx(){
    string file_name = get_bwd_idx_file_name();
    assert_file_exist("index file", file_name);
    std::ifstream ifs(file_name);
    boost::archive::binary_iarchive ia(ifs);
    ia >> idx;
}


static int random_walk_last(int start, const Graph &graph, int type) {
    int cur = start;
    while (true) {
        double rand01 = rand_double();
        // blocked by large ppr
        if (type == FWD) if (graph.node_rank[cur] < config.num_of_hubs) {
            result.num_block_by_hub++;
            return cur;
        }
        if (rand01 < config.alpha) {
            return cur;
        }
        int sz = (int) graph.g[cur].size();
        if (sz)
            cur = graph.g[cur][lrand() % sz];
    }
}

static void generate_fwd_label(int start, const Graph &graph, int type) {
    /*
     * type is FWD means it will stop when hitting a hub
     * type is HUB means it will generate the simple random walk
     * it will add the number of action to result.fwd_label_size
     * it will generate num_tries random walk
     */
    // Timer timer(5, "generate forward random walk");
    int64 num_tries = 0;
    int64 keep_tries = 0;
    double delta;
    delta = config.fwd_delta;
    num_tries = int64(1 / delta);
    result.num_randwalk += num_tries;
    keep_tries = 1;

    if(graph.g[start].size()==0) {
        dest_nodes[start] +=num_tries;
        return;
    }

    if (config.bwd_prune)
        num_tries *= config.bwd_prune_ratio;

    for (int64 i = 0; i < num_tries; i++) {
        int l = random_walk_last(start, graph, type);
        if(dest_nodes.notexist(l))
            dest_nodes.insert(l, 1);
        else
            dest_nodes[l]+=1;
    }
    int cnt = 0;
    for (int i=0; i<dest_nodes.occur.m_num; i++) {
        int node=dest_nodes.occur[i];
        if (dest_nodes[node] >= keep_tries) {
            cnt += 1;
        }
    }
    result.fwd_label_size += cnt;
}

static void select_forward_hub(const Graph &graph){
    string output_path = config.prefix+FILESEP + "global_pr" + FILESEP;
    output_path +=config.graph_alias+".forward.order";

    if(file_exists_test(output_path) && !config.force_rebuild){
        cerr<<"forward order file already exists "<<endl;
        return;
    }else{
        INFO("generating forward hub");
    }
    Timer timer(TIMER_SELECT_FORWARD_HUB, "select_forward_hub");

    static unordered_map<int, int> walked;
    static vector<int> random_walk_hit;
    static vector<int> accum_benefit;
    static vector<int> accum_benefit_static;
    vector<vector<pair<int,int> > > reverse_list_to_rw;
    vector<vector<int> > stored_walks;

    //vector<map<int, int> > nodes_went_through_walks;
    
    for(int i=0; i<graph.n; i++){
        random_walk_hit.push_back(0);
        accum_benefit.push_back(0);
        accum_benefit_static.push_back(0);
        vector<int> v_tmp;
        //stored_walks.push_back(v_tmp);
        vector<pair<int,int> > rev_rw;
        reverse_list_to_rw.push_back(rev_rw);
    }

    int SELECT_FORWARD_HUB_RW_NUM = 10*graph.n; //3*log(2/config.pfail)/config.epsilon/config.epsilon/config.delta;

    INFO(SELECT_FORWARD_HUB_RW_NUM);

    for(int i=0; i< SELECT_FORWARD_HUB_RW_NUM; i++){
        walked.clear();
        //if(i%100000==0) cout<<i<<endl;
        vector<int> walked_nodes_seq;
        int cur = lrand()%graph.n;
        walked_nodes_seq.push_back(cur);
        walked[cur] = 1;
        while (true) {
            double rand01 = drand();
            if (rand01 < config.alpha) {
                break;
            }
            int sz = (int) graph.g[cur].size();
            if (sz){
                cur = graph.g[cur][lrand() % sz];
                walked_nodes_seq.push_back(cur);
                if(walked.find(cur) == walked.end())
                    walked[cur] = walked_nodes_seq.size();
            }
            else
                break;
            //cur = graph.g[cur][next];
        }

        int walk_id = i;
        for(auto node: walked){
            random_walk_hit[node.first]++;
            reverse_list_to_rw[node.first].push_back(make_pair(walk_id, node.second));
            accum_benefit[node.first] += walked_nodes_seq.size() - node.second;
            accum_benefit_static[node.first] += walked_nodes_seq.size() - node.second;
        }
        stored_walks.push_back(walked_nodes_seq);
    }


    //INFO("Finished random walks, next select hubs and decide the number of stored randomwalks for each hub");
    static BinaryHeap<int, greater<int> > heap(graph.n, greater<int>());
    heap.clear();
    vector<int> order_to_vertex;
    for(int i=0; i<graph.n; i++){
        heap.insert(i, accum_benefit[i]);
        //vertex_to_order.push_back(0);
    }

    int order_cnt =0;
    while(heap.size()){
        auto top = heap.extract_top();
        int benefit = top.first;
        if(benefit == 0) break;
        //INFO(benefit);
        int v = top.second;
        //INFO(v);
        int updated_benefit = accum_benefit[v];
        heap.modify(v, updated_benefit);
        while(heap.extract_top().second != v){
            top = heap.extract_top();
            benefit = top.first;
            v = top.second;
            updated_benefit = accum_benefit[v];
            heap.modify(v, updated_benefit);
        }

        //INFO(v, benefit);

        order_to_vertex.push_back(v);
        //vertex_to_order[v] = order_cnt;
        order_cnt++;
        heap.delete_top();
        accum_benefit[v]=0;
        //update the scores for the remaining nodes;

        auto update_rws = reverse_list_to_rw[v];

        for(int i=0; i<update_rws.size(); i++){
            
            int rw_id = update_rws[i].first;

            vector<int>& rw = stored_walks[rw_id];
            unordered_map<int,int> to_update_helper;
            for(int j=0; j<rw.size(); j++){
                if(to_update_helper.find(rw[j]) == to_update_helper.end())
                    to_update_helper.insert(make_pair(rw[j], j+1));
            }

            if(to_update_helper.find(v) == to_update_helper.end())
                continue;
            int pos = to_update_helper[v];


            for(auto node: to_update_helper){
                if(node.second <pos){
                    accum_benefit[node.first] -= rw.size() - pos;
                    
                }else if( node.second > pos) {
                    accum_benefit[node.first] -= rw.size() - node.second;
                }else{
                    assert(v == node.first);
                }
            }

            rw.erase(rw.begin()+pos -1, rw.end());
        }
    }

    vector<pair<int,int> > remaining_nodes;
    while(heap.size()){
        auto top = heap.extract_top();
        int v = top.second;
        remaining_nodes.push_back(make_pair(random_walk_hit[v], v));
        heap.delete_top();
    }

    sort(remaining_nodes.begin(), remaining_nodes.end());
    std::reverse(remaining_nodes.begin(), remaining_nodes.end());

    for(auto res: remaining_nodes){
        //cout<<res.first<<" "<<res.second<<endl;
        int v = res.second;
        order_to_vertex.push_back(v);
        //vertex_to_order[v] = order_cnt;
        order_cnt++;
    }


    ofstream order_file(output_path);
    for(int i=0; i< order_to_vertex.size(); i++){
        int v = order_to_vertex[i];
        order_file<<v<<" "<<random_walk_hit[v]<<" "<<accum_benefit_static[v]<<" "<<SELECT_FORWARD_HUB_RW_NUM<<endl;
    }
    order_file.close();
}

static void reverse_local_update_for_hub_selection(unordered_map<int, long> &test_hub_list, const vector<int> &target_list, const Graph &graph){
    static vector<bool> idx_flag(graph.n);
    std:fill(idx_flag.begin(), idx_flag.end(), false);

    vector<int> q;
    q.reserve(graph.n);
    q.push_back(-1);
    unsigned long left = 1;

    double rmax = config.bwd_delta;
    INFO(rmax);
    // residual.clear();
    // exist.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();
    
    for(int target: target_list){
        bwd_residuals.insert(target, 1.0);
        q.push_back(target);
        idx_flag[target] = true;//0;
    }

    while (left < q.size()) {
        // get v from q and remove v from q
        // also remove it in index
        int v = q[left];
        idx_flag[v] = false;
        left++;
        if (bwd_residuals[v] < rmax)
            break;

        if(bwd_reserves.notexist(v))
            bwd_reserves.insert(v, bwd_residuals[v] * config.alpha);
        else
            bwd_reserves[v] += bwd_residuals[v] * config.alpha;
        
        test_hub_list[v]+=1; // count one more message passed from node v

        double resi = (1 - config.alpha) * bwd_residuals[v];
        for (int next : graph.gr[v]) {
            int cnt = (int) graph.g[next].size();

            if(bwd_residuals.notexist(next))
                bwd_residuals.insert(next, resi / cnt);
            else
                bwd_residuals[next] += resi / cnt;

            if (bwd_residuals[next] > rmax && idx_flag[next] != true) {
                // put next into q if next is not in q
                idx_flag[next] = true;//(int) q.size();
                q.push_back(next);
            }
        }
        bwd_residuals[v] = 0;
    }
}

static void select_backward_hub(const Graph &graph){
    string output_path = config.prefix +FILESEP + "global_pr" + FILESEP;
    output_path +=config.graph_alias+".backward.order";

    if(file_exists_test(output_path) && !config.force_rebuild){
        cerr<<"backward order file already exists "<<endl;
        return;
    }else{
        INFO("generating backward hub");
    }
    Timer timer(TIMER_SELECT_BWDWARD_HUB, "select_backward_hub");

    int64 omega = 1. /config.fwd_delta;
    INFO(omega);
    vector<int> target_list(omega);
    unordered_map<int, bool> target_flag;
    unordered_map<int, long> test_hub_list; // pair:<node_id, propagated_messages>

    int64 i=0;
    while(true){
        int target = lrand()%graph.n; //select a target node at random
        if(target_flag[target]!=true){
            target_list[i] = target;
            target_flag[target] = true;
            i++;
            if(i>=omega){
                break;
            }
        }
    }

    // start backward propagation from all selected targets
    reverse_local_update_for_hub_selection(test_hub_list, target_list, graph);

    // sort node v by number of propagated messages
    vector<pair<int, long>> hubs(test_hub_list.size());
    partial_sort_copy(test_hub_list.begin(), test_hub_list.end(), hubs.begin(), hubs.end(), [](pair<int, long> const& l, pair<int, long> const& r){return l.second > r.second;});
    
    INFO("writing hub list to disk size=", hubs.size());
    // write potential hubs to disk
    ofstream order_file(output_path);
    for(int i=0; i< hubs.size(); i++){
        int v = hubs[i].first;
        order_file<<v<<" ";
    }
    order_file.close();
}

static inline int random_walk(int start, const Graph &graph) {
    int cur = start;
    while (true) {
        double rand01 = rand_double();
        if (rand01 < config.alpha) {
            return cur;
        }
        int sz = (int) graph.g[cur].size();
        if (sz){
            int k = (sz-1)*rand_double();
            cur = graph.g[cur][k];
        }
        else
            return cur;
    }
}

static inline int random_walk_thread(int start, const Graph &graph, unsigned core_id) {
    int cur = start;
    //int sz = (int) graph.g[start].size();
    //int k = (sz-1)*drand_u(core_id);
    //int random_out_neighbor = graph.g[start][k];
   // int cur = random_out_neighbor;

    while (true) {
        double rand01 = drand_u(core_id);
        if (rand01 < config.alpha) {
            return cur;
        }
        int sz = (int) graph.g[cur].size();
        if (sz){ //cur = graph.g[cur][lrand_u() % sz];
            int k = (sz-1)*drand_u(core_id);
            cur = graph.g[cur][k];
        }
        else
            return cur;
    }
}



static int random_walk_with_forward_oracle(int start, const Graph &graph, unordered_map<int,int> & hub_used_samples) {
    int cur = start;
    while (true) {
        // blocked by large ppr
        // int cur_node_order = forward_node_order[cur];
        // if (cur_node_order< config.num_of_hubs && hub_used_samples[cur_node_order] < config.num_of_fwd_rw){
        if( fwd_idx_size[cur]!=0 && hub_used_samples[cur] < fwd_idx_size[cur]){//config.num_of_fwd_rw ){
            //INFO(cur, forward_node_order[cur], graph.node_rank[cur]);
            result.num_block_by_hub++;
            //assert(fwd_idx_uncompressed.find(cur)!= fwd_idx_uncompressed.end());
            // int node = fwd_idx_uncompressed[cur_node_order][hub_used_samples[cur_node_order]];
            int node = fwd_idx[ fwd_idx_ucp_pointers[cur].first ];
            hub_used_samples[cur]++;
            return node;
            //return -1;
        }
        double rand01 = rand_double();
        if (rand01 < config.alpha) {
            return cur;
        }

        int sz = (int) graph.g[cur].size();
        if (sz)
            cur = graph.g[cur][lrand() % sz];
        else
            return cur;
        //cur = graph.g[cur][next];
    }
}

static int random_walk_with_forward_oracle_thd(int start, const Graph &graph, unsigned core_id) {
    int cur = start;
    while (true) {
        // blocked by large ppr
        // int cur_node_order = forward_node_order[cur];
        if ( fwd_idx_size[cur]!=0 && ( multi_hub_used_samples[core_id].notexist(cur) || multi_hub_used_samples[core_id][cur] < fwd_idx_size[cur] ) ){//config.num_of_fwd_rw ){
            // result.num_block_by_hub++;
            int node = fwd_idx[ fwd_idx_ucp_pointers[cur].first ];
            if(multi_hub_used_samples[core_id].notexist(cur))
                multi_hub_used_samples[core_id].insert(cur, 1);
            else
                multi_hub_used_samples[core_id][cur]+=1;
                
            return node;
        }
        double rand01 = drand_u(core_id);//drand_r(core_id);
        if (rand01 < config.alpha) {
            return cur;
        }

        int sz = (int) graph.g[cur].size();
        if (sz){
            int k = (sz-1)*drand_u(core_id);//drand_r(core_id);
            cur = graph.g[cur][k];
        }
        else
            return cur;
    }
}


static int random_walk_with_compressed_forward_oracle(int start, const Graph &graph) {
    int cur = start;
    //int sz = (int) graph.g[start].size();
    //int k = lrand()%sz;
    //int random_out_neighbor = graph.g[start][k];
    //int cur = random_out_neighbor;
    while (true) {
        // blocked by hub nodes
        // int cur_node_order = forward_node_order[cur];
        if ( fwd_idx_size[cur]!=0 && (hub_used_samples.notexist(cur) || hub_used_samples[cur] <  fwd_idx_size[cur]) ){
            result.num_block_by_hub++;
            //int node = fwd_idx_uncompressed[cur_node_order][hub_used_samples[cur_node_order]];
            if(hub_used_samples.notexist(cur))
                hub_used_samples.insert(cur, 1);
            else
                hub_used_samples[cur]+=1;
            return -1;
        }

        double rand01 = rand_double();
        if (rand01 < config.alpha) {
            return cur;
        }

        int sz = (int) graph.g[cur].size();
        //INFO(walked_hops);
        if (sz){
            int k = (sz-1)*rand_double();
            cur = graph.g[cur][k];
        }
        else
            return cur;
        //cur = graph.g[cur][next];
    }
}


static void check_end_nodes(int source, vector<int> & targets){
    //this should not eliminate any node in our generated topk query, just double confirm.
    
    vector<int> temp;
    for(int i=0; i<targets.size(); i++){
        if(sample_number[targets[i]]== sample_number[source]){
            temp.push_back(targets[i]);
        }
    }

    targets = temp;
}

extern int current_target;
static void query_forward_compressed(int start, const Graph &graph) {
    /*
     * It will generate the simple random walk and
     * add the number of action to result.fwd_label_size
     * it will generate num_tries random walk
     */
    // Timer timer(5, "generate forward random walk");
    int64 num_tries = int64(1/config.fwd_delta/graph.n*sample_number[start]);
    
    // hub_used_samples.clean();
    if(graph.g[start].size()==0) {
        dest_nodes[start] +=num_tries;
        return;
    }else if(sample_number[start]!=sample_number[current_target]) return;

    result.num_randwalk += num_tries;
    

    if(fwd_idx_size[start]!=0){
        hub_used_samples.insert(start, fwd_idx_size[start]);
        result.num_block_by_hub+=fwd_idx_size[start];
        num_tries = max( int64(0),  num_tries - fwd_idx_size[start]);
    }else{
        num_tries = num_tries*(1-config.alpha);
        dest_nodes[start] += num_tries*config.alpha;
    }

    for (int64 i = 0; i < num_tries; i++) {
        if(fwd_idx_size[start] !=0){
            int l = random_walk_with_compressed_forward_oracle(start, graph);
            if(l >=0){
                if(dest_nodes.notexist(l)){
                    dest_nodes.insert(l, 1);
                }
                else{
                    dest_nodes[l]+=1;
                }
            }
        }else{
            int random_out_neighbor = rand_double()*(graph.g[start].size()-1);
            random_out_neighbor = graph.g[start][random_out_neighbor];
            int l = random_walk_with_compressed_forward_oracle(random_out_neighbor, graph);
            if(l >=0){
                if(dest_nodes.notexist(l)){
                    dest_nodes.insert(l, 1);
                }
                else{
                    dest_nodes[l]+=1;
                }
            }
        }
            // else
            //     result.num_block_by_hub++;
    }

    //INFO("forward hub: ", start, ppr.size());
    result.fwd_label_size += dest_nodes.occur.m_num;//ppr.size();
    // return ppr;
}


static inline int get_cur_num_hit_hub(int cur_node){
    int num=0;
    for(auto& fwd: multi_hub_used_samples){
        if(fwd.exist(cur_node)){
            num+=fwd[cur_node];
        }
    }
    return num;
}

static int random_walk_with_compressed_forward_oracle_thd(int start, const Graph &graph, unsigned core_id) {
    // static thread_local vector<int> my_fwd_idx_size(fwd_idx_size); 
    int cur = start;
    while (true) {
        // blocked by hub nodes
        if ( fwd_idx_size[cur]!=0 && (multi_hub_used_samples[core_id].notexist(cur) || get_cur_num_hit_hub(cur) < fwd_idx_size[cur] ) ){
            // multi_hub_used_samples[core_id][cur] <  fwd_idx_size[cur] ) { // note: should be cur-hub-idx-size/num-thread
            if(multi_hub_used_samples[core_id].notexist(cur))
                multi_hub_used_samples[core_id].insert(cur, 1);
            else
                multi_hub_used_samples[core_id][cur]+=1;
            return -1;
        }

        double rand01 =  drand_u(core_id);//drand_r(core_id);
        if (rand01 < config.alpha) {
            return cur;
        }

        int sz = (int) graph.g[cur].size();
        if (sz){
            int k = (sz-1)*drand_u(core_id);//drand_r(core_id);
            cur = graph.g[cur][k];
        }
        else
            return cur;
    }
}

static void query_forward_compressed_thd(int start, const Graph &graph, int64 num_rw_thd) {
    if(fwd_idx_size[start]!=0){
        num_rw_thd = max( int64(0),  num_rw_thd - fwd_idx_size[start] );
        if(multi_hub_used_samples[0].notexist(start))
            multi_hub_used_samples[0].insert(start, fwd_idx_size[start]);
        else
            multi_hub_used_samples[0][start] += fwd_idx_size[start];
    }

    omp_set_num_threads(config.num_thread);
#pragma omp parallel for
    for (int64 i = 0; i < num_rw_thd; i++) {
        unsigned core_id = omp_get_thread_num();
        int l = random_walk_with_compressed_forward_oracle_thd(start, graph, core_id);
        if(l >=0){
            if(multi_dest_nodes[core_id].notexist(l)){
                multi_dest_nodes[core_id].insert(l, 1);
            }
            else{
                multi_dest_nodes[core_id][l]+=1;
            }
        }
    }
}


static vector<unordered_map<int, int>> generate_fwd_label_compressed_binary_multi_set(int node_order, int start, const Graph &graph, int64 num_tries) {
    /*
     * It will generate the simple random walk and
     * add the number of action to result.fwd_label_size
     * it will generate num_tries random walk
     */
    //Timer timer(5, "generate forward random walk");
    // int64 num_tries = 0;
    // double delta;
    // delta = config.fwd_delta;

    // static int64 num_tries = int64(1 / config.fwd_delta );

    // double min_outdegree_of_in_neighbor = graph.n;
    // double avg_outdegree_of_in_neighbor = 0;

    static int64 multi_set_size = 1+ floor(log(num_tries)/log(2));
    int64 check_sample_size=0;

    vector<unordered_map<int,int> > ppr_multi_set(multi_set_size);
    for(int i=0; i<multi_set_size; i++){
        int set_num = min (pow(2, i), num_tries - pow(2, i) + 1);
        
        unordered_map<int,int> m_set;
        // m_set.clear();
        for(int j=0; j<set_num ; j++){
            int l = random_walk(start, graph);
            m_set[l]++;
            check_sample_size++;
        }
        ppr_multi_set[i] = m_set;
        // increased_size+=m_set.size();
        //result.fwd_label_size+= m_set.size();
    }
    return ppr_multi_set;
}

static vector<unordered_map<int, int>> generate_fwd_label_compressed_binary_multi_set_thread(int node_order, int start, const Graph &graph, int64 num_tries, unsigned core_id) {
    /*
     * It will generate the simple random walk and
     * add the number of action to result.fwd_label_size
     * it will generate num_tries random walk
     */
    //Timer timer(5, "generate forward random walk");
    // int64 num_tries = 0;
    // double delta;
    // delta = config.fwd_delta;

    // num_tries = int64(1 / delta );

    // double min_outdegree_of_in_neighbor = graph.n;
    // double avg_outdegree_of_in_neighbor = 0;

    // vector<unordered_map<int,int> > ppr_multi_set;
    static int64 multi_set_size = 1+ floor(log(num_tries)/log(2));
    int64 check_sample_size=0;

    vector<unordered_map<int,int> > ppr_multi_set(multi_set_size);
    for(int i=0; i<multi_set_size; i++){
        int set_num = min (pow(2, i), num_tries - pow(2, i) + 1);
        
        unordered_map<int,int> m_set;
        // m_set.clear();
        for(int j=0; j<set_num ; j++){
            int l = random_walk_thread(start, graph, core_id);
            m_set[l]++;
            check_sample_size++;
        }
        ppr_multi_set[i] = m_set;
        // increased_size+=m_set.size();
        //result.fwd_label_size+= m_set.size();
    }
    return ppr_multi_set;
}

static void read_from_fwd_oracle(int node, const Graph &graph) {
    Timer timer(87, "should minus -- read from fwd oracle");
    generate_fwd_label(node, graph, HUB);
}

static void generate_fwd_randwalk_thread(int start, const Graph &graph, int64 num_rw_thd) {
    omp_set_num_threads(config.num_thread);
#pragma omp parallel for
    for (int64 i = 0; i < num_rw_thd; i++) {
        unsigned core_id = omp_get_thread_num();
        int l = random_walk_thread(start, graph, core_id);//random_walk_thread_r(start, graph, core_id);
        if(multi_dest_nodes[core_id].notexist(l)){
            multi_dest_nodes[core_id].insert(l, 1);
        }
        else{
            multi_dest_nodes[core_id][l]+=1;
        }
    }
}

static void generate_fwd_randwalk_topk(int start, const Graph &graph, int64 num_tries) {
    // static int64 num_tries = int64(1 / config.fwd_delta);
    // result.num_randwalk+=num_tries;
    Timer timer(5, "generate forward random walk");
    if(dest_nodes.occur.m_num>0)
        dest_nodes.clean();
    if(graph.g[start].size()==0) {
        dest_nodes[start] += num_tries;
        return;
    }

    for (int64 i = 0; i < num_tries; i++) {
        int l = random_walk(start, graph);
        if(dest_nodes.notexist(l))
            dest_nodes.insert(l, 1);
        else
            dest_nodes[l]+=1;
    }
}

static void generate_fwd_randwalk_topk_martingale(int start, const Graph &graph, int64 num_tries) {
    // static int64 num_tries = int64(1 / config.fwd_delta);
    // result.num_randwalk+=num_tries;
    Timer timer(5, "generate forward random walk");
    if(graph.g[start].size()==0) {
        dest_nodes[start] += num_tries;
        return;
    }

    int l;
    for (int64 i = 0; i < num_tries; i++) {
        l = random_walk(start, graph);
        if(dest_nodes.notexist(l))
            dest_nodes.insert(l, 1);
        else
            dest_nodes[l]+=1;
    }
}

static void smaple_fwd_thread(int node, const Graph &graph, int64 num_rw_thd){
    Timer timer(5, "generate forward random walk");
    // unordered_map<int, int> &dest = fwd.first;
    // unordered_map<int, int> &hub_hit = fwd.second;
    // dest.clear();
    // hub_hit.clear();

    //set cpu affinity
	// if(-1 == set_cpu(core_id)){
	// 	return;
	// }
    result.num_randwalk += num_rw_thd;

    // use thread local to avoid False-Sharing
    // unordered_map<int, int> dest;
    // unordered_map<int, int> hub_hit;

    if (config.fwd_method == USE_ORACLE) {
        query_forward_compressed_thd(node, graph, num_rw_thd);
    }
    else if (config.fwd_method == NAIVE) {
        generate_fwd_randwalk_thread(node, graph, num_rw_thd);
    }
}





static void calculate_sample_number(const Graph &graph){

    sample_number.reserve(graph.n);
    for(int i=0; i<graph.n; i++){
        sample_number.push_back(0);
    }

    for(int i=0; i<graph.n; i++){
        int start_node = i;
        if(sample_number[start_node]!=0) continue;
        vector<int> bfs_queue;
        bfs_queue.clear();
        bfs_queue.push_back(start_node);
        map<int,int> marker;
        marker.insert(make_pair(start_node, 1));

        for(int j=0; j<bfs_queue.size(); j++){
            int cur_node = bfs_queue[j];
            for(int k=0; k<graph.g[cur_node].size(); k++){
                int out_neighbor = graph.g[cur_node][k];
                if(marker.find(out_neighbor) == marker.end()){
                    bfs_queue.push_back(out_neighbor);
                    marker.insert(make_pair(out_neighbor, 1));
                }else{
                    continue;
                }
            }
            for(int k=0; k<graph.gr[cur_node].size(); k++){
                int in_neighbor = graph.gr[cur_node][k];
                 if(marker.find(in_neighbor) == marker.end()){
                        bfs_queue.push_back(in_neighbor);
                        marker.insert(make_pair(in_neighbor, 1));
                }else{
                    continue;
                }
            }
        }
        int sample_number_tmp = bfs_queue.size();
        for(int j=0; j<bfs_queue.size(); j++){
            int node = bfs_queue[j];
            sample_number[node] = sample_number_tmp;
            //INFO(node, sample_number[node]);
        }
    }
}



static void sample_fwd(int node, const Graph &graph) {
    // sample forward random walk start from node
    // there are 3 cases
    // with full oracle, return a vector read from oracle
    // with naive, generate random walk
    Timer timer(5, "generate forward random walk");
    dest_nodes.clean();
    hub_used_samples.clean();

    result.fwd_label_size = 0;

    //INFO("fwd method", config.fwd_method);
    
    if (config.fwd_method == FULL_PRECOMPUTE) {
        read_from_fwd_oracle(node, graph);
    }
    else if (config.fwd_method == NAIVE) {
        assert(config.num_of_hubs == 0);
        generate_fwd_label(node, graph, HUB);
    }
    else if (config.fwd_method == USE_ORACLE) {
        query_forward_compressed(node, graph);
    }
    else {
        cerr << "forward method not recognized" << endl;
        exit(1);
    }
}

static void reverse_local_update_heap(int t, const Graph &graph, double init_residual = 1) {
    static BinaryHeap<double, greater<double> > heap(graph.n, greater<double>());
    // static map<int, double> pi;

    // Timer timer(6, "generate backward propagation");
    double myeps = config.bwd_delta;

    // pi.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();

    heap.clear();

    heap.insert(t, init_residual);
//    cerr << "init" << endl;
    while (heap.size()) {
        //cerr << "heapsize " << heap.size() << endl;
//        heap.display();
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;
//        cerr << "v " << v << endl;
        if (config.bwd_prune) {
            int sz = (int) graph.gr[v].size();
            double c = sz / graph.get_avg_degree();
            c = min(c, config.bwd_prune_ratio);
            c = max(c, 1 / config.bwd_prune_ratio);
            if (residual < c * myeps)
                continue;
        }
        else {
            if (residual < myeps)
                break;
        }
        heap.delete_top();
        if(bwd_reserves.notexist(v)){
            bwd_reserves.insert(v, residual * config.alpha);
        }
        else{
            bwd_reserves[v]+=residual * config.alpha;
        }
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
    result.count_exist += bwd_reserves.occur.m_num;
    result.count_residual += heap.size();
    // return MP(pi, heap.as_map());
    for(auto item: heap.get_elements()){
        bwd_residuals.insert(item.second, item.first);
    }
}

static void reverse_local_update_heap_map(int t, const Graph &graph, unordered_map<int, double>& map_reserve, unordered_map<int, double>& map_residual) {
    static BinaryHeap<double, greater<double> > heap(graph.n, greater<double>());

    Timer timer(6, "generate backward propagation");
    double myeps = config.bwd_delta;

    map_reserve.clear();
    map_residual.clear();

    heap.clear();

    heap.insert(t, 1);

    while (heap.size()) {
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;

        if (residual < myeps)
            break;

        heap.delete_top();
        map_reserve[v] += residual * config.alpha;

        for (int next : graph.gr[v]) {
            int cnt = (int) graph.g[next].size();
            double delta = ((1 - config.alpha) * residual) / cnt;
            if (heap.has_idx(next))
                heap.modify(next, heap.get_value(next) + delta);
            else
                heap.insert(next, delta);
        }
    }

    map_residual = heap.as_unordered_map();
}

static Bwdidx reverse_local_update_heap_thread_for_build_oracle(int t, const Graph &graph, double init_residual = 1) {
    static thread_local BinaryHeap<double, greater<double> > heap(graph.n, greater<double>());
    static thread_local map<int, double> pi;

    double myeps = config.bwd_delta;

    pi.clear();

    heap.clear();

    heap.insert(t, init_residual);
//    cerr << "init" << endl;
    while (heap.size()) {
        //cerr << "heapsize " << heap.size() << endl;
//        heap.display();
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;
//        cerr << "v " << v << endl;
        if (config.bwd_prune) {
            int sz = (int) graph.gr[v].size();
            double c = sz / graph.get_avg_degree();
            c = min(c, config.bwd_prune_ratio);
            c = max(c, 1 / config.bwd_prune_ratio);
            if (residual < c * myeps)
                continue;
        }
        else {
            if (residual < myeps)
                break;
        }
        heap.delete_top();
        pi[v] += residual * config.alpha;
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

    return MP(pi, heap.as_map());
}

static void reverse_local_update_heap_thread(int t, const Graph &graph, BinaryHeap<double, greater<double> >& heap, double init_residual = 1) {
    // map<int, double> &pi = rtn.first;

    // Timer timer(6, "generate backward propagation");
    double myeps = config.bwd_delta;

    // pi.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();

    heap.insert(t, init_residual);

    while (heap.size()) {
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;
        if (config.bwd_prune) {
            int sz = (int) graph.gr[v].size();
            double c = sz / graph.get_avg_degree();
            c = min(c, config.bwd_prune_ratio);
            c = max(c, 1 / config.bwd_prune_ratio);
            if (residual < c * myeps)
                continue;
        }
        else {
            if (residual < myeps)
                break;
        }
        heap.delete_top();
        if(bwd_reserves.notexist(v)){
            bwd_reserves.insert(v, residual * config.alpha);
        }
        else{
            bwd_reserves[v]+=residual * config.alpha;
        }
        for (int next : graph.gr[v]) {
            int cnt = (int) graph.g[next].size();
            double delta = ((1 - config.alpha) * residual) / cnt;
            if (heap.has_idx(next))
                heap.modify(next, heap.get_value(next) + delta);
            else
                heap.insert(next, delta);
        }
    }
    // result.count_exist += pi.size();
    // result.count_residual += heap.size();
    // return MP(pi, heap.as_map());
    // rtn.second = heap.as_map();
    for(auto item: heap.get_elements()){
        bwd_residuals.insert(item.second, item.first);
    }
}

static void reverse_local_update_linear(int t, const Graph &graph, double init_residual = 1) {
    // map<int, double> residual;
    // map<int, double> exist;

    static vector<bool> idx_flag(graph.n);
    std:fill(idx_flag.begin(), idx_flag.end(), false);

    vector<int> q;
    q.reserve(graph.n);
    q.push_back(-1);
    unsigned long left = 1;

    double myeps = config.bwd_delta;
    // residual.clear();
    // exist.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();

    q.push_back(t);
    bwd_residuals.insert(t, init_residual);

    idx_flag[t] = true;//0;
    while (left < q.size()) {
        // get v from q and remove v from q
        // also remove it in index
        int v = q[left];
        idx_flag[v] = false;
        left++;
        if (bwd_residuals[v] < myeps)
            break;

        if(bwd_reserves.notexist(v))
            bwd_reserves.insert(v, bwd_residuals[v] * config.alpha);
        else
            bwd_reserves[v] += bwd_residuals[v] * config.alpha;

        double resi = (1 - config.alpha) * bwd_residuals[v];
        for (int next : graph.gr[v]) {
            result.count_propagate_edge++;
            int cnt = (int) graph.g[next].size();

            if(bwd_residuals.notexist(next))
                bwd_residuals.insert(next, resi / cnt);
            else
                bwd_residuals[next] += resi / cnt;

            if (bwd_residuals[next] > myeps && idx_flag[next] != true) {
                // put next into q if next is not in q
                idx_flag[next] = true;//(int) q.size();
                q.push_back(next);
            }
        }
        bwd_residuals[v] = 0;
    }

    // return MP(exist, residual);
}

static void reverse_local_update_linear_thread(int t, const Graph &graph, double init_residual = 1) {
    // map<int, double> residual;
    // map<int, double> exist;
    static thread_local vector<bool> idx_flag(graph.n);

    std:fill(idx_flag.begin(), idx_flag.end(), false);
    vector<int> q;
    q.reserve(graph.n);
    q.push_back(-1);
    unsigned long left = 1;

    double myeps = config.bwd_delta;
    // residual.clear();
    // exist.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();

    q.push_back(t);
    bwd_residuals.insert(t, init_residual);

    idx_flag[t] = true;//0;
    while (left < q.size()) {
        // get v from q and remove v from q
        // also remove it in index
        int v = q[left];
        idx_flag[v] = false;
        left++;
        if (bwd_residuals[v] < myeps)
            break;

        if(bwd_reserves.notexist(v))
            bwd_reserves.insert(v, bwd_residuals[v] * config.alpha);
        else
            bwd_reserves[v] += bwd_residuals[v] * config.alpha;

        double resi = (1 - config.alpha) * bwd_residuals[v];
        for (int next : graph.gr[v]) {
            result.count_propagate_edge++;
            int cnt = (int) graph.g[next].size();
            if(bwd_residuals.notexist(next))
                bwd_residuals.insert(next, resi / cnt);
            else
                bwd_residuals[next] += resi / cnt;

            if (bwd_residuals[next] > myeps && idx_flag[next] != true) {
                // put next into q if next is not in q
                idx_flag[next] = true;//(int) q.size();
                q.push_back(next);
            }
        }
        bwd_residuals[v] = 0;
    }
    // return MP(exist, residual);
}

static void load_forward_oracle(const Graph &graph) {
    Timer timer(11, "loading forward oracle");

    deserialize_fwd_idx();

    // fwd_idx_size.initialize(graph.n);
    fwd_idx_ptrs.resize(graph.n);
    fwd_idx_size.resize(graph.n);
    std::fill(fwd_idx_size.begin(), fwd_idx_size.end(), 0);
    fwd_idx_size_k.initialize(graph.n);
    // statistic_hit_number.initialize(graph.n);

    config.num_of_hubs = fwd_idx_cp_pointers.size();
    // config.num_of_fwd_rw = ;

    if(config.compress_fwd){
        for(auto &ptrs: fwd_idx_cp_pointers){
            int node = ptrs.first;
            int size=0;

            int64 ptr = ptrs.second[0];
            int64 end_ptr = ptrs.second[ptrs.second.size()-1];
            for(; ptr<end_ptr; ptr+=2){
                size += fwd_idx[ptr+1];
            }

            fwd_idx_ptrs[node] = ptrs.second;

            // fwd_idx_size.insert(node, size);
            fwd_idx_size[node] = size;

            if(!config.num_of_fwd_rw)
                config.num_of_fwd_rw = size;

            int u = 1 + floor(log( fwd_idx_size[node]*1.0 )/log(2)); //we can pre-compute to avoid reduplicate computation
            int k = pow(2, u-1)-1;
            fwd_idx_size_k.insert(node, k);
        }

        fwd_idx_cp_pointers.clear();
    }
    else{
        for(auto &ptrs: fwd_idx_ucp_pointers){
            // fwd_idx_size.insert(ptrs.first, ptrs.second.second);
            fwd_idx_size[ptrs.first] = ptrs.second.second;
            if(!config.num_of_fwd_rw)
                config.num_of_fwd_rw = fwd_idx_size[ptrs.first];
        }
    }

    INFO("load_forward_oracle finished");
}

static void load_backward_oracle(const Graph &graph) {
    Timer timer(12, "loading backwrad oracle");
    deserialize_bwd_idx();
    assert(idx.size() > 0);
    INFO("load backward index finish");
}


static int reverse_local_update_topk(int s, int t, const Graph &graph, double& reserve, unordered_map<int, double>& map_residual, double rmax) {
    Timer timer(6, "generate backward propagation");
    int push_count=0;
    while (map_residual.size()) {
        auto max_node = std::max_element(map_residual.begin(), map_residual.end(), [](const pair<int, double>& p1, const pair<int, double>& p2) {return p1.second > p2.second; });
        int v = max_node->first;
        if (map_residual[v] < rmax)
            break;
        if (idx.find(v) != idx.end()) {
            vector<BwdidxWithResidual> &idxv = idx[v];
            int i;
            for (i = (int) (idxv.size() - 1); i >= 0; i--) {
                BwdidxWithResidual &x = idxv[i];
                if (x.first >= map_residual[v]) {
                    Bwdidx &useidx = x.second;
                    if(useidx.first.find(s) != useidx.first.end())
                        reserve += useidx.first[s]*map_residual[v]/x.first;
                    if (useidx.second.size() <= 1){
                        bwd_residuals[v] = 0;
                        break;
                    }
                    for (auto &residualkv:useidx.second) {
                        int next = residualkv.first;
                        double delta = residualkv.second * map_residual[v] / x.first;

                        map_residual[next] +=delta;
                    }
                    map_residual[v] = 0;
                    break;
                }
            }
            if(i>=0){
                continue;
            }
        }
        if(v == s)
            reserve += map_residual[v] * config.alpha;

        double resi = (1 - config.alpha) * map_residual[v];
        push_count+=graph.gr[v].size();
        for (int next: graph.gr[v]) {
            int cnt = graph.g[next].size();
            map_residual[next] += resi / cnt;
        }

        map_residual[v] = 0;
        // map_residual.erase(v);
    }
    return push_count;
}

static void reverse_local_update_linear_with_oracle(int t, const Graph &graph) {
    static vector<bool> idx_flag(graph.n);
    std:fill(idx_flag.begin(), idx_flag.end(), false);

    vector<int> q;
    q.reserve(2*graph.n);
    q.push_back(-1);
    unsigned long left = 1;

    double myeps = config.bwd_delta;
    bwd_reserves.clean();
    bwd_residuals.clean();

    q.push_back(t);
    bwd_residuals.insert(t, 1);

    idx_flag[t] = true;
    while (left < q.size()) {
        int v = q[left];
        idx_flag[v] = false;
        left++;
        if (bwd_residuals[v] < myeps)
            break;
        
        if (idx.find(v) != idx.end()) {
            vector<BwdidxWithResidual> &idxv = idx[v];
            for (int i = (int) (idxv.size() - 1); i >= 0; i--) {
                BwdidxWithResidual &x = idxv[i];
                if (x.first >= bwd_residuals[v]) {
                    Bwdidx &useidx = x.second;
                    for (auto &residualkv:useidx.first) {
                        int next = residualkv.first;
                        double delta = residualkv.second * bwd_residuals[v] / x.first;
                        if(bwd_reserves.notexist(next)){
                            bwd_reserves.insert(next, delta);
                        }
                        else{
                            bwd_reserves[next]+=delta;
                        }
                    }
                    if (useidx.second.size() <= 1){
                        bwd_residuals[v] = 0;
                        break;
                    }
                    for (auto &residualkv:useidx.second) {
                        int next = residualkv.first;
                        double delta = residualkv.second * bwd_residuals[v] / x.first;

                        if(bwd_residuals.notexist(next))
                            bwd_residuals.insert(next, delta);
                        else
                            bwd_residuals[next] +=delta;

                        if (bwd_residuals[next] >= myeps && idx_flag[next] != true) {
                            // put next into q if next is not in q
                            idx_flag[next] = true;//(int) q.size();
                            q.push_back(next);
                        }
                    }
                    bwd_residuals[v] = 0;
                    break;
                }
            }
        }
        else{
        // if(bwd_residuals[v]>=myeps){
            if(bwd_reserves.notexist(v))
                bwd_reserves.insert(v, bwd_residuals[v] * config.alpha);
            else
                bwd_reserves[v] += bwd_residuals[v] * config.alpha;

            double resi = (1 - config.alpha) * bwd_residuals[v];
            for (int next: graph.gr[v]) {
                result.count_propagate_edge++;
                int cnt = (int) graph.g[next].size();

                if(bwd_residuals.notexist(next))
                    bwd_residuals.insert(next, resi / cnt);
                else
                    bwd_residuals[next] += resi / cnt;

                if (bwd_residuals[next] >= myeps && idx_flag[next] != true) {
                    // put next into q if next is not in q
                    idx_flag[next] = true;//(int) q.size();
                    q.push_back(next);
                }
            }
            bwd_residuals[v] = 0;
        }

    }
}

static void reverse_local_update_with_oracle(int t, const Graph &graph) {

    static BinaryHeap<double, greater<double> > heap(graph.n, greater<double>()); // residual heap
    // static map<int, double> pi; // the reverse propagation lower bound estimation

    // Timer timer(6, "generate backward propagation with oracle");

    double myeps = config.bwd_delta;

    // pi.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();

    heap.clear();

    heap.insert(t, 1);
    while (heap.size()) {
//        INFO(heap.size());
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;
//        INFO(v);
        if (residual < myeps){
            break;
        }
        heap.delete_top();

        // v is in the index
        if (idx.find(v) != idx.end()) {
            result.num_use_backward_oracle++;
//            INFO("index find");
            vector<BwdidxWithResidual> &idxv = idx[v];
            for (int i = (int) (idxv.size() - 1); i >= 0; i--) {
                BwdidxWithResidual &x = idxv[i];
                if (x.first >= residual) {
//                    INFO("index choose");
                    Bwdidx &useidx = x.second;
                    for (auto residualkv:useidx.first) {
                        int next = residualkv.first;
                        double delta = residualkv.second * residual / x.first;
                        if(bwd_reserves.notexist(next)){
                            bwd_reserves.insert(next, delta);
                        }
                        else{
                            bwd_reserves[next]+=delta;
                        }
                    }
                    if (useidx.second.size() <= 1)
                        break;
                    for (auto residualkv:useidx.second) {
                        int next = residualkv.first;
                        double delta = residualkv.second * residual / x.first;

                        if (heap.has_idx(next))
                            heap.modify(next, heap.get_value(next) + delta);
                        else
                            heap.insert(next, delta);
                    }
                    break;
                }
        
            }
        }
        else {
            if(bwd_reserves.notexist(v)){
                bwd_reserves.insert(v, residual * config.alpha);
            }
            else{
                bwd_reserves[v]+=residual * config.alpha;
            }
            for (int next : graph.gr[v]) {
                int cnt = (int) graph.g[next].size();
                double delta = ((1 - config.alpha) * residual) / cnt;
//                INFO(next, delta);
                if (heap.has_idx(next))
                    heap.modify(next, heap.get_value(next) + delta);
                else
                    heap.insert(next, delta);
            }
        }
    }
//    for (auto kv: pi) {
//        cerr << kv.first << " " << kv.second << endl;
//    }

//    heap.display();
    result.count_exist += bwd_reserves.occur.m_num;
    result.count_residual += heap.size();
//    INFO(pi.size(), heap.size());
    // return MP(pi, heap.as_map());
    for(auto item: heap.get_elements()){
        bwd_residuals.insert(item.second, item.first);
    }
}


static void sample_bwd(int node, const Graph &graph) {
    Timer timer(6, "generate backward propagation");
    assert(config.bwd_delta > 0);
    result.count_exist = 0;
    result.count_residual = 0;
    // map<int, double> pi;
    // map<int, double> residual;
    if (config.bwd_method == FULL_PRECOMPUTE) {
        Timer timer(88, "should minus this one");
        reverse_local_update_heap(node, graph);
    }
    else if (config.bwd_method == USE_ORACLE) {
        // reverse_local_update_with_oracle(node, graph);
        reverse_local_update_linear_with_oracle(node, graph);
    }
    else if (config.bwd_method == NAIVE) {
        reverse_local_update_heap(node, graph);
    }
    else {
        cerr << "config.bwd_method cannot recognize" << endl;
        exit(1);
    }
    // assert(false);
    // rtn = MP(pi, residual);
//    avg.update(result.count_residual + result.count_exist);
}

static void reverse_local_update_with_oracle_thread(int t, const Graph &graph, BinaryHeap<double, greater<double> >& heap) {
    // map<int, double> &pi = rtn.first; // the reverse propagation lower bound estimation

    // pi.clear();
    bwd_reserves.clean();
    bwd_residuals.clean();

    double myeps = config.bwd_delta;

    heap.insert(t, 1);
    while (heap.size()) {
        auto top = heap.extract_top();
        double residual = top.first;
        int v = top.second;
        if (residual < myeps)
            break;
        heap.delete_top();

        if (idx.find(v) != idx.end()) {
            // result.num_use_backward_oracle++;
            vector<BwdidxWithResidual> &idxv = idx[v];
            for (int i = (int) (idxv.size() - 1); i >= 0; i--) {
                BwdidxWithResidual &x = idxv[i];
                if (x.first >= residual) {
                    Bwdidx &useidx = x.second;
                    for (auto residualkv:useidx.first) {
                        int next = residualkv.first;
                        double delta = residualkv.second * residual / x.first;
                        // pi[next] += delta;
                        if(bwd_reserves.notexist(next)){
                            bwd_reserves.insert(next, delta);
                        }
                        else{
                            bwd_reserves[next]+=delta;
                        }
                    }
                    if (useidx.second.size() <= 1)
                        break;
                    for (auto residualkv:useidx.second) {
                        int next = residualkv.first;
                        double delta = residualkv.second * residual / x.first;

                        if (heap.has_idx(next))
                            heap.modify(next, heap.get_value(next) + delta);
                        else
                            heap.insert(next, delta);
                    }
                    break;
                }
        
            }
        }
        else {
            if(bwd_reserves.notexist(v)){
                bwd_reserves.insert(v, residual * config.alpha);
            }
            else{
                bwd_reserves[v]+=residual * config.alpha;
            }
            for (int next : graph.gr[v]) {
                int cnt = (int) graph.g[next].size();
                double delta = ((1 - config.alpha) * residual) / cnt;
                if (heap.has_idx(next))
                    heap.modify(next, heap.get_value(next) + delta);
                else
                    heap.insert(next, delta);
            }
        }
    }

    // result.count_exist += pi.size();
    // result.count_residual += heap.size();
    // rtn.second = heap.as_map();
    for(auto item: heap.get_elements()){
        bwd_residuals.insert(item.second, item.first);
    }
}

static void sample_bwd_thread(int node, const Graph &graph, BinaryHeap<double, greater<double> >& heap) {
    assert(config.bwd_delta > 0);
    // result.count_exist = 0;
    // result.count_residual = 0;

    // set cpu affinity
	// if(-1 == set_cpu(0)){
	// 	return;
	// }
    Timer timer(6, "generate backward propagation");

    if (config.bwd_method == USE_ORACLE) {
        reverse_local_update_with_oracle_thread(node, graph, heap);
    }
    else if (config.bwd_method == NAIVE) {
        reverse_local_update_heap_thread(node, graph, heap);
    }
    else {
        cerr << "config.bwd_method cannot recognize" << endl;
        exit(1);
    }
}

#endif
