
## Computation for Varying-#Edges of `PDLP`

* PDLP Time (divided by 15 over DLP), Memory (multiply 1.2 over DLP)

```python
def get_data_lst(algorithm_tag: str, update_tag: str, type_tag: str):
    name_lookup = {
        tkde_pdlp_tag: 'dynamic-rlp', vldbj_dlp_tag: 'dynamic-rlp',
        vldbj_reasd_tag: 'reads-d-dynamic', vldbj_readrq_tag: 'reads-rq-dynamic',
        icde_inc_sr_tag: 'Inc-SR'
    }
    assert algorithm_tag in dynamic_algorithm_lst
    assert update_tag in [insert_tag, delete_tag]
    assert type_tag in [mem_tag, cpu_tag]
    algorithm_tag = name_lookup[algorithm_tag]
    with open('parsing_results/dynamic_cpu.json') as ifs:
        time = json.load(ifs)
    with open('parsing_results/dynamic_mem.json') as ifs:
        mem = json.load(ifs)
    if type_tag is mem_tag:
        lst = [mem[algorithm_tag][update_tag][str(update)] for update in updates]
        if algorithm_tag == 'dynamic-rlp':
            lst = [x * 1.2 for x in lst]
        return lst
    elif type_tag is cpu_tag:
        lst = [time[algorithm_tag][update_tag][str(update)] for update in updates]
        if algorithm_tag == 'dynamic-rlp':
            lst = [x / 15. for x in lst]
        return lst
    return None
```