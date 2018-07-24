#ifndef _POOLING_H_
#define _POOLING_H_

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
#include "Random.h"
#include <unordered_map>
#include <string.h>
#include <math.h>
#include "Graph.h"
#include "util.h"

// sort vector<pair<int double> >
bool comp(const pair<int, double> &a, const pair<int, double> &b) {
	return a.second>b.second;
}

class Poll
{
friend void _t_singlePair(Poll *poll, int u, int v, int walk_num, int min_meet_dist, int tid);
public:
	Graph g;
	double singlePairMulti(int u, int v, double walk_num, int min_meet_dist);
	const double C_value = 0.6;
	const static int NUMTHREAD = 20;
	Random Rs[NUMTHREAD];
	double singlePairSim[NUMTHREAD];
	unsigned int seeds[NUMTHREAD];
	struct MC_Tree{
		unordered_map<int, double> map1;
		unordered_map<int, vector<pair<int, double> > > map2;
	};

	Poll(Graph &g_)
	{
		g = g_;
	}

	MC_Tree createMCTree(int nodeId){
    	MC_Tree mc_tree;
    	int length = g.getInSize(nodeId);
    	if(length == 0)
    		return mc_tree;
    	for(int i = 0; i < length; i++){
    		int tempNode = g.getInVert(nodeId, i);
    		mc_tree.map1[tempNode] = C_value / (double) length;
    		int length2 = g.getInSize(tempNode);
    		for(int j = 0; j < length2; j++){
    			int tempNode2 = g.getInVert(tempNode, j);
    			mc_tree.map2[tempNode2].push_back(pair<int, double>(tempNode, C_value * C_value / (double) length / (double) length2));
    		}
    	}
    	return mc_tree;
	}

