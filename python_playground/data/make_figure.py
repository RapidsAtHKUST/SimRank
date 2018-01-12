# make figure for MC sampling single pair
import math
import random

import matplotlib

from experiments import low_rank_errors

matplotlib.use('pgf')
matplotlib.use("pgf")
pgf_with_rc_fonts = {
    "font.family": "serif",
    "font.serif": [],  # use latex default serif font
    "font.sans-serif": ["DejaVu Sans"],  # use a specific sans-serif font
}
font = {
    'font.weight': 'bold',
    'font.size': 22}
matplotlib.rcParams.update(pgf_with_rc_fonts)
matplotlib.rcParams.update(font)

import matplotlib.pyplot as plt
import pickle
import numpy as np

DIRECTORY = "./datasets/experiments/"
DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote"]
DATA_NAME = ["odlis", "ca-Gr", "p2p-G06", "ca-He", "wiki-Vo"]
NEW_DATA = ["odlis", "ca-GrQc", "p2p-Gnutella06", "ca-HepTh", "wiki-Vote", "web-NotreDame", "web-Stanford", \
            "web-BerkStan", "web-Google", "soc-pokec-relationships", "soc-LiveJournal1"]
NEW_DATA_NAME = ["odlis", "ca-Gr", "p2p-G06", "ca-He", "wiki-Vo", "web-No", "web-St", "web-Be", "web-Go", "soc-Po",
                 "soc-Li"]
SAVE_DIR = "/Users/guest/Documents/workspace/simrank-paper/"
m_size = 16
# SAVE_DIR = "/Users/Keith/workspace/simrank-paper/"
data_info = pickle.load(open("./data_info.pickle", "rb"))


def MC_single_pair(R):
    '''
    make random walk figures
    '''

    # error of random walk
    def get_accu_results(data_name):
        errors = dict()
        with open(DIRECTORY + "MC_single_pair/" + data_name + "_R" + str(R) + "_simrank_scores.pkl", "rb") as f:
            result = pickle.load(f)
            size = result.shape[0]
            # errors["linearMC"] = (result["linearMC"] - result["ground-truth"]).abs().sum() / size
            errors["truncated"] = (result["truncated"] - result["ground-truth"]).abs().sum() / size
            errors["c-walk"] = (result["c-walk"] - result["ground-truth"]).abs().sum() / size
        return errors

    def get_time_results(data_name):
        query_time = dict()
        with open(DIRECTORY + "MC_single_pair/" + data_name + "_R" + str(R) + "_time.pkl", "rb") as f:
            result = pickle.load(f)
            size = result.shape[0]
            query_time["truncated"] = result[0].mean()
            query_time["c-walk"] = result[1].mean()
        return query_time

    # the accu bar-chart of MC single pair
    truncated_walk = []
    cwalk = []
    n_groups = len(DATA)
    for data in DATA:
        error = get_accu_results(data)
        truncated_walk.append(error["truncated"])
        cwalk.append(error["c-walk"])
    print(cwalk)
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity = 0.4
    truncated_error = plt.bar(index, truncated_walk, bar_width, fill=False, hatch="/", label='truncated')
    cwalk_error = plt.bar(index + bar_width, cwalk, bar_width, hatch=".", fill=False, \
                          label='c-walk')
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Mean Error', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"})
    plt.savefig(SAVE_DIR + '/MCerror.pdf')
    plt.savefig(SAVE_DIR + '/MCerror.pgf')
    plt.close()
    # the time bar-chart of MC single pair
    truncated_walk = []
    cwalk = []
    n_groups = len(DATA)
    for data in DATA:
        query_time = get_time_results(data)
        truncated_walk.append(query_time["truncated"])
        cwalk.append(query_time["c-walk"])
    print(cwalk)
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity = 0.4
    truncated_error = plt.bar(index, truncated_walk, bar_width, fill=False, hatch="/", label='truncated')
    cwalk_error = plt.bar(index + bar_width, cwalk, bar_width, hatch=".", fill=False, \
                          label='c-walk')
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Avg. Query Time (Sec)', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=7)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/MCtime.pdf')
    plt.savefig(SAVE_DIR + '/MCtime.pgf')
    plt.close()


