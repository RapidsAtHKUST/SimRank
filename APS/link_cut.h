#ifndef __LINK_CUT_H__
#define __LINK_CUT_H__

#include <cstdio>
#include <fstream>

#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;
using namespace boost::archive;

template<typename A>
void save(A &dfg, string file_path) {
    ofstream file{file_path};
    text_oarchive oa{file};
    oa << dfg;
}

template<typename A>
void load(A &dfg, string file_path) {
    ifstream file{file_path};
    text_iarchive ia{file};
    ia >> dfg; // load the data into dfg
}

struct Node {
    int sz, label, color; /* size, label, color(the sampled path) */
    Node *p, *pp, *l, *r; /* parent, path-parent, left, right pointers */
    Node() { p = pp = l = r = NULL; }

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & p;
        ar & pp;
        ar & l;
        ar & r;
        ar & sz;
        ar & label;
        ar & color;
    }
};

extern void update(Node *x);

extern void rotr(Node *x);

extern void rotl(Node *x);

extern void splay(Node *x);

extern Node *access(Node *x);

extern int r_size(Node *x);

extern Node *Kth_large(Node *root, int k);

extern int Kth_ancestor(Node *x, int k);

extern Node *root(Node *x);

void cut(Node *x);

void link(Node *x, Node *y);

extern Node *lca(Node *x, Node *y);

extern int depth(Node *x);

class LinkCut {

public:
    vector<Node *> x; // x is an array of pointers
    LinkCut(int n) {   // x = new Node[n];
        add_node(n);
    }

    void add_node(int n) {
        for (int i = 0; i < n; i++) {   // x[i].label = i;
            x.push_back(new Node());
            x[i]->label = i;
            update(x[i]);
        }
    }

    LinkCut() {}

    // virtual ~LinkCut()
    // {
    //     delete[] x;
    //     // if(x != NULL){
    //     //     delete[] x;
    //     // }

    // }

    void link(int u, int v) {
        ::link(x[u], x[v]);
    }

    void cut(int u) {
        ::cut(x[u]);
    }

    int root(int u) {
        return ::root(x[u])->label;
    }

    int depth(int u) {
        return ::depth(x[u]);
    }

    int kth_ans(int u, int k) {
        return ::Kth_ancestor(x[u], k);
    }

    int lca(int u, int v) {
        return ::lca(x[u], x[v])->label;
    }

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & x;
    }
};

#endif
