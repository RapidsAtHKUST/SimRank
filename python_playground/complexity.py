import math


def power_method_iter_num(c=0.6, digit_num=6):
    eps = 10 ** (-digit_num)
    t = math.log(eps * (1 - c), c)
    print 'power method for eps:', eps, ',c:', c, ', requiring t:', t


def reads_sample_tree_num(c=0.6, delata=0.01, digit_num=2, depth=10):
    eps = 10 ** (-digit_num)
    r = math.log(delata) * (c ** 2) * (depth - 1) / (-2 * (eps) ** 2)
    print 'requiring r:', r


if __name__ == '__main__':
    power_method_iter_num()
    reads_sample_tree_num()
