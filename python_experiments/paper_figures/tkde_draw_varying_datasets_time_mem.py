import matplotlib.pyplot as plt
import numpy as np
from data_analysis.parallel_statistics.err_mem_size_statistics import *
from data_analysis.parallel_statistics.previous_data_statistics import *
from data_analysis.parallel_statistics.generate_parallel_speedup_md import *


def get_name_dict():
    with open('data_names.json') as ifs:
        return eval(''.join(ifs.readlines()))


def draw_figures():
    # data set abbreviation dictionary
    data_names = get_name_dict()

    # figure parameters
    FIG_SIZE_MULTIPLE = (32, 4)
    LABEL_SIZE = 22
    TICK_SIZE = 22
    LEGEND_SIZE = 22

    # init graph names and size
    global data_set_lst
    with open('tkde_data_set_lst.json') as ifs:
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
    algorithm_tag_lst = [prlp_tag, prlp_lock_free_tag,
                         rlp_tag, flp_tag,
                         lin_tag, cw_tag, pcg_tag]
    label_lst = ["PLB-Opt-LP", "PLF-Opt-LP", 'Opt-LP', 'FLP', "LIN", "MCSP", "PCG"]
    color_lst = [
        '#fe01b1', '#ceb301',
        'red',
        # 'orange', 'green',
        'blue',
        # 'm',
        'brown', 'k', 'green']
    # tick offset
    tick_offset = 3

    def get_algorithm_elapsed_time_lst(tag):
        if tag in [prlp_tag, prlp_lock_free_tag]:
            with open('../data_analysis/data-json/parallel_exp/scalability_04_24.json') as ifs:
                p_algo_dict = json.load(ifs)[tag]
                return map(lambda data_set: min(p_algo_dict[data_set][time_tag]), data_set_lst)
        elif tag in [rlp_tag, flp_tag]:
            with open('../data_analysis/data-json/parallel_exp/seq_time_04_24.json') as ifs:
                seq_our_algo_dict = json.load(ifs)[tag]
                return map(lambda data_set: seq_our_algo_dict[data_set], data_set_lst)
        else:
            with open('../data_analysis/data-json/parallel_exp/seq_time_previous.json') as ifs:
                seq_other_algo_dict = json.load(ifs)[tag]
                return map(lambda data_set: seq_other_algo_dict[data_set], data_set_lst)

    def get_algorithm_mem_usage_lst(tag):
        mem_lst = None
        if tag in [prlp_tag, prlp_lock_free_tag, rlp_tag, flp_tag]:
            with open('../data_analysis/data-json/parallel_exp/err_mem_size04_24.json') as ifs:
                our_algo_dict = json.load(ifs)[tag]
                mem_lst = map(lambda data_set: our_algo_dict[data_set][mem_size_tag], data_set_lst)
        else:
            with open('../data_analysis/data-json/parallel_exp/seq_mem_previous.json') as ifs:
                seq_other_algo_dict = json.load(ifs)[tag]
                mem_lst = map(lambda data_set: seq_other_algo_dict[data_set], data_set_lst)
        return map(lambda mem_size: mem_size / 1024., mem_lst)

    def draw_elapsed_time():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(algorithm_tag_lst):
            my_data_lst = get_algorithm_elapsed_time_lst(tag)
            ax.bar(indent_lst[idx], my_data_lst, width, hatch=hatch_lst[idx],
                   label=label_lst[idx], edgecolor=color_lst[idx], fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + tick_offset * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Elapsed Time (s)", fontsize=LABEL_SIZE)
        # plt.yticks([10 ** (num) for num in [-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8]], fontsize=TICK_SIZE)
        plt.yticks(fontsize=TICK_SIZE)
        plt.ylim(10 ** (-2), 10 ** 8)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=4)
        fig.savefig("./figures/" + 'static_cpu_compare.pdf', bbox_inches='tight', dpi=300)

    def draw_mem_usage():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(algorithm_tag_lst):
            my_data_lst = get_algorithm_mem_usage_lst(tag)
            ax.bar(indent_lst[idx], my_data_lst, width, hatch=hatch_lst[idx],
                   label=label_lst[idx], edgecolor=color_lst[idx], fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + tick_offset * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
        # plt.yticks([10 ** (num) for num in [-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8]], fontsize=TICK_SIZE)
        plt.yticks(fontsize=TICK_SIZE)
        plt.ylim(10 ** (0) * 3, 10 ** 6 / 1.1)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=4)
        fig.savefig("./figures/" + 'static_mem_compare.pdf', bbox_inches='tight', dpi=300)

    # draw indexing time and size
    draw_elapsed_time()
    draw_mem_usage()


if __name__ == '__main__':
    draw_figures()
