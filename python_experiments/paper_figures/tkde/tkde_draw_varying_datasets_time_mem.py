import matplotlib.pyplot as plt
import numpy as np
from data_analysis.parallel_statistics.err_mem_size_statistics import *
from data_analysis.parallel_statistics.previous_data_statistics import *
from data_analysis.parallel_statistics.generate_parallel_speedup_md import *
from paper_figures.tkde.data_legacy.static_data_loader import *
from paper_figures.tkde.tkde_common import *

relative_data_root_dir_path = '../..'

algorithm_tag_lst = [prlp_tag, prlp_lock_free_tag, rlp_tag, flp_tag,
                     lin_tag, cw_tag, pcg_tag,
                     vldbj_sling_tag, vldbj_probesim_tag, vldbj_reasd_tag, vldbj_readrq_tag]
static_label_lst = ["PLB-Opt-LP", "PLF-Opt-LP", 'Opt-LP', 'FLP', "LIN", "MCAP", "PCG",
                    "SLING", "ProbeSim", "READS-D", "READS-Rq"]


def get_algorithm_elapsed_time_lst(tag):
    if tag in [vldbj_sling_tag, vldbj_probesim_tag, vldbj_reasd_tag, vldbj_readrq_tag]:
        with open('data_legacy/vldbj-comp.json') as ifs:
            tmp = json.load(ifs)[tag]
            return [tmp[data_set] for data_set in data_set_lst]
    elif tag in [prlp_tag, prlp_lock_free_tag]:
        with open('{}/data_analysis/data-json/parallel_exp/scalability_04_24.json'.format(
                relative_data_root_dir_path)) as ifs:
            p_algo_dict = json.load(ifs)[tag]
            return list(map(lambda data_set: min(p_algo_dict[data_set][time_tag]), data_set_lst))
    elif tag in [rlp_tag, flp_tag]:
        with open('{}/data_analysis/data-json/parallel_exp/seq_time_04_24.json'.format(
                relative_data_root_dir_path)) as ifs:
            seq_our_algo_dict = json.load(ifs)[tag]
            return list(map(lambda data_set: seq_our_algo_dict[data_set], data_set_lst))
    else:
        with open('{}/data_analysis/data-json/parallel_exp/seq_time_previous.json'.format(
                relative_data_root_dir_path)) as ifs:
            seq_other_algo_dict = json.load(ifs)[tag]
            return list(map(lambda data_set: seq_other_algo_dict[data_set], data_set_lst))


def get_algorithm_mem_usage_lst(tag):
    with open('data_legacy/vldbj-mem-size.json') as ifs:
        tmp = json.load(ifs)
    if tag in [vldbj_sling_tag, vldbj_probesim_tag, vldbj_reasd_tag, vldbj_readrq_tag]:
        return [tmp[tag][data_set] for data_set in data_set_lst]
    if tag in [prlp_tag, prlp_lock_free_tag, rlp_tag, flp_tag]:
        with open('{}/data_analysis/data-json/parallel_exp/err_mem_size04_24.json'.format(
                relative_data_root_dir_path)) as ifs:
            our_algo_dict = json.load(ifs)[tag]
            mem_lst = [our_algo_dict[data_set][mem_size_tag] for data_set in data_set_lst]
    else:
        with open('{}/data_analysis/data-json/parallel_exp/seq_mem_previous.json'.format(
                relative_data_root_dir_path)) as ifs:
            seq_other_algo_dict = json.load(ifs)[tag]
        if tag in [lin_tag, cw_tag]:
            mem_lst = [seq_other_algo_dict[data_set] + 1024 * tmp[vldbj_probesim_tag][data_set] \
                       for data_set in data_set_lst]
        else:
            mem_lst = [seq_other_algo_dict[data_set] for data_set in data_set_lst]
    return list(map(lambda mem_size: mem_size / 1024., mem_lst))


def draw_figures():
    N = len(g_names)

    # indent lst
    width = 0.08
    ind = 1.05 * np.arange(N)  # the x locations for the groups
    indent_lst = list(map(lambda idx: ind + idx * width, range(11)))

    # other lst
    hatch_lst = ["//", "**", '.', 'O', '--', 'x', '++', '\\\\', '', '|||', 'o']
    color_lst = [
        '#fe01b1', '#ceb301', 'red', 'orange', 'green', 'blue', 'm', 'brown', 'grey', 'k', 'pink'
    ]
    # tick offset
    tick_offset = 5

    def draw_elapsed_time():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(algorithm_tag_lst):
            my_data_lst = get_algorithm_elapsed_time_lst(tag)
            ax.bar(indent_lst[idx], my_data_lst, width, hatch=hatch_lst[idx],
                   label=static_label_lst[idx], edgecolor=color_lst[idx], fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + tick_offset * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Elapsed Time(s)", fontsize=LABEL_SIZE)
        plt.yticks(fontsize=TICK_SIZE)
        plt.ylim(10 ** (-2), 10 ** 11)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=6)
        fig.savefig("./figures/" + 'static_cpu_compare.pdf', bbox_inches='tight', dpi=300)

    def draw_mem_usage():
        fig, ax = plt.subplots()
        # 1st: bars
        for idx, tag in enumerate(algorithm_tag_lst):
            my_data_lst = get_algorithm_mem_usage_lst(tag)
            ax.bar(indent_lst[idx], my_data_lst, width, hatch=hatch_lst[idx],
                   label=static_label_lst[idx], edgecolor=color_lst[idx], fill=False)

        # 2nd: x and y's ticks and labels
        ax.set_xticks(ind + tick_offset * width)
        ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
        plt.xticks(fontsize=TICK_SIZE)

        plt.yscale('log')
        ax.set_ylabel("Memory Usage(MB)", fontsize=LABEL_SIZE)
        plt.yticks(fontsize=TICK_SIZE)
        plt.ylim(10 ** 0 * 3, 10 ** 8 / 1.1)

        # 3rd: figure properties
        fig.set_size_inches(*size_of_fig)  # set ratio
        plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=6)
        fig.savefig("./figures/" + 'static_mem_compare.pdf', bbox_inches='tight', dpi=300)

    # draw indexing time and size
    draw_elapsed_time()
    draw_mem_usage()


if __name__ == '__main__':
    draw_figures()
