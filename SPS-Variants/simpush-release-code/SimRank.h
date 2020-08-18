//
// Created by shijm on 17/8/19.
//

#ifndef MYSIM_SIMRANK_H
#define MYSIM_SIMRANK_H

#include <cstdlib>
#include <cmath>
#include <sstream>
#include <queue>
#include <unordered_set>
#include <iomanip>
#include <cstring>
#include <set>
#include <chrono>

#include "Graph.h"
#include "robin_map.h"
#include "IdProbPair.h"
#include "Random.h"

typedef tsl::robin_map<int, double> mymapID;
typedef tsl::robin_map<int, tsl::robin_map<int, int>> mymapofmapII;
typedef tsl::robin_map<int, vector<int>> mymapOfVecI;
typedef tsl::robin_map<int, vector<mymapID>> mymapOfVecMapID;
typedef tsl::robin_map<int, vector<IdProbPair>> mymapOfVecIpps;

class SimRank {

public:
    Graph *g;//Class Graph
    Random RandObj;//Class Random
    string filelabel;
    double sqrtC;
    double C_value;
    double epsilon_h;
    string epsilonStr;
    int vert;

    double *result = NULL;

    int *frontierQueue = NULL;
    double *frontierPushValueQueue = NULL;
    int frontierSiz = 0;

    ~SimRank() {
        if (result != NULL) delete[] result;
        if (frontierQueue != NULL) delete[] frontierQueue;
        if (frontierPushValueQueue != NULL) delete[] frontierPushValueQueue;
    }

    SimRank(Graph *graph, string file_lable, double eps_h, string epsStr, double c) {
        filelabel = file_lable;
        epsilon_h = eps_h;
        epsilonStr = epsStr;
        C_value = c;
        sqrtC = sqrt(C_value);

        g = graph;
        vert = g->n;

        RandObj = Random();
        result = new double[vert];
        memset(result, 0, sizeof(double) * vert);

        frontierQueue = new int[vert];
        frontierPushValueQueue = new double[vert];
    }

    void reset() {
        for (int i = 0; i < vert; i++) {
            result[i] = 0.0;
        }
    }

    void singleSource(int sid) {

        double sidIndegRecip = g->indegRecip[sid];
        if (sidIndegRecip <= 0) {
            result[sid] = 1.0;
            return;
        }

        int maxLevToPush = detectMaxLevel(sid);

        vector<mymapOfVecI> parentsOfNodesInGu(maxLevToPush + 2);

        int levelOfPush = 0;
        vector<mymapID> hprobsFromS(maxLevToPush + 2);
        hprobsFromS[levelOfPush][sid] = 1;

        double pushValueThreshold = 0.0000005;
        //faster version:
        //pushValueThreshold = epsilon / 50;

        double spushValue = 1.0 * sqrtC * sidIndegRecip;
        if (spushValue >= pushValueThreshold) {
            frontierSiz = 0;
            frontierQueue[frontierSiz] = sid;
            frontierPushValueQueue[frontierSiz] = spushValue;
            frontierSiz++;
        }

        int queueIdx = 0;
        while (frontierSiz > 0 && levelOfPush < maxLevToPush) {
            int front = frontierQueue[queueIdx];
            double pushValue = frontierPushValueQueue[queueIdx];
            int indegFront = g->indegree[front];
            for (int i = 0; i < indegFront; ++i) {
                int vid = g->inAdjLists[front][i];
                hprobsFromS[levelOfPush + 1][vid] += pushValue;
                parentsOfNodesInGu[levelOfPush + 1][vid].push_back(front);
            }
            queueIdx++;

            if (queueIdx >= frontierSiz) {
                //GOTO NEXT STEP by step++: and then find frontiers for next step. when q.empty is true, it means frontiers in current step are all pushed.
                levelOfPush++;
                frontierSiz = 0;//prepare for next level frontier queue
                queueIdx = 0;
                for (auto element : hprobsFromS[levelOfPush]) {
                    int vid = element.first;
                    double prob = element.second;
                    double pushValue = g->indegRecip[vid] * prob * sqrtC;
                    if (pushValue >= pushValueThreshold) {
                        frontierQueue[frontierSiz] = vid;
                        frontierPushValueQueue[frontierSiz] = pushValue;
                        frontierSiz++;
                    }
                }
            }
        }

        //Get hitting prob from every node to every other node in Tu.
        vector<IdProbPair> attNodesByLevels(1 + levelOfPush);
        //auto start = std::chrono::high_resolution_clock::now();
        vector<mymapOfVecIpps> probTreesOfAttNodesByLevels = computeHittingProbsInGu(hprobsFromS, parentsOfNodesInGu,
                                                                                     levelOfPush, attNodesByLevels);
        //compute dk and integrate
        int lastLevWithAttNodes = probTreesOfAttNodesByLevels.size() - 1;
        vector<mymapID> revPushProbsAllLevels(lastLevWithAttNodes + 1);
        integrateCorrectionFactors(attNodesByLevels, lastLevWithAttNodes, probTreesOfAttNodesByLevels,
                                   revPushProbsAllLevels);

        batchedReversePush(revPushProbsAllLevels, lastLevWithAttNodes);

        result[sid] = 1.0;
    }

