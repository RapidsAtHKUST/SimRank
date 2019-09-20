import matplotlib.pyplot as plt
import numpy as np

from data_analysis.vldbj_data_parsing.generate_index_markdown import *
from paper_figures.tkde.data_legacy.static_data_loader import *


def get_name_dict():
    with open('data_names.json') as ifs:
        return eval(''.join(ifs.readlines()))


# data set abbreviation dictionary
data_names = get_name_dict()

# figure parameters
FIG_SIZE_MULTIPLE = (32, 4)
LABEL_SIZE = 22
TICK_SIZE = 22
LEGEND_SIZE = 22

with open('tkde_data_set_lst.json') as ifs:
    data_set_lst = json.load(ifs)


def get_algorithm_indexing_time_lst(tag):
    # init graph names and size
    with open('data_legacy/vldbj-icde-dynamic-time.json') as ifs:
        time = json.load(ifs)
    return [time[tag][data] for data in data_set_lst]


def draw_index_update_time():
    algorithm_lst = [tkde_pdlp_tag, vldbj_dlp_tag, icde_inc_sr_tag, vldbj_reasd_tag, vldbj_readrq_tag, ]
    g_names = list(map(lambda data: data_names[data], data_set_lst))

    size_of_fig = (FIG_SIZE_MULTIPLE[0], FIG_SIZE_MULTIPLE[1])
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.2
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    indent_lst = list(map(lambda idx: ind + idx * width, range(5)))

    # other lst
    hatch_lst = ["//", "**", '.', 'O', '--', 'x', '++', '\\\\', '', '|||', 'o']
    color_lst = [
        '#fe01b1', '#ceb301', 'red', 'orange', 'green', 'blue', 'm', 'brown', 'grey', 'k', 'pink'
    ]
    label_lst = ["PDLP", "DLP", "Inc-SR", "READS_D", "READS-Rq"]

    # 1st: bars
    for idx, tag in enumerate(algorithm_lst):
        ax.bar(indent_lst[idx], get_algorithm_indexing_time_lst(tag), width, hatch=hatch_lst[idx],
               label=label_lst[idx], edgecolor=color_lst[idx], fill=False)

    # 2nd: x and y's ticks and labels
    ax.set_xticks(ind + 1.5 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)

    plt.yscale('log')
    ax.set_ylabel("Avg Update Time (s)", fontsize=LABEL_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    plt.ylim(10 ** -6, 10 ** 6 * 6)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'dynamic_cpu_compare.pdf', bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw_index_update_time()
