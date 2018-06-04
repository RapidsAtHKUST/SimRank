#include <fstream>

#include <boost/format.hpp>

#include "local_push_yche.h"

using boost::format;

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

void LocalPush::push(NodePair &pab, double inc) {
    // the actually action of push
    n_push++;
    R[pab] += inc;
    if (fabs(R[pab]) > r_max) {
        // Q.insert(pab);
        if (marker[pab] == false) {
            Q.push(pab);
            marker[pab] = true;
        }
    }
}

LocalPush::LocalPush(GraphYche &g, string gName_, double c_, double epsilon_, size_t n_) {
    // init data members
    g_name = gName_;
    c = c_;
    // r_max = r_max_;
    epsilon = epsilon_;
    r_max = cal_rmax(c, epsilon);
    n = n_;
    string path = get_edge_list_path(gName_);
    n_push = 0;
    cpu_time = -1; // set the init value
}

Full_LocalPush::Full_LocalPush(GraphYche &g, string name, double c_, double r_max_, size_t n_) :
        LocalPush(g, name, c_, r_max_, n_) {
    string data_path = get_file_path_base() + ".P";
    cout << "data path " << data_path << endl;
//    if (file_exists(data_path)) { // local push from start
//        cout << "file exisis.." << endl;
//        load();
//    } else { // file exists
    if (!file_exists(data_path)) {
        /* init the P and R */
        P.add(n);
        R.add(n);
        marker.add(n);

        cout << "file not exists, compute from scratch" << endl;
        for (int i = 0; i < n; i++) {
            NodePair np(i, i);
            // R.insert({np,1});
            R[np] = 1;
            Q.push(np);
            marker[np] = true;
        }
    }
}

Reduced_LocalPush::Reduced_LocalPush(GraphYche &g, string name, double c_, double r_max_, size_t n_) :
        LocalPush(g, name, c_, r_max_, n_) {
    string data_path = get_file_path_base() + ".P";
//    if (file_exists(data_path)) { // local push from start
//        load();
//    } else { // file exists
    if (!file_exists(data_path)) {
        /* init the P and R */
        P.add(n);
        R.add(n);
        marker.add(n);

        cout << "file not exists, compute from scratch!!!" << endl;
        for (int i = 0; i < n; i++) {
            NodePair np(i, i);
            // R.insert({np,1});
            R[np] = 1;
            Q.push(np);
            marker[np] = true;
        }
    }
}

