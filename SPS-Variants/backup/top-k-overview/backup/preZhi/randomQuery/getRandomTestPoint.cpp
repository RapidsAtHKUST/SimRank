#include<iostream>
#include<cstdio>
#include<algorithm>
#include<vector>
#include<set>

using namespace std;

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("graphname selectnum\n");
        return 0;
    }
    char processedGraphPath[300];
    sprintf(processedGraphPath, "../../dataset/%s/%s.data.fmt", argv[1], argv[1]);
    char outputfile[300];
    sprintf(outputfile, "../../dataset/%s/%s.query", argv[1], argv[1]);
//	sprintf(outputfile, "%s.query", argv[1]);
    fprintf(stderr, "%s", argv[1]);
    char originalGraphPath[300];
    FILE *fp = fopen(processedGraphPath, "rb");
    int max_vertex_num;
    int edge_num;
    int *graphSrc;
    int *graphDst;
    int *rvertices;
    int *Degree;
    if (fp != NULL) {
        fread(&max_vertex_num, sizeof(int), 1, fp);
        fread(&edge_num, sizeof(int), 1, fp);
        graphSrc = new int[max_vertex_num + 1];
        graphDst = new int[edge_num];
        rvertices = new int[max_vertex_num];
        fread(graphSrc, sizeof(int), max_vertex_num + 1, fp);
        fread(graphDst, sizeof(int), edge_num, fp);
        fread(rvertices, sizeof(int), max_vertex_num, fp);
        int tmpmax = -1;
        Degree = new int[max_vertex_num];
        int numOfPointWithIndegree = 0;
        for (int i = 0; i < max_vertex_num; ++i) {
            int indegree = graphSrc[i + 1] - graphSrc[i];
            tmpmax = max(indegree, tmpmax);
            Degree[i] = indegree;
            if (indegree != 0) {
                ++numOfPointWithIndegree;
            }
        }
        delete[]graphSrc;
        delete[]graphDst;
        vector<vector<int> > cntDegree;
        cntDegree.resize(tmpmax + 1);
        for (int i = 0; i < max_vertex_num; ++i) {
            cntDegree[Degree[i]].push_back(i);
        }
        delete[]Degree;
        srand(time(NULL));
        //	string outputfilename = (string)(argv[1]);

        //	outputfilename = "../../dataset/"+ outputfilename + "/" + outputfilename + ".query";
        //	printf("%s\n", outputfilename);
        int totselect = atoi(argv[2]);
        FILE *fout = fopen(outputfile, "w");
//		if(fout == NULL)
//			printf("output file not opened\n");
        int debugCnt = 0;
        for (int i = 0; i <= tmpmax; ++i)
            if (cntDegree[i].size() > 0)
                printf("deg=%d cnt=%lu\n", i, cntDegree[i].size());
        for (int i = 1; i <= tmpmax; ++i) {
            unsigned int curSize = cntDegree[i].size();
            double percent = 1.0 * ((double) curSize) / (double) (1.0 * numOfPointWithIndegree);
            double tmpSelect = ((double) totselect) * percent;
            int finalSelect = (int) ((double) tmpSelect + 0.5);
            if ((unsigned int) finalSelect < curSize) {
                set<int> exist;
                for (int j = 0; j < finalSelect; ++j) {
                    int idx = rand() % curSize;
                    int point = cntDegree[i][idx];
                    if (exist.count(point) != 0) {
                        --j;
                        continue;
                    }
                    exist.insert(point);
                    /*fwrite(&rvertices[point], sizeof(int), 1, fout);*/
                    fprintf(fout, "%d\n", rvertices[point]);
                    printf("query: %d %d\n", i, rvertices[point]);
                }
            } else {
                for (int j = 0; j < curSize; ++j) {
                    int point = cntDegree[i][j];
                    fprintf(fout, "%d\n", rvertices[point]);
                    printf("query: %d %d\n", debugCnt++, rvertices[point]);
                }
            }
        }
        delete[]rvertices;
        fclose(fout);
    } else {
        printf("no format file\n");
        return 0;
    }
    fclose(fp);
    return 0;

}
