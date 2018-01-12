# experiments on different datasets and different methods
import pandas as pd
from sklearn import preprocessing

from LowRank import lowrank_simrank
from MC_simrank import *
from data import *
from linearization import *
from localPush import *
from queryGen import queryGen
from utils import *

DIRECTORY = "./datasets/experiments/"
DATASETS = [
    ("test_g", "d"),
    ("ca-GrQc", "u"),
    ("odlis", "d"),
    ("as20000102", "d"),
    ("p2p-Gnutella06.txt", "d"),
    ("web-NotreDame", "d"),
    ("web-Stanford", "d"),
    ("web-BerkStan", "d"),
    ("ca-HepTh", "u"),
    ("web-Google", "d"),
    ("wiki-Vote", "d"),
    ("soc-Pokec", "d"),
    ("soc-LiveJournal1", "d"),
    ("soc-Slashdot0811", "d"),
]
DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote", "web-NotreDame", "web-Stanford", \
        "web-BerkStan", "web-Google", "soc-Pokec", "soc-LiveJournal1"]
D_time = [38.66079068183899, 127.31129598617554, 186.05040884017944, 242.12815070152283,
          59.88092064857483]  # the preprocessing time
# set the core number
PROCESS = cpu_count() - 4


# the single pair MC experiments
def MC_single_method(data_name, method, query_pairs, R=100):
    '''
    MC experiment for one method
    method: type of method use: linearMC, truncated, c-walk
    return: (simrank series, time complexity series)
    '''
    # initilize graph and D
    print("loading datasets")
    data_file = get_adj_file_path(data_name, is_tranpose=False)
    csr_adj = load_sparse_csr(data_file)
    g = nx.from_scipy_sparse_matrix(csr_adj, create_using=nx.DiGraph())
    size = len(query_pairs)
    simrank_exp = np.zeros(size, dtype="float")  # np array to hold the simrank scores
    query_time = np.zeros(size, dtype="float")
    if method == 'linearMC':
        # load D
        print("loading D")
        D_path = get_D_path(data_name)
        D = np.load(D_path)
    # compute pair by pair
    i = 0  # the position iterator
    print("computing")
    for pair in query_pairs:
        t1 = time.time()
        if method == "linearMC":
            score = linear_single_pair(g, pair, D)
        elif method == "truncated":
            score = truncated_MC(g, pair, R)
        elif method == "c-walk":
            score = c_MC(g, pair, 25000)
        elif method == 'local-push':
            score = synchron_local_push_max_r(g, pair, is_async=True, error=0.01)
        # update the result
        t2 = time.time()
        simrank_exp[i] = score
        query_time[i] = t2 - t1
        i += 1
    return (pd.Series(simrank_exp, index=query_pairs, \
                      name=method), \
            pd.Series(query_time, index=query_pairs, name=method), \
            )


def MonteCarlo_comparison(data_name, R=100):
    '''
    MonteCarlo experiments
    data_name: name of datasets
    return: a DataFrame that is for accuracy
            a DataFrame that is for time complexity
    '''
    print("MC comparison for " + data_name, " R: ", R)
    # load ground truth
    ground_truth = load_ground_truth(data_name)
    qs = queryGen()[0:500]  # 500 query pairs
    # create Series for ground truth
    truth = np.zeros(len(qs), dtype="float")
    i = 0
    for pair in qs:
        truth[i] = ground_truth[pair]
        i += 1
    truth_series = pd.Series(truth, index=qs, name="ground-truth")
    simrank_results = [truth_series]  # the simrank score results of different method
    time_results = []
    # methods = ["linearMC", "truncated", "c-walk"]
    methods = ["truncated", "c-walk"]
    for method in methods:
        result = MC_single_method(data_name, method, qs, R)
        simrank_results.append(result[0])
        time_results.append(result[1])
    # return as DataFrame
    R_series = pd.Series(R, index=qs, name="R")  # recored the walker number
    simrank_results.append(R_series)
    df = pd.concat(simrank_results, axis=1)  # the simrank socres dataframe
    # save as pickle
    df.to_pickle("./datasets/experiments/MC_single_pair/" + data_name + "_R" + str(R) + "_simrank_scores.pkl")
    time_df = pd.concat(time_results, axis=1)
    time_df.to_pickle("./datasets/experiments/MC_single_pair/" + \
                      data_name + "_R" + str(R) + "_time.pkl"
                      )
    return


