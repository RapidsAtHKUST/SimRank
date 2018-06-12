#include <cstdlib>
#include <ctime>

#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "stat.h"
#include "bprw.h"
// #include "flpmc.h" 
// #include "bflpmc.h"
#include "simrank.h"
#include <util/graph_yche.h>
#include "rw_hub.h"
#include "phf.h"
#include "minimal_perfect_hash.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;

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

void test_blpmc(string data_name){
    string path = get_edge_list_path(data_name);
    GraphYche g(path);

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
    for(int i = 0; i < 50000;i++){
        int a = random_int(0,n);
        int b = random_int(0,n);
        queries.push_back(NodePair(a,b));
    }

    display_seperate_line();

    TruthSim ts(data_name, g, c, epsilon);
    double max_error = 0;

    auto start = std::chrono::high_resolution_clock::now();
    cout << bprw.config << endl;
    int i = 0;
    for(auto& q: queries){
        // cout << format("querying %s-th node pair.") % i << endl;
        double result = bprw.query_one2one(q);
        double truth = ts.sim(q.first,q.second);
        double abs_error = abs(truth - result);
        // cout << format("pair:%s, sim: %s, truth: %s, error:%s") % q % result % truth % abs_error << endl;
        if(abs_error > max_error){
            max_error = abs_error;
        }
        i ++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s") % elapsed.count() % max_error << endl;
    display_seperate_line();

    /* test the method for cost estimation */
    int nbh = 1000000;
    // for(int nbh = 1000000; nbh < 10000000; nbh = nbh + 1000000){ // varying number of hubs
    display_seperate_line();
    config.is_use_linear_regression_cost_estimation = true;
    config.is_use_hub_idx = true;
    config.number_of_hubs = nbh;
    config.number_of_samples_per_hub = 500;
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
        double truth = ts.sim(q.first,q.second);
        double abs_error = abs(truth - result);
        if(abs_error > max_error){
            max_error = abs_error;
            // cout << format("error: %s, np: %s, query result: %s, ground_truth: %s") % abs_error % q % 
        }
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << format("Total cost: %s, Maximum error: %s, #hubs: %s, #hits of hub: %s, #contain queries:%s") \
        % elapsed.count() % max_error % config.number_of_hubs  % bprw2.hub_hits % \
        bprw2.rw_hubs->number_of_contains_queries<< endl;
    display_seperate_line();
    // }
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

int test_bf(){
	//tells bbhash to use included hash function working on u_int64_t input keys:
    SFMTRand rand_gen;
    int length = 1000000;
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
	// boophf_t * bphf = new boomphf::mphf<u_int64_t,hasher_t>(input_keys.size(),input_keys,8);
	 
	 //query the mphf :

    // int number_of_unique_keys =  PHF::uniq(&input_keys[0], input_keys.size());
    // cout << format("number of unique keys: %s") % number_of_unique_keys << endl;
    // phf f;
    minimal_perfect_hash::MinimalPerfectHash<unsigned int> mph;
    int r = mph.Build(input_keys);
    assert(r == 0);
    // PHF::init<unsigned int, false>(&f, &input_keys[0], number_of_unique_keys, 0, 100, (unsigned int)rand_gen.double_rand() * 1000000);
    // for(int i = 0; i< length;i++){
    //     unsigned int hased_key = PHF::hash(&f, input_keys[i]) ;
    //     cout << format("key: %s, hashed key: %s") % input_keys[i] % hased_key << endl;
    //     // unsigned int hased_key = bphf->lookup(input_keys[i]);
    //     // cout << format("input key: %s, hashed key: %s") % input_keys[i] % bphf->lookup(input_keys[i]) << endl;
    //     s.insert(hased_key);
    // }
    std::unordered_set<unsigned int> hased_keys;
    for(auto & key: input_keys){
        // auto hashed_key = bphf->lookup(key);
        auto hashed_key = mph.GetHash(key);
        hased_keys.insert(hashed_key);
        cout << format("key: %s, hased key: %s") % key % hashed_key << endl;
    }
    cout << format("number of unique keys: %s") % s.size() << endl;
    cout << format("number of unique hased keys: %s") % hased_keys.size() << endl;

    // cout << "random keys" << endl;
    // for(int i = 0; i< 1000;i++){
    //     unsigned int rand_key = int(rand_gen.double_rand() * MAX_VALUE);
    //     cout << format("key: %s, hashed key: %s") % rand_key % PHF::hash(&f, rand_key) << endl;
    // }
}

void test_minimal_perfect_hash(){
    // Prepare the keys
    std::vector<int> t;
    t.push_back(1);
    t.push_back(11);
    t.push_back(111);
    t.push_back(1111);
    t.push_back(11111);
    t.push_back(111111);

    // Build
    minimal_perfect_hash::MinimalPerfectHash<int> mph;
    int r = mph.Build(t);
    assert(r == 0);

    // GetHash
    for (size_t i = 0; i < t.size(); ++i) {
      std::cout << t[i] << "\t" << mph.GetHash(t[i]) << std::endl;
    }
    std::cout << std::endl;
}

void test_Distance_1s(){
    int max_length = 0;
    vector<size_t> positions;
    Distanct_1s d(positions, max_length);

    for(auto & item: d.next){
        cout << format("%s, ") % item ;
    }
    cout << endl;

    for(int i = 0 ; i < 100; i++){
        int j = d.get();
        // cout << format(" distance to next 1: %s, current i: %s, current return: %s ") % d.energy % d.i % j << endl;
        cout << format("%s ") % j;
    }
    cout << endl;
}

int main(int args, char*argv[]){
    try {
        options_description desc{"Options"};
        double c = 0.6;
        double epsilon = 0.01;
        double delta = 0.01;
        string data_name;
        string method;
        int x, y;
        desc.add_options()
            ("help,h", "Help Screen")
            ("DataName,d", value<string>(&data_name), "Graph Name")
            ("decay,c", value<double>(&c), "Decay Facrot c")
            ("x,x", value<int>(&x), "x: the query node")
            ("y,y", value<int>(&y), "y: the query node")
            ("method,m", value<string>(&method), "Method: blp/flp/bflp")
            ("delta,f", value<double>(&delta), "Failure probability")
            ("epsilon,e", value<double>(&epsilon), "Error bound");

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
                test_blpmc(data_name);
            }
        }else{
            test_Distance_1s();
            // test_minimal_perfect_hash();
            // test_bf();
            // test_blpmc("ca-GrQc");
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
        }

    }
    catch (const error &ex ){
        cerr << ex.what() << endl;
    }
    return 0;
}
