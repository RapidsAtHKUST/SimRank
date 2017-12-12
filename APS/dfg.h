#ifndef __DFG_H__
#define __DFG_H__
// the dfg index
#include <random>
#include <iterator>
#include <iostream>
#include <unordered_map>
#include <sparsepp/spp.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include "link_cut.h"
#include "graph.h"

using spp::sparse_hash_set;
using spp::sparse_hash_map;
using namespace boost::archive;

extern void sample_owg(DirectedG &g, Owg &owg);


struct DFG_Index{
    public:
        // data members
        std::unordered_map<int,int> root_prt; //using stl map instead of boost
        LinkCut lc; // the link cut tree
        int n; // the number of nodes

        // helper
        vector<int> f; // f(x) is the functional value of x 


        // member function
        DFG_Index(DirectedG &g){sample_g(g);}
        DFG_Index(){}
        void show(); // print 
        void sample_g(DirectedG &g); // init from a grpah
        int position(int x,int L); // query length-L step termination node
        void re_sample(int x, DirectedG& g); // resample in-neighbor for node x
        void save(string);
        void load(string);
};
#endif
