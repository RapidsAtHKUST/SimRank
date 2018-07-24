#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "util.h"
#include "evaluate.h"
#include "pooling.h"
using namespace std;
int main(int argc, char** argv)
{
	/*
	cout << "graph file: " << argv[1] << endl;
	cout << "query file: " << argv[2] << endl;
	cout << "# query: " << argv[3] << endl;
	cout << "# result dir: " << argv[4] << endl;
	vector<string> dirs;
	for(int i = 0; i < atoi(argv[4]); i++)
		dirs.push_back(argv[4+i+1]);
	cout << "polled result dir: " << argv[4+1+atoi(argv[4])] << endl;
	int k = 50;
	Graph g = Graph(argv[1]);
	Poll poll(g);
	poll.poll(argv[2], atoi(argv[3]), k, dirs, argv[4+1+atoi(argv[4])]);
	*/
	evaluateTopK(argv[1], argv[2], argv[3], 50);
	return 0;
}