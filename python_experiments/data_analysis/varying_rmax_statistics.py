import os, json, decimal

flpmc_tag = 'flpmc'
bflpmc_tag = 'bflpmc'

flp_tag = "full local push"
rmax_lst = [0.01 * (i + 1) for i in range(30)]


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def get_file_path(rmax, algorithm):
    root_path = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/' \
                'git-repos/SimRank/python_experiments/exp_results/varying_rmax_exp'
    data_set_name = 'ca-GrQc'
    suffix_str = '-rand-rmax.txt'
    return os.sep.join(map(str, [root_path, data_set_name, rmax, algorithm + suffix_str]))


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


def get_cpu_time_lst(algorithm):
    def get_estimated_single_thread_time(rmax):
        # print get_file_path(rmax, algorithm)
        result = get_tag_info(get_file_path(rmax, algorithm), 'total query cpu time', min)
        cpu_time = result / 2.0
        return cpu_time

    return dict(zip(map(format_str, rmax_lst), map(get_estimated_single_thread_time, rmax_lst)))


def get_indexing_time_lst(algorithm):
    def get_index_time(rmax):
        return get_tag_info(get_file_path(rmax, algorithm), 'total indexing cost', min)

    return dict(zip(map(format_str, rmax_lst), map(get_index_time, rmax_lst)))


if __name__ == '__main__':
    data_folder = 'data-json/varying_parameters'
    os.system('mkdir -p ' + data_folder)

    algorithm_lst = [bflpmc_tag, flpmc_tag]


    def get_time_lst():
        time_lst = [get_cpu_time_lst(algorithm) for algorithm in algorithm_lst]

        with open(os.sep.join([data_folder, 'varying_rmax_query.json']), 'w') as ofs:
            ofs.write(json.dumps(dict(zip(algorithm_lst, time_lst)), indent=4))


    def get_index_lst():
        name_lst = [flp_tag]
        time_lst = [get_indexing_time_lst(algorithm) for algorithm in [flpmc_tag]]
        with open(os.sep.join([data_folder, 'varying_rmax_index.json']), 'w') as ofs:
            ofs.write(json.dumps(dict(zip(name_lst, time_lst)), indent=4))


    get_time_lst()
    get_index_lst()
