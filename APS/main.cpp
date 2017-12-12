#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unordered_set>
#include <boost/program_options.hpp>
#include "sling.h"
#include "dfg.h"
#include "graph.h"
#include "local_push.h"
#include "simrank.h"
#include "tkde17_sim.h"
#include "cloud_walker.h"
#include "linearD.h"
#include "bprw.h"
#include <reads/reads.h>
#include <reads/readsd.h>
#include <reads/readsrq.h>
using namespace std;
using namespace std::chrono;
using namespace boost::program_options;


vector<NodePair> e1 = { // the test graph
    {1, 2}, {2, 4}, {4, 1}, {4, 3}, {3, 1}
};
vector<NodePair> e2 = {
        {6,8},
        {3,1},
        {3,2},
        {3,5},
        {4,6},
        {5,6},
        {3,4},
        {6,7}
        };
vector<NodePair> e3= {
        {0,0},
        {1,2},
        {3,1},
        {1,3},
        {1,4},
        {2,1},
        {4,1}
        };
vector<NodePair> e4 = {
    {1,3}, {1,4}, {1,5}, {2,4}, {2,5}, {2,6} 
};
vector<NodePair> e6 = {
    {0,1}, {0,2}, {0,3}, {1,0}, {1,2}, {2,1}, {2,0}, {3,0}, {3,4}, {4,3}
};
void build_graph(DirectedG &g, vector<NodePair> e){
    DirectedG::vertex_descriptor i;
    for(auto &item:e){
        add_edge(item.first, item.second, g);
    }
    // show_graph(g);
}
void compare_sim(const SimRank_matrix& s1, LocalPush &lp, int n){
    // compare results with ground truth
    double error = 0;
    for(int i = 0; i< n; i++){
        for(int j=i;j<n;j++){
            cout << i << " " << j << ": " << str(format("%.4f") % s1[i][j])
                << "    " <<  format("%.4f")% lp.P[NodePair(i,j)] 
                << "    error: " << str(format("%.4f") % (lp.P[NodePair(i,j)] - s1[i][j]))
                        << endl;
            error +=  abs(lp.P[NodePair(i,j)] - s1[i][j]);
        }
    }
    cout << "total error: " << error << endl;
}
void test_local_push_test_data(vector<NodePair>& e1, bool is_full=false){
    DirectedG g;
    build_graph(g, e1);
    // show_graph(g);
    double C=0.6;
    size_t n = num_vertices(g);
    SimRank_matrix s{boost::extents[n][n]};
    basic_simrank(g,C, s);
    double epsilon = 0.001;
    LocalPush *lp;
    if(is_full){
        lp = new Full_LocalPush(g, "small_test", C, epsilon, n);
    }else{
        lp = new Reduced_LocalPush(g, "small_test", C, epsilon, n);
    }
    // Reduced_LocalPush lp(g, "small_test", C, epsilon, n);
    lp->local_push(g);
    compare_sim(s,*lp, n);
    cout << "size of R: " << lp->R.size() << endl;
    cout << "size of P: " << lp->P.size() << endl;
    delete lp;
}


void test_dynamic_local_push(vector<NodePair> &e){
    DirectedG g;
    std::unordered_set<int> nodes;
    for(auto& edge: e){
        nodes.insert(edge.first);
        nodes.insert(edge.second);
    }
    add_vertex(g);
    for(auto & node:nodes){
        add_vertex(g);
    }
    int n = num_vertices(g);
    double C =0.6;
    cout << "number of vertices of g " << num_vertices(g) << endl;
    double epsilon = 0.00001;
    Reduced_LocalPush lp(g, "small_test", C, cal_rmax(C, epsilon), n);
    lp.local_push(g);
    cout << "before showing lp" << endl;
    // lp.show();
    
    cout << "begin add edges" << endl;
    for(auto& edge:e){
        // cout << "inserting " << edge.first << " "  << edge.second << endl;
        add_edge(edge.first,edge.second,g);
        lp.insert(edge.first,edge.second,g);
    }
    SimRank_matrix s{boost::extents[n][n]};
    basic_simrank(g,C, s);
    compare_sim(s,lp, n);
}

bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full=false){
    LocalPush* lp_test;
    if(is_full){
        lp_test = new Full_LocalPush();
    }else{
        lp_test = new Reduced_LocalPush();
    }
    // Reduced_LocalPush lp_test;
    lp_test->g_name = data_name;
    lp_test->c = c;
    lp_test->epsilon=epsilon;
    string data_path =lp_test->get_file_path_base() + ".P";
    cout << "data path !!!!" << data_path << endl;
    if(file_exists(data_path)){ 
        return true;
    }else{
        return false;
    }

    delete lp_test;
}



void test_localpush(string data_name, double c=0.8, double epsilon=0.001, bool is_full=false){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);

    if(lp_file_exists(data_name,c,epsilon,n, is_full)){ 
        cout << "already computed..." << endl;
        return ;
    }

    LocalPush * lp;
    if(is_full){
        lp = new Full_LocalPush(g, data_name, c, epsilon, n);
    }else{
        lp = new Reduced_LocalPush(g, data_name, c, epsilon, n);
        // Reduced_LocalPush lp(g, data_name, c, epsilon, n);
    }
    // compute from 0
    // Full_LocalPush lp(g, data_name, C, r_max);
    lp->local_push(g);
    // lp.save();
    cout << "size of R: " << lp->R.size() << endl;
    cout << "size of P: " << lp->P.size() << endl;
    cout << "writing to disk.." << endl;
    lp->save();
    delete lp;
}

void IO_test(string data_name){
    DirectedG g;
    double epsilon = 0.001;
    double C = 0.6;
    double r_max = cal_rmax(C, epsilon);
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    Reduced_LocalPush lp(g, data_name, C, r_max, n);
    // Full_LocalPush lp(g, data_name, C, r_max);
    lp.local_push(g);
    lp.save();

    Reduced_LocalPush lp2(g, data_name, C, r_max, n);
    cout << "estimate of 0 0:  "  << lp2.P[NodePair(0,0)] << endl;
    // cout << "P size "<<lp2.P.size() << endl;
}

void test_hashIO(){
    // test case for serialize a set of objects
    DensePairMap<int> np, np2;
    string f_name("./test.dat"); 
    np.add(10);
    np[NodePair(1,9)] = 10;
    np[NodePair(7,8)] = 4;
    np.save(f_name);

    np2.load(f_name);
    cout << np2.size() << endl;

}

