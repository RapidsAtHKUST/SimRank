#include <fstream>
#include <boost/format.hpp>
#include "local_push_ywangby.h"
#include "../file_serialization.h"
#include <unordered_map>

using boost::format;

double cal_rmax(double c, double epsilon) {
    return (1 - c) * epsilon;
}

double cal_rmax(DirectedG &g, double c, double epsilon, double delta) {
    // calculate r_max
    int m, n;
    m = num_edges(g);
    n = num_vertices(g);
    double d = double(m) / double(n);
    // cout << d << endl;
    double a = (1 - c) * pow(d, 2) * pow(epsilon, 2) / (c * log(2 / delta));
    // cout << pow(a,1.0/3.0) << endl;
    return pow(a, 1.0 / 3.0);
}

// the help check file exists function
bool lp_file_exists(string data_name, double c, double epsilon, size_t n, bool is_full = false) {
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
    if (file_exists(data_path)) {
        return true;
    } else {
        return false;
    }

    delete lp_test;
}


void Full_LocalPush::push(NodePair &pab, double inc) {
    // the actually action of push
    n_push++;
    R[pab] += inc;
    // cout << pab.first << " " << pab.second << endl ;
    if (fabs(R[pab]) > r_max) {
        // Q.insert(pab);
        if (marker[pab] == false) {
            Q.push(pab);
            marker[pab] = true;
        }
    }
}

void Reduced_LocalPush::push(NodePair &pab, double inc) {
    // the actually action of push
    n_push++;
    R[pab] += inc;
    if (fabs(R[pab]) / sqrt(2) > r_max) { // the criteria for reduced linear system
        // Q.insert(pab);
        if (marker[pab] == false) {
            Q.push(pab);
            marker[pab] = true;
        }
    }
}

void LocalPush::init_PR() {
    string data_path = get_file_path_base() + ".P";
    cout << "data path " << data_path << endl;
    if (file_exists(data_path)) { // local push from start
        cout << "file exisis.." << endl;
        load();
    } else { // file exists
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

LocalPush::LocalPush(DirectedG &g, string gName_, double c_, double epsilon_, size_t n_) {
    // init data members
    /* init data memebrs */
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

Full_LocalPush::Full_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) : LocalPush(g, name, c_,
                                                                                                           r_max_, n_) {
    init_PR();
}

Reduced_LocalPush::Reduced_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) : LocalPush(g,
                                                                                                                 name,
                                                                                                                 c_,
                                                                                                                 r_max_,
                                                                                                                 n_) {
    init_PR();
}

void Reduced_LocalPush::push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) {
    // the push method using reduced linear system
    // out-neighbros of a,b
    DirectedG::out_edge_iterator
            outi_iter,
            outi_end,
            outj_iter,
            outj_end;
    bool is_singleton = np.first == np.second ? true : false;
    // /* only push to partial pairs*/
    size_t out_degree_i = out_degree(np.first, g);
    size_t out_degree_j = out_degree(np.second, g);

    tie(outi_iter, outi_end) = out_edges(np.first, g);
    tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator
    // cout << "------" << endl;

    tie(outi_iter, outi_end) = out_edges(np.first, g);
    if (is_singleton) {
        /* starting push for singleton nodes*/
        for (; outi_iter != outi_end; outi_iter++) {
            tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator
            auto out_a = target(*outi_iter, g); // out-neighbor
            for (; outj_iter != outj_end; outj_iter++) {
                auto out_b = target(*outj_iter, g);
                auto indegree_a = in_degree(out_a, g);
                auto indegree_b = in_degree(out_b, g);
                auto total_in = indegree_a * indegree_b;
                if (out_a < out_b) { // only push to partial pairs for a < b
                    NodePair pab(out_a, out_b); // the node-pair to be pushed to
                    double inc = c * current_residual / total_in; //
//                    cout << format("%s push to %s ") % np % pab << endl;
                    push(pab, sqrt(2) * inc); // do the push action, sqrt(2) factor is because it is from singleton node
                    // cout << format("pushing to (%s,%s) with factor %s") % pab.first % pab.second % sqrt(2) << endl;
                }
            }
        }
    } else {
        auto i_begin_iter = outi_iter; // mark the begining iterator
        for (; outi_iter != outi_end; outi_iter++) {
            tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator
            auto j_begin = outj_iter;
            for (; outj_iter != outj_end; outj_iter++) {
                auto out_a = target(*outi_iter, g); // out-neighbor
                auto out_b = target(*outj_iter, g);
                auto indegree_a = in_degree(out_a, g);
                auto indegree_b = in_degree(out_b, g);
                auto total_in = indegree_a * indegree_b;
                double inc = c * current_residual / total_in;
                bool oa_less_ob = out_a < out_b ? true : false;
                if (out_a == out_b) { //don't push to singleton nodes
                    continue;
                } else { //  a, b is normal in-neighbors of out_a and out_b
                    if (!oa_less_ob) {
                        swap(out_a, out_b);
                    }
                    NodePair pab(out_a, out_b);
//                    cout << format("%s push to %s ") % np % pab << endl;
                    push(pab, 1 * inc);
                }
            }
        }
    }
    // cout << "------" << endl;
}

