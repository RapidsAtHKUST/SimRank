#ifndef PPR_H
#define PPR_H

#include <vector>
#include <algorithm>
#include <queue>
#include <functional>
#include <iostream>
#include <fstream>
#include <future>
#include <string>
#include <sstream>
#include "Graph.h"
#include "Random.h"
#include "alias.h"
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <sys/time.h> 
#include <time.h>

double alpha = 0.2;
double c = 1;
double error_threshold = 0.0000000001;

bool maxScoreCmp(const pair<int, double>& a, const pair<int, double>& b){
    return a.second > b.second;
}

class pqcompare
{
  bool reverse;
public:
  pqcompare(const bool& revparam=false)
    {reverse=revparam;}
  bool operator() (const pair<int, double>& lhs, const pair<int, double>&rhs) const
  {
    if (reverse) return (lhs.second > rhs.second);
    else return (lhs.second < rhs.second);
  }
};


void RandomWalk(int walk_num, Alias &alias, Random &R, Graph& g, int* vert_count){
    for(int i = 0; i < walk_num; i++){
        int tempNode = alias.generateRandom_t(R);
        vert_count[tempNode]++;
        while(R.drand_t() > 0.2){
            int length = g.getOutSize(tempNode);
            if(length > 0){   
                int r = R.generateRandom_t() % length;
                tempNode = g.getOutVert(tempNode, r);
            }
            vert_count[tempNode]++;
        }
    }
}

class PPR
{
friend void ppr_t_PowerMethod(PPR* ppr, vector<int> nodeList, int iterations);
public:
    double avg_pre;
    double avg_recall;
    double avg_time;
    double avg_NDCG;
    double threshold_num;
    int error_num;
    int k;
    double error_rate;
    double error_eps;
    double bound_c;
    double forward_c;
    int tempAvgInterval;
    int maxInterval;
    Graph g;
    Random R;
    int vert;
    double alpha;
    string target_filename;
    double* pow_alpha;
    double* vert_count;
    int value_count;
    int* value_verts;
    double* r_t;
    double* r; 
    double* rmap_back;
    bool* isInQueueBack;
    unsigned NUM_CORES;
    int** multiVertCount;
    double* resultList;
    double back_walk_num;
    double avg_L1_error;
    double avg_max_error;
    double max_max_error;
    double avg_avg_error;
    double avg_top_err[500] = {0};
    int* r_hash_arr;
    bool* r_item_arr;
    double* r_max_c;
    double avg_forward_time;
    double avg_rsum;
    vector<vector<pair<int, double> > > r_vec;
    void PowerMethodMulti(int iterations, int node_count, int num_thread);
    //const static int NUMTHREAD = 20;
    Random* Rs;

    PPR(string name, double e_rate, double e_eps, int k_num, double input_alpha) {
        k = k_num;
        error_rate = e_rate;
        error_eps = e_eps;
        if(error_rate != 1){
            bound_c = 0.2;
            forward_c = 0.2;
        }
        else{
            bound_c = 1.7;
            //bound_c = 0.5;
            forward_c = 1;
        }
        error_num = k * (1-error_rate);
        avg_L1_error = 0;
        avg_max_error = 0;
        max_max_error = 0;
        avg_avg_error = 0;
        avg_time = 0;
        avg_pre = 0 ;
        avg_recall = 0;
        avg_NDCG = 0;
        avg_rsum = 0;
        avg_forward_time = 0;
        tempAvgInterval = 0;
        maxInterval = 0;
        back_walk_num = 0;
        threshold_num = 0;
        target_filename = name;
        string filename = "dataset/" + name + ".txt";
        g.inputGraph(filename);
        cout << "edge num: " << g.m << endl;
        vert = g.n;
        alpha = input_alpha;
        srand(unsigned(time(0)));
        R = Random(unsigned(rand()));
        vert_count = new double[vert];
        resultList = new double[vert];
        value_count = 0;
        r = new double[vert];
        rmap_back = new double[vert];
        value_verts = new int[vert];
        isInQueueBack = new bool[vert];
        r_hash_arr = new int[vert];
        r_item_arr = new bool[vert];
        r_max_c = new double[vert];
        for(int i =0 ; i < vert; i++){
            resultList[i] = 0;
            vert_count[i] = 0;
            value_verts[i] = -1;
            r[i] = 0;
            vector<pair<int, double> > temp_r_vec;
            r_vec.push_back(temp_r_vec);
            isInQueueBack[i] = false;
            rmap_back[i] = 0;
            r_hash_arr[i] = 0;
            r_item_arr[i] = false;
            r_max_c[i] = g.getInSize(i);
            //r_max_c[i] = 1;
        }
        NUM_CORES = std::thread::hardware_concurrency();
        assert(NUM_CORES >= 2);
        cout << "thread core: " << NUM_CORES << endl;
        multiVertCount = new int*[NUM_CORES];
        Rs = new Random[NUM_CORES];
        for(int i = 0; i < NUM_CORES; i++){
            Rs[i] = Random(unsigned(rand()));
            multiVertCount[i] = new int[vert];
            for(int j = 0; j < vert; j++){
                multiVertCount[i][j] = 0;
            }
        }
        cout << "init done! " << endl;
    }
    ~PPR() {
        for(int i = 0; i < NUM_CORES; i++){
            delete[] multiVertCount[i];
        }
        delete[] multiVertCount;
        delete[] vert_count;
        delete[] value_verts;
        delete[] r;
        delete[] isInQueueBack;
        delete[] rmap_back;
        delete[] Rs;
        delete[] r_hash_arr;
        delete[] r_item_arr;
        delete[] r_max_c;
    }

    bool is_file_exist(string fileName)
    {
        ifstream infile(fileName);
        return infile.good();
    }

