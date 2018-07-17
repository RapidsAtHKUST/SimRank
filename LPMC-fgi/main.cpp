#include <cstdlib>
#include <ctime>

#include <iostream>
#include <fstream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "stat.h"
#include "bprw.h"
// #include "flpmc.h" 
// #include "bflpmc.h"
#include "simrank.h"
#include <util/graph_yche.h>
#include "rw_hub.h"
#include "fgi.h"
#include "phf.h"
#include "minimal_perfect_hash.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;
using namespace bf;

string TOY_GRAPH = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/cloud_walker2.txt";
string TOY_P = "/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/";

string get_new_graph_path(string data_name){
    return string("/csproject/biggraph/ywangby/yche/git-repos/SimRank/LPMC/build/edge_list/") + data_name + string(".txt");
}

double ground_truth(string data_name, double c, double epsilon, double delta, int x, int y){
    /* compute the ground truth of x and y */
    string path = get_edge_list_path(data_name);
    GraphYche g(path);

    // 1st: init
    BackPush bprw(data_name, g, c, epsilon, delta);
    size_t n = g.n;
    NodePair q{x,y};

    TruthSim ts(data_name, g, c, epsilon);
    // cout << format("ground truth: %s") % ts.sim(x,y) << endl;
    return ts.sim(x,y);
}

void compute_ground_truth(string data_name, double c, double epsilon){
    string path = get_new_graph_path(data_name);
    GraphYche g(path);
    TruthSim ts(data_name, g, c, epsilon);
    ts.run(g);
    ts.save();
}

// void test_FLPMC(string data_name, double c, double epsilon, double delta, int x, int y){
//     DirectedG g;
//     load_graph(get_edge_list_path(data_name), g);
//     size_t n = num_vertices(g);
//     NodePair q{x,y};
//     int round = 1;
//     int failure_count = 0;
//     // ground truth
//     TruthSim ts(data_name, g, c, epsilon);
// 
//     double result;
//     double truth;
//     double max_error = 0;
//     NodePair max_pair;
//     FLPMC flpmc(data_name, g, c, epsilon, delta, 100);
//     // test all-pairs accuracy
//     int total_count = 5;
//     for(int i = 0; i< total_count ;i++){
//         // cout << "--------------" << endl;
//         for(int j = i; j < total_count;j++){
//             result = flpmc.query_one2one(NodePair{i,j});
//             truth = ts.sim(i,j);
//             double error = abs(result - truth);
//             if(error > max_error){
//                 max_error = error;
//                 max_pair.first = i;
//                 max_pair.second = j;
//             }
//             cout << format("(%s,%s): truth:%s query:%s error:%s") % i % j % truth % result % error   << endl;
//             if(error  > 0.01){
//                 failure_count ++;
//             }
//         }
//         // cout << "--------------" << endl;
//         // cout << format("result:%s") % result << endl;
//         // cout << format("error: %s") % error << endl;
//     }
//     // cout << result << endl;
//     // cout << format("ground truth: %s") % truth << endl;
//     // cout << format("error: %s") % (truth - result) << endl;
//     // cout << "indegree: " << in_degree(x,g) << ", " << in_degree(y, g) << endl;
//     // cout << format("ground truth:%s") % truth << endl;
//     cout << format("pair (%s,%s) max error: %s") % max_pair.first % max_pair.second % max_error  << endl;
//     // cout << format("failure/round: %s/%s") % failure_count % round << endl;
//     // cout << format("failure probability: %s") % (failure_count / double(round)) << endl;
// }


void test_bp(string data_name, double c, double epsilon, double delta, int x, int y) {
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    BackPush bprw(data_name, g, c, epsilon, delta);
    size_t n = g.n;
    NodePair q{x, y};
    // for(int i = 0; i < 100;i++){
    auto start = std::chrono::high_resolution_clock::now();

    double result = bprw.query_one2one(q);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cout << q << ": " << result << endl;


    cout << format("sim: %s:%s") % q % result << endl;

    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
    // ground truth
    double truth = ground_truth(data_name, c, epsilon, delta, x,y);
    cout << format("ground truth: %s") % truth << endl;
    cout << format("error: %s") % (truth - result) << endl;

}

// void test_BFLPMC(string data_name, double c, double epsilon, double delta, int x, int y){
//     DirectedG g;
//     load_graph(get_edge_list_path(data_name), g);
//     size_t n = num_vertices(g);
//     NodePair q{x,y};
//     BFLPMC bflpmc(data_name, g, c, epsilon, delta);
// 
//     double result = bflpmc.query_one2one({x,y});
//     cout << format("result of BFLPMC: %s") % result << endl;
// 
//     double truth = ground_truth(data_name, c, epsilon, delta, x, y);
//     cout << format("ground truth: %s") % truth << endl;
//     cout << format("error: %s") % (truth - result) << endl;
// }

