from data_analysis.vldbj_data_parsing.probesim_querying_time_statistics import probesim_tag
from paper_figures.vldbj.draw_indexing_time_size import *
from data_analysis.vldbj_data_parsing.querying_time_accuracy_statistics import *
from data_analysis.vldbj_data_parsing.generate_speedup_over_sling_markdown import get_cpu_time_dict_with_reads

g_cpu_time_dict = get_cpu_time_dict_with_reads(root_dir='../../data_analysis')


def get_cpu_time_others(root_dir='.'):
    with open(os.sep.join([root_dir, 'data-json', 'new_datasets', 'query_result_full_cpu_time_all.json'])) as ifs:
        return json.load(ifs)


g_cpu_time_dict_others = get_cpu_time_others('../../data_analysis')

us_to_ms_factor = 10 ** 3


def get_algorithm_time_lst(algorithm, data_lst):
    def select_first_data_set(my_lst):
        return my_lst[0]

    def get_time(data_set):
        cpu_time_dict = g_cpu_time_dict_others if data_set in ['digg-friends', 'flickr-growth'] else g_cpu_time_dict
        ret_data = 9999999999
        if algorithm in [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag]:
            ret_data = select_first_data_set(cpu_time_dict[algorithm][data_set][str(10 ** 6)])
        elif algorithm in [tsf_tag, isp_tag, reads_d_tag, reads_rq_tag]:
            ret_data = select_first_data_set(cpu_time_dict[algorithm][data_set][str(10 ** 5)]) * 10
        elif algorithm in [probesim_tag]:
            lst = map(lambda my_str: select_first_data_set(cpu_time_dict[algorithm][data_set][my_str]),
                      map(str, [10 ** 5, 10 ** 4]))
            for idx, number in enumerate(lst):
                if number is not None and number < 999999999999999:
                    ret_data = number * (10 ** (idx + 1))
                    break
        else:
            lst = map(lambda my_str: select_first_data_set(cpu_time_dict[algorithm][data_set][my_str]),
                      map(str, [10 ** 5, 10 ** 4, 10 ** 3]))
            if data_set in ['digg-friends', 'flickr-growth']:
                print(algorithm, lst)
            for idx, number in enumerate(lst):
                if number is not None and number != 999999999999999:
                    ret_data = number * (10 ** (idx + 1))
                    if data_set in ['digg-friends', 'flickr-growth']:
                        print(ret_data)
                    break
        if ret_data is None:
            ret_data = 9999999999
        if ret_data != 9999999999:
            ret_data = ret_data / 2
        return ret_data

    # print algorithm
    return map(get_time, data_lst)


def draw_average_query_one_pair_time():
    global data_set_lst
    with open('../vldbj/data_set_lst.json') as ifs:
        data_set_lst = json.load(ifs)

    g_names = list(map(lambda data: data_names[data], data_set_lst))
    algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag,
                         reads_d_tag, reads_rq_tag,
                         isp_tag, tsf_tag, lind_tag, cw_tag, probesim_tag]

    avg_query_time_dict = dict()
    for idx, tag in enumerate(algorithm_tag_lst):
        # out of memory > 192GB

        my_data_lst = list(
            map(lambda val: float(val) / us_to_ms_factor / us_to_ms_factor, get_algorithm_time_lst(tag, data_set_lst)))
        if tag == reads_d_tag:
            my_data_lst[-1] = 0
        avg_query_time_dict[tag] = dict(zip(data_set_lst, my_data_lst))
        with open('data_legacy/query.json', 'w') as ofs:
            ofs.write(json.dumps(avg_query_time_dict, indent=4))


if __name__ == '__main__':
    draw_average_query_one_pair_time()
