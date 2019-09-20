#pragma once

extern string RESULT_ROOT_DIR;
extern string INPUT_DIR;

extern string TXT_INPUT_DIR;
extern string BIN_INPUT_DIR;

extern string SLING_INDEX_DIR;
extern string READSRQ_INDEX_DIR;
extern string READSD_INDEX_DIR;
extern string GROUND_TRUTH_DIR;
extern string TKDE17_RESULT_DIR;
extern string LOCAL_PUSH_DIR;

void mkdir_if_not_exist(string &path);