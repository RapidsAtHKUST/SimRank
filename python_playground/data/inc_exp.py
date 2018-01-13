# experiments on inc simrank
import os

from reads.readsd import readsd
from tabulate import tabulate

from IncSimRank import *
from queryGen import queryGen
from test_readsd import cal_reads_N

NEW_DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote", "web-NotreDame", "web-Stanford", \
            "web-BerkStan", "web-Google", "soc-pokec-relationships", "soc-LiveJournal1"]
NAME = {
    "odlis": "OD",
    "ca-GrQc": "CG",
    "p2p-Gnutella06": "PG",
    "ca-HepTh": "CH",
    "wiki-Vote": "WV",
    "web-NotreDame": "WN",
    "web-Stanford": "WS",
    "web-BerkStan": "WB",
    "web-Google": "WG",
    "soc-pokec-relationships": "PR",
    "soc-LiveJournal1": "LJ"
}
READSD_DIR = "./datasets/edge_list/"
DFG_DIR = "./datasets/dfg_index/"
LOCAL_PUSH_DIR = "./datasets/local_push_offline/"
PROCESS = cpu_count() - 4
EXP_DIR = "./inc_exp_results/"
c = 0.6
t = 10


def load_single_dfg(data_name):
    print("loading ", data_name)
    with open(data_name, "rb") as f:
        dfg = pickle.load(f)
    return dfg


def load_dfgs(data_name):
    '''
    local dfg index given a data_name
    '''
    t1 = time.time()
    print("loading dfg index for ", data_name)
    dfg_files = list()
    dfgs = list()
    for f in os.listdir(DFG_DIR):
        if data_name in f:
            dfg_files.append(DFG_DIR + f)
    print("number of index", len(dfg_files))
    with Pool(PROCESS) as pool:
        results = pool.imap_unordered(load_single_dfg, dfg_files)
        for dfg in results:
            dfgs.append(dfg)
    # for f in dfg_files:
    #     dfg = load_single_dfg(f)
    #     dfgs.append(dfg)
    t2 = time.time()
    print("total", t2 - t1, "seconds")
    return dfgs


def get_key(a, b, n):
    return str(a * n + b)


