## SimStruct.h

```cpp

	double oneHopAndRandomizedProbe(int u, string res_dir)
	{
		vector<int> nodeList;
		double* resultList = new double[g.n];
		for (int i = 0; i < g.n; i++){		
			resultList[i] = 0;
		}
		trunStep = 2;
		cout << "trunStep= " << trunStep << endl;
		clock_t ts = clock();
		// one hop deter
		oneHopDeter(u, resultList);
		for (int k = 1; k <= nr; k++)
		{
			nodeList.clear();
			int tempNode = u;
			nodeList.push_back(u);
			int nodeCount = 1;
			
			while (R.drand() < C_value){
				int length = g.getInSize(tempNode);
				if (length == 0)
					break;
				int r = R.generateRandom() % length;	
				
				int newNode = g.getInVert(tempNode, r);
				nodeList.push_back(newNode);
				tempNode = newNode;
				nodeCount++;
				if (nodeCount > trunStep + 1)
					break;

				if(nodeCount > 2)
					randomProbe(nodeList, resultList);
			}
			
		}
		clock_t te = clock();

		vector<pair<int, double> >sims;
		for (int i = 0; i < g.n; i++){
			if(i != u && resultList[i] > 0)
				sims.push_back(pair<int, double>(i, resultList[i] / (double)nr));	
		}
		sort(sims.begin(), sims.end(), comp);
		stringstream ss_out;
		ss_out << res_dir << "/" << u << ".txt";
		ofstream of_res(ss_out.str());
		for(int i=0; i<sims.size(); i++)
			of_res << sims[i].first << " " << sims[i].second << endl;
		of_res.flush(); of_res.close();
		delete[] resultList;
				
		return (te - ts) / (double)CLOCKS_PER_SEC;
	}

	int unbiasedPrefilter2(vector<int> nodeList, double* resultList){
		if(nodeList.size() - 1 > trunStep)
			return 0;
		int num_visit_nodes = 0;

		//cout << endl;
		//
		int target = nodeList.size() - 1;
		int root_node = nodeList[target];
		int ind = 0;
		H[ind][root_node] = 1;
		int Ucount = 1;
		int Ucount1 = 0;
		//int UCcount = 0;
		U[0][0] = root_node;
		Count[0][root_node] = 1;
		for (int i = 0; i < target; i++){
			for (int j = 0; j < Ucount; j++){
				int tempNode = U[ind][j];
				int outCount = g.getOutSize(tempNode);
				// prefilter
				double threshold = 1.0 / R.drand();
				//cout << "tempNode= " << tempNode << " , Count[ind][tempNode]= " << Count[ind][tempNode] << endl;

				num_visit_nodes += outCount;

				for (int k = 0; k < outCount; k++){
					int newNode = g.getOutVert(tempNode, k);
					// prefilter
					if(g.indegree[newNode] > threshold)
					{
						//cout << "newNode= " << newNode << " , indegree= " << g.indegree[newNode] << " , thres= " << threshold << endl;
						break;
					}

					if (newNode != nodeList[target - i - 1]){
						if(H[1-ind][newNode] == 0)
						{
							H[1-ind][newNode] += Count[ind][tempNode];
							Count[1-ind][newNode] = 1;
							U[1-ind][Ucount1] = newNode;
							//cout << "newNode= " << newNode << " , Ucount1= " << Ucount1 << " , Count[1-ind][newNode]= " << Count[1-ind][newNode] << endl;
							Ucount1++;
						}
						//
						else
						{
							H[1-ind][newNode] += Count[ind][tempNode];
							Count[1-ind][newNode] ++;
							//cout << "newNode= " << newNode << " , Ucount1= " << Ucount1 << " , Count[1-ind][newNode]= " << Count[1-ind][newNode] << endl;
						}
					}
				}
			}
			//cout << "..." << endl;
			for (int j = 0; j < Ucount; j++){
				H[ind][U[ind][j]] = 0;
				//
				Count[ind][U[ind][j]] = 0;
				U[ind][j] = -1;
			}
			Ucount = Ucount1;
			Ucount1 = 0;
			//UCcount = 0;
			ind = 1 - ind;
			if (Ucount == 0)
				break;
		}
		//cout << "...(2)" << endl;
		for (int i = 0; i < Ucount; i++){
			int tempNode = U[ind][i];
			if(resultList != nullptr)	//
				resultList[tempNode] += H[ind][tempNode];
			U[ind][i] = -1;
			H[ind][tempNode] = 0;
		}
		Ucount = 0;
		//cout << "...(3)" << endl;
		return num_visit_nodes;
	}

```

