//
// Created by yche on 9/18/19.
//

#include <string>
#include <sstream>

using namespace std;

#include "util/stat.h"
#include "util/log.h"

// Should not be const to be extern variables.
string RESULT_ROOT_DIR = "/export/data/ywangby/simrank-results/";
string INPUT_DIR = "/export/data/ywangby/datasets/";

string TXT_INPUT_DIR = INPUT_DIR + "/edge_list/";
string BIN_INPUT_DIR = INPUT_DIR + "/bin_edge_list/";

string GROUND_TRUTH_DIR = RESULT_ROOT_DIR + "/ground_truth_02/";
string SLING_INDEX_DIR = RESULT_ROOT_DIR + "/sling_16";
string READSRQ_INDEX_DIR = RESULT_ROOT_DIR + "/reads_rq_17/";
string READSD_INDEX_DIR = RESULT_ROOT_DIR + "/reads_d_17/";
string TKDE17_RESULT_DIR = RESULT_ROOT_DIR + "/pcg_tkde_17/";
string LOCAL_PUSH_DIR = RESULT_ROOT_DIR + "/local_push1819/";
string CLOUD_WALKER_DIR = RESULT_ROOT_DIR + "/cloudwalker/";
string LINEAR_D_DIR = RESULT_ROOT_DIR + "linearD/";

void mkdir_if_not_exist(string &path) {
    stringstream ss;
    ss << "mkdir -p " << path;
    log_info("%s", exec(ss.str().c_str()).c_str());
}
