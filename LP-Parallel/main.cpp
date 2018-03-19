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

int FULL_LOCAL_PUSH_TYPE = 0;
int REDUCED_LOCAL_PUSH_TYPE = 1;
int PARALLEL_LOCAL_PUSH_TYPE = 2;

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

int main(int args, char *argv[]) {
    try {
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
        }
    }

    catch (const error &ex) {
        cerr << ex.what() << endl;
    }
}
