import matplotlib.pyplot as plt
from data_analysis.varying_eps_statistics import *
from draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE
import json

from paper_figures.draw_varying_c import us_to_ms_factor, large_size_plus


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


rmax_index_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_rmax_index.json')
rmax_query_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_rmax_query.json')


def draw_query_index_time():
    # rmax_lst = [0.01 * (i + 1) for i in xrange(0, 30, 1)]
    rmax_lst = [0.01 * (i + 1) for i in xrange(4, 10, 1)]
    exp_figure, ax_tuple = plt.subplots(1, 2, sharex=True, figsize=(16, 6))

    # 1st: draw querying time
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag]
    legend_lst = ['FBLPMC', 'FLPMC']

    ax = ax_tuple[0]
    lst_lst = []
    for idx, algorithm in enumerate(algorithm_tag_lst):
        time_lst = map(lambda rmax: rmax_query_dict[algorithm][format_str(rmax)], rmax_lst)
        time_lst = map(lambda val: float(val) / us_to_ms_factor, time_lst)
        lst_lst.append(time_lst)
        shape_lst = ['H-.', 's--', 'o:', 'x-', 'v-', '^-', '<-', '>-']
        color_lst = ['blue', 'orange', 'green', 'red', 'm', 'brown', 'pink', 'gray']
        ax.plot(rmax_lst, time_lst, shape_lst[idx], color=color_lst[idx], markersize=18, markerfacecolor='none')
        # ax.set_yscale('log')

    # setup ticks for x and y axis
    ax.set_ylim(0, 0.04)
    ax.set_yticks([0, 0.01, 0.02, 0.03, 0.04])
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    # setup labels and grid, legend
    ax.set_ylabel('Avg Query Time (ms)', fontsize=LABEL_SIZE + large_size_plus)
    ax.set_xlabel('$r_{max}$', fontsize=LABEL_SIZE + large_size_plus)
    ax.grid(True, alpha=0.4)
    ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=1)

    # 2nd: draw the index
    algorithm_tag_lst = [flp_tag]
    legend_lst = ['FLP']

    ax = ax_tuple[1]
    lst_lst = []
    for idx, algorithm in enumerate(algorithm_tag_lst):
        time_lst = map(lambda rmax: rmax_index_dict[algorithm][format_str(rmax)], rmax_lst)
        lst_lst.append(time_lst)

        shape_lst = ['H-.', 'x-', '^-']
        color_lst = ['blue', 'red', 'brown']
        ax.plot(rmax_lst, time_lst, shape_lst[idx], color=color_lst[idx], markersize=18, markerfacecolor='none')
        # ax.set_yscale('log')

    # setup ticks for x and y axis
    ax.set_ylim(0.03, 0.05)

    ax.set_yticks([0.03, 0.035, 0.040, 0.045, 0.05])
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    # setup labels and grid, legend
    ax.set_ylabel('Indexing Time (s)', fontsize=LABEL_SIZE + large_size_plus)
    ax.set_xlabel('$r_{max}$', fontsize=LABEL_SIZE + large_size_plus)
    ax.grid(True, alpha=0.4)
    ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=1)

    # 3rd: save the figure
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()
    plt.savefig('figures/' + 'varying_rmax' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


if __name__ == '__main__':
    # unit: us
    algorithm_lst = [bflpmc_tag, flpmc_tag]
    for algorithm in algorithm_lst:
        print algorithm, rmax_query_dict[algorithm]

    print
    index_lst = [flp_tag]
    for algorithm in index_lst:
        print algorithm, rmax_index_dict[algorithm]
    draw_query_index_time()
