#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

class Graph
{
public:
	int n;	//number of nodes
	long m;	//number of edges
	int** inAdjList;
	int** outAdjList;
	int* indegree;
	int* outdegree;

	Graph(){}
	Graph(string file)
	{
		m=0;
		ifstream infile(file);
		infile >> n;
		cout << "# nodes= " << n << endl;

		indegree = new int[n];
		outdegree = new int[n];
		for(int i=0; i<n; i++)
		{
			indegree[i] = 0;
			outdegree[i] = 0;
		}
		//read graph and get degree info
		int from;
		int to;
		while(infile.good())
		{
			infile >> from >> to;
			outdegree[from]++;
			indegree[to]++;
		}
		cout << "..." << endl;
		inAdjList = new int*[n];
		outAdjList = new int*[n];
		for(int i=0; i<n; i++)
		{
			inAdjList[i] = new int[indegree[i]];
			outAdjList[i] = new int[outdegree[i]];
		}
		int* pointer_in = new int[n];
		int* pointer_out = new int[n];
		for(int i=0;i<n;i++)
		{
			pointer_in[i] = 0;
			pointer_out[i] = 0;
		}
		infile.clear();
		infile.seekg(0);

		clock_t t1=clock();
		infile >> n;
		cout << "# nodes= " << n << endl;
		while(infile.good())
		{
			infile >> from >> to;
			outAdjList[from][pointer_out[from]] = to;
			pointer_out[from]++;
			inAdjList[to][pointer_in[to]] = from;
			pointer_in[to]++;

			m++;
		}
		infile.close();
		clock_t t2=clock();
		cout << "# edges= " << m << endl;
		cout << "reading in graph takes " << (t2-t1)/(double)CLOCKS_PER_SEC << " s" << endl;

		delete[] pointer_in;
		delete[] pointer_out;
	}
	~Graph()
	{
		/*
		if(indegree != nullptr)
			delete[] indegree;
		if(outdegree != nullptr)
			delete[] outdegree;
		for(int i=0; i<n; i++)
		{
			if(inAdjList[i] != nullptr)
				delete[] inAdjList[i];
			if(outAdjList[i] != nullptr)
				delete[] outAdjList[i];
		}
		if(inAdjList != nullptr)
			delete[] inAdjList;
		if(outAdjList != nullptr)
			delete[] outAdjList;
		*/
	}
	int getInSize(int vert){
		return indegree[vert];
	}
	int getInVert(int vert, int pos){
		return inAdjList[vert][pos];
	}
	int getOutSize(int vert){
		return outdegree[vert];
	}
	int getOutVert(int vert, int pos){
		return outAdjList[vert][pos];
	}
};


#endif