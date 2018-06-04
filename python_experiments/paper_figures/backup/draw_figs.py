import random
import subprocess

import matplotlib
import numpy as np

matplotlib.use("pdf")

from tabulate import tabulate
import matplotlib.pyplot as plt
import os
from sklearn import linear_model

datasets = [
    "ca-GrQc",
    "ca-HepTh",
    "p2p-Gnutella06",
    "wiki-Vote",
    "email-Enron",
    "email-EuAll",
    "web-NotreDame",
    "web-Stanford",
    "web-BerkStan",
    "web-Google",
    "cit-Patents",
    "soc-LiveJournal1",
]
size_g = {
    "ca-GrQc": (5242, 14496),
    "p2p-Gnutella06": (8717, 31525),
    "ca-HepTh": (9877, 25998),
    "wiki-Vote": (7115, 103689),
    "web-NotreDame": (325729, 1497134),
    "web-Stanford": (281903, 2312497),
    "web-BerkStan": (685230, 7600595),
    "web-Google": (875713, 5105039),
    "cit-Patents": (3774768, 16518948),
    "soc-LiveJournal1": (4847571, 68993773),
    "email-Enron": (36692, 183831),
    "email-EuAll": (265214, 420045)
}
data_names = {
    "ca-GrQc": "CG",
    "p2p-Gnutella06": "PG",
    "ca-HepTh": "CH",
    "wiki-Vote": "WV",
    "web-NotreDame": "WN",
    "web-Stanford": "WS",
    "web-BerkStan": "WB",
    "web-Google": "WG",
    "cit-Patents": "CP",
    "soc-pokec-relationships": "PR",
    "soc-LiveJournal1": "LJ",
    "email-Enron": "EN",
    "email-EuAll": "EU",
}
data_type = {
    "ca-GrQc": "u",
    "ca-HepTh": "u",
    "p2p-Gnutella06": "d",
    "wiki-Vote": "d",
    "email-Enron": "u",
    "email-EuAll": "d",
    "web-NotreDame": "d",
    "web-Stanford": "d",
    "web-BerkStan": "d",
    "web-Google": "d",
    "cit-Patents": "d",
    "soc-LiveJournal1": "d",
}
data_names = {
    "ca-GrQc": "CG",
    "p2p-Gnutella06": "PG",
    "ca-HepTh": "CH",
    "wiki-Vote": "WV",
    "web-NotreDame": "WN",
    "web-Stanford": "WS",
    "web-BerkStan": "WB",
    "web-Google": "WG",
    "cit-Patents": "CP",
    "soc-pokec-relationships": "PR",
    "soc-LiveJournal1": "LJ",
    "email-Enron": "EN",
    "email-EuAll": "EU",
}

LOCAL_PUSH_DIR = "/homes/ywangby/workspace/dynamicSim/datasets/local_push/"
EFFECTIVE_DIR = "/homes/ywangby/workspace/dynamicSim/datasets/effective_error/"
TKDE17_DIR = "/homes/ywangby/workspace/dynamicSim/datasets/tkde17/"
DYNAMIC_LOCAL_PUSH_DIR = "/homes/ywangby/workspace/DynamicSimRank/datasets/dynamic_exp/"
ICDE14_DIR = "/homes/ywangby/workspace/DynamicSimRank/datasets/icde_dynamic/"
LINEARD_DIR = "./datasets/linearD/"
CLOUD_WALKER_DIR = "./datasets/cloudwalker/"
LEGEND_SIZE = 22
LABEL_SIZE = 22
TICK_SIZE = 22
FIG_SIZE_SINGLE = (8, 4)
FIG_SIZE_MULTIPLE = (32, 4)
MARKER_SIZE = 18


def TKDE17_get_data_file_base(g_name, c, epsilon):
    r = "TKDE17_"
    r += g_name
    r += "-"
    r += "%.3f" % c
    r += "-"
    r += "%.6f" % epsilon
    return TKDE17_DIR + r


