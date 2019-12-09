import os, json, decimal

root_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/experiments/exp_results/180805'

def get_file_path(q, k, dataset_name, algo_name):
    return os.sep.join(map(str, [root_path, q, k, dataset_name, algo_name + '.txt']))

def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        print 'not found', file_path
        return None
    with open(file_path) as ifs:
        lst = map(lambda line: eval(line[line.find(tag):].split(':')[1].split(',')[0].split('s')[0]),
                filter(lambda line: line.find(tag) != -1, ifs.readlines()))
        return None if len(lst) == 0 else functor(lst)

if __name__ == '__main__':
    data_folder = 'data_json/180805'
    os.system('mkdir -p ' + data_folder)

    algo_lst = ['sling-topk', 'blpmc', 'carmo', 'carmo-h', 'carmo-t', 'tsf-topk']
    small_dataset_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06']
    tag_lst = ['indexing time', 'topk cost', 'max err', 'precison', 'ndcg']
    q_lst = [10 ** 4, 10 ** 5, 10 ** 6]
    k_lst = [100, 200, 300, 400, 500, 800, 1600]

    algo_data_lst = []
    for algo_name in algo_lst:
        dataset_data_lst = []
        for dataset_name in small_dataset_lst:
            q_data_lst = []
            for q in q_lst:
                k_data_lst = []
                for k in k_lst:
                    tag_data_lst = []
                    for tag in tag_lst:
                        # round_data_lst = []
                        # for round_idx in xrange(2):
                        #     round_data_lst.append(get_tag_info(get_file_path(q, k, dataset_name, algo_name, round_idx), tag, min))
                        # tag_data_lst.append(dict(zip(xrange(2), round_data_lst)))
                        if tag == 'ndcg' or tag == 'precison':
                            tag_data_lst.append(get_tag_info(get_file_path(q, k, dataset_name, algo_name), tag, max))
                        else:
                            tag_data_lst.append(get_tag_info(get_file_path(q, k, dataset_name, algo_name), tag, min))
                    k_data_lst.append(dict(zip(tag_lst, tag_data_lst)))
                q_data_lst.append(dict(zip(k_lst, k_data_lst)))
            dataset_data_lst.append(dict(zip(q_lst, q_data_lst)))
        algo_data_lst.append(dict(zip(small_dataset_lst, dataset_data_lst)))


    with open(os.sep.join([data_folder, '180805.json']), 'w') as ofs:
        ofs.write(json.dumps(dict(zip(algo_lst, algo_data_lst)), indent=4))


