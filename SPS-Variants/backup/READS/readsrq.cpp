#include "readsrq.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include <vector>
#include <algorithm>

#include "sparsehash/sparse_hash_set"
#include "sparsehash/dense_hash_map"
#include "sparsehash/dense_hash_set"

#include "timer.h"
// #include "inBuf.h"
// #include "outBuf.h"
// #include "meminfo.h"

using google::sparse_hash_set;
using google::dense_hash_map;
using google::dense_hash_set;

readsrq::readsrq(char *gName_, int n_, int r_, int rq_, double c_, int t_) {
    sprintf(gName, "%s", gName_);
    n = n_, r = r_, c = c_, t = t_;
    rq = rq_;
    t1 = t2 = qCnt = 0;

    char iName[125];
    sprintf(iName, "%s.readsrq.%d_%d_%lf_%d", gName, n, r, c, t);


    // if (fopen(iName, "rb") != NULL)
    // {
    // 	printf("load ");
    // 	inBuf buf(iName);
    // 	leaf = new vector<array<int,3> >[r];

    // 	for (int i = 0; i < r; i++)
    // 	{
    // 		leaf[i].resize(n);
    // 		for (int j = 0; j < n; j++)
    // 		{
    // 			buf.nextInt(leaf[i][j][0]);
    // 			buf.nextInt(leaf[i][j][1]);
    // 			buf.nextInt(leaf[i][j][2]);
    // 		}
    // 	}

    // 	inode = new vector<sparse_hash_map<int, array<int, 3> > >[r];
    // 	for (int i = 0, s, x[4]; i < r; i++)
    // 	{
    // 		inode[i].resize(t);
    // 		for (int j = 0; j < t; j++)
    // 		{
    // 			inode[i][j].set_deleted_key(-1);
    // 			buf.nextInt(s);
    // 			for (int k = 0; k < s; k++)
    // 			{
    // 				buf.nextInt(x[0]);
    // 				buf.nextInt(x[1]);
    // 				buf.nextInt(x[2]);
    // 				buf.nextInt(x[3]);
    // 				inode[i][j][x[0]] = {x[1], x[2], x[3]};
    // 			}
    // 		}
    // 	}

    // 	ef.resize(n);
    // 	for (int i = 0, s; i < n; i++)
    // 	{
    // 		buf.nextInt(s);
    // 		ef[i].resize(s);
    // 		for (int j = 0; j < s; j++)
    // 			buf.nextInt(ef[i][j]);
    // 	}

    // 	eb.resize(n);
    // 	for (int i = 0, s; i < n; i++)
    // 	{
    // 		buf.nextInt(s);
    // 		eb[i].resize(s);
    // 		for (int j = 0; j < s; j++)
    // 			buf.nextInt(eb[i][j]);
    // 	}
    // 	rtime = 0;
    // 	return;
    // }

    FILE *fg = fopen(gName, "r");
    if (fg == NULL) {
        printf("No graph %s\n", gName);
        exit(0);
    }
    Timer tm;
    ef.resize(n);
    eb.resize(n);

    for (int x, y; fscanf(fg, "%d%d", &x, &y) != EOF;) {
        ef[x].push_back(y);
        eb[y].push_back(x);
    }
    rtime = tm.getTime();

    fclose(fg);

    int cc = int(RAND_MAX * c);

    leaf = new vector<array<int, 3> >[r];
    inode = new vector<sparse_hash_map<int, array<int, 3> > >[r];
    sparse_hash_map<int, array<int, 3> >::iterator jt;

    // inode = new vector<dense_hash_map<int, array<int, 3> > >[r];
    // dense_hash_map<int, array<int, 3> >::iterator jt;
    // array<int, 3> ek = {-2,-2,-2};





    for (int i = 0; i < r; i++) {
        leaf[i].resize(n);
        inode[i].resize(t);

        for (int j = 0; j < t; j++)
            inode[i][j].set_deleted_key(-1);

        for (int j = 0, p; j < n; j++)
            if (!eb[j].empty()) {
                leaf[i][j] = {p = eb[j][rand() % eb[j].size()], -1, -1};
                if ((jt = inode[i][0].find(p)) == inode[i][0].end())
                    inode[i][0][p] = {-1, j, j};
                else {
                    leaf[i][j][1] = jt->second[2];
                    leaf[i][jt->second[2]][2] = j;
                    inode[i][0][p][2] = j;
                }
            } else leaf[i][j] = {-1, -1, -1};


        for (int j = 0, rr; j < t - 1 && inode[i][j].size() > 1; j++)
            for (auto it = inode[i][j].begin(); it != inode[i][j].end(); it++)
                if (!eb[it->first].empty() && ((rr = rand()) < cc || j == 0)) {
                    it->second[0] = eb[it->first][rr % eb[it->first].size()];
                    if ((jt = inode[i][j + 1].find(it->second[0])) == inode[i][j + 1].end())
                        inode[i][j + 1][it->second[0]] = {-1, it->second[1], it->second[2]};
                    else {
                        leaf[i][it->second[1]][1] = jt->second[2];
                        leaf[i][jt->second[2]][2] = it->second[1];
                        jt->second[2] = it->second[2];
                    }
                }


//		if (i % 100 == 0) {printf("%d ", i); print_mem_info();}

    }


    for (int i = 0; i < n; i++) {
        random_shuffle(ef[i].begin(), ef[i].end());
        random_shuffle(eb[i].begin(), eb[i].end());
    }


    // tm.reset();

    // outBuf buf(iName);

    // for (int i = 0; i < r; i++)
    // 	for (int j = 0; j < n; j++)
    // 	{
    // 		buf.insert(leaf[i][j][0]);
    // 		buf.insert(leaf[i][j][1]);
    // 		buf.insert(leaf[i][j][2]);
    // 	}

    // for (int i = 0; i < r; i++)
    // 	for (int j = 0; j < t; j++)
    // 	{
    // 		buf.insert(inode[i][j].size());
    // 		for (auto it = inode[i][j].begin(); it != inode[i][j].end(); it++)
    // 		{
    // 			buf.insert(it->first);
    // 			buf.insert(it->second[0]);
    // 			buf.insert(it->second[1]);
    // 			buf.insert(it->second[2]);
    // 		}
    // 	}


    // for (int i = 0; i < n; i++)
    // {
    // 	buf.insert(ef[i].size());
    // 	for (auto j = ef[i].begin(); j != ef[i].end(); j++)
    // 		buf.insert(*j);
    // }

    // for (int i = 0; i < n; i++)
    // {
    // 	buf.insert(eb[i].size());
    // 	for (auto j = eb[i].begin(); j != eb[i].end(); j++)
    // 		buf.insert(*j);
    // }

    // rtime += tm.getTime();

}