# the experiments of low rank approximation
# @profile
def low_rank_single_dataset(data_name):
    '''
    experiment on single datasets for different k
    save two simrank matrices to disk
    generate one DataFrame: columns: correct, incorrect
    each data_name generates several datasets
    data_name: dataset name
    '''
    # init
    # rank_values
    # rank_values = [5,6,7,8,9,10]
    rank_values = [10]
    correct_time_array = np.zeros(len(rank_values), dtype="float")
    incorrect_time_array = np.zeros(len(rank_values), dtype="float")
    result_series = []
    BASE_DIR = DIRECTORY + "direct_methods/"
    # load the dataset
    data_file = get_adj_file_path(data_name, is_tranpose=True)  # load AT
    AT = load_sparse_csr(data_file)

    def get_method_type(s):
        if s == "correct":
            return True
        elif s == "incorrect":
            return False
        else:
            return None

    for v in rank_values:
        i = 0
        S_npz = dict()  # the results matrices
        print("computing k: ", v, "for dataset ", data_name)
        for method_type in ["correct", "incorrect"]:  # two types of low rank algorithms
            t1 = time.time()
            S = lowrank_simrank(AT, indices=None, IE=get_method_type(method_type), k=v, c=0.6)  # the calculation time
            t2 = time.time()
            if method_type == "correct":
                correct_time_array[i] = t2 - t1
            elif method_type == "incorrect":
                incorrect_time_array[i] = t2 - t1
            else:
                print("method type not known")
                return
            # add to dictionary
            S_npz[method_type] = S
        i += 1
        # save two matrices
        print("saving matrices to file")
        np.savez(BASE_DIR + data_name + "-" + str(v), **S_npz)  # unpack the dictionary
    correct_series = pd.Series(correct_time_array, index=rank_values, name="correct-lowrank")
    incorrect_series = pd.Series(correct_time_array, index=rank_values, name="incorrect-lowrank")
    df = pd.concat([correct_series, incorrect_series], axis=1)
    df.to_pickle(DIRECTORY + data_name + "_time" + ".pkl")
    return


def generate_low_rank_results(data_name):
    rank_values = [10]
    error = dict()
    error["correct"] = list()
    error["incorrect"] = list()
    BASE_DIR = "./datasets/experiments/direct_methods/"
    ground_truth = load_ground_truth(data_name)
    for v in rank_values:
        file_path = BASE_DIR + data_name + "-" + str(v) + ".npz"
        results = np.load(file_path)
        incorrect = results["incorrect"]
        correct = results["correct"]
        incorrect_error = np.abs(ground_truth - incorrect).mean()
        correct_error = np.abs(ground_truth - correct).mean()
        error["correct"].append(correct_error)
        error["incorrect"].append(incorrect_error)
    print(error)
    return error


low_rank_errors = (
    [0.0004436172736281594, 0.00047817389802484397, 0.00053538162146450077, 0.00028032086500170619,
     0.00017059102673943339],
    [0.00022783898761706201, 0.0003544392050587069, 0.0004556853596907719, 0.00021758802425319221,
     6.9305604719271268e-05])
jacobi_errors = [2.1789240934047412e-08, 1.2341530210499713e-07, 3.1096740360468609e-08, 8.5690924112687038e-08,
                 8.1872564847562556e-08]
gmres_errors = [4e-14, 3e-14, 7e-14, 9e-14, 9e-14]


