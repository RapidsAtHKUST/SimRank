#include "../../../config.h"

#define METHOD_NUM 20 // max number of method
#define MAX_TOPK 1000 // max k for topk
#define MAX_QUERY_NUM 1000 //max query number for query in file

char graph_name[100];
char outputpath[METHOD_NUM][100];
char method_name[METHOD_NUM][100];

int exact_vid[MAX_QUERY_NUM][MAX_TOPK];
double exact_val[MAX_QUERY_NUM][MAX_TOPK];

int appro_vid[MAX_QUERY_NUM][MAX_TOPK];
double appro_val[MAX_QUERY_NUM][MAX_TOPK];

int query_num;//global variable, to record how many queries in a single file.
int topk_num = 50;
int analysis_k = 50;


double get_NDCG(int k);

int findNode(int *topk, int node, int len);//find the index of node in topk[]
double get_Precision(int k);

double get_AvgDiff(int k);

bool readExactFile(char *filename);//stores in exact_vid[], exact_val[]
bool readApproFile(char *filename);//stores in appro_vid[], appro_val[]

bool read_config();

bool topKWith_1();

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("please specify: graphname analysis_k\n");
        return 0;
    }
    strcpy(graph_name, argv[1]);
    analysis_k = atoi(argv[2]);
    char category[100];
    sprintf(category, "../../dataset/%s/output", graph_name);

    sprintf(method_name[0], "partialsr");
    sprintf(method_name[1], "oipdmst");
    sprintf(method_name[2], "faster_parsr");

    sprintf(method_name[3], "opt_nisim");

    sprintf(method_name[4], "topsim");
    sprintf(method_name[5], "trun_topsim");
    sprintf(method_name[6], "prio_topsim");

    sprintf(method_name[7], "tsf_orig");

    sprintf(method_name[8], "fpsr");
    sprintf(method_name[9], "kmsr");

    for (int i = 0; i <= 9; ++i) {
        sprintf(outputpath[i], "%s/%s", category, method_name[i]);
    }

    if (!readExactFile(outputpath[0])) {
        printf("%s\n", "file cannot be opened.");
        return 0;
    }
    if (topKWith_1()) {
        printf("%s\n", "topK is too large, please make topK smaller in config file or resample some test points.");
        return 0;
    }

    for (int fi = 1; fi <= 9; fi++) {
        if (!readApproFile(outputpath[fi])) {
            printf("%s cannot be opened\n", outputpath[fi]);
            continue;
        }
        double precision = get_Precision(analysis_k);
        double ndcg = get_NDCG(analysis_k);
        double avg_diff = get_AvgDiff(analysis_k);
        printf("%s %d %lf %lf %lf\n", method_name[fi], analysis_k, precision, ndcg, avg_diff);
    }

    return 0;
}

bool topKWith_1() {//whether
    bool flag = false;
    for (int i = 0; i < query_num; i++) {
        for (int j = 0; j < topk_num; j++) {
            if (exact_vid[i][j] == -1) {
                printf("%dth query contains -1, please remove this query\n", i);
                flag = true;
                break;
            }
        }
    }
    return flag;
}

bool readExactFile(char *filename) {
    memset(exact_vid, 0, sizeof(exact_vid));
    memset(exact_val, 0, sizeof(exact_val));
    FILE *fp = fopen(filename, "rb");
    if (fp != NULL) {
        int qv, k;
        query_num = 0;//to record the case ID
        while (fread(&qv, sizeof(int), 1, fp) != 0) {
            fread(&k, sizeof(int), 1, fp);
            for (int dataI = 0; dataI < k; dataI++) {
                fread(&exact_vid[query_num][dataI], sizeof(int), 1, fp);
                fread(&exact_val[query_num][dataI], sizeof(double), 1, fp);
            }
            query_num++;
        }
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

bool readApproFile(char *filename) {
    memset(appro_vid, 0, sizeof(appro_vid));
    memset(appro_val, 0, sizeof(appro_val));

    FILE *fp = fopen(filename, "rb");
    if (fp != NULL) {

        int qv, k;
        query_num = 0;//to record the case ID
        while (fread(&qv, sizeof(int), 1, fp) != 0) {
            fread(&k, sizeof(int), 1, fp);
            for (int dataI = 0; dataI < k; dataI++) {
                fread(&appro_vid[query_num][dataI], sizeof(int), 1, fp);
                fread(&appro_val[query_num][dataI], sizeof(double), 1, fp);
            }
            query_num++;
        }
        fclose(fp);
        return true;
    } else {
        return false;
    }
}

int findNode(int *topk, int node, int len) {//if find node in topk[], return index, else return -1.
    for (int ti = 0; ti < len; ti++) {
        if (topk[ti] == node) {
            return ti;
        }
    }
    return -1;
}

double get_Precision(int k) {
    int sum = 0;
    for (int qi = 0; qi < query_num; qi++) {//query case
        for (int ti = 0; ti < k; ti++) {//top 1, top2, top3...
            if (findNode(exact_vid[qi], appro_vid[qi][ti], k) != -1) {//intersection of first k elements
                sum++;
            }
        }
    }

    return 1.0 * sum / (k * query_num);
}

double get_AvgDiff(int k) {
    double ad = 0.0;
    int not_hit = 0;
    for (int qi = 0; qi < query_num; qi++) {
        int count = 0;
        double avgdiff = 0;
        for (int ti = 0; ti < k; ti++) {
            int exact_in_appro = findNode(appro_vid[qi], exact_vid[qi][ti], k);
            if (exact_in_appro != -1) {
                count++;
                avgdiff += abs(exact_val[qi][ti] - appro_val[qi][exact_in_appro]);
                //printf("query: %d; hit: %d\n", qi, ti);
            }
        }
        if (count == 0) {
            not_hit++;
            //printf("even not hit one\n");
        } else
            ad += avgdiff / count;
    }
    return ad / (query_num - not_hit);
}

double get_NDCG(int k) {
    double nc = 0.0;
    for (int qi = 0; qi < query_num; qi++) {
        double tmp_exact = 0;
        for (int ti = 0; ti < k; ti++) {
            tmp_exact += (pow(2, exact_val[qi][ti]) - 1) / (log(ti + 2) / log(2));
        }
        double tmp_appro = 0;
        for (int ti = 0; ti < k; ti++) {
            int appro_in_exact = findNode(exact_vid[qi], appro_vid[qi][ti], k);
            if (appro_in_exact != -1)
                tmp_appro += (pow(2, exact_val[qi][appro_in_exact]) - 1) / (log(ti + 2) / log(2));
        }
        nc += tmp_appro / tmp_exact;
    }
    return nc / query_num;
}
