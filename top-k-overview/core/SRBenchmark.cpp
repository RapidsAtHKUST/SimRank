#include "config.h"
#include "TSF.h"
#include "PartialSR.h"
#include "FPSR.h"
#include "KMSR.h"
#include "KMSRIndex.h"
#include "topsim.h"
#include "sparsehash/dense_hash_map"
#include "sparsehash/sparse_hash_map"
#include "meminfo.hpp"
#include "NISim.h"
#include "origNISim.h"
#include "SimMatEVD.h"
#include "SimMatSVD.h"
#include "OptSimMatSVD.h"
#include "OIPSimRank.h"
#include "ParSRSimRank.h"
#include "mymkdir.h"
#include "naiveSR.h"

using google::dense_hash_map;
using std::hash;

/* method flags. */
const char NAIVE[10] = "naive";
const char OIP_DMST[10] = "oipdmst";//Yu Weiren ICDE, OIP_DMST alg
const char PARTIAL_SR[10] = "partialsr"; //Lizorkin's algorithm

const char NI_SIM[10] = "nisim";//Kronecker simrank, EDBT alg
const char SIM_MAT[10] = "simmat";// yasuhiro.ICDE alg
const char PAR_SR[10] = "parsr";//Yu Weiren 2015VLDB, PSR alg

const char KM_SR[10] = "kmsr"; //KM algorithm
const char FP_SR[10] = "fpsr"; //FR algorithm
const char TSF_NAME[10] = "tsf"; //TSF algorithm
const char TOPSIM[10] = "topsim"; //topsim, type 1: topsim-sm; type 2: trun-topsim-sm; type 3: prio-topsim-sm;

/*default configuration */

char graph_name[125] = "AL";
char config_file[125] = "config/AL";
char method[56] = "partialsr";
bool isFm = true;
int usDisk = 0;
double initValue = 1.0;
int tsm_type = 1;
int nisim_type = 1;
int simmat_type = 1;
bool hasIndex = false;
bool buildIndex = false;
bool queryInFile = false;
int numIter = 10; // default
double decayFactor = 0.8; // default
int sampleNum = 100;
int sampleQueryNum = 20;
int Rank = 20;
int parsr_type = 0;
int DEFAULT_TOPK = 50;
int verticesNum = 456;
int edgeNum = 71959;
bool isHalf = true;
bool needOrig = false;

struct eqint {
    bool operator()(const int a, const int b) const {
        return a == b;
    }
};

/* original/reversed graph structure */
int MAX_VERTEX_NUM; /* vertex id is labeled [0, MAX_VERTEX_NUM) */
int *graph_src; /* reserved graph */
int *graph_dst;
int *orig_graph_src; /* original graph */
int *orig_graph_dst;
google::sparse_hash_map<int, int, hash<int>, eqint> vertices; /* relable vertex id */
int *rvertices; /* new vid => old vid */

/* ouput path */
char outputpath[125] = "";
FILE *fout;

/* function */
bool readConfig();

void readGraph();

bool getOutPath();

void doComputation(int vid, int k, SimRankMethod *srm);

SimRankMethod *createSimRankMethod();

void constructSinglePath(char *graph_name, char *method_level1);

void constructSinglePath(char *graph_name, char *method_level1, char *method_level2);

void constructPath(char *graph_name);

void help();