readsrq::~readsrq() {
    delete[] leaf;
    delete[] inode;
}

//todo
double readsrq::queryOne(int x, int y) {
}

void readsrq::queryAll(int x, double *ansVal) {

    memset(ansVal, 0, sizeof(double) * n);
    if (eb[x].empty()) {
        ansVal[x] = 1;
        return;
    }

    if (qCnt >= 20 && t1 < t2 || qCnt < 20 && qCnt % 2 == 0) {
        if (qCnt < 20) tm.reset();

        dense_hash_map<int, double> sim1;
        sim1.set_empty_key(-1);

        double cc = (1 - c) * r * rq / eb[x].size(), c1 = rq * r / c / eb[x].size();


        int p;
        for (auto i = eb[x].begin(); i != eb[x].end(); i++) {
            int q;
            for (auto j = eb[p = *i].begin(); j != eb[p].end() && j - eb[p].begin() < 10; j++)
                for (auto k = ef[q = *j].begin(); k != ef[q].end() && k - ef[q].begin() < 10; k++)
                    if (*k != p)
                        sim1[*k] += cc / eb[p].size() / eb[*k].size();
//??
            sim1[*i] += c1;
        }


        sparse_hash_map<int, array<int, 3> >::iterator inodeIT;
        vector<int> q(t);


        for (int i = 0, ncnt = 0; i < r; i++)
            for (int ir = 0, k; ir < rq; ir++) {
                int j = eb[x][(ncnt++) % eb[x].size()];

                if (eb[j].empty()) continue;

                for (q[k = 0] = eb[j][rand() % eb[j].size()]; !eb[q[k]].empty() && k < t - 1; k++) {
                    if ((inodeIT = inode[i][k].find(q[k])) != inode[i][k].end()) {
                        if (inodeIT->second[0] != -1) q[k + 1] = inodeIT->second[0];
                        else {
                            q[k + 1] = eb[q[k]][rand() % eb[q[k]].size()];
                            q[k] -= n;
                        }
                    } else q[k + 1] = eb[q[k]][rand() % eb[q[k]].size()];
                }

                while (k >= 0 && q[k] >= 0 && inode[i][k].find(q[k]) == inode[i][k].end()) k--;
                if (k < 0) continue;
                if (q[k] >= 0) q[k] -= n;

                for (; k >= 0; k--)
                    if (q[k] < 0) {
                        q[k] += n;
                        for (int jj = inode[i][k][q[k]][1]; jj != -1; jj = leaf[i][jj][2])
                            if (jj != j) sim1[jj] += c;
                    }


            }


        cc = c * c / r / rq;
        for (auto it = sim1.begin(); it != sim1.end(); it++)
            for (auto jt = ef[p = it->first].begin(); jt != ef[p].end(); jt++)
                ansVal[*jt] += it->second * cc / eb[*jt].size();


        if (qCnt++ < 20) t1 += tm.getTime();
    } else {

        if (qCnt < 20) tm.reset();

        double *sim1 = new double[n];
        memset(sim1, 0, sizeof(double) * n);

        //sim1, sim2, ansVal = real*g[x][0]*r/c^2

        double cc = (1 - c) * r * rq / eb[x].size(), c1 = rq * r / c / eb[x].size();


        int p;
        for (auto i = eb[x].begin(); i != eb[x].end(); i++) {
            int q;
            for (auto j = eb[p = *i].begin(); j != eb[p].end() && j - eb[p].begin() < 10; j++)
                for (auto k = ef[q = *j].begin(); k != ef[q].end() && k - ef[q].begin() < 10; k++)
                    if (*k != p)
                        sim1[*k] += cc / eb[p].size() / eb[*k].size();
            sim1[*i] += c1;
        }


        sparse_hash_map<int, array<int, 3> >::iterator inodeIT;
        vector<int> q(t);


        for (int i = 0, ncnt = 0; i < r; i++)
            for (int ir = 0, k; ir < rq; ir++) {
                int j = eb[x][(ncnt++) % eb[x].size()];

                if (eb[j].empty()) continue;

                for (q[k = 0] = eb[j][rand() % eb[j].size()]; !eb[q[k]].empty() && k < t - 1; k++) {
                    if ((inodeIT = inode[i][k].find(q[k])) != inode[i][k].end()) {
                        if (inodeIT->second[0] != -1) q[k + 1] = inodeIT->second[0];
                        else {
                            q[k + 1] = eb[q[k]][rand() % eb[q[k]].size()];
                            q[k] -= n;
                        }
                    } else q[k + 1] = eb[q[k]][rand() % eb[q[k]].size()];
                }

                while (k >= 0 && q[k] >= 0 && inode[i][k].find(q[k]) == inode[i][k].end()) k--;
                if (k < 0) continue;
                if (q[k] >= 0) q[k] -= n;

                for (; k >= 0; k--)
                    if (q[k] < 0) {
                        q[k] += n;
                        for (int jj = inode[i][k][q[k]][1]; jj != -1; jj = leaf[i][jj][2])
                            if (jj != j) sim1[jj] += c;
                    }


            }


        for (int i = 0; i < n; i++)
            if (sim1[i] > 0)
                for (auto j = ef[i].begin(); j != ef[i].end(); j++)
                    ansVal[*j] += sim1[i];
        //		for (int j = 0; j < ef[i].size(); j++)
        //			ansVal[ef[i][j]] += sim1[i];
        cc = c * c / r / rq;
        for (int i = 0; i < n; i++)
            if (eb[i].size() > 0)
                ansVal[i] = cc * ansVal[i] / eb[i].size();

        if (qCnt++ < 20) t2 += tm.getTime();

    }

}

