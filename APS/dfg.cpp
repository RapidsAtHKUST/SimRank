#include <iostream>
#include "dfg.h"
using namespace std;
using std::unordered_map;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

void sample_owg(DirectedG &g, Owg &owg){
    // sample an one-way graph 
    DirectedG::vertex_iterator node_iter,node_end;
    tie(node_iter, node_end) = vertices(g);
    for(;node_iter != node_end; node_iter ++){
        auto a = *node_iter;
        if (in_degree(a, g) > 0){
            auto b = sample_in_neighbor(a, g);
            owg[a] = b;
        }
    }
} 

void DFG_Index::sample_g(DirectedG &g){
    auto start = high_resolution_clock::now();
    cout << "building DFG_index..." << endl;
    n = num_vertices(g);
    //init the function vector 
    f.clear();
    f.insert(f.end(),n,-1); // fill the vector with n of -1s
    lc.add_node(n); // init Link-cut tree with size n
    vector<bool> marker(n,false);// record whether a node is already sampled
    int i = 0; // the iterator to sample all nodes
    // sparse_hash_set<int> node_set;
    DirectedG::vertex_iterator node_iter,node_end;
    tie(node_iter, node_end) = vertices(g);
    // init the node_set
    // for(;node_iter != node_end;node_iter++){
    //     node_set.insert(*node_iter);
    // }
    cout << "n: " << n << endl;
    while(i < n){
        while(marker[i] == true ){ // ignore the already sampled nodes
            i++;
        }
        if(i >= n){ // i is out of range
            break;
        }
        // cout << "sample for " << i << endl;
        auto a = i; // a new node needs to be sampled
        // auto a = *(node_set.begin());
        auto current_color = a;
        // node_set.erase(node_set.begin());
        lc.x[a]->color = a; // set the color
        marker[a] = true;
        while(in_degree(a,g) > 0){
            // node_set.erase(a);
            int b = sample_in_neighbor(a,g);
            marker[a] = true;
            f[a]=b; // update f(x)
            if (marker[b] == true){ // b has been already explored
                if(lc.x[b]->color == current_color){// b is the same path with a
                    root_prt[a] = b;
                }else{ // a link to a existing WCC
                    lc.link(a,b);
                }
                break;
            }else{ // b has not been explored
                lc.x[b]->color = current_color;
                lc.link(a,b);
                a = b;
            }
        }
        current_color = -1;
    }
    auto finish = high_resolution_clock::now();
    duration<double> elapsed = finish - start;
    cout << "cost " << elapsed.count() <<"s"<< endl;

}
void DFG_Index::show(){
    cout << "f(x)" << endl;
    for(size_t i =0;i < f.size(); i++ ){
        cout << i << ": " << f[i] << endl;
    }
    cout << "below are root pointers:" << endl;
    for(auto &item: root_prt){
        cout << "root " << item.first << " pointes to " << item.second << endl;
    }
    cout << "roots of all nodes: "<< endl;
    for(int  i=0;i<n;i++){
        cout << "root of " << i  << "  is  " << lc.root(i) << endl;
    }
}