void Full_LocalPush::push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) {
    DirectedG::out_edge_iterator
            outi_iter,
            outi_end,
            outj_iter,
            outj_end;

    tie(outi_iter, outi_end) = out_edges(np.first, g);
    for (; outi_iter != outi_end; outi_iter++) {
        auto out_a = target(*outi_iter, g); // out-neighbor
        tie(outj_iter, outj_end) = out_edges(np.second, g);
        for (; outj_iter != outj_end; outj_iter++) {
            auto out_b = target(*outj_iter, g);
            if (out_a == out_b) {
                continue;
            }
            auto indegree_a = in_degree(out_a, g);
            auto indegree_b = in_degree(out_b, g);
            auto total_in = indegree_a * indegree_b;
            NodePair pab(out_a, out_b);
            double inc = c * current_residual / total_in;
            cout << format("%s push to %s ") % np % pab << endl;
            push(pab, inc);
        }
    }
}

double Full_LocalPush::how_much_residual_to_push(DirectedG &g, NodePair &np) {
    return R[np];
}

double Reduced_LocalPush::how_much_residual_to_push(DirectedG &g, NodePair &np) {
    // determine the residual value for current pair to push
    double r = R[np];
    if (np.first == np.second) { //singleton node
        return r - r_max / (1 - c); // singleton nodes do not need to push all residual as 1
    }
    // /* check whether np forms a self-loop */
    // if(edge(np.first,np.second,g).second == true && edge(np.second, np.first,g).second == true){ // check whether exists reverse edge 
    //     auto in_deg_a = in_degree(np.first,g);
    //     auto in_deg_b = in_degree(np.second,g);
    //     double alpha = c / (in_deg_a * in_deg_b);
    //     int k = ceil( log(r_max / fabs(r)) / log(alpha) );
    //     double residual_to_push=(1 - pow(alpha,k)) * r / (1-alpha);
    //     return residual_to_push;
    // }else{
    //     auto push_residual = r;
    //     return push_residual;
    // }
    return r;

}

void LocalPush::local_push(DirectedG &g) { // local push given current P and R
    // cout << r_max << endl;
    auto start = std::chrono::high_resolution_clock::now();
    double sum_of_est = 0;

    while (!Q.empty()) {
        NodePair np = Q.front();
        Q.pop();
        marker[np] = false;

        // log the P and R information
//        cout << "P: " << endl;
//        P.display();
//        cout << "R: " << endl;
//        R.display();
//        cout << format("pushing on %s") % np << endl;
//        cout << "=======" << endl;



        // cout << "Current: " << np.first << " " << np.second << ": " << R[np] << endl;
        double residual_to_push = how_much_residual_to_push(g, np);
        sum_of_est += residual_to_push;
        // cout << " node pair " << np.first << " " << np.second << endl;
        // R.erase(np); //  remove from residual
        R[np] -= residual_to_push;
        P[np] += residual_to_push;
        push_to_neighbors(g, np, residual_to_push); // push residuals to neighbros of np

    }
//    cout << "final P and R " << endl;
//    cout << "P: " << endl;
//    P.display();
//    cout << "R: " << endl;
//    R.display();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    if (cpu_time == -1) {
        cpu_time = elapsed.count();
        mem_size = getValue();
    }
    // cout << "n: " << n << " m: " << num_edges(g) << " r_max: " << r_max << " time: " << elapsed.count() <<"s"<< endl;
    // cout << sum_of_est * 2 - n << endl;
    cout << "number of push: " << n_push << endl;
    // save the time to files

    // save time info to file
    // string time_file_name = get_file_path_base() + ".txt"; // the time file
    // fstream time_file(time_file_name, ios::out);
    // cout <<"timing " << "saving to " << time_file_name << endl;
    // time_file << elapsed.count() << endl;
    // time_file.close();
    return;
}

