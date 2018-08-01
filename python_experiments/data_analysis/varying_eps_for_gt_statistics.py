import os, json, decimal

from data_analysis.probesim_querying_time_statistics import probesim_gt_tag

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


def get_file_path(eps, algorithm, pair_num=10 ** 6, data_set_name='ca-GrQc'):
    root_path = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/' \
                'git-repos/SimRank/python_experiments/exp_results/varying_eps_for_gt_exp' \
        if algorithm is not probesim_gt_tag \
        else '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new-git-repos-yche/SimRank/' \
             'python_experiments/exp_results/varying_eps_for_gt_exp'
    if algorithm is probesim_gt_tag:
        suffix_str = '.txt'
    elif algorithm not in [read_d_tag, reads_rq_tag]:
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


def get_max_err_lst(algorithm, data_set_name, pair_num=10 ** 6):
    def get_estimated_single_thread_time(eps):
        if algorithm in {bflpmc_tag, flpmc_tag, bprw_tag}:
            avg = lambda lst: sum(lst) / len(lst)
            result = get_tag_info(get_file_path(eps, algorithm, pair_num, data_set_name), 'max err', min)
        elif algorithm == tsf_tag:
            result = get_tag_info(get_file_path(eps, algorithm, pair_num, data_set_name), 'max err', max)
        else:
            result = get_tag_info(get_file_path(eps, algorithm, pair_num, data_set_name), 'max err', lambda x: x[-1])
        return result

    return dict(zip(map(format_str, eps_lst), map(get_estimated_single_thread_time, eps_lst)))


def get_pair_num(algorithm):
    if algorithm in [tsf_tag, probesim_gt_tag]:
        return 10 ** 4
    else:
        return 10 ** 5


if __name__ == '__main__':
    def get_eps_for_non_probesim_algorithms():
        algorithm_lst = [bprw_tag, bflpmc_tag, flpmc_tag, sling_tag,
                         tsf_tag, isp_tag, read_d_tag, reads_rq_tag]

        data_folder = 'data-json/varying_parameters'
        os.system('mkdir -p ' + data_folder)

        def get_max_err(data_set_name):
            max_err_lst = [get_max_err_lst(algorithm, data_set_name=data_set_name, pair_num=get_pair_num(algorithm)) for
                           algorithm in algorithm_lst]
            for eps_str in map(format_str, [0.001 * i for i in xrange(1, 8)]):
                max_err_lst[-1][eps_str] = None
            return dict(zip(algorithm_lst, max_err_lst))

        data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
        eps_gt_dict = {}
        for data_set in data_set_lst:
            eps_gt_dict[data_set] = get_max_err(data_set)

        with open(os.sep.join([data_folder, 'varying_eps_max_err.json']), 'w') as ofs:
            ofs.write(json.dumps(eps_gt_dict, indent=4))


    def get_eps_for_probesim():
        algorithm_lst = [probesim_gt_tag]

        data_folder = 'data-json/varying_parameters'
        os.system('mkdir -p ' + data_folder)

        def get_max_err(data_set_name):
            max_err_lst = [get_max_err_lst(algorithm, data_set_name=data_set_name, pair_num=get_pair_num(algorithm)) for
                           algorithm in algorithm_lst]
            # for eps_str in map(format_str, [0.001 * i for i in xrange(1, 8)]):
            #     max_err_lst[-1][eps_str] = None
            return dict(zip(algorithm_lst, max_err_lst))

        data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
        eps_gt_dict = {}
        for data_set in data_set_lst:
            eps_gt_dict[data_set] = get_max_err(data_set)
        print eps_gt_dict
        with open(os.sep.join([data_folder, 'varying_eps_max_err_probesim.json']), 'w') as ofs:
            ofs.write(json.dumps(eps_gt_dict, indent=4))


    get_eps_for_probesim()
