import math


def power_method_iter_num(c=0.6, digit_num=6):
    eps = 10 ** (-digit_num)
    t = math.log(eps * (1 - c), c)
    print '(2002original) power method for eps:', eps, ',c:', c, ', requiring t:', t


def reads_naive_sample_tree_num(c=0.6, delata=0.01, digit_num=2, depth=10):
    eps = 10 ** (-digit_num)
    r = math.log(delata) * (c ** 2) * (depth - 1) / (-2 * (eps) ** 2)
    print '(2017reads) requiring r:', r


def isp_parameters(c=0.6, convergence_delta=0.01):
    print '(2010isp) steps:', math.log(convergence_delta, c)


def tsf_parameters(c=0.6, eps=0.01, delta=0.01):
    b = min(1, c / (1 - c))
    print '(2015tsp) sample#:', math.log(delta / 2) / (-2) / (eps ** 2) * ((b - c) ** 2)


if __name__ == '__main__':
    power_method_iter_num()

    isp_parameters()
    tsf_parameters()

    reads_naive_sample_tree_num()
