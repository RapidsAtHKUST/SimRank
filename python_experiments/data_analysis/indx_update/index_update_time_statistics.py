data_set_lst = [
    "ca-GrQc",
    "ca-HepTh",
    "p2p-Gnutella06",
    "wiki-Vote",
    "email-Enron",
    "email-EuAll",
    "web-NotreDame",
    "web-Stanford",
    "web-BerkStan",
    "web-Google",
    "cit-Patents",
    "soc-LiveJournal1",
    "wiki-Link"
]

if __name__ == '__main__':
    root_folder = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/git-repos/SimRank/python_experiments/exp_results/'


    def get_data_time_lst(file_name):
        with open(root_folder + file_name) as ifs:
            lines = map(lambda line: line.strip(), ifs.readlines())
            data_set_lst = [line.split('/')[-1].rstrip('.txt') for line in
                            filter(lambda line: 'loading' in line, lines)]
            time_lst = [eval(line.split(':')[-1]) for line in
                        filter(lambda line: 'avg' in line and 'time' in line, lines)]
            return zip(data_set_lst, time_lst)


    def print_time_lst(file_name):
        time_dict = {}
        for data, time in get_data_time_lst(file_name):
            if data not in time_dict:
                time_dict[data] = []
            time_dict[data].append(time)
        print [time_dict[data_set][0] if len(time_dict[data_set]) < 2 else time_dict[data_set][1] for data_set in
               data_set_lst]


    print_time_lst('tsf_dynamic_update_time_1000_ins_0407.txt')
    print_time_lst('tsf_dynamic_update_time_1000_del_0407.txt')

    print '\n reads-rq:'
    print_time_lst('reads-rq-dynamic-exp_dynamic_update_time_1000_0407.txt')
    print_time_lst('reads-rq-dynamic-del_dynamic_update_time_1000_0407.txt')
