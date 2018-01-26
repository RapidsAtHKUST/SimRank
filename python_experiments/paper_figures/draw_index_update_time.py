import matplotlib.pyplot as plt
import numpy as np

from data_analysis.generate_index_markdown import *
from data_analysis.reads_indexing_statistics import reads_d_tag, reads_rq_tag


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


def get_algorithm_indexing_time_lst(tag):
    if tag in [tsf_tag]:
        return [4.28582e-06, 3.54921e-06, 4.10694e-06, 4.28403e-06, 3.70801e-06, 1.33611e-05,
                # 5.06265e-05,
                2.68152e-05, 2.64846e-05, 4.53474e-05, 1.5878e-05, 5.61959e-05]
    elif tag in [reads_d_tag]:
        return [0.00519134, 0.00605724, 0.00391006, 0.000957195, 0.00752088, 0.00470619,
                # 0.00904561,
                0.0212018, 0.0262394, 0.011979, 0.00976721, 0.0828088]
    elif tag in [reads_rq_tag]:
        return [0.00024528, 0.000229015, 0.000147289, 0.000109361, 0.000355261, 0.000196691,
                # 0.00100307,
                0.00045801, 0.00154875, 0.000718491, 0.00103323, 0.00236655]
    else:
        return [0.00130415, 0.00241406, 0.00237629, 0.00163426, 0.0139743, 0.0493718,
                # 0.124753,
                0.102021, 0.271308, 0.268973, 1.25391, 2.47118]


def draw_index_update_time():
    g_names = map(lambda data: data_names[data], data_set_lst)

    size_of_fig = (FIG_SIZE_MULTIPLE[0] * 0.75, FIG_SIZE_MULTIPLE[1])
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.2
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(5))

    # other lst
    hatch_lst = ["//", '', 'O', '--', ]
    algorithm_tag_lst = [local_push_tag, reads_d_tag, reads_rq_tag, tsf_tag]
    label_lst = ["FLP", "READS_D", "READS-RQ", "TSF"]
    color_lst = ['blue', '#fe01b1', '#ceb301', 'brown']

    # 1st: bars
    for idx, tag in enumerate(algorithm_tag_lst):
        ax.bar(indent_lst[idx], get_algorithm_indexing_time_lst(tag), width, hatch=hatch_lst[idx], label=label_lst[idx],
               edgecolor=color_lst[idx],
               fill=False)

    # 2nd: x and y's ticks and labels
    ax.set_xticks(ind + 1.5 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)

    plt.yscale('log')
    ax.set_ylabel("Avg Update Time (s)", fontsize=LABEL_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    plt.ylim(10 ** -6, 5)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'index_update_time.pdf', bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw_index_update_time()