// void test_bflp_all_pair(string data_name){
//     // set up blpmc
//     DirectedG g;
//     load_graph(get_edge_list_path(data_name), g);
//     size_t n = num_vertices(g);
//     double c = 0.6;
//     double epsilon = 0.01;
//     double delta = 0.01;
//     BFLPMC bflpmc(data_name, g, c, epsilon, delta);
// 
//     // set up ground truth
//     TruthSim ts(data_name, g, c, epsilon);
//     double max_error = 0;
// 
//     int error_count = 0;
//     int query_count = 0;
//     auto start = std::chrono::high_resolution_clock::now();
//     for(int i = n-1; i >=0 ; i--){
//         // cout << format("%sth column") % i << endl;
//         for(int j = i ; j < n; j++){
//             query_count ++;
//             double result = bflpmc.query_one2one({i,j});
//             double truth = ts.sim(i,j);
//             double error = abs(truth - result);
//             if(error > max_error){
//                 max_error = error;
//             }
//             if(error > 0.01){
//                 cout << format("pair:%s,%s, query result: %s, truth:%s, error: %s ") % i %j %result  % truth % error << endl;
//                 error_count ++; 
//             }
//         }
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> elapsed = end - start;
// 
//     cout << format("fail count/query number: %s/%s, prob: %s") % error_count % query_count % (double(error_count) / query_count) << endl;
//     cout << format("max error: %s") %  max_error << endl;
//     cout << format("total query cost: %s") %  elapsed.count() << endl;
// 
// }

