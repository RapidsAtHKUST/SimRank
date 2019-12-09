//
// Created by yche on 1/22/18.
//

#include <boost/program_options.hpp>

#include "../yche_refactor/simrank.h"
#include "../yche_refactor/parallel_local_push_yche.h"

using namespace std;
using namespace boost::program_options;


template <typename T>
vector<size_t> sort_indexes(const vector<T> &v) {

  // 初始化索引向量
  vector<size_t> idx(v.size());
  //使用iota对向量赋0~？的连续值
  iota(idx.begin(), idx.end(), 0);

  // 通过比较v的值对索引idx进行排序
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});
  return idx;
}


bool rules(int i, int j) { 
    return (i>j);
}


int main() {
    // 1st: compute reduce local push
    // string g_name = argv[1];
    // double eps = atof(argv[2]);
    string g_name = "id_rel";
    // string path = get_edge_list_path();
    string tmp = "/csproject/biggraph/ywangby/hete_dataset/imdb_homo/item_item.txt";
    GraphYche g(tmp);
    double c = 0.6;
    double eps = 0.001;

    auto n = g.n;

#ifdef FLP
    auto lp = new PFLP(g, g_name, c, eps, n);
#else
    auto lp = new PRLP(g, g_name, c, eps, n);
#endif

    auto start_time = std::chrono::high_resolution_clock::now();
    lp->local_push(g);
    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end_time - start_time;
    cout << format("computation time: %s s") % elapsed.count() << endl; // record the pre-processing time
    cout << format("mem size: %s KB") % getValue() << endl;
    cout << format("P size: %s") % lp->P.size() << endl;
    cout << format("R size: %s") % lp->R.size() << endl;
    ofstream my_file;
    int x = 3281747;
    vector<float> res(n);
    for (auto i = 0; i < n; i++){
        res[i] = lp->query_P(x, i);
    }
    vector<size_t> ind = sort_indexes(res);
    my_file.open(string("imdb_simrank_res.txt"), ios::out|ios::ate|ios::app);
    if(my_file.is_open()){
        for(int a = 0; a < n; a++){
           my_file <<ind[a]<< endl;
        }
        // my_file <<ind<<endl;
        my_file.close();
    }
    // string save_str = "save";
    // if (argc >= 4 && string(argv[3]) == "save")
    //     lp->save();

//     // 2nd: verify the correcness
//     if (n < 10000) {
//         TruthSim ts(g_name, g, c, eps);
//         auto max_err = 0.0;

//         double err = 0;
// #pragma omp parallel for reduction(max:max_err), reduction(+:err) schedule(dynamic, 1)
//         for (auto i = 0u; i < n; i++) {
//             for (auto j = i; j < n; j++) {
//                 auto res = lp->query_P(i, j);
//                 max_err = max(max_err, abs(ts.sim(i, j) - res));
//                 err += abs(ts.sim(i, j) - res);
//                 if (abs(ts.sim(i, j) - res) > eps + pow(10, -6)) {
// #pragma omp critical
//                     cout << i << ", " << j << "," << ts.sim(i, j) << "," << res << endl;
//                 }
//             }
//         }
//         cout << "max err:" << max_err << endl;
//         cout << "mean err:" << (err / ((n + 1) * n / 2)) << endl;

    // };
}