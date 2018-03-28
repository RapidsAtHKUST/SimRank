#include <cstdlib>
#include <ctime>

#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/random/linear_congruential.hpp>

#include "graph.h"
#include "local_push.h"
#include "util/simrank.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;


vector<NodePair> e1 = { // the test graph
        {1, 2},
        {2, 4},
        {4, 1},
        {4, 3},
        {3, 1}
};

vector<NodePair> e2 = {
        {6, 8},
        {3, 1},
        {3, 2},
        {3, 5},
        {4, 6},
        {5, 6},
        {3, 4},
        {6, 7}
};
vector<NodePair> e3 = {
        {0, 0},
        {1, 2},
        {3, 1},
        {1, 3},
        {1, 4},
        {2, 1},
        {4, 1}
};
vector<NodePair> e4 = {
        {1, 3},
        {1, 4},
        {1, 5},
        {2, 4},
        {2, 5},
        {2, 6}
};
vector<NodePair> e5 = {
        {0, 1},
        {0, 2},
        {1, 0},
        {1, 2},
        {2, 1},
        {2, 0}
};
vector<NodePair> e6 = {
        {0, 1},
        {0, 2},
        {0, 3},
        {1, 0},
        {1, 2},
        {2, 1},
        {2, 0},
        {3, 0},
        {3, 4},
        {4, 3}
};
vector<NodePair> e7 = {
        {1, 4},
        {2, 1},
        {3, 1},
        {3, 2},
        {4, 3}
};
int FULL_LOCAL_PUSH_TYPE = 0;
int REDUCED_LOCAL_PUSH_TYPE = 1;
int PARALLEL_LOCAL_PUSH_TYPE = 2;

void build_graph(DirectedG &g, vector<NodePair> e) {
    DirectedG::vertex_descriptor i;
    for (auto &item:e) {
        add_edge(item.first, item.second, g);
    }
    // show_graph(g);
}

void compare_sim(const SimRank_matrix &s1, LocalPush &lp, int n) {
    // compare results with ground truth
    // maximum error
    double error = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            // cout << i << " " << j << ": " << str(format("%.4f") % s1[i][j])
            //     << "    " <<  format("%.4f")% lp.query_P(i,j)
            //     << "    error: " << str(format("%.4f") % (lp.query_P(i,j) - s1[i][j]))
            //             << endl;
            error = max(abs(lp.query_P(i, j) - s1[i][j]), error);
        }
    }
    cout << "maximum error: " << error << endl;
}

void test_local_push_test_data(vector<NodePair> &e1, int local_push_type = 0) {
    // local_push_type: the type of local push: 0:full local push
    DirectedG g;
    build_graph(g, e1);
    // show_graph(g);
    double C = 0.6;
    size_t n = num_vertices(g);
    SimRank_matrix s{boost::extents[n][n]};
    basic_simrank(g, C, s);
    double epsilon = 0.001;
    LocalPush *lp;
    if (local_push_type == FULL_LOCAL_PUSH_TYPE) {
        lp = new Full_LocalPush(g, "small_test", C, epsilon, n);
    } else if (local_push_type == REDUCED_LOCAL_PUSH_TYPE) {
        lp = new Reduced_LocalPush(g, "small_test", C, epsilon, n);
    } else if (local_push_type == PARALLEL_LOCAL_PUSH_TYPE) {
        lp = new Parallel_LocalPush(g, "small_test", C, epsilon, n);
        return;
    }
    // Reduced_LocalPush lp(g, "small_test", C, epsilon, n);
    lp->local_push(g);
    compare_sim(s, *lp, n);
    // cout << "size of R: " << lp->R.size() << endl;
    // cout << "size of P: " << lp->P.size() << endl;
    delete lp;
}

