import os

if __name__ == '__main__':
    root_dir = '/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/yche/new_dynamic_datasets/edge_list_raw/'


    def check_incremental_timestamp(dataset_name='flickr-growth'):
        with open(os.sep.join([root_dir, dataset_name, 'out.' + dataset_name])) as ifs:
            lines = ifs.readlines()[1:]
            print 'finish load'
            values = map(lambda line: int(line.rstrip().split()[-1]), lines)
            for i in xrange(len(values) - 1):
                if values[i] > values[i + 1]:
                    print 'err', values[i], values[i + 1]
            print 'finished'


    def check_epinion():
        with open(os.sep.join([root_dir, 'epinions', 'out.epinions'])) as ifs:
            lines = ifs.readlines()[1:]
            print 'finish load'
            values = filter(lambda tup: tup[2] == '1',
                            map(lambda line: line.rstrip().split(), lines))
            print len(values)
            for i in xrange(len(values) - 1):
                if int(values[i][3]) > int(values[i + 1][3]):
                    print 'err', values[i], values[i + 1]
            print 'finished'


    check_epinion()
