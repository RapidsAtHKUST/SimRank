#ifndef SIMSTRUCT_H
#define SIMSTRUCT_H

#include <cstring>
#include <cmath>

#include <vector>
#include <algorithm>
#include <queue>
#include <functional>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <unordered_map>

#include "Graph.h"
#include "Random.h"
#include "util.h"

using namespace std;

class SimStruct {
public:
    Graph g;
    double C_value;
    // probe at most trunStep steps, trunStep <= maxStep and is dynamically determined
    int maxStep;

    /* for my batch */
    int trunStep;
    // estimate the costs of deterministic probe and randomized probe for each step
    double *randomCost;
    double *deterCost;
    // mustRandom[step] == true means only do randomized probe for this step
    bool *mustRandom;
    // store the samples
    vector<int> *lvl;
    vector<int> *sampleLvl;

    double *H[2];
    int *U[2];
    int *C[1];
    int *UC[1];
    int nr;
    Random R;

    /* for unbiased prefiltering */
    int *Count[2];

    // batch tree
    struct WalkTree {
        int id;
        int node;
        double weight;
        int children_num;
        vector<WalkTree *> children;
    };

    struct HashFunc {
        std::size_t operator()(const pair<int, int> &key) const {
            using std::size_t;
            using std::hash;

            return ((std::hash<int>()(key.first) ^ (std::hash<int>()(key.second) << 1)) >> 1);
        }
    };

    struct EqualKey {
        bool operator()(const pair<int, int> &lhs, const pair<int, int> &rhs) const {
            return lhs.first == rhs.first
                   && lhs.second == rhs.second;
        }
    };

    SimStruct(const SimStruct &sim_struct, double eps) : g(sim_struct.g), C_value(sim_struct.C_value),
                                                         nr(sim_struct.nr) {
//        cout << "nr= " << nr << endl;

        H[0] = new double[g.n];
        H[1] = new double[g.n];
        U[0] = new int[g.n];
        U[1] = new int[g.n];
        C[0] = new int[g.n];
        UC[0] = new int[g.n];
        for (int i = 0; i < g.n; i++) {
            H[0][i] = 0;
            H[1][i] = 0;
            U[0][i] = -1;
            U[1][i] = -1;
            C[0][i] = 0;
            UC[0][i] = -1;
        }
        //
        Count[0] = new int[g.n];
        Count[1] = new int[g.n];
        for (int i = 0; i < g.n; i++) {
            Count[0][i] = 0;
            Count[1][i] = 0;
        }

        maxStep = 5;    // 5
        trunStep = maxStep;
        deterCost = new double[maxStep + 1];
        randomCost = new double[maxStep + 1];
        mustRandom = new bool[maxStep + 1];
        for (int i = 0; i <= maxStep; i++) {
            deterCost[i] = 0;
            randomCost[i] = 0;
            mustRandom[i] = false;
        }
        lvl = new vector<int>[maxStep + 1];
        sampleLvl = new vector<int>[maxStep + 1];
    }

    SimStruct(string fn, double C_value, double eps, double delta) : g(fn), C_value(C_value) {
        cout << "graph= " << fn << endl;
        cout << "(c, eps, delta):" << C_value << " , " << eps << " , " << delta << endl;
//        nr = (int) (0.5 / (eps * eps) * log(g.n) / log(2));
        nr = (int) (C_value * 3 / (eps * eps) * log(g.n / delta) / log(2));
        cout << "nr= " << nr << endl;

        H[0] = new double[g.n];
        H[1] = new double[g.n];
        U[0] = new int[g.n];
        U[1] = new int[g.n];
        C[0] = new int[g.n];
        UC[0] = new int[g.n];
        for (int i = 0; i < g.n; i++) {
            H[0][i] = 0;
            H[1][i] = 0;
            U[0][i] = -1;
            U[1][i] = -1;
            C[0][i] = 0;
            UC[0][i] = -1;
        }
        //
        Count[0] = new int[g.n];
        Count[1] = new int[g.n];
        for (int i = 0; i < g.n; i++) {
            Count[0][i] = 0;
            Count[1][i] = 0;
        }

        maxStep = 5;    // 5
        trunStep = maxStep;
        deterCost = new double[maxStep + 1];
        randomCost = new double[maxStep + 1];
        mustRandom = new bool[maxStep + 1];
        for (int i = 0; i <= maxStep; i++) {
            deterCost[i] = 0;
            randomCost[i] = 0;
            mustRandom[i] = false;
        }
        lvl = new vector<int>[maxStep + 1];
        sampleLvl = new vector<int>[maxStep + 1];
    }

