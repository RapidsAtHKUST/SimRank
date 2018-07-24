#ifndef _UTIL_H_
#define _UTIL_H_

#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "Graph.h"

int readfile(string file, vector<int> &vec)
{
	ifstream infile(file);
	int cnt = 0;
	while(infile.good())
	{
		int id;
		infile >> id;
		if(find(vec.begin(), vec.end(), id) == vec.end())
		{
			vec.push_back(id);
		}
		cnt ++;
	}
	infile.close();
	return cnt;
}

int readfile(string file, vector<int> &vec, int k)
{
	ifstream infile(file);
	int cnt = 0;
	while(infile.good() && cnt < k)
	{
		int id;
		double score;
		infile >> id >> score;
		if(find(vec.begin(), vec.end(), id) == vec.end())
		{
			vec.push_back(id);
		}
		cnt ++;
	}
	infile.close();
	return cnt;
}

int readfile2(string file, vector<int> &vec, int k)
{
	ifstream infile(file);
	int cnt = 0;
	double thres = 0;
	while(infile.good())
	{
		int id;
		double score;
		infile >> id >> score;

		if(cnt >= k && thres > score)
			break;

		if(find(vec.begin(), vec.end(), id) == vec.end())
			vec.push_back(id);
		cnt ++;
		if(cnt == k)
			thres = score;
	}
	infile.close();
	return cnt;
}

int readfile(string file, vector<pair<int, double> > &vec)
{
	ifstream infile(file);
	int cnt = 0;
	while(infile.good())
	{
		int id;
		double score;
		infile >> id >> score;

		if(score <= 0)
			break;

		if(find(vec.begin(), vec.end(), pair<int,double>(id, score)) == vec.end())
		{
			vec.push_back(pair<int,double>(id, score));
		}
		cnt ++;
	}
	infile.close();
	return cnt;
}

int readfile(string file, unordered_map<int, double> &map)
{
	ifstream infile(file);
	int cnt = 0;
	while(infile.good())
	{
		int id;
		double score;
		infile >> id >> score;

		if(map.find(id) == map.end())
		{
			map[id] = score;
		}
		cnt ++;
	}
	infile.close();
	return cnt;
}

#endif