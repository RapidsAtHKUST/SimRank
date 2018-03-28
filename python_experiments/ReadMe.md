## Index Dir and Space Complexity

see [IndexDir.md](IndexDir.md)

## Data Analysis Scripts

Data: in [data_analysis/data-json](data_analysis/data-json) and [data_analysis/data-markdown](data_analysis/data-markdown).

Scripts: see [data_analysis](data_analysis).

## Paper Figures Scripts

Figures: see [paper_figures/figures](paper_figures/figures)

Scripts: see [paper_figures](paper_figures)

## Run-Exp Scripts

utility files: [time_out_util.py](time_out_util.py), [generate_random_pairs.py](generate_random_pairs.py)

### Varying Datasets

file | utility
--- | ---
[run_our_executables.py](run_our_executables.py) | previously for index info
[run_other_algorithms.py](run_other_algorithms.py) | previously for index info
[run_our_executables_rand_pairs.py](run_our_executables_rand_pairs.py) | currently for querying, index time and size
[run_other_executables_rand_pairs.py](run_other_executables_rand_pairs.py) | currently for querying, index time and size

### Dynamic Update

file | utility
--- | ---
[run_local_push_dynamic_update.py](run_local_push_dynamic_update.py) | our flp
[run_tsf_dynamic_update.py](run_tsf_dynamic_update.py) | tsf

### Varying Parameters

file | utility
--- | ---
[run_varying_c_exp.py](run_varying_c_exp.py) | varying c
[run_varying_delta_exp.py](run_varying_delta_exp.py) | varying delta
[run_varying_eps_exp.py](run_varying_eps_exp.py) | varying eps
[run_varying_rmax_exp.py](run_varying_rmax_exp.py) | varying rmax, only for our flp

### READS

file | utility
--- | ---
[run_reads_executables_rand_pairs.py](run_reads_executables_rand_pairs.py) | varying datasets for querying time, indexing time and size
[run_reads_dynamic_update.py](run_reads_dynamic_update.py) | dynamic update (1000-edge insertions)
