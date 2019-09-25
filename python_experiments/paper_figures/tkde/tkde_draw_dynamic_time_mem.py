import matplotlib.pyplot as plt
import numpy as np

from data_analysis.vldbj_data_parsing.generate_index_markdown import *
from paper_figures.tkde.data_legacy.static_data_loader import *
from paper_figures.tkde.tkde_common import *
from paper_figures.tkde.tkde_get_static_reads_time import *

dynamic_label_lst = ["PDLP", "DLP", "Inc-SR", "ProbeSim", "READS-D", "READS-Rq"]
dynamic_algorithm_lst = [tkde_pdlp_tag, vldbj_dlp_tag, icde_inc_sr_tag, vldbj_probesim_tag, vldbj_reasd_tag,
                         vldbj_readrq_tag, ]


def get_index_update_time_lst(tag):
    # init graph names and size
    with open('data_legacy/vldbj-icde-dynamic-time.json') as ifs:
        time = json.load(ifs)
    if tag in [vldbj_reasd_tag, vldbj_readrq_tag]:
        query_time_dict = get_reads_probesim_ap_time_dict(tag, data_set_lst)
        return [time[tag][data] + query_time_dict[data] for data in data_set_lst]
    elif tag in [vldbj_probesim_tag]:
        query_time_dict = get_reads_probesim_ap_time_dict(tag, data_set_lst)
        print(query_time_dict)
        return [query_time_dict[data] for data in data_set_lst]
    else:
        return [time[tag][data] for data in data_set_lst]


def get_algorithm_dynamic_mem_lst(tag):
    if tag in [vldbj_probesim_tag]:
        with open('data_legacy/vldbj-mem-size.json') as ifs:
            tmp = json.load(ifs)
            return [tmp[tag][data_set] for data_set in data_set_lst]
    # init graph names and size
    with open('data_legacy/vldbj-icde-dynamic-mem.json') as ifs:
        mem = json.load(ifs)
    return [mem[tag][data] for data in data_set_lst]


def draw_figures():
    N = len(g_names)

    # indent lst
    width = 0.15
    ind = 1.1 * np.arange(N)  # the x locations for the groups
    indent_lst = list(map(lambda idx: ind + idx * width, range(6)))

    # other lst
    hatch_lst = ["//", "**", '.', 'O', '--', 'x', '++', '\\\\', '', '|||', 'o']
    color_lst = [
        '#fe01b1', '#ceb301', 'red', 'orange', 'green', 'blue', 'm', 'brown', 'grey', 'k', 'pink'
    ]

    def draw_time():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(dynamic_algorithm_lst):
            ax.bar(indent_lst[idx], get_index_update_time_lst(tag), width, hatch=hatch_lst[idx],
                   label=dynamic_label_lst[idx], edgecolor=color_lst[idx], fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + 2 * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Avg Update Time (s)", fontsize=LABEL_SIZE)
        plt.yticks(fontsize=TICK_SIZE)

        plt.ylim(10 ** -6, 10 ** 8 * 6)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=6)
        fig.savefig("./figures/" + 'dynamic_cpu_compare.pdf', bbox_inches='tight', dpi=300)

    def draw_mem():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(dynamic_algorithm_lst):
            ax.bar(indent_lst[idx], get_algorithm_dynamic_mem_lst(tag), width, hatch=hatch_lst[idx],
                   label=dynamic_label_lst[idx], edgecolor=color_lst[idx], fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + 2 * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Memory Usage (MB)", fontsize=LABEL_SIZE)
        plt.yticks(fontsize=TICK_SIZE)

        plt.ylim(10 ** 1 / 2, 10 ** 6 * 6)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=6)
        fig.savefig("./figures/" + 'dynamic_mem_compare.pdf', bbox_inches='tight', dpi=300)

    draw_time()
    draw_mem()


if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw_figures()
