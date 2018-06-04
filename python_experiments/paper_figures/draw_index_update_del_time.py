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
FIG_SIZE_MULTIPLE = (32, 6)
LABEL_SIZE = 22
TICK_SIZE = 22
LEGEND_SIZE = 22


def get_algorithm_indexing_time_lst(tag):
    if tag in [tsf_tag]:
        return [3.83223e-06, 2.99363e-06, 2.8017e-06, 3.74258e-06, 4.32549e-06, 6.50677e-06,
                # 2.39533e-05,
                1.75263e-05, 2.76335e-05, 2.1705e-05, 1.33162e-05, 3.79819e-05, 7.76859e-05]
    elif tag in [reads_d_tag]:
        return [0.00372522, 0.00453147, 0.00266618, 0.000856222, 0.00533698, 0.00204701,
                # 0.00649191,
                0.00664998, 0.0153763, 0.0089581, 0.00976721, 0.0828088, 0]
    elif tag in [reads_rq_tag]:
        return [0.00020761, 0.000227971, 0.000128773, 7.0902e-05, 0.000227616, 6.56451e-05,
                # 0.000299639,
                0.000449817, 0.00105135, 0.000437338, 0.000619871, 0.00149946, 0.00185752]
    else:
        return [0.0008325534, 0.0016021934, 0.0013406, 0.000796173, 0.007979267,
                0.0191974,
                # 0.111208,
                0.072434, 0.271308, 0.23858666666666664, 1.25391 / 1.1, 2.47118 / 1.1, 9.09415 / 1.1]


def draw_index_update_time():
    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll',
        'web-Stanford', 'web-BerkStan', 'web-Google',
        'cit-Patents', 'soc-LiveJournal1',
        'wiki-Link']

    g_names = map(lambda data: data_names[data], data_set_lst)

    size_of_fig = (FIG_SIZE_MULTIPLE[0], FIG_SIZE_MULTIPLE[1])
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.2
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(5))

    # other lst
    hatch_lst = ["//", '', 'O', '--', ]
    algorithm_tag_lst = [local_push_tag, reads_d_tag, reads_rq_tag, tsf_tag]
    label_lst = ["FLP", "READS_D", "READS-Rq", "TSF"]
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

    plt.ylim(10 ** -6, 13)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'index_update_time_del.pdf', bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw_index_update_time()
