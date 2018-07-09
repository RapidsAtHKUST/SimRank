#ifndef __FGI_H__
#define __FGI_H__

// the functional graph index
//
#include "stat.h"
#include <util/graph_yche.h>
#include <util/sfmt_based_rand.h>
typedef GraphYche DirectedG;

typedef struct {
    int n; // number of disjoint sets
    vector<int> p, s; // parent, size

    void init(int n_) {
        n = n_;
        p.resize(n);
        s.resize(n);
        for (int i = 0; i < n; ++i) {
            p[i] = i;
            s[i] = 1;
        }
    }

    int F(int x) {
        return x == p[x] ? x : (p[x] = F(p[x]));
    }

    void U(int x, int y) {
        x = F(x), y = F(y);
        if (x == y) return;
        --n;
        if (s[x] < s[y]) {
            p[x] = y;
            s[y] += s[x];
            s[x] = 0;
        } else {
            p[y] = x;
            s[x] += s[y];
            s[y] = 0;
        }
    }
} Union_Find;

struct FG_Index {
    DirectedG * g_ptr;
    int N; // number of trees
    int gn; // number of nodes
    double c;
    vector<vector<int>> t_pos; // the terminate positions for different nodes as index
    vector<vector<int>> f; // in neighbor
    vector<int> len; // walk length
    SFMTRand rand_gen;
    vector<Union_Find> uf;
    int fgi_hit;
    int fgi_miss;

    FG_Index(DirectedG &g, int N_, double c_): g_ptr(&g), c(c_) {
        N = N_;
        gn = g_ptr->n;
        // init the vector
        t_pos.resize(N);
        f.resize(N);
        uf.resize(N);
        for (int i = 0; i < N; i++) {
            t_pos[i].resize(g_ptr->n);
            f[i].resize(g_ptr->n);
            uf[i].init(g_ptr->n);
        }
        len.resize(N);
        fgi_hit = fgi_miss = 0;
    }

    void build_index();
    int query(const NodePair& np, int i); // query whether walks of np meet in tree i
};



#endif