```cpp

	int prefilter(vector<int> nodeList, double* resultList){
		if(nodeList.size() - 1 > trunStep)
			return 0;
		int num_visit_nodes = 0;

		int target = nodeList.size() - 1;
		int root_node = nodeList[target];
		int ind = 0;
		H[ind][root_node] = 1;
		int Ucount = 1;
		int Ucount1 = 0;
		//int UCcount = 0;
		U[0][0] = root_node;
		for (int i = 0; i < target; i++){
			for (int j = 0; j < Ucount; j++){
				int tempNode = U[ind][j];
				int outCount = g.getOutSize(tempNode);
				// prefilter
				double threshold = 1.0 / R.drand();

				num_visit_nodes += outCount;

				for (int k = 0; k < outCount; k++){
					int newNode = g.getOutVert(tempNode, k);
					// prefilter
					if(g.indegree[newNode] > threshold)
					{
						//cout << "newNode= " << newNode << " , indegree= " << g.indegree[newNode] << " , thres= " << threshold << endl;
						break;
					}

					if (newNode != nodeList[target - i - 1]){
						if(H[1-ind][newNode] == 0)
						{
							H[1-ind][newNode] = 1;
							U[1-ind][Ucount1] = newNode;
							Ucount1++;
						}
					}
				}
			}
			
			for (int j = 0; j < Ucount; j++){
				H[ind][U[ind][j]] = 0;
				U[ind][j] = -1;
			}
			Ucount = Ucount1;
			Ucount1 = 0;
			//UCcount = 0;
			ind = 1 - ind;
			if (Ucount == 0)
				break;
		}
		for (int i = 0; i < Ucount; i++){
			int tempNode = U[ind][i];
			if(resultList != nullptr)	//
				resultList[tempNode] += H[ind][tempNode];
			U[ind][i] = -1;
			H[ind][tempNode] = 0;
		}
		Ucount = 0;

		return num_visit_nodes;
	}

	int unbiasedPrefilter(vector<int> nodeList, double* resultList){
		if(nodeList.size() - 1 > trunStep)
			return 0;
		int num_visit_nodes = 0;

		//cout << endl;
		//
		int target = nodeList.size() - 1;
		int root_node = nodeList[target];
		int ind = 0;
		H[ind][root_node] = 1;
		int Ucount = 1;
		int Ucount1 = 0;
		//int UCcount = 0;
		U[0][0] = root_node;
		Count[0][root_node] = 1;
		for (int i = 0; i < target; i++){
			for (int j = 0; j < Ucount; j++){
				int tempNode = U[ind][j];
				int outCount = g.getOutSize(tempNode);
				// prefilter
				double threshold = Count[ind][tempNode] / R.drand();
				//cout << "tempNode= " << tempNode << " , Count[ind][tempNode]= " << Count[ind][tempNode] << endl;

				num_visit_nodes += outCount;

				for (int k = 0; k < outCount; k++){
					int newNode = g.getOutVert(tempNode, k);
					// prefilter
					if(g.indegree[newNode] > threshold)
					{
						//cout << "newNode= " << newNode << " , indegree= " << g.indegree[newNode] << " , thres= " << threshold << endl;
						break;
					}

					if (newNode != nodeList[target - i - 1]){
						if(H[1-ind][newNode] == 0)
						{
							H[1-ind][newNode] = 1;
							Count[1-ind][newNode] = 1;
							U[1-ind][Ucount1] = newNode;
							//cout << "newNode= " << newNode << " , Ucount1= " << Ucount1 << " , Count[1-ind][newNode]= " << Count[1-ind][newNode] << endl;
							Ucount1++;
						}
						//
						else
						{
							Count[1-ind][newNode] ++;
							//cout << "newNode= " << newNode << " , Ucount1= " << Ucount1 << " , Count[1-ind][newNode]= " << Count[1-ind][newNode] << endl;
						}
					}
				}
			}
			//cout << "..." << endl;
			for (int j = 0; j < Ucount; j++){
				H[ind][U[ind][j]] = 0;
				//
				Count[ind][U[ind][j]] = 0;
				U[ind][j] = -1;
			}
			Ucount = Ucount1;
			Ucount1 = 0;
			//UCcount = 0;
			ind = 1 - ind;
			if (Ucount == 0)
				break;
		}
		//cout << "...(2)" << endl;
		for (int i = 0; i < Ucount; i++){
			int tempNode = U[ind][i];
			if(resultList != nullptr)	//
				resultList[tempNode] += H[ind][tempNode];
			U[ind][i] = -1;
			H[ind][tempNode] = 0;
		}
		Ucount = 0;
		//cout << "...(3)" << endl;
		return num_visit_nodes;
	}
```