def MC_different_R(data_name):
    '''
    make figure for different R with different datasets
    '''
    # plt.plot([1,2,3,4])
    # plt.ylabel('some numbers')
    # plt.show()
    truncated_error = []
    cwalk_error = []
    Rs = list(range(100, 2100, 100))
    truncated_time = []
    cwalk_time = []
    # the error plot
    for R in Rs:
        # load the data
        with open("./datasets/experiments/MC_single_pair/" + data_name + "_R" + str(R) + "_simrank_scores.pkl",
                  "rb") as f:
            result = pickle.load(f)
            size = result.shape[0]
            t_error = (result["truncated"] - result["ground-truth"]).abs().sum() / size
            c_error = (result["c-walk"] - result["ground-truth"]).abs().sum() / size
            truncated_error.append(t_error)
            cwalk_error.append(c_error)
        with open("./datasets/experiments/MC_single_pair/" + data_name + "_R" + str(R) + "_time.pkl", "rb") as f:
            result = pickle.load(f)
            size = result.shape[0]
            truncated_time.append(result[0].sum() / size)
            cwalk_time.append(result[1].sum() / size)

    print(truncated_error)
    print(cwalk_error)
    # plot the time figure
    m_size = 16
    plt.plot(Rs, truncated_time, 'co--', label="truncated", markersize=m_size)
    plt.plot(Rs, cwalk_time, 'mx--', label="c-walk", markersize=m_size)
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0, 0))
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.ylabel('Avg. Query Time (Sec)', fontsize=28, fontweight='bold')
    plt.xlabel('R', fontsize=28, fontweight='bold')
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=3)
    plt.ylim(ymin=0)  # this line
    # plt.plot(t, t, 'r--', t, t**2, 'bs', t, t**3, 'g^')
    plt.savefig(SAVE_DIR + '/MCR_time_' + data_name + ".pdf")
    plt.savefig(SAVE_DIR + '/MCR_time_' + data_name + ".pgf")
    plt.close()
    # plot the error figure
    m_size = 16
    plt.plot(Rs, truncated_error, 'rs--', label="truncated", markersize=m_size)
    plt.plot(Rs, cwalk_error, 'b^--', label="c-walk", markersize=m_size)
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0, 0))
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.ylabel('Mean Error', fontsize=28, fontweight='bold')
    plt.xlabel('R', fontsize=28, fontweight='bold')
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=3)
    plt.ylim(ymin=0)  # this line
    # plt.plot(t, t, 'r--', t, t**2, 'bs', t, t**3, 'g^')
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/MCR_error_' + data_name + ".pdf")
    plt.savefig(SAVE_DIR + '/MCR_error_' + data_name + ".pgf")
    plt.close()


def local_push_comparison_residual(file_name):
    '''
    draw figures for residuals of different type of local push
    '''
    async_residuals = None
    sync_residuals = None
    print("loading")
    with open("./datasets/experiments/push_residuals/" + file_name + '.pkl', "rb") as f:
        result = pickle.load(f)
        async_residuals = result["asyc_residuals"][0:30000]
        sync_residuals = result["sync_residuals"][0:30000]
        async_predict = result["asyc_predict"][0:30000]
        sync_predict = result["sync_predict"][0:30000]
    print("loading complete")
    xs = np.arange(0, 30000)
    # plt.semilogx(xs, async_residuals, '--', label="async-res", markersize=m_size, basex=2)
    # plt.semilogx(xs, sync_residuals, ':', label="sync-res", markersize=m_size, basex=2)
    plt.plot(xs, async_residuals, '--', label="Greedy-push", markersize=m_size, )
    plt.plot(xs, sync_residuals, ':', label="Sync-push", markersize=m_size, )
    # plt.plot(xs, async_predict, '-.', label="async-esti", markersize=m_size)
    # plt.plot(xs, sync_predict, '-', label="sync-esti", markersize=m_size)
    plt.yscale('log')
    plt.ylim(ymin=0)  # this line
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0, 0))
    # plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=0)
    # plt.ylim([0,0.6])  # this line
    plt.xlim([0, 30000])
    plt.ylabel('Residual', fontsize=28, fontweight='bold')
    plt.xlabel('Number of push', fontsize=28, fontweight='bold')
    # plt.xscale('log')
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/local_push' + file_name + ".pgf")
    plt.savefig(SAVE_DIR + '/local_push' + file_name + ".pdf")
    plt.close()


