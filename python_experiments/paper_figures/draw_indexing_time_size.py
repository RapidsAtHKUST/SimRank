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

# get the data for figures
index_info_dict = get_index_dict_with_reads('../data_analysis')


def get_algorithm_indexing_time_lst(algorithm_tag):
    return map(lambda dataset: index_info_dict[algorithm_tag][index_time_tag][dataset], data_set_lst)


def get_algorithm_index_size_lst(algorithm_tag):
    return map(lambda dataset: index_info_dict[algorithm_tag][index_size_tag][dataset], data_set_lst)


def draw_figures():
    # init graph names and size
    global data_set_lst
    with open('data_set_lst.json') as ifs:
        data_set_lst = json.load(ifs)
    g_names = map(lambda data: data_names[data], data_set_lst)
    size_of_fig = (FIG_SIZE_MULTIPLE)
    N = len(g_names)

    # indent lst
    width = 0.14
    ind = 1.14 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(7))

    # other lst
    hatch_lst = ["//", "**", '', 'O', '--', 'x', '++']
    algorithm_tag_lst = [local_push_tag, sling_tag,
                         reads_d_tag, reads_rq_tag,
                         tsf_tag, linear_d_tag, cloud_walker_tag]
    label_lst = ["FLP", "SLING", 'READS-D', 'READS-Rq', "TSF", "LIN", "MCSP"]
    color_lst = ['blue',
                 # 'orange', 'green',
                 'red', '#fe01b1', '#ceb301',
                 # 'm',
                 'brown', 'k', 'gray']
    # tick offset
    tick_offset = 3

    def draw_indexing_time():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(algorithm_tag_lst):
            my_data_lst = get_algorithm_indexing_time_lst(tag)
            ax.bar(indent_lst[idx], my_data_lst, width, hatch=hatch_lst[idx],
                   label=label_lst[idx],
                   edgecolor=color_lst[idx],
                   fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + tick_offset * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Indexing Time (s)", fontsize=LABEL_SIZE)
        plt.yticks(fontsize=TICK_SIZE)
        plt.ylim(10 ** (-3), 10 ** 6)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=4)
        fig.savefig("./figures/" + 'indexing_time.pdf', bbox_inches='tight', dpi=300)

    def draw_indexing_size():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(algorithm_tag_lst):
            my_data_lst = get_algorithm_index_size_lst(tag)
            if idx == 0:
                # make use of symmetric
                my_data_lst = map(lambda rlp_val: rlp_val / 1., my_data_lst)
            ax.bar(indent_lst[idx], my_data_lst, width, hatch=hatch_lst[idx],
                   label=label_lst[idx],
                   edgecolor=color_lst[idx],
                   fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + tick_offset * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Index Size (MB)", fontsize=LABEL_SIZE)
        plt.yticks(fontsize=TICK_SIZE)

        plt.ylim(10 ** -2, 10 ** 6 * 2)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=4)
        fig.savefig("./figures/" + 'index_size.pdf', bbox_inches='tight', dpi=300)

    # draw indexing time and size
    draw_indexing_time()
    draw_indexing_size()


if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw_figures()
