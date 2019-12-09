import matplotlib.pyplot as plt
import json

with open('./data_json/small_test/full.json') as ifs:
    data_lst = json.load(ifs)
small_dataset_lst = ['ca-GrQc', 'ca-HepTh', 'p2p-Gnutella06']
algo_lst = ['sling-topk', 'blpmc', 'carmo', 'carmo-h', 'carmo-t']

def draw_small():
    fig, ax = plt.subplots()
    for idx, tag in enumerate(algo_lst):
        ax.bar()

    fig.savefig('./figures/' + 'topk_cost' + '.pdf', bbox_inches='tight', dpi=300)

if __name__ == '__main__':
    os.system('mkdir -p figures')
    draw()
