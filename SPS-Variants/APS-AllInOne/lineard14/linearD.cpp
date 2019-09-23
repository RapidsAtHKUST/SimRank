#include "linearD.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "../playground/boost_serialization_eigen.h"

string LinearD::get_file_path_base() {
    mkdir_if_not_exist(LINEAR_D_DIR);
    return LINEAR_D_DIR + str(format("%s-%.3f-%s-%s-%s") % g_name % c % T % L % R);
}

string LinearD::get_p_file_path_base() {
    mkdir_if_not_exist(LINEAR_D_DIR);
    return LINEAR_D_DIR + str(format("%s") % g_name);
}

void LinearD::build_or_load_index() {
    string p_file = get_p_file_path_base() + ".P";
    string pt_file = get_p_file_path_base() + ".PT";
    string d_file = get_file_path_base() + ".D";
    if (file_exists(d_file) && file_exists(p_file) && file_exists(pt_file)) {
        std::ifstream ifs(d_file, ios::binary | ios::in);
        boost::archive::binary_iarchive ia(ifs);
        boost::serialization::load(ia, D, 0);
        ifs.close();

        std::ifstream ifs1(p_file);
        Deserialize(ifs1, P);
        ifs1.close();

        std::ifstream ifs2(pt_file);
        Deserialize(ifs2, PT);
        ifs2.close();
        n = static_cast<size_t>(D.size());
    } else {
        DirectedG my_graph;
        auto data_name = string(g_name);
        load_graph(get_edge_list_path(data_name), my_graph);
        n = num_vertices(my_graph);

        g = &my_graph;
        D.resize(n);
        D.setOnes();
        P.resize(n, n);
        PT.resize(n, n);

        auto start = std::chrono::high_resolution_clock::now();
        compute_D();
        compute_P();
        g = nullptr;
        auto pre_time = std::chrono::high_resolution_clock::now();
        cout << "indexing time:"
             << float(std::chrono::duration_cast<std::chrono::microseconds>(pre_time - start).count()) / (pow(10, 6))
             << " s\n";

        // serialization
        std::ofstream ofs(d_file);
        boost::archive::binary_oarchive oa(ofs);
        boost::serialization::save(oa, D, 0);
        ofs.close();

        std::ofstream ofs1(p_file);
        Serialize(ofs1, P);
        ofs1.close();

        std::ofstream ofs2(pt_file);
        Serialize(ofs2, PT);
        ofs2.close();

        cout << "mem size:" << getValue() << endl;
    }
}

#ifdef SAVE_LOAD
void LinearD::save() {
    cout << "saving to disk..." << endl;
    write_binary((get_file_path_base() + ".bin").c_str(), sim);
    ofstream out(get_file_path_base() + ".meta");
    out << cpu_time << endl;
    out << mem_size << endl;
    out << n << endl;
    out.close();
}

void LinearD::load() {
    read_binary((get_file_path_base() + ".bin").c_str(), sim);
}
#endif

LinearD::LinearD(DirectedG *graph, string name, double c_, int T_, int L_, int R_) {
    c = c_;
    T = T_;
    L = L_;
    R = R_;
    g_name = std::move(name);

    g = graph;
    n = num_vertices(*g);

    D.resize(n);
    D.setOnes();
    P.resize(n, n);
    PT.resize(n, n);

    compute_D();
    compute_P();
    cout << "mem size:" << getValue() << endl;
}

LinearD::LinearD(string name, double c_, int T_, int L_, int R_) {
    c = c_;
    T = T_;
    L = L_;
    R = R_;
    g_name = name;
    build_or_load_index();
}