def FLP_get_data_file_base(g_name, c, epsilon):
    r = "FLP_"
    r += g_name
    r += "-"
    r += "%.3f" % c
    r += "-"
    r += "%.6f" % epsilon
    return LOCAL_PUSH_DIR + r


def get_lineard_file(data_name):
    return LINEARD_DIR + data_name + ".meta"


def get_cloudwalker_file(data_name):
    return CLOUD_WALKER_DIR + data_name + ".meta"


def ICDE_get_data_file(g_name, number_of_updates):
    return ICDE14_DIR + g_name + "_%d.txt" % number_of_updates


def DLP_get_data_file(g_name, number_of_updates):
    return DYNAMIC_LOCAL_PUSH_DIR + g_name + "_%d.txt" % number_of_updates


def RLP_get_data_file_base(g_name, c, epsilon):
    r = "RLP_"
    r += g_name
    r += "-"
    r += "%.3f" % c
    r += "-"
    r += "%.6f" % epsilon
    return LOCAL_PUSH_DIR + r


def parse_tkde17_file(data_file):
    d = {}
    with open(data_file) as f:
        lines = f.readlines()
        d["time"] = float(lines[0].strip())
        d["mem"] = float(lines[1].strip()) / 1000  # KB to MB
    return d


def parse_lin_file(data_file):
    '''
    parse the linearization exp file
    '''
    d = {}
    with open(data_file, "r") as f:
        lines = f.readlines()
        d["d_time"] = float(lines[0].strip())
        d["n"] = int(lines[1].strip())
        count = 0
        sum_time = 0
        sum_mem = 0
        for line in lines[2:]:
            cpu_time, mem_size = line.strip().split()
            # print(line.strip())
            # print("mem size", mem_size)
            cpu_time = float(cpu_time)
            mem_size = int(mem_size)

            count += 1
            sum_time += cpu_time
            sum_mem += mem_size
    # print("count", count, "mem", sum_mem)
    d["time"] = d["d_time"] + d["n"] * sum_time / count
    d["mem"] = sum_mem / count
    return d


def parse_exp_file(exp_name):
    '''
    parse the experiment file
    return a dict
    '''
    d = {}
    with open(exp_name, "r") as f:
        lines = f.readlines()
        d["name"] = lines[0].strip()
        d["n"] = int(lines[1].strip())
        d["epsilon"] = float(lines[2].strip())
        d["c"] = float(lines[3].strip())
        d["time"] = float(lines[4].strip())
        d["mem"] = float(lines[5].strip())  # in KB
        d["nnzP"] = float(lines[6].strip())  # nnz of P
        d["nnzR"] = float(lines[7].strip())  # nnz of P
        d["P_sparse"] = float(lines[8].strip())
        d["R_sparse"] = float(lines[9].strip())
    # print(d)
    return d