bool read_config();

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("<Usage: ./SRBenchMark parameters_files>\n");
        exit(1);
    }
    strcpy(config_file, argv[1]);
    if (!read_config()) return 0;
    printf("InputGraph=%s;\n method=%s;\n num_iter=%d;\ndecay_factor=%.4lf\n; sampleNum=%d;"
                   "\nsampleQueryNum=%d;\nis_fm=%s;\nuDisk=%d;\nhasIndex=%d;\nbuildIndex=%d;"
                   "\nneedOrig=%d;\ntopsimtype=%d;\nnisimtype=%d;\nsimmattype=%d;\n rank=%d \n\n",
           graph_name, method, numIter, decayFactor, sampleNum, sampleQueryNum, (isFm == 0 ? "false" : "true"),
           usDisk, hasIndex, buildIndex, needOrig, tsm_type, nisim_type, simmat_type, Rank);
    constructPath(graph_name);
    getOutPath();
    fout = fopen(outputpath, "wb");
    if (fout == NULL) {
        printf("failed to open output file\n");
    }

    Time timer;
    timer.start();
    readGraph();
    timer.stop();
    printf("Time cost for reading graph: %.5lf\n", timer.getElapsedTime());

    SimRankMethod *srm = createSimRankMethod();
    if (srm == NULL) return 0;
    if (buildIndex) {
        timer.reset();
        timer.start();
        srm->initialize();
        timer.stop();
        printf("Method=%s, graph = %s, udsik=%d: Time cost for build index: %.5lf\n", method, graph_name, usDisk,
               timer.getElapsedTime());
        return 0;
    }

    int qv;
    int k = DEFAULT_TOPK;
    timer.reset();
    timer.start();
    int qcnt = 0;
    if (queryInFile) {
        char querypath[125];
        sprintf(querypath, "dataset/%s/%s.query", graph_name, graph_name);
        FILE *qfp = fopen(querypath, "rb");
        printf("querypath=%s\n", querypath);
        if (qfp == NULL) {
            printf("failed to open the query file\n");
        }
        while (fscanf(qfp, "%d\n", &qv) == 1) {
            printf("Query(qv=%d, k=%d, nid=%d, deg=%d):\n", qv, k, vertices[qv],
                   graph_src[vertices[qv] + 1] - graph_src[vertices[qv]]);
            doComputation(vertices[qv], k, srm);
            qcnt++;
        }
        if (qfp != NULL)
            fclose(qfp);
    } else {
        while (scanf("%d %d", &qv, &k) != EOF) {
            printf("Query(qv=%d, k=%d, nid=%d, deg=%d):\n", qv, k, vertices[qv],
                   graph_src[vertices[qv] + 1] - graph_src[vertices[qv]]);
            doComputation(vertices[qv], k, srm);
            qcnt++;
        }
    }
    timer.stop();
    printf("Time cost for executing %d queires: %.5lf\n", qcnt, timer.getElapsedTime());

    delete srm;
    fclose(fout);
    return 0;
}

bool getOutPath() {
    //constructing output path
    char method_name[100];
    if (strcmp(method, TOPSIM) == 0) {
        switch (tsm_type) {
            case 0:
                sprintf(method_name, "topsim");
                break;
            case 1:
                sprintf(method_name, "trun_topsim");
                break;
            case 2:
                sprintf(method_name, "prio_topsim");
                break;
            default:
                printf("Invalid type %d of topsim. [valid ones: 0, 1, 2.]\n", tsm_type);
        }
        sprintf(outputpath, "dataset/%s/output/%s", graph_name, method_name);
    } else if (strcmp(method, NI_SIM) == 0) {
        switch (nisim_type) {
            case 0:
                sprintf(method_name, "orig_nisim");
                break;
            case 1:
                sprintf(method_name, "opt_nisim");
                break;
            default:
                printf("Invalid type %d of NISim. [valid ones: 0, 1.]\n", nisim_type);
        }
        sprintf(outputpath, "dataset/%s/output/%s", graph_name, method_name);
    } else if (strcmp(method, SIM_MAT) == 0) {
        switch (simmat_type) {
            case 0:
                sprintf(method_name, "simmat_evd");
                break;
            case 1:
                sprintf(method_name, "simmat_svd");
                break;
            case 2:
                sprintf(method_name, "opt_simmat_svd");
                break;
            default:
                printf("Invalid type %d of SimMat. [valid ones: 0, 1, 2.]\n", simmat_type);
        }
        sprintf(outputpath, "dataset/%s/output/%s", graph_name, method_name);
    } else if (strcmp(method, TSF_NAME) == 0) {
        switch (usDisk) {
            case 0:
                sprintf(method_name, "tsf_orig");
                break;
            case 1:
                sprintf(method_name, "tsf_usDisk");
                break;
            case 2:
                sprintf(method_name, "tsf_diskCompress");
                break;
            default:
                printf("Invalid type %d of TSF. [valid ones: 0, 1, 2.]\n", usDisk);
        }
        sprintf(outputpath, "dataset/%s/output/%s", graph_name, method_name);
    } else if (strcmp(method, PAR_SR) == 0) {
        switch (parsr_type) {
            case 0:
                sprintf(method_name, "naive_parsr");
                break;
            case 1:
                sprintf(method_name, "faster_parsr");
                break;
            default:
                printf("Invalid type %d of ParSR. [valid ones: 0, 1.]\n", parsr_type);
        }
        sprintf(outputpath, "dataset/%s/output/%s", graph_name, method_name);
    } else
        sprintf(outputpath, "dataset/%s/output/%s", graph_name, method);
    return true;
}