    //取s点的groundtruth
    vector<int> getRealTopK(int s, int k){
        stringstream ss;
        ss << "ppr-answer/" << target_filename << "/" << s << ".txt";
        string infile = ss.str();
        ifstream real(infile);
        vector<int> realList;
        vector<double> simList;
        for(int i = 0; i < vert; i++){
            int tempId; 
            double tempSim;
            real >> tempId >> tempSim;
            if(i >= k && tempSim < simList[k-1]){
                break; 
             } 
             realList.push_back(tempId);
             simList.push_back(tempSim);
         }
         real.close();
         return realList;
     }

    unordered_map<int, double> getRealTopKMap(int s, int k){
        unordered_map<int, double> answer_map;
        stringstream ss;
        ss << "ppr-answer/" << target_filename << "/" << s << ".txt";
        string infile = ss.str();
        ifstream real(infile);
        double k_Sim = 0;
        for(int i = 0; i < vert; i++){
            int tempId;
            double tempSim;
            real >> tempId >> tempSim;
            if(i == k - 1){
                k_Sim = tempSim;
            }
            if(i >= k && tempSim < k_Sim){
                break;
            }
            answer_map[tempId] = tempSim;
        }
        real.close();
        return answer_map;
    }

    double* MonteCarlo(int u, double walk_num){
        vector<int> realList = getRealTopK(u, k);
        unordered_map<int, double> realMap = getRealTopKMap(u, k);
        double* resultList = new double[vert];
        for(int i = 0; i < vert; i++)
            resultList[i] = 0;
        if(g.getOutSize(u) == 0){
            resultList[u] = alpha;
            return resultList;
        }    

        clock_t t0 = clock();
        for(double i = 0; i < walk_num; i++){
            int tempNode = u;
            resultList[tempNode] += alpha;
            while(R.drand() > alpha){
                int length = g.getOutSize(tempNode);
                if(length == 0)
                    tempNode = u;
                else{
                    int r = R.generateRandom() % length;
                    tempNode = g.getOutVert(tempNode, r);
                }
                resultList[tempNode] += alpha;
            }
        }
        clock_t t1 = clock();
        cout << "MonteCarlo time: " << (t1 - t0) / (double) CLOCKS_PER_SEC << endl;
        for(int i = 0; i < vert; i++){
            resultList[i] /= (double) walk_num;
        }
        for(int i = 0; i<k; i++){
            int tempNode = realList[i];
            avg_top_err[i] += abs(realMap[tempNode] - resultList[tempNode]);
        }

        vector<int> newLeftVec;
        typedef priority_queue<pair<int, double>, vector<pair<int,double> >, pqcompare> pq;
        pq upper_pq(pqcompare(true));

        double UpperBound = 0;
        for(int i = 0; i < vert; i++){ 
            if(i < k){
                upper_pq.push(pair<int, double>(i, resultList[i]));
                if(i == k - 1)
                    UpperBound = upper_pq.top().second;
            }
            else{
                if(resultList[i] > UpperBound){
                    upper_pq.pop();
                    upper_pq.push(pair<int, double>(i, resultList[i]));   
                    UpperBound = upper_pq.top().second;
                }
            }
        }
        for(int i = 0; i < k; i++){
            newLeftVec.push_back(upper_pq.top().first);
            upper_pq.pop();           
        }

        cout << "precision: " << calPrecision(newLeftVec, realList, k) << endl;
        avg_pre += calPrecision(newLeftVec, realList, k);
        avg_time +=  (t1 - t0) / (double) CLOCKS_PER_SEC ;
        return resultList;
    }

    //在一个大的pair<int, double>数组中取TopK大个（按照double的值大小）
    vector<pair<int, double> > getTopK(vector<pair<int, double> > target, int k){
        typedef priority_queue<pair<int, double>, vector<pair<int,double> >, pqcompare> pq;
        pq upper_pq(pqcompare(true));

        double UpperBound = 0;
        for(int i = 0; i < target.size(); i++){ 
            if(i < k){
                upper_pq.push(target[i]);
                if(i == k - 1)
                    UpperBound = upper_pq.top().second;
            }
            else{
                if(target[i].second > UpperBound){
                    upper_pq.pop();
                    upper_pq.push(target[i]);   
                    UpperBound = upper_pq.top().second;
                }
            }
        }

        vector<pair<int, double> > answer;
        for(int i = 0; i < k; i++){
            answer.push_back(upper_pq.top());
            upper_pq.pop();           
        }
        return answer;
    }