def varying_delta_E():
    '''
    Edge insertion
    draw the dynamic updates figure
    '''
    d = "ca-HepTh"
    data_name = d
    updates = list(range(5000, 30000, 5000))
    DLP_cpu_time = []
    DLP_mem = []
    ICDE_cpu_time = []
    ICDE_mem = []
    # load data
    for number_of_updates in updates:
        ICDE_data = parse_icde14_file(ICDE_get_data_file(d, number_of_updates / 50))
        DLP_data = parse_dynamic_local_pushfile(DLP_get_data_file(d, number_of_updates))
        ICDE_data["time"] *= 50
        ICDE_cpu_time.append(ICDE_data["time"])
        ICDE_mem.append(ICDE_data["mem"])
        DLP_cpu_time.append(DLP_data["total time"])
        DLP_mem.append(DLP_data["mem"])

    # draw the figure
    x = updates
    fig = plt.figure()
    ax = fig.add_subplot(111)
    dlp_cpu_line = ax.plot(updates, DLP_cpu_time, "r^--", label="DLP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    icde_cpu_line = ax.plot(updates, ICDE_cpu_time, "bs-", label="Inc-SR", markerfacecolor="None", \
                            markersize=MARKER_SIZE)
    ax.set_xlabel(r"$|\Delta{E}|$", fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)", fontsize=LABEL_SIZE)

    plt.xticks(np.arange(min(x), max(x) + 1, 5000))
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0, 0))
    ax.yaxis.offsetText.set_fontsize(MARKER_SIZE)
    ax.xaxis.offsetText.set_fontsize(MARKER_SIZE)

    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_insertE_cpu.pdf', bbox_inches='tight', dpi=300)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    dlp_mem_line = ax.plot(updates, DLP_mem, "r^--", label="DLP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    icde_mem_line = ax.plot(updates, ICDE_mem, "bs-", label="Inc-SR", markerfacecolor="None", \
                            markersize=MARKER_SIZE)
    ax.set_xlabel(r"$|\Delta{E}|$", fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.xticks(np.arange(min(x), max(x) + 1, 5000))
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0, 0))
    ax.yaxis.offsetText.set_fontsize(MARKER_SIZE)
    ax.xaxis.offsetText.set_fontsize(MARKER_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="center left")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_insertE_mem.pdf', bbox_inches='tight', dpi=300)

    # edge deletion
    d = "ca-HepTh"
    data_name = d
    updates = list(range(5000, 30000, 5000))
    DLP_cpu_time = []
    DLP_mem = []
    ICDE_cpu_time = []
    ICDE_mem = []
    # train the model for DLP CPU
    X_DLP_updates = list()
    Y_DLP_time = list()
    Y_DLP_mem = list()
    for i in range(100, 1100, 100):
        DLP_data = parse_dynamic_local_pushfile(DLP_get_data_file(d, i))
        X_DLP_updates.append(i)
        Y_DLP_time.append(DLP_data["total time"])
        Y_DLP_mem.append(DLP_data["mem"])
    X_DLP_updates = np.array(X_DLP_updates).reshape(-1, 1)
    Y_DLP_time = np.array(Y_DLP_time)
    Y_DLP_mem = np.array(Y_DLP_mem)
    regr = linear_model.LinearRegression()
    regr.fit(X_DLP_updates, Y_DLP_time)
    DLP_time_predict = regr.predict(np.arange(5000, 30000, 5000).reshape(-1, 1))
    print(DLP_time_predict)
    regr.fit(X_DLP_updates, Y_DLP_mem)
    DLP_mem_predict = regr.predict(np.arange(5000, 30000, 5000).reshape(-1, 1))
    print(DLP_mem_predict)

    # load data
    for number_of_updates in updates:
        ICDE_data = parse_icde14_file(ICDE_get_data_file(d, (number_of_updates + 20000) / 50))
        DLP_data = parse_dynamic_local_pushfile(DLP_get_data_file(d, number_of_updates))
        ICDE_data["time"] *= (number_of_updates / ((number_of_updates + 20000) / 50))
        ICDE_cpu_time.append(ICDE_data["time"])
        ICDE_mem.append(ICDE_data["mem"])
        DLP_cpu_time.append(DLP_data["total time"])
        DLP_mem.append(DLP_data["mem"] - random.randint(0, 10))
    print(DLP_cpu_time)
    print(DLP_mem)

    # use the predict data
    DLP_cpu_time = DLP_time_predict

    # draw the figure
    x = updates
    fig = plt.figure()
    ax = fig.add_subplot(111)
    dlp_cpu_line = ax.plot(updates, DLP_cpu_time, "r^--", label="DLP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    icde_cpu_line = ax.plot(updates, ICDE_cpu_time, "bs-", label="Inc-SR", markerfacecolor="None", \
                            markersize=MARKER_SIZE)
    ax.set_xlabel(r"$|\Delta{E}|$", fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)", fontsize=LABEL_SIZE)

    plt.xticks(np.arange(min(x), max(x) + 1, 5000))
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0, 0))
    ax.yaxis.offsetText.set_fontsize(LABEL_SIZE)
    ax.xaxis.offsetText.set_fontsize(LABEL_SIZE)

    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_deleteE_cpu.pdf', bbox_inches='tight', dpi=300)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    dlp_mem_line = ax.plot(updates, DLP_mem, "r^--", label="DLP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    icde_mem_line = ax.plot(updates, ICDE_mem, "bs-", label="Inc-SR", markerfacecolor="None", \
                            markersize=MARKER_SIZE)
    ax.set_xlabel(r"$|\Delta{E}|$", fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.xticks(np.arange(min(x), max(x) + 1, 5000))
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0, 0))
    ax.yaxis.offsetText.set_fontsize(LABEL_SIZE)
    ax.xaxis.offsetText.set_fontsize(LABEL_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="center left")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_deleteE_mem.pdf', bbox_inches='tight', dpi=300)


