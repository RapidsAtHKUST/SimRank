import json

import matplotlib
import numpy as np

matplotlib.use("pdf")

import matplotlib.pyplot as plt
import os
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

    # print()
    time_dict = dict(zip(datasets, icde14_time))
    mem_dict = dict(zip(datasets, icde14_mem))
    print(json.dumps({'time': time_dict, 'mem': mem_dict}, indent=4))

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


if __name__ == '__main__':
    os.system('mkdir -p {}'.format("./figures/"))
    draw_dynamic_compare()
