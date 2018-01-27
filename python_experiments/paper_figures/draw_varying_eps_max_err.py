import matplotlib.pyplot as plt
from data_analysis.varying_eps_statistics import *
from draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE, reads_d_tag, reads_rq_tag
import json


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


eps_max_err_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_eps_max_err.json')


def draw_max_err():
    eps_lst = [0.001 * (i + 1) for i in xrange(1, 15, 1)]
    xtick_lst = [0, 0.005, 0.01, 0.015]
    exp_figure, ax_tuple = plt.subplots(1, 1, sharex=True, figsize=(10.5, 5))

    # 1st: draw querying time
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_d_tag, reads_rq_tag, isp_tag, tsf_tag]
    legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING',
                  'READS-D', 'READS-RQ', 'ISP', 'TSF', '$\\epsilon$-Bound']

    ax = ax_tuple
    lst_lst = []
    for idx, algorithm in enumerate(algorithm_tag_lst):
        max_err_lst = map(lambda eps: eps_max_err_dict[algorithm][format_str(eps)], eps_lst)

        max_err_lst = map(lambda err: 0.015 if err > 0.015 else err, max_err_lst)
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
                return 20
            else:
                return 14

        ax.plot(eps_lst, max_err_lst, shape_lst[idx], color=color_lst[idx], markersize=get_marker_size(idx),
                markerfacecolor='none')
    total_eps_lst = [0.001 * i for i in xrange(17)]
    ax.plot(total_eps_lst, total_eps_lst, '--', color='black', markerfacecolor='none')
    # setup ticks for x and y axis

    ax.set_xlim(0,0.016)
    ax.set_ylim(0, 0.016)
    ax.set_yticks([0, 0.005, 0.010, 0.015])
    ax.set_yticklabels(['0', '0.005', '0.010', '> 0.015'])
    ax.set_xticks(xtick_lst)
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE)

    # setup labels and grid, legend
    ax.set_ylabel('Maximum Error', fontsize=LABEL_SIZE + 3)
    ax.set_xlabel('$\\epsilon$', fontsize=LABEL_SIZE)
    ax.grid(True, alpha=0.4)
    plt.tight_layout()
    plt.legend(legend_lst, ncol=1, fontsize=LEGEND_SIZE, prop={'size': LEGEND_SIZE, "weight": "bold"},
               bbox_to_anchor=(1.0, 1.05))
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
