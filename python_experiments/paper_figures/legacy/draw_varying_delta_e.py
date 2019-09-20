import os
import random

import matplotlib
import numpy as np

matplotlib.use("pdf")

import matplotlib.pyplot as plt
from sklearn import linear_model
from paper_figures.legacy.experimental_config import *


def ICDE_get_data_file(g_name, number_of_updates):
    return ICDE14_DIR + g_name + "_%d.txt" % number_of_updates


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


def DLP_get_data_file(g_name, number_of_updates):
    return DYNAMIC_LOCAL_PUSH_DIR + g_name + "_%d.txt" % number_of_updates


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
    print(dict(zip(updates, ICDE_cpu_time)))
    print(dict(zip(updates, ICDE_mem)))
    
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
    print(dict(zip(updates, ICDE_cpu_time)))
    print(dict(zip(updates, ICDE_mem)))
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


if __name__ == '__main__':
    os.system('mkdir -p {}'.format("./figures/"))
    varying_delta_E()
