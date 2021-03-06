import matplotlib.pyplot as plt
from data_analysis.vldbj_data_parsing.varying_c_statistics import *
from paper_figures.vldbj.draw_indexing_time_size import TICK_SIZE, LEGEND_SIZE, LABEL_SIZE, reads_d_tag, reads_rq_tag
import json
import itertools

us_to_ms_factor = 10 ** 3
large_size_plus = 4


def get_dict(file_path):
    with open(file_path) as ifs:
        return json.load(ifs)


relative_path = '../..'
c_index_dict = get_dict('{}/data_analysis/data-json/varying_parameters/varying_c_index.json'.format(relative_path))
c_reads_index_dict = get_dict(
    '{}/data_analysis/data-json/varying_parameters/varying_c_index_reads.json'.format(relative_path))
c_index_dict = dict(itertools.chain(c_index_dict.items(), c_reads_index_dict.items()))

c_query_dict = get_dict('{}/data_analysis/data-json/varying_parameters/varying_c_query.json'.format(relative_path))
c_reads_query_dict = get_dict(
    '{}/data_analysis/data-json/varying_parameters/varying_c_query_reads.json'.format(relative_path))
c_probesim_query_dict = get_dict(
    '{}/data_analysis/data-json/varying_parameters/probesim_varying_c_query.json'.format(relative_path))
c_query_dict = dict(
    itertools.chain(c_query_dict.items(), c_reads_query_dict.items(), c_probesim_query_dict.items()))


# print c_query_dict

def draw_query_index_time():
    exp_figure, ax_tuple = plt.subplots(1, 2, sharex=True, figsize=(16, 7))
    c_lst = [0.4, 0.5, 0.6, 0.7, 0.8]

    # 1st: draw querying time
    def draw_querying_time():
        algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                             reads_d_tag, reads_rq_tag, isp_tag, tsf_tag, probesim_tag]
        legend_lst = ['FBLPMC', 'FLPMC', 'BLPMC', 'SLING',
                      'READS-D', 'READS-Rq',
                      'ISP', 'TSF', 'ProbeSim']

        ax = ax_tuple[0]
        lst_lst = []
        for idx, algorithm in enumerate(algorithm_tag_lst):
            time_lst = list(map(lambda c: c_query_dict[algorithm][format_str(c)], c_lst))
            time_lst = list(map(lambda val: float(val) / us_to_ms_factor if val is not None else None, time_lst))

            lst_lst.append(time_lst)
            color_lst = ['blue', 'orange', 'green', 'red',
                         '#fe01b1', '#ceb301',
                         'm', 'brown', 'purple', 'k', 'gray']
            shape_lst = ['D-.', 's--', 'o:', 'x-',
                         'P-', '*-',
                         'v-', '^-', '+-',
                         '<-', '>-', ]

            def get_marker_size():
                if idx == 0:
                    return 18
                elif idx == 5:
                    return 26
                elif idx == 8:
                    return 30
                else:
                    return 22

            # print idx, algorithm, time_lst
            ax.plot(c_lst, time_lst, shape_lst[idx], color=color_lst[idx],
                    markersize=get_marker_size(),
                    markerfacecolor='none')
            # print 'after plot', idx, algorithm

        # setup ticks for x and y axis
        ax.set_ylim(0.4 / us_to_ms_factor, 10 ** 10.5 * 0.3 / us_to_ms_factor)
        ax.set_xticks(c_lst)
        ax.set_yscale('log')

        # setup font size for ticks and labels
        for tick in ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE + large_size_plus)
        for tick in ax.xaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE + large_size_plus)
        ax.set_ylabel('Avg Query Time (ms)', fontsize=LABEL_SIZE + large_size_plus)
        ax.set_xlabel('$c$', fontsize=LABEL_SIZE + large_size_plus)
        ax.grid(True, alpha=0.4)
        ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE - 2, "weight": "bold"}, loc=1)

    # 2nd: draw the index
    def draw_idx():
        algorithm_tag_lst = [flp_tag, sling_tag, reads_d_tag, reads_rq_tag, tsf_tag]
        legend_lst = ['FLP', 'SLING', 'READS-D', 'READS-Rq', 'TSF']

        ax = ax_tuple[1]
        lst_lst = []
        for idx, algorithm in enumerate(algorithm_tag_lst):
            time_lst = list(map(lambda c: c_index_dict[algorithm][format_str(c)], c_lst))
            if algorithm in [tsf_tag]:
                time_lst = list(map(lambda time_val: 0.0042 if time_val > 0.005 else time_val, time_lst))
            lst_lst.append(time_lst)

            shape_lst = ['D-.', 'x-', 'P-', '*-', '^-']
            color_lst = ['blue', 'red', '#fe01b1', '#ceb301', 'brown']

            def get_marker_size():
                if idx == 0:
                    return 18
                elif idx == 3:
                    return 26
                else:
                    return 22

            ax.plot(c_lst, time_lst, shape_lst[idx], color=color_lst[idx],
                    markersize=get_marker_size(),
                    markerfacecolor='none')
            ax.set_yscale('log')

        # setup ticks for x and y axis
        ax.set_ylim(10 ** -3, 10 ** 5 * 12)
        for tick in ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE + large_size_plus)
        for tick in ax.xaxis.get_major_ticks():
            tick.label.set_fontsize(TICK_SIZE + large_size_plus)
        # setup labels and grid, legend
        ax.set_ylabel('Indexing Time (s)', fontsize=LABEL_SIZE + large_size_plus)
        ax.set_xlabel('$c$', fontsize=LABEL_SIZE + large_size_plus)
        ax.grid(True, alpha=0.2)
        ax.legend(legend_lst, ncol=2, prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=1)

    draw_querying_time()
    draw_idx()

    # 3rd: save the figure
    exp_figure.subplots_adjust(wspace=0)
    plt.tight_layout()
    plt.savefig('figures/' + 'varying_c' + '.pdf', bbox_inches='tight', dpi=300)
    plt.close()


if __name__ == '__main__':
    # unit: us
    algorithm_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, tsf_tag, probesim_tag]
    # for algorithm in algorithm_lst:
    #     print algorithm, c_query_dict[algorithm]

    index_lst = [flp_tag, sling_tag, tsf_tag]
    # for algorithm in index_lst:
    #     print algorithm, c_index_dict[algorithm]
    draw_query_index_time()