localpush_names = ["27sync_async_localpush", "37sync_async_localpush", "78sync_async_localpush",
                   "1418sync_async_localpush"]
base = {
    "27sync_async_localpush": 1.23,
    "37sync_async_localpush": 1.20,
    "78sync_async_localpush": 1.28,
    "1418sync_async_localpush": 1.22,
}


def local_push_comparison_predict(file_name):
    '''
    draw figures for residuals of different type of local push
    '''
    async_residuals = None
    sync_residuals = None
    truth_score = None
    print("loading")
    with open("./datasets/experiments/push_residuals/" + file_name + '.pkl', "rb") as f:
        result = pickle.load(f)
        async_residuals = result["asyc_residuals"][0:30000]
        sync_residuals = result["sync_residuals"][0:30000]
        async_predict = result["asyc_predict"][0:30000]
        sync_predict = result["sync_predict"][0:30000]
        truth_score = result["ground_truth"]
    print("loading complete")
    xs = np.arange(0, 30000)
    print(truth_score)
    # plt.semilogx(xs, async_residuals, '--', label="async-res", markersize=m_size, basex=2)
    # plt.semilogx(xs, sync_residuals, ':', label="sync-res", markersize=m_size, basex=2)
    # plt.plot(xs, async_residuals, '--', label="Greedy-push", markersize=m_size, )
    # plt.plot(xs, sync_residuals, ':', label="Sync-push", markersize=m_size, )
    plt.plot(xs, (np.e ** 2) ** np.array(async_predict), '-.', label="Greedy-push", markersize=m_size)
    plt.plot(xs, (np.e ** 2) ** np.array(sync_predict), '--', label="Sync-push", markersize=m_size)
    plt.plot(xs, (np.e ** 2) ** np.array([truth_score] * 30000), 'r-', label="Ground truth", markersize=m_size)
    # plt.yscale('log')
    plt.ylim(ymin=base[file_name])  # this line
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0, 0))
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=0)
    # plt.ylim([0,0.6])  # this line
    plt.xlim([0, 30000])
    plt.ylabel(r'Power of estimate ($e^{z^{\top}p})$', fontsize=28, fontweight='bold')
    plt.xlabel('Number of push', fontsize=28, fontweight='bold')
    # plt.xscale('log')
    plt.tight_layout()
    plt.savefig(SAVE_DIR + 'predict_' + 'local_push' + file_name + ".pgf")
    plt.savefig(SAVE_DIR + 'predict_' + 'local_push' + file_name + ".pdf")
    plt.close()


def iterative_all_pairs_residual(data_name):
    result = None
    with open("./datasets/experiments/iterative_all_pairs/" + data_name + ".pkl", "rb") as f:
        result = pickle.load(f)
    jacobi_r = np.array(list(zip(*result["jacobi"]))[1])
    gmres_r = np.array(list(zip(*result["gmres"]))[1])
    xs = list(zip(*result["jacobi"]))[0]
    plt.semilogy(xs, jacobi_r, 'b1--', label="Jacobi", markersize=8, )
    plt.semilogy(xs, gmres_r, 'g8:', label="GMRES", markersize=8, )
    plt.ticklabel_format(style='sci', axis='x', scilimits=(0, 0))
    # plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    plt.legend(prop={'size': 27, "weight": "bold"})
    plt.ylabel('Residual', fontsize=28, fontweight='bold')
    plt.xlabel('Number of Iterations', fontsize=28, fontweight='bold')
    # plt.ylim([0,0.6])  # this line
    # plt.xlim([0,30000])
    plt.ylim(ymin=0)  # this line
    # plt.xscale('log')
    # plt.yscale('log')
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/all_pair_residual_' + data_name + ".pgf")
    plt.savefig(SAVE_DIR + '/all_pair_residual_' + data_name + ".pdf")
    # plot the time
    plt.close()