    int detectMaxLevel(int source) {
        double delta = 0.00001;
        mymapofmapII nodeCntsByLevels;
        //In practice, no need to perform too many random walks i.e.,log(1/((1-sqrt(C_value))*epsilon_h*delta))/epsilon_h/epsilon_h
        double amountPerShare = epsilon_h / (double) log(g->n);
        int numWs = 1.0 / amountPerShare;
        int sIndeg = g->indegree[source];
        if (sIndeg > 0) {
            for (int j = 0; j < numWs; ++j) {
                int curV = source;
                int curIndeg = sIndeg;
                int curLev = 0;
                while (RandObj.drand() <= sqrtC) {
                    if (curIndeg == 0) break;
                    curV = g->inAdjLists[curV][RandObj.generateRandom() % curIndeg];
                    curLev++;
                    nodeCntsByLevels[curLev][curV]++;
                    curIndeg = g->indegree[curV];
                    if (curLev > 200) break;
                }
            }
        }

        int maxLev = 0;
        for (auto lentry : nodeCntsByLevels) {
            int lev = lentry.first;
            for (auto nentry : lentry.second) {
                int nid = nentry.first;
                int cnt = nentry.second;
                double inc = amountPerShare * (double) cnt * pow(0.8, lev);
                if (inc > epsilon_h / 2.0) {
                    if (lev > maxLev) maxLev = lev;
                }
            }
        }
        return maxLev;
    }

    double compCorrFactorOfAttNode(int attNode, int levelFromS, vector<mymapOfVecIpps> &probTreesOfAttNodesByLevels) {
        auto itr = probTreesOfAttNodesByLevels[levelFromS].find(attNode);
        if (itr == probTreesOfAttNodesByLevels[levelFromS].end()) {
//            return 1.0;
            cout << "ERROR: att node " << attNode << " at level " << levelFromS << " does not have prob tree" << endl;
            exit(-1);
        }

        double df = 1.0;//correction factor
        vector<IdProbPair> probTreeFromAttNode = itr.value();
        int totalSiz = probTreeFromAttNode.size();
        if (totalSiz <= 1) return df;

        vector<mymapID> dfPerLevels(totalSiz);
        //minus the first level first meeting probability from df
        //AND prepare for later computation
        IdProbPair ippFirstLev = probTreeFromAttNode[1];
        for (int i = 0; i < ippFirstLev.ids.size(); ++i) {
            int vid = ippFirstLev.ids[i];
            double hprob = ippFirstLev.probs[i];
            dfPerLevels[1][vid] = hprob * hprob;
            df -= hprob * hprob;
        }

        bool isEmpty = true;
        for (int lv = 2; lv < totalSiz; ++lv) {
            IdProbPair ipp = probTreeFromAttNode[lv];
            if (ipp.ids.size() > 0) isEmpty = false;
            for (int i = 0; i < ipp.ids.size(); ++i) {
                int vid = ipp.ids[i];
                double hprob = ipp.probs[i];
                dfPerLevels[lv][vid] = hprob * hprob;
            }
        }

        if (isEmpty) return df;

        //compute df by minus the second, third ... levels first meeting probabilities.
        double df_prev = df;
        for (int lv = 2; lv < totalSiz; ++lv) {
            //Compute all d factors at lv level. Must compute early level first since later levels rely on it.
            for (int lv_d = 1; lv_d < lv; ++lv_d) {
                int lv_h = lv - lv_d;
                int levelFromSToD = levelFromS + lv_d;
                for (auto dentry : dfPerLevels[lv_d]) {
                    double dprob = dentry.second;
                    //there are many extremely small value; filter such values out for efficiency
                    if (dprob < 0.00005) continue;
                    int dvid = dentry.first;
                    //get all y nodes that dvid can push to using lv_h level and update d_lv(node, y)
                    auto itrd = probTreesOfAttNodesByLevels[levelFromSToD].find(dvid);
                    if (itrd != probTreesOfAttNodesByLevels[levelFromSToD].end()) {
                        vector<IdProbPair> probTreeOfDvid = itrd.value();
                        if (lv_h < probTreeOfDvid.size()) {
                            IdProbPair ipph = probTreeOfDvid[lv_h];
                            for (int i = 0; i < ipph.ids.size(); ++i) {
                                double hprob = ipph.probs[i];
                                int yid = ipph.ids[i];
                                dfPerLevels[lv][yid] -= dprob * hprob * hprob;
                            }
                        }
                    }
                }
            }
            for (auto dentry : dfPerLevels[lv]) {
                df -= dentry.second;
            }

            double delta = abs(df_prev - df);
            if (delta < 0.00001) {
                break;
            }
            df_prev = df;
        }
        return df;
    }

