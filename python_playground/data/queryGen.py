# query generator
# method: 0: 1,2,3,4,5 , 10: 11, 12,13,14,15...

def queryGen(data_name=None):
    if data_name is None:
        query_pairs = []
        for i in range(0, 200):
            for j in range(i + 1, i + 6):
                query_pairs.append((i, j))
        return query_pairs
    else:
        # load local index and choose with non zero
        pass


def test():
    Qs = queryGen()
    print(Qs)
    print(len(Qs))


if __name__ == '__main__':
    test()