// void readsrq::queryK(int x, int k, int * ansNode)
// {
// 	double * ansVal = new double[n];
// //	memset(ansVal, 0, sizeof(double)*n);
// // 	double * sim1 = new double[n];
// // 	memset(sim1, 0, sizeof(double)*n);
// // 	double * sim2 = new double[n];
// // 	memset(sim2, 0, sizeof(double)*n);

// // sim1, sim2, ansVal = real*g[x][0]*r/c

// // 	double cc = (1-c)*r;

// // 	for (int i = 1; i <= g[x][0]; i++)
// // 		for (int j = 1, p = g[x][i]; j <= g[p][0]; j++)
// // 			sim2[g[p][j]] += cc/g[p][0];
// // //set?

// // 	int * nBeg = new int[g[x][0] + 1];
// // 	bool * nID = new bool[n];
// // 	memset(nID, 0, sizeof(bool)*n);
// // 	for (int j = 1; j <= g[x][0]; j++)
// // 		nID[g[x][j]] = 1;

// // 	for (int i = 0; i < n; i++)
// // 		if (sim2[i] > 0)
// // 		for (int j = 1; j <= f[i][0]; j++)
// // 			sim1[f[i][j]] += sim2[i]/g[f[i][j]][0];





// // 	for (int i = 0, j, cnt, *leafPos, nSize; i < r; i++)
// // 	{
// // 		nSize = 0;
// // 		for (j = 1; j <= g[x][0]; j++)
// // 			if (leaf[i][g[x][j]] > -1) nBeg[nSize++] = leaf[i][g[x][j]];
// // 		sort(nBeg, nBeg + nSize);

// // 		nBeg[nSize] = -1;

// // 		for (j = 1, cnt = 1; j <= nSize; j++)
// // 			if (nBeg[j] == nBeg[j-1]) cnt++;
// // 			else
// // 			{
// // 				for	(leafPos = leaf[i] + nBeg[j-1]; *leafPos >= 0; leafPos++)
// // 					if (nID[*leafPos])
// // 						sim1[*leafPos] += c*(cnt-1) + 1 - (1-c)/g[*leafPos][0];
// // 					else
// // 						sim1[*leafPos] += c*cnt;
// // 				if (nID[*leafPos + n])
// // 					sim1[*leafPos + n] += c*(cnt-1) + 1 - (1-c)/g[*leafPos+n][0];
// // 				else
// // 					sim1[*leafPos + n] += c*cnt;
// // 				cnt = 1;
// // 			}
// // 	}

// // 	for (int i = 0; i < n; i++)
// // 		if (sim1[i] > 0)
// // 		for (int j = 1; j <= f[i][0]; j++)
// // 			ansVal[f[i][j]] += sim1[i]/g[f[i][j]][0];

// //	delete [] nBeg;
// //	delete [] nID;
// //	delete [] sim1;
// //	delete [] sim2;


// queryAll(x, ansVal);


// // //sim1, sim2, ansVal = real*g[x][0]*r/c

// // 	cc = c*g[x][0];
// // 	for (int i = 0, * j; i < r; i++)
// // 		if (leaf[i][x] > -1)
// // 		{
// // 			for (j = leaf[i] + leaf[i][x]; *j >= 0; j++)
// // 				ansVal[*j] += cc;
// // 			ansVal[*j + n] += cc;
// // 		}

// // 	cc = r*(1-c);
// // 	for (int i = 1; i <= g[x][0]; i++)
// // 		for (int j = 1, p = g[x][i]; j <= f[p][0]; j++)
// // 			ansVal[f[p][j]] += cc/g[f[p][j]][0];





// 	ansVal[x] += g[x][0]*(double)r;
// 	ansNode[0] = 0;



// 	int i;
// 	for (i = 0; i < n && ansNode[0] < k; i++)
// 		if (ansVal[i] > 0)
// 			ansNode[++ansNode[0]] = i;
// 	while (i < n && ansVal[i] == 0) i++;
// 	if (i < n)
// 	{
// 		for (int j = (k>>1), tmp, p; j > 0; j--)
// 		{
// 			tmp = ansNode[p = j];
// 			while ((p <<= 1) <= k)
// 			{
// 				if (p < k && ansVal[ansNode[p]] > ansVal[ansNode[p+1]]) p++;
// 				if (ansVal[ansNode[p]] < ansVal[tmp]) ansNode[p>>1] = ansNode[p];
// 				else break;
// 			}
// 			ansNode[p>>1] = tmp;
// 		}

