from draw_indexing_time_size import *
from data_analysis.querying_time_accuracy_statistics import *
from data_analysis.generate_speedup_over_sling_markdown import get_cpu_time_dict_with_reads

g_cpu_time_dict = get_cpu_time_dict_with_reads(root_dir='../data_analysis')
us_to_ms_factor = 10 ** 3


def get_algorithm_time_lst(algorithm, data_lst, cpu_time_dict=g_cpu_time_dict):
    def select_first_data_set(my_lst):
        return my_lst[0]

    def get_time(data_set):
        ret_data = 9999999999
        if algorithm in [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag]:
            ret_data = select_first_data_set(cpu_time_dict[algorithm][data_set][str(10 ** 6)])
        elif algorithm in [tsf_tag, isp_tag, reads_d_tag, reads_rq_tag]:
            ret_data = select_first_data_set(cpu_time_dict[algorithm][data_set][str(10 ** 5)]) * 10
        else:
            lst = map(lambda my_str: select_first_data_set(cpu_time_dict[algorithm][data_set][my_str]),
                      map(str, [10 ** 5, 10 ** 4, 10 ** 3]))
            for idx, number in enumerate(lst):
                if number is not None:
                    ret_data = number * (10 ** (idx + 1))
                    break
        if ret_data != 9999999999:
            ret_data = ret_data / 2
        return ret_data

    return map(get_time, data_lst)


def draw_average_query_one_pair_time():
    print data_set_lst
    g_names = map(lambda data: data_names[data], data_set_lst)

    size_of_fig = (FIG_SIZE_MULTIPLE)
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.1
    ind = 1.1 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(10))

    # other lst
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_d_tag, reads_rq_tag,
                         isp_tag, tsf_tag, lind_tag, cw_tag]
    is_fill = [False] * len(algorithm_tag_lst)
    is_fill[1] = True
    hatch_lst = ['//', '', '.', "**", '', 'O', '\\', '--', 'x', '++']
    label_lst = ["FBLPMC", "FLPMC", "BLPMC", "SLING", 'READS-D', 'READS-Rq', "ISP", "TSF", "LIN", "MCSP"]
    color_lst = ['blue', 'orange', 'green', 'red', '#fe01b1', '#ceb301', 'm', 'brown', 'k', 'gray']

    # 1st: bars
    for idx, tag in enumerate(algorithm_tag_lst):
        ax.bar(indent_lst[idx],
               map(lambda val: float(val) / us_to_ms_factor, get_algorithm_time_lst(tag, data_set_lst)), width,
               hatch=hatch_lst[idx],
               edgecolor=color_lst[idx],
               # color=color_lst[idx],
               label=label_lst[idx], fill=is_fill[idx])

    # 2nd: x and y's ticks and labels
    ax.set_xticks(ind + 4.5 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)

    plt.ylim(10 ** (-3) / 2, 10 ** 6)
    plt.yscale('log')
    ax.set_ylabel("Avg Query Time (ms)", fontsize=LABEL_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=5)
    fig.savefig("./figures/" + 'avg_query_time.pdf', bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    draw_average_query_one_pair_time()
