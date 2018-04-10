import matplotlib.pyplot as plt

from data_analysis.querying_time_accuracy_statistics import lind_tag, cw_tag
from data_analysis.varying_eps_statistics import *
from draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE, reads_d_tag, reads_rq_tag, get_name_dict
import json


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


eps_max_err_dict = get_dict('../data_analysis/data-json/topk_precision/precision_top_800_sample_10000.json')
data_names = get_name_dict()


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.0000')))


eps_lst = [0.0001, 0.0004, 0.0016, 0.0064, 0.0256]

algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                     reads_d_tag, reads_rq_tag, isp_tag, tsf_tag,
                     # lind_tag, cw_tag
                     ]
legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING',
              'READS-D', 'READS-Rq', 'ISP', 'TSF', 'LIN', 'MCSP']
data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
color_lst = ['blue', 'orange', 'green', 'red',
             '#fe01b1', '#ceb301', 'm', 'brown', 'k', 'gray']
shape_lst = ['D-.', 's--', 'o:', 'x-',
             'P-', '*-',
             'v-', '^-', '<-', '>-']
caption_lst = ['(a) dataset = ', '(b) dataset = ', '(c) dataset = ', '(d) dataset = ']


def get_marker_size(idx):
    if idx == 0:
        return 12
    elif idx == 5:
        return 20
    else:
        return 14


def draw_precision():
    exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 7))

    for ax_idx, ax in enumerate(ax_tuple):
        lst_lst = []
        data_set_name = data_set_lst[ax_idx]
        y_lim_lst = [(0.58, 1.05), (0.48, 1.05), (0.48, 1.05), (0.72, 1.03)]
        for idx, algorithm in enumerate(algorithm_tag_lst):
            max_err_lst = map(lambda eps: eps_max_err_dict[data_set_name][algorithm][format_str(eps)], eps_lst)
            for i in xrange(len(max_err_lst) - 2, -1, -1):
                if max_err_lst[i] is None:
                    max_err_lst[i] = max_err_lst[i + 1]

            lst_lst.append(max_err_lst)

            ax.plot(eps_lst, max_err_lst, shape_lst[idx], color=color_lst[idx], markersize=get_marker_size(idx),
                    markerfacecolor='none')
        ax.set_ylim(y_lim_lst[ax_idx])
        ax.set_xscale('log')
        ax.set_xticks(eps_lst)
        ax.set_xticklabels(eps_lst)
        for tick in ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        for tick in ax.xaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        ax.set_xlabel('$\\epsilon$\n' + caption_lst[ax_idx] + data_names[data_set_name], fontsize=LABEL_SIZE + 4)
    ax = ax_tuple[0]
    # setup labels and grid, legend
    ax.set_ylabel('Precision', fontsize=LABEL_SIZE + 6)

    plt.tight_layout()
    plt.legend(legend_lst, ncol=len(legend_lst), fontsize=LEGEND_SIZE, prop={'size': LEGEND_SIZE + 3, "weight": "bold"},
               bbox_to_anchor=(0.55, 1.15))
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()

    plt.savefig('figures/' + 'varying_eps_precision' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


def draw_avg_diff():
    exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 7))

    for ax_idx, ax in enumerate(ax_tuple):
        lst_lst = []
        data_set_name = data_set_lst[ax_idx]
        for idx, algorithm in enumerate(algorithm_tag_lst):
            max_err_lst = map(lambda eps: eps_max_err_dict[data_set_name][algorithm][format_str(eps)], eps_lst)
            for i in xrange(len(max_err_lst) - 2, -1, -1):
                if max_err_lst[i] is None:
                    max_err_lst[i] = max_err_lst[i + 1]
            lst_lst.append(max_err_lst)

            ax.plot(eps_lst, max_err_lst, shape_lst[idx], color=color_lst[idx], markersize=get_marker_size(idx),
                    markerfacecolor='none')
        ax.set_ylim(10 ** (-6), 2 * 10 ** (-3))
        ax.set_xscale('log')
        ax.set_yscale('log')
        ax.set_xticks(eps_lst)
        ax.set_xticklabels(eps_lst)

        for tick in ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        for tick in ax.xaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        ax.set_xlabel('$\\epsilon$\n' + caption_lst[ax_idx] + data_names[data_set_name], fontsize=LABEL_SIZE + 4)
    ax = ax_tuple[0]
    # setup labels and grid, legend
    ax.set_ylabel('Avg Diff', fontsize=LABEL_SIZE + 6)

    plt.tight_layout()
    plt.legend(legend_lst, ncol=len(legend_lst), fontsize=LEGEND_SIZE, prop={'size': LEGEND_SIZE + 3, "weight": "bold"},
               bbox_to_anchor=(0.55, 1.15))
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()

    plt.savefig('figures/' + 'varying_eps_avg_diff' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


if __name__ == '__main__':
    draw_precision()
    eps_max_err_dict = get_dict('../data_analysis/data-json/topk_precision/avg_diff_top_800_sample_10000.json')
    draw_avg_diff()