// 		for (int p; i < n; i++)
// 			if (ansVal[i] > ansVal[ansNode[1]])
// 			{
// 				p = 1;
// 				while ((p <<= 1) <= k)
// 				{
// 					if (p < k && ansVal[ansNode[p]] > ansVal[ansNode[p+1]]) p++;
// 					if (ansVal[ansNode[p]] < ansVal[i]) ansNode[p>>1] = ansNode[p];
// 					else break;
// 				}
// 				ansNode[p>>1] = i;
// 			}
// 	}
// 	delete [] ansVal;

// }




// void readsrq::insEdge(int x, int y)
// {

// //x = 1499, y = 6728;
// //printf("%d %d\n", x, y);
// bool ff = 0;
// //if (x == 1499 && y == 6728) ff = 1;
// //if (x == 4514 && y == 4500) 
// if (x == 4507 && y == 4492) 
// {
// printf("ff\n");
// ff = 1;
// }
// 	ef[x].push_back(y);
// 	eb[y].push_back(x);

// 	vector<array<int, 3> > ud[2], * unext, utmp(1);

// 	sparse_hash_map<int, array<int, 3> >::iterator it, kt;
// 	array<int, 3> * jt;
// 	int cc = int(RAND_MAX*sqrt(c));


// 	for (int i = 0; i < r; i++)
// 	{

// if (ff) printf("(%d) iter\n", i);


// if (ff && i == 37)
// {
// 	int p = 4485;
// 	for (int j = 0; j < t && p != -1; j++)
// 		if (inode[i][j].find(p) == inode[i][j].end())
// 		{
// 			printf("%d %d error\n", j, p);
// 			break;
// 		}
// 		else 
// 		{
// 			printf("%d %d %d %d\n", j, p, inode[i][j][p][1], inode[i][j][p][2]);
// 			p = inode[i][j][p][0];
// 		}
// 	exit(0);

// }


// 		(unext = &ud[0])->resize(0);
// 		if ( (jt = &leaf[i][y])->at(0) == -1 || rand() < RAND_MAX/eb[y].size() && jt->at(0) != x)
// 		{

// 			if (jt->at(0) != -1)
// 			{
// 				if ( (it = inode[i][0].find(jt->at(0)))->second[1] == y)
// 				{
// 					if (it->second[2] == y)
// 					{
// 						unext->push_back({it->first - n, 0, it->second[1]});
// 						it->second[1] = -1;
// 					}
// 					else
// 					{
// 						unext->push_back({it->first, 1, y});
// 						it->second[1] = jt->at(2);
// 					}
// 				}
// 				else if (it->second[2] == y)
// 				{
// 					unext->push_back({it->first, 2, y});
// 					it->second[2] = jt->at(1);
// 				}

// 				if (jt->at(1) != -1)
// 					leaf[i][jt->at(1)][2] = jt->at(2);
// 				if (jt->at(2) != -1)
// 					leaf[i][jt->at(2)][1] = jt->at(1);
// 			}

// 			if ( (it=inode[i][0].find(x)) != inode[i][0].end())
// 			{
// 				jt->at(0) = x;
// 				if ((jt->at(2) = leaf[i][it->second[2]][2]) != -1)
// 					leaf[i][ jt->at(2) ][1] = y;
// 				if ((jt->at(1) = it->second[2]) != -1)
// 					leaf[i][ jt->at(1) ][2] = y;
// 				unext->push_back( {x, 2, it->second[2]} );
// 				it->second[2] = y;
// 			}
// 			else
// 			{
// 				*jt = {x, -1, -1};
// 				unext->push_back({x-n, 1, y});
// 				inode[i][0][x] = {-1, y, y};
// 			}

// 		}
// if (ff) printf("leaf\n");

// 		for (int j = 0, rr, fa, o1, o2; j < t-1; j++)
// 		{

// if (ff) printf("%d %d\n", j, ud[j%2].size());

// 			bool fy = 0;
// 			auto uy = utmp.begin();
// 			(unext = &ud[1-j%2])->resize(0);

// 			for (auto ut = ud[j%2].begin(); ut != ud[j%2].end(); ut++)
// 			{
// 				if (ut->at(0) == y)
// 				{
// if (ff) printf("1\n");
// 					fy = 1;
// 					uy = ut;
// 				}
// 				else if (ut->at(0) >= 0)
// 				{
// if (ff) printf("2 %d %d %d %d\n", ut->at(0), ut->at(1), ut->at(2), 
// 	(kt=inode[i][j].find(ut->at(0))) != inode[i][j].end());
// if (ff) printf("%d %d %d %d\n", (it=inode[i][j+1].find(kt->second[0])) != inode[i][j+1].end(),
// 	kt->second[0], kt->second[1], kt->second[2] );