def get_pair(key, n):
    key = int(key)
    return (key // n), key % n


def get_dfgs_filenames(data_name):
    dfg_files = list()
    for f in os.listdir(DFG_DIR):
        if data_name in f:
            dfg_files.append(DFG_DIR + f)
    return dfg_files


def query_position_dfg(x):
    dfg_name, a, b = x
    # print("querying on ", dfg_name)
    with open(dfg_name, "rb") as f:
        dfg = pickle.load(f)
        t1 = time.time()
        L = np.random.geometric(1 - c) - 1
        pa = dfg.position(a, L)
        pb = dfg.position(b, L)
        t2 = time.time()
    return (pa, pb, t2 - t1)


def twf_single_pair_test(data_name):
    '''
    TWF framework for single-pair
    used methods:  TWF
    '''
    query_pairs = queryGen()
    g = load_csr_g(data_name)
    n = g.shape[0]
    m = g.nnz
    print("size of graph: ", n, m)

    @jit
    def get_key(a1, a2):
        global get_key
        return get_key(a1, a2, n)  # the disk dict only has string as keys

    @jit
    def get_pair(key):
        global get_pair
        return get_pair(key, n)

    with PersistentDict(EXP_DIR + "./TWF_single_pair_query_" + data_name + ".json", flag="c", format="json") as p:
        # dfgs = load_dfgs(data_name)
        dfg_files = get_dfgs_filenames(data_name)
        N = len(dfg_files)
        local_index = load_local_push_index(data_name)
        P = local_index["P"]
        R = local_index["R"]
        for qs in query_pairs:  # initialize
            a, b = qs
            key_ab = get_key(a, b)
            p[key_ab] = dict()
            p[key_ab]["time"] = 0
            p[key_ab]["score"] = P.get(key_ab, 0)

        count = 0
        for dfg_file in dfg_files:  # iterate over indexs
            start_time = time.time()
            gc.disable()
            dfg = load_single_dfg(dfg_file)
            gc.enable()
            for qs in query_pairs:
                a, b = qs
                key_ab = get_key(a, b)
                t1 = time.time()
                L = np.random.geometric(1 - c) - 1
                pa = dfg.position(a, L)
                pb = dfg.position(b, L)
                t2 = time.time()
                p[key_ab]["time"] += t2 - t1
                if pa != -1 and pb != -1:  # update score
                    p[key_ab]["score"] += R.get(get_key(pa, pb), 0) / ((1 - c) * N)
            end_time = time.time()
            count += 1
            print("#", count, "costs:", end_time - start_time, "estimated remaining time", \
                  (end_time - start_time) * (len(dfg_files) - count) / 3600, "h")
            # for dfg in dfgs:
            #     L = np.random.geometric(1-c) - 1
            #     pa = dfg.position(a, L)
            #     pb = dfg.position(b, L)
            #     if pa != -1 and pb != -1:
            #         residual_sum += R.get(get_key(pa,pb), 0)
            # score = P.get(key_ab, 0) + residual_sum / ((1-c) * N)
            # t2 = time.time()
            # p[key_ab]["score"] = score


def reads_single_pair_test(data_name):
    query_pairs = queryGen()
    a = load_sparse_csr(get_adj_file_path(data_name, is_tranpose=False))
    n = a.shape[0]
    print("size of graph: ", n)
    r = cal_reads_N()
    print("loading...")
    r_index = readsd(get_edge_list_path(data_name), n, r, c, t)
    with PersistentDict(EXP_DIR + "./READS_single_pair_query_" + data_name + ".json", flag="c", format="json") as p:
        for qs in query_pairs:
            print("computing", qs)
            t1 = time.time()
            a, b = qs
            key_ab = get_key(a, b, n)
            p[key_ab] = dict()
            score = r_index.queryOne(a, b)
            t2 = time.time()
            p[key_ab]["score"] = score
            p[key_ab]["time"] = t2 - t1


def offline_index_size():
    '''
    build off line building time
    two level dictionary: dfg, local_push, readsd
    '''
    with PersistentDict("./off_line_size.json", flag="c", format="json") as p:
        #  readsd size
        p["readsd"] = dict()
        p["dfg"] = dict()
        p["local_push"] = dict()
        for data in NEW_DATA:
            for f in os.listdir(READSD_DIR):
                if (not f.endswith(".txt")) and (data in f):
                    p["readsd"][data] = os.path.getsize(READSD_DIR + f)
        # dfg size
        for data in NEW_DATA:
            for f in os.listdir(DFG_DIR):
                if data in f:
                    p["dfg"].setdefault(data, 0)
                    p["dfg"][data] += os.path.getsize(DFG_DIR + f)
        # local push size
        for data in NEW_DATA:
            p["local_push"][data] = dict()
            for f in os.listdir(LOCAL_PUSH_DIR):
                if data in f and f.endswith("_P.json"):
                    p["local_push"][data]["P"] = os.path.getsize(LOCAL_PUSH_DIR + f)
                if data in f and f.endswith("_R.json"):
                    p["local_push"][data]["R"] = os.path.getsize(LOCAL_PUSH_DIR + f)


def print_fg_stat():
    l = list()
    hs = ["Graph", "$n$", "$m$", \
          "#WCCs", "Avg.Size"]
    with open("./functional_graph_state.pkl", "rb") as f:
        stats = pickle.load(f)
        for key in stats:
            n, m, wcc = stats[key]
            l.append((key, n, m, wcc, n / wcc))
    l.sort(key=lambda x: x[1])
    print(tabulate(l, headers=hs, tablefmt="latex"))
    return


def test():
    # print_fg_stat()
    data = "ca-GrQc"
    with PersistentDict(EXP_DIR + "./READS_single_pair_query_" + data + ".json", flag="r") as r, \
            PersistentDict(EXP_DIR + "./TWF_single_pair_query_" + data + ".json", flag="r") as t:
        truth = load_ground_truth(data)
        n = truth.shape[0]
        count = 0
        mean_reads = 0
        mean_twf = 0
        for key in r:
            pair = get_pair(key, n)
            reads_error = abs(truth[pair] - r[key]["score"])
            twf_error = abs(truth[pair] - t[key]["score"])
            mean_reads += reads_error / len(r)
            mean_twf += twf_error / len(r)
            if reads_error > twf_error:
                count += 1
            print(reads_error, twf_error)
        print(len(r), count)
        print(mean_reads, mean_twf)


if __name__ == "__main__":
    # offline_index_size()
    # with Pool(PROCESS) as pool:
    #     pool.map(reads_single_pair_test, NEW_DATA)
    for data in NEW_DATA[-1:]:
        twf_single_pair_test(data)
        # load_dfgs(data)
    # twf_single_pair_test(NEW_DATA[1])
    # test()
    # reads_single_pair_test(NEW_DATA[-1])