SimRankMethod *createSimRankMethod() {
    SimRankMethod *srm = NULL;
    if (strcmp(method, KM_SR) == 0) {
        srm = new KMSRIndex(numIter, sampleNum, decayFactor, graph_src, graph_dst,
                            MAX_VERTEX_NUM, graph_name, hasIndex, orig_graph_src, orig_graph_dst);
        //srm = new KMSR(numIter, sampleNum, decayFactor, graph_src, graph_dst, MAX_VERTEX_NUM, graph_name);
    } else if (strcmp(method, TOPSIM) == 0) {
        srm = new TopSimFamily(numIter, sampleNum, decayFactor, graph_src, graph_dst,
                               orig_graph_src, orig_graph_dst, MAX_VERTEX_NUM, tsm_type);
    } else if (strcmp(method, FP_SR) == 0) {
        srm = new FPSR(numIter, sampleNum, decayFactor, graph_src, graph_dst,
                       MAX_VERTEX_NUM, graph_name, hasIndex);
    } else if (strcmp(method, PARTIAL_SR) == 0) {
        srm = new PartialSR(numIter, decayFactor, graph_src, graph_dst, MAX_VERTEX_NUM,
                            graph_name, hasIndex, isHalf);
    } else if (strcmp(method, NAIVE) == 0) {
        srm = new NaiveSR(numIter, decayFactor, graph_src, graph_dst, MAX_VERTEX_NUM,
                          graph_name, hasIndex, isHalf);
    } else if (strcmp(method, TSF_NAME) == 0) {
        char index_path[125];
        sprintf(index_path, "dataset/%s/index/%s", graph_name, TSF_NAME);
        srm = new TSF(numIter, sampleNum, decayFactor, sampleQueryNum, graph_src, graph_dst,
                      MAX_VERTEX_NUM, usDisk, index_path, hasIndex, isFm);
    } else if (strcmp(method, NI_SIM) == 0) {
        if (nisim_type == 1)
            srm = new NISim(decayFactor, orig_graph_src, orig_graph_dst, Rank, MAX_VERTEX_NUM,
                            hasIndex, graph_name);
        else if (nisim_type == 0)
            srm = new origNISim(decayFactor, orig_graph_src, orig_graph_dst, Rank, MAX_VERTEX_NUM,
                                hasIndex, graph_name);
        else
            printf("Invalid NISim types,valid ones are [0,1]\n");
    } else if (strcmp(method, SIM_MAT) == 0) {
        if (simmat_type == 0)
            srm = new SimMatEVD(decayFactor, orig_graph_src, orig_graph_dst, Rank, MAX_VERTEX_NUM,
                                hasIndex, graph_name);
        else if (simmat_type == 1)
            srm = new SimMatSVD(decayFactor, orig_graph_src, orig_graph_dst, Rank, MAX_VERTEX_NUM,
                                hasIndex, graph_name);
        else if (simmat_type == 2)
            srm = new OptSimMatSVD(decayFactor, orig_graph_src, orig_graph_dst, Rank, MAX_VERTEX_NUM,
                                   hasIndex, graph_name);
        else {
            printf("valid type is [0=SimMatEVD,1=SimMatSVD,2=OptSimMatSVD]\n");
        }
    } else if (strcmp(method, OIP_DMST) == 0) {
        srm = new OIPSimRank(numIter, decayFactor, graph_src, graph_dst, MAX_VERTEX_NUM,
                             graph_name, hasIndex, isHalf);
    } else if (strcmp(method, PAR_SR) == 0) {
        srm = new ParSRSimRank(numIter, decayFactor, graph_src, graph_dst, orig_graph_src,
                               orig_graph_dst, MAX_VERTEX_NUM, parsr_type);
    } else {
        printf("Unsupported Method. %s. Valid ones are [naive, partialsr, oipdmst,\n nisim, parsr, simmat,\nkmsr, fpsr, tsf, topsim].\n",
               method);
    }
    return srm;
}

