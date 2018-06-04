#include <fstream>

#include <boost/format.hpp>

#include "local_push_yche.h"

using boost::format;

bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full) {
    LocalPush *lp_test;
    if (is_full) {
        lp_test = new Full_LocalPush();
    } else {
        lp_test = new Reduced_LocalPush();
    }
    // Reduced_LocalPush lp_test;
    lp_test->g_name = data_name;
    lp_test->c = c;
    lp_test->epsilon = epsilon;
    string data_path = lp_test->get_file_path_base() + ".P";
    cout << "data path !!!!" << data_path << endl;
    delete lp_test;

    return file_exists(data_path);
}

double cal_rmax(double c, double epsilon) {
    return (1 - c) * epsilon;
}

double cal_rmax(GraphYche &g, double c, double epsilon, double delta) {
    // calculate r_max
    int m, n;
    m = g.m;
    n = g.n;
    double d = double(m) / double(n);
    double a = (1 - c) * pow(d, 2) * pow(epsilon, 2) / (c * log(2 / delta));
    return pow(a, 1.0 / 3.0);
}

LocalPush::LocalPush(GraphYche &g, string gName_, double c_, double epsilon_, size_t n_) {
    // init data members
    max_q_size = 0;

    g_name = gName_;
    c = c_;
    epsilon = epsilon_;
    r_max = cal_rmax(c, epsilon);
    n = n_;
    string path = get_edge_list_path(gName_);
    n_push = 0;
    cpu_time = -1; // set the init value
}

void LocalPush::init_PR() {
    string data_path = get_file_path_base() + ".P";
    cout << "data path " << data_path << endl;

    P.add(n);
    R.add(n);
    marker.add(n);

    cout << "file not exists, compute from scratch" << endl;
    for (int i = 0; i < n; i++) {
        NodePair np(i, i);
        R[np] = 1;
        Q.push(np);
//        Q.push_back(np);
        marker[np] = true;
    }
}

Full_LocalPush::Full_LocalPush(GraphYche &g, string name, double c_, double epsilon, size_t n_) :
        LocalPush(g, name, c_, epsilon, n_) {
    string data_path = get_file_path_base() + ".P";
    init_PR();
}

//void LocalPush::local_push(GraphYche &g) { // local push given current P and R
//    auto start = std::chrono::high_resolution_clock::now();
//
//    while (!Q.empty()) {
//        max_q_size = max(max_q_size, Q.size());
//        NodePair np = Q.front();
//        Q.pop();
//        marker[np] = false;
//        double residual_to_push = how_much_residual_to_push(g, np);
//
//        R[np] -= residual_to_push;
//        P[np] += residual_to_push;
//        push_to_neighbors(g, np, residual_to_push); // push residuals to neighbors of np
//    }
//
//    auto finish = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double> elapsed = finish - start;
//    if (cpu_time == -1) {
//        cpu_time = elapsed.count();
//        mem_size = getValue();
//    }
//}

void Full_LocalPush::push(NodePair &pab, double inc) {
    n_push++;
    // only probing once
    auto &res_ref = R[pab];
    res_ref += inc;
    if (fabs(res_ref) > r_max) {
        auto &is_in_q_flag_ref = marker[pab];
        if (!is_in_q_flag_ref) {
            Q.push(pab);
//            Q.push_back(pab);
            is_in_q_flag_ref = true;
        }
    }
}

double Full_LocalPush::how_much_residual_to_push(GraphYche &g, NodePair &np) {
    return R[np];
}

void Full_LocalPush::push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {
    auto a = np.first;
    auto b = np.second;
    for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
        auto out_nei_a = g.neighbors_out[off_a];
        for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
            auto out_nei_b = g.neighbors_out[off_b];
            if (out_nei_a == out_nei_b) {
                continue;
            }
            auto in_degree_a = g.in_degree(out_nei_a);
            auto in_degree_b = g.in_degree(out_nei_b);
            auto total_in = in_degree_a * in_degree_b;
            NodePair pab(out_nei_a, out_nei_b);
            double inc = c * current_residual / total_in;
            push(pab, inc);
        }
    }
}

void LocalPush::save() {
    // save data to disk
    // save P
    string p_path = get_file_path_base() + ".P";
    P.save(p_path);

    //save R
    string r_path = get_file_path_base() + ".R";
    R.save(r_path);

    ofstream out;
    // // save meta info
    string meta_path = get_file_path_base() + ".meta";
    out.open(meta_path);
    out << g_name << endl;
    out << n << endl;
    out << epsilon << endl;
    out << c << endl;
    out << cpu_time << endl;
    out << mem_size << endl;
    auto p_size = P.size();
    auto r_size = R.size();
    out << p_size << endl;
    out << r_size << endl;
    out << double(p_size) / (n * n) << endl;
    out << double(r_size) / (n * n) << endl;
    out.close();

    // save exp data
    cout << "save complete" << endl;
}

void LocalPush::load() {
    // load existing local push data
    // load P
    string p_path = get_file_path_base() + ".P";
    P.load(p_path);

    //load R
    string r_path = get_file_path_base() + ".R";
    R.load(r_path);

    string meta_path = get_file_path_base() + ".meta";
    ifstream in;
    in.open(meta_path);
    in >> g_name >> n >> epsilon >> c >> cpu_time >> mem_size;
    r_max = cal_rmax(c, epsilon);
    marker.add(n); // initialize marker
    in.close();
}

string Full_LocalPush::get_file_path_base() {
    return LOCAL_PUSH_DIR + str(format("FLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}

double Full_LocalPush::query_P(int a, int b) {
    return P.query(a, b);
}

void Full_LocalPush::local_push(GraphYche &g) {
    auto start = std::chrono::high_resolution_clock::now();

    while (!Q.empty()) {
        max_q_size = max(max_q_size, Q.size());
        NodePair np = Q.front();
        Q.pop();

//        NodePair np = Q.back();
//        Q.pop_back();
        marker[np] = false;

        auto &R_np_ref = R[np];
        double residual_to_push = R_np_ref;

        R_np_ref -= residual_to_push;
        P[np] += residual_to_push;

        auto a = np.first;
        auto b = np.second;

        // push to neighbors
        for (auto off_a = g.off_out[a]; off_a < g.off_out[a + 1]; off_a++) {
            auto out_nei_a = g.neighbors_out[off_a];
            for (auto off_b = g.off_out[b]; off_b < g.off_out[b + 1]; off_b++) {
                auto out_nei_b = g.neighbors_out[off_b];
                if (out_nei_a == out_nei_b) {
                    continue;
                }
                auto in_degree_a = g.in_degree(out_nei_a);
                auto in_degree_b = g.in_degree(out_nei_b);
                auto total_in = in_degree_a * in_degree_b;
                NodePair pab(out_nei_a, out_nei_b);
                double inc = c * residual_to_push / total_in;

                // push(pab, inc);
                n_push++;
                // only probing once
                auto &res_ref = R[pab];
                res_ref += inc;
                if (fabs(res_ref) > r_max) {
                    auto &is_in_q_flag_ref = marker[pab];
                    if (!is_in_q_flag_ref) {
                        Q.push(pab);
//                        Q.push_back(pab);
                        is_in_q_flag_ref = true;
                    }
                }
            }
        }
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    if (cpu_time == -1) {
        cpu_time = elapsed.count();
        mem_size = getValue();
    }
}

double LocalPush::query_R(int a, int b) {
    if (a > b) {
        return R.query(b, a);
    } else {
        return R.query(a, b);
    }
}