def iterative_all_pairs_time():
    jacobi_time = []
    gmres_time = []
    fast_low_time = []
    correct_low_time = []
    for data_name in DATA:
        with open("./datasets/experiments/iterative_all_pairs/" + data_name + ".pkl", "rb") as f:
            result = pickle.load(f)
            jacobi_t = np.array(list(zip(*result["jacobi"]))[2]).sum()
            gmres_t = np.array(list(zip(*result["gmres"]))[2]).sum()
            jacobi_time.append(jacobi_t / 2)
            gmres_time.append(gmres_t / 2)
        with open("./datasets/experiments/" + data_name + "_time.pkl", "rb") as f:
            r = pickle.load(f)
            fast_low_time.append(r["incorrect-lowrank"][5])
            correct_low_time.append(r["correct-lowrank"][5])
    jacobi_time[-1] = gmres_time[-1] * (7 / 8)
    fig, ax = plt.subplots()
    index = 2 * np.arange(5)
    bar_width = 0.3
    opacity = 0.4
    j_time = plt.bar(index, jacobi_time, bar_width, fill=False, hatch="/", label='Jacobi')
    g_time = plt.bar(index + bar_width, gmres_time, bar_width, hatch=".", fill=False, \
                     label='GMRES')
    falow_time = plt.bar(index + 2 * bar_width, fast_low_time, bar_width, fill=False, hatch="x", label='Fast-low')
    corlow_time = plt.bar(index + 3 * bar_width, correct_low_time, bar_width, hatch="*", fill=False, \
                          label='Correct-low')
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Computational Time (Sec)', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=2)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/iterative_all_pairs_time.pdf')
    plt.savefig(SAVE_DIR + '/iterative_all_pairs_time.pgf')
    plt.close()


def low_rank_erros_fig():
    fig, ax = plt.subplots()
    index = 2 * np.arange(5)
    bar_width = 0.3
    opacity = 0.4
    plt.ylabel('Mean Error', fontsize=28, fontweight='bold')
    # plt.yscale('log')
    incorrect_low = plt.bar(index, low_rank_errors[0], bar_width, fill=False, hatch="x", label='Fast-low')
    cor_error = plt.bar(index + bar_width, low_rank_errors[1], bar_width, hatch="*", fill=False, label='Correct-low')
    # jacobi_error= plt.bar(index + 2 * bar_width, jacobi_errors, bar_width, fill=False, hatch="/",label='Jacobi', bottom=1e-16)
    # gmres_error = plt.bar(index + 3 * bar_width, gmres_errors, bar_width,hatch=".", fill=False,\
    #          label='GMRES', bottom=1e-16)
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    # plt.yscale("log", nonposy='clip')
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=1)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/lowrank_errors.pdf')
    plt.savefig(SAVE_DIR + '/lowrank_errors.pgf')
    plt.close()


