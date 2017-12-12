#include "linearD.h"

string LinearD::get_file_path_base(){
    return LINEAR_D_DIR + str(format("%s-%.3f-%s-%s-%s") % g_name % c % T % L % R );
}

void LinearD::save(){
    cout << "saving to disk..." << endl;
    write_binary((get_file_path_base()+".bin").c_str(),sim);
    ofstream out(get_file_path_base()+".meta");
    out << cpu_time << endl;
    out << mem_size << endl;
    out << n << endl;
    out.close();
}
void LinearD::load(){
    read_binary((get_file_path_base()+".bin").c_str(),sim);
}

LinearD::LinearD(DirectedG * graph, string name, double c_, int T_, int L_, int R_){
    c = c_;
    T = T_;
    L = L_;
    R = R_;
    g = graph;
    g_name = name;
    n = num_vertices(*g);
    D.resize(n);
    D.setOnes();
    P.resize(n,n);
    PT.resize(n,n);
    compute_D();
    compute_P();
}

void LinearD::compute_D(){
    cout << "computing D for " << g_name << endl;
    cout << "T: " << T << endl;
    for(int l = 0; l < L; l++){
        for(size_t k = 0; k < n; k++){
            double alpha, beta, delta;
            tie(alpha, beta) = estimate_SDkk_SEkk(k);
            // cout << "alpha and beta: " << alpha << " " << beta << endl;
            delta = (1.0-alpha) / beta;
            D(k) += delta;
        }
    }
}

void LinearD::compute_P(){
    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletListP, tripletListPT;
    tripletListP.reserve(num_edges(*g));
    tripletListPT.reserve(num_edges(*g));
    DirectedG::edge_iterator edge_it, edge_end;
    tie(edge_it, edge_end) = edges(*g);
    for(;edge_it != edge_end; edge_it++){
        auto s = source(*edge_it,*g);
        auto t = target(*edge_it,*g);
        double v = 1.0 / in_degree(t,*g);
        tripletListP.push_back(T(s,t,v));
        tripletListPT.push_back(T(t,s,v));
    }
    P.setFromTriplets(tripletListP.begin(), tripletListP.end());
    PT.setFromTriplets(tripletListPT.begin(), tripletListPT.end());
}

pair<double, double> LinearD::estimate_SDkk_SEkk(int k){
    // estimate S(D)_{kk} and S(E_{k})_{kk}
    // alpha: S(D)_{kk}
    // beta: S(E)_{kk}
    double alpha = 0, beta = 0;

    std::unordered_map<int,int> *pre_pos,*next_pos, pre_hash, next_hash;
    pre_pos = &pre_hash;
    next_pos = &next_hash;
    (*pre_pos)[k] = R; // initially there 
    for(int t = 0; t<T; ++t){
        // update 
        (*next_pos).clear();
        for(auto& item: (*pre_pos)){
            int position; 
            int number; // number of walks at current position
            tie(position,number) = item;
            // cout << position << " " << number << endl;
               
            alpha += pow(c,t) * pow( double(number) / double(R), 2.0) * D(position);
            if( position == k){
                beta += pow(c,t) * pow(double(number) / double(R), 2.0);
            }
            // move to the next step
            for(int ii = 0; ii < number; ii++){
                auto sampled_in_neighbor = sample_in_neighbor(position,*g);
                if(sampled_in_neighbor != -1){ // there is no in-neighbor at current node
                    (*next_pos)[sampled_in_neighbor] += 1;
                }
            }
        }
        swap(pre_pos,next_pos);
    }
    return pair<double,double>{alpha, beta};
}

void LinearD::all_pair(){
    sim.resize(n,n);
    sim.setZero();
    VectorXd tmp(n);
    for(size_t i= 0; i < n;i++){
        tmp.setZero();
        single_source(i,tmp);
        sim.row(i) = tmp;
    }
}

void LinearD::single_source(int i, VectorXd &r){
    // single source simrank
    MatrixXd Tstep_dist(T+1, n);
    Tstep_dist.setZero();
    VectorXd e(n);
    e.setZero();
    e(i) = 1;
    for(int t = 0; t<=T; ++t){
        Tstep_dist.row(t) = e;
        e = P * e;
    }
    for(int t = 0; t<= T; ++t){
        Tstep_dist.row(t) = Tstep_dist.row(t).cwiseProduct(D);
    }
    r = Tstep_dist.row(T);
    for(int t = T-1; t>= 0; t--){
        r = Tstep_dist.row(t) + c * PT * r;
    }
    mem_size = getValue();
}

