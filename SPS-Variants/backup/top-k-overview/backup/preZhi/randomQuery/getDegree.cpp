#include<iostream>
#include<cstdio>
#include<algorithm>
#include<vector>
#include<set>

using namespace std;

//const int nodePerDeg = 5;
int main(int argc, char **argv) {
    if (argc != 4) {
        printf("graphname selectnum nodePerDeg\n");
        return 0;
    }
    char processedGraphPath[300];
    sprintf(processedGraphPath, "../../dataset/%s/%s.data.fmt", argv[1], argv[1]);
    char outputfile[300];
    sprintf(outputfile, "../../dataset/%s/%s.query", argv[1], argv[1]);
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
        int MAX_DEG = atoi(argv[2]);
        int nodePerDeg = atoi(argv[3]);
        FILE *fout = fopen(outputfile, "w");
        int debugCnt = 0;
        for (int i = 10; i <= MAX_DEG; i += 10) {
            unsigned int curSize = cntDegree[i].size();
            if (nodePerDeg < curSize) {
                set<int> exist;
                for (int j = 0; j < nodePerDeg; ++j) {
                    int idx = rand() % curSize;
                    int point = cntDegree[i][idx];
                    if (exist.count(point) != 0) {
                        --j;
                        continue;
                    }
                    exist.insert(point);
                    fprintf(fout, "%d\n", rvertices[point]);
                    printf("query: %d %d\n", i, rvertices[point]);
                }
            } else {
                for (int j = 0; j < nodePerDeg; j++) {
                    int point = cntDegree[i][j % curSize];
                    fprintf(fout, "%d\n", rvertices[point]);
                    printf("query: %d %d\n", i, rvertices[point]);
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