void LocalPush::save() {
    // save data to disk
    // save P
    string p_path = get_file_path_base() + ".P";
    P.save(p_path);
    // FILE *outP = fopen(p_path.c_str(), "wb");
    // cout << "saving P to " << p_path << endl;
    // P.serialize(FileSerializer(), outP);
    // fclose(outP);
    //save R
    string r_path = get_file_path_base() + ".R";
    R.save(r_path);
    // FILE *outR = fopen(r_path.c_str(), "wb");
    // cout << "saving R to " << r_path << endl;
    // R.serialize(FileSerializer(), outR);
    // fclose(outR);

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

    // // save the experimental data
    // string exp_path =get_file_path_base()+".exp";
    // out.open(exp_path);
    // out << n << endl;
    // cout << c << endl;
    // cout << epsilon << endl;
    // out.close();
    // save exp data
    cout << "save complete" << endl;

}

void LocalPush::load() {
    // load existing local push data
    // load P
    string p_path = get_file_path_base() + ".P";
    P.load(p_path);
    // FILE *inP = fopen(p_path.c_str(), "rb");
    // cout << "loading P to " << p_path << endl;
    // P.unserialize(FileSerializer(), inP);
    // fclose(inP);
    //load R
    string r_path = get_file_path_base() + ".R";
    R.load(r_path);
    // FILE *inR = fopen(r_path.c_str(), "rb");
    // cout << "loading R to " << r_path << endl;
    // R.unserialize(FileSerializer(), inR);
    // fclose(inR);

    string meta_path = get_file_path_base() + ".meta";
    ifstream in;
    in.open(meta_path);
    in >> g_name >> n >> epsilon >> c >> cpu_time >> mem_size;
    r_max = cal_rmax(c, epsilon);
    marker.add(n); // initialize marker
    in.close();
}

void LocalPush::show() {
    // cout << P.size() << endl;
    // for(auto &item:P){
    //     cout << "(" << item.first.first << "," << item.first.second << "): " << item.second << endl;
    // }
}

string Reduced_LocalPush::get_file_path_base() {
    // return the file path, exluding the suffix
    return LOCAL_PUSH_DIR + str(format("RLP_%s-%.3f-%.6f") %
                                g_name % c % epsilon);
}

string Full_LocalPush::get_file_path_base() {
    // return the file path, exluding the suffix
    return LOCAL_PUSH_DIR + str(format("FLP_%s-%.3f-%.6f") %
                                g_name % c % epsilon);
}

double Reduced_LocalPush::query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    if (a == b) {
        return P[NodePair(a, b)];
    } else if (a > b) {
        return P[NodePair(b, a)];
    } else {
        return P[NodePair(a, b)];
    }
}

double Reduced_LocalPush::query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    if (a == b) {
        return R[NodePair(a, b)];
    } else if (a > b) {
        return R[NodePair(b, a)];
    } else {
        return R[NodePair(a, b)];
    }
}

double Full_LocalPush::query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    return P[NodePair(a, b)];
}

double Full_LocalPush::query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    return R[NodePair(a, b)];
}

