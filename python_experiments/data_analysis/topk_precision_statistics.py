import json
import os

from data_analysis.probesim_querying_time_statistics import probesim_gt_tag


def get_tag_info(file_path, tag, functor=lambda x: x[0]):
    # print file_path
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        lst = map(lambda line: line.split(':')[-1],
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


def get_precision_val(input_str):
    return float(eval(input_str.split('/')[0])) / eval(input_str.split('/')[1]) if input_str is not None else None


def get_precision_lst_per_algorithm(algorithm_name, data_set_name,
                                    eps_lst=list(reversed([0.0001, 0.0004, 0.0016, 0.0064, 0.0256])), k=str(800)):
    tag = 'exp_results'
    folder_name = 'varying_eps_for_topk_precision_exp'
    sample_num = str(10 ** 4)

    precision_lst = []
    avg_diff_lst = []
    for eps in eps_lst:
        statistics_dir = os.sep.join(map(str, [
            '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank/'
            'python_experiments' if algorithm_name is not probesim_gt_tag \
                else '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new-git-repos-yche/SimRank/' \
                     'python_experiments', tag,
            folder_name, sample_num, k, data_set_name, eps]))
        statistics_file_path = statistics_dir + os.sep + algorithm_name + '.txt'
        precision_lst.append(get_precision_val(get_tag_info(statistics_file_path, 'precision #')))
        info = get_tag_info(statistics_file_path, 'avg difference')
        avg_diff_lst.append(None if info is None else eval(info))
    return dict(zip(eps_lst, precision_lst)), dict(zip(eps_lst, avg_diff_lst))


if __name__ == '__main__':
    def get_result_non_probesim():
        exec_path_lst = [
            '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bprw-rand-bench-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/flpmc-rand-bench-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-Profile/build/bflpmc-rand-bench-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/sling/build/sling-rand-bench-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/isp-yche/build/isp-rand-bench-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/tsf/build/tsf-rand-bench-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-d-rand-bench',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/READS/build/reads-rq-rand-bench',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/lind-rand-ben-gt',
            '/homes/ywangby/workspace/yche/git-repos/SimRank/SPS-Variants/LinearD/build/cw-rand-gen-gt',
        ]
        algo_name_lst = map(lambda algorithm_path: algorithm_path.split('/')[-1], exec_path_lst)

        data_set_name_lst = [
            'ca-GrQc',
            'ca-HepTh',
            'p2p-Gnutella06',
            'wiki-Vote'
        ]

        precision_figure_dict = {}
        avg_diff_dict = {}
        for data_set in data_set_name_lst:
            my_dict = {}
            my_dict2 = {}
            for name in algo_name_lst:
                key_name = name.split('-')[0] if 'reads' not in name else '-'.join(name.split('-')[0:2])
                my_dict[key_name], my_dict2[key_name] = get_precision_lst_per_algorithm(name, data_set)
                if 'reads-rq' in key_name:
                    for eps in [0.0001, 0.0004, 0.0016]:
                        my_dict[key_name][eps] = None
                        my_dict2[key_name][eps] = None

            precision_figure_dict[data_set] = my_dict
            avg_diff_dict[data_set] = my_dict2

        dir_path = 'data-json/topk_precision'
        os.system('mkdir -p ' + dir_path)
        k = str(800)
        with open(dir_path + os.sep + '_'.join(['precision', 'top', k, 'sample', '10000']) + '.json', 'w') as ofs:
            ofs.write(json.dumps(precision_figure_dict, indent=4))
        with open(dir_path + os.sep + '_'.join(['avg_diff', 'top', k, 'sample', '10000']) + '.json', 'w') as ofs:
            ofs.write(json.dumps(avg_diff_dict, indent=4))


    def get_result_probesim():
        algo_name_lst = [probesim_gt_tag]

        data_set_name_lst = [
            'ca-GrQc',
            'ca-HepTh',
            'p2p-Gnutella06',
            'wiki-Vote'
        ]

        precision_figure_dict = {}
        avg_diff_dict = {}
        for data_set in data_set_name_lst:
            my_dict = {}
            my_dict2 = {}
            for name in algo_name_lst:
                key_name = name
                my_dict[key_name], my_dict2[key_name] = get_precision_lst_per_algorithm(name, data_set)
            precision_figure_dict[data_set] = my_dict
            avg_diff_dict[data_set] = my_dict2

        dir_path = 'data-json/topk_precision'
        os.system('mkdir -p ' + dir_path)
        k = str(800)
        with open(dir_path + os.sep + '_'.join(['precision', 'probesim', 'top', k, 'sample', '10000']) + '.json',
                  'w') as ofs:
            ofs.write(json.dumps(precision_figure_dict, indent=4))
        with open(dir_path + os.sep + '_'.join(['avg_diff', 'probesim', 'top', k, 'sample', '10000']) + '.json',
                  'w') as ofs:
            ofs.write(json.dumps(avg_diff_dict, indent=4))
    get_result_probesim()