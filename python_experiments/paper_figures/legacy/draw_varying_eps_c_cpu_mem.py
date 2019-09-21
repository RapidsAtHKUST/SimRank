import matplotlib
import numpy as np

matplotlib.use("pdf")

import matplotlib.pyplot as plt
import os
from paper_figures.legacy.experimental_config import *


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
        # rlp_data = parse_exp_file(RLP_get_data_file_base(d, c, epsilon) + ".meta")
        # flp_data = parse_exp_file(FLP_get_data_file_base(d, c, epsilon) + ".meta")
        tkde_data = parse_tkde17_file(TKDE17_get_data_file_base(d, c, epsilon) + ".meta")
        # rlp_cpu_time.append(rlp_data["time"])
        # rlp_mem.append(rlp_data["mem"] / 1000)
        # flp_cpu_time.append(flp_data["time"])
        # flp_mem.append(flp_data["mem"] / 1000)
        tkde_cpu_time.append(tkde_data["time"])
        tkde_mem.append(tkde_data["mem"])
    print(dict(zip(['%.3f' % x for x in epsilons], tkde_cpu_time)))
    print(dict(zip(['%.3f' % x for x in epsilons], tkde_mem)))
    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)
    # rlp_cpu_line = ax.plot(epsilons, rlp_cpu_time, "r--", label="Opt-LP")
    # flp_cpu_line = ax.plot(epsilons, flp_cpu_time, "b-", label="FLP")
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
    # rlp_mem_line = ax.semilogy(epsilons, rlp_mem, "r--", label="Opt-LP")
    # flp_mem_line = ax.semilogy(epsilons, flp_mem, "b-", label="FLP")
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
        # rlp_data = parse_exp_file(RLP_get_data_file_base(d, c, epsilon) + ".meta")
        # flp_data = parse_exp_file(FLP_get_data_file_base(d, c, epsilon) + ".meta")
        tkde_data = parse_tkde17_file(TKDE17_get_data_file_base(d, c, epsilon) + ".meta")
        # rlp_cpu_time.append(rlp_data["time"])
        # rlp_mem.append(rlp_data["mem"] / 1000)
        # flp_cpu_time.append(flp_data["time"])
        # flp_mem.append(flp_data["mem"] / 1000)
        tkde_cpu_time.append(tkde_data["time"])
        tkde_mem.append(tkde_data["mem"])
    print(dict(zip(['%.1f' % x for x in c_range], tkde_cpu_time)))
    print(dict(zip(['%.1f' % x for x in c_range], tkde_mem)))
    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)
    # rlp_cpu_line = ax.plot(c_range, rlp_cpu_time, "r^--", label="Opt-LP", markerfacecolor="None", \
    #                        markersize=MARKER_SIZE)
    # flp_cpu_line = ax.plot(c_range, flp_cpu_time, "bs-", label="FLP", markerfacecolor="None", \
    #                        markersize=MARKER_SIZE)
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
    # rlp_mem_line = ax.plot(c_range, rlp_mem, "r^--", label="Opt-LP", markerfacecolor="None", \
    #                        markersize=MARKER_SIZE)
    # flp_mem_line = ax.plot(c_range, flp_mem, "bs-", label="FLP", markerfacecolor="None", \
    #                        markersize=MARKER_SIZE)
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


if __name__ == '__main__':
    os.system('mkdir -p {}'.format("./figures/"))
    varying_epsilon_on_caGrQc()
    varying_c_on_ca_GrQc()
