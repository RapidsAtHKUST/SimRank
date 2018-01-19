from querying_time_accuracy_statistics import *
import decimal


def format_str(float_num):
    return str(decimal.Decimal.from_float(float_num).quantize(decimal.Decimal('0.000')))


def get_cpu_time_dict(root_dir='.'):
    with open(root_dir + os.sep + 'data-json' + os.sep + 'query_result_full_total_query_cpu_time' + '.json') as ifs:
        # with open(root_dir + os.sep + 'data-json' + os.sep + 'query_result_full_total_query_cost' + '.json') as ifs:
        return json.load(ifs)


cpu_time_dict = get_cpu_time_dict()


def lst_divide(l_lst, r_lst):
    assert len(l_lst) == len(r_lst)
    return map(lambda my_pair: float(my_pair[0]) / my_pair[1], zip(l_lst, r_lst))


def select_first_data_set(my_lst):
    return my_lst[0]


if __name__ == '__main__':
    bflpmc_lst = map(lambda data_set: select_first_data_set(cpu_time_dict[bflpmc_tag][data_set][str(10 ** 6)]),
                     data_set_lst)
    flpmc_lst = map(lambda data_set: select_first_data_set(cpu_time_dict[flpmc_tag][data_set][str(10 ** 6)]),
                    data_set_lst)
    bprw_lst = map(lambda data_set: select_first_data_set(cpu_time_dict[bprw_tag][data_set][str(10 ** 6)]),
                   data_set_lst)

    sling_lst = map(lambda data_set: select_first_data_set(cpu_time_dict[sling_tag][data_set][str(10 ** 6)]),
                    data_set_lst)
    print lst_divide(sling_lst, bflpmc_lst)
    print lst_divide(sling_lst, flpmc_lst)
    print lst_divide(sling_lst, bprw_lst)

    # time (us) for each pair, two is for hyper-threading
    print lst_divide(sling_lst, len(sling_lst) * [2])
    print lst_divide(bflpmc_lst, len(bflpmc_lst) * [2])


    def get_time_table(data_set_lst):
        table_lines = []
        header = ['algo\\data'] + data_set_lst
        table_lines.append(' | '.join(header))
        table_lines.append(' | '.join(['---'] * (len(data_set_lst) + 1)))
        lines = map(lambda algorithm:
                    ' | '.join(
                        [algorithm] + map(lambda num_lst: format_str(select_first_data_set(num_lst) / 2.0) + " us",
                                          map(lambda data_set: cpu_time_dict[algorithm][data_set][str(10 ** 6)],
                                              data_set_lst)))
                    , [bflpmc_tag, flpmc_tag, bprw_tag, sling_tag])
        table_lines.extend(lines)
        return '\n'.join(table_lines)


    def get_algorithm_time_lst(algorithm, data_lst):
        return map(lambda num_lst: select_first_data_set(num_lst),
                   map(lambda data_set: cpu_time_dict[algorithm][data_set][str(10 ** 6)], data_lst))


    def get_speedup_over_sling_table(data_set_lst):
        table_lines = []
        header = ['algo\\data'] + data_set_lst
        table_lines.append(' | '.join(header))
        table_lines.append(' | '.join(['---'] * (len(data_set_lst) + 1)))

        sling_time_lst = get_algorithm_time_lst(sling_tag, data_set_lst)
        algorithm_tag_lst = [bflpmc_tag, flpmc_tag, bprw_tag]
        speedup_lst = map(lambda algo_tag:
                          lst_divide(sling_time_lst, get_algorithm_time_lst(algo_tag, data_set_lst)),
                          algorithm_tag_lst)
        lines = map(lambda my_pair:
                    ' | '.join([my_pair[0]] + map(format_str, my_pair[1])),
                    zip(algorithm_tag_lst, speedup_lst))
        table_lines.extend(lines)
        return '\n'.join(table_lines)


    os.system('mkdir -p data-markdown')
    with open('data-markdown/speedup_over_sling.md', 'w') as ofs:
        ofs.writelines(['## Query Time (us)', '\n\n'])
        ofs.writelines([get_time_table(data_set_lst[0:6]), '\n\n'])
        ofs.writelines([get_time_table(data_set_lst[6:len(data_set_lst)]), '\n\n'])

        ofs.writelines(['## Speedup Over Sling', '\n\n'])
        ofs.writelines([get_speedup_over_sling_table(data_set_lst[0:6]), '\n\n'])
        ofs.writelines([get_speedup_over_sling_table(data_set_lst[6:len(data_set_lst)]), '\n\n'])