    //TopPPR算法 dead node return to s, adaptive forward + random + back
    vector<pair<int, double> > TopPPR(int s, double r_threshold, int k){
        cout << "node: " << s << endl;
        vector<int> deadNodeList;
        vector<pair<int, double> > topkppr;
        for(int i = 0; i < vert; i++){
            if(g.getOutSize(i) == 0){
                deadNodeList.push_back(i);
            }
            resultList[i] = 0;
        }
        vector<int> realList = getRealTopK(s, k);
        int* forwardCandidate = new int[vert];
        int* restR = new int[vert];
        double* pi;
        double* r0;
        bool* isInQueue;
        double* betas;
        pi = new double[vert];
        r0 = new double[vert];
        betas = new double[vert];
        isInQueue = new bool[vert];
        queue<int> r_queue;
        if(g.getOutSize(s) == 0){
            resultList[s] = 1;
            topkppr.push_back(pair<int , double>(s,1));
            delete[] pi;
            delete[] r0;
            delete[] betas;
            delete[] isInQueue;        
            delete[] forwardCandidate;
            delete[] restR;
            return topkppr;
        }
        
        double total_t1 = 0, total_t2 = 0, total_t3 = 0, total_t4 = 0;
        int upgrad_count = 0;
        int walk_num = 0;
        double r_sum = 0;
        
        int backSizeThreshold = max(500, k);
        int candidate_size = 0;
        int restRCount = 0;
        int fora_candidate = 0;
        double upperBound = 0, lowerBound = 0;
        while(true){   
            candidate_size = 0;
            fora_candidate = 0;
            restRCount = 0;
            for(int i = 0; i < vert; i++){
                pi[i] = 0;
                r0[i] = 0;
                resultList[i] = 0;
                forwardCandidate[i] = 0;
                restR[i] = 0;
                betas[i] = 0;
                isInQueue[i] = false;
                vert_count[i] = 0;
                value_verts[i] = 0;
            }
            r0[s] = 1;
            if(1 / (double) g.getOutSize(s) > r_threshold){
                r_queue.push(s);
                isInQueue[s] = true;
            }
            else{
                restR[restRCount++] = s;
                isInQueueBack[s] = true;
            }
            clock_t t0 = clock();
            double r_res = 1;
            while(r_queue.size() > 0){
                int tempNode = r_queue.front();
                r_queue.pop();
                isInQueue[tempNode] = false;
                int tempOutSize = g.getOutSize(tempNode);
                double tempR = r0[tempNode];
                r0[tempNode] = 0;
                pi[tempNode] += alpha * tempR;
                double resume_residula = tempR * (1-alpha);
                r_res -= alpha * tempR;
                if(tempOutSize == 0){
                    if(r0[s] == 0 && resume_residula / (double) g.getOutSize(s) <= r_threshold && !isInQueueBack[s]){
                        restR[restRCount++] = s;
                        isInQueueBack[s] = true;
                    }
                    r0[s] += resume_residula;
                    if(g.getOutSize(s) > 0 && !isInQueue[s] && r0[s] / (double) g.getOutSize(s) > r_threshold){
                        isInQueue[s] = true;
                        r_queue.push(s);
                    }
                }
                else{
                    for(int i = 0; i < tempOutSize; i++){
                        int newNode = g.getOutVert(tempNode, i);
                        if(r0[newNode] == 0 && resume_residula / (double) tempOutSize / (double) g.getOutSize(newNode) <= r_threshold && !isInQueueBack[newNode]){
                            restR[restRCount++] = newNode;
                            isInQueueBack[newNode] = true;
                        }
                        r0[newNode] += resume_residula / (double) tempOutSize; 
                        if(!isInQueue[newNode] && r0[newNode] / (double) g.getOutSize(newNode) > r_threshold){
                            isInQueue[newNode] = true;
                            r_queue.push(newNode);
                        }
                    }
                }            
                if(resultList[tempNode] == 0){
                    forwardCandidate[fora_candidate++] = tempNode;
                }
                resultList[tempNode] = pi[tempNode];
            }
            vector<pair<int, double> > aliasP;
            r_sum = 0;
            //cout << restRCount << endl;
            for(int i = 0; i < restRCount; i++){
                int tempNode = restR[i];
                isInQueueBack[tempNode] = false;
                if(r0[tempNode] > 0){
                    r_sum += r0[tempNode];
                    aliasP.push_back(pair<int, double>(tempNode, r0[tempNode]));
                }
            }
            //cout << "rsum: " << r_sum << ", " << r_res << endl;

            Alias alias = Alias(aliasP);
            clock_t ta = clock();
            walk_num = max(1000, (int) ((ta - t0) / (double) CLOCKS_PER_SEC * 400000));
            //cout << "walk num: " << walk_num << endl;
            double increment = r_sum * alpha / (double) walk_num;        
            double temp_count = 0;
            value_count = 0;
            for(int i = 0; i < walk_num; i++){
                int tempNode = alias.generateRandom(R);
                if(vert_count[tempNode] == 0){
                    value_verts[value_count++] = tempNode;       
                }
                vert_count[tempNode]++;
                if(resultList[tempNode] == 0){
                    forwardCandidate[fora_candidate++] = tempNode;  
                }
                resultList[tempNode] += increment;
                while(R.drand() > alpha){
                    int length = g.getOutSize(tempNode);
                    if(length == 0){                    
                        tempNode = s;                   
                    }
                    else{
                        int tempIndex = R.generateRandom() % length;
                        tempNode = g.getOutVert(tempNode, tempIndex);
                    }
                    if(vert_count[tempNode] == 0){
                        value_verts[value_count++] = tempNode;       
                    }
                    if(resultList[tempNode] == 0){
                        forwardCandidate[fora_candidate++] = tempNode;  
                    }
                    resultList[tempNode] += increment;
                    vert_count[tempNode]++;
                }
            }

            clock_t t1 = clock();
            double gap1 = forward_c * sqrt(log(2*vert)) / (double) walk_num/* * error_rate * error_rate */;
            double gap2 = forward_c * alpha * 7 / (double)3 * r_sum * log(vert) / (double) walk_num/* * error_rate * error_rate*/;
            double multiCoff = r_sum * r_sum * alpha * alpha / increment;
            clock_t t2 = clock();

            typedef priority_queue<pair<int, double>, vector<pair<int,double> >, pqcompare> pq;
            pq upper_pq(pqcompare(true));
            pq lower_pq(pqcompare(true));

            double temp_upper_min = 0, temp_lower_min = 0;
            for(int i = 0; i < fora_candidate; i++){ 
                int tempNode = forwardCandidate[i];
                betas[tempNode] = gap1 * sqrt(multiCoff * (resultList[tempNode] - pi[tempNode])) + gap2;
                double tempUpperBound = resultList[tempNode] + betas[tempNode];
                double tempLowerBound = resultList[tempNode] - betas[tempNode];
                if(i < k){
                    lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                }
                else if(i == k){
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                    lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                    lower_pq.pop();
                    temp_lower_min = lower_pq.top().second;
                }
                else if(i == k + 1){
                    if(tempLowerBound > temp_lower_min){
                        lower_pq.pop();
                        lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                        temp_lower_min = lower_pq.top().second;
                    }
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                    upper_pq.pop();
                    temp_upper_min = upper_pq.top().second;
                }
                else{
                    if(tempLowerBound > temp_lower_min){
                        lower_pq.pop();
                        lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                        temp_lower_min = lower_pq.top().second;
                    }
                    if(tempUpperBound > temp_upper_min){
                        upper_pq.pop();
                        upper_pq.push(pair<int, double>(tempNode, tempUpperBound));   
                        temp_upper_min = upper_pq.top().second;
                    }
                }       
            }
            upperBound = upper_pq.top().second;
            lowerBound = lower_pq.top().second;          
            int leftCandidateSize = 0;

            for(int i = 0; i < fora_candidate; i++){
                int tempNode = forwardCandidate[i];
                double lowBound = resultList[tempNode] - betas[tempNode];
                double upBound = resultList[tempNode] + betas[tempNode];
                if(lowBound > upperBound){
                    leftCandidateSize++;
                }
                else if(lowBound <= upperBound && upBound >= lowerBound){
                    candidate_size++;
                }
            }
            clock_t t3 = clock();
            total_t1 += ta - t0;
            total_t2 += t1 - ta;
            total_t3 += t2 - t1;
            total_t4 += t3 - t2;

            if(candidate_size <= backSizeThreshold || leftCandidateSize >= k - error_num || gap2 < 0.00000001/*|| (r_sum > 0 && r_sum < 0.0000000001)*/){
                clock_t t3_1 = clock();
                vector<pair<int, double> > leftVec, middleVec, rightVec;
                for(int i = 0; i < fora_candidate; i++){
                    int tempNode = forwardCandidate[i];
                    double lowBound = resultList[tempNode] - betas[tempNode];
                    double upBound = resultList[tempNode] + betas[tempNode];

                    if(lowBound > upperBound){
                        leftVec.push_back(pair<int, double>(tempNode, resultList[tempNode]));
                    }
                    else if(upBound >= lowerBound){
                        middleVec.push_back(pair<int, double>(tempNode, resultList[tempNode]));
                    }
                }

                clock_t t4 = clock();
                
                vector<int> candidate_left;
                for(int i =0 ; i < leftVec.size(); i++){
                    candidate_left.push_back(leftVec[i].first);
                }
                //cout << "left size: " << candidate_left.size() << endl;
                
                clock_t t5 = clock();
                //cout << "fora precision: " << calPrecision(candidate_left, realList, candidate_left.size()) << endl;
                vector<pair<int, double> > backResult;
                struct timeval t_start,t_end;
                gettimeofday(&t_start, NULL); 
                long start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 
                int newK = k - (int)leftVec.size();  
                double time_back = 0;
                //cout << "newK: " << newK << ", middle size: " << middleVec.size() << "error_num: " << error_num << endl;   
                if(fora_candidate < min(k, 500)){
                    for(int i = 0; i < middleVec.size(); i++){
                        leftVec.push_back(middleVec[i]);
                    }
                    for(int i = 0; i < newK - (int)middleVec.size(); i++){
                        leftVec.push_back(pair<int, double>(i, 0));
                    }
                }
                else if(newK > error_num && ((int)middleVec.size() - newK) >  error_num) {
                    //cout << "candidate size: " << middleVec.size() << endl;
                    vector<int> candidate_nodes;
                    for(int i = 0; i < middleVec.size(); i++){
                        candidate_nodes.push_back(middleVec[i].first);
                    }
                    backResult = BackwardSearchNodes(candidate_nodes, 1, walk_num, s, newK, alias, pi, r0, r_sum, deadNodeList);
                    for(int i = 0; i < newK; i++){
                        leftVec.push_back(backResult[i]);
                    }
                }
                else if(newK > 0){
                    //cout << "sort middle" << endl;
                    vector<pair<int, double> > targetKVec = getTopK(middleVec, newK);
                    for(int i = 0; i < newK; i++){
                        leftVec.push_back(targetKVec[i]);
                    }
                }
                else{
                    //cout << "enough back" << endl;
                }
                gettimeofday(&t_end, NULL); 
                long end = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000; 
                int cost_time = end - start;
                sort(leftVec.begin(), leftVec.end(), maxScoreCmp);

                unordered_map<int, double> realMap = getRealTopKMap(s, k);
                int breakId = k;
                for(int i = 0; i < k; i++){
                    if(realMap[realList[i]] == 0){
                        breakId = i;
                    }
                }
                //cout << breakId << endl;
                vector<int> newLeftVec;
                for(int i = 0; i < breakId; i++){
                    newLeftVec.push_back(leftVec[i].first);
                    topkppr.push_back(leftVec[i]);
                }
                leftVec.clear();
                double lastPrecision = calPrecision(newLeftVec, realList, newLeftVec.size(), true);
                // cout << "last precision: " << lastPrecision << endl;
                // cout << "create forward time: " << total_t1 / (double) CLOCKS_PER_SEC << endl;
                // cout << "random walk time: " << total_t2 / (double) CLOCKS_PER_SEC << endl;
                // cout << "constant time: " << total_t3 / (double) CLOCKS_PER_SEC << endl;
                // cout << "sort time: " << total_t4 / (double) CLOCKS_PER_SEC << endl;
                // cout << "bound time: " << (t4 - t3_1) / (double) CLOCKS_PER_SEC << endl;
                // cout << "useless time: " << (t5 - t4) / (double) CLOCKS_PER_SEC << endl;
                // cout << "back time: " << cost_time / (double) 1000 << endl;
                double totalTime = (total_t1 + total_t2 + total_t3 + total_t4 + t4 - t3_1) / (double) CLOCKS_PER_SEC + cost_time / (double) 1000; 
                //cout << "total time: " << totalTime << endl;
                avg_pre += lastPrecision;
                avg_time += totalTime;
                break;
            }
            r_threshold *= 0.5;
        }

        delete[] pi;
        delete[] r0;
        delete[] isInQueue;
        delete[] betas;
        delete[] restR;
        delete[] forwardCandidate;
        return topkppr;
    }