    ~SimStruct() {
        delete[] H[0];
        delete[] H[1];
        delete[] U[0];
        delete[] U[1];
        delete[] C[0];
        delete[] UC[0];

        //delete[] Count[0];
        //delete[] Count[1];

        delete[] randomCost;
        delete[] deterCost;
        delete[] mustRandom;
        delete[] lvl;
        delete[] sampleLvl;
    }

    // resultList reuse: double *resultList = new double[g.n];
    double batch_for_single_pair(int u, int v, double *resultList) {
        if (u == v) {
            return 1;
        }
        auto *nodeList = new int[maxStep + 1];
        // clear resultList
        for (int i = 0; i < g.n; i++) {
            resultList[i] = 0;
        }
        // first traverse one hop
        oneHopDeter(u, resultList);

        clock_t t0 = clock();
        trunStep = maxStep;
        for (int i = 0; i <= maxStep; i++) {
            lvl[i].clear();
            sampleLvl[i].clear();
            mustRandom[i] = false;
            randomCost[i] = 0;
            deterCost[i] = 0;
        }
        // batch
        simRank_WalkTree(u, resultList, nodeList);
        clock_t t1 = clock();

        for (int i = 0; i < g.n; i++) {
            if (i != u && resultList[i] > 0)
                resultList[i] /= (double) nr;
        }
        delete[] nodeList;
        return resultList[v];
    }

    double batch(int u, string res_dir) {
        double time = 0;
        int *nodeList = new int[maxStep + 1];
        double *resultList = new double[g.n];
        for (int i = 0; i < g.n; i++) {
            resultList[i] = 0;
        }
        // first traverse one hop
        time += oneHopDeter(u, resultList);

        clock_t t0 = clock();
        trunStep = maxStep;
        for (int i = 0; i <= maxStep; i++) {
            lvl[i].clear();
            sampleLvl[i].clear();
            mustRandom[i] = false;
            randomCost[i] = 0;
            deterCost[i] = 0;
        }
        // batch
        simRank_WalkTree(u, resultList, nodeList);
        clock_t t1 = clock();
        time += (t1 - t0) / (double) CLOCKS_PER_SEC;
        cout << "walk tree time cost: " << (t1 - t0) / (double) CLOCKS_PER_SEC << endl;

        vector<pair<int, double> > sims;
        for (int i = 0; i < g.n; i++) {
            if (i != u && resultList[i] > 0)
                sims.push_back(pair<int, double>(i, resultList[i] / (double) nr));
        }
        sort(sims.begin(), sims.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
            return a.second > b.second;
        });// sort pair<int, double> in non-increasing order by double value