void doComputation(int qv, int k, SimRankMethod *srm) {
    Time timer;
    timer.start();
    srm->run(qv, k);
    timer.stop();
    printf("SRBenchMark::time for query(%d, %d): %.5lf\n", qv, k, timer.getElapsedTime());
    if (strcmp(method, FP_SR) == 0 || strcmp(method, PARTIAL_SR) == 0) {
        hasIndex = true;
    } else if (strcmp(method, TSF_NAME) == 0 && (usDisk == 1 || usDisk == 2)) {
        hasIndex = true;
    } else if (strcmp(method, NI_SIM) == 0 || strcmp(method, SIM_MAT) == 0) {
        hasIndex = true;
    }

    fwrite(&rvertices[qv], sizeof(int), 1, fout);
    fwrite(&k, sizeof(int), 1, fout);

    for (int i = 0; i < k; ++i) {
        int vid = srm->getRes(i).getVid();
        double val = srm->getRes(i).getValue();
        if (vid != -1) {
            fwrite(&rvertices[vid], sizeof(int), 1, fout);
            fwrite(&val, sizeof(double), 1, fout);
            // printf("%d %lf\n", rvertices[vid], val);
            printf("%d %lf\n", vid, val);
        } else {
            int temp_ID = -1;
            double temp_Score = -1.0;
            fwrite(&temp_ID, sizeof(int), 1, fout);
            fwrite(&temp_Score, sizeof(double), 1, fout);
            fprintf(stderr, " -1");
            printf("-1 -1\n");
        }
    }
}

int cmp(const void *a, const void *b) {
    return (*(int *) a - *(int *) b);
}