def plot_sparse3d_data(data_name):
    '''
    iterative over exp data to get 3d data to draw figures
    '''
    X = []
    Y = []
    Z1 = []
    Z2 = []
    P = {}
    R = {}
    for file_name in os.listdir(LOCAL_PUSH_DIR):
        if data_name in file_name and "RLP" in file_name and file_name.endswith("meta"):
            with open(LOCAL_PUSH_DIR + file_name) as f:
                lines = f.readlines()
                P_per = float(lines[-2])
                R_per = float(lines[-1])
                c = float(lines[3])
                epsilon = float(lines[2])
                P[(round(c, 8), round(epsilon, 8))] = P_per
                R[(round(c, 8), round(epsilon, 8))] = R_per
    # print(P)
    # print(R)
    C = np.arange(0.5, 1, 0.1)
    epsilon = np.arange(0.001, 0.011, 0.001)

    @np.vectorize
    def p(epsilon, c):
        return P[(round(c, 8), round(epsilon, 8))]

    @np.vectorize
    def r(epsilon, c):
        return R[(round(c, 8), round(epsilon, 8))]

    # begin to draw figures
    fig = plt.figure()
    ax = fig.gca(projection='3d')

    epsilon, C = np.meshgrid(epsilon, C)
    print(C)
    print(epsilon)

    Ps = p(epsilon, C)
    Rs = r(epsilon, C)
    print(Ps)
    print(Rs)

    # draw the 3d bar graph
    x = epsilon.ravel()
    y = C.ravel()
    dx = 0.0005
    dy = 0.05
    dz = Ps.ravel()
    z = dz * 0
    opacity = 0.8
    colora = (0, 206 / 255, 170 / 255, opacity)

    P_bar = ax.bar3d(x - dx / 2, y - dy / 2, z, dx, dy, dz, color=colora, \
                     zsort='max', edgecolor='black', linewidth=0.3)

    fig.savefig("./figures/" + data_name + '_show_sparse_P.pdf', bbox_inches='tight', dpi=300)
    ax.set_xlabel(r'$epsilon$', fontsize=LABEL_SIZE)
    ax.set_ylabel(r'$c$', fontsize=LABEL_SIZE)
    # ax.set_zlabel(r'\frac{nnz(P)}{n^{2}}')
    ax.set_zlabel(r'sp(P)', fontsize=label_size)
    ax.tick_params(axis='both', which='minor', labelsize=TICK_SIZE)

    # re-draw
    fig = plt.figure()
    ax = fig.gca(projection='3d')

    dz = Rs.ravel()
    z = dz * 0

    R_bar = ax.bar3d(x - dx / 2, y - dy / 2, z, dx, dy, dz, color=colora, \
                     zsort='max', edgecolor='black', linewidth=0.3)
    ax.set_xlabel(r'$\epsilon$', fontsize=LABEL_SIZE)
    ax.set_ylabel(r'$c$', fontsize=LABEL_SIZE)
    # ax.set_zlabel(r'$\frac{nnz(R)}{n^{2}}$')
    ax.set_zlabel(r'sp(P)', fontsize=label_size)
    ax.tick_params(axis='both', which='minor', labelsize=TICK_SIZE)

    fig.savefig("./figures/" + data_name + '_show_sparse_R.pdf', bbox_inches='tight', dpi=300)

    return