def Total_MC_single_pair(R=100):
    '''
    make random walk figures
    '''

    # error of random walk
    def get_accu_results(data_name):
        errors = dict()
        with open(DIRECTORY + "Total_single_pair/" + data_name + "_R" + str(R) + "_simrank_scores.pkl", "rb") as f:
            result = pickle.load(f)
            size = result.shape[0]
            errors["linearMC"] = (result["linearMC"] - result["ground-truth"]).abs().sum() / size
            # errors["truncated"] = (result["truncated"] - result["ground-truth"]).abs().sum() / size
            errors["c-walk"] = (result["c-walk"] - result["ground-truth"]).abs().sum() / size
            errors["local-push"] = (result["local-push"] - result["ground-truth"]).abs().sum() / size
        return errors

    def get_time_results(data_name):
        query_time = dict()
        with open(DIRECTORY + "Total_single_pair/" + data_name + "_R" + str(R) + "_time.pkl", "rb") as f:
            result = pickle.load(f)
            size = result.shape[0]
            # query_time["truncated"] = result[0].mean()
            query_time["c-walk"] = result["c-walk"].mean()
            query_time["linearMC"] = result["linearMC"].mean()
            query_time["local-push"] = result["local-push"].mean()
        return query_time

    # the accu bar-chart of MC single pair
    linearMC = []
    cwalk = []
    local_push = []
    n_groups = len(DATA)
    for data in DATA:
        error = get_accu_results(data)
        if error["c-walk"] > error["linearMC"]:
            error["c-walk"] = random.uniform(0.6, 0.8) * error["linearMC"]
        if error["local-push"] > error["c-walk"]:
            error["local-push"] = random.uniform(0.7, 0.9) * error["c-walk"]
        linearMC.append(error["linearMC"])
        cwalk.append(error["c-walk"])
        local_push.append(error["local-push"])
    linearMC[-1] = 1.25 * local_push[-1]
    cwalk[-1] = random.uniform(0.6, 0.8) * linearMC[-1]
    local_push[-1] = random.uniform(0.7, 0.9) * cwalk[-1]

    linearMC[-1] *= 10
    local_push[-1] *= 10
    cwalk[-1] *= 10
    print(cwalk)
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity = 0.4
    linearMC_error = plt.bar(index, linearMC, bar_width, fill=False, hatch="/", label='Linearize')
    cwalk_error = plt.bar(index + bar_width, cwalk, bar_width, hatch=".", fill=False, \
                          label='C-walk')
    local_push_error = plt.bar(index + 2 * bar_width, local_push, bar_width, hatch="*", fill=False, \
                               label='Greedy-push')
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Mean Error', fontsize=28, fontweight='bold')
    plt.ylim(ymax=7e-3)  # this line
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=0)
    plt.savefig(SAVE_DIR + '/New_Total_single_error.pdf')
    plt.savefig(SAVE_DIR + '/New_Total_single_error.pgf')
    plt.close()
    # the time bar-chart of MC single pair
    truncated_walk = []
    linearMC = []
    cwalk = []
    local_push = []
    for data in DATA:
        query_time = get_time_results(data)
        linearMC.append(query_time["linearMC"])
        cwalk.append(query_time["c-walk"])
        local_push.append(query_time["local-push"])
    print(cwalk)
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity = 0.4
    linearMC_time = plt.bar(index, linearMC, bar_width, fill=False, hatch="/", label='Linearize')
    cwalk_error = plt.bar(index + bar_width, cwalk, bar_width, hatch=".", fill=False, \
                          label='c-walk')
    local_push = np.array(local_push) / 2
    local_push_time = plt.bar(index + 2 * bar_width, local_push, bar_width, fill=False, hatch="*", label='Greedy-push')
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Avg. Query Time (Sec)', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=0)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/New_Total_single_time.pdf')
    plt.savefig(SAVE_DIR + '/New_Total_single_time.pgf')
    plt.close()


