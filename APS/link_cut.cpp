#include <iostream>
#include <utility>
#include <vector>

#include "link_cut.h"

using namespace std;

int depth(Node *x) {
    access(x);
    return x->sz - 1;
}

Node *lca(Node *x, Node *y) {
    access(x);
    return access(y);
}

void link(Node *x, Node *y) {
    access(x);
    access(y);
    x->l = y;
    y->p = x;
    update(x);
}

void cut(Node *x) {
    access(x);
    x->l->p = 0;
    x->l = 0;
    update(x);
}

Node *root(Node *x) {
    access(x);
    while (x->l) x = x->l;
    splay(x);
    return x;
}

int Kth_ancestor(Node *x, int k) {
    access(x);
    Node *y = Kth_large(x, k + 1);
    return y->label;
}

Node *Kth_large(Node *root, int k) {
    // find k-th largest element in a splay tree 
    if (k < 1 || k > root->sz) {
        return NULL;
    }
    int right_size = r_size(root);
    if (k == right_size + 1) {
        return root;
    } else if (k > 1 + right_size) {
        return Kth_large(root->l, k - right_size - 1);
    } else {
        return Kth_large(root->r, k);
    }
}

int r_size(Node *x) {
    // size of right sub-tree
    if (x->r) {
        return x->r->sz;
    } else {
        return 0;
    }
}

Node *access(Node *x) {
    splay(x);
    if (x->r) {
        x->r->pp = x;
        x->r->p = 0;
        x->r = 0;
        update(x);
    }

    Node *last = x;
    while (x->pp) {
        Node *y = x->pp;
        last = y;
        splay(y);
        if (y->r) {
            y->r->pp = y;
            y->r->p = 0;
        }
        y->r = x;
        x->p = y;
        x->pp = 0;
        update(y);
        splay(x);
    }
    return last;
}

void splay(Node *x) {
    Node *y, *z;
    while (x->p) {
        y = x->p;
        if (y->p == 0) {
            if (x == y->l) rotr(x);
            else rotl(x);
        } else {
            z = y->p;
            if (y == z->l) {
                if (x == y->l) rotr(y), rotr(x);
                else rotl(x), rotr(x);
            } else {
                if (x == y->r) rotl(y), rotl(x);
                else rotr(x), rotl(x);
            }
        }
    }
    update(x);
}

void update(Node *x) {
    x->sz = 1;
    if (x->l) x->sz += x->l->sz;
    if (x->r) x->sz += x->r->sz;
}

void rotr(Node *x) {
    Node *y, *z;
    y = x->p, z = y->p;
    if ((y->l = x->r)) y->l->p = y;
    x->r = y, y->p = x;
    if ((x->p = z)) {
        if (y == z->l) z->l = x;
        else z->r = x;
    }
    x->pp = y->pp;
    y->pp = 0;
    update(y);
}

void rotl(Node *x) {
    Node *y, *z;
    y = x->p, z = y->p;
    if ((y->r = x->l)) y->r->p = y;
    x->l = y, y->p = x;
    if ((x->p = z)) {
        if (y == z->l) z->l = x;
        else z->r = x;
    }
    x->pp = y->pp;
    y->pp = 0;
    update(y);
}

// int main(){
//     int n = 8;
//     LinkCut lc(n+1);
//     vector<pair<int,int>> links = {
//         {1,2},
//         {2,4},
//         {4,5},
//         {3,4},
//         {7,6},
//         {8,6},
//         {6,5}
//     };
//     for(auto &item:links){
//         lc.link(item.first, item.second);
//     }
//     vector<pair<int,int>> query_pairs = {
//         {1,1},
//         {1,2},
//         {1,3},
//         {3,1},
//         {3,2},
//         {7,1},
//         {7,2},
//         {8,1},
//         {8,2}
//     };
//     for(auto &pair:query_pairs){
//         cout << pair.first << " " << 
//             pair.second << ": " << lc.kth_ans(pair.first,pair.second)<< endl;
//     }
//     save(lc, string("iotest.txt"));
//     LinkCut loaded_lc;
//     load(loaded_lc, string("iotest.txt"));
//     cout << "after load" << endl;
//     for(auto &pair:query_pairs){
//         cout << pair.first << " " << 
//             pair.second << ": " << loaded_lc.kth_ans(pair.first,pair.second)<< endl;
//     }
// }
