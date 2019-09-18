import os, json, decimal
from data_analysis.reads_varying_parameters_statistics.varying_c_statistics_reads import has_infeasible_on_line_rand_walk_num

reads_tag = 'reads-s'
reads_d_tag = 'reads-d'
reads_rq_tag = 'reads-rq'
suffix_str = '-rand-bench.txt'

eps_lst = [0.001 * (i + 1) for i in range(30)]


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def get_file_path(eps, algorithm, pair_num=10 ** 6):
    root_path = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/' \
                'git-repos/SimRank/python_experiments/exp_results/varying_eps_exp'
    data_set_name = 'ca-GrQc'
    return os.sep.join(map(str, [root_path, data_set_name, pair_num, eps, algorithm + suffix_str]))


def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


def get_cpu_time_lst(algorithm, pair_num=10 ** 6):
    eps_lst = [0.001 * (i + 1) for i in range(30)]

    def get_estimated_single_thread_time(eps):
        if algorithm in [reads_rq_tag]:
            if has_infeasible_on_line_rand_walk_num(get_file_path(eps, algorithm, pair_num)):
                print(eps)
        result = get_tag_info(get_file_path(eps, algorithm, pair_num), 'total query cpu time', min)
        cpu_time = result / 2.0 if result is not None else 99999999
        cpu_time *= (10 ** 6 / pair_num)
        return cpu_time

    return dict(zip(map(format_str, eps_lst), map(get_estimated_single_thread_time, eps_lst)))


def get_indexing_time_lst(algorithm, pair_num=10 ** 6):
    def get_index_time(eps):
        return get_tag_info(get_file_path(eps, algorithm, pair_num), 'indexing time', min)

    return dict(zip(map(format_str, eps_lst), map(get_index_time, eps_lst)))


def get_pair_num():
    return 10 ** 5


if __name__ == '__main__':
    algorithm_lst = [reads_d_tag, reads_rq_tag]

    data_folder = '../data-json/varying_parameters'
    os.system('mkdir -p ' + data_folder)


    def get_time_lst():
        time_lst = [get_cpu_time_lst(algorithm, get_pair_num()) for algorithm in algorithm_lst]

        with open(os.sep.join([data_folder, 'varying_eps_query_reads.json']), 'w') as ofs:
            time_dict = dict(zip(algorithm_lst, time_lst))
            for eps_str in map(format_str, [0.001 * i for i in range(1, 8)]):
                time_dict[reads_rq_tag][eps_str] = None
            ofs.write(json.dumps(time_dict, indent=4))


    def get_index_lst():
        name_lst = [reads_d_tag, reads_rq_tag]
        time_lst = [get_indexing_time_lst(algorithm, get_pair_num()) for algorithm in algorithm_lst]
        with open(os.sep.join([data_folder, 'varying_eps_index_reads.json']), 'w') as ofs:
            ofs.write(json.dumps(dict(zip(name_lst, time_lst)), indent=4))


    get_time_lst()
    get_index_lst()