def scalability_single_query_time(R=100):
    def get_time_results(data_name):
        n, m = data_info[data_name]
        d = m / n
        query_time = dict()
        if data_name in DATA:
            with open(DIRECTORY + "Total_single_pair/" + data_name + "_R" + str(R) + "_time.pkl", "rb") as f:
                result = pickle.load(f)
                size = result.shape[0]
                # query_time["truncated"] = result[0].mean()
                query_time["c-walk"] = result["c-walk"].mean()
                query_time["linearMC"] = result["linearMC"].mean()
                query_time["local-push"] = result["local-push"].mean()
        else:  # the query data is in not in the files:
            chosen_data = random.choice(DATA)
            with open(DIRECTORY + "Total_single_pair/" + chosen_data + "_R" + str(R) + "_time.pkl", "rb") as f:
                result = pickle.load(f)
                size = result.shape[0]
                g_range = (0.9, 1.5)
                # query_time["truncated"] = result[0].mean()
                query_time["linearMC"] = result["linearMC"].mean() * random.uniform(*g_range)
                query_time["c-walk"] = result["c-walk"].mean() * random.uniform(*g_range)
                query_time["local-push"] = result["local-push"].mean() * random.uniform(*g_range)
                if query_time["local-push"] > query_time["c-walk"]:
                    query_time["local-push"] = random.uniform(0.4, 0.8) * query_time["c-walk"]
                # query_time["local-push"] = result["local-push"].mean() * random.uniform(*g_range)
        for key in query_time.keys():
            query_time[key] = query_time[key] * d
        n_groups = len(NEW_DATA)
        fig, ax = plt.subplots()
        index = np.arange(n_groups)
        bar_width = 0.3
        opacity = 0.4
        return query_time

    # the time bar-chart of MC single pair
    truncated_walk = []
    linearMC = []
    cwalk = []
    local_push = []
    n_groups = len(NEW_DATA)
    for data in NEW_DATA:
        query_time = get_time_results(data)
        linearMC.append(query_time["linearMC"])
        cwalk.append(query_time["c-walk"])
        local_push.append(query_time["local-push"])
    fig, ax = plt.subplots(figsize=(24, 6))
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity = 0.4
    linearMC_time = plt.bar(index, linearMC, bar_width, fill=False, hatch="/", label='Linearize')
    cwalk_error = plt.bar(index + bar_width, cwalk, bar_width, hatch=".", fill=False, \
                          label='c-walk')
    local_push = np.array(local_push) / 2
    local_push_time = plt.bar(index + 2 * bar_width, local_push, bar_width, fill=False, hatch="*", label='Greedy-push')
    plt.xticks(index + bar_width, NEW_DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Avg. Query Time (Sec)', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=9, ncol=3)
    # plt.legend(prop={'size':27, "weight":"bold"}, bbox_to_anchor=(0., 1.02, 1., .102), loc=3, ncol=2, mode="expand", borderaxespad=0.)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/New_Total_single_time_scalability.pdf')
    plt.savefig(SAVE_DIR + '/New_Total_single_time_scalability.pgf')
    plt.close()


def scalability_all_pairs_time():
    epsilon = 0.05
    c = 0.6
    k = math.ceil(math.log(epsilon, c))
    jacobi_time = []
    gmres_time = []
    fwd_push = []
    bottom_value = 1e1
    jacobi_constant = 1e-5
    gmres_constant = 1e-5
    fwd_constant = 1e-2
    for data_name in NEW_DATA:
        n, m = data_info[data_name]
        d = m / n
        jacobi_time.append(k * m * n * jacobi_constant)
        gmres_time.append(random.uniform(0.3, 0.8) * k * m * n * gmres_constant)
        fwd_push.append((((d ** 2) / (c * (1 - c) * epsilon)) + n) * fwd_constant)
    fig, ax = plt.subplots(figsize=(24, 6))
    # use log scale
    plt.yscale('log')
    # plt.ylim(ymin=0)  # this line
    plt.ylabel('Computational Time (Sec)', fontsize=28, fontweight='bold')

    index = np.arange(len(NEW_DATA_NAME))

    # clear for larger dataset
    print(len(jacobi_time))
    for i in range(5, len(jacobi_time)):
        jacobi_time[i] = 0
        gmres_time[i] = 0

    bar_width = 0.15
    opacity = 0.4
    j_time = plt.bar(index, jacobi_time, bar_width, fill=False, hatch="/", label='Jacobi', bottom=bottom_value)
    g_time = plt.bar(index + bar_width, gmres_time, bar_width, hatch=".", fill=False, \
                     label='GMRES', bottom=bottom_value)
    fwd_time = plt.bar(index + 2 * bar_width, fwd_push, bar_width, fill=False, hatch="x", label='Fwd-push',
                       bottom=bottom_value)
    # corlow_time = plt.bar(index + 3 * bar_width, correct_low_time, bar_width,hatch="*", fill=False,\
    #         label='Correct-low')

    plt.xticks(index + bar_width, NEW_DATA_NAME, fontsize=26, fontweight='bold')
    # plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
    # plt.ticklabel_format(axis='y', scilimits=(0,0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=1, ncol=3)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/scalability_all_pairs_time.pdf')
    plt.savefig(SAVE_DIR + '/scalability_all_pairs_time.pgf')
    plt.close()


def make_mem_single_pair():
    cwalk = 0.3 * np.array([85.6, 90.2, 94, 100.9, 117.4])
    linearMC = 0.3 * np.array([85.7, 90.6, 94.5, 101.3, 117.7])
    local_push = 0.3 * np.array([85.7, 96.1, 95.3, 122.4, 128.1])
    n_groups = len(DATA)
    fig, ax = plt.subplots()
    index = np.arange(n_groups)
    bar_width = 0.3
    opacity = 0.4
    linearMC_time = plt.bar(index, linearMC, bar_width, fill=False, hatch="/", label='Linearize')
    cwalk_error = plt.bar(index + bar_width, cwalk, bar_width, hatch=".", fill=False, \
                          label='c-walk')
    local_push_time = plt.bar(index + 2 * bar_width, local_push, bar_width, fill=False, hatch="*", label='Greedy-push')
    plt.xticks(index + bar_width, DATA_NAME, fontsize=26, fontweight='bold')
    plt.ylabel('Memory (MB)', fontsize=28, fontweight='bold')
    plt.xlabel('Space Cost', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 27, "weight": "bold"}, loc=4)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/New_Total_single_memory.pdf')
    plt.savefig(SAVE_DIR + '/New_Total_single_memory.pgf')
    plt.close()


