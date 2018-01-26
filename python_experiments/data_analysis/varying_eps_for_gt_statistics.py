import os, json, decimal

bprw_tag = 'bprw'
flpmc_tag = 'flpmc'
bflpmc_tag = 'bflpmc'
sling_tag = 'sling'
isp_tag = 'isp'
tsf_tag = 'tsf'
read_d_tag = 'reads-d'
reads_rq_tag = 'reads-rq'

eps_lst = [0.001 * (i + 1) for i in xrange(30)]


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def get_file_path(eps, algorithm, pair_num=10 ** 6):
    root_path = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/' \
                'git-repos/SimRank/python_experiments/exp_results/varying_eps_for_gt_exp'
    data_set_name = 'ca-GrQc'
    if algorithm not in [read_d_tag, reads_rq_tag]:
        suffix_str = '-rand-varying-gt.txt'
    else:
        suffix_str = '-rand-bench.txt'
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


def get_max_err_lst(algorithm, pair_num=10 ** 6):
    def get_estimated_single_thread_time(eps):
        result = get_tag_info(get_file_path(eps, algorithm, pair_num), 'max err', min)
        return result

    return dict(zip(map(format_str, eps_lst), map(get_estimated_single_thread_time, eps_lst)))


def get_pair_num(algorithm):
    if algorithm in [tsf_tag]:
        return 10 ** 4
    else:
        return 10 ** 5


if __name__ == '__main__':
    algorithm_lst = [bprw_tag, bflpmc_tag, flpmc_tag, sling_tag,
                     tsf_tag, isp_tag, read_d_tag, reads_rq_tag]

    data_folder = 'data-json/varying_parameters'
    os.system('mkdir -p ' + data_folder)


    def get_max_err():
        max_err_lst = [get_max_err_lst(algorithm, get_pair_num(algorithm)) for algorithm
                       in algorithm_lst]
        for eps_str in map(format_str, [0.001 * i for i in xrange(1, 8)]):
            max_err_lst[-1][eps_str] = None
        with open(os.sep.join([data_folder, 'varying_eps_max_err.json']), 'w') as ofs:
            ofs.write(json.dumps(dict(zip(algorithm_lst, max_err_lst)), indent=4))


    get_max_err()