pair<double, double> LinearD::estimate_SDkk_SEkk(int k) {
    // estimate S(D)_{kk} and S(E_{k})_{kk}
    // alpha: S(D)_{kk}
    // beta: S(E)_{kk}
    double alpha = 0, beta = 0;

    std::unordered_map<int, int> *pre_pos, *next_pos, pre_hash, next_hash;
    pre_pos = &pre_hash;
    next_pos = &next_hash;
    (*pre_pos)[k] = R; // initially there
    for (int t = 0; t < T; ++t) {
        // update
        (*next_pos).clear();
        for (auto &item: (*pre_pos)) {
            int position;
            int number; // number of walks at current position
            tie(position, number) = item;
            // cout << position << " " << number << endl;

            alpha += pow(c, t) * pow(double(number) / double(R), 2.0) * D(position);
            if (position == k) {
                beta += pow(c, t) * pow(double(number) / double(R), 2.0);
            }
            // move to the next step
            for (int ii = 0; ii < number; ii++) {
                auto sampled_in_neighbor = sample_in_neighbor(position, *g);
                if (sampled_in_neighbor != -1) { // there is no in-neighbor at current node
                    (*next_pos)[sampled_in_neighbor] += 1;
                }
            }
        }
        swap(pre_pos, next_pos);
    }
    return pair<double, double>{alpha, beta};
}

void LinearD::compute_D() {
    cout << "computing D for " << g_name << endl;
    cout << "T: " << T << endl;
    for (int l = 0; l < L; l++) {
        for (size_t k = 0; k < n; k++) {
            double alpha, beta, delta;
            tie(alpha, beta) = estimate_SDkk_SEkk(k);
//             cout << "alpha and beta: " << alpha << " " << beta << endl;
            delta = (1.0 - alpha) / beta;
            D(k) += delta;
        }
    }
}

void LinearD::compute_P() {
    cout << "compute p..." << endl;
    typedef Eigen::Triplet<double> T;
    std::vector<T> tripletListP, tripletListPT;
    tripletListP.reserve(num_edges(*g));
    tripletListPT.reserve(num_edges(*g));
    DirectedG::edge_iterator edge_it, edge_end;
    tie(edge_it, edge_end) = edges(*g);
    for (; edge_it != edge_end; edge_it++) {
        auto s = source(*edge_it, *g);
        auto t = target(*edge_it, *g);
        double v = 1.0 / in_degree(t, *g);
        tripletListP.push_back(T(s, t, v));
        tripletListPT.push_back(T(t, s, v));
    }
    P.setFromTriplets(tripletListP.begin(), tripletListP.end());
    PT.setFromTriplets(tripletListPT.begin(), tripletListPT.end());
    cout << "finish compute p" << endl;
}

void LinearD::all_pair() {
    sim.resize(n, n);
    sim.setZero();
    VectorXd tmp(n);
    for (size_t i = 0; i < n; i++) {
        tmp.setZero();
        single_source(i, tmp);
        sim.row(i) = tmp;
    }
}

// time comleixty: T * m, space complexity: (T+1) * n
void LinearD::single_source(int i, VectorXd &r) {
    MatrixXd Tstep_dist(T + 1, n);
    Tstep_dist.setZero();
    VectorXd e(n);
    e.setZero();
    e(i) = 1;
    for (int t = 0; t <= T; ++t) {
        Tstep_dist.row(t) = e;
        e = P * e;
    }
    for (int t = 0; t <= T; ++t) {
        Tstep_dist.row(t) = Tstep_dist.row(t).transpose().cwiseProduct(D);
    }
    r = Tstep_dist.row(T);
    for (int t = T - 1; t >= 0; t--) {
        r = Tstep_dist.row(t).transpose() + c * PT * r;
    }
    mem_size = getValue();
}

// time complexity: T * m, space complexity: 2n
double LinearD::single_pair(int i, int j) {
    VectorXd lhs_vec(n), rhs_vec(n);
    return single_pair(i, j, lhs_vec, rhs_vec);
}

double LinearD::single_pair(int i, int j, VectorXd &lhs_vec, VectorXd &rhs_vec) {
    if (i == j) { return 1; }
    double res = 0.0;
    double cur_decay = 1.0;
    lhs_vec.setZero();
    lhs_vec(i) = 1;
    rhs_vec.setZero();
    rhs_vec(j) = 1;

    for (auto t = 0; t < T; ++t) {
        res += cur_decay * lhs_vec.cwiseProduct(D).transpose() * (rhs_vec);
        lhs_vec = P * lhs_vec;
        rhs_vec = P * rhs_vec;
        cur_decay *= c;
    }
    res += cur_decay * lhs_vec.cwiseProduct(D).transpose() * (rhs_vec);
    return res;
}