	double singlePairDeter2Hop(int u, int v, MC_Tree& mc_tree_u)
	{
		double answer = 0;
		int length = g.getInSize(v);

		if(length > 0){
			for(int i = 0; i < length; i++){
	    		int tempNode = g.getInVert(v, i);
	    		if(mc_tree_u.map1.find(tempNode) != mc_tree_u.map1.end()){
	    			answer += mc_tree_u.map1[tempNode] / (double) length;
	    		}
	    		int length2 = g.getInSize(tempNode);
	    		for(int j = 0; j < length2; j++){
	    			int tempNode2 = g.getInVert(tempNode, j);
	    			if(mc_tree_u.map2.find(tempNode2) != mc_tree_u.map2.end()){
	    				vector<pair<int, double> > candidate_list = mc_tree_u.map2[tempNode2];
	    				for(int k = 0; k < candidate_list.size(); k++){
	    					int level1_node = candidate_list[k].first;
	    					double psim = candidate_list[k].second;
	    					if(tempNode != level1_node)
	    						answer += psim / (double) length / (double) length2;
	    				}
	    			}
	    		}
			}
		}
		return answer;
	}

void t_singlePair(int u, int v, int walk_num, int min_meet_dist, int tid){
	Random tempR = Rs[tid];
    clock_t t0 = clock();
    //double sqrtC = sqrt(C_value);
	double answer = 0;
	//unsigned int seed = time(NULL);
	unsigned int seed = seeds[tid];
	for(int i = 0; i < walk_num; i++){
		int u_newNode = u, v_newNode = v, u_nextNode, v_nextNode;
		int meet_dist=0;
		//while(rand_r(&seed) / (double)RAND_MAX < C_value){
		while(tempR.drand() < C_value){
			int length = g.getInSize(u_newNode);
			if(length == 0)
				break;
			//int r = rand_r(&seed) % length;
			int r = tempR.generateRandom() % length;
			u_nextNode = g.getInVert(u_newNode, r);
			length = g.getInSize(v_newNode);
			if(length == 0)
				break;
			//r = rand_r(&seed) % length;
			r = tempR.generateRandom() % length;
			v_nextNode = g.getInVert(v_newNode, r);
			meet_dist++;
			if(u_nextNode == v_nextNode){
				if(meet_dist > min_meet_dist)
					answer += 1;
				break;
			}
			u_newNode = u_nextNode;
			v_newNode = v_nextNode;
		}
	}
	singlePairSim[tid] = answer / (double) walk_num;
    clock_t t1 = clock();
    cout << "thread: " << tid << ", walk: " << walk_num << ", time: " << (t1 - t0) / (double) CLOCKS_PER_SEC << endl;
}

void poll(string query_file, int num_query, int k, vector<string> dirs, string poll_gt_dir)
{
	srand(unsigned(time(0)));
    for(int i = 0; i < NUMTHREAD; i++){
    	seeds[i] = unsigned(rand());
    	Rs[i] = Random(seeds[i]);
		singlePairSim[i] = 0;
    }

	ifstream if_query(query_file);
	double total_deter_t=0, total_random_t=0;
	for(int cnt = 0; cnt < num_query; cnt++)
	{
		int qv;
		if_query >> qv;
		cout << "query vertex: " << qv << endl;

		vector<int> res;
		for(int i = 0; i < dirs.size(); i++)
		{
			stringstream ss_temp;
			ss_temp << dirs[i] << "/" << qv << ".txt";
			readfile(ss_temp.str(), res, k);
		}
		cout << "k: " << k << endl;
		cout << "size of poll list: " << res.size();

		vector<pair<int, double> > mc_gt;
		double deter_t = 0, random_t = 0;
		clock_t t0 = clock();
		MC_Tree mc_tree = createMCTree(qv);
		//MC_Tree3 mc_tree3 = createMCTree3(qv);
		clock_t t1 = clock();
		cout << "constructing mc_tree costs " << (t1-t0)/(double)CLOCKS_PER_SEC << "s" << endl;
		deter_t += (t1 - t0)/(double)CLOCKS_PER_SEC;

		for(int i = 0; i < res.size(); i++)
		{
			cout << "top-k candidate: " << res[i] << endl;
			clock_t t2 = clock();
			double answer1 = singlePairDeter2Hop(qv, res[i], mc_tree);
			clock_t t3 = clock();
			cout << "deterministically walk 2 steps: " << (t3 - t2)/(double)CLOCKS_PER_SEC << "s" << endl;
			deter_t += (t3 - t2)/(double)CLOCKS_PER_SEC;

			clock_t t4 = clock();
			double answer2 = singlePairMulti(qv, res[i], 1000*1000*10, 2);
			clock_t t5 = clock();
			cout << "multithread random walks: " << (t5 - t4)/(double)CLOCKS_PER_SEC << "s" << endl;
			random_t += (t5 - t4)/(double)CLOCKS_PER_SEC;

			mc_gt.push_back(pair<int, double>(res[i], answer1+answer2));
		}

		sort(mc_gt.begin(), mc_gt.end(), comp);
		stringstream ss_gt;
		ss_gt << poll_gt_dir << "/" << qv << ".txt";
		ofstream of_gt(ss_gt.str());
		for(int i = 0; i < mc_gt.size(); i++)
			of_gt << mc_gt[i].first << " " <<mc_gt[i].second << endl;
		
		cout << "query vertex: " << qv << "\tdeter_t= " << deter_t << "s , random_t= " << random_t << "s" << endl;
		total_deter_t += deter_t;
		total_random_t += random_t;
		deter_t=0;
		random_t=0;
	}
	if_query.close();
	cout << "poll: total_deter_t= " << total_deter_t << "s , total_random_t= " << total_random_t << "s" << endl;
}
};

void _t_singlePair(Poll* poll, int u, int v, int walk_num, int min_meet_dist, int tid){
	poll -> t_singlePair(u, v, walk_num, min_meet_dist, tid);
}

double Poll::singlePairMulti(int u, int v, double walk_num, int min_meet_dist){
	cout << "init..." << endl;

	int thread_walk = walk_num / NUMTHREAD;
	vector<thread> threads;
	for(int i=0; i < NUMTHREAD - 1; i++)
		threads.push_back(thread(_t_singlePair, this, u, v, thread_walk, min_meet_dist, i));
	cout << "init done!" << endl;
	t_singlePair(u, v, thread_walk, min_meet_dist, NUMTHREAD - 1);

	for(int i=0; i<NUMTHREAD-1; i++)
		threads[i].join();

	double answer=0;
	for(int i = 0; i < NUMTHREAD; i++){
 		answer += singlePairSim[i];
 	}
 	return answer / (double)NUMTHREAD;
}
#endif