void Reduced_LocalPush::update_residuals_by_deleting_edge(DirectedG &g, DirectedG::vertex_descriptor u,
                                                          DirectedG::vertex_descriptor v) {
    // udpate residual when an edge (u,v) is removed from G
    // assumption: G has not been updated, (u,v) exists in G
    // a: the iterated node over G
    // v: the node whose in-neighbor is changed
    DirectedG::vertex_iterator v_begin, v_end, v_it;
    tie(v_begin, v_end) = vertices(g);
    double updated_R = 0;
    for (v_it = v_begin; v_it != v_end; v_it++) {
        auto a = *v_it;
        if (in_degree(a, g) == 0) {
            continue;
        }
        if (a == v) {
            // singleton node: do nothing
        } else {
            bool a_smaller_v = a < v ? true : false;
            NodePair np; // the node pair to be updated
            if (a_smaller_v) {
                np = NodePair(a, v);
            } else {
                np = NodePair(v, a);
            }
            DirectedG::in_edge_iterator in_a_iter, in_v_iter, in_a_begin, in_a_end, in_v_begin, in_v_end;
            double P_av = P[np]; // current P(a,v)
            double R_av = R[np]; // current R(a,v)
            //iterate over a's in-neighbors
            double updated_R = P_av + R_av;
            if (in_degree(v, g) == 1) {
                // u is the only in-neighbor of v
                updated_R = -P_av;
            } else {
                tie(in_a_begin, in_a_end) = in_edges(a, g);
                double u_contrib = 0;
                for (in_a_iter = in_a_begin; in_a_iter != in_a_end; in_a_iter++) {
                    auto a_prime = source(*in_a_iter, g);
                    double contrib = 0;
                    if (a_prime > u) {
                        contrib = c * P[NodePair(u, a_prime)];
                    } else if (a_prime < u) {
                        contrib = c * P[NodePair(a_prime, u)];
                    } else if (a_prime == u) {
                        contrib = c * sqrt(2) * P[NodePair(a_prime, u)];
                        // cout << "singleton node: " << P[NodePair(a_prime,u)] << endl;
                    }
                    u_contrib += contrib;
                }
                u_contrib = u_contrib / (in_degree(v, g) * in_degree(a, g));
                updated_R = updated_R - u_contrib;
                updated_R = updated_R * ((in_degree(v, g)) / (in_degree(v, g) - 1.0));
                updated_R -= P_av;
            }
            R[np] = updated_R;
            // cout << format("our computed R(%s,%s): %s") % np.first % np.second % updated_R << endl;
            // cout << "estimated u contribute: " << u_contrib << endl;
            // cout << "scaled rest: " << scaled_rest << endl;
            // cout << "---------" << endl;
            if (fabs(R[np]) / sqrt(2) > r_max) {
                if (marker[np] == false) {
                    // cout << np << "is pushed into Q" << endl;
                    Q.push(np);
                    marker[np] = true;
                }
            }
        }
    }
}


