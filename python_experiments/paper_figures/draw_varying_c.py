import matplotlib.pyplot as plt
from data_analysis.varying_c_statistics import *
from draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE
import json

us_to_ms_factor = 10 ** 3
large_size_plus = 4


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


c_index_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_c_index.json')
c_query_dict = get_dict('../data_analysis/data-json/varying_parameters/varying_c_query.json')


def draw_query_index_time():
    exp_figure, ax_tuple = plt.subplots(1, 2, sharex=True, figsize=(16, 6))
    c_lst = [0.4, 0.5, 0.6, 0.7, 0.8]
    # 1st: draw querying time
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, isp_tag, tsf_tag]
    legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING', 'ISP', 'TSF']

    ax = ax_tuple[0]
    lst_lst = []
    for idx, algorithm in enumerate(algorithm_tag_lst):
        time_lst = map(lambda c: c_query_dict[algorithm][format_str(c)], c_lst)
        time_lst = map(lambda val: float(val) / us_to_ms_factor, time_lst)
        lst_lst.append(time_lst)
        shape_lst = ['D-.', 's--', 'o:', 'x-', 'v-', '^-', '<-', '>-']
        color_lst = ['blue', 'orange', 'green', 'red', 'm', 'brown', 'pink', 'gray']
        ax.plot(c_lst, time_lst, shape_lst[idx], color=color_lst[idx], markersize=22 if idx != 0 else 18,
                markerfacecolor='none')
        ax.set_yscale('log')

    # setup ticks for x and y axis
    ax.set_ylim(0.4 / us_to_ms_factor, 10 ** 8 * 0.3 / us_to_ms_factor)
    ax.set_xticks(c_lst)

    # setup font size for ticks and labels
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    ax.set_ylabel('Avg Query Time (ms)', fontsize=LABEL_SIZE + large_size_plus)
    ax.set_xlabel('$c$', fontsize=LABEL_SIZE + large_size_plus)
    ax.grid(True, alpha=0.4)
    ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=1)

    # 2nd: draw the index
    algorithm_tag_lst = [flp_tag, sling_tag, tsf_tag]
    legend_lst = ['FLP', 'SLING', 'TSF']

    ax = ax_tuple[1]
    lst_lst = []
    for idx, algorithm in enumerate(algorithm_tag_lst):
        time_lst = map(lambda c: c_index_dict[algorithm][format_str(c)], c_lst)
        if algorithm in [tsf_tag]:
            time_lst = map(lambda time_val: 0.0042 if time_val > 0.005 else time_val, time_lst)
        lst_lst.append(time_lst)

        shape_lst = ['D-.', 'x-', '^-']
        color_lst = ['blue', 'red', 'brown']
        ax.plot(c_lst, time_lst, shape_lst[idx], color=color_lst[idx], markersize=22 if idx != 0 else 18,
                markerfacecolor='none')
        ax.set_yscale('log')

    # setup ticks for x and y axis
    ax.set_ylim(10 ** -3, 10 ** 5 * 2)
    for tick in ax.yaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    for tick in ax.xaxis.get_major_ticks():
        tick.label.set_fontsize(TICK_SIZE + large_size_plus)
    # setup labels and grid, legend
    ax.set_ylabel('Indexing Time (s)', fontsize=LABEL_SIZE + large_size_plus)
    ax.set_xlabel('$c$', fontsize=LABEL_SIZE + large_size_plus)
    ax.grid(True, alpha=0.4)
    ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=1)

    # 3rd: save the figure
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()
    plt.savefig('figures/' + 'varying_c' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


if __name__ == '__main__':
    # unit: us
    algorithm_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, tsf_tag]
    for algorithm in algorithm_lst:
        print algorithm, c_query_dict[algorithm]

    print
    index_lst = [flp_tag, sling_tag, tsf_tag]
    for algorithm in index_lst:
        print algorithm, c_index_dict[algorithm]
    draw_query_index_time()