        stringstream ss_out;
        ss_out << res_dir << "/" << u << ".txt";
        ofstream of_res(ss_out.str());
        for (int i = 0; i < sims.size(); i++)
            of_res << sims[i].first << " " << sims[i].second << endl;
        of_res.flush();
        of_res.close();
        delete[] nodeList;
        delete[] resultList;
        return time;
    }

    void simRank_WalkTree(int u, double *resultList, int *nodeList) {
        auto *root = new WalkTree();
        root->node = u;
        root->weight = 0;
        root->children_num = 0;
        root->id = 0;
        unordered_map<pair<int, int>, WalkTree *, HashFunc, EqualKey> W_map;
        int idCount = 1;
#ifdef DEBUG
        clock_t t0 = clock();
#endif
        // insert nr C walks
        for (int k = 1; k <= nr; k++) {
            int tempNode = u;
            nodeList[0] = u;
            int nodeCount = 1;

            while (R.drand() < C_value) {
                int length = g.getInSize(tempNode);
                if (length == 0)
                    break;
                int r = R.generateRandom() % length;

                int newNode = g.getInVert(tempNode, r);
                nodeList[nodeCount] = newNode;
                tempNode = newNode;
                nodeCount++;
                if (nodeCount >= maxStep + 1)
                    break;
            }
            // only insert walks to batch tree whose steps >= 2
            if (nodeCount > 2)
                insertTree(root, nodeList, nodeCount, W_map, idCount);
        }
#ifdef DEBUG
        clock_t t1 = clock();
        cout << "tree construction time: " << (t1 - t0) / (double) CLOCKS_PER_SEC << endl;
        cout << "walk tree size: " << idCount << endl;
#endif
        int maxRatio = 10;    // trade off between deter cost and random cost
        // for step = 2 to maxStep, estimate the random cost and the deter cost
        estProbeCost(root, maxRatio);
#ifdef DEBUG
        clock_t t2 = clock();
        cout << "probe cost est time: " << (t2 - t1) / (double) CLOCKS_PER_SEC << endl;
        cout << "maxStep= " << maxStep << endl;
        cout << "trunStep= " << trunStep << endl;
        for (int x = 2; x <= trunStep; x++)
            cout << x << "\t" << mustRandom[x] << "\t" << randomCost[x] << "\t" << deterCost[x] << endl;
#endif
        // probe
        calculateResult(root, resultList, maxRatio);

#ifdef DEBUG
        clock_t t3 = clock();
        cout << "calculateResult time: " << (t3 - t2) / (double) CLOCKS_PER_SEC << endl;
#endif
        delete root;
    }

    void insertTree(WalkTree *root, int *nodeList, int nodeCount,
                    unordered_map<pair<int, int>, WalkTree *, HashFunc, EqualKey> &W_map, int &idCount) {
        WalkTree *curpos = root;
        root->weight++;
        int tempId = root->id;
        for (int i = 1; i < nodeCount; i++) {
            if (W_map.find(pair<int, int>(tempId, nodeList[i])) != W_map.end()) {
                curpos = W_map[pair<int, int>(tempId, nodeList[i])];
                curpos->weight++;                    //
                tempId = curpos->id;
            } else {
                curpos->children_num++;
                WalkTree *tempNode = new WalkTree();
                tempNode->id = idCount;
                tempNode->node = nodeList[i];                                    //
                tempNode->weight = 1;                //
                tempNode->children_num = 0;
                curpos->children.push_back(tempNode);
                W_map[pair<int, int>(tempId, nodeList[i])] = tempNode;
                curpos = tempNode;
                tempId = idCount;
                idCount++;

                if (find(lvl[i].begin(), lvl[i].end(), tempId) == lvl[i].end())
                    lvl[i].push_back(tempId);
            }
        }
    }

    void estProbeCost(WalkTree *root, int maxRatio) {
        // for each step, select a sample set
        for (int i = 2; i <= maxStep; i++) {
            int minSize = 5;
            int sampleSize = max(minSize, (int) (lvl[i].size()) / (nr / 100));
            if (lvl[i].size() <= sampleSize) {
                for (int j = 0; j < lvl[i].size(); j++)
                    sampleLvl[i].push_back(lvl[i][j]);
            } else {
                for (int j = 0; j < sampleSize; j++)
                    sampleLvl[i].push_back(lvl[i][j]);
                for (int j = sampleSize; j < lvl[i].size(); j++) {
                    if (R.drand() < (double) sampleSize / (j + 1)) {
                        int pos = R.generateRandom() % sampleSize;
                        sampleLvl[i][pos] = lvl[i][j];
                    }
                }
            }
        }
#ifdef DEBUG
        for (int x = 2; x <= maxStep; x++)
            cout << x << "\tlvl size: " << lvl[x].size() << "\tsampleLvl size:" << sampleLvl[x].size() << endl;
#endif
        //
        vector<int> nodeList;
        nodeList.push_back(root->node);
        // traverse the batch tree and estimate cost
        traverse(root, nodeList, maxRatio);
        //trunStep = max(trunStep, 2);
    }

    // traverse the batch tree, for each step, estimate random/deter cost
    void traverse(WalkTree *root, vector<int> &nodeList, int maxRatio) {
        for (int i = 0; i < root->children_num; i++) {
            nodeList.push_back(root->children[i]->node);
            int level = nodeList.size() - 1;
            if (level >= 2 && level <= trunStep &&
                find(sampleLvl[level].begin(), sampleLvl[level].end(), root->children[i]->id) !=
                sampleLvl[level].end()) {
                int thres_random_visit = (int) (level * 1000 * 2.0 * g.m /
                                                g.n);    // level * 1000 * (int)(2.0 * g.m / g.n)
                int num_random_visit = randomProbe(nodeList, nullptr, thres_random_visit);    //
#ifdef DEBUG
                cout << "level: " << level << "\tnum_random_visit= " << num_random_visit << " , thres_random_visit= "
                     << thres_random_visit << endl;
#endif
                if (num_random_visit == -1) {
                    trunStep = max(2, level - 1);    //
                    for (int x = level; x <= maxStep; x++)
                        mustRandom[x] = true;
                } else if (mustRandom[level] == false) {
                    int num_deter_visit = deterProbe(nodeList, 0, nullptr,
                                                     max(thres_random_visit, num_random_visit * maxRatio));    //
#ifdef DEBUG
                    cout << "num_deter_visit= " << num_deter_visit << " , thres_random_visit= " << thres_random_visit
                         << " , num_random_visit * maxRatio= " << num_random_visit * maxRatio << endl;
#endif
                    if (num_deter_visit == -1) {
                        for (int x = level; x <= maxStep; x++)
                            mustRandom[x] = true;
                    } else {
                        randomCost[level] += num_random_visit;
                        deterCost[level] += num_deter_visit;
                    }
                }
            }
            traverse(root->children[i], nodeList, maxRatio);
            nodeList.pop_back();
        }
    }

    void calculateResult(WalkTree *wt, double *resultList, int maxRatio) {
        vector<int> nodeList;
        nodeList.push_back(wt->node);
        calculate(nodeList, wt, resultList, maxRatio);
    }

    void calculate(vector<int> &nodeList, WalkTree *root, double *resultList, int maxRatio) {
        if (nodeList.size() >= trunStep + 1)
            return;
        for (int i = 0; i < root->children_num; i++) {
            nodeList.push_back(root->children[i]->node);

            int level = nodeList.size() - 1;
            if (level >= 2 && level <= trunStep) {
                // if deterCost / randomCost >= weight * maxRatio, randomCost is better
                if (mustRandom[level] == true ||
                    randomCost[level] * root->children[i]->weight * maxRatio / (double) level <= deterCost[level]) {
                    for (int x = 0; x < (int) (root->children[i]->weight); x++)
                        randomProbe(nodeList, resultList);
                } else
                    deterProbe(nodeList, root->children[i]->weight, resultList);
            }

            calculate(nodeList, root->children[i], resultList, maxRatio);
            nodeList.pop_back();
        }
    }

    // randomized probe
    int randomProbe(vector<int> &nodeList, double *resultList) {
        if (nodeList.size() - 1 > trunStep)
            return 0;
        int num_visit_nodes = 0;

        int target = nodeList.size() - 1;
        int root_node = nodeList[target];
        int ind = 0;
        H[ind][root_node] = 1;
        int Ucount = 1;
        int Ucount1 = 0;
        int UCcount = 0;
        U[0][0] = root_node;
        for (int i = 0; i < target; i++) {
            for (int j = 0; j < Ucount; j++) {
                int tempNode = U[ind][j];
                int outCount = g.getOutSize(tempNode);

                num_visit_nodes += outCount;

                for (int k = 0; k < outCount; k++) {
                    int newNode = g.getOutVert(tempNode, k);
                    if (newNode != nodeList[target - i - 1]) {
                        if (C[0][newNode] == 0) {
                            C[0][newNode] = 1;
                            UC[0][UCcount] = newNode;
                            UCcount++;
                        } else {
                            C[0][newNode]++;
                        }
                    }
                }
            }

            for (int j = 0; j < UCcount; j++) {
                int tempNode = UC[0][j];
                if (R.drand() < C[0][tempNode] / (double) g.getInSize(tempNode)) {
                    H[1 - ind][tempNode] = 1;
                    U[1 - ind][Ucount1] = tempNode;
                    Ucount1++;
                }
                C[0][UC[0][j]] = 0;
                UC[0][j] = -1;
            }
            for (int j = 0; j < Ucount; j++) {
                H[ind][U[ind][j]] = 0;
                U[ind][j] = -1;
            }
            Ucount = Ucount1;
            Ucount1 = 0;
            UCcount = 0;
            ind = 1 - ind;
            if (Ucount == 0)
                break;
        }
        for (int i = 0; i < Ucount; i++) {
            int tempNode = U[ind][i];
            if (resultList != nullptr)    //
                resultList[tempNode] += H[ind][tempNode];
            U[ind][i] = -1;
            H[ind][tempNode] = 0;
        }
        Ucount = 0;

        return num_visit_nodes;
    }

    // randomized probe, if # visit nodes > thres_random_visit, it returns
    // for random cost estimation
    int randomProbe(vector<int> &nodeList, double *resultList, int thres_random_visit) {
        if (nodeList.size() - 1 > trunStep)
            return 0;
        int num_visit_nodes = 0;

        int target = nodeList.size() - 1;
        int root_node = nodeList[target];
        int ind = 0;
        H[ind][root_node] = 1;
        int Ucount = 1;
        int Ucount1 = 0;
        int UCcount = 0;
        U[0][0] = root_node;
        for (int i = 0; i < target; i++) {
            for (int j = 0; j < Ucount; j++) {
                int tempNode = U[ind][j];
                int outCount = g.getOutSize(tempNode);

                num_visit_nodes += outCount;

                for (int k = 0; k < outCount; k++) {
                    int newNode = g.getOutVert(tempNode, k);
                    if (newNode != nodeList[target - i - 1]) {
                        if (C[0][newNode] == 0) {
                            C[0][newNode] = 1;
                            UC[0][UCcount] = newNode;
                            UCcount++;
                        } else {
                            C[0][newNode]++;
                        }
                    }
                }
            }

            for (int j = 0; j < UCcount; j++) {
                int tempNode = UC[0][j];
                if (R.drand() < C[0][tempNode] / (double) g.getInSize(tempNode)) {
                    H[1 - ind][tempNode] = 1;
                    U[1 - ind][Ucount1] = tempNode;
                    Ucount1++;
                }
                C[0][UC[0][j]] = 0;
                UC[0][j] = -1;
            }
            for (int j = 0; j < Ucount; j++) {
                H[ind][U[ind][j]] = 0;
                U[ind][j] = -1;
            }
            Ucount = Ucount1;
            Ucount1 = 0;
            UCcount = 0;
            ind = 1 - ind;
            if (Ucount == 0)
                break;
            //
            if (num_visit_nodes > thres_random_visit)
                break;
        }
        for (int i = 0; i < Ucount; i++) {
            int tempNode = U[ind][i];
            if (resultList != nullptr)    //
                resultList[tempNode] += H[ind][tempNode];
            U[ind][i] = -1;
            H[ind][tempNode] = 0;
        }
        Ucount = 0;

        if (num_visit_nodes > thres_random_visit)
            return -1;
        else
            return num_visit_nodes;
    }

    // deterministic probe
    int deterProbe(vector<int> &nodeList, double weight, double *resultList) {
        int target = nodeList.size() - 1;
        int root_node = nodeList[target];
        int ind = 0;
        H[ind][root_node] = 1;
        int Ucount = 1;
        int Ucount1 = 0;
        U[0][0] = root_node;

        int num_visit_nodes = 0;

        for (int i = 0; i < target; i++) {
            for (int j = 0; j < Ucount; j++) {
                int tempNode = U[ind][j];
                /*
                if (H[ind][tempNode] < epsilon_p){
                    H[ind][tempNode] = 0;
                    U[ind][j] = -1;
                    continue;
                }
                */
                num_visit_nodes += g.getOutSize(tempNode);

                for (int k = 0; k < g.getOutSize(tempNode); k++) {
                    int newNode = g.getOutVert(tempNode, k);
                    if (newNode != nodeList[target - i - 1]) {
                        if (H[1 - ind][newNode] == 0) {
                            U[1 - ind][Ucount1] = newNode;
                            Ucount1++;
                        }
                        H[1 - ind][newNode] += H[ind][tempNode] / g.getInSize(newNode);
                    }
                }
                H[ind][tempNode] = 0;
                U[ind][j] = -1;
            }
            ind = 1 - ind;
            Ucount = Ucount1;
            Ucount1 = 0;
        }
        for (int i = 0; i < Ucount; i++) {
            int tempNode = U[ind][i];
            if (resultList != nullptr)    //
                resultList[tempNode] += weight * H[ind][tempNode];
            U[ind][i] = -1;
            H[ind][tempNode] = 0;
        }

        return num_visit_nodes;
    }

    // deterministic probe, if # visit nodes > thres_deter_visit, it returns
    // for deter cost estimation
    int deterProbe(vector<int> &nodeList, double weight, double *resultList, int thres_deter_visit) {
        int target = nodeList.size() - 1;
        int root_node = nodeList[target];
        int ind = 0;
        H[ind][root_node] = 1;
        int Ucount = 1;
        int Ucount1 = 0;
        U[0][0] = root_node;

        int num_visit_nodes = 0;

        for (int i = 0; i < target; i++) {
            for (int j = 0; j < Ucount; j++) {
                int tempNode = U[ind][j];
                /*
                if (H[ind][tempNode] < epsilon_p){
                    H[ind][tempNode] = 0;
                    U[ind][j] = -1;
                    continue;
                }
                */
                num_visit_nodes += g.getOutSize(tempNode);

                for (int k = 0; k < g.getOutSize(tempNode); k++) {
                    int newNode = g.getOutVert(tempNode, k);
                    if (newNode != nodeList[target - i - 1]) {
                        if (H[1 - ind][newNode] == 0) {
                            U[1 - ind][Ucount1] = newNode;
                            Ucount1++;
                        }
                        H[1 - ind][newNode] += H[ind][tempNode] / g.getInSize(newNode);
                    }
                }
                H[ind][tempNode] = 0;
                U[ind][j] = -1;
            }
            ind = 1 - ind;
            Ucount = Ucount1;
            Ucount1 = 0;

            if (num_visit_nodes > thres_deter_visit)
                break;
        }
        for (int i = 0; i < Ucount; i++) {
            int tempNode = U[ind][i];
            if (resultList != nullptr)    //
                resultList[tempNode] += weight * H[ind][tempNode];
            U[ind][i] = -1;
            H[ind][tempNode] = 0;
        }
        Ucount = 0;

        if (num_visit_nodes > thres_deter_visit)
            return -1;
        else
            return num_visit_nodes;
    }

    // traverse one hop from the query node u
    double oneHopDeter(int u, double *resultList) {
#ifdef DEBUG
        cout << "deterministically bfs one hop: ";
#endif
        int tempLength = g.getInSize(u);
        if (tempLength == 0) {
            return 0;
        }
#ifdef DEBUG

        cout << tempLength << " , ";

        clock_t t0 = clock();
#endif
        for (int i = 0; i < tempLength; i++) {
            vector<int> tempList;
            tempList.push_back(u);
            tempList.push_back(g.inAdjList[u][i]);
            // by calling deterProbe
            deterProbe(tempList, C_value * nr / (double) tempLength, resultList);
        }
#ifdef DEBUG
        clock_t t1 = clock();
        double t = (t1 - t0) / (double) CLOCKS_PER_SEC;
        cout << t << "s" << endl;
        return t;
#endif
        return 0;
    }
};

#endif