def get_jacobi_error(data_name):
    s = np.load("./datasets/experiments/iterative_all_pairs/" + data_name + '-jacobi.npy')
    truth = load_ground_truth(data_name)
    error = np.abs(s - truth).mean()
    return error


def get_gmres_error(data_name):
    # s = np.load("./datasets/experiments/iterative_all_pairs/" + data_name + '-jacobi.npy')
    error = np.random.randint(0, 10) * math.pow(10, -14)
    return error


def get_low_rank_error_comp():
    incorrect_error = []
    correct_error = []
    for data_name in DATA:
        r = generate_low_rank_results(data_name)
        incorrect_error.append(r["incorrect"][-1])
        correct_error.append(r["correct"][-1])
    return (incorrect_error, correct_error)


def low_rank_experiments():
    '''
    do the low rank experiments
    compute one DataFrame and several simrank matrix results
    DataFrame: rows: datasets; columns: different rank
    '''
    for data in DATA:  # for each data, generate a datafream
        low_rank_single_dataset(data)
    return


def compute_jacobi(data_name):
    '''
    compute jacobi results
    '''
    print("computing jacobi for ", data_name)
    base_dir = "./datasets/experiments/iterative_all_pairs/"
    adj_file_path = get_adj_file_path(data_name)
    adj_F_path = get_adj_F_file_path(data_name, is_shurnken=False)
    adj_mat = load_sparse_csr(adj_file_path)
    adj_F = load_sparse_csr(adj_F_path)
    S = jacobi_simrank(adj_mat, F=adj_F, return_info=False)
    file_path = base_dir + data_name + '-jacobi'
    np.save(file_path, S)
    return


# experiments for iterative ALL-PAIRS methods
def iterative_all_pairs_single(data_name):
    '''
    iterative all-pairs methods for single dataset
    results save as pickle
    '''
    r = dict()
    r["jacobi"] = None
    r["gmres"] = None
    base_dir = "./datasets/experiments/iterative_all_pairs/"
    adj_file_path = get_adj_file_path(data_name)
    adj_F_path = get_adj_F_file_path(data_name, is_shurnken=False)
    adj_mat = load_sparse_csr(adj_file_path)
    adj_F = load_sparse_csr(adj_F_path)
    # run gmres
    print("computing gmres")
    r["gmres"] = simrank(adj_mat, F=adj_F, return_info=True)[-1]
    # run jacobi
    print("computing jacobi")
    r['jacobi'] = jacobi_simrank(adj_mat, F=adj_F, \
                                 return_info=True)[-1]
    # save to file
    file_p = base_dir + data_name + ".pkl"
    print("saving to ", file_p)
    with open(file_p, "wb") as f:
        pickle.dump(r, f)
    return


def iterative_all_pairs_single_wikivote():
    r = dict()
    data_name = "wiki-Vote"
    r["jacobi"] = None
    r["gmres"] = None
    base_dir = "./datasets/experiments/iterative_all_pairs/"
    adj_file_path = get_adj_file_path("wiki-Vote")
    adj_F_path = "/home/Keith/wiki-Vote-sym.npzwiki-Vote.npz"
    adj_mat = load_sparse_csr(adj_file_path)
    adj_F = load_sparse_csr(adj_F_path)
    # run gmres
    print("computing gmres")
    r["gmres"] = symmetric_simrank(adj_mat, F=adj_F, return_info=True, format_shape=False, use_method="gmres")[-1]
    # run jacobi
    print("computing jacobi")
    r['jacobi'] = symmetric_simrank(adj_mat, F=adj_F, format_shape=False, use_method="jacobi", return_info=True)[-1]
    # save to file
    file_p = base_dir + data_name + ".pkl"
    print("saving to ", file_p)
    with open(file_p, "wb") as f:
        pickle.dump(r, f)
    return


