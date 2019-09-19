from paper_figures.tkde.tkde_draw_varying_datasets_time_mem import *


def gen_line(lst):
    return ' | '.join([str('%.3lf' % w if isinstance(w, float) else w) for w in lst])


if __name__ == '__main__':
    logger = get_logger('/home/yche/logs/tmp.log', __name__)
    md_folder = 'data-md'
    os.system('mkdir -p {}'.format(md_folder))

    md_cols = len(data_set_lst) + 1
    abbr_lst = [data_names[data] for data in data_set_lst]
    splitter_lst = ['---' for _ in range(md_cols)]
    md_header = ['Algorithm'] + abbr_lst
    line_lst = [md_header, splitter_lst]
    line_lst2 = [md_header, splitter_lst]

    algorithm_abbr = dict(zip(algorithm_tag_lst, label_lst))
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
