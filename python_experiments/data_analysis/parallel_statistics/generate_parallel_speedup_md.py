from parallel_time_speedup import *

if __name__ == '__main__':
    with open('../data-json/parallel_exp/scalability.json') as ifs:
        print json.load(ifs)