/* the experiment codes */
void exp_dynamic(string data_name, double c, double epsilon, int num_updates=100){
    // exp codes for dynamic graph, score for old graph must be already computed
    // load the graph
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    if(!lp_file_exists(data_name, c, epsilon, n)){
        cout << data_name << ": the lp file has not been computed!!!" << endl;
        return;
    }
    Reduced_LocalPush rlp(g,data_name,c,epsilon,n); // already load the local push data

    // generate edges
    vector<pair<unsigned int, unsigned int>> ins_edges;
    /* initialize random seed: */
    srand (time(NULL));
    while(ins_edges.size()<num_updates){
        auto e1 = rand() % n ;
        auto e2 = rand() % n ;
        if(boost::edge(e1,e2,g).second == false){
            ins_edges.push_back({e1,e2});
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    // do the test
    for(auto &edge:ins_edges){
        // add to g
        add_edge(edge.first, edge.second,g);
        // update P and R
        rlp.insert(edge.first, edge.second,g);
    }
    

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    cout << "data: " << data_name << endl;
    cout << "number of updates: " << num_updates << endl;
    cout << "total time: " << elapsed.count() << endl;
    cout << "avg time: " << elapsed.count() / num_updates << endl;
    cout << "mem usage: " << getValue() << endl;

    return;
}
void compute_ground_truth(string data_name, double c = 0.6, double epsilon = 0.00001){
    DirectedG g;
    load_graph(get_edge_list_path(data_name),g);
    TruthSim ts(data_name, g, c, epsilon);
    size_t n = num_vertices(g);
    cout << "computing " << data_name << " " << c << " " << epsilon << endl;
    ts.run(g);
    ts.save();
    if(n < 100){
        cout << "ground truth for: " << data_name << endl;
        cout << ts.sim << endl;
    }
}
void test_tkde17(string data_name, float c = 0.6, float epsilon = 0.001){ 
    // run tkde 17 experiments
    cout << "computing " << data_name << " " << c << epsilon << endl;
    DirectedG g;
    load_graph(get_edge_list_path(data_name),g);
    LinearSystemSim lys(data_name, g, c, epsilon);
    // cout << "--------------" << endl;
    // size_t n = num_vertices(g);
    // SimRank_matrix s{boost::extents[n][n]};
    // basic_simrank(g,c, s);
    // for(size_t i = 0;i<n;i++){
    //     for(size_t j = 0; j<n;j++){
    //         cout << s[i][j] << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "--------------" << endl;
    // TruthSim ts("test_g", g, c, 0.00001);
    // ts.run(g);
    // ts.save();
    // cout << ts.sim << endl;

    cout << "==============" << endl;
    return;
}
void generate_effective_error_rlp(string data_name){
    // generate rlp effective
    string ERROR_DIR("/homes/ywangby/workspace/dynamicSim/datasets/effective_error/");
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    double c = 0.6;
    size_t n = num_vertices(g);
    double epsilon = 0.001;
    ofstream out(ERROR_DIR+data_name+string(".rlp.txt"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    while(epsilon < 0.011){
        Reduced_LocalPush rlp(g, data_name, c, epsilon, n);
        // make a matrix
        MatrixXf lp_result(n,n);
        lp_result.setZero();
        for(size_t i = 0;i < n;i++){
            for(auto & item: rlp.P.v[i]){
                lp_result(i,item.first) = item.second; // fill up the matrix
                lp_result(item.first, i) = item.second; // fill up the matrix
            }
        }
        cout << lp_result(2,2) << " " << ts.sim(2,2) << endl;
        double max_error = (lp_result - ts.sim).cwiseAbs().maxCoeff();
        double mean_error = (lp_result - ts.sim).cwiseAbs().mean();
        out << max_error << " " << mean_error << endl;
        epsilon += 0.001;
    }
    out.close();
}
void generate_effective_error_flp(string data_name){
    // generate rlp effective
    string ERROR_DIR("/homes/ywangby/workspace/dynamicSim/datasets/effective_error/");
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    double c = 0.6;
    size_t n = num_vertices(g);
    double epsilon = 0.001;
    ofstream out(ERROR_DIR+data_name+string(".flp.txt"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    while(epsilon < 0.011){
        Full_LocalPush rlp(g, data_name, c, epsilon, n);
        // make a matrix
        MatrixXf lp_result(n,n);
        lp_result.setZero();
        for(size_t i = 0;i < n;i++){
            for(auto & item: rlp.P.v[i]){
                lp_result(i,item.first) = item.second; // fill up the matrix
                lp_result(item.first, i) = item.second; // fill up the matrix
            }
        }
        cout << lp_result(2,2) << " " << ts.sim(2,2) << endl;
        double max_error = (lp_result - ts.sim).cwiseAbs().maxCoeff();
        double mean_error = (lp_result - ts.sim).cwiseAbs().mean();
        out << max_error << " " << mean_error << endl;
        epsilon += 0.001;
    }
    out.close();
}

void compute_tkde17_error(string data_name){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    double c = 0.6;
    double epsilon = 0.01;
    size_t n = num_vertices(g);
    ofstream out(TKDE17_RESULT_DIR+data_name + string(".error"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    LinearSystemSim tkde17(data_name, g, c, epsilon);
    double max_error = (ts.sim-tkde17.sim).cwiseAbs().maxCoeff();
    double mean_error = (ts.sim-tkde17.sim).cwiseAbs().mean();
    out << max_error << " " << mean_error << endl;
    out.close();
}

void compute_cloudwalker_error(string data_name){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    cout << "load graph complete." << endl;
    double c = 0.6;
    int T = 10;
    int R = 100;
    int R_prime = 10000;
    int L = 3;
    double epsilon = 0.01;
    auto n = num_vertices(g);
    CloudWalker cw(&g, data_name, c, T,L,R,R_prime);
    cw.load();
    auto sim = cw.sim.cast<float>();
    ofstream out(CLOUD_WALKER_DIR + data_name + string(".error"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    double mean_error = (ts.sim-sim).cwiseAbs().mean();

    double max_error = (ts.sim-sim).cwiseAbs().maxCoeff();

    out << max_error << " " << mean_error << endl;
    out.close();
}

void compute_linearD_error(string data_name){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    cout << "load graph complete." << endl;
    double c = 0.6;
    int T = 10;
    int R = 100;
    int R_prime = 10000;
    int L = 3;
    double epsilon = 0.01;
    auto n = num_vertices(g);
    LinearD lin(&g, data_name, c, T,L,R);
    lin.load();
    auto sim = lin.sim.cast<float>();
    ofstream out(LINEAR_D_DIR + data_name + string(".error"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    double mean_error = (ts.sim-sim).cwiseAbs().mean();

    double max_error = (ts.sim-sim).cwiseAbs().maxCoeff();

    out << max_error << " " << mean_error << endl;
    out.close();
}


void test_cloud_walker(string data_name, double c=0.6, int T=10, int L=3, int R=100, int R_prime=10000){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    srand (time(NULL)); // random number generator
    std::chrono::duration<double> elapsed;
    int sample_size = 1000;

    auto start = std::chrono::high_resolution_clock::now();
    CloudWalker cw(&g, data_name, c, T,L,R,R_prime);
    auto pre_time = std::chrono::high_resolution_clock::now();
    size_t n = num_vertices(g);
    ofstream out(CLOUD_WALKER_DIR+cw.g_name+string(".meta")); // the file to store the execute info
    elapsed =pre_time-start ;
    out << elapsed.count() << endl; // record the pre-processing time
    out << n << endl; // number of vertices
    for(int i = 0; i< sample_size;i++){
        if(i % 10 == 0){
            cout << i << " th sample of " << cw.g_name << endl;
        }
        int source_node = rand() % n;
        VectorXd tmp(n);
        auto start_ss = std::chrono::high_resolution_clock::now();
        cw.mcss(source_node,tmp);
        auto end_ss = std::chrono::high_resolution_clock::now();
        elapsed = end_ss - start_ss;
        auto mem_size = cw.mem_size; // record the memeory size
        auto cpu_time = elapsed.count();
        out << cpu_time << " " << mem_size << endl;
    }
    out.close();
    
    // cout << "D: " << endl;
    // cout << cw.D << endl;

    // cw.mcap();
    // cout << cw.get_file_path_base() << endl;
    // cout << cw.sim << endl;
    // cw.save();
    // cout << "return ... " << endl;
    return;
}

void test_linearD(string data_name, double c, int T, int L, int R){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    srand (time(NULL)); // random number generator
    std::chrono::duration<double> elapsed;
    int sample_size = 1000;

    auto start = std::chrono::high_resolution_clock::now();
    LinearD lin(&g, data_name, c, T,L,R);
    auto pre_time = std::chrono::high_resolution_clock::now();
    size_t n = num_vertices(g);
    ofstream out(LINEAR_D_DIR+lin.g_name+string(".meta")); // the file to store the execute info
    elapsed =pre_time-start ;
    out << elapsed.count() << endl; // record the pre-processing time
    out << n << endl; // number of vertices
    for(int i = 0; i< sample_size;i++){
        if(i % 10 == 0){
            cout << i << " th sample for" <<  lin.g_name << endl;
        }
        int source_node = rand() % n;
        VectorXd tmp(n);
        auto start_ss = std::chrono::high_resolution_clock::now();
        lin.single_source(source_node,tmp);
        auto end_ss = std::chrono::high_resolution_clock::now();
        elapsed = end_ss - start_ss;
        auto mem_size = lin.mem_size; // record the memeory size
        auto cpu_time = elapsed.count();
        out << cpu_time << " " << mem_size << endl;
    }
    out.close();

    // lin.mcap();
    // cout << lin.get_file_path_base() << endl;
    // cout << lin.sim << endl;
    // lin.save();
    cout << "return ... " << endl;
    return;
}

void test_lineD_all_pair(string data_name, double c, int T, int L, int R){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    LinearD lin(&g, data_name, c, T,L,R);
    lin.all_pair();
    cout << lin.get_file_path_base() << endl;
    lin.save();
    return;
}

void test_readsd(string data_name, double c, double epsilon, int r, int t){
    string data_path = get_edge_list_path(data_name);
    DirectedG g;
    load_graph(data_path, g);
    int n = num_vertices(g);

	double * ansVal = new double[n];

	// reads i1(argv[1], n, r, c, t);	
	// //querying one-to-all simrank value from node o
	// i1.queryAll(0, ansVal);
    auto start = std::chrono::high_resolution_clock::now();

	readsd i2((char *)data_path.c_str(), n, r, c, t);	
	//querying one-to-all simrank value from node o
	i2.queryAll(1, ansVal);

	// readsrq i3(argv[1], n, r, rq, c, t);	
	// //querying one-to-all simrank value from node o
	// i3.queryAll(0, ansVal);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    elapsed =end-start ;
    cout << format("total cost: %s s") % elapsed.count() << endl; 


	delete [] ansVal;
}

void test_readsrq(string data_name, double c, double epsilon, int r, int rq, int t){
    string data_path = get_edge_list_path(data_name);
    DirectedG g;
    load_graph(data_path, g);
    int n = num_vertices(g);

	double * ansVal = new double[n];

	// reads i1(argv[1], n, r, c, t);	
	// //querying one-to-all simrank value from node o
	// i1.queryAll(0, ansVal);

	readsrq i((char *)data_path.c_str(), n, r,rq, c, t);	
	//querying one-to-all simrank value from node o
    auto start = std::chrono::high_resolution_clock::now();
	i.queryAll(1, ansVal);

	// readsrq i3(argv[1], n, r, rq, c, t);	
	// //querying one-to-all simrank value from node o
	// i3.queryAll(0, ansVal);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    elapsed =end-start ;
    cout << format("total cost: %s s") % elapsed.count() << endl; 


	delete [] ansVal;
}

void test_bp(string data_name, double c, double epsilon, double delta, int x, int y){
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    DirectedG g;
    load_graph(get_edge_list_path(data_name),g);
    BackPush bprw(data_name, g, c, epsilon, delta);
    size_t n = num_vertices(g);
    NodePair q{x,y};
    // for(int i = 0; i < 100;i++){
    auto start = std::chrono::high_resolution_clock::now();

    double result = bprw.query_one2one(q);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    elapsed =end-start ;

    cout << q << ": " << result << endl;


    TruthSim ts(data_name, g, c, epsilon);
    cout  << format("sim: %s:%s") % q % result << endl;
    cout << format("ground truth: %s") % ts.sim(q.first,q.second) << endl;
    cout << format("error: %s") % (ts.sim(q.first,q.second) - result) << endl;
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time
    
}

void test_sling(string data_name, double c, double epsilon, int x, int y){
    cout << "testing sling" << endl;
    // load graph
    
    cout << format("%s %s %s") % data_name % c % epsilon << endl;
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    Graph sling_g(g); // the sling graph

    NodePair q{x,y};

    // print the graph
    // cout << sling_g << endl;

    Sling sling(&sling_g, c);
    // build the index
    sling.calcD(0.005);
    sling.backward(0.000725);
    // compute single-pair query
    double result;

    auto start = std::chrono::high_resolution_clock::now();
    result = sling.simrank(q.first,q.second);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed;
    elapsed =end-start ;

    cout << result << endl;

    // ground truth
    TruthSim ts(data_name, g, c, epsilon);
    cout  << format("sim: %s:%s") % q % result << endl;
    cout << format("ground truth: %s") % ts.sim(q.first,q.second) << endl;
    cout << format("error: %s") % (ts.sim(q.first,q.second) - result) << endl;
    cout << format("total query cost: %s s") % elapsed.count() << endl; // record the pre-processing time

}


int main(int args, char * argv[]){
    try{
        options_description desc{"Options"};
        string algo_name;
        string data_name;
        double c=0.6;
        double epsilon=0.01;
        double delta = 0.01;
        int number_of_updates=100; // number of edge updates for dynamic graphs
        int r = 100; // number of sample
        int rq = 10; // number of online reverse random walk
        int t = 10; // the length of random walks
        int L = 3; // the parameter for D computation
        int T = 10;
        int R = 100;
        int R_prime = 10000;
        int x,y;
        desc.add_options()
            // notice no space in the first parameter
            ("help,h", "Help Screen")
            ("DataName,d", value<string>(&data_name), "Graph Name")
            ("IterationD,l", value<int>(&L), "number of iterations to compute D")
            ("algorithm,a", value<string>(&algo_name), "Algorithm Name")
            ("decay,c", value<double>(&c), "Decay Facrot c")
            ("epsilon,e", value<double>(&epsilon), "Error bound")
            ("NumUpdates,n", value<int>(&number_of_updates), "Number of updates (used in dynamic graphs)")
            ("R,R", value<int>(&R), "number of random walk to estimates D/online random walks for READS")
            ("R_prime,r", value<int>(&R_prime), "number of random walk for MCSS/READS")
            ("T,t", value<int>(&T), "T: the number of iteration of linearization technique/length of random walks")
            ("x,x", value<int>(&x), "x: the query node")
            ("y,y", value<int>(&y), "y: the query node")
            ;

        variables_map vm;
        store(parse_command_line(args, argv, desc), vm);
        notify(vm);

        if (vm.count("help")){
            cout << desc << endl;
        }else if(vm.count("algorithm")){
            if(algo_name == "full"){
                test_localpush(data_name, c, epsilon, true);
            }else if(algo_name == "static"){
                test_localpush(data_name, c, epsilon);
            }else if(algo_name == "truth"){
                compute_ground_truth(data_name, c); // ignore the other parameters for ground truth
            }else if(algo_name == "tkde17"){
                test_tkde17(data_name, c, epsilon);
            }else if(algo_name == "dynamic"){
                exp_dynamic(data_name, c, epsilon, number_of_updates);
            }else if(algo_name == "cloudwalker"){
                test_cloud_walker(data_name, c, T, L, R, R_prime);
            }else if(algo_name == "lin"){
                test_linearD(data_name, c, T, L, R);
            }else if(algo_name == "linAll"){ // Linearized all-pair
                test_lineD_all_pair(data_name, c, T, L, R);
            }else if(algo_name == "readsd"){
                test_readsd(data_name,c,epsilon,R_prime,t);
            }else if(algo_name == "readsrq"){
                test_readsrq(data_name,c,epsilon,R_prime,R,t);
            }else if(algo_name == "bp"){ // the backward push algorithm
                test_bp(data_name, c, epsilon, delta, x, y);
            }else if(algo_name == "sling"){
                test_sling(data_name, c, epsilon, x,y);
            }
        }else{
            // default behavior
            // vector<string> datasets{"ca-GrQc","ca-HepTh","p2p-Gnutella06","wiki-Vote"};
            // for(auto data_name:datasets){
            //     // test_lineD_all_pair(data_name, c, T, L, R);
            //     compute_linearD_error(data_name);
            // }
            //
            // test sling
            
        }
    }
    catch(const error &ex){
        cerr << ex.what() << endl;
    }

    // /* old style parsing */
    // vector<NodePair> e5;
    // int n = 5;
    // for(int i = 1; i< n; i++){
    //     for(int j = 1; j< n;j++){
    //         if(i !=j ){ // no self loop
    //             e5.push_back(NodePair(i,j));
    //         }
    //     }
    // }
    // if(args >=5 ){
    //     string data_name = string(argv[2]);
    //     double c = atof(argv[3]);
    //     double epsilon = atof(argv[4]);
    //     if(string(argv[1]) == "static"){
    //         test_localpush(data_name, c, epsilon);
    //     }else if(string(argv[1]) == "dynamic"){
    //         if(args == 5){
    //             exp_dynamic(data_name, c, epsilon);
    //         }else if(args == 6){
    //             auto number_of_updates = atoi(argv[5]);
    //             exp_dynamic(data_name, c, epsilon, number_of_updates);
    //         }
    //     }else if(string(argv[1]) == "full"){
    //         test_localpush(data_name, c, epsilon, true);
    //     }else if(string(argv[1]) == "truth"){
    //         compute_ground_truth(data_name); // ignore the other parameters for ground truth
    //     }else if(string(argv[1]) == "tkde17"){
    //         test_tkde17(data_name, c, epsilon);
    //     }
    // }else if(args == 2){
    //     if(string(argv[1]) == string("test")){
    //         cout << "testing full local push " << endl;
    //         test_local_push_test_data(e1, true);
    //         test_local_push_test_data(e2, true);
    //         test_local_push_test_data(e3, true);
    //         test_local_push_test_data(e4, true);
    //         // test Dense hash map
    //         // try clique
    //         test_local_push_test_data(e5, true);

    //         cout << "testing reduced local push" << endl;
    //         test_dynamic_local_push(e1);
    //         test_dynamic_local_push(e2);
    //         test_dynamic_local_push(e3);
    //         test_dynamic_local_push(e4);
    //         // // test Dense hash map
    //         // // try clique

    //         test_dynamic_local_push(e5);
    //     }
    // }else{
    //     vector<string> datasets{"ca-GrQc","ca-HepTh","p2p-Gnutella06","wiki-Vote"};
    //     for(auto s:datasets){
    //         // compute_tkde17_error(s);
    //         generate_effective_error_rlp(s);
    //         generate_effective_error_flp(s);
    //     }
    //     // test tkde 17
    //     // vector<vector<NodePair>> egs{e1,e2,e3,e4,e5, e6};
    //     // vector<vector<NodePair>> egs{e1};
    //     // for(auto& e: egs){
    //     //     DirectedG g;
    //     //     build_graph(g,e);
    //     //     test_tkde17(g);
    //     // }
    //     return 0;
    // }
    // //     }else if(string(argv[1]) == string("dynamic")){
    // //         test_dynamic_local_push(e1);
    // //         test_dynamic_local_push(e2);
    // //         test_dynamic_local_push(e3);
    // //         test_dynamic_local_push(e4);
    // //         // // test Dense hash map
    // //         // // try clique

    // //         test_dynamic_local_push(e5);
    // //     }else if(string(argv[1]) == string("io")){
    // //         IO_test("ca-GrQc");
    // //     }else{
    // //         test_localpush(string(argv[1]));
    // //     }
    // // }else if(args == 3 ){
    // //     string data_name = string(argv[1]);
    // //     double c = atof(argv[2]);
    // //     test_localpush(data_name, c);
    // // }else if(args == 4){
    // //     string data_name = string(argv[1]);
    // //     double c = atof(argv[2]);
    // //     double epsilon = atof(argv[3]);
    // //     test_localpush(data_name, c, epsilon);
    // // }else{
    // //     // IO_test("ca-GrQc");
    // //     cout << getValue() << endl;
    // // }
    // // DirectedG  g;
    // // cout << "loading graph" << endl;
    // // load_graph(get_edge_list_path("soc-LiveJournal1"), g);
    // // string fp("dfg_test.txt");
    // // cout << "saving...." << endl;
    // // dfg.save(fp);
    // // DFG_Index test_dfg;
    // // cout << "loading..." << endl;
    // // test_dfg.load(fp);
    // // cout << test_dfg.n << endl;
    // // Owg o;
    // // load_sparse_map(o, string("one-way-livej"));
    // // cout << o.size() << endl;
}
