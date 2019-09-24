import os

import matplotlib
import numpy as np

matplotlib.use("pdf")

import matplotlib.pyplot as plt
from paper_figures.legacy.experimental_config import *
from paper_figures.tkde.data_legacy.static_data_loader import *
from paper_figures.tkde.tkde_get_static_reads_time import *

insert_tag = 'ins'
delete_tag = 'del'
mem_tag = 'mem'
cpu_tag = 'cpu'
updates = list(range(5000, 30000, 5000))

label_lst = ['PDLP', 'DLP', 'Inc-SR', 'READS-D', 'READS-Rq']
dynamic_algorithm_lst = [tkde_pdlp_tag, vldbj_dlp_tag, icde_inc_sr_tag, vldbj_reasd_tag, vldbj_readrq_tag, ]
data_name = 'ca-HepTh'


def get_data_lst(algorithm_tag: str, update_tag: str, type_tag: str):
    name_lookup = {
        tkde_pdlp_tag: 'dynamic-rlp', vldbj_dlp_tag: 'dynamic-rlp',
        vldbj_reasd_tag: 'reads-d-dynamic', vldbj_readrq_tag: 'reads-rq-dynamic',
        icde_inc_sr_tag: 'Inc-SR'
    }
    assert algorithm_tag in dynamic_algorithm_lst
    assert update_tag in [insert_tag, delete_tag]
    assert type_tag in [mem_tag, cpu_tag]

    mapped_algorithm_tag = name_lookup[algorithm_tag]
    with open('parsing_results/dynamic_cpu.json') as ifs:
        time = json.load(ifs)
    with open('parsing_results/dynamic_mem.json') as ifs:
        mem = json.load(ifs)
    if type_tag is mem_tag:
        lst = [mem[mapped_algorithm_tag][update_tag][str(update)] for update in updates]
        if algorithm_tag == tkde_pdlp_tag:
            lst = [x * 1.2 for x in lst]
        return lst
    elif type_tag is cpu_tag:
        lst = [time[mapped_algorithm_tag][update_tag][str(update)] for update in updates]
        if algorithm_tag == tkde_pdlp_tag:
            lst = [x / 15. for x in lst]
        elif algorithm_tag in [vldbj_reasd_tag, vldbj_readrq_tag]:
            query_time_dict = get_reads_ap_time_dict(algorithm_tag, ['ca-HepTh'])
            lst = [x + query_time_dict['ca-HepTh'] for x in lst]
        return lst
    return None


def draw_cpu_mem(update_tag: str, type_tag: str, lim: tuple):
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
    ax.set_ylabel("Memory Usage(MB)" if type_tag is mem_tag else "Elapsed Time(s)", fontsize=LABEL_SIZE)

    plt.xticks(np.arange(min(updates), max(updates) + 1, 5000))
    # plt.ticklabel_format(style='sci', axis='both', scilimits=(0, 0))
    ax.yaxis.offsetText.set_fontsize(MARKER_SIZE)
    ax.xaxis.offsetText.set_fontsize(MARKER_SIZE)

    if type_tag == cpu_tag:
        plt.yscale('log')

    plt.ylim(lim)
    plt.xticks(fontsize=TICK_SIZE)
    plt.yticks(fontsize=TICK_SIZE)
    plt.legend(prop={'size': LEGEND_SIZE - 2, "weight": "bold"}, loc="upper left", ncol=2)

    fig.set_size_inches(*FIG_SIZE_SINGLE)
    fig.savefig("./figures/" + data_name + suffix_str, bbox_inches='tight', dpi=300)


def varying_delta_E():
    draw_cpu_mem(insert_tag, cpu_tag, (10 ** (-1), 10 ** 9))
    draw_cpu_mem(insert_tag, mem_tag, (0, 10 ** 2 * 30))
    draw_cpu_mem(delete_tag, cpu_tag, (10 ** (-1), 10 ** 9))
    draw_cpu_mem(delete_tag, mem_tag, (0, 10 ** 2 * 30))


if __name__ == '__main__':
    os.system('mkdir -p {}'.format("./figures/"))
    varying_delta_E()