void Reduced_LocalPush::push_to_neighbors(GraphYche &g, NodePair &np, double current_residual) {
    // the push method using reduced linear system
    // out-neighbors of a,b
    auto a = np.first;
    auto b = np.second;
    bool is_singleton = (a == b);

    // /* only push to partial pairs*/
    auto out_degree_i = static_cast<size_t>(g.out_degree(a));
    auto out_degree_j = static_cast<size_t>(g.out_degree(b));

    /*the indicator whether the position is common neighbor */
    vector<bool> outs_i_common(out_degree_i, false);
    vector<bool> outs_j_common(out_degree_j, false);

    if (is_singleton) {
        /* starting push for singleton nodes*/
        for (auto off_out_a = g.off_out[a]; off_out_a < g.off_out[a + 1]; off_out_a++) {
            auto out_nei_a = g.neighbors_out[off_out_a];
            for (auto off_out_b = g.off_out[b]; off_out_b < g.off_out[b + 1]; off_out_b++) {
                auto out_nei_b = g.neighbors_out[off_out_b];
                auto in_degree_a = g.in_degree(out_nei_a);
                auto in_degree_b = g.in_degree(out_nei_b);
                auto total_in = in_degree_a * in_degree_b;
                if (out_nei_a < out_nei_b) { // only push to partial pairs for a < b
                    NodePair pab(out_nei_a, out_nei_b); // the node-pair to be pushed to
                    double inc = c * current_residual / total_in;
                    push(pab, inc); // do the push action
                }
            }
        }
    } else {
        /* mark the common neighbors */
        const auto off_beg_a = g.off_out[a];
        const auto off_beg_b = g.off_out[b];
        for (auto off_a = off_beg_a; off_a < g.off_out[a + 1]; off_a++) {
            auto out_nei_a = g.neighbors_out[off_a];
            for (auto off_b = off_beg_b; off_b < g.off_out[b + 1]; off_b++) {
                auto out_nei_b = g.neighbors_out[off_b];
                if (out_nei_a == out_nei_b) {
                    outs_i_common[off_a - off_beg_a] = true;
                    outs_j_common[off_b - off_beg_b] = true;
                    break;
                }
            }
        }

        /* starting push for non-singleton nodes*/
        for (auto off_a = off_beg_a; off_a < g.off_out[a + 1]; off_a++) {
            auto is_i_common = outs_i_common[off_a - off_beg_a];

            for (auto off_b = off_beg_b; off_b < g.off_out[b + 1]; off_b++) {
                auto is_j_common = outs_j_common[off_b - off_beg_b];
                // put out_nei_a here, since swap happens later
                auto out_nei_a = g.neighbors_out[off_a];
                auto out_nei_b = g.neighbors_out[off_b];

                auto in_degree_a = g.in_degree(out_nei_a);
                auto in_degree_b = g.in_degree(out_nei_b);
                auto total_in = in_degree_a * in_degree_b;
                double inc = c * current_residual / total_in;
                if (out_nei_a == out_nei_b) { //don't push to singleton nodes
                    continue;
                }
                bool oa_less_ob = (out_nei_a < out_nei_b);
                if (!oa_less_ob) {
                    swap(out_nei_a, out_nei_b);
                }

                NodePair pab(out_nei_a, out_nei_b);
                if (!is_i_common) {
                    // i is not common neighbor
                    push(pab, inc);
                } else {
                    // i is a common neighbor
                    if (is_j_common) {
                        if (oa_less_ob) {
                            push(pab, 2 * inc); // push twice for two commons
                        }
                    } else {// normal case
                        push(pab, inc);
                    }
                }
            }
        }
    }
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

double Full_LocalPush::how_much_residual_to_push(GraphYche &g, NodePair &np) {
    return R[np];
}

double Reduced_LocalPush::how_much_residual_to_push(GraphYche &g, NodePair &np) {
    // determine the residual value for current pair to push
    double r = R[np];
    if (r > 1.01) {
        cout << "err" << endl;
        exit(0);
    }
    if (np.first == np.second) { //singleton node
        return r - r_max / (1 - c); // singleton nodes do not need to push all residual as 1
        // return r;
        // return 1;
    }
    /* check whether np forms a self-loop */
    if (g.exists_edge(np.first, np.second) &&
        g.exists_edge(np.second, np.first)) { // check whether exists reverse edge
        auto in_deg_a = g.in_degree(np.first);
        auto in_deg_b = g.in_degree(np.second);
        double alpha = c / (in_deg_a * in_deg_b);
        int k = ceil(log(r_max / fabs(r)) / log(alpha));
        double residual_to_push = (1 - pow(alpha, k)) * r / (1 - alpha);
        return residual_to_push;
    } else {
        auto push_residual = r;
        return push_residual;
    }
}

void LocalPush::local_push(GraphYche &g) { // local push given current P and R
    // cout << r_max << endl;
    auto start = std::chrono::high_resolution_clock::now();
    double sum_of_est = 0;

    while (!Q.empty()) {
        NodePair np = Q.front();
        Q.pop();
        marker[np] = false;
        double residual_to_push = how_much_residual_to_push(g, np);
        sum_of_est += residual_to_push;
//        cout << residual_to_push << endl;

        // R.erase(np); //  remove from residual
        R[np] -= residual_to_push;
        P[np] += residual_to_push;
        push_to_neighbors(g, np, residual_to_push); // push residuals to neighbros of np

    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    if (cpu_time == -1) {
        cpu_time = elapsed.count();
        mem_size = getValue();
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

string Reduced_LocalPush::get_file_path_base() {
    // return the file path, exluding the suffix
    return LOCAL_PUSH_DIR + str(format("RLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}

double Reduced_LocalPush::query_P(int a, int b) {
    if (a > b) {
        return P.query(b, a);
    } else {
        return P.query(a, b);
    }
}

double Reduced_LocalPush::query_R(int a, int b) {
    if (a > b) {
        return R.query(b, a);
    } else {
        return R.query(a, b);
    }
}

string Full_LocalPush::get_file_path_base() {
    // return the file path, exluding the suffix
    return LOCAL_PUSH_DIR + str(format("FLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}

double Full_LocalPush::query_P(int a, int b) {
    return P.query(a, b);
}

double Full_LocalPush::query_R(int a, int b) {
    return R.query(a, b);
}