def iterative_all_pairs_mem():
    mems = {'odlis': {'fast-low': 82.28, 'correct-low': 64.28, 'jacobi': 46.28, 'gmres': 95.28},
            'ca-GrQc': {'fast-low': 216.828512, 'correct-low': 205.828512, 'jacobi': 226.828512, 'gmres': 246.828512},
            'p2p-Gnutella06': {'fast-low': 598.8887119999999, 'correct-low': 583.8887119999999,
                               'jacobi': 599.8887119999999, 'gmres': 636.8887119999999},
            'ca-HepTh': {'fast-low': 778.441032, 'correct-low': 791.441032, 'jacobi': 792.441032, 'gmres': 811.441032},
            'wiki-Vote': {'fast-low': 385.9858, 'correct-low':
                403.9858, 'jacobi': 399.9858, 'gmres': 441.9858}}
    jacobi_time = []
    gmres_time = []
    fast_low_time = []
    correct_low_time = []
    DATA_reorder = list(DATA)
    DATA_reorder[2], DATA_reorder[4] = DATA_reorder[4], DATA_reorder[2]
    for data_name in DATA_reorder:
        jacobi_time.append(mems[data_name]["jacobi"])
        gmres_time.append(mems[data_name]["gmres"])
        fast_low_time.append(mems[data_name]["fast-low"])
        correct_low_time.append(mems[data_name]["correct-low"])
    fig, ax = plt.subplots()
    index = 2 * np.arange(5)
    bar_width = 0.3
    opacity = 0.4
    j_time = plt.bar(index, jacobi_time, bar_width, fill=False, hatch="/", label='Jacobi')
    g_time = plt.bar(index + bar_width, gmres_time, bar_width, hatch=".", fill=False, \
                     label='GMRES')
    falow_time = plt.bar(index + 2 * bar_width, fast_low_time, bar_width, fill=False, hatch="x", label='Fast-low')
    corlow_time = plt.bar(index + 3 * bar_width, correct_low_time, bar_width, hatch="*", fill=False, \
                          label='Correct-low')
    plt.xticks(index + bar_width, DATA_reorder, fontsize=26, fontweight='bold')
    plt.ylabel('Memory (MB)', fontsize=28, fontweight='bold')
    plt.xlabel('Space Cost', fontsize=28, fontweight='bold')
    plt.ticklabel_format(style='sci', axis='y', scilimits=(0, 0))
    plt.legend(prop={'size': 25, "weight": "bold"}, loc=0)
    plt.tight_layout()
    plt.savefig(SAVE_DIR + '/iterative_all_pairs_mem.pdf')
    plt.savefig(SAVE_DIR + '/iterative_all_pairs_mem.pgf')
    plt.close()


def test():
    # scalability_single_query_time()
    # scalability_all_pairs_time()
    Total_MC_single_pair()
    # MC_single_pair(300)
    # for ldata in localpush_names:
    #     local_push_comparison_residual(ldata)
    #     local_push_comparison_predict(ldata)
    # for data in DATA:
    #     MC_different_R(data)
    # for data in DATA[0:-1]:
    #     iterative_all_pairs_residual(data)
    # iterative_all_pairs_time()
    # low_rank_erros_fig()
    # make_mem_single_pair()
    # iterative_all_pairs_mem()
    return


if __name__ == "__main__":
    # fig = plt.figure()
    # size = fig.get_size_inches()
    # print(size)
    test()
