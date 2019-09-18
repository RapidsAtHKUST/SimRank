import matplotlib.pyplot as plt

from data_analysis.probesim_querying_time_statistics import probesim_gt_tag
from data_analysis.varying_eps_statistics import *
from paper_figures.draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE, reads_d_tag, reads_rq_tag, get_name_dict
import json


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


eps_max_err_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_eps_max_err.json')
tmp = get_dict(
    '../data_analysis/data-json/varying_parameters/varying_eps_max_err_probesim.json')
for data_set in eps_max_err_dict:
    eps_max_err_dict[data_set][probesim_gt_tag] = tmp[data_set][probesim_gt_tag]
    tmp_dct = eps_max_err_dict[data_set][probesim_gt_tag]
    for i in range(3, 1, -1):
        tmp_str = '0.00' + str(i)
        tmp_str2 = '0.00' + str(i + 1)
        if tmp_dct[tmp_str] is None:
            tmp_dct[tmp_str] = tmp_dct[tmp_str2]
data_names = get_name_dict()


def draw_max_err():
    eps_lst = [0.001 * (i + 1) for i in range(1, 15, 1)]
    xtick_lst = [0, 0.005, 0.01, 0.015]
    exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 7))

    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_d_tag, reads_rq_tag, isp_tag, tsf_tag, probesim_gt_tag]
    legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING',
                  'READS-D', 'READS-Rq', 'ISP', 'TSF', 'ProbeSim', '$\\epsilon$-Bound']
    data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
    color_lst = ['blue', 'orange', 'green', 'red',
                 '#fe01b1', '#ceb301', 'm', 'brown', 'purple', 'k', 'gray']
    shape_lst = ['D-.', 's--', 'o:', 'x-',
                 'P-', '*-',
                 'v-', '^-', '+-', '<-', '>-', ]
    caption_lst = ['(a) dataset = ', '(b) dataset = ', '(c) dataset = ', '(d) dataset = ']
    for ax_idx, ax in enumerate(ax_tuple):
        lst_lst = []
        data_set_name = data_set_lst[ax_idx]
        for idx, algorithm in enumerate(algorithm_tag_lst):
            max_err_lst = list(map(lambda eps: eps_max_err_dict[data_set_name][algorithm][format_str(eps)], eps_lst))
            print(max_err_lst)
            max_err_lst = list(map(lambda err: 0.015 if err is not None and err > 0.015 else err, max_err_lst))

            lst_lst.append(max_err_lst)

            def get_marker_size(idx):
                if idx == 0:
                    return 12
                elif idx == 5:
                    return 20
                elif idx == 8:
                    return 30
                else:
                    return 14

            ax.plot(eps_lst, max_err_lst, shape_lst[idx], color=color_lst[idx], markersize=get_marker_size(idx),
                    markerfacecolor='none')
        total_eps_lst = [0.001 * i for i in range(17)]
        ax.plot(total_eps_lst, total_eps_lst, '--', color='black', markerfacecolor='none')

        # setup ticks for x and y axis
        ax.set_xlim(0, 0.016)
        ax.set_ylim(0, 0.016)
        ax.set_yticks([0, 0.005, 0.010, 0.015])
        ax.set_yticklabels(['0', '0.005', '0.010', '> 0.015'])
        ax.set_xticks(xtick_lst)
        for tick in ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        for tick in ax.xaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        ax.set_xlabel('$\\epsilon$\n' + caption_lst[ax_idx] + data_names[data_set_name], fontsize=LABEL_SIZE + 6)
    ax = ax_tuple[0]
    # setup labels and grid, legend
    ax.set_ylabel('Maximum Error', fontsize=LABEL_SIZE + 6)

    # ax.grid(True, alpha=0.4)

    plt.tight_layout()
    plt.legend(legend_lst, ncol=len(legend_lst), fontsize=LEGEND_SIZE, prop={'size': LEGEND_SIZE + 3, "weight": "bold"},
               bbox_to_anchor=(0.9, 1.15))
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()
    plt.savefig('figures/' + 'varying_eps_max_err' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


if __name__ == '__main__':
    draw_max_err()