    vector<mymapOfVecIpps> computeHittingProbsInGu(vector<mymapID> &hprobsFromS,
                                                   vector<mymapOfVecI> &parentsOfNodesInGu, int totalLevel,
                                                   vector<IdProbPair> &attNodesByLevels) {

        //A node is a meeting node if its hitting prob from source > epsilon
        vector<mymapOfVecMapID> probTreesOfAllNodes(totalLevel + 1);
        int lastLevWithAttNode = 0;
        //init self-hitting prob for all MEETING Nodes at all levels.
        for (int levInGu = 1; levInGu <= totalLevel; ++levInGu) {
            IdProbPair attNodesAtLev;
            for (auto hentry : hprobsFromS[levInGu]) {
                int nid = hentry.first;
                double prob = hentry.second;
                if (prob > epsilon_h) {
                    vector<mymapID> probsFromNode(totalLevel - levInGu + 1);
                    probsFromNode[0][nid] = 1.0;//hitting prob to itself in Gu
                    lastLevWithAttNode = levInGu;
                    probTreesOfAllNodes[levInGu][nid] = probsFromNode;
                    attNodesAtLev.ids.push_back(nid);
                    attNodesAtLev.probs.push_back(prob);
                }
            }
            if (attNodesAtLev.ids.size() > 0) {
                attNodesByLevels[levInGu] = attNodesAtLev;
            }
        }
        attNodesByLevels.resize(lastLevWithAttNode + 1);

        for (int childLevelInGu = lastLevWithAttNode; childLevelInGu >= 2; --childLevelInGu) {
            //compute the hitting probs in parent level based on child level
            int parentLevInGu = childLevelInGu - 1;
            for (auto probTree : probTreesOfAllNodes[childLevelInGu]) {
                int childid = probTree.first;
                vector<mymapID> childProbTree = probTree.second;
                //for each parent of the child
                for (auto pid : parentsOfNodesInGu[childLevelInGu][childid]) {
                    if (probTreesOfAllNodes[parentLevInGu].find(pid) == probTreesOfAllNodes[parentLevInGu].end()) {
                        //The parent is non-attention node， create prob tree for it: NO need to init self to self hitting prob for NON-Attention NODES.
                        vector<mymapID> probTreeOfParent(totalLevel - parentLevInGu + 1);
                        probTreesOfAllNodes[parentLevInGu][pid] = probTreeOfParent;
                    }
                    double onestepProb = sqrtC * g->indegRecip[pid];
                    for (int stepInChild = 0; stepInChild < childProbTree.size(); ++stepInChild) {
                        if (childProbTree[stepInChild].size() == 0) continue;
                        for (auto probEntry : childProbTree[stepInChild]) {
                            probTreesOfAllNodes[parentLevInGu][pid][stepInChild + 1][probEntry.first]
                                    += probEntry.second * onestepProb;
                        }
                    }
                }
            }
        }

        //only get the prob trees of attention nodes, excluding the non attention nodes.
        vector<mymapOfVecIpps> probTreesOfAttNodes(lastLevWithAttNode + 1);
        for (int levAtGu = 1; levAtGu <= lastLevWithAttNode; ++levAtGu) {
            for (auto vmid : attNodesByLevels[levAtGu].ids) {
                vector<mymapID> probtree = probTreesOfAllNodes[levAtGu][vmid];
                int lastNonemptyLevel = probtree.size();
                for (int step = probtree.size() - 1; step >= 0; --step) {
                    if (probtree[step].size() > 0) {
                        lastNonemptyLevel = step;
                        break;
                    }
                }
                vector<IdProbPair> ipp(lastNonemptyLevel + 1);
                for (int step = 0; step <= lastNonemptyLevel; ++step) {
                    if (probtree[step].size() == 0) continue;
                    for (auto entry : probtree[step]) {
                        ipp[step].ids.push_back(entry.first);
                        ipp[step].probs.push_back(entry.second);
                    }
                }
                probTreesOfAttNodes[levAtGu][vmid] = ipp;
            }
        }

        frontierSiz = 0;
        return probTreesOfAttNodes;
    }

