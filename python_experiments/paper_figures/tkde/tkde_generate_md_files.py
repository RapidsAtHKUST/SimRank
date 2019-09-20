from paper_figures.tkde.tkde_draw_varying_datasets_time_mem import *
from paper_figures.tkde.tkde_draw_dynamic_time_mem import *


def format_str_float(w):
    if isinstance(w, str):
        return w
    elif w == 0:
        return str(0)
    elif w > 0.001:
        return str('%.3lf' % w)
    else:
        return str('{0: .5e}'.format(w))


def gen_line(lst):
    return ' | '.join([format_str_float(w) for w in lst])


if __name__ == '__main__':
    logger = get_logger('/home/yche/logs/tmp.log', __name__)
    md_folder = 'data-md'
    os.system('mkdir -p {}'.format(md_folder))

    md_cols = len(data_set_lst) + 1
    abbr_lst = [data_names[data] for data in data_set_lst]
    splitter_lst = ['---' for _ in range(md_cols)]
    md_header = ['Algorithm'] + abbr_lst


    def md_static():
        line_lst = [md_header, splitter_lst]
        line_lst2 = [md_header, splitter_lst]
        algorithm_abbr = dict(zip(algorithm_tag_lst, static_label_lst))
        for idx, tag in enumerate(algorithm_tag_lst):
            line_lst.append([algorithm_abbr[tag]] + get_algorithm_elapsed_time_lst(tag))
            line_lst2.append([algorithm_abbr[tag]] + get_algorithm_mem_usage_lst(tag))

        static_cpu_compare_md = '\n'.join([gen_line(l) for l in line_lst])
        logger.info(static_cpu_compare_md)
        static_mem_compare_md = '\n'.join([gen_line(l) for l in line_lst2])
        logger.info(static_mem_compare_md)

        with open('data-md/static_cpu_compare.md', 'w') as ofs:
            ofs.write('## Unit: seconds\n\n')
            ofs.write(static_cpu_compare_md)

        with open('data-md/static_mem_compare_md.md', 'w') as ofs:
            ofs.write('## Unit: MB\n\n')
            ofs.write(static_mem_compare_md)


    def md_dynamic():
        line_lst = [md_header, splitter_lst]
        line_lst2 = [md_header, splitter_lst]
        algorithm_abbr = dict(zip(dynamic_algorithm_lst, dynamic_label_lst))
        for idx, tag in enumerate(dynamic_algorithm_lst):
            line_lst.append([algorithm_abbr[tag]] + get_index_update_time_lst(tag))
            line_lst2.append([algorithm_abbr[tag]] + get_algorithm_dynamic_mem_lst(tag))

        static_cpu_compare_md = '\n'.join([gen_line(l) for l in line_lst])
        logger.info(static_cpu_compare_md)
        static_mem_compare_md = '\n'.join([gen_line(l) for l in line_lst2])
        logger.info(static_mem_compare_md)

        with open('data-md/dynamic_cpu_compare.md', 'w') as ofs:
            ofs.write('## Unit: seconds\n\n')
            ofs.write(static_cpu_compare_md)

        with open('data-md/dynamic_mem_compare_md.md', 'w') as ofs:
            ofs.write('## Unit: MB\n\n')
            ofs.write(static_mem_compare_md)


    md_static()
    md_dynamic()
