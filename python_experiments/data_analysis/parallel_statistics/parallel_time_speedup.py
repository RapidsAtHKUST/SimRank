import os
import json

time_tag = 'computation time'
thread_lst_tag = 'thread num lst'
speedup_tag = 'speedup lst'


def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        return None
    with open(file_path) as ifs:
        # assume unit seconds
        lst = map(lambda line: eval(line.split(':')[-1].split('s')[0]),
                  filter(lambda line: line.startswith(tag), ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)


if __name__ == '__main__':
    data_set_lst = [
        'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
        'email-Enron', 'email-EuAll', 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google',
        'cit-Patents', 'soc-LiveJournal1'
    ]
    our_algorithm_lst = [
        'prlp-lock-free',
        'prlp-with-lock',
        'pflp-with-lock'
    ]
    thread_num_lst = [1, 2, 4, 8, 16, 32, 56]

    stat_dict = {}
    date_str = '04_24'
    for our_algorithm in our_algorithm_lst:
        print our_algorithm
        tmp_dict = {}
        for data_set_name in data_set_lst:
            def get_file_path(thread_num):
                statistics_folder_path = os.sep.join(
                    ['/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank'
                     '/python_experiments/plp_scalability_results_' + date_str, our_algorithm, data_set_name])

                return os.sep.join([statistics_folder_path, str(thread_num) + ".txt"])


            time_lst = map(lambda thread_num:
                           get_tag_info(get_file_path(thread_num), time_tag, min), thread_num_lst)
            if data_set_name == 'soc-LiveJournal1' and our_algorithm == 'pflp-with-lock':
                time_lst[0] = 7280.

            tmp_dict[data_set_name] = {
                thread_lst_tag: thread_num_lst,
                time_tag: time_lst,
                speedup_tag: map(lambda time_val: time_lst[0] / time_val, time_lst)
            }
        stat_dict[our_algorithm] = tmp_dict

    root_dir_path = '../data-json/parallel_exp'
    os.system('mkdir -p ' + root_dir_path)
    with open(os.sep.join([root_dir_path, 'scalability_' + date_str + '.json']), 'w') as ofs:
        ofs.write(json.dumps(stat_dict, indent=4))
