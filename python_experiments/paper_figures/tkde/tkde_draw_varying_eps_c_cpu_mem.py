import matplotlib
import numpy as np
import json

matplotlib.use("pdf")

import matplotlib.pyplot as plt
import os
from paper_figures.legacy.experimental_config import *

eps_lst = ['%.3lf' % (x * 0.001) for x in range(1, 101, 4)]
epsilons = np.arange(0.001, 0.101, 0.004)

c_lst = ['%.1lf' % (x * 0.1) for x in range(1, 10)]

time_tag = 'cpu'
mem_tag = 'mem'


def none_if_zero(val):
    return val if val != 0 else None


def get_eps_lst(algorithm: str, info_tag: str, c: str, eps_lst: list):
    assert info_tag in [time_tag, mem_tag]
    with open('parsing_results/varying-eps_{}.json'.format(info_tag)) as ifs:
        data = json.load(ifs)
    return [none_if_zero(data[algorithm][c][eps]) for eps in eps_lst]


def get_c_lst(algorithm: str, info_tag: str, c_lst: list, eps: str):
    assert info_tag in [time_tag, mem_tag]
    with open('parsing_results/varying-c_{}.json'.format(info_tag)) as ifs:
        data = json.load(ifs)
    return [data[algorithm][c][eps] for c in c_lst]


tag_lst = ['rlp', 'flp', 'pcg', 'sling_ss_ap_bench', 'probesim_ss_ap_bench']
label_lst = ['Opt-LP', 'FLP', 'PCG', 'SLING', 'ProbeSim']
color_lst = ['red', 'blue', 'green', '#ceb301', '#fe01b1', 'm', 'brown', 'grey', ]
shape_lst = ['s--', 'o:', '*-.', 'v-', 'x-', 'P-', '*-', 'v-', '^-', '<-', '>-']


def varying_eps_on_caGrQc(update_tag: str, lim: tuple):
    data_name = "ca-GrQc"

    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)

    marker_size = 14
    for idx, tag in enumerate(tag_lst):
        ax.plot(epsilons, get_eps_lst(tag, update_tag, '0.6', eps_lst), shape_lst[idx], label=label_lst[idx],
                markerfacecolor="None", markersize=marker_size, color=color_lst[idx])
    ax.set_xlabel(r"$\epsilon$", fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)" if update_tag is time_tag else "Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE - 2, "weight": "bold"}, ncol=2, loc="upper right")
    plt.ylim(lim)
    plt.yscale('log')
    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_e_{}.pdf'.format(update_tag), bbox_inches='tight', dpi=300)


def varying_c_on_caGrQc(update_tag: str, lim: tuple):
    c_range = np.arange(0.1, 1, 0.1)
    data_name = "ca-GrQc"

    # draw the figure
    fig = plt.figure()
    ax = fig.add_subplot(111)

    marker_size = 14
    for idx, tag in enumerate(tag_lst):
        ax.plot(c_range, get_c_lst(tag, update_tag, c_lst, '0.01'), shape_lst[idx], label=label_lst[idx],
                markerfacecolor="None", markersize=marker_size, color=color_lst[idx])
    ax.set_xlabel(r"$c$", fontsize=LABEL_SIZE)
    ax.set_ylabel("CPU Time(s)" if update_tag is time_tag else "Memory Usage(MB)", fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE - 2, "weight": "bold"}, ncol=2, loc="upper left")
    plt.ylim(lim)
    plt.yscale('log')
    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + '_varying_c_{}.pdf'.format(update_tag), bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    os.system('mkdir -p {}'.format("./figures/"))
    # varying_epsilon_on_caGrQc()
    varying_eps_on_caGrQc(time_tag, lim=(10 ** (-2), 10 ** 7 / 2))
    varying_eps_on_caGrQc(mem_tag, lim=(10 ** 1 / 2, 10 ** 5 * 2))

    varying_c_on_caGrQc(time_tag, lim=(10 ** (-2), 10 ** 8 * 4))
    varying_c_on_caGrQc(mem_tag, lim=(10 ** 1 / 2, 10 ** 5 * 2))
