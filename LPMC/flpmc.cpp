#include <boost/program_options.hpp>
#include <cmath>
#include <algorithm> 
#include "flpmc.h"
#include "simrank.h"
using namespace std;
using namespace boost::program_options;

FLPMC::FLPMC(string g_name_, DirectedG &g_, double c_, double epsilon_, double delta_, size_t Q_):
    g_name(g_name_),
    g(&g_),
    c(c_),
    epsilon(epsilon_),
    delta(delta_),
    Q(Q_){
        // init local push
        size_t n = num_vertices(*g);
        cout << format("r_max: %s") % get_rmax() << endl;
        lp = new Reduced_LocalPush(*g, g_name, c, get_lp_epsilon(), n);
        // lp = new Full_LocalPush(*g, g_name, c, get_lp_epsilon(), n);
        if(!lp_file_exists(g_name,c,get_lp_epsilon(),n,false)){ // test wether the local push index exists
            cout << "local push offline index doesn't exists.. " << endl;
            lp->local_push(*g);
            cout << format("building compete, saving to %s " ) % lp->get_file_path_base() << endl;
            lp->save();
            cout << "saved." << endl;
        }else{
            cout << "offline index exists..loading " << endl;
            lp->load();
        }
    }

double FLPMC::get_rmax(){
    double r = sqrt(epsilon);
    cout << format("r_max of local push: %s") % r << endl;
    return r;
}

double FLPMC::get_lp_epsilon(){
    double lp_epsilon = get_rmax() * (1-c);
    cout << format("local push error: %s") % lp_epsilon << endl;
    return lp_epsilon;
}


double FLPMC::query_one2one(NodePair np){
    if(np.first == np.second){
        return 1 ;
    }else if(np.first > np.second){ // make sure np.first < np.second
        np = NodePair({np.second, np.first});
    }
    /* FLP result */
    double p_i = lp->P[np];

    // set up the random number generator
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<double> distribution(0.0,1.0);

    /* MC sampling phase */
    int N = get_N();
    double E_residual = 0;
    for(int i = 0; i< N;i++){
        double sum = 0; // the sum of residuals that passed by
        int a = np.first;
        int b = np.second;
        sum += lp->query_R(a,b);
        while(distribution(gen) < c && (a != b)){
            a = sample_in_neighbor(a, *g);
            b = sample_in_neighbor(b, *g);
            if(a == -1 || b == -1){
                break;
            }
            sum += lp->query_R(a,b);
        }
        E_residual += sum;
    }

    cout << format("p_i: %s")  % p_i << endl;
    cout << format("number of samples: %s") % get_N() << endl;
    return p_i + E_residual / N;
}

double FLPMC::get_N(){
    // compute the number of needed samples 
    double tmp = ( pow(c,2.) * pow(get_rmax(),2.) * log(2. / delta) ) / ( 2 * pow(epsilon,2.) * pow((1-c),2.));
    int n = ceil(tmp);
    return n;
}

void test_FLPMC(string data_name, double c, double epsilon, double delta, int x, int y){
    DirectedG g;
    load_graph(get_edge_list_path(data_name), g);
    size_t n = num_vertices(g);
    NodePair q{x,y};

    FLPMC flpmc(data_name, g, c, epsilon, delta, 100);
    double result = flpmc.query_one2one(q);
    cout << result << endl;

    // ground truth
    TruthSim ts(data_name, g, c, epsilon);
    cout << format("ground truth: %s") % ts.sim(x,y) << endl;
    cout << format("error: %s") % (ts.sim(q.first, q.second) - result) << endl;
}

int main(int args, char*argv[]){
    try {
        options_description desc{"Options"};
        double c = 0.6;
        double epsilon = 0.01;
        double delta = 0.01;
        string data_name;
        int x, y;
        desc.add_options()
            ("help,h", "Help Screen")
            ("DataName,d", value<string>(&data_name), "Graph Name")
            ("decay,c", value<double>(&c), "Decay Facrot c")
            ("x,x", value<int>(&x), "x: the query node")
            ("y,y", value<int>(&y), "y: the query node")
            ("epsilon,e", value<double>(&epsilon), "Error bound");
        variables_map vm;
        store(parse_command_line(args, argv, desc), vm);
        notify(vm);

        if (vm.count("help")){
            cout << desc << endl;
        }else{
            test_FLPMC(data_name, c, epsilon, delta, x, y);
        }

    }
    catch (const error &ex ){
        cerr << ex.what() << endl;
    }
    return 0;
}
