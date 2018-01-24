```cpp

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
```