    // adaptive (backpush + walk)
    vector<pair<int, double> > BackwardSearchNodes(vector<int> candidate_nodes, double r_threshold, double walk_num, int s, int tempK, Alias& alias, double* pi0, double* r0_map, double r_sum0, vector<int> deadNodeList){
        double c = bound_c;
        double time1 = 0;
        vector<pair<int, double> >  result;
        queue<int> r_candidates;
        int r_hash_count = 0;
        unordered_map<int, double> answer_sim;
        unordered_map<int, double> answer_sim2;
        for(int i = 0; i < candidate_nodes.size(); i++){
            answer_sim[candidate_nodes[i]] = pi0[candidate_nodes[i]];
        }

        unordered_map<int, double> vars;
        for(int i = 0; i < candidate_nodes.size(); i++){
            vars[candidate_nodes[i]] = 0;
        }

        clock_t ta = clock();
        for(int ind = 0; ind < candidate_nodes.size(); ind++){
            int t = candidate_nodes[ind];
            answer_sim2[t] += r0_map[t] * alpha;
            if(t == s){
                for(int i =0 ; i < deadNodeList.size(); i++){
                    int newNode = deadNodeList[i];
                    double newR =  (1-alpha);  
                    answer_sim2[t] += vert_count[newNode] * newR * alpha  * r_sum0 / (double) walk_num;
                    vars[t] += vert_count[newNode] * newR * newR * alpha * alpha * r_sum0 * r_sum0;     
                }
            }                                                                       
            for(int i = 0; i < g.getInSize(t); i++){
                int newNode = g.getInVert(t, i);
                double newR =  (1-alpha) / (double) g.getOutSize(newNode);                
                answer_sim2[t] += vert_count[newNode] * newR * alpha  * r_sum0 / (double) walk_num;
                vars[t] += vert_count[newNode] * newR * newR * alpha * alpha * r_sum0 * r_sum0;                
            }      

        }
        clock_t t1 = clock();

        double time2 = (t1 - ta) / (double) CLOCKS_PER_SEC;
        //cout << "create struct time: " << (ta - t0) / (double) CLOCKS_PER_SEC << endl;
        //cout << "random walk time: " << (t1 - ta) / (double) CLOCKS_PER_SEC << endl;
        unordered_map<int, double> bounds;
        for(int i = 0; i < candidate_nodes.size(); i++){                
            bounds[candidate_nodes[i]] = c * sqrt(3 * vars[candidate_nodes[i]] * log((int)candidate_nodes.size())) / (double) walk_num + r_threshold * alpha * c * 7 / (double)3 * r_sum0 * log(3 * (int)candidate_nodes.size()) / (double) walk_num;
        }
        typedef priority_queue<pair<int, double>, vector<pair<int,double> >, pqcompare> pq;
        pq upper_pq(pqcompare(true));
        pq lower_pq(pqcompare(true));
        double temp_upper_min = 0, temp_lower_min = 0;
        for(int i = 0; i < candidate_nodes.size(); i++){
            int tempNode = candidate_nodes[i];
            double deter_sim = answer_sim[tempNode] + answer_sim2[tempNode];
            double tempUpperBound = deter_sim + bounds[tempNode];
            double tempLowerBound = deter_sim - bounds[tempNode];
            if(i < tempK){
                lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
            }
            else if(i == tempK){
                upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                lower_pq.pop();
                temp_lower_min = lower_pq.top().second;               
            }
            else if(i == tempK+ 1){
                if(tempLowerBound > temp_lower_min){
                    lower_pq.pop();
                    lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                    temp_lower_min = lower_pq.top().second;
                }
                upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                upper_pq.pop();
                temp_upper_min = upper_pq.top().second;
            }
            else{
                if(tempLowerBound > temp_lower_min){
                    lower_pq.pop();
                    lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                    temp_lower_min = lower_pq.top().second;
                }
                if(tempUpperBound > temp_upper_min){
                    upper_pq.pop();
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));   
                    temp_upper_min = upper_pq.top().second;
                }
            }
        }
        double upperBound = upper_pq.top().second;
        double lowerBound = lower_pq.top().second;
        vector<int> leftVec, middleVec, rightVec;
        for(int i = 0; i < candidate_nodes.size(); i++){
            int tempNode = candidate_nodes[i];
            if(answer_sim[tempNode] + answer_sim2[tempNode] - bounds[tempNode] + error_eps > upperBound){
                leftVec.push_back(tempNode);
            }
            else if(answer_sim[tempNode] + answer_sim2[tempNode] + bounds[tempNode] - error_eps < lowerBound){
                rightVec.push_back(tempNode);
            }
            else{
                middleVec.push_back(tempNode);
            }
        }

        for(int i = 0; i < leftVec.size(); i++){
            result.push_back(pair<int, double>(leftVec[i], answer_sim[leftVec[i]] + answer_sim2[leftVec[i]]));
        }
        
        tempK = tempK - (int)leftVec.size();
        if((int)middleVec.size() - tempK <= error_num){
            vector<pair<int, double> > targetCandidate;
            for(int i = 0; i < middleVec.size(); i++){
                targetCandidate.push_back(pair<int, double>(middleVec[i], answer_sim[middleVec[i]] + answer_sim2[middleVec[i]]));
            }
            sort(targetCandidate.begin(), targetCandidate.end(), maxScoreCmp);
            for(int i = 0; i < tempK; i++){
                result.push_back(targetCandidate[i]);
            }
            tempK = 0;
        }
        for(int i = 0; i < candidate_nodes.size(); i++){
            answer_sim2[candidate_nodes[i]] = 0;
            vars[candidate_nodes[i]] = 0;
        }
        candidate_nodes.clear();
        candidate_nodes = middleVec;


        //rmax back
        int lastCandidate = candidate_nodes.size();
        double lastVar = 0;
        time1 = 0;

        unordered_map<int, double> candidate_r_max;
    unordered_map<int, double> candidate_edge;
        while(tempK > error_num && (int)candidate_nodes.size() - tempK > error_num && r_threshold > 0.000001){
            //cout << "epoch: " << r_threshold << endl;
            clock_t t0 = clock();
            //vector<int> r_hash;
            int r_hash_avg_count = 0;
            int total_count = 0, max_count = 0;
            for(int ind = 0; ind < candidate_nodes.size(); ind++){
                r_hash_count = 0;
                int t = candidate_nodes[ind];
                candidate_r_max[t] = 0;
                candidate_edge[t] = 0;
                rmap_back[t] = 1;
                r_item_arr[t] = true;
                r_hash_arr[r_hash_count++] = t;
                double temp_r_threshold = r_threshold;
                if(error_num > 0)
                    temp_r_threshold = min(r_max_c[t] * r_threshold, 1.0);
                total_count++;
                if(temp_r_threshold == 1.0){
                    max_count++;
                }
                if(rmap_back[t] >= temp_r_threshold){
                    r_candidates.push(t);
                    isInQueueBack[t] = true;
                }

                while(r_candidates.size() > 0){
                    int tempNode = r_candidates.front();
                    //temp_r_threshold = min(r_max_c[tempNode] * r_threshold, 1.0);
                    total_count++;
                    if(temp_r_threshold == 1.0){
                        max_count++;
                    }
                    r_candidates.pop();
                    isInQueueBack[tempNode] = false;
                    double tempR = rmap_back[tempNode];
                    rmap_back[tempNode] = 0;
                    answer_sim2[t] += r0_map[tempNode] * (alpha * tempR);                        
                    if(tempNode == s){
                        candidate_edge[t] += deadNodeList.size();
                        for(int i = 0; i < deadNodeList.size(); i++){
                            int newNode = deadNodeList[i];
                            rmap_back[newNode] += (1-alpha) * tempR;
                            if(rmap_back[newNode] <= temp_r_threshold){
                                //r_hash.push_back(newNode);
                                if(r_item_arr[newNode] == false){
                                    r_item_arr[newNode] = true;
                                    r_hash_arr[r_hash_count++] = newNode;
                                }
                            }
                            if(!isInQueueBack[newNode] && rmap_back[newNode] > temp_r_threshold){
                                r_candidates.push(newNode);
                                isInQueueBack[newNode] = true;
                            }
                        }
                    }
                    candidate_edge[t] += g.getInSize(tempNode);    
                    for(int i = 0; i < g.getInSize(tempNode); i++){
                        int newNode = g.getInVert(tempNode, i);
                        rmap_back[newNode] += (1-alpha) * tempR / (double) g.getOutSize(newNode);
                        if(rmap_back[newNode] <= temp_r_threshold){
                            //r_hash.push_back(newNode);
                            if(r_item_arr[newNode] == false){
                                r_item_arr[newNode] = true;
                                r_hash_arr[r_hash_count++] = newNode;
                            }
                        }
                        if(!isInQueueBack[newNode] && rmap_back[newNode] > temp_r_threshold){
                            r_candidates.push(newNode);
                            isInQueueBack[newNode] = true;
                        }
                    }                                            
                }
                r_hash_avg_count += r_hash_count;
                for(int i = 0; i < r_hash_count; i++){
                    if(rmap_back[r_hash_arr[i]] > 0){
                        candidate_r_max[t] = max(candidate_r_max[t], rmap_back[r_hash_arr[i]]);
                        r_vec[r_hash_arr[i]].push_back(pair<int, double>(t ,rmap_back[r_hash_arr[i]]));
                        rmap_back[r_hash_arr[i]] = 0;
                    }
                    r_item_arr[r_hash_arr[i]] = false;
                }
                //r_hash.clear();
            }
            clock_t ta = clock();
            time1 = (ta - t0) / (double) CLOCKS_PER_SEC;
            double tempWalk = (int) (400000 * time1);
            walk_num += tempWalk;
           // cout << "walk: " << walk_num << endl;
            
            for(int i = 0; i < tempWalk; i++){                
                int tempNode = alias.generateRandom(R);
                vert_count[tempNode]++;
                if(vert_count[tempNode] == 1)
                    value_verts[value_count++] = tempNode;
                    while(R.drand() > alpha){
                        int length = g.getOutSize(tempNode);
                        if(length > 0){
                            int r = R.generateRandom() % length;
                            tempNode = g.getOutVert(tempNode, r);
                        }             
                        else{
                            tempNode = s;
                        }          
                    vert_count[tempNode]++;
                    if(vert_count[tempNode] == 1)
                        value_verts[value_count++] = tempNode;
                }
            }


            for(int i = 0; i < value_count; i++){
                int temp_vert = value_verts[i];
                for(int j = 0; j < r_vec[temp_vert].size(); j++){
                    int temp_t = r_vec[temp_vert][j].first;
                    double r_t = r_vec[temp_vert][j].second;
                    answer_sim2[temp_t] += vert_count[temp_vert] * r_t * alpha * r_sum0 / (double) walk_num;
                    vars[temp_t] += vert_count[temp_vert] * r_t * r_t * alpha * alpha * r_sum0 * r_sum0;
                }
            }
            for(int i = 0; i < vert; i++){
                r_vec[i].clear();
            }
            //cout << total_count << ", " << max_count << endl;
            unordered_map<int, double> bounds;
            for(int i = 0; i < candidate_nodes.size(); i++){  
                //cout << "bound " << candidate_nodes[i] << ": " << candidate_edge[candidate_nodes[i]] << ", " << candidate_r_max[candidate_nodes[i]] << ", " << c * sqrt(3 * vars[candidate_nodes[i]] * log(candidate_nodes.size())) / (double) walk_num << ", " <<  0.1 * candidate_r_max[candidate_nodes[i]] * alpha * c * 7 / (double)3 * r_sum0 * log(3 * candidate_nodes.size()) / (double) walk_num << ", " << answer_sim[candidate_nodes[i]] + answer_sim2[candidate_nodes[i]] << endl;  
                bounds[candidate_nodes[i]] = c * sqrt(3 * vars[candidate_nodes[i]] * log((int)candidate_nodes.size())) / (double) walk_num + 0.1 * candidate_r_max[candidate_nodes[i]] * alpha * c * 7 / (double)3 * r_sum0 * log(3 * (int)candidate_nodes.size()) / (double) walk_num;
            }
            typedef priority_queue<pair<int, double>, vector<pair<int,double> >, pqcompare> pq;
            pq upper_pq(pqcompare(true));
            pq lower_pq(pqcompare(true));
            double temp_upper_min = 0, temp_lower_min = 0;
            for(int i = 0; i < candidate_nodes.size(); i++){
                int tempNode = candidate_nodes[i];
                double deter_sim = answer_sim[tempNode] + answer_sim2[tempNode];
                double tempUpperBound = deter_sim + bounds[tempNode];
                double tempLowerBound = deter_sim - bounds[tempNode];
                if(i < tempK){
                    lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                }
                else if(i == tempK){
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                    lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                    lower_pq.pop();
                    temp_lower_min = lower_pq.top().second;
                }
                else if(i == tempK+ 1){
                    if(tempLowerBound > temp_lower_min){
                        lower_pq.pop();
                        lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                        temp_lower_min = lower_pq.top().second;
                    }
                    upper_pq.push(pair<int, double>(tempNode, tempUpperBound));
                    upper_pq.pop();
                    temp_upper_min = upper_pq.top().second;
                }
                else{
                    if(tempLowerBound > temp_lower_min){
                        lower_pq.pop();
                        lower_pq.push(pair<int, double>(tempNode, tempLowerBound));
                        temp_lower_min = lower_pq.top().second;
                    }
                    if(tempUpperBound > temp_upper_min){
                        upper_pq.pop();
                        upper_pq.push(pair<int, double>(tempNode, tempUpperBound));   
                        temp_upper_min = upper_pq.top().second;
                    }
                }
            }
            double upperBound = upper_pq.top().second;
            double lowerBound = lower_pq.top().second;
            //cout << "upper and lower: " << upperBound << ", " << lowerBound << endl; 
            vector<int> leftVec, middleVec, rightVec;
            for(int i = 0; i < candidate_nodes.size(); i++){
                int tempNode = candidate_nodes[i];
                if(answer_sim[tempNode] + answer_sim2[tempNode] - bounds[tempNode] + error_eps > upperBound){
                    leftVec.push_back(tempNode);
                    //cout << "real left: " << tempNode << ", " << answer_sim[tempNode] + answer_sim2[tempNode] << ", " << bounds[tempNode] << endl;

                }
                else if(answer_sim[tempNode] + answer_sim2[tempNode] + bounds[tempNode] - error_eps < lowerBound){
                    rightVec.push_back(tempNode);
                }
                else{
                    middleVec.push_back(tempNode);
                    //cout << "middle: " << tempNode << ", " << answer_sim[tempNode] + answer_sim2[tempNode] << endl;
                }
            }
            //cout << "epoch precision: " << calPrecision(leftVec, realList, leftVec.size()) << endl;
            for(int i = 0; i < leftVec.size(); i++){
                result.push_back(pair<int, double>(leftVec[i], answer_sim[leftVec[i]] + answer_sim2[leftVec[i]]));
            }
            
            tempK = tempK - (int)leftVec.size();
            //cout << "fileter to: " << middleVec.size() << " -> " << tempK << endl;
            if(tempK < error_num || (int)middleVec.size() - tempK < error_num || r_threshold < 0.000002){
                vector<pair<int, double> > targetCandidate;
                for(int i = 0; i < middleVec.size(); i++){
                    targetCandidate.push_back(pair<int, double>(middleVec[i], answer_sim[middleVec[i]] + answer_sim2[middleVec[i]]));
                }
                sort(targetCandidate.begin(), targetCandidate.end(), maxScoreCmp);
                for(int i = 0; i < tempK; i++){
                    result.push_back(targetCandidate[i]);
                }
                tempK = 0;
            }
            for(int i = 0; i < candidate_nodes.size(); i++){
                answer_sim2[candidate_nodes[i]] = 0;
                vars[candidate_nodes[i]] = 0;
            }
            r_threshold /= 2;
            candidate_nodes.clear();
            candidate_nodes = middleVec;

        }
        //cout << r_sum0 << ", " << walk_num << ", " << r_threshold * 2 << endl;
        return result;
    }

    vector<int> getTopKList(double* resultList, int u, int k){
        vector<int> topKList;
        vector<pair<int,double> > sim;
        for(int i = 0; i < vert; i++){
            sim.push_back(pair<int, double>(i, resultList[i]));
        }
        sort(sim.begin(), sim.end(), maxScoreCmp);
        for(int i = 0; i < k; i++){
          topKList.push_back(sim[i].first);
      }
      return topKList;
    }

    //计算Precision
    double calPrecision(vector<int> topK1, vector<int> realList, int k, bool isShowMissing = false){
        int size = realList.size();
       int size2 = topK1.size();
        int hitCount = 0;
        for(int i = 0; i < size2; i++){
           bool isFind = false;
            for(int j = 0; j < size; j++){
                if(topK1[i] == realList[j]){
                    hitCount++;
                    isFind = true;
                    break;
                }
            }
            /*if(!isFind){
               cout << "useless node: " << topK1[i] << endl;
            }*/
        }
        double result = hitCount / (double) k;
        return result < 1 ? result : 1;
    }


    void t_PowerMethod(vector<int> nodeList, int iterations){
        for(int i = 0; i < nodeList.size(); i++){
            int tempNode = nodeList[i];
            stringstream ss;
            ss << "ppr-answer/" << target_filename << "/" << tempNode << ".txt";
            string outputFile = ss.str();
            cout << "file: " << outputFile << endl;
            PowerMethodK(iterations, outputFile, tempNode, 500);
        cout << outputFile << "done!"  << endl;        
        }
    }

    void PowerMethodK(int iterations, string outputFile, int u, int k){
        unordered_map<int, double> map_residual;
        map_residual.clear();
        map_residual[u] = 1.0;

        int num_iter=0;
        double* map_ppr = new double[vert];
        for(int i = 0; i < vert; i++){
            map_ppr[i] = 0;
        }
        while( num_iter < iterations ){
            cout << u << ": iter " << num_iter << endl;
            num_iter++;

            vector< pair<int,double> > pairs(map_residual.begin(), map_residual.end());
            map_residual.clear();
            for(auto &p: pairs){
                if(p.second > 0){
                    map_ppr[p.first] += alpha*p.second;
                    int out_deg = g.getOutSize(p.first);

                    double remain_residual = (1-alpha)*p.second;
                    if(out_deg==0){
                        map_residual[u] += remain_residual;
                    }
                    else{
                        double avg_push_residual = remain_residual / out_deg;
                        for(int i = 0; i < g.getOutSize(p.first); i++){
                            int next = g.getOutVert(p.first, i);
                            map_residual[next] += avg_push_residual;
                        }
                    }
                }
            }
        }
        ofstream fout(outputFile);
        vector<pair<int, double> > pprs;
        for(int j = 0; j < vert; j++){
            pprs.push_back(pair<int, double>(j, map_ppr[j]));
        }
        sort(pprs.begin(), pprs.end(), maxScoreCmp);
        for(int j = 0; j < vert; j++){
            if(pprs[j].second >= 0){
                fout << pprs[j].first << " " << pprs[j].second << "\n";
            }
            /*if(j >= 10000 && pprs[j].second < pprs[499].second){
                break;
            }*/
        }
        fout.close();
        delete[] map_ppr;
    }

    
