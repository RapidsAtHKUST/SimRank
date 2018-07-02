#ifndef __FGI_H__
#define __FGI_H__

// the functional graph index
//
#include <util/graph_yche.h>
#include <util/sfmt_based_rand.h>
typedef GraphYche DirectedG;

struct FG_Index{
    DirectedG * g_ptr;
    int N; // number of trees
    double c;
    vector<vector<unsigned int>> t_pos;  // the terminate positions for different nodes as index
    FG_Index(DirectedG &g, int N_, double c_ ): g_ptr(&g), c(c_){
        N = N_;
        // init the vector
        t_pos.resize(N);
        for(int i = 0; i< N ; i++){
            t_pos[i].resize(g_ptr->n);
        }
    }

    void build_index();
};



#endif