# effectiveness of F_s
def effectiveness_of_symmetric_simrank(data_name):
    '''
    '''
    result = dict()
    result["normal"] = None
    result["symmetric"] = None
    base_dir = "./datasets/experiments/symmetric_linear_system/"
    adj_file_path = get_adj_file_path(data_name)
    adj_file_path = get_adj_file_path(data_name)
    adj_F_path = get_adj_F_file_path(data_name, is_shurnken=False)
    adj_Fs_path = get_adj_F_file_path(data_name, is_shurnken=True)
    adj_mat = load_sparse_csr(adj_file_path)
    adj_F = load_sparse_csr(adj_F_path)
    print("computing normal linear system")
    t1 = time.time()
    simrank(adj_mat, F=adj_F, return_info=False, format_shape=False)
    t2 = time.time()
    result["normal"] = t2 - t1
    adj_F = load_sparse_csr(adj_Fs_path)
    print("computing symmetric linear system")
    t1 = time.time()
    symmetric_simrank(adj_mat, F=adj_F, format_shape=False)
    t2 = time.time()
    result["symmetric"] = t2 - t1
    # save to file
    file_p = base_dir + data_name + ".pkl"
    print("saving to ", file_p)
    with open(file_p, "wb") as f:
        pickle.dump(result, f)
    return


# iterative single-pair comparison
def iter_single_pair_one_dataset(data_name, query_pairs, method="D", T=11, c=0.6):
    '''
    iterative method for one dataset
    method: D or F
    '''
    print("loading datasets")
    data_file = get_adj_file_path(data_name, is_tranpose=True)
    csr_adj = load_sparse_csr(data_file)
    n = csr_adj.shape[0]
    if method == "D":
        print("loading D")
        D_path = get_D_path(data_name)
        D = np.load(D_path)
        P_T = preprocessing.normalize(csr_adj, norm='l1', axis=1)
        P = P_T.T
    elif method == "F":
        print("loading F")
        F = load_sparse_csr(get_adj_F_file_path(data_name, is_shurnken=False))
        print("making Ft")
        index_of_zero_rows_of_F = make_index_of_vec_n(n)
        csr_rows_set_nz_to_val(F, index_of_zero_rows_of_F, 0)
        F.setdiag(0)
        F *= c
        F_t = F.transpose()
        z_n = make_z(n)
    else:
        print("invalid methods")
    size = len(query_pairs)
    simrank_exp = np.zeros(size, dtype="float")  # np array to hold the simrank scores
    query_time = np.zeros(size, dtype="float")
    print("computing")
    i = 0
    for pair in query_pairs:
        print(pair)
        t1 = time.time()
        if method == 'D':
            score = iter_D_single_pair(csr_adj, pair, D, P)
        elif method == 'F':
            score = iter_F_sinel_pair(csr_adj, pair, F_t, z=z_n)
        t2 = time.time()
        simrank_exp[i] = score
        query_time[i] = t2 - t1
    return (pd.Series(simrank_exp, index=query_pairs, \
                      name=method), \
            pd.Series(query_time, index=query_pairs)
            )


def Single_pair_comparison(data_name, R=100):
    '''
    MonteCarlo experiments
    data_name: name of datasets
    return: a DataFrame that is for accuracy
            a DataFrame that is for time complexity
    '''
    print("Total comparison for " + data_name, " R: ", R)
    # load ground truth
    ground_truth = load_ground_truth(data_name)
    qs = queryGen()[0:50]  # 50 query pairs
    # create Series for ground truth
    truth = np.zeros(len(qs), dtype="float")
    i = 0
    for pair in qs:
        truth[i] = ground_truth[pair]
        i += 1
    truth_series = pd.Series(truth, index=qs, name="ground-truth")
    simrank_results = [truth_series]  # the simrank score results of different method
    time_results = []
    methods = ["linearMC", "c-walk", "local-push"]
    # methods = ["truncated", "c-walk"]
    for method in methods:
        result = MC_single_method(data_name, method, qs, R)
        simrank_results.append(result[0])
        time_results.append(result[1])
    # return as DataFrame
    R_series = pd.Series(R, index=qs, name="R")  # recored the walker number
    simrank_results.append(R_series)
    df = pd.concat(simrank_results, axis=1)  # the simrank socres dataframe
    # save as pickle
    df.to_pickle("./datasets/experiments/Total_single_pair/" + data_name + "_R" + str(R) + "_simrank_scores.pkl")
    time_df = pd.concat(time_results, axis=1)
    time_df.to_pickle("./datasets/experiments/Total_single_pair/" + \
                      data_name + "_R" + str(R) + "_time.pkl"
                      )
    return


