#include <fstream>
#include <unordered_map>

#include <boost/format.hpp>

#include "local_push.h"
#include "file_serialization.h"

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

void LocalPush::push(NodePair &pab, double inc) {
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


    // check the local puah exits
    // string data_path =get_file_path_base() + ".P";
    // cout << "data path " << data_path << endl;
    // if(file_exists(data_path)){ // local push from start
    //     cout << "file exisis.." << endl;
    //     load();
    // }else{ // file exists
    //     /* init the P and R */
    //     cout << "file not exists, compute from scratch" << endl;
    //     for(int i=0;i<n;i++){
    //         NodePair np(i,i);
    //         R.insert({np,1});
    //         Q.push(np);
    //         marker[np] = true;
    //     }
    // }
}

Full_LocalPush::Full_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) : LocalPush(g, name, c_,
                                                                                                           r_max_, n_) {
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

Reduced_LocalPush::Reduced_LocalPush(DirectedG &g, string name, double c_, double r_max_, size_t n_) : LocalPush(g,
                                                                                                                 name,
                                                                                                                 c_,
                                                                                                                 r_max_,
                                                                                                                 n_) {
    string data_path = get_file_path_base() + ".P";
    // cout << "data path " << data_path << endl;
    if (file_exists(data_path)) { // local push from start
        // cout << "file exisis.." << endl;
        load();
        // cout << g_name << " " << r_max << " " << c << " " << n << endl;
    } else { // file exists
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
    /*the indicator whether the position is common neighbor */
    vector<bool> outs_i_common(out_degree_i, false);
    vector<bool> outs_j_common(out_degree_j, false);
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
                    double inc = c * current_residual / total_in;
                    push(pab, inc); // do the push action
                }
            }
        }
    } else {
        /* mark the common neighbors */
        size_t i, j;
        for (i = 0; i < out_degree_i; i++) {
            auto a = target(*(outi_iter + i), g);
            for (j = 0; j < out_degree_j; j++) {
                auto b = target(*(outj_iter + j), g);
                if (a == b) {
                    outs_i_common[i] = true;
                    outs_j_common[j] = true;
                    break;
                }
            }
        }
        /* starting push for non-singleton nodes*/
        auto i_begin_iter = outi_iter; // mark the begining iterator
        for (; outi_iter != outi_end; outi_iter++) {
            tie(outj_iter, outj_end) = out_edges(np.second, g);// init the iterator
            auto j_begin = outj_iter;
            bool is_i_common = outs_i_common[outi_iter - i_begin_iter];// indicator of whether i is a common neighbor
            for (; outj_iter != outj_end; outj_iter++) {
                bool is_j_common = outs_j_common[outj_iter - j_begin];
                auto out_a = target(*outi_iter, g); // out-neighbor
                auto out_b = target(*outj_iter, g);
                auto indegree_a = in_degree(out_a, g);
                auto indegree_b = in_degree(out_b, g);
                auto total_in = indegree_a * indegree_b;
                double inc = c * current_residual / total_in;
                if (out_a == out_b) { //don't push to singleton nodes
                    continue;
                }
                bool oa_less_ob = out_a < out_b ? true : false;
                // cout << "i com :" << is_i_common << " j com: " << is_j_common << endl;
                if (!oa_less_ob) {
                    swap(out_a, out_b);
                }
                NodePair pab(out_a, out_b);
                if (!is_i_common) {
                    // i is not common neighbonr
                    push(pab, inc);
                } else {
                    // i is a common neighbor
                    if (is_j_common) {
                        if (oa_less_ob) {
                            push(pab, 2 * inc); // push twice for two commons
                        }
                    } else {// notmal case
                        push(pab, inc);
                    }
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
        // return r;
        // return 1;
    }
    /* check whether np forms a self-loop */
    if (edge(np.first, np.second, g).second == true &&
        edge(np.second, np.first, g).second == true) { // check whether exists reverse edge
        auto in_deg_a = in_degree(np.first, g);
        auto in_deg_b = in_degree(np.second, g);
        double alpha = c / (in_deg_a * in_deg_b);
        int k = ceil(log(r_max / fabs(r)) / log(alpha));
        double residual_to_push = (1 - pow(alpha, k)) * r / (1 - alpha);
        return residual_to_push;
    } else {
        auto push_residual = r;
        return push_residual;
        // optimize for neighbor-loop
        // double current_max = 0;
        // double max_m = 0;
        // double alpha_i;
        // double alpha_0;
        // DirectedG::out_edge_iterator out_a_it, out_a_end;
        // DirectedG::out_edge_iterator out_b_it, out_b_end;
        // alpha_0 = c / (in_degree(np.first,g) * in_degree(np.second,g));
        // tie(out_a_it, out_a_end) = out_edges(np.first,g);
        // for(;out_a_it != out_a_end; out_a_it ++){
        //     tie(out_b_it, out_b_end) = out_edges(np.second,g);
        //     auto out_a = target(*out_a_it,g);
        //     auto in_a = in_degree(out_a,g);
        //     bool out_a_to_first = edge(out_a,np.first,g).second ;// whether there is a reverse link from  out_a to np.first
        //     for(;out_b_it!=out_b_end;out_b_it ++){
        //         auto out_b = target(*out_b_it,g);
        //         auto in_b = in_degree(out_b,g);
        //         double current_alpha_i = c / (in_a * in_b);
        //         bool out_b_to_second = edge(out_b,np.second,g).second;// whether there is a reverse link from out_b to np.second 
        //         if(out_a_to_first && out_b_to_second){ // is a neighbor-loop
        //             NodePair bnp(out_a, out_b);
        //             double m = fabs(R[bnp] + r * current_alpha_i);
        //             if(m > r_max && m > current_max){
        //                 current_max = m;
        //                 alpha_i = current_alpha_i;
        //                 // if(fabs(r_x) > current_max){
        //                 //     current_max = r_x;
        //                 // }
        //             }
        //         }
        //     }
        // }
        // if(current_max > 0){ // if there is any self lop
        //     int k = int(ceil(log(r_max / (alpha_0 * current_max))/ log(alpha_0 * alpha_i))) + 1;
        //     double r_x= 0; // the residual to be pushed
        //     r_x = r + alpha_0 * current_max * (1 - pow(alpha_0 * alpha_i, k)) / (1 - alpha_0 * alpha_i); 
        //     // cout << "neighbor loop.." << r_x << endl;
        //     return r_x;
        // }else{
        //     return r;
        // }
    }


    // // non-singleton nodes

    // // compute optimal value to push
    // double sum_airi =  0;
    // double sum_ai_square = 0;
    // DirectedG::out_edge_iterator out_a_it, out_a_end;
    // DirectedG::out_edge_iterator out_b_it, out_b_end;
    // for(;out_a_it != out_a_end; out_a_it ++){
    //     auto out_a = target(*out_a_it,g);
    //     auto in_a = in_degree(out_a,g);
    //     for(;out_b_it!=out_b_end;out_b_it ++){
    //         auto out_b = target(*out_b_it,g);
    //         auto in_b = in_degree(out_b,g);
    //         double alpha = c / (in_a * in_b);
    //         sum_airi += alpha * R[NodePair(out_a, out_b)];
    //         sum_ai_square += pow(alpha,2);
    //     }
    // }
    // r = (r - sum_airi) / (1 + sum_ai_square);

    /* determine the residual to push by computing the maximum overlapping interval */
    // cout << "current r " << r << endl;
    // auto out_deg_a = out_degree(np.first,g);
    // auto out_deg_b = out_degree(np.second,g);
    // auto total_out = out_deg_a * out_deg_b;
    // double start[total_out+1];
    // double end[total_out+1];
    // size_t i = 0;
    // if(edge(np.first,np.second,g).second == true && edge(np.second, np.first,g).second == true){
    //     // self loop
    //     double alpha = c / (in_degree(np.first,g) * in_degree(np.second,g));
    //     start[i] = (r_max -r ) / (alpha - 1);
    //     end[i] = (-r_max - r) / (alpha - 1);
    // }else{
    //     double alpha = c / (in_degree(np.first,g) * in_degree(np.second,g));
    //     start[i] = r-r_max;
    //     end[i] = r + r_max;
    // }

    // i++;

    // DirectedG::out_edge_iterator out_a_it, out_a_end;
    // DirectedG::out_edge_iterator out_b_it, out_b_end;
    // for(;out_a_it != out_a_end; out_a_it ++){
    //     auto out_a = target(*out_a_it,g);
    //     auto in_a = in_degree(out_a,g);
    //     for(;out_b_it!=out_b_end;out_b_it ++){
    //         auto out_b = target(*out_b_it,g);
    //         auto in_b = in_degree(out_b,g);
    //         double alpha = c / (in_a * in_b);
    //         double r_i = R[NodePair(out_a, out_b)];
    //         auto current_start = - (r_max + r_i) / alpha;
    //         auto current_end = (r_max - r_i) / alpha;
    //         if(current_start > end[0] || current_end < start[0]){
    //             // filter out the nodes that conflicts with source node
    //             continue;
    //         }else{
    //             if(current_start < start[0]){
    //                 current_start = start[0];
    //             }
    //             if(current_end > end[0]){
    //                 current_end = end[0];
    //             }
    //             start[i] = current_start;
    //             end[i] = current_end;
    //             i ++;
    //         }
    //     }
    // }
    // // for(i = 0; i< total_out +1;i++){
    // //     int a;
    // //     cout << "lower bound: " << start[i] << " upper bound: "<< end[i] << endl;
    // //     cin >> a;
    // // }

    // double push_residual = findMaxInterval(start, end, i); // i is the length 
}

void LocalPush::local_push(DirectedG &g) { // local push given current P and R
    // cout << r_max << endl;
    auto start = std::chrono::high_resolution_clock::now();
    double sum_of_est = 0;

    while (!Q.empty()) {
        NodePair np = Q.front();
        Q.pop();
        marker[np] = false;

        double residual_to_push = how_much_residual_to_push(g, np);
        sum_of_est += residual_to_push;

        R[np] -= residual_to_push;
        P[np] += residual_to_push;
        push_to_neighbors(g, np, residual_to_push); // push residuals to neighbros of np
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
#ifdef OUTPUT
    if (cpu_time == -1) {
        cpu_time = elapsed.count();
        mem_size = getValue();
    }
#endif
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
    return LOCAL_PUSH_DIR + str(format("RLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}

string Full_LocalPush::get_file_path_base() {
    // return the file path, exluding the suffix
    return LOCAL_PUSH_DIR + str(format("FLP_%s-%.3f-%.6f") % g_name % c % epsilon);
}

double Full_LocalPush::query_P(unsigned long a, unsigned long b) {
    return P.query(a, b);
}

double Full_LocalPush::query_R(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    return R.query(a, b);
}

void Full_LocalPush::insert(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v, DirectedG &g) {
    DirectedG::vertex_iterator v_it, v_end;
    tie(v_it, v_end) = vertices(g);
    auto in_deg_v = in_degree(v, g);
    for (; v_it != v_end; v_it++) {
        auto a = *v_it;
        if (a != v) {
            update_residual(u, v, a, g, in_deg_v);
        } else {
            // a == v
            R[NodePair(a, a)] = 1 - P.query(a, a);
        }
    }
}

void Full_LocalPush::update_residual(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v,
                                     DirectedG::vertex_descriptor a, DirectedG &g, int in_deg_v) {
    // 1st: accumulate estimates
    DirectedG::in_edge_iterator in_a_it, in_a_end;
    tie(in_a_it, in_a_end) = in_edges(a, g);
    float estimation_sum = 0;
    for (; in_a_it != in_a_end; in_a_it++) {
        auto ina = source(*in_a_it, g);
        estimation_sum += P.query(u, ina);
    }

    // 2nd: update residual
    auto np = NodePair(v, a);
    float left_part = c * estimation_sum / (in_deg_v + 1) / in_degree(a, g);
    float right_part = (P.query(v, a) + R.query(v, a)) / in_deg_v;
    R[np] = left_part - right_part;
    // 3rd: update queue
    if (fabs(R[np]) > r_max) {
        if (!marker[np]) {
            Q.push(np);
            marker[np] = true;
        }
    }
}

void LocalPush::insert(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v,
                       DirectedG &g) {
    // insert edge(u,v) to g, noted we assume g is already updated, and we just update P and R
    DirectedG::vertex_iterator v_it, v_end;
    tie(v_it, v_end) = vertices(g);
    for (; v_it != v_end; v_it++) {
        auto a = *v_it;
//        auto in_deg_v = in_degree(v, g); // the new degree of v
        if (a > v) {
            update_residual(g, v, a);
        } else if (a < v) {
            update_residual(g, a, v);
        } else {
            // a == v
            R[NodePair(a, a)] = 1 - P[NodePair(a, a)];
        }
    }
//    local_push(g);
}

void LocalPush::remove(DirectedG::vertex_descriptor u, DirectedG::vertex_descriptor v,
                       DirectedG &g) {
    // remove edge(a,b) to g, noted we assume g is already updated, and we just update P and R
}

double LocalPush::query_P(DirectedG::vertex_descriptor a, DirectedG::vertex_descriptor b) {
    if (a > b) {
        return P[NodePair(b, a)];
    } else {
        return P[NodePair(a, b)];
    }
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
//        cout << format("in_deg_a:%s, in_deg_b:%s") % in_deg_a % in_deg_b << endl;
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

// dynamic update support: deleting and inserting edge, time complexity: O(E)
void Reduced_LocalPush::update_residuals_by_deleting_edge(DirectedG &g, DirectedG::vertex_descriptor u,
                                                          DirectedG::vertex_descriptor v) {
    // udpate residual when an edge (u,v) is removed from G
    // assumption: G has not been updated, (u,v) exists in G
    // a: the iterated node over G
    // v: the node whose in-neighbor is changed
    DirectedG::vertex_iterator v_begin, v_end, v_it;
    tie(v_begin, v_end) = vertices(g);
    for (v_it = v_begin; v_it != v_end; v_it++) {
        auto a = *v_it;
        if (a != v && in_degree(a, g) != 0) {
            NodePair np = a < v ? NodePair(a, v) : NodePair(v, a); // the node pair to be updated

            DirectedG::in_edge_iterator in_a_iter, in_a_begin, in_a_end;
            double P_av = P.query(np.first, np.second); // current P(a,v)
            auto &R_av_ref = R[np]; // current R(a,v)
            double updated_R = P_av + R_av_ref;

            if (in_degree(v, g) == 1) {
                // u is the only in-neighbor of v
                updated_R = -P_av;
            } else {
                tie(in_a_begin, in_a_end) = in_edges(a, g);
                double u_contrib = 0;
                //iterate over a's in-neighbors
                for (in_a_iter = in_a_begin; in_a_iter != in_a_end; in_a_iter++) {
                    auto a_prime = source(*in_a_iter, g);
                    double contrib = 0;
                    if (a_prime > u) {
                        contrib = c * P.query(u, a_prime);
                    } else if (a_prime < u) {
                        contrib = c * P.query(a_prime, u);
                    } else if (a_prime == u) {
                        contrib = c * sqrt(2) * P.query(a_prime, u);
                    }
                    u_contrib += contrib;
                }
                u_contrib = u_contrib / (in_degree(v, g) * in_degree(a, g));
                updated_R = updated_R - u_contrib;
                updated_R = updated_R * ((in_degree(v, g)) / (in_degree(v, g) - 1.0));
                updated_R -= P_av;
            }
            R_av_ref = updated_R;

            if (fabs(updated_R) / sqrt(2) > r_max) {
                if (!marker[np]) {
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
    for (v_it = v_begin; v_it != v_end; v_it++) { // ignore the starting node 0
        auto a = *v_it;

        if (a != v && in_degree(a, g) != 0) {
            double u_contrib = 0; // the amount of R^{'} related to u
            NodePair np = a < v ? NodePair(a, v) : NodePair(v, a); // the node pair to be updated

            auto &residual_ref = R[np];
            auto scaled_rest =
                    (in_degree(v, g) / (in_degree(v, g) + 1.0)) * (residual_ref + P.query(np.first, np.second));

            // iterate over a's in-neighbors
            DirectedG::in_edge_iterator in_a_iter, in_a_begin, in_a_end;
            tie(in_a_begin, in_a_end) = in_edges(a, g);
            for (in_a_iter = in_a_begin; in_a_iter != in_a_end; in_a_iter++) {
                auto a_prime = source(*in_a_iter, g);
                double contrib = 0;
                if (a_prime > u) {
                    contrib = c * P.query(u, a_prime);
                } else if (a_prime < u) {
                    contrib = c * P.query(a_prime, u);
                } else if (a_prime == u) {
                    contrib = c * sqrt(2) * P.query(a_prime, u);
                }
                u_contrib += contrib;
            }
            u_contrib = u_contrib / ((in_degree(v, g) + 1.0) * in_degree(a, g));

            auto updated_residual = u_contrib + scaled_rest - P.query(np.first, np.second);
            residual_ref = updated_residual;

            if (fabs(updated_residual) / sqrt(2) > r_max) {
                if (!marker[np]) {
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
}