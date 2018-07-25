with open('/home/yche/mnt/wangyue-clu/csproject/biggraph/ywangby/LinsysSimRank/datasets/edge_list/p2p-Gnutella06.txt') as ifs:
    lines = ifs.readlines()
    lines = map(lambda line: map(int, line.rstrip().split()), lines)
    print len(lines)
    for line in lines:
        # print line
        if line[0] == line[1]:
            print line