    void batchedReversePush(vector<mymapID> revPushProbsAllLevels,
                            int lastLevelWithAttNodes) {
        for (int deltaLevel = lastLevelWithAttNodes; deltaLevel >= 1; deltaLevel--) {
            mymapID revPushProbsCurrent = revPushProbsAllLevels[deltaLevel];
            int deltaLNext = deltaLevel - 1;
            for (auto revPEntry : revPushProbsCurrent) {
                int front = revPEntry.first;
                double residuefront = revPEntry.second;

                double residueSqrtCtmp = residuefront * sqrtC;
                if (residueSqrtCtmp > epsilon_h) {
                    int outdegFront = g->outdegree[front];
                    for (int i = 0; i < outdegFront; ++i) {
                        int outV = g->outAdjLists[front][i];
                        double pushResidue = residueSqrtCtmp * g->indegRecip[outV];
                        if (deltaLNext == 0) {
                            result[outV] += pushResidue;
                        } else {
                            revPushProbsAllLevels[deltaLNext][outV] += pushResidue;
                        }
                    }
                }
            }
        }
    }

    void integrateCorrectionFactors(vector<IdProbPair> &attNodesByLevels,
                                    int lastLevWithAttNodes,
                                    vector<mymapOfVecIpps> &probTreesOfAttNodesByLevels,
                                    vector<mymapID> &revPushProbsAllLevels) {
        mymapID dklist;//dk is the correction factor
        for (int meetLevel = 1; meetLevel <= lastLevWithAttNodes; ++meetLevel) {
            IdProbPair attNodesAtLev = attNodesByLevels[meetLevel];
            for (int i = 0; i < attNodesAtLev.ids.size(); ++i) {
                int attVid = attNodesAtLev.ids[i];
                double hprobStoAttVid = attNodesAtLev.probs[i];
                double dkAttVid = 1;
                if (meetLevel != lastLevWithAttNodes) {
                    if (dklist.find(attVid) != dklist.end()) {
                        dkAttVid = dklist[attVid];
                    } else {
                        dkAttVid = compCorrFactorOfAttNode(attVid, meetLevel, probTreesOfAttNodesByLevels);
                        dklist[attVid] = dkAttVid;
                    }
                }
                revPushProbsAllLevels[meetLevel][attVid] = hprobStoAttVid * dkAttVid;
            }
        }
    }

    void writeSingleSourceResult(int queryNid) {
        stringstream ssout;
        ssout << "result/" << filelabel << "_" << queryNid << "_" << epsilonStr << "_online.txt";
        ofstream fout(ssout.str());

        //sort first.
        vector<pair<double, int> > algoanswers;
        for (int j = 0; j < vert; j++) {
            algoanswers.push_back(make_pair(result[j], j));
        }
        sort(algoanswers.begin(), algoanswers.end(), greater<pair<double, int> >());
        for (int i = 0; i < 100000; ++i) {
            fout << algoanswers[i].second << " " << setprecision(10) << algoanswers[i].first << "\n";
        }
        fout.close();
    }
};


#endif //MYSIM_SIMRANK_H
