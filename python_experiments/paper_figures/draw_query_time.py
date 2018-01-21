from draw_indexing_time_size import *
from data_analysis.querying_time_accuracy_statistics import *


def get_cpu_time_dict(root_dir='.'):
    with open(root_dir + os.sep + 'data-json' + os.sep + 'query_result_full_total_query_cpu_time' + '.json') as ifs:
        return json.load(ifs)


g_cpu_time_dict = get_cpu_time_dict('../data_analysis')

us_to_ms_factor = 10 ** 3


def get_algorithm_time_lst(algorithm, data_lst, cpu_time_dict=g_cpu_time_dict):
    def select_first_data_set(my_lst):
        return my_lst[0]

    def get_time(data_set):
        ret_data = 9999999999
        if algorithm in [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag]:
            ret_data = select_first_data_set(cpu_time_dict[algorithm][data_set][str(10 ** 6)])
        elif algorithm in [tsf_tag, isp_tag]:
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
    g_names = map(lambda data: data_names[data], data_set_lst)

    size_of_fig = (FIG_SIZE_MULTIPLE)
    fig, ax = plt.subplots()
    N = len(g_names)

    # indent lst
    width = 0.125
    ind = 1.125 * np.arange(N)  # the x locations for the groups
    indent_lst = map(lambda idx: ind + idx * width, range(8))

    # other lst
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag, isp_tag, tsf_tag, lind_tag, cw_tag]

    hatch_lst = ['//', '*', 'o', ".", '\\', '-', 'x', '+']
    label_lst = ["FBLPMC", "FLPMC", "BLPMC", "SLING", "ISP", "TSF", "LIN", "MCSP"]

    # 1st: bars
    for idx, tag in enumerate(algorithm_tag_lst):
        ax.bar(indent_lst[idx],
               map(lambda val: float(val) / us_to_ms_factor, get_algorithm_time_lst(tag, data_set_lst)), width,
               hatch=hatch_lst[idx],
               # edgecolor=color_lst[idx],
               label=label_lst[idx], fill=False)

    # 2nd: x and y's ticks and labels
    ax.set_xticks(ind + 2 * width)
    ax.set_xticklabels(g_names, fontsize=LABEL_SIZE)
    plt.xticks(fontsize=TICK_SIZE)

    plt.yscale('log')
    ax.set_ylabel("Avg Query Time (ms)", fontsize=LABEL_SIZE)
    plt.yticks(fontsize=TICK_SIZE)

    # 3rd: figure properties
    fig.set_size_inches(*size_of_fig)  # set ratio
    plt.legend(prop={'size': LEGEND_SIZE, "weight": "bold"}, loc="upper left", ncol=8)
    fig.savefig("./figures/" + 'avg_query_time.pdf', bbox_inches='tight', dpi=300)


if __name__ == '__main__':
    draw_average_query_one_pair_time()