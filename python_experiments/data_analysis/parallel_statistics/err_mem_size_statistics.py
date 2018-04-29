import json
import os

mem_size_tag = 'mem size'
mean_err_tag = 'mean err'
max_err_tag = 'max err'

prlp_tag = 'prlp-with-lock'
prlp_lock_free_tag = 'prlp-lock-free'
pflp_tag = 'pflp-with-lock'
rlp_tag = 'rlp'
flp_tag = 'flp'


def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('KB')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


if __name__ == '__main__':
    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        'cit-Patents', 'soc-LiveJournal1'
    ]
    date_str = '04_24'


    def get_algorithm_statistics():
        our_algorithm_lst = [
            'prlp-lock-free',
            'prlp-with-lock',
            'rlp',
            'flp'
            # 'pflp-with-lock'
        ]

        folder_prefix_str = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank' \
                            '/python_experiments/plp_scalability_results_'
        stat_dict = {}
        for our_algorithm in our_algorithm_lst:
            print our_algorithm
            tmp_dict = {}
            for data_set_name in data_set_lst:
                def get_file_path(thread_num):
                    statistics_folder_path = os.sep.join(
                        [folder_prefix_str + date_str, our_algorithm, data_set_name])

                    return os.sep.join([statistics_folder_path, str(thread_num) + ".txt"])

                info_tag_lst = [mem_size_tag, max_err_tag, mean_err_tag]

                tmp_dict[data_set_name] = dict(zip(info_tag_lst, map(
                    lambda tag: get_tag_info(get_file_path(thread_num=1 if our_algorithm in [rlp_tag, flp_tag] else 56),
                                             tag, lambda lst: None if len(lst) == 0 else min(lst)), info_tag_lst)))
            stat_dict[our_algorithm] = tmp_dict

        root_dir_path = '../data-json/parallel_exp'
        os.system('mkdir -p ' + root_dir_path)
        with open(os.sep.join([root_dir_path, 'err_mem_size' + date_str + '.json']), 'w') as ofs:
            ofs.write(json.dumps(stat_dict, indent=4))


    get_algorithm_statistics()
