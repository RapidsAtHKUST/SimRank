#ifndef __SIMRANK_H__
#define __SIMRANK_H__
#include <boost/format.hpp>
#include "graph.h"


extern void basic_simrank(DirectedG &g, double c, SimRank_matrix&);
const string GROUND_TRUTH_DIR("./datasets/ground_truth/");

struct TruthSim{
    size_t n;
    MatrixXf sim;
    float epsilon;
    float c;
    string g_name;
    TruthSim(){};
    TruthSim(string name, DirectedG &g, double c_, double epsilon_);
    void run(DirectedG &g);
    void save(){
        // save to disk
        write_binary(get_file_path().c_str(), sim);
    }     
    void load(){
        // load from disk
        read_binary(get_file_path().c_str(), sim);
    }
    string get_file_path(){ // file path to save and load
        return GROUND_TRUTH_DIR + str(format("GROUND_%s-%.3f.bin")  % 
                g_name % c );
    }
};

#endif