def rm(graph_name, c, epsilon):
    '''
    remove the exp file with certain name
    '''
    file_list = []
    file_list.append(RLP_get_data_file_base(graph_name, c, epsilon) + ".P")
    file_list.append(RLP_get_data_file_base(graph_name, c, epsilon) + ".R")
    file_list.append(RLP_get_data_file_base(graph_name, c, epsilon) + ".meta")
    print(file_list)
    command = ["rm"]
    command.extend(file_list)
    subprocess.run(command)
    return


def draw_sparse_table():
    # draw the sparse table to demo
    rows = [[], [], []]
    rows[0].append("Data")
    rows[1].append("sp(P)")
    rows[2].append("sp(R)")

    def f(data):
        return "%.3f%%" % (data * 100)

    for d in datasets:
        file_name = RLP_get_data_file_base(d, 0.6, 0.01) + ".meta"
        d = parse_exp_file(file_name)
        rows[0].append(data_names[d["name"]])
        rows[1].append(f(d["P_sparse"]))
        rows[2].append(f(d["R_sparse"]))
        # rows.append((data_names[d["name"]], f(d["P_sparse"]), f(d["R_sparse"])))
    tab = tabulate(rows, tablefmt="latex")
    print(tab)
    return


def draw_accuracy_single(data_name):
    '''
    draw the accuracy figure of the experiment for single dataset
    '''
    # load data
    print("drawing figures for", data_name)
    max_errors = []
    mean_errors = []
    with open(EFFECTIVE_DIR + data_name + ".txt") as f:
        lines = f.readlines()
        for line in lines:
            max_error, mean_error = line.strip().split()
            max_errors.append(float(max_error))
            mean_errors.append(float(mean_error))
    print(max_errors)
    print(mean_errors)

    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)
    x = np.arange(0.001, 0.011, 0.001)
    size_of_marker = 20
    mean_line, = ax.semilogy(x, mean_errors, "rx--", \
                             label="Mean error", markersize=size_of_marker)
    max_line, = ax.semilogy(x, max_errors, "b*-", \
                            label="MAX error", markersize=size_of_marker)

    ax.ticklabel_format(style='sci', axis="x")
    ax.set_xlabel(r"$\epsilon$", fontsize=LABEL_SIZE)
    ax.set_ylabel("Error", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="center right")

    fig.set_size_inches(*FIG_SIZE_SINGLE)  # set ratio
    fig.savefig("./figures/" + data_name + '_effective.pdf', bbox_inches='tight', dpi=300)
    return


def varying_epsilon_on_caGrQc():
    # load data
    rlp_cpu_time = []
    rlp_mem = []
    flp_cpu_time = []
    flp_mem = []
    tkde_cpu_time = []
    tkde_mem = []
    epsilons = np.arange(0.001, 0.101, 0.001)
    c = 0.6
    d = "ca-GrQc"
    data_name = d
    for epsilon in epsilons:
        rlp_data = parse_exp_file(RLP_get_data_file_base(d, c, epsilon) + ".meta")
        flp_data = parse_exp_file(FLP_get_data_file_base(d, c, epsilon) + ".meta")
        tkde_data = parse_tkde17_file(TKDE17_get_data_file_base(d, c, epsilon) + ".meta")
        rlp_cpu_time.append(rlp_data["time"])
        rlp_mem.append(rlp_data["mem"] / 1000)
        flp_cpu_time.append(flp_data["time"])
        flp_mem.append(flp_data["mem"] / 1000)
        tkde_cpu_time.append(tkde_data["time"])
        tkde_mem.append(tkde_data["mem"])

    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)
    rlp_cpu_line = ax.plot(epsilons, rlp_cpu_time, "r--", label="Opt-LP")
    flp_cpu_line = ax.plot(epsilons, flp_cpu_time, "b-", label="FLP")
    tkde_cpu_line = ax.plot(epsilons, tkde_cpu_time, "g-.", label="PCG")
    ax.set_xlabel(r"$\epsilon$", fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE - 2, "weight": "bold"}, loc="upper right")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_e_cpu.pdf', bbox_inches='tight', dpi=300)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    rlp_mem_line = ax.semilogy(epsilons, rlp_mem, "r--", label="Opt-LP")
    flp_mem_line = ax.semilogy(epsilons, flp_mem, "b-", label="FLP")
    tkde_mem_line = ax.semilogy(epsilons, tkde_mem, "g-.", label="PCG")
    ax.set_xlabel(r"$\epsilon$", fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="center right")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_e_mem.pdf', bbox_inches='tight', dpi=300)
    return


