if __name__ == '__main__':
    with open('../data-markdown/parallel/speedup_04_28.md') as ifs:
        lines = map(lambda my_str: my_str.strip(), ifs.readlines()[115:])
        three_tuple_lst = [map(float, line.split('|')[-3:]) for line in lines]
        data_set_lst = [
            'ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06', 'wiki-Vote',
            'email-Enron', 'email-EuAll',
            'web-NotreDame', 'web-Stanford', 'web-BerkStan', 'web-Google', 'cit-Patents',
            'soc-LiveJournal1']

        # for three_tupe in three_tuple_lst:
        #     print three_tupe[0] / min(three_tupe)
        speedup_lst = map(lambda three_tuple: three_tuple[0] / min(three_tuple), three_tuple_lst)
        for k in zip(data_set_lst, speedup_lst):
            print k
