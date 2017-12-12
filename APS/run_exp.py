import subprocess
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib  as mpl
from multiprocessing import Pool
mpl.use("PDF")
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import os
datasets = [
    "ca-GrQc",
    "ca-HepTh",
    "p2p-Gnutella06",
    "wiki-Vote",
    "email-Enron",
    "email-EuAll",
    "web-NotreDame",
    "web-Stanford",
    "web-BerkStan",
    "web-Google",
    "cit-Patents",
    "soc-LiveJournal1",
        ]

# mat file base
MAT_DIR = "/homes/ywangby/workspace/inc_sr/datasets/"

# file to save the results
LOCAL_PUSH_DIR = "/homes/ywangby/workspace/dynamicSim/datasets/local_push/"
DYNAMIC_RESULT_DIR = "/homes/ywangby/workspace/dynamicSim/datasets/dynamic_exp/"
ICDE14_DYNAMIC_RESULT_DIR = "/homes/ywangby/workspace/dynamicSim/datasets/icde_dynamic/"
DATASETS_DIR="/homes/ywangby/workspace/LinsysSimRank/datasets/edge_list/"

def scale_free_name(n,d):
    file_name = "scale-free_%d_%d" % (n , d)
    print("saving to ", file_name)
    return file_name

def generate(item):
    n,d = item
    print(n,d)
    subprocess.run(["java", "-jar", \
            "../ROLL/target/ROLL-0.3-SNAPSHOT-jar-with-dependencies.jar",\
            "-n", str(n), "-m", str(d), '-o', DATASETS_DIR+scale_free_name(n,d)])
    return

def gen_scale_free_graph():
    # java -jar target/ROLL-0.3-SNAPSHOT-jar-with-dependencies.jar -n 1000 -o test.txt

    task_list = []
    for n in range(10 ** 6, 10**7+10 ** 6, 10 **6):
        d = 2
        task_list.append((n,d))


    for d in range(2,22,2):
        n = 10 ** 4
        task_list.append((n,d))
    with Pool() as p:
        p.map(generate, task_list)

def test_synthetic():
    c = 0.6
    epsilon = 0.01
    for n in range(10 ** 6, 10**7+10 ** 6, 10 **6):
        d = 2
        data_name = scale_free_name(n,d)
        test_static(data_name, c, epsilon)


    for d in range(2,22,2):
        n = 10 ** 4
        data_name = scale_free_name(n,d)
        test_static(data_name, c, epsilon)


def test_static(d,c,epsilon):
    subprocess.run(["./test", "static" ,d, str(c), str(epsilon)])
    return

def test_tkde17(d,c,epsilon):
    subprocess.run(["./test", "tkde17" ,d, str(c), str(epsilon)])
    return

def test_full(d,c,epsilon):
    '''
    test full local push
    '''
    subprocess.run(["./test", "full" ,d, str(c), str(epsilon)])
    return

def dynamic_result_file(d,n):
    return DYNAMIC_RESULT_DIR+d+"_"+ str(n)+".txt"

def test_dynamic(d,c, epsilo, n):
    file_name = dynamic_result_file(d,n)
    f = open(file_name, "w")
    subprocess.run(["./test", "dynamic" ,d, str(c), str(epsilon), str(n)], stdout=f)
    f.close()
    return

def get_icde_results_file(d, number_of_updates=100):
    return ICDE14_DYNAMIC_RESULT_DIR + d + "_" + str(number_of_updates) + ".txt"


def test_dynamic_icde14(d, number_of_updates=100):
    data_path = MAT_DIR + d + ".mat"
    file_name = get_icde_results_file(d, number_of_updates)
    f = open(file_name, "w")
    # change the working directory
    os.chdir("/homes/ywangby/workspace/inc_sr/")
    commands = ["matlab", "-nodesktop", "-nosplash", \
            "-logfile",  "`date +%Y_%m_%d-%H_%M_%S`.log"," -r",
            """ \"inc_sr_main('%s', %d)\" """ % (data_path, number_of_updates)]
    print(commands)
    subprocess.run(commands, stdout=f)
    f.close()

def test_efficiency():
    '''
    test efficiency on one single data, varying epsilon
    '''
    data = "ca-GrQc"
    c_range = np.arange(0.1,1,0.1)
    epsilons = np.arange(0.001, 0.101, 0.001)
    for epsilon in epsilons:
        test_static(data, 0.6, epsilon)
        test_full(data, 0.6, epsilon)
        test_tkde17(data, 0.6, epsilon)
    for c in c_range:
        test_static(data, c, 0.001)
        test_full(data, c, 0.001)
        test_tkde17(data, c, 0.001)

def test_cloudwalker(data_name):
    c = 0.6
    T = 10
    L = 3
    R = 100
    R_prime = 10000
    subprocess.run(["./test", "-a", "cloudwalker" , "-d", str(data_name), "-l", str(L), "-R" , str(R), "-r", str(R_prime), \
            "-c", str(c)])
def test_lin(data_name):
    c = 0.6
    T = 10
    L = 3
    R = 100
    subprocess.run(["./test", "-a", "lin" , "-d", str(data_name), "-l", str(L), "-R" , str(R), \
            "-c", str(c)])

def run_dynamic():
    for d in datasets:
        test_dynamic(d)

def run_icde():
    for d in datasets:
        test_dynamic_icde14(d)

def test():
    for d in datasets:
        c = 0.6
        epsilon = 0.01
        subprocess.run(["./test", d, str(c), str(epsilon)])


if __name__ == "__main__":
    # c = 0.6
    # epsilon = 0.01
    # for d in datasets[0:4]:
    #     test_static(d,c,epsilon)
    #     test_full(d,c,epsilon)

    # test_cloudwalker("claw")
    with Pool() as pool:
        pool.map(test_cloudwalker, datasets)
        pool.map(test_lin, datasets)
    # with Pool(2) as pool:
    #     pool.map(test_cloudwalker, datasets[5:10])
    # test_cloudwalker(datasets[-2])
    # test_cloudwalker(datasets[-1])



    # for d in datasets:
    #     test_cloudwalker(d)
    #     test_dynamic(d,c,epsilon,number_of_updates)
    # for d in datasets[0:5]:
    #     test_dynamic_icde14(d,number_of_updates)
    # d = "ca-HepTh"
    # c = 0.6
    # epsilon = 0.01
    # for number_of_updates in range(100,1100, 100):
    #     test_dynamic(d,c,epsilon,number_of_updates)
    # for number_of_updates in range(100,1100, 100):
    #     test_dynamic_icde14(d, number_of_updates)
    # with Pool() as pool:
    #     args = []
    #     for number_of_updates in range(5000,30000, 5000):
    #         args.append((d,c,epsilon, number_of_updates))
    #     pool.starmap(test_dynamic, args)
    #     args = []
    #     for number_of_updates in range(5000,30000, 5000):
    #         args.append((d,number_of_updates))
    #     pool.starmap(test_dynamic_icde14, args)


    # data = "ca-GrQc"
    # epsilon = 0.05
    # c_range = np.arange(0.1,1,0.1)
    # for c in c_range:
    #     test_static(data, c, epsilon)
    #     test_full(data, c, epsilon)
    #     test_tkde17(data, c, epsilon)
    # for num in range(100,1100,100):
    #     test_dynamic(d, num)
    #     # test_dynamic_icde14("ca-GrQc", num)
    # test_synthetic()
    # gen_scale_free_graph()
    # test_efficiency()
    # run_icde()
    # for d in datasets:
    #     # test_full(d,0.6,0.01)
    #     test_static(d, 0.6,0.01)
    #     # test_dynamic(d)

