import os, json, decimal

root_path = '/homes/ywangby/workspace/yche/git-repos/SimRank/LPMC-fgi/experiments/exp_results/vary_k'

def get_file_path(q, k, eps, dataset_name, algo_name):
    return os.sep.join(map(str, [root_path, q, k, eps, dataset_name, algo_name + '.txt']))

def get_tag_info(file_path, tag, functor):
    if not os.path.exists(file_path):
        print 'not found', file_path
        return None
    with open(file_path) as ifs:
        lst = map(lambda line: eval(line[line.find(tag):].split(':')[1].split(',')[0].split('s')[0]),
                filter(lambda line: line.find(tag) != -1, ifs.readlines()))
        return 0 if len(lst) == 0 else functor(lst)

if __name__ == '__main__':
    data_folder = 'data_json'
    os.system('mkdir -p ' + data_folder)

    algo_lst = [
            'sling-topk',
            'blpmc', 'carmo', 'carmo-h', 'carmo-t',
            'tsf-topk']
    small_dataset_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06']
    large_dataset_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06',
                    'web-Stanford',
                    'web-BerkStan',
                    'web-Google',
                    'cit-Patents',
                    'soc-LiveJournal1',
                    'wiki-Link']
    tag_lst = ['indexing time', 'topk cost', "mem size"]
    q_lst = [10 ** 5]
    k_lst = [100, 200, 300, 400, 500]

    algo_data_lst = []
    for algo_name in algo_lst:
        dataset_data_lst = []
        for dataset_name in ['ca-GrQc']:
            k_data_lst = []
            for k in k_lst:
                tag_data_lst = []
                for tag in tag_lst:
                    if tag == 'ndcg' or tag == 'precison':
                        tag_data_lst.append(get_tag_info(get_file_path(10 ** 5, k, 0.01, dataset_name, algo_name), tag, max))
                    else:
                        tag_data_lst.append(get_tag_info(get_file_path(10 ** 5, k, 0.01, dataset_name, algo_name), tag, min))
                k_data_lst.append(dict(zip(tag_lst, tag_data_lst)))
            dataset_data_lst.append(dict(zip(k_lst, k_data_lst)))
        algo_data_lst.append(dict(zip(large_dataset_lst, dataset_data_lst)))


    with open(os.sep.join([data_folder, 'vary_k.json']), 'w') as ofs:
        ofs.write(json.dumps(dict(zip(algo_lst, algo_data_lst)), indent=4))