/*void PowerMethodK2(int iterations, string outputFile, int u, int k){
    double* scores = new double[vert];
    double* nextScores = new double[vert];
    for(int i = 0; i < vert; i++){
        if(i == u){
            scores[i] = 1;
            nextScores[i] = 0;
        }
        else{
            scores[i] = 0;
            nextScores[i] = 0;
        }
    }
    for(int i = 1; i <= iterations; i++){
        for(int j = 0; j < vert; j++){
            if(j == u)
                nextScores[j] = alpha;
            else
                nextScores[j] = 0;
            for(int k = 0; k < g.getInSize(j); k++){
                int tempNode = g.getInVert(j, k);
                nextScores[j] += (1-alpha) * scores[tempNode] / (double) g.getOutSize(tempNode);
            }
        }
    bool isBreak = true;
        for(int j = 0; j < vert; j++){
            if(fabs(scores[j] - nextScores[j]) > 0.0000000001)
                isBreak = false;
            scores[j] = nextScores[j];
    }
        if(isBreak){
            cout << "break! " << endl;
            break;
        }
    }
    ofstream fout(outputFile);
    vector<pair<int, double> > pprs;
    for(int j = 0; j < vert; j++){
        pprs.push_back(pair<int, double>(j, scores[j]));
    }
    sort(pprs.begin(), pprs.end(), maxScoreCmp);
    for(int j = 0; j < vert; j++){
        if(pprs[j].second >= 0){
            fout << pprs[j].first << " " << pprs[j].second << "\n";
        }
        if(j >= 10000 && pprs[j].second < pprs[499].second){w
            break;
        }
    }
    fout.close();
    delete[] scores;
    delete[] nextScores;
}*/

    double calNDCG(vector<int> candidates, int k, int s){
        vector<int> topK = getRealTopK(s, k);
        unordered_map<int, double> realMap = getRealTopKMap(s, k);

        double correct = 0;
        for(int i = 0; i < k; i++){
            if(realMap[candidates[i]] == realMap[topK[i]])
                correct++;
            else{
                cout << "misMatch : " << candidates[i] << ", " << topK[i] << endl;
            }
        }
        return correct / (double)k;

        double Zp = 0;
        for(int i = 1; i <= k; i++){
            Zp += (pow(2, realMap[topK[i-1]]) - 1) / (log(i+1) / log(2));
        }
        double NDCG = 0;
        for(int i = 1; i <= k; i++){
            NDCG += (pow(2, realMap[candidates[i-1]]) - 1) / (log(i+1) / log(2));
        }
        return NDCG / Zp;
    }

    //generate random query node
    void generateQueryNode(int nodeNum, ofstream& fout){
        for(int i = 0; i < nodeNum; i++){
            int tempNode = R.generateRandom() % vert;
            if(g.getOutSize(tempNode) == 0){
                i--;
                continue;   
            }
            fout << tempNode << endl;
        }
    }
};

