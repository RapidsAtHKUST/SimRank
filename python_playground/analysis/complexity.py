import math


def power_method_iter_num(c=0.6, digit_num=22):
    eps = 10 ** (-digit_num)
    t = math.log(eps * (1 - c), c)
    print '(2002original) power method for eps:', eps, ',c:', c, ', requiring t:', t


def isp_parameters(c=0.6, convergence_delta=0.01):
    print '(2010isp) steps:', math.log(convergence_delta, c), 'threshold filter(empirical):', 0.0001


def tsf_parameters(c=0.6, eps=0.01, delta=0.01):
    b = min(1, c / (1 - c))
    print '(2015tsf) sample# (R*Rq):', math.log(delta / 2) / (-2) / (eps ** 2) * ((b - c) ** 2)


# sling delta_d = delta / n (according to the sling paper page1865),
# delta is the failure probability of sim score estimation
def sling_eps_bound(c=0.6, eps_d=0.005, theta=0.000725):
    left_part = eps_d / (1 - c)
    right_part = (2 * math.sqrt(c)) / (1 - math.sqrt(c)) / (1 - c) * theta
    print '(2016sling) eps:', left_part + right_part, left_part, right_part


def sling_parameters(c=0.6, eps=0.01):
    half_eps = eps / 2
    eps_d = (1 - c) * half_eps
    theta = half_eps / ((2 * math.sqrt(c)) / (1 - math.sqrt(c)) / (1 - c))
    print '(2016sling) eps_d:', eps_d, 'threshold theta:', theta


def reads_naive_sample_tree_num(c=0.6, delata=0.01, digit_num=2, depth=10):
    eps = 10 ** (-digit_num)
    r = math.log(delata) * (c ** 2) * (depth - 1) / (-2 * (eps) ** 2)
    print '(2017reads) requiring r:', r


def reads_sample_num(c=0.6, eps=0.01, delta=0.01):
    print '(2017reads) requiring sample#:', math.log(delta / 2) * (c ** 6) / (-2 * eps * eps)


if __name__ == '__main__':
    power_method_iter_num()
    # sling_eps_bound()
    # reads_naive_sample_tree_num()

    isp_parameters()
    tsf_parameters()
    sling_parameters()
    reads_sample_num()