void test_blpmc(string data_name, int h=1000000, int l=50, int q = 50000){
    string path = get_new_graph_path(data_name);
    GraphYche g(path);
    int max_small_graph_size = 10000; // don't load ground truth

    // GraphYche * g_ptr;
    // g_ptr = &g;
    // for(int i = 0; i < g_ptr->n; i++){
    //     int out_start = g_ptr->off_out[i]; 
    //     int out_end = g_ptr->off_out[i+1];
    //     for(int j = out_start; j < out_end; j++){ // j is not out-neighbor
    //         auto out_neighbor = g_ptr->neighbors_out[j];
    //         cout << format("edge: (%s, %s)") % i % out_neighbor << endl;
    //     }
    // }


    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;
    BLPMC_Config config;
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    BackPush bprw(data_name, g, c, epsilon, delta, config);
    size_t n = g.n;

    // fill in queries
    vector<NodePair> queries;
    for(int i = 0; i <q; i++){
        int a = random_int(0,n);
        int b = random_int(0,n);
        queries.push_back(NodePair(a,b));
    }

    display_seperate_line();

    TruthSim *ts;
    if(n < max_small_graph_size){
        ts = new TruthSim(data_name, g, c, epsilon);
    }
    double max_error = 0;

    auto start = std::chrono::high_resolution_clock::now();
    cout << bprw.config << endl;
    int i = 0;
    for(auto& q: queries){
        // cout << format("querying %s-th node pair.") % i << endl;
        double result = bprw.query_one2one(q);
        if(n < max_small_graph_size){
            double truth = ts->sim(q.first,q.second);
            double abs_error = abs(truth - result);
            if(abs_error > max_error){
                max_error = abs_error;
            }
        }
        i ++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s") % elapsed.count() % max_error << endl;
    display_seperate_line();

    /* test the method for cost estimation */
    int nbh = h;
    // for(int nbh = 1000000; nbh < 10000000; nbh = nbh + 1000000){ // varying number of hubs
    display_seperate_line();
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = true;
    config.number_of_hubs = nbh;
    config.number_of_samples_per_hub = l;
    BackPush bprw2(data_name, g, c, epsilon, delta, config);
    cout << bprw2.config << endl;
    max_error = 0;


    start = std::chrono::high_resolution_clock::now();
    i = 0 ; 
    for(auto& q: queries){
        // cout << format("working on %s") % q << endl;
        // cout << format("querying %s-th node pair") % i << endl;
        double result = bprw2.query_one2one(q);
        i ++;
        if(n < max_small_graph_size){
            double truth = ts->sim(q.first,q.second);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            if(abs_error > max_error){
                max_error = abs_error;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s, #hubs: %s, #rw/hub: %s, #hits of hub: %s, #contain queries:%s, #1s: %s in average") \
        % elapsed.count() % max_error % bprw2.rw_hubs->N % config.number_of_samples_per_hub % bprw2.hub_hits % \
        bprw2.rw_hubs->number_of_contains_queries % (bprw2.rw_hubs->number_of_1s/double(nbh)) << endl;
    display_seperate_line();
    // }
    cout << "close the problem" << endl;
}


double compute_sim_using_pure_MC(DirectedG & g, int x, int y, double c, int N){
    SFMTRand rand_gen;
    double meeting_count = 0;
    for(int i = 0 ; i< N;i++){
        int length = 1;
        while(rand_gen.double_rand() < c){
            length ++;
        }
        int result = sample_an_1c_walk(NodePair{x,y}, g, length, rand_gen);
        meeting_count += result;
    }
    return c * meeting_count / N;
}

void test_blpmc_fgi(string data_name, int h=1000000, int l=1000, int nt = 3443, int q = 50000) {
    string path = get_new_graph_path(data_name);
    GraphYche g(path);
    int max_small_graph_size = 10000; // don't load ground truth

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    BLPMC_Config config;
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = false;
    BackPush bprw(data_name, g, c, epsilon, delta, config);
    size_t n = g.n;

    // fill in queries
    vector<NodePair> queries;
    for(int i = 0; i <q; i++){
        int a = random_int(0,n);
        int b = random_int(0,n);
        queries.push_back(NodePair(a,b));
    }

    display_seperate_line();

    TruthSim *ts;
    if(n < max_small_graph_size){
        ts = new TruthSim(data_name, g, c, epsilon);
    }
    double max_error = 0;
    int exceederr = 0;
    int nonzero = 0;

    auto start = std::chrono::high_resolution_clock::now();
    cout << bprw.config << endl;
    int i = 0;
    for(auto& q: queries){
        // cout << format("querying %s-th node pair.") % i << endl;
        double result = bprw.query_one2one(q);
        if(n < max_small_graph_size){
            double truth = ts->sim(q.first,q.second);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            if(abs_error > max_error){
                max_error = abs_error;
            }
            if (abs_error > 0) ++nonzero;
            if (abs_error > epsilon) {
            	exceederr++;
            	// cout << q << " " << abs_error << endl;
            }
        }
        i ++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s") % elapsed.count() % max_error << endl;
    cout << "Exceed epsilon: " << exceederr << ", Nonzero " << nonzero << endl;
    display_seperate_line();

    /* test the method for cost estimation */
    int nbh = h;
    // // for(int nbh = 1000000; nbh < 10000000; nbh = nbh + 1000000){ // varying number of hubs
    display_seperate_line();
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = true;
    config.number_of_hubs = nbh;
    config.number_of_samples_per_hub = l;
    BackPush bprw2(data_name, g, c, epsilon, delta, config);
    cout << bprw2.config << endl;
    max_error = 0, nonzero = 0, exceederr = 0;


    start = std::chrono::high_resolution_clock::now();
    i = 0 ; 
    for(auto& q: queries){
        // cout << format("working on %s") % q << endl;
        // cout << format("querying %s-th node pair") % i << endl;
        double result = bprw2.query_one2one(q);
        i ++;
        if(n < max_small_graph_size){
            double truth = ts->sim(q.first,q.second);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            if(abs_error > max_error){
                max_error = abs_error;
            }
            if (abs_error > 0) ++nonzero;
            if (abs_error > epsilon) {
                exceederr++;
            }

        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s, #hubs: %s, #rw/hub: %s, #hits of hub: %s, #contain queries:%s, #1s: %s in average") \
        % elapsed.count() % max_error % bprw2.rw_hubs->N % config.number_of_samples_per_hub % bprw2.hub_hits % \
        bprw2.rw_hubs->number_of_contains_queries % (bprw2.rw_hubs->number_of_1s/double(nbh)) << endl;
    cout << "Exceed epsilon: " << exceederr << ", Nonzero " << nonzero << endl;
    display_seperate_line();

    // for(int nbh = 1000000; nbh < 10000000; nbh = nbh + 1000000){ // varying number of hubs
    display_seperate_line();
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = true;
    config.number_of_trees = nt;
    BackPush bprw3(data_name, g, c, epsilon, delta, config);
    cout << bprw3.config << endl;
    max_error = 0, nonzero = 0, exceederr = 0;

    start = std::chrono::high_resolution_clock::now();
    i = 0 ; 
    for(auto& q: queries){
        // cout << format("working on %s") % q << endl;
        // cout << format("querying %s-th node pair") % i << endl;{2308,2129}
        double result = bprw3.query_one2one(q);
        i ++;
        if(n < max_small_graph_size){
            double truth = ts->sim(q.first,q.second);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            if (abs_error > max_error) {
                max_error = abs_error;
            }
            if (abs_error > 0) ++nonzero;
            if (abs_error > epsilon) {
            	exceederr++;
            	// cout << q << " " << abs_error << " " << g.in_deg_arr[q.first] << " " << g.in_deg_arr[q.second] << endl;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s, #trees: %s") \
        % elapsed.count() % max_error % bprw3.fg_idx->N << endl;
    cout << "Exceed epsilon: " << exceederr << ", Nonzero " << nonzero << endl;
    cout << "FGI hits: " << bprw3.fg_idx->fgi_hit << ", FGI misses: " << bprw3.fg_idx->fgi_miss << endl;
    display_seperate_line();

    cout << "close the problem" << endl;
}

void test_topk(string data_name, int q=50000, int k=500, int debug=0) {
    string path = get_new_graph_path(data_name);
    // string path = TOY_P + data_name;
    GraphYche g(path);
    int max_small_graph_size = 10000;
    
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;
    BLPMC_Config config;
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = false;
    BackPush bprw(data_name, g, c, epsilon, delta, config);
    size_t n = g.n;
    double max_error = 0;
    int exceederr = 0;

    vector<NodePair> queries;
    for (int i = 0; i < q; ++i) {
        int a = random_int(0, n);
        int b = random_int(0, n);
        // while (debug && a == b) b = random_int(0, n);
        queries.push_back(NodePair(a, b));
    }

    display_seperate_line();

    TruthSim *ts;
    vector<QPair> ts_topk;
    if (n < max_small_graph_size) {
        ts = new TruthSim(data_name, g, c, epsilon);
        // ts->run(g);
        for (int i = 0; i < q; ++i) {
            ts_topk.push_back({i, ts->sim(queries[i].first, queries[i].second)});
        }
        sort(ts_topk.begin(), ts_topk.end(), [](QPair x, QPair y) {
            return x.second > y.second;
        });
        if (debug) {
            for (int i = 0; i < k; ++i) {
                cout << queries[ts_topk[i].first] << " " << ts_topk[i].second << endl;    
            }
        }
    }

    display_seperate_line();

    auto start = std::chrono::high_resolution_clock::now();
    vector<QPair> topk;
    topk = bprw.top_k_naive(queries, k);
    for (int i = 0; i < k; ++i) {
        if (debug) cout << queries[topk[i].first] << " " << topk[i].second << endl;
        if (n < max_small_graph_size) {
            // double abs_error = abs(topk[i].second - ts_topk[i].second);
            double abs_error = abs(ts->sim(queries[topk[i].first].first, queries[topk[i].first].second) - ts_topk[i].second);
            if (abs_error > max_error) {
                max_error = abs_error;
            }
            if (abs_error > epsilon) {
                exceederr++;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s") % elapsed.count() % max_error << endl;
    cout << "Exceed epsilon: " << exceederr << endl;
    
    display_seperate_line();

    start = std::chrono::high_resolution_clock::now();
    max_error = 0;
    exceederr = 0;
    topk = bprw.top_k(queries, k);
    for (int i = 0; i < k; ++i) {
        if (debug) cout << queries[topk[i].first] << " " << topk[i].second << endl;
        if (n < max_small_graph_size) {
            // double abs_error = abs(topk[i].second - ts_topk[i].second);
            double abs_error = abs(ts->sim(queries[topk[i].first].first, queries[topk[i].first].second) - ts_topk[i].second);
            if (abs_error > max_error) max_error = abs_error;
            if (abs_error > epsilon) exceederr++;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s") % elapsed.count() % max_error << endl;
    cout << "Exceed epsilon: " << exceederr << endl;
}

void test_blpmc_sp(string data_name, int h=1000000, int l=50, int nt = 3443, int q = 50000, int x = 2308, int y = 2129){
    string path = get_new_graph_path(data_name);
    GraphYche g(path);
    int max_small_graph_size = 10000; // don't load ground truth

    cout << endl << endl << format("Pair (%s, %s)") % x % y << endl;
    //cout << "in deg: " << g.in_deg_arr[x] << " " << g.in_deg_arr[y] << endl;
    //cout << "out deg: " << g.out_deg_arr[x] << " " << g.out_deg_arr[y] << endl;

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;
    BLPMC_Config config;
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = false;
    BackPush bprw(data_name, g, c, epsilon, delta, config);
    size_t n = g.n;

    display_seperate_line();

    TruthSim *ts;
    if(n < max_small_graph_size){
        ts = new TruthSim(data_name, g, c, epsilon);
    }
    double max_error = 0;
    int exceederr = 0;
    int nonzero = 0;

    auto start = std::chrono::high_resolution_clock::now();
    cout << bprw.config << endl;
    int i = 0;
    for(int iter = 0; iter < q; ++iter) {
        // cout << format("querying %s-th node pair.") % i << endl;
        double result = bprw.query_one2one({x,y});
        if(n < max_small_graph_size){
            double truth = ts->sim(x,y);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            if(abs_error > max_error){
                max_error = abs_error;
            }
            if (abs_error > 0) ++nonzero;
            if (abs_error > epsilon) {
            	exceederr++;
            	// cout << q << " " << abs_error << endl;
            }
        }
        i ++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << format("> Total cost: %s, Maximum error: %s") % elapsed.count() % max_error << endl;
    cout << "> Exceed epsilon: " << exceederr << ", Nonzero " << nonzero << endl;
    display_seperate_line();
/*
    int nbh = h;
    // for(int nbh = 1000000; nbh < 10000000; nbh = nbh + 1000000){ // varying number of hubs
    display_seperate_line();
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = true;
    config.number_of_hubs = nbh;
    config.number_of_samples_per_hub = l;
    BackPush bprw2(data_name, g, c, epsilon, delta, config);
    cout << bprw2.config << endl;
    max_error = 0, nonzero = 0, exceederr = 0;

    start = std::chrono::high_resolution_clock::now();
    i = 0 ; 
    for(int iter = 0; iter < q; ++iter) {
        // cout << format("working on %s") % q << endl;
        // cout << format("querying %s-th node pair") % i << endl;
        double result = bprw2.query_one2one({x, y});
        i ++;
        if(n < max_small_graph_size){
            double truth = ts->sim(x, y);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            if(abs_error > max_error){
                max_error = abs_error;
            }
            if (abs_error > 0) ++nonzero;
            if (abs_error > epsilon) {
            	exceederr++;
            	// cout << q << " " << abs_error << endl;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format(">> Total cost: %s, Maximum error: %s, #hubs: %s, #rw/hub: %s, #hits of hub: %s, #contain queries:%s, #1s: %s in average") \
        % elapsed.count() % max_error % bprw2.rw_hubs->N % config.number_of_samples_per_hub % bprw2.hub_hits % \
        bprw2.rw_hubs->number_of_contains_queries % (bprw2.rw_hubs->number_of_1s/double(nbh)) << endl;
    cout << ">> Exceed epsilon: " << exceederr << ", Nonzero " << nonzero << endl;
    display_seperate_line();
*/
    display_seperate_line();
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = true;
    config.number_of_trees = nt;
    BackPush bprw3(data_name, g, c, epsilon, delta, config);
    cout << bprw3.config << endl;
    max_error = 0, exceederr = 0, nonzero = 0;

    start = std::chrono::high_resolution_clock::now();
    i = 0 ;
    for(int iter = 0; iter < q; ++iter){
        // cout << format("working on %s") % q << endl;
        // cout << format("querying %s-th node pair") % i << endl;{2308,2129}
        // display_seperate_line();
        double result = bprw3.query_one2one({x,y});
        i ++;
        if(n < max_small_graph_size){
            double truth = ts->sim(x,y);
            double abs_error = abs(truth - result);
            // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
            // cout << format("(%s,%s), truth: %s, result: %s, error: %s")
            //                % i % j % truth->sim(i,j) % r % abs(r - truth->sim(i, j))
            //                << endl;
            // double mc_result = compute_sim_using_pure_MC(g,x,y,c,9537) ;
            // double mc_error = abs(truth - mc_result) ;
            // cout << format("pure MC sampling result: %s, error: %s") % mc_result % mc_error << endl;
            if(abs_error > max_error){
                max_error = abs_error;
            }
            if (abs_error > 0) ++nonzero;
            if (abs_error > epsilon) {
            	exceederr++;
            	// cout << q << " " << abs_error << endl;
            }
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format(">>> Total cost: %s, Maximum error: %s, #trees: %s") \
        % elapsed.count() % max_error % bprw3.fg_idx->N << endl;
    cout << ">>> Exceed epsilon: " << exceederr << ", Nonzero " << nonzero << endl;
    cout << "FGI hits: " << bprw3.fg_idx->fgi_hit << ", FGI misses: " << bprw3.fg_idx->fgi_miss << endl;
    display_seperate_line();

    cout << "close the problem" << endl;
}

void test_lr(){
    // test linear regression module
    string path = get_edge_list_path("ca-GrQc");
    GraphYche g(path);

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;
    BackPush bprw("ca-GrQc", g, c, epsilon, delta);
    size_t n = g.n;
    bprw.build_cost_estimation_model();

    // LinearRegression lr(10,3);
    // vector<vector<double>> X;
    // vector<double> Y;
    // int N = 10;
    // int p = 3;
    // for(int i = 0; i< N; i++){
    //     vector<double> x;
    //     for(int j = 0; j < p; j++){
    //         x.push_back(i + j);
    //     }
    //     X.push_back(x);
    //     Y.push_back( 2 * x[0] + 3 * x[1] + 4 * x[2]);
    // }

    // lr.fit(X,Y);
    // vector<double> x{1,3,5};
    // cout << lr.predict(x) << endl;
}

int test_hub(string data_name){
    string path = get_edge_list_path("ca-GrQc");
    GraphYche g(path);

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    BLPMC_Config config;
    config.is_use_hub_idx = true;
    BackPush bprw("ca-GrQc", g, c, epsilon, delta, config);

    NodePair np{15,433};
    int number_of_meets = bprw.rw_hubs->query_1s(np, 1000);
    cout << format("number of meets: %s") % number_of_meets << endl;
}

void test_fgi() {
    string path = get_edge_list_path("ca-GrQc");
    GraphYche g(path);

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    BLPMC_Config config;
    config.is_use_fg_idx = true;
    BackPush bprw("ca-GrQc", g, c, epsilon, delta, config);

    NodePair np{15,433};
}

int test_bf(){
	//tells bbhash to use included hash function working on u_int64_t input keys:
    SFMTRand rand_gen;
    int length = 20000000;
    unsigned int MAX_VALUE = 100000000;

    // generate input key
    std::unordered_set<unsigned int> s;
    for(int i = 0; i< length;i++){
        unsigned int rand_key = int(rand_gen.double_rand() * MAX_VALUE);
        s.insert(rand_key);
    }
	//tells bbhash to use included hash function working on u_int64_t input keys :
    typedef boomphf::SingleHashFunctor<u_int64_t>  hasher_t;
	typedef boomphf::mphf<  u_int64_t, hasher_t  > boophf_t;
    vector<unsigned int> input_keys;
    for(auto & item: s){
        input_keys.push_back(item);
    }
	 
	// ... fill the input_keys vector
	//build the mphf  
	boophf_t * bphf = new boomphf::mphf<size_t,hasher_t>(input_keys.size(),input_keys,8);
	 
	 //query the mphf :

    // int number_of_unique_keys =  PHF::uniq(&input_keys[0], input_keys.size());
    // cout << format("number of unique keys: %s") % number_of_unique_keys << endl;
    // phf f;
    // minimal_perfect_hash::MinimalPerfectHash<unsigned int> mph;
    // int r = mph.Build(input_keys);
    // assert(r == 0);
    // PHF::init<unsigned int, false>(&f, &input_keys[0], number_of_unique_keys, 0, 100, (unsigned int)rand_gen.double_rand() * 1000000);
    // for(int i = 0; i< length;i++){
    //     unsigned int hased_key = PHF::hash(&f, input_keys[i]) ;
    //     cout << format("key: %s, hashed key: %s") % input_keys[i] % hased_key << endl;
    //     // unsigned int hased_key = bphf->lookup(input_keys[i]);
    //     // cout << format("input key: %s, hashed key: %s") % input_keys[i] % bphf->lookup(input_keys[i]) << endl;
    //     s.insert(hased_key);
    // }
    std::unordered_set<u_int64_t> hashed_keys;
    for(auto & key: input_keys){
        size_t hashed_key = bphf->lookup(key);
        // auto hashed_key = mph.GetHash(key);
        hashed_keys.insert(hashed_key);
        // cout << format("key: %s, hased key: %s") % key % hashed_key << endl;
    }
    cout << format("number of unique keys: %s") % s.size() << endl;
    cout << format("number of unique hased keys: %s") % hashed_keys.size() << endl;
    cout << format("maximum id of key: %s") % (*std::max_element(hashed_keys.begin(), hashed_keys.end())) << endl;

    // cout << "random keys" << endl;
    // for(int i = 0; i< 1000;i++){
    //     unsigned int rand_key = int(rand_gen.double_rand() * MAX_VALUE);
    //     cout << format("key: %s, hashed key: %s") % rand_key % PHF::hash(&f, rand_key) << endl;
    // }
}

void test_bloomfilter(){
	// Construction.
	bloom_filter* bf = new basic_bloom_filter(0.8, 100);

	// Addition.
	bf->add("foo");
	bf->add(42);
    bf->add(Custome_HASH_NP{}(NodePair{4,3}));

	// Lookup.
	cout << bf->lookup("foo")  << endl;
	cout << bf->lookup(42)  << endl;
	cout << bf->lookup(34)  << endl;
    cout << bf->lookup(Custome_HASH_NP{}(NodePair{4,3})) << endl;
    cout << bf->lookup(Custome_HASH_NP{}(NodePair{3,4})) << endl;

	// Remove all elements from the Bloom filter.
	bf->clear();
}

void test_Distance_1s(){
    // int max_length = 2;
    // vector<size_t> positions{1};
    // Distanct_1s d;
    // d.init(positions, max_length);

    // for(auto & item: d.next){
    //     cout << format("%s, ") % item ;
    // }
    // cout << endl;

    // for(int i = 0 ; i < 100; i++){
    //     int j = d.get();
    //     // cout << format(" distance to next 1: %s, current i: %s, current return: %s ") % d.energy % d.i % j << endl;
    //     cout << format("%s ") % j;
    // }
    // cout << endl;
}

void test_hash64(){
    NodePair np{34,56};
    boost::dynamic_bitset<> left(sizeof(np.first) * 8, np.first);
    boost::dynamic_bitset<> right(sizeof(np.second) * 8, np.second);
    size_t x = Custome_HASH_NP{}(np);
    boost::dynamic_bitset<> hashed(sizeof(x) * 8,x);
    size_t y = 0;
    cout << left << endl;
    cout << right << endl;
    cout << hashed << endl;
    cout << format("size of np: %s, size of hashed: %s") % sizeof(np.first) % sizeof(x) << endl;
    cout << format("size of size_t: %s") % sizeof(size_t) << endl;

    google::dense_hash_map<NodePair, int, Custome_HASH_NP, Custome_EQ_NP> h;
    h.set_empty_key(NodePair{0,0});
    h[NodePair{2,3}] = 1;
    h[NodePair{2,2}] = 1;
    h[NodePair{2,2}] = 200;
    for(auto & item : h){
        cout << format("key: %s, value: %s") % item.first % item.second << endl;
    }
}

void sort_graph_node_based_on_utility(string data_name){
    string path = get_edge_list_path(data_name);
    GraphYche g(path);
    GraphYche * g_ptr = &g;
    vector<pair<size_t, double>> utility(g_ptr->n); // utility for each node: node id, utility
    vector<unsigned int> rank(g_ptr->n);
    auto n = g_ptr->n;
    // fill in the utility table
    cout << "filling in the utility values..." << endl;
    for(int i = 0; i < g_ptr->n; i++){
        int out_start = g_ptr->off_out[i]; 
        int out_end = g_ptr->off_out[i+1];
        utility[i].first = i;
        for(int j = out_start; j < out_end; j++){
            auto out_neighbor = g_ptr->neighbors_out[j];
            int in_deg = g_ptr->in_degree(out_neighbor);
            if(in_deg > 0){
                utility[i].second += (1.0 / double(in_deg));
            }
        }
    }
    cout << "sorting the index" << endl;
    sort(utility.begin(), utility.end(), sort_hub_pred());

    // fill in the rank
    cout << "filling in the rank array" << endl;
    for(int i = 0; i< n;i++){
        rank[utility[i].first] = i;
    }

    // flush out the converted edges
    string output_path = string("./edge_list/") + data_name + string(".txt");
    ofstream outfile;
    outfile.open(output_path);

    vector<pair<int,int>> new_edge_list;
    for(int i = 0; i < g_ptr->n; i++){
        int out_start = g_ptr->off_out[i]; 
        int out_end = g_ptr->off_out[i+1];
        for(int j = out_start; j < out_end; j++){ // j is not out neighbor of i, the out_neighbor is
            auto out_neighbor = g_ptr->neighbors_out[j];
            int new_i = rank[i];
            int new_out = rank[out_neighbor];
            new_edge_list.push_back({new_i,new_out});
        }
    }
    for(auto & item:new_edge_list){
        outfile << format("%s %s") % item.first % item.second << endl;
    }

    outfile.close();
}

void test_new_graph(string data_name){
    string path = get_new_graph_path(data_name);
    GraphYche g(path);
    GraphYche * g_ptr;
    g_ptr = &g;
    for(int i = 0; i < g_ptr->n; i++){
        int out_start = g_ptr->off_out[i]; 
        int out_end = g_ptr->off_out[i+1];
        for(int j = out_start; j < out_end; j++){ // j is not out-neighbor
            auto out_neighbor = g_ptr->neighbors_out[j];
            cout << format("edge: (%s, %s)") % i % out_neighbor << endl;
        }
    }
}

void test_toy(string data_name, int q=1) {
    // some example data
    // string path = TOY_GRAPH;
    string path = TOY_P + data_name;
    GraphYche g(path);
    // cout << format("number of nodes: %s, query pair: %s") % g.n % NodePair{x,y} << endl;
    cout << format("number of nodes: %s") % g.n << endl;

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    // compute ground truth
    TruthSim * truth = new TruthSim("toy_graph", g,c,epsilon);
    truth->run(g);
    // for(int i = 0; i < g.n;i++){
    // 	for(int j = 0; j < g.n;j++){
    // 		cout << format("%s: %s") % NodePair{i,j} % truth->sim(i,j) << endl;
    // 	}
    // }

    BLPMC_Config config1;
    config1.is_use_linear_regression_cost_estimation = true;
    config1.is_use_hub_idx = false;
    config1.is_use_fg_idx = false;
    BackPush bprw1("cloud_walker", g, c, epsilon, delta, config1);
    // size_t n = g.n;
    double maxerr1 = 0;
    int exceederr1 = 0;

    for (int iter = 0; iter < q; ++iter)
    for (int i = 0; i < g.n; ++i){
        for (int j = 0; j < g.n; ++j) {
                    NodePair q{i, j};
            // display_seperate_line();
                    double r = bprw1.query_one2one(q);
                    // cout << format("query_one2one(%s, %s) = %s") % i % j % r << endl;
                    if (abs(r - truth->sim(i, j)) > maxerr1) {
                        maxerr1 = abs(r - truth->sim(i, j));
                    }
            // cout << format("(%s,%s), truth: %s, result: %s, error: %s") % i % j % truth->sim(i,j) % r % abs(r - truth->sim(i, j)) << endl;
            // double mc_result = compute_sim_using_pure_MC(g,i,j,c,9537) ;
            // double mc_error = abs(truth->sim(i,j) - mc_result) ;
            // cout << format("pure MC sampling result: %s, error: %s") % mc_result % mc_error << endl;
                    if (abs(r - truth->sim(i, j)) > epsilon) {
                        ++exceederr1;
                    }
            // if (mc_error > epsilon) ++exceederr1;
            // if (mc_error > maxerr1) maxerr1 = mc_error;
        }
    }
    cout << maxerr1 << " " << exceederr1 << endl;

    display_seperate_line();

    BLPMC_Config config;
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = false;
    config.is_use_fg_idx = true;
    config.number_of_trees = 3443;
    BackPush bprw("cloud_walker", g, c, epsilon, delta, config);
    size_t n = g.n;
    double maxerr = 0;
    int exceederr=0;

    for (int iter = 0; iter < q; ++iter)
    for (int i = 0; i < g.n; ++i){
    	for (int j = 0; j < g.n; ++j) {
		    // if (i != 0 || j != 2) continue;
            NodePair q{i, j};
            // display_seperate_line();
		    double r = bprw.query_one2one(q);
		    // cout << format("query_one2one(%s, %s) = %s") % i % j % r << endl;
		    if (abs(r - truth->sim(i, j)) > maxerr) {
		    	maxerr = abs(r - truth->sim(i, j));
		    }
            // cout << format("(%s,%s), truth: %s, result: %s, error: %s") % i % j % truth->sim(i,j) % r % abs(r - truth->sim(i, j)) << endl;
            // double mc_result = compute_sim_using_pure_MC(g,i,j,c,9537) ;
            // double mc_error = abs(truth->sim(i,j) - mc_result) ;
            // cout << format("pure MC sampling result: %s, error: %s") % mc_result % mc_error << endl;
		    if (abs(r - truth->sim(i, j)) > epsilon) {
		    	++exceederr;
            cout << format("(%s,%s), truth: %s, result: %s, error: %s") % i % j % truth->sim(i,j) % r % abs(r - truth->sim(i, j)) << endl;
		    }
            // if (mc_error > epsilon) ++exceederr1;
            // if (mc_error > maxerr1) maxerr1 = mc_error;
    	}
    }
    cout << maxerr << " " << exceederr << endl;
    return;
}


void demo_for_paper(int x, int y){
    // some example data
    string path = TOY_GRAPH;
    GraphYche g(path);
    cout << format("number of nodes: %s, query pair: %s") % g.n % NodePair{x,y} << endl;

    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;
    BLPMC_Config config;
    config.is_use_linear_regression_cost_estimation = false;
    config.is_use_hub_idx = false;
    BackPush bprw("cloud_walker", g, c, epsilon, delta, config);
    size_t n = g.n;

    NodePair q{x,y};
    double r = bprw.query_one2one(q);
    return;
}

int main(int args, char*argv[]){
    vector<string> datasets{"ca-GrQc", "ca-HepTh", "p2p-Gnutella06", "wiki-Vote", "email-Enron", "email-EuAll", "web-Stanford", \
        "web-BerkStan", "web-Google", "cit-Patents", "soc-LiveJournal1", "WikiLink"};
    try {
        options_description desc{"Options"};
        double c = 0.6;
        double epsilon = 0.01;
        double delta = 0.01;
        int q = 50000; 
        string data_name;
        string method;
        int x, y, h, l, t, k, b;
        desc.add_options()
            ("help,h", "Help Screen")
            ("DataName,d", value<string>(&data_name), "Graph Name")
            ("decay,c", value<double>(&c), "Decay Facrot c")
            ("x,x", value<int>(&x), "x: the query node")
            ("y,y", value<int>(&y), "y: the query node")
            ("q,q", value<int>(&q), "q: the number of queries")
            ("method,m", value<string>(&method), "Method: blp/flp/bflp")
            ("delta,f", value<double>(&delta), "Failure probability")
            ("n,n", value<int>(&h), "Number of hubs")
            ("l,l", value<int>(&l), "Number of random walks per hub")
            ("t,t", value<int>(&t), "Number of trees")
            ("epsilon,e", value<double>(&epsilon), "Error bound")
            ("k,k", value<int>(&k), "Top K")
            ("b,b", value<int>(&b), "debug");

        variables_map vm;
        store(parse_command_line(args, argv, desc), vm);
        notify(vm);

        if (vm.count("help")){
            cout << desc << endl;
        }else if(vm.count("method")){
            if(method == "flp"){
                // test_FLPMC(data_name, c, epsilon, delta, x, y);
            }else if(method == "blp"){
                test_bp(data_name, c, epsilon, delta, x, y);
            }else if (method == "bflp"){
                // test_BFLPMC(data_name, c, epsilon, delta, x,  y);
            }else if (method == "bflpap"){
                // test_bflp_all_pair(data_name);
            }else if (method == "carmo"){
                test_blpmc(data_name, h,l,q);
            }else if (method == "fgi") {
                test_blpmc_fgi(data_name, h, l, t, q);
            }else if (method == "sp") {
                test_blpmc_sp(data_name, h, l, t, q, x, y);
            }else if (method == "topk") {
                test_topk(data_name, q, k, b);
            }
        }else{
            // SFMTRand srand;
            // for(int i = 0; i< 10;i++){
            //     cout << srand.double_rand() << endl;
            // }

            // demo_for_paper(x,y);
            // vector<string> ground_data{"ca-GrQc", "ca-HepTh", "p2p-Gnutella06", "wiki-Vote"};
            // for(auto dataset: ground_data){
            //     cout << format("computing %s...") % dataset << endl;
            //     // sort_graph_node_based_on_utility(dataset);
            //     compute_ground_truth(dataset, 0.6, 0.0000001);
            // }
            // test_bloomfilter();
            // test_bf();
            // test_hash64();
            // test_Distance_1s();
            // test_minimal_perfect_hash();
            // test_bf();
//            test_blpmc("ca-GrQc");
            // sparse_hash_map<NodePair, int, hash_duplicate, hash_duplicate_equal> test;
            // test[NodePair{1,2}] = 1;
            // test[NodePair{2,1}] = 100;

            // cout << test[{1,2}] << endl;
            // cout << test[{2,1}] << endl;
            // test_blpmc("ca-HepTh");
            // test_blpmc("p2p-Gnutella06");
            // test_blpmc("wiki-Vote");
            // test_blpmc("web-Google");
            // test_hub("ca-GrQc");
            // test_lr();
            // string d("ca-GrQc");
            // test_bflp_all_pair(d);
            // test_fgi();
            // test_blpmc_fgi("ca-GrQc");
            test_toy(data_name, q);
        }

    }
    catch (const error &ex ){
        cerr << ex.what() << endl;
    }
    return 0;
}
