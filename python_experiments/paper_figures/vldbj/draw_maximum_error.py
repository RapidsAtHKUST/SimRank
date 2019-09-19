import matplotlib.pyplot as plt

from data_analysis.vldbj_data_parsing.generate_accuracy_markdown import *
from data_analysis.vldbj_data_parsing.probesim_querying_time_statistics import probesim_gt_tag
from paper_figures.vldbj.draw_indexing_time_size import get_name_dict, LABEL_SIZE, TICK_SIZE, LEGEND_SIZE

data_names = get_name_dict()


def get_algorithm_max_err_lst(algorithm_tag, data_set):
    accuracy_dict = get_accuracy_dict_with_reads('../../data_analysis')
    return accuracy_dict[algorithm_tag][data_set][str(10 ** 5)]


def draw_figures_max_err():
    data_set_lst = accuracy_data_set_lst
    round_lst = list(range(10))
    exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 7))
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_d_tag, reads_rq_tag,
                         isp_tag, tsf_tag, lind_tag, cw_tag, probesim_gt_tag]
    legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING',
                  'READS-D', 'READS-Rq', 'ISP', 'TSF', 'LIN', 'MCSP', 'ProbeSim', '$\\epsilon$-Bound']

    for ax_idx, ax in enumerate(ax_tuple):
        max_err_lst_lst = []
        for idx, algorithm in enumerate(algorithm_tag_lst):
            time_lst = get_algorithm_max_err_lst(algorithm, data_set_lst[ax_idx])
            max_err_lst_lst.append(time_lst)
            color_lst = ['blue', 'orange', 'green', 'red',
                         '#fe01b1', '#ceb301', 'm', 'brown', 'k', 'gray', 'purple']
            # shape_lst = ['D-.', 's--', 'o:', 'x-',
            #              'P-', '*-',
            #              'v-', '^-', '<-', '>-']
            shape_lst = ['D', 's', 'o', 'x',
                         'P', '*',
                         'v', '^', '<', '>', '+']
            cur_mark_size = 16 if idx != 5 else 20
            if idx == 10:
                cur_mark_size = 30
            ax.plot(round_lst, time_lst, shape_lst[idx], markersize=cur_mark_size if idx != 0 else cur_mark_size - 4,
                    markerfacecolor='none',
                    color=color_lst[idx])
            ax.set_yscale('log')
        ax.plot([-1] + round_lst + [10], [10 ** (-2) for _ in range(len(round_lst) + 2)], '--',
                markerfacecolor='none', color='black')
        plt.yticks(fontsize=TICK_SIZE)

    # sub title on the top
    sub_titles = ['(a) dataset = ', '(b) dataset = ', '(c) dataset = ', '(d) dataset = ']
    for idx in range(len(sub_titles)):
        sub_titles[idx] += data_names[accuracy_data_set_lst[idx]]

    for idx, my_ax in enumerate(ax_tuple):
        # my_ax.set_title(sub_titles[idx], fontsize=LABEL_SIZE)
        if idx == 0:
            plt.yticks(fontsize=LABEL_SIZE)
            my_ax.set_ylabel('Maximum Error', fontsize=LABEL_SIZE + 6)
        for tick in my_ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE)
        my_ax.set_xlabel('Round Number\n' + sub_titles[idx], fontsize=LABEL_SIZE + 6)
        my_ax.set_xlim(-0.5, 9.5)
        my_ax.set_xticks(range(10))
        my_ax.set_xticklabels([i + 1 for i in range(10)], fontsize=TICK_SIZE)
        # my_ax.grid(True, alpha=0.2)

    exp_figure.subplots_adjust(wspace=0, hspace=0.2)
    plt.tight_layout()
    plt.legend(legend_lst,
               ncol=int(len(legend_lst) / 2),
               # ncol=len(legend_lst) / 2,
               fontsize=LEGEND_SIZE,
               prop={'size': LEGEND_SIZE + 3, "weight": "bold"},
               bbox_to_anchor=(0.3, 1.3), handletextpad=2.5
               )
    plt.savefig('figures/' + 'max_err' + '.pdf', bbox_inches='tight', dpi=300)
    # exp_figure.show()
    plt.close()


if __name__ == '__main__':
    draw_figures_max_err()