int DFG_Index::position(int u, int k){
    // return kth-step position of node u
    int u_depth = lc.depth(u);
    if(k < 0){
        return -1;
    }else if(k>=0 && k<=u_depth){
        return lc.kth_ans(u,k);
    }else{ // longer than depth
        k = k - u_depth -1;
        int root = lc.root(u);
        if(root_prt.find(root) == root_prt.end()){
            // this root has no cycle
            return root;
        }else{
            int s = root_prt[root]; // the cycle children
            int s_depth = lc.depth(s);
            int cycle_length = s_depth + 1;
            k = k % cycle_length;
            return lc.kth_ans(s,k);
        }
    }
}
void DFG_Index::re_sample(int u, DirectedG& g){
    if(in_degree(u,g) > 0){
        auto v = sample_in_neighbor(u, g);
        f[u] = v;
        if(v == position(u,1)){
            // same as previous
            return;
        }else{
            // test whether in cycle
            int root_u = lc.root(u);
            int root_v = lc.root(v);
            int s;
            if(root_prt.find(root_u) == root_prt.end()){
                // this tree has no cycle
                s = -1;
            }else{
                s = root_prt[root_u]; // the lowest children in the cycle
            }
            if(s == -1 || lc.lca(u,s) != u){
                // u is not in cycle or u's tree has no cycle
                lc.cut(u);
                lc.link(u,v);
            }else{
                // u is in cycle
                if(u != root_u){
                    // make u as root
                    lc.cut(u);
                    lc.link(root_u, root_prt[root_u]);
                    root_prt.erase(root_u);
                }
                if(root_u == root_v){
                    root_prt[u] = v;
                }else{
                    lc.link(u,v);
                }
            }
        }
    }else{
        // now u has  no in-neighbor
        f[u] = -1;
        int root_u = lc.root(u);
        if(u == root_u){
            root_prt.erase(u);
        }else{
            lc.cut(u);
        }
    }
}
void DFG_Index::save(string path){
    std::ofstream file{path};
    text_oarchive oa{file};
    oa << f << root_prt;
}
void DFG_Index::load(string path){
    auto start = high_resolution_clock::now();
    std::ifstream file{path};
    text_iarchive ia{file};
    ia >> f >> root_prt;
    n = f.size();
    lc.add_node(n); // init the link cut tree
    for(size_t i=0; i<f.size();++i){
        if(root_prt.find(i) == root_prt.end() && f[i] != -1){
            // i is not a tree root
            lc.link(i,f[i]);
        }
    }
    auto finish = high_resolution_clock::now();
    duration<double> elapsed = finish - start;
    cout << "cost " << elapsed.count() <<"s"<< endl;
    // // build dfg from f 
    // DirectedG g;
    // for(size_t i=0;i<f.size();i++){
    //     if(f[i] != -1){
    //         add_edge(f[i],i, g);
    //     }
    // }
    // cout << "loading the graph: " << endl;
    // // show_graph(g);
    // sample_g(g);
}

void save_sparse_map(Owg & o, string fp){
    FILE *out = fopen(fp.c_str(), "wb");
    o.serialize(FileSerializer(), out);
    fclose(out);
}
void load_sparse_map(Owg & o, string fp){
    FILE *input = fopen(fp.c_str(), "rb");
    o.unserialize(FileSerializer(), input);
    fclose(input);
}

void test(){
    DirectedG g;
    vector<pair<int,int>> edges = {
        {5,4},
        {5,6},
        {4,3},
        {4,2},
        {2,1},
        {6,7},
        {6,8},
        {8,5}
    };
    for(auto &edge:edges){
        add_edge(edge.first, edge.second, g);
    }
    cout << "g nodes: " << num_vertices(g) << endl;
    DFG_Index dfg(g);
    dfg.show();
    remove_edge(8,5,g);
    for(int i = 1;i<=8;i++){
        cout << "resample for " << i << endl;
        dfg.re_sample(i, g);
    }
    cout << "after resampling" << endl;
    dfg.show();
    string s("dfg_io.test");
    dfg.save(s);
    DFG_Index dfg2;
    cout << "after loading..." << endl;
    dfg2.load(s);
    dfg2.show();
    for(int i = 1;i<=8;i++){
        for(int l=0;l<=10;l++){
            cout << l<<"th ancestor of "  << i << " is " << dfg2.position(i,l) << endl;
        }
    }
}


// int main(){
//     // test();
//     DirectedG  g;
//     cout << "loading graph" << endl;
//     load_graph(get_edge_list_path("soc-LiveJournal1"), g);
//     DFG_Index dfg(g);
//     string fp("dfg_test.txt");
//     cout << "saving...." << endl;
//     dfg.save(fp);
//     DFG_Index test_dfg;
//     cout << "loading..." << endl;
//     test_dfg.load(fp);
//     cout << test_dfg.n << endl;
//     return 0;
//     // Owg o;
//     // load_sparse_map(o, string("one-way-livej"));
//     // cout << o.size() << endl;
// }
