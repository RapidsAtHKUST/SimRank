import matplotlib.pyplot as plt

from data_analysis.parallel_statistics.generate_parallel_speedup_md import *


def get_name_dict():
    with open('data_names.json') as ifs:
        return eval(''.join(ifs.readlines()))


def draw_figures():
    data_set_lst = ["web-Stanford", "web-Google", "cit-Patents", "soc-LiveJournal1"]
    with open('../data_analysis/data-json/parallel_exp/scalability_04_24.json') as ifs:
        para_algo_dict = json.load(ifs)
    with open('../data_analysis/data-json/parallel_exp/seq_time_04_24.json') as ifs:
        seq_algo_dict = json.load(ifs)
    with open('../data_analysis/data-json/parallel_exp/scalability_gen_time_04_24.json') as ifs:
        gen_time_dict = json.load(ifs)

    thread_lst = None
    for algorithm in [prlp_tag, prlp_lock_free_tag]:
        print(algorithm)
        for data_set in data_set_lst:
            if thread_lst is None:
                thread_lst = para_algo_dict[algorithm][data_set][thread_lst_tag]
            print(para_algo_dict[algorithm][data_set][time_tag], gen_time_dict[algorithm][data_set][task_gen_tag])

    for algorithm in [rlp_tag]:
        print(algorithm)
        for data_set in data_set_lst:
            print([seq_algo_dict[algorithm][data_set]] * len(thread_lst))

    # some parameters
    thread_lst[-1] = 64
    thread_idx_lst = range(1, len(thread_lst) + 1)
    TICK_SIZE = 22
    LABEL_SIZE = 22
    LEGEND_SIZE = 22

    def get_algorithm_time_lst(algorithm, data_set):
        if algorithm in [prlp_tag, prlp_lock_free_tag]:
            return para_algo_dict[algorithm][data_set][time_tag]
        else:
            return [seq_algo_dict[algorithm][data_set]] * len(thread_lst)

    def get_gen_time_lst(algorithm, data_set):
        return gen_time_dict[algorithm][data_set][task_gen_tag]

    color_lst = ['#fe01b1', '#ceb301', 'red']
    shape_lst = ['D-.', 's--', 'o:', 'x-',
                 'P-', '*-',
                 'v-', '^-', '<-', '>-']
    # sub title on the top
    sub_titles = ['(a) dataset = ', '(b) dataset = ', '(c) dataset = ', '(d) dataset = ']
    data_names = get_name_dict()
    for idx in range(len(sub_titles)):
        sub_titles[idx] += data_names[data_set_lst[idx]]

    def draw_overall_time():
        exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 5))
        algorithm_tag_lst = [prlp_tag, prlp_lock_free_tag, rlp_tag]
        legend_lst = ['PLB-Opt-LP', 'PLF-Opt-LP', 'Opt-LP']

        for ax_idx, ax in enumerate(ax_tuple):
            time_lst_lst = []
            for idx, algorithm in enumerate(algorithm_tag_lst):
                time_lst = get_algorithm_time_lst(algorithm, data_set_lst[ax_idx])
                # print time_lst, thread_lst
                time_lst_lst.append(time_lst)
                ax.plot(thread_idx_lst, time_lst, shape_lst[idx], markersize=20, markerfacecolor='none',
                        color=color_lst[idx])
                ax.set_yscale('log')
                ax.set_xlabel('Number of Threads\n' + sub_titles[ax_idx], fontsize=LABEL_SIZE + 6)
                y_lim_lst = [(10, 10 ** 3), (1, 10 ** 2 * 2), (10, 10 ** 3), (10 ** 2, 10 ** 4 * 3)]

                ax.set_xticklabels([0] + thread_lst, fontsize=TICK_SIZE)
                ax.set_ylim(y_lim_lst[ax_idx])

            plt.yticks(fontsize=TICK_SIZE)

        for idx, my_ax in enumerate(ax_tuple):
            if idx == 0:
                plt.yticks(fontsize=LABEL_SIZE)
                my_ax.set_ylabel('Elapsed Time(s)', fontsize=LABEL_SIZE + 6)
            for tick in my_ax.yaxis.get_major_ticks():
                tick.label.set_fontsize(TICK_SIZE)

        exp_figure.subplots_adjust(wspace=0)
        plt.tight_layout()
        plt.legend(legend_lst, ncol=len(legend_lst), fontsize=LEGEND_SIZE + 3,
                   prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=2,
                   bbox_to_anchor=(-3.0, 1.3, 3.5, 0.0), mode='expand')
        plt.savefig('figures/' + 'tkde_varying_num_threads_overall' + '.pdf', bbox_inches='tight', dpi=300)
        plt.close()

    def draw_gen_time():
        exp_figure, ax_tuple = plt.subplots(1, 4, sharex=True, figsize=(32, 5))
        algorithm_tag_lst = [prlp_tag, prlp_lock_free_tag]
        legend_lst = ['PLB-Opt-LP Task Table Generation', 'PLF-Opt-LP Task Table Generation']

        for ax_idx, ax in enumerate(ax_tuple):
            time_lst_lst = []
            for idx, algorithm in enumerate(algorithm_tag_lst):
                time_lst = get_gen_time_lst(algorithm, data_set_lst[ax_idx])
                # print time_lst, thread_lst
                time_lst_lst.append(time_lst)
                ax.plot(thread_idx_lst, time_lst, shape_lst[idx], markersize=20, markerfacecolor='none',
                        color=color_lst[idx])
                ax.set_yscale('log')
                ax.set_xlabel('Number of Threads\n' + sub_titles[ax_idx], fontsize=LABEL_SIZE + 6)
                y_lim_lst = [(10 / 2, 10 ** 2 * 2), (1, 10 ** 2), (10 / 2, 10 ** 2 * 4), (10 ** 1 * 2, 10 ** 3 * 2)]

                ax.set_xticklabels([0] + thread_lst, fontsize=TICK_SIZE)
                ax.set_ylim(y_lim_lst[ax_idx])

            plt.yticks(fontsize=TICK_SIZE)

        for idx, my_ax in enumerate(ax_tuple):
            if idx == 0:
                plt.yticks(fontsize=LABEL_SIZE)
                my_ax.set_ylabel('Elapsed Time(s)', fontsize=LABEL_SIZE + 6)
            for tick in my_ax.yaxis.get_major_ticks():
                tick.label.set_fontsize(TICK_SIZE)

        exp_figure.subplots_adjust(wspace=0)
        plt.tight_layout()
        plt.legend(legend_lst, ncol=len(legend_lst), fontsize=LEGEND_SIZE + 3,
                   prop={'size': LEGEND_SIZE, "weight": "bold"}, loc=2,
                   bbox_to_anchor=(-3.0, 1.3, 3.5, 0.0), mode='expand')
        plt.savefig('figures/' + 'tkde_varying_num_threads_gen' + '.pdf', bbox_inches='tight', dpi=300)
        plt.close()

    draw_overall_time()
    draw_gen_time()


if __name__ == '__main__':
    draw_figures()
