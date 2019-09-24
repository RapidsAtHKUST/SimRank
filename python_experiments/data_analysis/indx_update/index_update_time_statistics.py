import decimal

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


def format_str(float_num):
    my_str = str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000000')))
    return float(my_str)


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
        return [0 if data_set not in time_dict else
                time_dict[data_set][0] if len(time_dict[data_set]) < 2 else time_dict[data_set][1] for data_set in
                data_set_lst]

    # print print_time_lst('tsf_dynamic_update_time_1000_ins_0407.txt')
    # print print_time_lst('tsf_dynamic_update_time_1000_del_0407.txt')
    #
    # print '\n reads-rq:'
    # print print_time_lst('reads-rq-dynamic-exp_dynamic_update_time_1000_0407.txt')
    # print print_time_lst('reads-rq-dynamic-del_dynamic_update_time_1000_0407.txt')
    #
    # print '\n reads-d:'
    # print print_time_lst('reads-d-dynamic-exp_dynamic_update_time_1000_0407.txt')
    # print print_time_lst('reads-d-dynamic-del_dynamic_update_time_1000_0407.txt')
    #
    # print '\n lp:'
    # print map(lambda val: format_str(val), [0.00130415, 0.00241406, 0.00237629, 0.00163426, 0.0139743, 0.0493718,
    #                                         0.124753,
    #                                         0.102021, 0.271308, 0.268973, 1.25391, 2.47118, 9.09415])
    #
    # print map(lambda val: format_str(val / 1.5), print_time_lst('rlp_dynamic_update_time_1000_0407.txt'))
