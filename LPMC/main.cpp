#include <cstdlib>
#include <ctime>

#include <iostream>
#include <unordered_set>

#include <boost/program_options.hpp>

#include "bprw.h"

using namespace std;
using namespace std::chrono;
using namespace boost::program_options;

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

}

int main(int args, char *argv[]) {
    string data_name(argv[1]);
    double c = 0.6;
    double epsilon = 0.01;
    double delta = 0.01;

    int x = atoi(argv[2]), y = atoi(argv[3]);
    test_bp(data_name, c, epsilon, delta, x, y);
}