// 					if ( (kt=inode[i][j].find(ut->at(0)))->second[0] != -1 
// 						&& (it=inode[i][j+1].find(kt->second[0]))->second[ut->at(1)] == ut->at(2))
// 					{
// if (ff) printf("3\n");
// 						unext->push_back({it->first, ut->at(1), ut->at(2)});
// 						it->second[ut->at(1)] = kt->second[ut->at(1)];
// 					}
// if (ff) printf("2\n");
// 				}
// 				else if (ut->at(1) == 0)
// 				{
// if (ff) printf("4\n");
// 					if ( (kt=inode[i][j].find(ut->at(0)+n))->second[0] != -1 
// 						&& kt->second[1] == -1 
// 						&& (it=inode[i][j+1].find(kt->second[0]))->second[1] == ut->at(2) 
// 						&& it->second[2] == kt->second[2])
// 					{
// if (ff) printf("5\n");
// 						unext->push_back({it->first - n, 0, it->second[1]});
// 						it->second[1] = -1;
// 					}
// 					if (kt->second[1] == -1) 
// 					{

// if (i == 37 && j == 4 && kt->first == 4487)
// {
// 	printf("%d %d ++\n", x, y);

// }
// 						inode[i][j].erase(kt);
// 					}
// 				}
// 				else if ( !eb[fa=ut->at(0)+n].empty() && (j == 0 || rand()<cc) )
// 				{
// if (ff) printf("6 %d %d %d %d\n", i, j, fa, inode[i][j].find(fa) != inode[i][j].end());
// 					(kt=inode[i][j].find(fa))->second[0] = eb[fa][rand()%eb[fa].size()];
// if (ff) printf("6 %d\n", kt->second[0]);					
// 					unext->push_back({kt->second[0] - n, 1, 0});
// if (ff) printf("%d %d %d %d\n", i, j+1, kt->second[0], inode[i][j+1].find(kt->second[0]) != inode[i][j+1].end());
// 					inode[i][j+1][kt->second[0]] ={ -1, kt->second[1], kt->second[2]};
// if (ff) printf("%d %d %d %d\n", i, j+1, kt->second[0], inode[i][j+1].find(kt->second[0]) != inode[i][j+1].end());
// 				}
// 			}

// if (ff) printf("%d ut\n", j);


// 			if (!fy && (kt=inode[i][j].find(y)) != inode[i][j].end() ) 
// 			{
// 				fy = 1;
// if (ff) printf("resized %d %d %d\n", uy == utmp.begin(), j, t);
// //if (ff) inode[i][j+1][x] = {-1, -1, -1};

// if (ff) printf("resized %d\n", uy == utmp.begin());

// 				utmp[0] = {y, 1, kt->second[1]};
// 				// uy->at(0) = y;
// 				// uy->at(1) = 1;
// 				// uy->at(2) = kt->second[1];
// 			}
// //if (ff) printf("%d begin gen %d %d %d %d\n", j, fy, (kt=inode[i][j].find(y)) != inode[i][j].end(), eb[y].size(), kt->second[0]);
// if (ff) printf("%d begin gen %d %d %d %d\n", j, fy, (kt=inode[i][j].find(y)) != inode[i][j].end(), eb[y].size(), inode[i][j][y][0]);

// 			if ( fy && 
// 				((kt = inode[i][j].find(y))->second[0] != -1 
// 				|| eb[y].size() == 1 && (j == 0 || rand() < cc))
// 				&& rand()<RAND_MAX/eb[y].size() 
// 				&& kt->second[0] != x)
// 			{

// if (ff) printf("here %d %d %d\n", kt->second[0], kt->second[1], kt->second[2]);

// 				if (kt->second[0] != -1)
// 				{	

// 					if (uy->at(1) == 1) 
// 					{
// 						o1 = uy->at(2);
// 						o2 = kt->second[2];
// 					}
// 					else 
// 					{
// 						o1 = kt->second[1];
// 						o2 = uy->at(2);
// 					}
// 					if ( (it = inode[i][j+1].find(kt->second[0]))->second[1] == o1 )
// 					{
// 						if (it->second[2] == o2)
// 						{
// 							unext->push_back({it->first - n, 0, it->second[1]});
// 							it->second[1] = -1;
// 						}
// 						else
// 						{
// 							unext->push_back({it->first, 1, o1});
// 							it->second[1] = leaf[i][kt->second[2]][2];								
// 						}
// 					}
// 					else if (it->second[2] == o2)
// 					{
// 						unext->push_back({it->first, 2, o2});
// 						it->second[2] = leaf[i][kt->second[1]][1];
// 					}
// 				}

// 				o1 = leaf[i][kt->second[1]][1];
// 				o2 = leaf[i][kt->second[2]][2];
// 				if (o1 != -1) leaf[i][o1][2] = o2;
// 				if (o2 != -1) leaf[i][o2][1] = o1;

// if (ff) printf("here %d %d %d\n", o1, o2, inode[i][j+1].find(x) != inode[i][j+1].end());

// 				if ( (it=inode[i][j+1].find(x)) != inode[i][j+1].end() )
// 				{
// 					kt->second[0] = x;
// 					if (it->second[1] == -1)
// 					{
// 						leaf[i][it->second[1] = kt->second[1]][1] = -1;
// 						leaf[i][it->second[2] = kt->second[2]][2] = -1;
// 					}
// 					else
// 					{
// 						if ( (o2 = leaf[i][kt->second[2]][2] = leaf[i][it->second[2]][2]) != -1 )
// 							leaf[i][o2][1] = kt->second[2];
// 						if ( (o1 = leaf[i][kt->second[1]][1] = it->second[2]) != -1 )
// 							leaf[i][o1][2] = kt->second[1];
// 					}
// 				}
// 				else 
// 				{
// 					kt->second[0] = x;
// if (ff) printf("here\n");
// 					leaf[i][kt->second[1]][1] = leaf[i][kt->second[2]][2] = -1;
// if (ff) printf("here\n");
// 					unext->push_back({x-n, 1, y});
// int xx = x;
// if (ff) printf("here %d %d %d %d %d %d==\n", i, j+1, kt->second[1], kt->second[2], inode[i][j+1].find(xx) != inode[i][j+1].end(), inode[i][j+1].size());
// if (ff) printf("here\n");
// 					inode[i][j+1][x] = {-1, kt->second[1], kt->second[2]};
// if (ff) printf("here\n");
// 				}
// 			}
// if (ff) printf("%d gen\n", j);

