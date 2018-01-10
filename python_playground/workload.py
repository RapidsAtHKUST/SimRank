def compute_pair_num(v_num):
    return v_num ** 2 / 2 - v_num


if __name__ == '__main__':
    v_num_dict = {
        'ca-GrQc': 5242,
        'ca-HepTh': 9877,
        'p2p-Gnutella06': 8717,
        'wiki-Vote': 7115
    }

    for key, v_num in v_num_dict.iteritems():
        print key, compute_pair_num(v_num)