void display_F(vector<NodePair> &e1) {
    // display the entries in the matrix F
    DirectedG g;
    build_graph(g, e1);
    DirectedG::vertex_iterator v_begin, v_end, v_it, u_it;
    tie(v_begin, v_end) = vertices(g);
    int count = 0;
    for (v_it = v_begin; v_it != v_end; v_it++) {
        for (u_it = v_begin; u_it != v_end; u_it++) {
            int in_v = in_degree((*v_it), g);
            int in_u = in_degree(*u_it, g);
            if (in_v * in_u > 0 && (*v_it != *u_it)) {
                // cout << format("(%s,%s)[%s]: ") % *v_it % *u_it % (in_u * in_v);
                DirectedG::in_edge_iterator v_in_it, v_in_begin, v_in_end;
                DirectedG::in_edge_iterator u_in_it, u_in_begin, u_in_end;
                tie(v_in_begin, v_in_end) = in_edges(*v_it, g);
                tie(u_in_begin, u_in_end) = in_edges(*u_it, g);
                for (v_in_it = v_in_begin; v_in_it != v_in_end; v_in_it++) {
                    for (u_in_it = u_in_begin; u_in_it != u_in_end; u_in_it++) {
                        auto a = source(*v_in_it, g);
                        auto b = source(*u_in_it, g);
                        cout << format("(%s,%s), (%s,%s): %s ") % *v_it % *u_it % a % b % (in_v * in_u) << endl;
                        count++;
                    }
                }
            }
        }
    }
    cout << count << endl;
}

void test_dynamic_local_push(vector<NodePair> &e) {
    DirectedG g;
    std::unordered_set<int> nodes;
    for (auto &edge: e) {
        nodes.insert(edge.first);
        nodes.insert(edge.second);
    }
    add_vertex(g);
    for (auto &node:nodes) {
        add_vertex(g);
    }
    int n = num_vertices(g);
    double C = 0.6;
    cout << "number of vertices of g " << num_vertices(g) << endl;
    double epsilon = 0.00001;
    Reduced_LocalPush lp(g, "small_test", C, cal_rmax(C, epsilon), n);
    lp.local_push(g);
    cout << "before showing lp" << endl;
    // lp.show();

    cout << "begin add edges" << endl;
    for (auto &edge:e) {
        // cout << "inserting " << edge.first << " "  << edge.second << endl;
        add_edge(edge.first, edge.second, g);
        lp.insert(edge.first, edge.second, g);
    }
    SimRank_matrix s{boost::extents[n][n]};
    basic_simrank(g, C, s);
    compare_sim(s, lp, n);
}

bool lp_file_exists(string data_name, double c, double epsilon, size_t n, int local_push_type = 0) {
    LocalPush *lp_test;
    if (local_push_type == FULL_LOCAL_PUSH_TYPE) {
        lp_test = new Full_LocalPush();
    } else if (local_push_type == REDUCED_LOCAL_PUSH_TYPE) {
        lp_test = new Reduced_LocalPush();
    } else if (local_push_type == PARALLEL_LOCAL_PUSH_TYPE) {
        return false;
        // lp_test = new Reduced_LocalPush();
    }
    // Reduced_LocalPush lp_test;
    lp_test->g_name = data_name;
    lp_test->c = c;
    lp_test->epsilon = epsilon;
    string data_path = lp_test->get_file_path_base() + ".P";
    cout << "data path !!!!" << data_path << endl;
    if (file_exists(data_path)) {
        return true;
    } else {
        return false;
    }

    delete lp_test;
}

void test_localpush(string data_name, double c = 0.8, double epsilon = 0.001, int local_push_type = 0) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);

    if (lp_file_exists(data_name, c, epsilon, n, local_push_type)) {
        cout << "already computed..." << endl;
        return;
    }


    LocalPush *lp;
    if (local_push_type == FULL_LOCAL_PUSH_TYPE) {
        lp = new Full_LocalPush(g, data_name, c, epsilon, n);
    } else if (local_push_type == REDUCED_LOCAL_PUSH_TYPE) {
        lp = new Reduced_LocalPush(g, data_name, c, epsilon, n);
        // Reduced_LocalPush lp(g, data_name, c, epsilon, n);
    } else if (local_push_type == PARALLEL_LOCAL_PUSH_TYPE) {
        lp = new Parallel_LocalPush(g, data_name, c, epsilon, n);
    }
    // compute from 0
    // Full_LocalPush lp(g, data_name, C, r_max);
    lp->local_push(g);
    // lp.save();
    cout << "size of R: " << lp->R.size() << endl;
    cout << "size of P: " << lp->P.size() << endl;
    // cout << "writing to disk.." << endl;
    // lp->save();
    delete lp;
}