// 		}	

// 		for (auto ut = unext->begin(); ut != unext->end(); ut++)
// 		{
// 			if (ut->at(0) < 0 && ut->at(1) == 0 && (kt=inode[i][t-1].find(ut->at(0)+n))->second[1] == -1)
// 			{
// 				inode[i][t-1].erase(kt);
// 			}
// 		}

// if (ff) printf("%d last\n", i);

// 	}

// 	if (inode[37][4].find(4487) == inode[37][4].end()) 
// 	{
// 		printf("%d %d ==\n", x, y);
// 		exit(0);
// 	}


// }







void readsrq::insEdge(int x, int y) {

    ef[x].push_back(y);
    eb[y].push_back(x);

    sparse_hash_map<int, array<int, 3> >::iterator it, kt;
    array<int, 3> *jt;
    int cc = int(RAND_MAX * c);


    for (int i = 0, o1, o2, rr; i < r; i++) {
        if ((jt = &leaf[i][y])->at(0) == -1 || rand() < RAND_MAX / eb[y].size() && jt->at(0) != x) {

            if (jt->at(0) != -1) {

                for (int j = 0, p = jt->at(0); j < t && p != -1; j++)
                    if ((it = inode[i][j].find(p))->second[1] == y) {
                        p = it->second[0];
                        if (it->second[2] == y) inode[i][j].erase(it);
                        else it->second[1] = jt->at(2);
                    } else if (it->second[2] == y) {
                        p = it->second[0];
                        it->second[2] = jt->at(1);
                    } else break;

                if (jt->at(1) != -1)
                    leaf[i][jt->at(1)][2] = jt->at(2);
                if (jt->at(2) != -1)
                    leaf[i][jt->at(2)][1] = jt->at(1);
            }

            if ((it = inode[i][0].find(x)) != inode[i][0].end()) {

                jt->at(0) = x;
                if ((jt->at(2) = leaf[i][it->second[2]][2]) != -1)
                    leaf[i][jt->at(2)][1] = y;
                if ((jt->at(1) = it->second[2]) != -1)
                    leaf[i][jt->at(1)][2] = y;

                for (int j = 0, p = x; j < t && p != -1; j++) {
                    if ((it = inode[i][j].find(p))->second[2] == jt->at(1)) it->second[2] = y;
                    else break;
                    p = it->second[0];
                }
            } else {

                *jt = {x, -1, -1};
                int j = 0, p = x;

                for (int q; j < t - 1; j++) {
                    if (eb[p].empty() || (rr = rand()) >= cc && j != 0) {
                        inode[i][j][p] = {-1, y, y};
                        break;
                    } else {
                        inode[i][j][p] = {q = eb[p][rr % eb[p].size()], y, y};
                        p = q;
                        if ((it = inode[i][j + 1].find(p)) != inode[i][j + 1].end()) {

                            if ((jt->at(2) = leaf[i][it->second[2]][2]) != -1)
                                leaf[i][jt->at(2)][1] = y;
                            if ((jt->at(1) = it->second[2]) != -1)
                                leaf[i][jt->at(1)][2] = y;

                            for (j++; j < t && p != -1; j++) {
                                if ((it = inode[i][j].find(p))->second[2] == jt->at(1)) it->second[2] = y;
                                else break;
                                p = it->second[0];
                            }
                            j = t - 2;
                            break;
                        }
                    }
                }

                if (j == t - 1) {
                    inode[i][j][p] = {-1, y, y};
                }
            }

        }


        for (int j = 0; j < t - 1; j++) {

            if ((kt = inode[i][j].find(y)) != inode[i][j].end() &&
                (j == 0 && kt->second[0] == -1 ||
                 kt->second[0] != -1 && kt->second[0] != x && rand() < RAND_MAX / eb[y].size())) {


                if (kt->second[0] != -1) {

                    o1 = leaf[i][kt->second[1]][1], o2 = leaf[i][kt->second[2]][2];
                    for (int k = j + 1, p = kt->second[0]; j < t && p != -1; k++) {
                        if ((it = inode[i][k].find(p))->second[1] == kt->second[1]) {
                            p = it->second[0];
                            if (it->second[2] == kt->second[2]) inode[i][k].erase(it);
                            else {
                                it->second[1] = o2;
                            }
                        } else if (it->second[2] == kt->second[2]) {
                            p = it->second[0];
                            it->second[2] = o1;
                        } else break;
                    }

                    if (o1 != -1) leaf[i][o1][2] = o2;
                    if (o2 != -1) leaf[i][o2][1] = o1;

                }

                if ((it = inode[i][j + 1].find(x)) != inode[i][j + 1].end()) {

                    kt->second[0] = x;
                    if ((o2 = leaf[i][kt->second[2]][2] = leaf[i][it->second[2]][2]) != -1)
                        leaf[i][o2][1] = kt->second[2];
                    if ((o1 = leaf[i][kt->second[1]][1] = it->second[2]) != -1)
                        leaf[i][o1][2] = kt->second[1];

                    for (int k = j + 1, p = x; k < t && p != -1; k++) {
                        if ((it = inode[i][k].find(p))->second[2] == o1) it->second[2] = kt->second[2];
                        else break;
                        p = it->second[0];
                    }

                } else {

                    kt->second[0] = x;
                    leaf[i][kt->second[1]][1] = leaf[i][kt->second[2]][2] = -1;
                    int k = j + 1, p = x;
                    for (int q; k < t - 1; k++) {
                        if (eb[p].empty() || (rr = rand()) >= cc) {
                            inode[i][k][p] = {-1, kt->second[1], kt->second[2]};
                            break;
                        } else {
                            inode[i][k][p] = {q = eb[p][rr % eb[p].size()], kt->second[1], kt->second[2]};
                            p = q;
                            if ((it = inode[i][k + 1].find(p)) != inode[i][k + 1].end()) {
                                if ((o2 = leaf[i][kt->second[2]][2] = leaf[i][it->second[2]][2]) != -1)
                                    leaf[i][o2][1] = kt->second[2];
                                if ((o1 = leaf[i][kt->second[1]][1] = it->second[2]) != -1)
                                    leaf[i][o1][2] = kt->second[1];


                                for (k++; k < t && p != -1; k++) {
                                    if ((it = inode[i][k].find(p))->second[2] == o1) it->second[2] = kt->second[2];
                                    else break;
                                    p = it->second[0];
                                }
                                k = t - 2;
                                break;
                            }

                        }

                    }

                    if (k == t - 1) {
                        inode[i][k][p] = {-1, kt->second[1], kt->second[2]};
                    }
                }


            }
        }
    }
}