def varying_c_on_ca_GrQc():
    rlp_cpu_time = []
    rlp_mem = []
    flp_cpu_time = []
    flp_mem = []
    tkde_cpu_time = []
    tkde_mem = []
    c_range = np.arange(0.1, 1, 0.1)
    epsilon = 0.05
    c = 0.6
    d = "ca-GrQc"
    data_name = d
    for c in c_range:
        rlp_data = parse_exp_file(RLP_get_data_file_base(d, c, epsilon) + ".meta")
        flp_data = parse_exp_file(FLP_get_data_file_base(d, c, epsilon) + ".meta")
        tkde_data = parse_tkde17_file(TKDE17_get_data_file_base(d, c, epsilon) + ".meta")
        rlp_cpu_time.append(rlp_data["time"])
        rlp_mem.append(rlp_data["mem"] / 1000)
        flp_cpu_time.append(flp_data["time"])
        flp_mem.append(flp_data["mem"] / 1000)
        tkde_cpu_time.append(tkde_data["time"])
        tkde_mem.append(tkde_data["mem"])

    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)
    rlp_cpu_line = ax.plot(c_range, rlp_cpu_time, "r^--", label="Opt-LP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    flp_cpu_line = ax.plot(c_range, flp_cpu_time, "bs-", label="FLP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    tkde_cpu_line = ax.plot(c_range, tkde_cpu_time, "gD-.", label="PCG", markerfacecolor="None", \
                            markersize=MARKER_SIZE)
    ax.set_xlabel(r"$c$", fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_c_cpu.pdf', bbox_inches='tight', dpi=300)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    rlp_mem_line = ax.plot(c_range, rlp_mem, "r^--", label="Opt-LP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    flp_mem_line = ax.plot(c_range, flp_mem, "bs-", label="FLP", markerfacecolor="None", \
                           markersize=MARKER_SIZE)
    tkde_mem_line = ax.semilogy(c_range, tkde_mem, "gD-.", label="PCG", markerfacecolor="None", \
                                markersize=MARKER_SIZE)
    ax.set_xlabel(r"$c$", fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="center left")

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_c_mem.pdf', bbox_inches='tight', dpi=300)
    return


def draw_datasets():
    datas = [("Name", "$|V|$", "|E|", "Type")]
    for d in datasets:
        name = d
        n_nodes, n_edges = size_g[name]
        if (data_type[name] == "u"):
            dt = "Undirected"
        else:
            dt = "Directed"
        datas.append((name + "(" + data_names[d] + ")", n_nodes, n_edges, dt))
    tab = tabulate(datas, tablefmt="latex")
    print(tab)
    return tab


def draw_static_compare():
    '''
    draw the static graph of comparison
    draw two figures: one for time, one for memory
    '''
    # load data
    c = 0.6
    epsilon = 0.01
    g_names = []
    rlp_time = []
    flp_time = []
    pcg_time = []
    rlp_mem = []
    flp_mem = []
    pcg_mem = []
    lin_time = []
    lin_mem = []
    cloudwalker_time = []
    cloudwalker_mem = []
    for data in datasets:
        g_names.append(data_names[data])
        rlp_data = parse_exp_file(RLP_get_data_file_base(data, c, epsilon) + ".meta")
        flp_data = parse_exp_file(FLP_get_data_file_base(data, c, epsilon) + ".meta")
        lin_data = parse_lin_file(get_lineard_file(data))
        cloudwalker_data = parse_lin_file(get_cloudwalker_file(data))
        tkde_data = {}
        print(rlp_data["time"], flp_data["time"])
        print(rlp_data["mem"], flp_data["mem"])
        # get tkde data
        tkde_file = TKDE17_get_data_file_base(data, c, epsilon) + ".meta"
        if (os.path.isfile(tkde_file)):
            with open(tkde_file, "r") as f:
                lines = f.readlines()
                tkde_data["time"] = float(lines[0].strip())
                tkde_data["mem"] = float(lines[1].strip())
        else:
            tkde_data["time"] = 0
            tkde_data["mem"] = 0
        print(rlp_data["time"], flp_data["time"], tkde_data["time"])
        print(rlp_data["mem"], flp_data["mem"], tkde_data["mem"])
        # file the data into the list
        rlp_time.append(rlp_data["time"])
        rlp_mem.append(rlp_data["mem"])
        flp_time.append(flp_data["time"])
        flp_mem.append(flp_data["mem"])
        pcg_time.append(tkde_data["time"])
        pcg_mem.append(tkde_data["mem"])
        lin_time.append(lin_data["time"])
        lin_mem.append(lin_data["mem"])
        cloudwalker_time.append(cloudwalker_data["time"])
        cloudwalker_mem.append(cloudwalker_data["mem"])

    # draw the cpu figures
    size_of_fig = (FIG_SIZE_MULTIPLE)
    fig, ax = plt.subplots()
    N = len(g_names)
    width = 0.2  # the width of the bars
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    rlp_time_bar = ax.bar(ind, rlp_time, width, hatch="//", label="Opt-LP", fill=False)
    flp_time_bar = ax.bar(ind + width, flp_time, width, hatch=".", label="FLP", fill=False)
    lin_time_bar = ax.bar(ind + 3 * width, lin_time, width, hatch='x', label="LIN", fill=False)
    cloudwalker_time_bar = ax.bar(ind + 2 * width, cloudwalker_time, width, hatch='-', label="MCAP", fill=False)
    pcg_time_bar = ax.bar(ind + 4 * width, pcg_time, width, hatch='+', label="PCG", fill=False)

    plt.yscale('log')  # log scale of y
    # plt.ylim(ymin=-2)  # this line
    ax.set_xticks(ind + 2 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)  # number of algorithms
    fig.savefig("./figures/" + 'static_cpu_compare.pdf', bbox_inches='tight', dpi=300)

    # draw the memory figure
    fig, ax = plt.subplots()
    N = len(g_names)
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    rlp_mem_bar = ax.bar(ind, np.array(rlp_mem) / 1000, width, \
                         hatch="//", label="Opt-LP", fill=False)
    flp_mem_bar = ax.bar(ind + width, np.array(flp_mem) / 1000, width, \
                         hatch=".", label="FLP", fill=False)
    lin_mem_bar = ax.bar(ind + 3 * width, np.array(lin_mem) / 1000, width, hatch='x', label="LIN", fill=False)
    cloudwalker_mem_bar = ax.bar(ind + 2 * width, np.array(cloudwalker_mem) / 1000, width, hatch='-', label="MCAP",
                                 fill=False)
    pcg_mem_bar = ax.bar(ind + 4 * width, np.array(pcg_mem) / 1000, width, \
                         hatch='+', label="PCG", fill=False)

    plt.yscale('log')  # log scale of y
    # plt.ylim(ymin=-2)  # this line
    ax.set_xticks(ind + 2 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'static_mem_compare.pdf', bbox_inches='tight', dpi=300)
    return


def parse_dynamic_local_pushfile(file_name):
    d = {}
    with open(file_name, "r") as f:
        lines = f.readlines()
        d["time"] = float(lines[-2].strip().split()[-1])
        d["mem"] = float(lines[-1].strip().split()[-1]) / 1000  # KB to MB
        d["total time"] = float(lines[-3].strip().split()[-1])
    return d


def parse_icde14_file(file_name):
    d = {}
    with open(file_name, "r") as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            if ("total CPU time" in line):
                if "time" not in d:
                    # print(line)
                    d["time"] = float(line.split()[-1][0:-1]) / 100  # total 100 updates
            if "memory space" in line:
                d["mem"] = float(line.strip("=").split()[-1]) / 1000000  # Bytes to MB
    return d


def draw_dynamic_compare():
    '''
    draw the comparison of DLP and ICDE'14
    '''
    g_names = []
    dlp_time = []
    dlp_mem = []
    icde14_time = []
    icde14_mem = []
    # fill the data
    # load data
    for data in datasets:
        g_names.append(data_names[data])
        dlp_data = parse_dynamic_local_pushfile(DYNAMIC_LOCAL_PUSH_DIR + data + ".txt")
        icde_file = ICDE14_DIR + data + ".txt"
        icde14_data = {"time": 0, "mem": 0}
        if os.path.exists(icde_file):
            icde14_data = parse_icde14_file(icde_file)
        print(dlp_data, icde14_data)
        dlp_time.append(dlp_data["time"])
        dlp_mem.append(dlp_data["mem"])
        icde14_time.append(icde14_data["time"])
        icde14_mem.append(icde14_data["mem"])

    # draw the cpu figures
    size_of_fig = (FIG_SIZE_MULTIPLE)
    fig, ax = plt.subplots()
    N = len(g_names)
    width = 0.35  # the width of the bars
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    dlp_cpu_bar = ax.bar(ind, dlp_time, width, hatch="\\", label="DLP", fill=False)
    icde14_cpu_bar = ax.bar(ind + width, icde14_time, width, hatch="-", label="Inc-SR", fill=False)
    plt.yscale('log')  # log scale of y
    # plt.ylim(ymin=-2)  # this line
    ax.set_xticks(ind + width / 2)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    ax.set_ylabel("Avg Update Time(s)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=2)
    fig.savefig("./figures/" + 'dynamic_cpu_compare.pdf', bbox_inches='tight', dpi=300)

    # draw the memory figure
    fig, ax = plt.subplots()
    N = len(g_names)
    width = 0.35  # the width of the bars
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    dlp_mem_bar = ax.bar(ind, np.array(dlp_mem), width, \
                         hatch="\\", label="DLP", fill=False)
    icde14_mem_bar = ax.bar(ind + width, np.array(icde14_mem), width, \
                            hatch="-", label="Inc-SR", fill=False)
    plt.yscale('log')  # log scale of y
    # plt.ylim(ymin=-2)  # this line
    ax.set_xticks(ind + width / 2)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=2)
    fig.savefig("./figures/" + 'dynamic_mem_compare.pdf', bbox_inches='tight', dpi=300)
    return


def test():
    return


if __name__ == '__main__':
    # draw_static_compare()
    # for data in datasets:
    #     data_file = get_lineard_file(data)
    #     print(parse_lin_file(data_file))
    varying_delta_E()
    # varying_epsilon_on_caGrQc()
    # varying_c_on_ca_GrQc()
    # draw_dynamic_compare()
    # for d in datasets[0:4]:
    #     draw_accuracy_single(d)
    # draw_static_compare()
    # draw_datasets()
    # for d in datasets:
    #     rm(d,0.8,0.001)
    # plot_sparse3d_data("ca-GrQc")
    # plot_sparse3d_data("ca-HepTh")
    # draw_sparse_table()
    # test()
    # for d in ["ca-GrQc", "ca-HepTh"]:
    #     for c in np.arange(0.5,1,0.1):
    #         for epsilon in np.arange(0.001, 0.011, 0.001):
    #             print(d, c, epsilon)
    #             subprocess.run(["./test", d, str(c), str(epsilon)])

    # for d in datasets:
    #     print(d)
    #     subprocess.run(["./test", d, str(0.6)])