void IO_test(string data_name) {
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
    cout << "estimate of 0 0:  " << lp2.P[NodePair(0, 0)] << endl;
    // cout << "P size "<<lp2.P.size() << endl;
}

void test_hashIO() {
    // test case for serialize a set of objects
    DensePairMap<int> np, np2;
    string f_name("./test.dat");
    np.add(10);
    np[NodePair(1, 9)] = 10;
    np[NodePair(7, 8)] = 4;
    np.save(f_name);

    np2.load(f_name);
    cout << np2.size() << endl;

}

/* the experiment codes */
void exp_dynamic(string data_name, double c, double epsilon, int num_updates = 100) {
    // exp codes for dynamic graph, score for old graph must be already computed
    // load the graph
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    if (!lp_file_exists(data_name, c, epsilon, n)) {
        cout << data_name << ": the lp file has not been computed!!!" << endl;
        return;
    }
    Reduced_LocalPush rlp(g, data_name, c, epsilon, n); // already load the local push data

    // generate edges
    vector<pair<unsigned int, unsigned int>> ins_edges;
    /* initialize random seed: */
    srand(time(NULL));
    while (ins_edges.size() < num_updates) {
        auto e1 = rand() % n;
        auto e2 = rand() % n;
        if (boost::edge(e1, e2, g).second == false) {
            ins_edges.push_back({e1, e2});
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    // do the test
    for (auto &edge:ins_edges) {
        // add to g
        add_edge(edge.first, edge.second, g);
        // update P and R
        rlp.insert(edge.first, edge.second, g);
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

void compute_ground_truth(string data_name, double c = 0.6, double epsilon = 0.00001) {
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    TruthSim ts(data_name, g, c, epsilon);
    size_t n = num_vertices(g);
    cout << "computing " << data_name << " " << c << " " << epsilon << endl;
    ts.run(g);
    ts.save();
    if (n < 100) {
        cout << "ground truth for: " << data_name << endl;
        cout << ts.sim << endl;
    }
}

void generate_effective_error_rlp(string data_name) {
    // generate rlp effective
    string ERROR_DIR("/homes/ywangby/workspace/dynamicSim/datasets/effective_error/");
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    double c = 0.6;
    size_t n = num_vertices(g);
    double epsilon = 0.001;
    ofstream out(ERROR_DIR + data_name + string(".rlp.txt"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    while (epsilon < 0.011) {
        Reduced_LocalPush rlp(g, data_name, c, epsilon, n);
        // make a matrix
        MatrixXf lp_result(n, n);
        lp_result.setZero();
        for (size_t i = 0; i < n; i++) {
            for (auto &item: rlp.P.v[i]) {
                lp_result(i, item.first) = item.second; // fill up the matrix
                lp_result(item.first, i) = item.second; // fill up the matrix
            }
        }
        cout << lp_result(2, 2) << " " << ts.sim(2, 2) << endl;
        double max_error = (lp_result - ts.sim).cwiseAbs().maxCoeff();
        double mean_error = (lp_result - ts.sim).cwiseAbs().mean();
        out << max_error << " " << mean_error << endl;
        epsilon += 0.001;
    }
    out.close();
}

void generate_effective_error_flp(string data_name) {
    // generate rlp effective
    string ERROR_DIR("/homes/ywangby/workspace/dynamicSim/datasets/effective_error/");
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    double c = 0.6;
    size_t n = num_vertices(g);
    double epsilon = 0.001;
    ofstream out(ERROR_DIR + data_name + string(".flp.txt"));
    TruthSim ts(data_name, g, c, epsilon);
    cout << ts.sim.sum() << endl;
    while (epsilon < 0.011) {
        Full_LocalPush rlp(g, data_name, c, epsilon, n);
        // make a matrix
        MatrixXf lp_result(n, n);
        lp_result.setZero();
        for (size_t i = 0; i < n; i++) {
            for (auto &item: rlp.P.v[i]) {
                lp_result(i, item.first) = item.second; // fill up the matrix
                lp_result(item.first, i) = item.second; // fill up the matrix
            }
        }
        cout << lp_result(2, 2) << " " << ts.sim(2, 2) << endl;
        double max_error = (lp_result - ts.sim).cwiseAbs().maxCoeff();
        double mean_error = (lp_result - ts.sim).cwiseAbs().mean();
        out << max_error << " " << mean_error << endl;
        epsilon += 0.001;
    }
    out.close();
}

vector<NodePair> graph_gen(int n = 100, double p = 0.5) {
    // generate random edge list 
    cout << format("generating graph with %s nodes") % n << endl;
    typedef boost::adjacency_list<> Graph;
    typedef boost::erdos_renyi_iterator<boost::minstd_rand, Graph> ERGen;
    static boost::minstd_rand gen(time(NULL)); // seed the random number generator using current time
    // Create graph with 100 nodes and edges with probability 0.05
    Graph g(ERGen(gen, n, p), ERGen(), n);
    Graph::edge_iterator edge_iter, edge_end;
    tie(edge_iter, edge_end) = edges(g);
    vector<NodePair> result;
    for (auto it = edge_iter; it != edge_end; it++) {
        // cout << *it << endl;
        auto s = source(*it, g);
        auto t = target(*it, g);
        result.push_back({s, t});
    }
    cout << "generating complete..." << endl;
    return result;
}


void test_full_reduced_accu() {
    double reduced_time = 0;
    double full_time = 0;
    cout << "REDUCED:" << endl;
    for (int i = 0; i < 50; i++) {
        double t1, t2;
        auto test_d = graph_gen(50);
        cout << "testing full linear system" << endl;
        t1 = omp_get_wtime();
        test_local_push_test_data(test_d, true);
        t2 = omp_get_wtime();
        full_time += (t2 - t1);
        cout << "---------------" << endl;
        cout << "testing reduced linear system" << endl;
        t1 = omp_get_wtime();
        test_local_push_test_data(test_d);
        t2 = omp_get_wtime();
        reduced_time += (t2 - t1);
        cout << "---------------" << endl;
    }
    cout << format("full linear system cost: %ss") % full_time << endl;
    cout << format("reduced linear system cost: %ss") % reduced_time << endl;
}


int main(int args, char *argv[]) {
    try {
        vector<vector<NodePair>> test_data{e1, e2, e3, e4, e6};
        options_description desc{"Options"};
        string algo_name;
        string data_name;
        double c = 0.6;
        double epsilon = 0.01;
        double delta = 0.01;
        int number_of_updates = 100; // number of edge updates for dynamic graphs
        int r = 100; // number of sample
        int rq = 10; // number of online reverse random walk
        int t = 10; // the length of random walks
        int L = 3; // the parameter for D computation
        int T = 10;
        int R = 100;
        int R_prime = 10000;
        int x, y;
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
                ("y,y", value<int>(&y), "y: the query node");

        variables_map vm;
        store(parse_command_line(args, argv, desc), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
        } else if (vm.count("algorithm")) {
            if (algo_name == "full") {
                test_localpush(data_name, c, epsilon, true);
            } else if (algo_name == "static") {
                test_localpush(data_name, c, epsilon);
            }
        } else {
            // test multi-thread
            // test_reduction();
            // test_localpush("ca-GrQc", c, epsilon, PARALLEL_LOCAL_PUSH_TYPE);
            display_F(e7);

            // default behavior
            // vector<string> datasets{"ca-GrQc","ca-HepTh","p2p-Gnutella06","wiki-Vote"};
            // for(auto data_name:datasets){
            //     // test_lineD_all_pair(data_name, c, T, L, R);
            //     // compute_linearD_error(data_name);
            // }
            //
            //
            // test test_data
            // for(auto e: test_data){
            //     cout << "testing full linear system" << endl;
            //     test_local_push_test_data(e, true);
            //     cout << "testing reduced linear system" << endl;
            //     test_local_push_test_data(e);
            // }
            // // cout << "testing reduced linear system" << endl;
            // // test_local_push_test_data(e5);
        }
    }
    catch (const error &ex) {
        cerr << ex.what() << endl;
    }

}