void readsrq::delEdge(int x, int y) {
    bool fy = 0;
    for (auto it = ef[x].begin(); it != ef[x].end(); it++)
        if (*it == y) {
            ef[x].erase(it);
            fy = 1;
            break;
        }
    if (!fy) return;
    for (auto it = eb[y].begin(); it != eb[y].end(); it++)
        if (*it == x) {
            eb[y].erase(it);
            break;
        }


    sparse_hash_map<int, array<int, 3> >::iterator it, kt;
    array<int, 3> *jt;
    int cc = int(RAND_MAX * c);


//printf("%d %d\n", x, y);
    for (int i = 0, o1, o2, rr, z; i < r; i++) {

        if ((jt = &leaf[i][y])->at(0) == x) {

//if (ff) printf("1\n");


            for (int j = 0, p = jt->at(0); j < t && p != -1; j++)
                if ((it = inode[i][j].find(p))->second[1] == y) {
                    p = it->second[0];
                    if (it->second[2] == y) inode[i][j].erase(it);
                    else it->second[1] = jt->at(2);
                } else if (it->second[2] == y) {
                    p = it->second[0];
                    it->second[2] = jt->at(1);
                } else break;

            if (jt->at(1) != -1)
                leaf[i][jt->at(1)][2] = jt->at(2);
            if (jt->at(2) != -1)
                leaf[i][jt->at(2)][1] = jt->at(1);

            if (eb[y].empty()) {
                *jt = {-1, -1, -1};
                continue;
            } else z = eb[y][rand() % eb[y].size()];

            if ((it = inode[i][0].find(z)) != inode[i][0].end()) {
//if (ff) printf("3\n");

                jt->at(0) = z;
                if ((jt->at(2) = leaf[i][it->second[2]][2]) != -1)
                    leaf[i][jt->at(2)][1] = y;
                if ((jt->at(1) = it->second[2]) != -1)
                    leaf[i][jt->at(1)][2] = y;

                for (int j = 0, p = z; j < t && p != -1; j++) {
                    if ((it = inode[i][j].find(p))->second[2] == jt->at(1)) it->second[2] = y;
                    else break;
                    p = it->second[0];
                }
            } else {
//if (ff) printf("4 %d %d %d\n", leaf[i][4089][0], leaf[i][4089][1], leaf[i][4089][2]);

                *jt = {z, -1, -1};
                int j = 0, p = z;

                for (int q; j < t - 1; j++) {
                    if (eb[p].empty() || (rr = rand()) >= cc && j != 0) {
                        inode[i][j][p] = {-1, y, y};
                        break;
                    } else {
                        inode[i][j][p] = {q = eb[p][rr % eb[p].size()], y, y};
                        p = q;
//if (ff) printf("%d %d\n", j+1, p);						
                        if ((it = inode[i][j + 1].find(p)) != inode[i][j + 1].end()) {
                            // jt->at(1) = it->second[2];
                            // leaf[i][it->second[2]][2] = y;

                            if ((jt->at(2) = leaf[i][it->second[2]][2]) != -1)
                                leaf[i][jt->at(2)][1] = y;
                            if ((jt->at(1) = it->second[2]) != -1)
                                leaf[i][jt->at(1)][2] = y;

                            for (j++; j < t && p != -1; j++) {
                                if ((it = inode[i][j].find(p))->second[2] == jt->at(1)) it->second[2] = y;
                                else break;
                                p = it->second[0];
                            }
                            j = t - 2;
                            break;
                        }
                    }
                }

//if (ff) printf("4 %d %d %d\n", leaf[i][4089][0], leaf[i][4089][1], leaf[i][4089][2]);

                if (j == t - 1) {
                    inode[i][j][p] = {-1, y, y};
                }
            }

        }


        for (int j = 0; j < t - 1; j++) {

//ff = (x == 3514 && y == 3507 && i == 47 );


            if ((kt = inode[i][j].find(y)) != inode[i][j].end() && kt->second[0] == x) {

//if (ff) printf("%d 1 ===============%d %d %d\n", j, inode[i][2][3507][0], inode[i][2][3507][1], inode[i][2][3507][2]);


//if (ff) printf("%d 2 | %d %d %d\n", j, kt->second[1], kt->second[2], leaf[i][kt->second[2]][2]);

                o1 = leaf[i][kt->second[1]][1], o2 = leaf[i][kt->second[2]][2];
                for (int k = j + 1, p = kt->second[0]; j < t && p != -1; k++) {
//if (ff) printf("%d %d ==\n", k, p);
                    if ((it = inode[i][k].find(p))->second[1] == kt->second[1]) {
                        p = it->second[0];
                        if (it->second[2] == kt->second[2]) inode[i][k].erase(it);
                        else {
//if (ff) printf("%d\n", o2);								
                            it->second[1] = o2;
                        }
                    } else if (it->second[2] == kt->second[2]) {
                        p = it->second[0];
                        it->second[2] = o1;
                    } else break;
                }

                if (o1 != -1) leaf[i][o1][2] = o2;
                if (o2 != -1) leaf[i][o2][1] = o1;

                if (eb[y].empty()) {
                    kt->second[0] = -1;
                    leaf[i][kt->second[1]][1] = leaf[i][kt->second[2]][2] = -1;
                    continue;
                } else z = eb[y][rand() % eb[y].size()];


                if ((it = inode[i][j + 1].find(z)) != inode[i][j + 1].end()) {
//if (ff) printf("%d 3\n", j);

                    kt->second[0] = z;
                    if ((o2 = leaf[i][kt->second[2]][2] = leaf[i][it->second[2]][2]) != -1)
                        leaf[i][o2][1] = kt->second[2];
                    if ((o1 = leaf[i][kt->second[1]][1] = it->second[2]) != -1)
                        leaf[i][o1][2] = kt->second[1];

                    for (int k = j + 1, p = z; k < t && p != -1; k++) {
                        if ((it = inode[i][k].find(p))->second[2] == o1) it->second[2] = kt->second[2];
                        else break;
                        p = it->second[0];
                    }

                } else {
//if (ff) printf("%d 4\n", j);

                    kt->second[0] = z;
                    leaf[i][kt->second[1]][1] = leaf[i][kt->second[2]][2] = -1;
                    int k = j + 1, p = z;
                    for (int q; k < t - 1; k++) {
                        if (eb[p].empty() || (rr = rand()) >= cc) {
                            inode[i][k][p] = {-1, kt->second[1], kt->second[2]};
                            break;
                        } else {
                            inode[i][k][p] = {q = eb[p][rr % eb[p].size()], kt->second[1], kt->second[2]};
                            p = q;
                            if ((it = inode[i][k + 1].find(p)) != inode[i][k + 1].end()) {
                                if ((o2 = leaf[i][kt->second[2]][2] = leaf[i][it->second[2]][2]) != -1)
                                    leaf[i][o2][1] = kt->second[2];
                                if ((o1 = leaf[i][kt->second[1]][1] = it->second[2]) != -1)
                                    leaf[i][o1][2] = kt->second[1];

                                // o2 = leaf[i][kt->second[1]][1] = it->second[2];
                                // leaf[i][o2][2] = kt->second[1];


                                for (k++; k < t && p != -1; k++) {
                                    if ((it = inode[i][k].find(p))->second[2] == o1) it->second[2] = kt->second[2];
                                    else break;
                                    p = it->second[0];
                                }
                                k = t - 2;
                                break;
                            }

                        }

                    }

                    if (k == t - 1) {
                        inode[i][k][p] = {-1, kt->second[1], kt->second[2]};
                    }
                }


            }

// if (ff)
// {
// int q = 3525;
// for (int p = leaf[i][q][1]; p != -1; p = leaf[i][p][1])
// 	if (p == q) 
// 	{
// 		printf("%d error\n", j);
// 		break;
// 	}
// }	



        }




// int q = 3525;
// for (int p = leaf[i][q][1]; p != -1; p = leaf[i][p][1])
// 	if (p == q)
// 	{
// 		printf("%d %d %d\n", x, y, i);
// 		sparse_hash_set<int> a1, a2;
// 		a1.set_deleted_key(-1);
// 		a2.set_deleted_key(-1);
// 		a1.insert(leaf[i][q][0]);
// 		printf("leaf\n");
// 		printf("%d %d %d %d\n", q, leaf[i][q][0], leaf[i][q][1], leaf[i][q][2]);
// 		for (int p = leaf[i][q][1]; p != q; p = leaf[i][p][1])
// 		{
// 			printf("%d %d %d %d\n", p, leaf[i][p][0], leaf[i][p][1], leaf[i][p][2]);
// 			a1.insert(leaf[i][p][0]);
// 		}

// 		for (int j = 0; j < t; j++)
// 		{
// 			a2.clear();
// 			printf("%d\n", j);
// 			for (auto it = a1.begin(); it != a1.end(); it++)
// 			{
// 				printf("%d %d %d %d\n", *it, inode[i][j][*it][0], inode[i][j][*it][1], inode[i][j][*it][2]);
// 				if (inode[i][j][*it][0] != -1) a2.insert(inode[i][j][*it][0]);
// 			}
// 			a1 = a2;
// 		}
// 		exit(0);
// 	}



// if (leaf[47][7736][1] != -1 && leaf[47][leaf[47][7736][1]][2] != 7736 )
// {
// 	printf("%d %d\n", x, y);
// 	exit(0);
// }


    }


}