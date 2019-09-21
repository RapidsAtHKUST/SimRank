import json

if __name__ == '__main__':
    with open('varying_eps_c.dicts') as ifs:
        pcg_varying_eps_cpu = eval(ifs.readline())
        pcg_varying_eps_mem = eval(ifs.readline())

        pcg_varying_c_cpu = eval(ifs.readline())
        pcg_varying_c_mem = eval(ifs.readline())

        pcg_tag = 'pcg'

        with open('pcg-varying-eps-cpu.json', 'w') as ofs:
            ofs.write(json.dumps({
                pcg_tag: {
                    '0.6':
                        pcg_varying_eps_cpu

                }
            }, indent=4))
        with open('pcg-varying-eps-mem.json', 'w') as ofs:
            ofs.write(json.dumps({
                pcg_tag: {
                    '0.6':
                        pcg_varying_eps_mem

                }
            }, indent=4))
        with open('pcg-varying-eps-cpu.json', 'w') as ofs:
            ofs.write(json.dumps({
                pcg_tag: {
                    '0.6':
                        pcg_varying_eps_cpu

                }
            }, indent=4))


        def combine(data: dict, extra):
            res = dict()
            for c, val in data.items():
                res[c] = {extra: val}
            return res


        with open('pcg-varying-c-cpu.json', 'w') as ofs:
            ofs.write(json.dumps({
                pcg_tag:
                    combine(pcg_varying_c_cpu, '0.01')
            }, indent=4))
        with open('pcg-varying-c-mem.json', 'w') as ofs:
            ofs.write(json.dumps({
                pcg_tag:
                    combine(pcg_varying_c_mem, '0.01')

            }, indent=4))
