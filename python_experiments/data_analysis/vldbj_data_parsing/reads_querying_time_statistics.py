from data_analysis.vldbj_data_parsing.reads_indexing_statistics import *

pair_num_lst = [10 ** 4, 10 ** 5]

query_time_tag = 'total query cpu time'


def get_time_lst(data_set, pair_num, round_num, result_file_name):
    cpu_time_lst = []
    stat_folder = reads_efficiency_folder

    for round_i in range(round_num):
        my_path = os.sep.join(map(str, [stat_folder, data_set, pair_num, round_i, result_file_name]))
        # use the first one for this round
        querying_time = get_tag_info(my_path, query_time_tag)
        if querying_time is None:
            querying_time = 999999999999999
        cpu_time_lst.append(querying_time)
    return cpu_time_lst


def get_query_time(data_set, algorithm_name, pair_num, round_idx):
    algorithm_log_name = algorithm_name + suffix_str
    file_path = os.sep.join([reads_efficiency_folder, data_set, str(pair_num), str(round_idx), algorithm_log_name])
    querying_time = get_tag_info(file_path, 'total query cpu time')
    if querying_time is None:
        return 999999999999999.
    return float(format_str(querying_time)) / 2.


def get_time_lst_per_algorithm(round_num, result_file_name):
    query_time_dict_lst = []
    for data_set in data_set_lst:
        query_time_dict_lst.append(
            dict(zip(pair_num_lst, map(lambda pair_num: get_time_lst(data_set, pair_num, round_num,
                                                                     result_file_name + suffix_str), pair_num_lst))))
    return dict(zip(data_set_lst, query_time_dict_lst))


if __name__ == '__main__':
    querying_dict = {
        reads_tag: get_time_lst_per_algorithm(1, reads_tag),
        reads_d_tag: get_time_lst_per_algorithm(1, reads_d_tag),
        reads_rq_tag: get_time_lst_per_algorithm(1, reads_rq_tag)
    }
    with open('data-json/query_result_full_cpu_time_reads.json', 'w') as ofs:
        ofs.write(json.dumps(querying_dict, indent=4))
