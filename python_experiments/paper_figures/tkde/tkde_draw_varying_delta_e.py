import os

import matplotlib
import numpy as np

matplotlib.use("pdf")

import matplotlib.pyplot as plt
from paper_figures.legacy.experimental_config import *
from paper_figures.tkde.data_legacy.static_data_loader import *

insert_tag = 'ins'
delete_tag = 'del'
mem_tag = 'mem'
cpu_tag = 'cpu'
updates = list(range(5000, 30000, 5000))
inc_sr_time_ins = {5000: 828.2532715, 10000: 1583.839004, 15000: 2357.798905, 20000: 3135.9563235, 25000: 3902.367272}
inc_sr_mem_ins = {5000: 1207.69154, 10000: 1209.622656, 15000: 1222.702304, 20000: 1234.045476, 25000: 1236.55864}

inc_sr_time = {5000: 780.4734544, 10000: 1545.050671, 15000: 2355.302817428571, 20000: 3053.92380275,
               25000: 3873.9596019444443}
inc_sr_mem = {5000: 1236.55864, 10000: 1252.076852, 15000: 1264.300012, 20000: 1268.899444, 25000: 1271.284524}

label_lst = ['PDLP', 'DLP', 'Inc-SR', 'READS-D', 'READS-Rq']
dynamic_algorithm_lst = [tkde_pdlp_tag, vldbj_dlp_tag, icde_inc_sr_tag, vldbj_reasd_tag, vldbj_readrq_tag, ]
data_name = 'ca-HepTh'


def get_data_lst(algorithm_tag: str, update_tag: str, type_tag: str):
    assert algorithm_tag in dynamic_algorithm_lst
    assert update_tag in [insert_tag, delete_tag]
    assert type_tag in [mem_tag, cpu_tag]
    return [update for update in updates]


def draw_cpu_mem(update_tag: str, type_tag: str):
    suffix_str = '_varying_{}E_{}.pdf'.format('insert' if update_tag is insert_tag else 'delete',
                                              'mem' if type_tag is mem_tag else 'cpu')
    fig = plt.figure()
    ax = fig.add_subplot(111)

    color_lst = ['#fe01b1', 'red', 'blue', '#ceb301', 'green', ]
    shape_lst = ['D-.', 's--', 'o:', 'x-',
                 'P-', '*-',
                 'v-', '^-', '<-', '>-']
    for idx, tag in enumerate(dynamic_algorithm_lst):
        ax.plot(updates, get_data_lst(tag, update_tag, type_tag), shape_lst[idx], label=label_lst[idx],
                color=color_lst[idx], markerfacecolor="None", markersize=MARKER_SIZE)
    ax.set_xlabel(r"$|\Delta{E}|$", fontsize=LABEL_SIZE)
    ax.set_ylabel("Memory Usage(MB)" if type_tag is mem_tag else "CPU Time(s)", fontsize=LABEL_SIZE)

    plt.xticks(np.arange(min(updates), max(updates) + 1, 5000))
    plt.ticklabel_format(style='sci', axis='both', scilimits=(0, 0))
    ax.yaxis.offsetText.set_fontsize(MARKER_SIZE)
    ax.xaxis.offsetText.set_fontsize(MARKER_SIZE)

    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE - 2, "weight": "bold"}, loc="upper left", ncol=2)
    plt.subplots_adjust(wspace=0.2)
    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + suffix_str, bbox_inches='tight', dpi=300)
    plt.ylim(10 ** (-2), 10 ** 11)


def varying_delta_E():
    draw_cpu_mem(insert_tag, cpu_tag)
    draw_cpu_mem(insert_tag, mem_tag)
    draw_cpu_mem(delete_tag, cpu_tag)
    draw_cpu_mem(delete_tag, mem_tag)


if __name__ == '__main__':
    os.system('mkdir -p {}'.format("./figures/"))
    varying_delta_E()
