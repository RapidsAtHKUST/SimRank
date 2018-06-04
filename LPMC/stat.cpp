#include "stat.h"

void LinearRegression::fit(vector<vector<double>> & _X, vector<double> &_Y){
    MatrixXd X(N, P);
    VectorXd Y(N);

    // fill in the data
    for(int i = 0; i < N; i++){
        for (int j = 0; j< P; j++){
            X(i,j) = _X[i][j];
        }
        Y(i) =  _Y[i];
    }
    // cout << X << endl;
    // cout << Y << endl;
    W = X.bdcSvd(ComputeThinU | ComputeThinV).solve(Y); // least squared estimation 
    // cout << W << endl;

    // print the data
    // for(int i = 0; i < N; i++){
    //     for (int j = 0; j< p; j++){
    //         cout <<  format("%s,%s:%s") % i % j % X(i,j) << endl;
    //     }
    // }
}

double LinearRegression::predict(vector<double> &x){
    // x: have p features
    double hat_y = 0;
    for(int i = 0; i<P;i++){
        hat_y += (x[i] * W(i));
    }
    return hat_y;
}


int parseLine(char *line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char *p = line;
    while (*p < '0' || *p > '9') p++;
    line[i - 3] = '\0';
    i = atoi(p);
    return i;
}

int getValue() { //Note: this value is in KB!
    FILE *file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

double random_01(){
    // generate random number from 0 to 1
    static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine generator(seed);
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}

int random_int(int lower, int upper){
    return lower + int((upper - lower) * random_01());
}

int sample_an_1c_walk(NodePair np, DirectedG &g, int length_of_random_walk){
    int a = np.first;
    int b = np.second;
    double prob;
    double indicator = 0;
    int step = 0; // 


    while( step < length_of_random_walk && a != b){ // walk when > c or the first step
        a = sample_in_neighbor(a, g);
        b = sample_in_neighbor(b, g);
        step ++;
        if(a == -1 || b == -1){
            break;
        }else if (a == b){
            indicator = 1;
            break;
        }
    }
    return indicator;
}
