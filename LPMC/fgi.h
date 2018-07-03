#ifndef __FGI_H__
#define __FGI_H__

// the functional graph index
//
#include "stat.h"
#include <util/graph_yche.h>
#include <util/sfmt_based_rand.h>
typedef GraphYche DirectedG;

struct FG_Index {
    DirectedG * g_ptr;
    int N; // number of trees
    int gn; // number of nodes
    double c;
    vector<vector<int>> t_pos;  // the terminate positions for different nodes as index
    vector<vector<int>> f; // in neighbor
    vector<int> len; // walk length
    SFMTRand rand_gen;
    
    FG_Index(DirectedG &g, int N_, double c_): g_ptr(&g), c(c_) {
        N = N_;
        gn = g_ptr->n;
        // init the vector
        t_pos.resize(N);
        f.resize(N);
        for (int i = 0; i < N; i++) {
            t_pos[i].resize(g_ptr->n);
            f[i].resize(g_ptr->n);
        }
        len.resize(N);
    }

    void build_index();
    bool query(const NodePair& np, int i); // query whether walks of np meet in tree i
};



#endif
