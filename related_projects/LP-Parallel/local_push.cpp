#include <fstream>
#include <unordered_map>

#include <boost/format.hpp>

#include "local_push.h"

using boost::format;

double cal_rmax(double c, double epsilon) {
    return (1 - c) * epsilon;
}

void Full_LocalPush::push(NodePair &pab, double inc) {
    // the actually action of push
    n_push++;
    R[pab] += inc;
    // cout << pab.first << " " << pab.second << endl ;
    if (fabs(R[pab]) > r_max) {
        // Q.insert(pab);
        if (!marker[pab]) {
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
        if (!marker[pab]) {
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

Full_LocalPush::Full_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) :
        LocalPush(g, name, c_, r_max_, n_) {
    init_PR();
}

Reduced_LocalPush::Reduced_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) :
        LocalPush(g, name, c_, r_max_, n_) {
    init_PR();
}

void Reduced_LocalPush::push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) {
    // the push method using reduced linear system
    // out-neighbors of a,b
    DirectedG::out_edge_iterator outi_iter, outi_end, outj_iter, outj_end;
    // /* only push to partial pairs*/

    tie(outi_iter, outi_end) = out_edges(np.first, g);
    tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator

    tie(outi_iter, outi_end) = out_edges(np.first, g);
    if (np.first == np.second) {
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
                    push(pab, sqrt(2) * inc); // do the push action, sqrt(2) factor is because it is from singleton node
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
                if (out_a == out_b) { //don't push to singleton nodes
                    continue;
                } else { //  a, b is normal in-neighbors of out_a and out_b
                    if (out_a > out_b) {
                        swap(out_a, out_b);
                    }
                    NodePair pab(out_a, out_b);
                    push(pab, 1 * inc);
                }
            }
        }
    }
}

void Full_LocalPush::push_to_neighbors(DirectedG &g, NodePair &np, double current_residual) {
    DirectedG::out_edge_iterator outi_iter, outi_end, outj_iter, outj_end;

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
        // cout << "Current: " << np.first << " " << np.second << ": " << R[np] << endl;
        double residual_to_push = how_much_residual_to_push(g, np);
        sum_of_est += residual_to_push;
        // cout << " node pair " << np.first << " " << np.second << endl;
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
    // cout << "n: " << n << " m: " << num_edges(g) << " r_max: " << r_max << " time: " << elapsed.count() <<"s"<< endl;
    // cout << sum_of_est * 2 - n << endl;
    // cout << "number of push: " << n_push << endl;
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

void LocalPush::insert(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v,
                       DirectedG &g) {
    // insert edge(u,v) to g, noted we assume g is already updated, and we just update P and R
    DirectedG::vertex_iterator v_it, v_end;
    tie(v_it, v_end) = vertices(g);
    for (; v_it != v_end; v_it++) {
        auto a = *v_it;
        auto in_deg_v = in_degree(v, g); // the new degree of v
        if (a > v) {
            update_residual(g, v, a);
        } else if (a < v) {
            update_residual(g, a, v);
        } else {
            // a == v
            R[NodePair(a, a)] = 1 - P[NodePair(a, a)];
        }
    }
    local_push(g);
}

void LocalPush::remove(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v,
                       DirectedG &g) {
    // remove edge(a,b) to g, noted we assume g is already updated, and we just update P and R
}

double Reduced_LocalPush::query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    if (a == b) {
        return P[NodePair(a, b)];
    } else if (a > b) {
        return P[NodePair(b, a)] / sqrt(2);
    } else {
        return P[NodePair(a, b)] / sqrt(2);
    }
}

double Full_LocalPush::query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    return P[NodePair(a, b)];
}

double LocalPush::query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    if (a > b) {
        return R[NodePair(b, a)];
    } else {
        return R[NodePair(a, b)];
    }
}

void Reduced_LocalPush::update_residual(DirectedG &g, DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    // cout << "-----" << endl;
    // cout << "updating residual "<< a << " " << b << endl;
    NodePair np(a, b); // a < b
    PairMarker indicator; // the indicator of in-neighbors of (a,b)
    DirectedG::in_edge_iterator in_a_it, in_a_end, in_b_it, in_b_end;
    tie(in_a_it, in_a_end) = in_edges(a, g);
    auto in_deg_a = in_degree(a, g);
    auto in_deg_b = in_degree(b, g);
    if (in_deg_a * in_deg_b > 0) {
        double sum_neighbor_residuals = 0;
        for (; in_a_it != in_a_end; in_a_it++) {
            tie(in_b_it, in_b_end) = in_edges(b, g);
            for (; in_b_it != in_b_end; in_b_it++) {
                auto ina = source(*in_a_it, g);
                auto inb = source(*in_b_it, g);
                // indicator[NodePair(min(ina,inb), max(ina,inb))] = true;
                sum_neighbor_residuals += P[NodePair(min(ina, inb), max(ina, inb))];
            }
        }
        // for(auto& item:indicator){
        //     // cout << "collect neighbor P (" << item.first.first 
        //     //     << "," << item.first.second <<"): " << P[item.first]  << endl;
        //     sum_neighbor_residuals += P[item.first]; // item.first is the node pair
        // }
        R[np] = c * sum_neighbor_residuals / (in_deg_a * in_deg_b) - P[np];
        // cout << "neighbros residuals " << sum_neighbor_residuals << endl;
        // cout << " new residual " << R[np] <<  " its current estimates " << P[np] << endl;
        if (fabs(R[NodePair(a, b)]) > r_max) {
            if (marker[np] == false) {
                Q.push(np);
                marker[np] = true;
            }
        }
    }
}



