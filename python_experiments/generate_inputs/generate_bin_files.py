import os

if __name__ == '__main__':
    small_data_set_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote']
    data_set_lst = [
        # 'email-Enron', 'email-EuAll',
        # 'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google', 'cit-Patents',
        # 'soc-LiveJournal1',
        # 'wiki-Link',
        'digg-friends',
        'flickr-growth',
    ]
    generate_exec_path = '/homes/ywangby/workspace/yche/new-git-repos-yche/SimRank/LPMC-Profile/build/util/bin_converter'

    for data_set in small_data_set_lst + data_set_lst:
        cmd = ' '.join(map(str, [generate_exec_path, data_set]))
        print cmd
        os.system(cmd)
