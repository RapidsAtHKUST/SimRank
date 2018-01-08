#include "cloud_walker.h"

string CloudWalker::get_file_path_base() {
    return CLOUD_WALKER_DIR + str(format("%s-%.3f-%s-%s-%s-%s") % g_name % c % T % L % R % R_prime);
}

void CloudWalker::save() {
    cout << "saving to disk..." << endl;
    write_binary((get_file_path_base() + ".bin").c_str(), sim);
    ofstream out(get_file_path_base() + ".meta");
    out << cpu_time << endl;
    out << mem_size << endl;
    out << n << endl;
    out.close();
}

void CloudWalker::load() {
    read_binary((get_file_path_base() + ".bin").c_str(), sim);
}

void CloudWalker::mcap() {
    auto start = std::chrono::high_resolution_clock::now();
    sim.resize(n, n);
    sim.setZero();
    VectorXd tmp(n);
    for (size_t i = 0; i < n; i++) {
        tmp.setZero();
        if (i % 100 == 0) {
            cout << str(format("computing single source for %s/%s") % i % n) << endl;
        }
        mcss(i, tmp);
        sim.row(i) = tmp;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    cpu_time = elapsed.count();
    mem_size = getValue();
}

CloudWalker::CloudWalker(DirectedG *graph,
                         string name, double c_, int T_, int L_, int R_, int R_prime_) {
    c = c_;
    T = T_;
    L = L_;
    R = R_;
    R_prime = R_prime_;
    g = graph;
    g_name = name;
    n = num_vertices(*g);

    D.resize(n);
    // D.setOnes(); // initially D = I
    D.setZero();
    F.resize(n);
    F.setZero();
    hat_P.resize(n, n);
    vector<int> out_degrees(n, 0);
    for (int i = 0; i < n; i++) {
        out_degrees[i] = out_degree(i, *g);
    }
    hat_P.reserve(out_degrees);
    preprocess_D();
    preprocess_F();
    preprocess_hat_P();
}

void CloudWalker::Tstep_distribution(int i, MatrixXd &pos_dist) {
    pos_dist.setZero();
    /* start run R random walks */
    int current_pos;
//    for (int k = 0; k < R; k++) {
    for (int k = 0; k < R_prime; k++) {
        int t = 0;
        current_pos = i;
        while (t <= T) {
//            pos_dist(t, current_pos) += (1.0 / R);
            pos_dist(t, current_pos) += (1.0 / R_prime);
            auto sampled_in_neighbor = sample_in_neighbor(current_pos, *g);
            if (sampled_in_neighbor != -1) {
                current_pos = sampled_in_neighbor;
            } else {
                break; // there is no in-neighbor for current_pos
            }
            ++t;
        }
    }
}

void CloudWalker::preprocess_D() {
    cout << "computing D" << endl;
    // use jacobi method to compute D

    vector<std::unordered_map<int, double>> A; // the sparse matrix A for the linear system
    for (int i = 0; i < n; i++) {
        A.push_back(std::unordered_map<int, double>());
    }


    DirectedG::vertex_descriptor i, current_pos;

    std::unordered_map<int, int> *pre_ptr, pre_pos, *next_ptr, next_pos;
    pre_ptr = &pre_pos;
    next_ptr = &next_pos;
    for (i = 0; i < n; i++) {
        // fill the sparse matrix A
        // cout << "i: " << endl;
        (*pre_ptr).clear();
        (*pre_ptr)[i] = R;
        for (int t = 0; t <= T; t++) {
            // cout << "T: " << t << endl;
            (*next_ptr).clear();
            for (auto &item : (*pre_ptr)) {
                // update a_i
                int position, number;
                tie(position, number) = item;
                A[i][position] += pow(c, t) * pow(double(number) / double(R), 2); // update the sparse matrix A
                // choose the next position
                for (int k = 0; k < number; k++) {
                    auto sampled_in_neighbor = sample_in_neighbor(position, *g);
                    if (sampled_in_neighbor != -1) {
                        (*next_ptr)[sampled_in_neighbor] += 1;
                    }
                }
            }
            swap(pre_ptr, next_ptr);
        }
    }
    // cout << A << endl;
    // solve the linear system for D
    VectorXd *new_D_ptr, *current_D_ptr, new_D(n);
    new_D.setZero();
    D.setOnes();
    new_D_ptr = &new_D;
    current_D_ptr = &D;
    for (size_t l = 0; l < L; ++l) {
        (*new_D_ptr).setZero();
        for (int i = 0; i < n; i++) {
            double dot_sum = 0;
            for (auto &item: A[i]) {
                int j;
                double value;
                tie(j, value) = item;
                if (j != i) {
                    dot_sum += (*current_D_ptr)(j) * value;
                }
            }
            if (A[i].find(i) != A[i].end()) {
                // cout << i << " "  << dot_sum << " " << A.coeff(i,i) << endl;
                auto v = A[i][i];
                // cout <<  str(format(" i: %s, A[i][i]: %s dot_sum: %s ") % i % v % dot_sum) <<  endl;
                (*new_D_ptr)(i) = (1.0 - dot_sum) / v;
            }
        }
        swap(new_D_ptr, current_D_ptr);
    }
    // cout << (A * D) << endl;

}

void CloudWalker::preprocess_F() {
    cout << "computing F" << endl;
    DirectedG::edge_iterator edge_it, edge_end;
    tie(edge_it, edge_end) = edges(*g);
    for (; edge_it != edge_end; ++edge_it) {
        auto s = source(*edge_it, *g);
        auto t = target(*edge_it, *g);
        F[s] += 1.0 / in_degree(t, *g);
    }
}

void CloudWalker::preprocess_hat_P() {
    cout << "computing P" << endl;
    DirectedG::edge_iterator edge_it, edge_end;
    tie(edge_it, edge_end) = edges(*g);
    for (; edge_it != edge_end; ++edge_it) {
        auto s = source(*edge_it, *g);
        auto t = target(*edge_it, *g);
        int in_deg_t = in_degree(t, *g);
        if (in_deg_t > 0) {
            hat_P.coeffRef(t, s) += (1.0 / in_deg_t) / F(s);
        }
    }
    // make hat_P accumulative among each column
    hat_P.makeCompressed();
    auto value_ptr = hat_P.valuePtr();
    auto inner_ptr = hat_P.innerIndexPtr();
    auto outer_ptr = hat_P.outerIndexPtr();
    for (int k = 0; k < hat_P.outerSize(); k++) {
        auto start_ptr = outer_ptr[k];
        auto end_ptr = outer_ptr[k + 1];
        auto col_nnz = end_ptr - start_ptr;
        if (col_nnz > 1) {
            for (int offset = 1; offset < col_nnz; offset++) {
                value_ptr[start_ptr + offset] += value_ptr[start_ptr + offset - 1];
            }
        }
    }
    hat_P.makeCompressed();
}

void CloudWalker::mcss(int i, VectorXd &r) {
    r.setZero();
    auto value_ptr = hat_P.valuePtr();
    auto inner_ptr = hat_P.innerIndexPtr();
    auto outer_ptr = hat_P.outerIndexPtr();

    // uniform generator
    std::default_random_engine generator, generator1;
    std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);

    MatrixXd pos_dist(T + 1, n); // to store the distribution of random walks from i
    Tstep_distribution(i, pos_dist); // T-step random walk distribution
    // cout << "T step distribution: " << endl;
    // cout << pos_dist << endl;
    VectorXd v(n);
    for (size_t t = 0; t <= T; ++t) {
        v.setZero();
        // evaluate Pe_{i}
        v = pos_dist.row(t);
        v = v.cwiseProduct(D);// now v = DPe_{i}

        for (size_t tt = 0; tt < t; ++tt) {
            // compute P^{ Tt}*v
            VectorXd one_step_dist(n);
            one_step_dist.setZero();
            v = v.cwiseProduct(F);
            double w = v.sum();
            v = v / w;
            auto &u = v; // initial distribution
            // set up the distribution initial positions
            discrete_distribution<int> dist(u.data(), u.data() + u.size());
            for (size_t k = 0; k < R_prime; ++k) { // start random walks
                auto init_pos = dist(generator1); // sample a starting node
                // sample out-neighbor, too heavy
                if (out_degree(init_pos, *g) > 0) {
                    /* out-neighbor iterator for weights */
                    // auto out_start = hat_P.valuePtr() + hat_P.outerIndexPtr()[start_pos];
                    // auto out_end = hat_P.valuePtr() + hat_P.outerIndexPtr()[start_pos+1];

                    // discrete_distribution<int> out_dist(out_start, out_end);
                    // auto out_neighbor_index = out_dist(generator2); // sample a out-neighbor index
                    // auto out_neighbor = hat_P.innerIndexPtr()[hat_P.outerIndexPtr()[start_pos]+out_neighbor_index];
                    // sample an out_neighbor
                    double sample_value = uniform_dist(generator);
                    // cout << sample_value << endl;
                    int start_pos = outer_ptr[init_pos];
                    int end_pos = outer_ptr[init_pos + 1];
                    auto value_start_iter = value_ptr + start_pos;
                    auto value_end_iter = value_ptr + end_pos;

                    auto chosen_index_ptr = upper_bound(value_start_iter, value_end_iter, sample_value);
                    int offset = chosen_index_ptr - value_start_iter;
                    int out_neighbor = inner_ptr[start_pos + offset];

                    one_step_dist(out_neighbor) += (1.0 / R_prime);
                }
            }
            // cout << "one step distribution: " << endl;
            // cout << one_step_dist << endl;
            v = w * one_step_dist;
        }
        // cout << "v of iteration: " << t << endl;
        // cout << v << endl;
        r += pow(c, t) * v;
    }
    mem_size = getValue();
}

double CloudWalker::mcsp(int u, int v, MatrixXd &pos_dist_u, MatrixXd &pos_dist_v) {
    auto tmp_start = std::chrono::high_resolution_clock::now();

    Tstep_distribution(u, pos_dist_u); // T-step random walk distribution
    Tstep_distribution(v, pos_dist_v);

    double sim_score = 0.0;
    double cur_decay = 1.0;
    for (auto t = 0; t <= T; t++) {
        sim_score +=
                cur_decay * pos_dist_u.row(t).transpose().cwiseProduct(D).transpose() * pos_dist_v.row(t).transpose();
        cur_decay *= c;
    }
    auto tmp_end = std::chrono::high_resolution_clock::now();
//    cout << "finish single one "
//         << float(std::chrono::duration_cast<std::chrono::microseconds>(tmp_end - tmp_start).count()) / (pow(10, 6))
//         << " s\n";
    return sim_score;
}

double CloudWalker::mcsp(int u, int v) {
    MatrixXd pos_dist_u(T + 1, n);
    MatrixXd pos_dist_v(T + 1, n);

    // 2nd: query
    return mcsp(u, v, pos_dist_u, pos_dist_v);
}




