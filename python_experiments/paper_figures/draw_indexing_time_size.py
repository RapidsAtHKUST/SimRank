import matplotlib.pyplot as plt
import numpy as np

from data_analysis.generate_index_markdown import *


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

index_info_dict = get_index_dict('../data_analysis')


def get_algorithm_indexing_time_lst(algorithm_tag):
    return map(lambda dataset: index_info_dict[algorithm_tag][index_time_tag][dataset], data_set_lst)


def get_algorithm_index_size_lst(algorithm_tag):
    return map(lambda dataset: index_info_dict[algorithm_tag][index_size_tag][dataset], data_set_lst)


def draw_indexing_time():
    g_names = map(lambda data: data_names[data], data_set_lst)

    size_of_fig = (FIG_SIZE_MULTIPLE)
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.2
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(5))

    # other lst
    algorithm_tag_lst = [local_push_tag, sling_tag, linear_d_tag, cloud_walker_tag, tsf_tag]
    hatch_lst = ["//", ".", '-', 'x', '+']
    label_lst = ["FLP", "SLING", "LIN", "MCSP", "TSF"]

    # 1st: bars
    for idx, tag in enumerate(algorithm_tag_lst):
        ax.bar(indent_lst[idx], get_algorithm_indexing_time_lst(tag), width, hatch=hatch_lst[idx], label=label_lst[idx],
               fill=False)

    # 2nd: x and y's ticks and labels
    ax.set_xticks(ind + 2 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)

    plt.yscale('log')
    ax.set_ylabel("Indexing Time(s)", fontsize=LABEL_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'indexing_time.pdf', bbox_inches='tight', dpi=300)


def draw_indexing_size():
    g_names = map(lambda data: data_names[data], data_set_lst)

    size_of_fig = (FIG_SIZE_MULTIPLE)
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.2
    ind = 1.2 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(5))

    # other lst
    algorithm_tag_lst = [local_push_tag, sling_tag, linear_d_tag, cloud_walker_tag, tsf_tag]
    hatch_lst = ["//", ".", '-', 'x', '+']
    label_lst = ["FLP", "SLING", "LIN", "MCSP", "TSF"]

    # 1st: bars
    for idx, tag in enumerate(algorithm_tag_lst):
        ax.bar(indent_lst[idx], get_algorithm_index_size_lst(tag), width, hatch=hatch_lst[idx], label=label_lst[idx],
               fill=False)

    # 2nd: x and y's ticks and labels
    ax.set_xticks(ind + 2 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)

    plt.yscale('log')
    ax.set_ylabel("Index Size(MB)", fontsize=LABEL_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'index_size.pdf', bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw_indexing_time()
    draw_indexing_size()