/* read the graph with reversing and relabeling the graph.*/
void readGraph() {
    char originalGraphPath[125];
    char processedGraphPath[125];
    char orig_processedGraphPath[125];

    sprintf(originalGraphPath, "dataset/%s/%s.data", graph_name, graph_name);
    sprintf(processedGraphPath, "dataset/%s/%s.data.fmt", graph_name, graph_name);
    sprintf(orig_processedGraphPath, "dataset/%s/%s.data.fmt.orig", graph_name, graph_name);

    FILE *fp = fopen(processedGraphPath, "rb");
    if (fp != NULL) {
        printf("reading from processed graph path: %s\n", processedGraphPath);

        /* read from the binary file */
        fread(&MAX_VERTEX_NUM, sizeof(int), 1, fp);
        fread(&edgeNum, sizeof(int), 1, fp);
        graph_src = new int[MAX_VERTEX_NUM + 1];
        graph_dst = new int[edgeNum];
        rvertices = new int[MAX_VERTEX_NUM];
        fread(graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
        fread(graph_dst, sizeof(int), edgeNum, fp);
        fread(rvertices, sizeof(int), MAX_VERTEX_NUM, fp);
        vertices.resize(MAX_VERTEX_NUM);
        for (int i = 0; i < MAX_VERTEX_NUM; ++i) {
            vertices[rvertices[i]] = i;
        }
        fclose(fp);
        if (needOrig == true) {
            fp = fopen(orig_processedGraphPath, "rb");
            if (fp == NULL) {
                /*construct the original version */
                orig_graph_src = new int[edgeNum];
                orig_graph_dst = new int[edgeNum];
                int *tcnt = new int[MAX_VERTEX_NUM + 1];
                memset(tcnt, 0, sizeof(int) * (MAX_VERTEX_NUM + 1));
                int et = 0;
                for (int i = 0; i < MAX_VERTEX_NUM; ++i) {
                    int s = graph_src[i];
                    int e = graph_src[i + 1];
                    if (e == s) {
                        continue;
                    }
                    for (int j = s; j < e; ++j) {
                        tcnt[graph_dst[j]]++;
                        orig_graph_src[et] = graph_dst[j];
                        orig_graph_dst[et] = i;
                        et++;
                    }
                }
                int *otmp = new int[edgeNum];
                for (int i = 1; i <= MAX_VERTEX_NUM; ++i) tcnt[i] += tcnt[i - 1];
                for (int i = 0; i < edgeNum; ++i) {
                    --tcnt[orig_graph_src[i]];
                    otmp[tcnt[orig_graph_src[i]]] = orig_graph_dst[i];
                }
                delete[] orig_graph_src;
                delete[] orig_graph_dst;
                orig_graph_src = tcnt;
                orig_graph_dst = otmp;

                fp = fopen(orig_processedGraphPath, "wb");
                fwrite(&MAX_VERTEX_NUM, sizeof(int), 1, fp);
                fwrite(&edgeNum, sizeof(int), 1, fp);
                fwrite(orig_graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
                fwrite(orig_graph_dst, sizeof(int), edgeNum, fp);
                fclose(fp);
            } else {
                int a, b;
                fread(&a, sizeof(int), 1, fp);
                fread(&b, sizeof(int), 1, fp);
                orig_graph_src = new int[MAX_VERTEX_NUM + 1];
                orig_graph_dst = new int[edgeNum];
                fread(orig_graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
                fread(orig_graph_dst, sizeof(int), edgeNum, fp);
                fclose(fp);
            }
        }
        printf("[input]EdgeNum=%d, inVertexNum=%d.\n", edgeNum, MAX_VERTEX_NUM);

    } else {
        printf("reading from original graph path: %s\n", originalGraphPath);
        fp = fopen(originalGraphPath, "r");
        int a, b;
        MAX_VERTEX_NUM = -1;
        int id = 0;
        int ecnt = 0, lcnt = 0;
        printf("require the EdgeList format.\n");

        int *cnt = new int[verticesNum + 1];
        graph_src = new int[edgeNum];
        graph_dst = new int[edgeNum];
        rvertices = new int[verticesNum];

        vertices.resize(verticesNum);

        memset(cnt, 0, sizeof(int) * (verticesNum + 1));

        printf("before reading in graph: meminfo ");
        print_mem_info();

        while (fscanf(fp, "%d %d", &a, &b) != EOF) {
            if (vertices.find(a) == vertices.end()) {
                rvertices[id] = a;
                vertices[a] = id++;
            } // relabel
            if (vertices.find(b) == vertices.end()) {
                rvertices[id] = b;
                vertices[b] = id++;
            }
            if (a == b) { lcnt++; }
            graph_src[ecnt] = vertices[b]; //reverse the graph here.
            graph_dst[ecnt] = vertices[a];
            cnt[vertices[b]]++;
            ecnt++;
        }
        edgeNum = ecnt;
        MAX_VERTEX_NUM = id;
        fclose(fp);

        printf("reading in graph: meminfo ");
        print_mem_info();

        int *tmp = new int[edgeNum];
        for (int i = 1; i <= MAX_VERTEX_NUM; ++i) cnt[i] += cnt[i - 1];
        for (int i = 0; i < edgeNum; ++i) {
            --cnt[graph_src[i]];
            tmp[cnt[graph_src[i]]] = graph_dst[i];
        }
        delete[] graph_src;
        delete[] graph_dst;
        graph_src = cnt;
        graph_dst = tmp;

        /*delete the duplicated edge here!!!*/
        if (MAX_VERTEX_NUM < 10) {
            for (int i = 0; i <= MAX_VERTEX_NUM; ++i) {
                printf("(%d, %d) ", i, graph_src[i]);
            }
            printf("\n");
            for (int i = 0; i < edgeNum; ++i) {
                printf("%d ", graph_dst[i]);
            }
            printf("\n");
        }
        int np = -1;
        for (int i = 0; i < MAX_VERTEX_NUM; ++i) {
            int s = graph_src[i];
            int e = graph_src[i + 1];
            graph_src[i] = np + 1;
            if (e == s) {
                continue;
            }
            qsort(graph_dst + s, e - s, sizeof(int), cmp);
            ++np;
            graph_dst[np] = graph_dst[s];
            for (int j = s + 1; j < e; ++j) {
                if (graph_dst[j] != graph_dst[np]) {
                    ++np;
                    graph_dst[np] = graph_dst[j];
                }
            }
        }
        printf(" mvid =%d en = %d ien=%d\n", MAX_VERTEX_NUM, np + 1, edgeNum);
        graph_src[MAX_VERTEX_NUM] = np + 1;
        edgeNum = np + 1;

        /*construct the original version */
        orig_graph_src = new int[edgeNum];
        orig_graph_dst = new int[edgeNum];
        int *tcnt = new int[MAX_VERTEX_NUM + 1];
        memset(tcnt, 0, sizeof(int) * (MAX_VERTEX_NUM + 1));
        int et = 0;
        for (int i = 0; i < MAX_VERTEX_NUM; ++i) {
            int s = graph_src[i];
            int e = graph_src[i + 1];
            if (e == s) {
                continue;
            }
            for (int j = s; j < e; ++j) {
                tcnt[graph_dst[j]]++;
                orig_graph_src[et] = graph_dst[j];
                orig_graph_dst[et] = i;
                et++;
            }
        }
        int *otmp = new int[edgeNum];
        for (int i = 1; i <= MAX_VERTEX_NUM; ++i) tcnt[i] += tcnt[i - 1];
        for (int i = 0; i < edgeNum; ++i) {
            --tcnt[orig_graph_src[i]];
            otmp[tcnt[orig_graph_src[i]]] = orig_graph_dst[i];
        }
        delete[] orig_graph_src;
        delete[] orig_graph_dst;
        orig_graph_src = tcnt;
        orig_graph_dst = otmp;

        //save graph in binary format here.
        printf("saving the graph in binary format.\n");
        fp = fopen(processedGraphPath, "wb");
        fwrite(&MAX_VERTEX_NUM, sizeof(int), 1, fp);
        fwrite(&edgeNum, sizeof(int), 1, fp);
        fwrite(graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
        fwrite(graph_dst, sizeof(int), edgeNum, fp);
        fwrite(rvertices, sizeof(int), MAX_VERTEX_NUM, fp);
        fclose(fp);
        fp = fopen(orig_processedGraphPath, "wb");
        fwrite(&MAX_VERTEX_NUM, sizeof(int), 1, fp);
        fwrite(&edgeNum, sizeof(int), 1, fp);
        fwrite(orig_graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
        fwrite(orig_graph_dst, sizeof(int), edgeNum, fp);
        fclose(fp);
        printf("[input]EdgeNum=%d, inVertexNum=%d. [online]mvid=%d, edge=%d, loop=%d\n",
               edgeNum, verticesNum, MAX_VERTEX_NUM, ecnt, lcnt);
    }

    if (MAX_VERTEX_NUM < 10) {
        for (int i = 0; i <= MAX_VERTEX_NUM; ++i) {
            printf("(%d, %d) ", i, graph_src[i]);
        }
        printf("\n");
        for (int i = 0; i < edgeNum; ++i) {
            printf("%d ", graph_dst[i]);
        }
        printf("\n");
        if (needOrig == true) {
            printf("orignal graph\n");
            for (int i = 0; i <= MAX_VERTEX_NUM; ++i) {
                printf("(%d, %d) ", i, orig_graph_src[i]);
            }
            printf("\n");
            for (int i = 0; i < edgeNum; ++i) {
                printf("%d ", orig_graph_dst[i]);
            }
            printf("\n");
        }
    }
}

void constructSinglePath(const char *graph_name, const char *method_level1) {
    /* build a index dir for a method under dir: ./dataset/graph_name/index/methodname/ */
    char temp[100];
    sprintf(temp, "dataset/%s/index/%s", graph_name, method_level1);
    light::mkpath(temp);
}

void constructSinglePath(const char *graph_name, const char *method_level1, const char *method_level2) {
    /* build a index dir for a method: dataset/graph_name/index/method_level1/method_level2 */
    char temp[100];
    sprintf(temp, "dataset/%s/index/%s/%s", graph_name, method_level1, method_level2);
    light::mkpath(temp);
}

void constructPath(char *graph_name) {
    char outputpath[100];
    sprintf(outputpath, "dataset/%s/output/", graph_name);
    light::mkpath(outputpath);

    char indexpath[100];
    sprintf(indexpath, "dataset/%s/index/", graph_name);
    light::mkpath(indexpath);

    constructSinglePath(graph_name, NAIVE);
    constructSinglePath(graph_name, PARTIAL_SR);
    constructSinglePath(graph_name, OIP_DMST);

    constructSinglePath(graph_name, NI_SIM, "OrigKron");
    constructSinglePath(graph_name, NI_SIM, "OptKron");
    constructSinglePath(graph_name, SIM_MAT, "evd");
    constructSinglePath(graph_name, SIM_MAT, "svd");
    constructSinglePath(graph_name, SIM_MAT, "optsvd");
    constructSinglePath(graph_name, PAR_SR);

    constructSinglePath(graph_name, TSF_NAME, "original");
    constructSinglePath(graph_name, TSF_NAME, "compress");
    constructSinglePath(graph_name, FP_SR);
    constructSinglePath(graph_name, KM_SR);
    constructSinglePath(graph_name, TOPSIM);

}

void help() {
    printf("App Usage:\n");
    printf("specify the configuration file:\n");
    printf("\t-c\t<string>, specify the file of configuration. [config]\n");

    printf("\n%s\n", "The following parameters can be specifies in config files.\n");
    printf("\t-qf\t<string>, indicate query in the file.\n");
    printf("common parameter For SimRank:\n");
    printf("\t-T\t<int>, the length of random walk.\n");
    printf("\t-C\t<double>, decayFactor.\n");
    printf("\t-fm\t<bool>, specify whether first-meeting guarantee or not. [true]\n");
    printf("\t-bi\t<bool>, only build index.\n");
    printf("\t-hi\t<bool>, specify the samples has been built.\n");
    printf("\t-iv\t<double>, specify the initial values. [1.0]\n");

    printf("%s\n", "parameters for different types of Simrank");
    printf("\t-sn\t<int>, specify number of global sampling. [100]\n");
    printf("\t-sqn\t<int>, specify the number of local sampling. [10]\n");
    printf("\t-rank\t<int>, specify the rank of matrix-based Simrank.[20]\n");

    printf("%s\n", "parameters for input graph");
    printf("\t-m\t<string>, specify the method of computing SimRank.\n");
    printf("\t-g\t<string>, the name of input graph.\n");
    printf("\t-en\t<int>, the number of edges in the input graph.\n");
    printf("\t-vn\t<int>, the number of vertices in the input graph.\n");

    printf("%s\n", "parameters for different types of methods");
    printf("\t-ts\t<int>, specify the type of TopSim method [0,1,2].\n");
    printf("\t-ud\t<int>, specify using disk to store the sampled graph [0=original.\n");
    printf("\t-ks\t<int>, specify the type of Kronecker SimRank. [0=original,1=optimized]\n");
    printf("\t-es\t<int>, specify the type of Efficient Simrank. [0=EVD,1=SVD,2=optimized SVD]\n");
    printf("\t-psrt\t<int>, specify the type of PSR, [0=original, 1=faster]\n");
}


//read the default configuration
bool read_config() {
    bool flag = true;
    FILE *fp = fopen(config_file, "r");
    char line[1024];
    char key[128];
    char value[128];
    char dummy[128];
    while (fgets(line, 1024, fp) != NULL) {
        sscanf(line, "%s %s #%s", key, value, dummy);
        if (strcmp(key, "-qf") == 0) {
            queryInFile = (strcmp(value, "true") == 0 ? true : false);
        } else if (strcmp("-C", key) == 0) {
            decayFactor = atof(value);
        } else if (strcmp("-topk", key) == 0) {
            DEFAULT_TOPK = atoi(value);
        } else if (strcmp(key, "-T") == 0) {
            numIter = atoi(value);
        } else if (strcmp(key, "-bi") == 0) {
            buildIndex = (strcmp(value, "true") == 0 ? true : false);
        } else if (strcmp(key, "-hi") == 0) {
            hasIndex = (strcmp(value, "true") == 0 ? true : false);
        } else if (strcmp(key, "-half") == 0) {
            isHalf = (strcmp(value, "true") == 0 ? true : false);
        } else if (strcmp(key, "-m") == 0) {
            strcpy(method, value);
        } else if (strcmp(key, "-g") == 0) {
            strcpy(graph_name, value);
        } else if (strcmp(key, "-en") == 0) {
            edgeNum = atoi(value);
        } else if (strcmp(key, "-vn") == 0) {
            verticesNum = atoi(value);
        } else if (strcmp(key, "-iv") == 0) {
            initValue = atof(value);
        } else if (strcmp(key, "-rank") == 0) {
            Rank = atoi(value);
        } else if (strcmp(key, "-fm") == 0) {
            isFm = (strcmp(value, "true") == 0 ? true : false);
        } else if (strcmp("-sn", key) == 0) {
            sampleNum = atoi(value);
        } else if (strcmp("-sqn", key) == 0) {
            sampleQueryNum = atoi(value);
        } else if (strcmp(key, "-ud") == 0) {
            usDisk = atoi(value);
        } else if (strcmp(key, "-ts") == 0) {
            tsm_type = atoi(value);
        } else if (strcmp(key, "-es") == 0) {
            simmat_type = atoi(value);
        } else if (strcmp(key, "-ks") == 0) {
            nisim_type = atoi(value);
        } else if (strcmp(key, "-psrt") == 0) {
            parsr_type = atoi(value);
        } else {
            printf("invalid parameter: %s\n", key);
            help();
            flag = false;
        }
    }
    // the following four methods need original graph.
    if (strcmp(method, TOPSIM) == 0 || strcmp(method, NI_SIM) == 0 ||
        strcmp(method, SIM_MAT) == 0 || strcmp(method, PAR_SR) == 0 ||
        strcmp(method, KM_SR) == 0) {
        needOrig = true;
    }

    if (strlen(graph_name) == 0 || strlen(method) == 0 || edgeNum == -1 || verticesNum == -1) {
        printf("No Input Graph or Testing Method! or edgeNum and vertciesNum are not specified.\n");
        help();
        flag = false;
    }

    fclose(fp);
    return flag;
}

