import os, json, decimal

from data_analysis.probesim_querying_time_statistics import probesim_tag

bprw_tag = 'bprw'
flpmc_tag = 'flpmc'
bflpmc_tag = 'bflpmc'
sling_tag = 'sling'
isp_tag = 'isp'
tsf_tag = 'tsf'

flp_tag = "full local push"
eps_lst = [0.001 * (i + 1) for i in range(30)]


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def get_file_path(eps, algorithm, pair_num=10 ** 6):
    root_path = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/' \
                'git-repos/SimRank/python_experiments/exp_results/varying_eps_exp' if algorithm is not probesim_tag \
        else '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new-git-repos-yche/SimRank/' \
             'python_experiments/exp_results/varying_eps_exp'
    data_set_name = 'ca-GrQc'
    suffix_str = '-rand-varying.txt' if algorithm is not probesim_tag else '.txt'
    return os.sep.join(map(str, [root_path, data_set_name, pair_num, eps, algorithm + suffix_str]))


def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        # print 'not found', file_path
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


def get_tag_no_colon_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(tag)[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


def get_cpu_time_lst(algorithm, pair_num=10 ** 6):
    eps_lst = [0.001 * (i + 1) for i in xrange(100)]

    def get_estimated_single_thread_time(eps):
        result = get_tag_info(get_file_path(eps, algorithm, pair_num), 'total query cpu time', min)
        cpu_time = result / 2.0 if result is not None else 99999999
        cpu_time *= (10 ** 6 / pair_num)
        return cpu_time

    return dict(zip(map(format_str, eps_lst), map(get_estimated_single_thread_time, eps_lst)))


def get_indexing_time_lst(algorithm, pair_num=10 ** 6):
    def get_index_time(eps):
        if algorithm in [bflpmc_tag, flpmc_tag]:
            return get_tag_info(get_file_path(eps, algorithm, pair_num), 'total indexing cost', min)
        elif algorithm in [sling_tag]:
            cal_d_parallel_time = get_tag_no_colon_info(get_file_path(eps, algorithm, pair_num), 'finish calcD', min)
            cal_h_time = get_tag_no_colon_info(get_file_path(eps, algorithm, pair_num), 'finish backward', min)
            return cal_d_parallel_time * 6 + cal_h_time
        elif algorithm in [tsf_tag]:
            return get_tag_info(get_file_path(eps, algorithm, pair_num), 'indexing computation time', min)

    return dict(zip(map(format_str, eps_lst), map(get_index_time, eps_lst)))


def get_pair_num(algorithm):
    if algorithm in [tsf_tag]:
        return 10 ** 3
    elif algorithm in [probesim_tag]:
        return 10 ** 3
    else:
        return 10 ** 5 if algorithm in [isp_tag] else 10 ** 6


if __name__ == '__main__':
    data_folder = 'data-json/varying_parameters'
    os.system('mkdir -p ' + data_folder)


    def stat_without_probesim():
        algorithm_lst = [bprw_tag, bflpmc_tag, flpmc_tag, sling_tag,
                         isp_tag, tsf_tag]

        def get_time_lst():
            time_lst = [get_cpu_time_lst(algorithm, get_pair_num(algorithm)) for algorithm
                        in algorithm_lst]

            with open(os.sep.join([data_folder, 'varying_eps_query.json']), 'w') as ofs:
                ofs.write(json.dumps(dict(zip(algorithm_lst, time_lst)), indent=4))

        def get_index_lst():
            name_lst = [flp_tag, sling_tag, tsf_tag]
            time_lst = [get_indexing_time_lst(algorithm, get_pair_num(algorithm)) for
                        algorithm in [flpmc_tag, sling_tag, tsf_tag]]
            with open(os.sep.join([data_folder, 'varying_eps_index.json']), 'w') as ofs:
                ofs.write(json.dumps(dict(zip(name_lst, time_lst)), indent=4))

        get_time_lst()
        get_index_lst()


    def stat_with_probesim():
        algorithm_lst = [probesim_tag]

        def get_time_lst():
            time_lst = [get_cpu_time_lst(algorithm, get_pair_num(algorithm)) for algorithm
                        in algorithm_lst]

            with open(os.sep.join([data_folder, 'probesim_varying_eps_query.json']), 'w') as ofs:
                ofs.write(json.dumps(dict(zip(algorithm_lst, time_lst)), indent=4))

        get_time_lst()


    stat_with_probesim()