void ppr_t_PowerMethod(PPR* ppr, vector<int> nodeList, int iterations){
    return ppr->t_PowerMethod(nodeList, iterations);
}

void PPR::PowerMethodMulti(int iterations, int node_count, int num_thread){
    struct timeval t_start,t_end; 
    gettimeofday(&t_start, NULL); 
    long start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 
    string inputFile = "dataset/" + target_filename + ".query";
    ifstream node_file(inputFile);
    vector<int> nodes;
    for(int i = 0; i < node_count; i++){
        int temp_node;
        node_file >> temp_node;
        if(g.getOutSize(temp_node) == 0){
            i--;
            cout << "illegal : " << temp_node << endl;
            continue;
        }
        nodes.push_back(temp_node);
    }
    node_file.close();
    if(node_count < num_thread){
        num_thread = node_count;
    }
    vector<thread> threads;
    for(int i = 0; i < num_thread-1; i++){
        vector<int> t_nodes;
        for(int j = 0; j < node_count / num_thread; j++){
            t_nodes.push_back(nodes[i * node_count / num_thread + j]);
        }
        threads.push_back(thread(ppr_t_PowerMethod, this, t_nodes, iterations));
    }
    vector<int> t_nodes;
    for(int j = 0; j < node_count / num_thread; j++){
        t_nodes.push_back(nodes[(num_thread-1) * node_count / num_thread + j]);
    }
    t_PowerMethod(t_nodes, iterations);
    for (int i = 0; i < num_thread - 1; i++){
        threads[i].join();
    }
    gettimeofday(&t_end, NULL); 
    long end = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000; 
    int cost_time = end - start;

    cout << "cost: " << cost_time / (double) 1000 << endl;
}
#endif