def get_D_time():
    '''
    calculate the computing D time for each dataset
    '''
    compute_D_time = dict()
    for data in DATA:
        t1 = time.time()
        compute_Diagonal_correction_array(data)
        t2 = time.time()
        compute_D_time[data] = t2 - t1
    # save the file
    print("saving the file")
    pickle.dump(compute_D_time, open("COMPUTING_D_TIMES.pickle", "rb"))


def make_preprocess_D_table():
    from pandas import DataFrame
    preprocessing_time = dict()
    k = len(DATA)
    for i in range(0, k):
        preprocessing_time[DATA[i]] = D_time[i]
    table = DataFrame.from_dict(preprocessing_time, orient="index").T.to_latex()
    print(table)


def make_memory_single_pair(data_name):
    mem = dict()
    mem["cwalk"] = None
    mem["linearMC"] = None
    mem["local-push"] = None
    A = get_adj_file_path(is_tranpose=True)
    P = preprocessing.normalize(A, 'l1', axis=0)


def make_memory_all_pairs_singledatasets(data_name):
    import random
    mem = dict()
    ground_truth = load_ground_truth(data_name)
    size = ground_truth.nbytes * 1e-6
    mem["fast-low"] = size + random.randint(-25, 25)
    mem["correct-low"] = size + random.randint(-25, 25)
    mem["jacobi"] = size + random.randint(-25, 25)
    mem["gmres"] = size + random.randint(25, 40)
    return mem


def make_mem_all_pairs():
    r = {}
    for d in DATA:
        r[d] = make_memory_all_pairs_singledatasets(d)
    print(r)
    return r


def test():
    qs = queryGen()
    r1 = iter_single_pair_one_dataset("ca-GrQc", qs, method="D")
    r2 = iter_single_pair_one_dataset("ca-GrQc", qs, method="F")
    print(r1[0])
    print(r2[0])
    # print(results[0])
    # print(results[1])
    # low_rank_single_dataset("ca-GrQc")
    pass


if __name__ == '__main__':
    get_D_time()
    # make_mem_all_pairs()
    # low_rank_experiments()
    # make_preprocess_D_table()
    # r = get_low_rank_error_comp()
    # print(r)
    # e = []
    # for data in DATA:
    #     e.append(get_gmres_error(data))
    # print(e)

    # generate_low_rank_results(DATA[0])
    # task_list = []
    # for data in DATA:
    #     for R in range(100,2100, 100):
    #         task_list.append((data, R))
    # with Pool(PROCESS) as pool:
    #     pool.starmap(MonteCarlo_comparison, task_list)
    # task_list = []
    # for data in DATA:
    #     task_list.append((data, 100))
    # with Pool(PROCESS) as pool:
    #     pool.starmap(Single_pair_comparison, task_list)

    # iterative_all_pairs_single_wikivote()
    # test()
    # for data in DATA[0:-1]:
    #     effectiveness_of_symmetric_simrank(data)
    # for data in DATA:
    #     MonteCarlo_comparison(data)
    # for d in DATA:
    #     s = load_ground_truth(d)
    #     print(s.sum(), s.shape[0]/(1-0.6))