```cpp

	void newRandomProbe(vector<int> nodeList, double* resultList){
		int target = nodeList.size() - 1;
		int root_node = nodeList[target];
		int ind = 0;
		H[ind][root_node] = 1;
		int Ucount = 1;
		int Ucount1 = 0;
		int UCcount = 0;
		U[0][0] = root_node;
		for (int i = 0; i < target; i++){
			for (int j = 0; j < Ucount; j++){
				int tempNode = U[ind][j];
				int outCount = g.getOutSize(tempNode);
				double tempMaxInSize = 1 / R.drand();
				//cout << "tempMax: " << tempMaxInSize << endl;
				for (int k = 0; k < outCount; k++){
					int newNode = g.getOutVert(tempNode, k);
				//	cout << "inSize: " << g.getInSize(newNode) << endl;
					if(g.getInSize(newNode) > tempMaxInSize){
						break;
					}
					if(H[1-ind][newNode] == 1 || newNode == nodeList[target - i - 1])
						continue;
					H[1-ind][newNode] = 1;
					U[1-ind][Ucount1++] = newNode;
				}
			}
			for (int j = 0; j < Ucount; j++){
				H[ind][U[ind][j]] = 0;
				U[ind][j] = -1;
			}
			Ucount = Ucount1;
			Ucount1 = 0;
			ind = 1 - ind;
			if (Ucount == 0)
				break;
		}
		for (int i = 0; i < Ucount; i++){
			int tempNode = U[ind][i];
			resultList[tempNode] += H[ind][tempNode];
			U[ind][i] = -1;
			H[ind][tempNode] = 0;
		}
		Ucount = 0;
	}
	double prefiltering(int u, string res_dir)
	{
		clock_t ts = clock();
		vector<int> nodeList;
		double* resultList = new double[g.n];
		for (int i = 0; i < g.n; i++){		
			resultList[i] = 0;
		}

		for (int k = 1; k <= nr; k++)
		{
			nodeList.clear();
			int tempNode = u;
			nodeList.push_back(u);
			int nodeCount = 1;
			
			while (R.drand() < C_value){
				int length = g.getInSize(tempNode);
				if (length == 0)
					break;
				int r = R.generateRandom() % length;	
				
				int newNode = g.getInVert(tempNode, r);
				nodeList.push_back(newNode);
				tempNode = newNode;
				nodeCount++;
				if (nodeCount > maxStep + 1)
					break;

				//prefilter(nodeList, resultList);
				//unbiasedPrefilter(nodeList, resultList);
				unbiasedPrefilter2(nodeList, resultList);
				//cout << "done" << endl;
			}
			
		}
		//cout << "?" << endl;
		vector<pair<int, double> >sims;
		for (int i = 0; i < g.n; i++){
			if(i != u && resultList[i] > 0)
				sims.push_back(pair<int, double>(i, resultList[i] / (double)nr));	
		}
		cout << "sims.size()= " << sims.size() << endl;
		for(int x = 0; x < sims.size(); x++)
			cout << sims[x].first << "\t" << sims[x].second << endl;
		sort(sims.begin(), sims.end(), comp);
		//cout << "??a" << endl;
		stringstream ss_out;
		//cout << "??b" << endl;
		cout << res_dir << endl;
		ss_out << res_dir << "/" << u << ".txt";
		//cout << "??c" << endl;
		ofstream of_res(ss_out.str().c_str());
		//cout << "??d" << endl;
		for(int i = 0; i < sims.size(); i++)
			of_res << sims[i].first << " " << sims[i].second << endl;
		of_res.flush(); of_res.close();
		//cout << "??" << endl;
		delete[] resultList;

		clock_t te = clock();
		//cout << "???" << endl;
		return (te - ts) / (double)CLOCKS_PER_SEC;
	}
```