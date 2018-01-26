import matplotlib.pyplot as plt
from data_analysis.varying_eps_statistics import *
from draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE, reads_d_tag, reads_rq_tag
import json

from paper_figures.draw_varying_c import large_size_plus


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


eps_max_err_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_eps_max_err.json')


def draw_max_err():
    eps_lst = [0.001 * (i + 1) for i in xrange(1, 15, 1)]
    xtick_lst = [0.005, 0.01, 0.015]
    exp_figure, ax_tuple = plt.subplots(1, 1, sharex=True, figsize=(8, 4))

    # 1st: draw querying time
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_d_tag, reads_rq_tag, isp_tag, tsf_tag]
    legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING',
                  'READS-D', 'READS-RQ', 'ISP', 'TSF']

    ax = ax_tuple
    lst_lst = []
    for idx, algorithm in enumerate(algorithm_tag_lst):
        max_err_lst = map(lambda eps: eps_max_err_dict[algorithm][format_str(eps)], eps_lst)

        max_err_lst = map(lambda err: 0.025 if err > 0.025 else err, max_err_lst)
        lst_lst.append(max_err_lst)
        color_lst = ['blue', 'orange', 'green', 'red',
                     '#fe01b1', '#ceb301', 'm', 'brown', 'k', 'gray']
        shape_lst = ['D-.', 's--', 'o:', 'x-',
                     'P-', '*-',
                     'v-', '^-', '<-', '>-']

        def get_marker_size(idx):
            if idx == 0:
                return 12
            elif idx == 5:
                return 16
            else:
                return 14

        ax.plot(eps_lst, max_err_lst, shape_lst[idx], color=color_lst[idx], markersize=get_marker_size(idx),
                markerfacecolor='none')
        # ax.set_yscale('log')

    # setup ticks for x and y axis
    # ax.set_ylim(0.8 / us_to_ms_factor, 10 ** 10 * 4 / us_to_ms_factor)
    ax.set_ylim(0, 0.028)

    # ax.set_yticks(map(lambda digit: 10 ** (-digit), [4, 3, 2, 1, 0]))
    ax.set_xticks(xtick_lst)
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE)
    # setup labels and grid, legend
    ax.set_ylabel('Maximum Error', fontsize=LABEL_SIZE)
    ax.set_xlabel('$\\epsilon$', fontsize=LABEL_SIZE)
    ax.grid(True, alpha=0.4)
    # ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=1)
    plt.tight_layout()
    # plt.subplots_adjust(right=0.5)
    plt.legend(legend_lst, ncol=1, fontsize=LEGEND_SIZE, prop={'size': LEGEND_SIZE, "weight": "bold"},
               bbox_to_anchor=(1.05, 1.1))
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()
    plt.savefig('figures/' + 'varying_eps_max_err' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


if __name__ == '__main__':
    # unit: us
    algorithm_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                     reads_d_tag, reads_rq_tag, isp_tag, tsf_tag]
    for algorithm in algorithm_lst:
        print algorithm, eps_max_err_dict[algorithm]

    draw_max_err()
