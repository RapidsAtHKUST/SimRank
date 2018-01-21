import matplotlib.pyplot as plt

from data_analysis.generate_accuracy_markdown import *
from draw_indexing_time_size import get_name_dict, LABEL_SIZE, TICK_SIZE, LEGEND_SIZE

data_names = get_name_dict()


def get_algorithm_max_err_lst(algorithm_tag, data_set):
    accuracy_dict = get_accuracy_dict('../data_analysis')
    return accuracy_dict[algorithm_tag][data_set][str(10 ** 5)]


def draw_figures_max_err():
    data_set_lst = accuracy_data_set_lst
    round_lst = xrange(10)
    exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 6))
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, isp_tag, tsf_tag, lind_tag, cw_tag]
    legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING', 'ISP', 'TSF', 'LIN', 'MCSP']

    for ax_idx, ax in enumerate(ax_tuple):
        max_err_lst_lst = []
        for idx, algorithm in enumerate(algorithm_tag_lst):
            time_lst = get_algorithm_max_err_lst(algorithm, data_set_lst[ax_idx])
            max_err_lst_lst.append(time_lst)
            color_lst = ['blue', 'orange', 'green', 'red', 'm', 'brown', 'pink', 'gray']
            shape_lst = ['H-.', 's--', 'o:', 'x-', 'v-', '^-', '<-', '>-']
            cur_mark_size = 22
            ax.plot(round_lst, time_lst, shape_lst[idx], markersize=cur_mark_size, markerfacecolor='none',
                    color=color_lst[idx])
            ax.set_yscale('log')
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
        my_ax.set_xlabel('Round Number\n' + sub_titles[idx], fontsize=LABEL_SIZE + 4)
        my_ax.set_xticks(range(10))
        my_ax.set_xticklabels([i + 1 for i in range(10)], fontsize=TICK_SIZE)
        my_ax.grid(True, alpha=0.4)

    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()
    plt.subplots_adjust(top=0.8)
    plt.legend(legend_lst, ncol=len(legend_lst), fontsize=LEGEND_SIZE, prop={'size': LEGEND_SIZE + 3, "weight": "bold"},
               bbox_to_anchor=(0.32, 1.25))
    plt.savefig('figures/' + 'max_err' + '.pdf', bbox_inches='tight', dpi=300)
    # exp_figure.show()
    plt.close()


if __name__ == '__main__':
    draw_figures_max_err()
