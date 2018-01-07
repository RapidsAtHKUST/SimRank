#include <cstdlib>
#include <ctime>

#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "bprw.h"
#include "flpmc.h" 
#include "bflpmc.h"
#include "simrank.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;

double ground_truth(string data_name, double c, double epsilon, double delta, int x, int y){
    /* compute the ground truth of x and y */
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    NodePair q{x,y};

    TruthSim ts(data_name, g, c, epsilon);
    // cout << format("ground truth: %s") % ts.sim(x,y) << endl;
    return ts.sim(x,y);
}

void test_FLPMC(string data_name, double c, double epsilon, double delta, int x, int y){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    NodePair q{x,y};

    FLPMC flpmc(data_name, g, c, epsilon, delta, 100);
    double result = flpmc.query_one2one(q);
    cout << result << endl;

    // ground truth
    double truth = ground_truth(data_name, c, epsilon, delta, x,y);
    cout << format("ground truth: %s") % truth << endl;
    cout << format("error: %s") % (truth - result) << endl;
}


void test_bp(string data_name, double c, double epsilon, double delta, int x, int y) {
    // test_readsrq(data_name,c,epsilon,R_prime,R,t);
    // test the max heap functionality
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    BackPush bprw(data_name, g, c, epsilon, delta);
    size_t n = num_vertices(g);
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

void test_BFLPMC(string data_name, double c, double epsilon, double delta, int x, int y){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    NodePair q{x,y};
    BFLPMC bflpmc(data_name, g, c, epsilon, delta);

    double result = bflpmc.query_one2one({x,y});
    cout << format("result of BFLPMC: %s") % result << endl;

    double truth = ground_truth(data_name, c, epsilon, delta, x, y);
    cout << format("ground truth: %s") % truth << endl;
    
}

// int main(int args, char *argv[]) {
//     string data_name(argv[1]);
//     double c = 0.6;
//     double epsilon = 0.01;
//     double delta = 0.01;
// 
//     int x = atoi(argv[2]), y = atoi(argv[3]);
//     test_bp(data_name, c, epsilon, delta, x, y);
// }

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
            ("epsilon,e", value<double>(&epsilon), "Error bound");

        variables_map vm;
        store(parse_command_line(args, argv, desc), vm);
        notify(vm);

        if (vm.count("help")){
            cout << desc << endl;
        }else if(vm.count("method")){
            if(method == "flp"){
                test_FLPMC(data_name, c, epsilon, delta, x, y);
            }else if(method == "blp"){
                test_bp(data_name, c, epsilon, delta, x, y);
            }else if (method == "bflp"){
                test_BFLPMC(data_name, c, epsilon, delta, x,  y);
            }
        }

    }
    catch (const error &ex ){
        cerr << ex.what() << endl;
    }
    return 0;
}