void Reduced_LocalPush::update_residuals_by_adding_edge(DirectedG &g, DirectedG::vertex_descriptor u,
                                                        DirectedG::vertex_descriptor v) {
    // update the residual when an edge (u,v) is inserted to g
    // node a: the iterated node over G
    // node v: whose in-neighbor has changed (u added)
    // assumption: g has not been updated, (u,v) does not exist in g
    DirectedG::vertex_iterator v_begin, v_end, v_it;
    tie(v_begin, v_end) = vertices(g);
    double updated_R = 0;
    for (v_it = v_begin; v_it != v_end; v_it++) { // ignore the starting node 0
        auto a = *v_it;
        if (in_degree(a, g) == 0) { // a has no in-neighbor, do nothing
            continue;
        }
        if (a == v) {
            // do nothing, no need to update singleton nodes
        } else {
            double u_contrib = 0; // the amout of R^{'} related to u
            double scaled_rest;
            bool a_smaller_v = a < v ? true : false;
            double P_av, R_av;
            double new_R = 0;
            NodePair np; // the node pair to be updated
            if (a_smaller_v) {
                np = NodePair(a, v);
            } else {
                np = NodePair(v, a);
            }
//            cout << format("Pnp %s Rnp %s") % P[np] % R[np] << endl;

            // update the residual of (a,v) using the naive way
            // add_edge(u,v,g);
            // double corrected_u_contrib = 0;
            DirectedG::in_edge_iterator in_a_iter, in_v_iter, in_a_begin, in_a_end, in_v_begin, in_v_end;
            // tie(in_a_begin, in_a_end) = in_edges(a,g);
            // tie(in_v_begin, in_v_end) = in_edges(v,g);
            // for(in_a_iter = in_a_begin; in_a_iter != in_a_end; in_a_iter ++){
            //     for(in_v_iter = in_v_begin; in_v_iter != in_v_end; in_v_iter++){
            //         auto a_prime = source(*in_a_iter,g);
            //         auto v_prime = source(*in_v_iter,g);
            //         double inc = 0;
            //         if(a_prime == v_prime){
            //             inc = c * sqrt(2) * P[NodePair(a_prime, v_prime)];
            //         }else if(a_prime < v_prime){
            //              inc = c * P[NodePair(a_prime, v_prime)];
            //         }else{
            //             inc = c * P[NodePair(v_prime, a_prime)];
            //         }
            //         new_R += inc;
            //         if(v_prime == u){
            //             corrected_u_contrib += inc;
            //         }
            //     }

            // }
            // new_R = new_R / (in_degree(a,g) * in_degree(v,g));
            // new_R = new_R - P[np];
            // remove_edge(u,v,g);
            // cout << format("corrected new R(%s,%s): %s") % np.first % np.second % new_R <<  endl;
            // cout << format("corrected u contribute: %s") % corrected_u_contrib << endl;
            // cout << format("corrected scaled rest: %s") % (new_R - corrected_u_contrib) << endl;

            // udpate R use the advanced method
            P_av = P[np]; // current P(a,v)
            R_av = R[np]; // current R(a,v)
            scaled_rest = (in_degree(v, g) / (in_degree(v, g) + 1.0)) * (R[np] + P[np]);

            // iterate over a's in-neighbors
            tie(in_a_begin, in_a_end) = in_edges(a, g);
            for (in_a_iter = in_a_begin; in_a_iter != in_a_end; in_a_iter++) {
                auto a_prime = source(*in_a_iter, g);
                double contrib = 0;
                if (a_prime > u) {
                    contrib = c * P[NodePair(u, a_prime)];
                } else if (a_prime < u) {
                    contrib = c * P[NodePair(a_prime, u)];
                } else if (a_prime == u) {
                    contrib = c * sqrt(2) * P[NodePair(a_prime, u)];
                    // cout << "singleton node: " << P[NodePair(a_prime,u)] << endl;
                }
                u_contrib += contrib;
            }
            u_contrib = u_contrib / ((in_degree(v, g) + 1.0) * in_degree(a, g));

            updated_R = u_contrib + scaled_rest - P[np];

            R[np] = updated_R;
            // cout << format("our computed R(%s,%s): %s") % np.first % np.second % updated_R << endl;
            // cout << "estimated u contribute: " << u_contrib << endl;
            // cout << "scaled rest: " << scaled_rest << endl;
            // cout << "---------" << endl;
            if (fabs(R[np]) / sqrt(2) > r_max) {
                if (marker[np] == false) {
                    // cout << np << "is pushed into Q" << endl;
                    Q.push(np);
                    marker[np] = true;
                }
            }
        }
    }
}

void Reduced_LocalPush::update_edges(DirectedG &g, vector<NodePair> edges, char update_type = '+') {
    // update G by a set of edges
    if (update_type == '+') {
        for (auto &e: edges) {
            int s = e.first;
            int t = e.second;
            update_residuals_by_adding_edge(g, s, t);
            add_edge(s, t, g);
        }
    } else if (update_type == '-') {
        for (auto &e: edges) {
            int s = e.first;
            int t = e.second;

            if (edge(s, t, g).second) {
                update_residuals_by_deleting_edge(g, s, t);
                remove_edge(s, t, g);
            }
        }
    } else {
        cout << "Please indicate the edge update type" << endl;
        return;
    }
    Reduced_LocalPush::local_push(g);
    cout << "size of Q: " << Q.size() << endl;
}







