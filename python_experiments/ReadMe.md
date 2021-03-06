## Index Dir and Space Complexity

see [IndexDir.md](IndexDir.md)

## Data Analysis Scripts

Data: in [data_analysis/data-json](data_analysis/data-json) and [data_analysis/data-markdown](data_analysis/data-markdown).

Scripts: see [data_analysis](data_analysis).

## Paper Figures Scripts

Figures: see [paper_figures/figures](paper_figures/figures)

Scripts: see [paper_figures](paper_figures)

## Run-Exp Scripts

utility files: [time_out_util.py](time_out_util.py), [generate_random_pairs.py](generate_inputs/generate_random_pairs.py)

### Parallel LocalPush

file | utility
--- | ---
[run_parallel_local_push.py](run_parallel_local_push.py) | scalability experiment
[run_parallel_local_push_gen_index.py](run_parallel_local_push_gen_index.py) | generate index via `pflp` and `prlp`

### Varying Datasets

file | utility
--- | ---
[run_our_executables.py](run_vldbj_experiments/run_our_executables.py) | previously for index info
[run_other_algorithms.py](run_vldbj_experiments/run_other_algorithms.py) | previously for index info
[run_our_executables_rand_pairs.py](run_vldbj_experiments/run_our_executables_rand_pairs.py) | currently for querying, index time and size
[run_other_executables_rand_pairs.py](run_vldbj_experiments/run_other_executables_rand_pairs.py) | currently for querying, index time and size

### READS

file | utility
--- | ---
[run_reads_executables_rand_pairs.py](run_vldbj_experiments/run_reads_executables_rand_pairs.py) | varying datasets for querying time, indexing time and size
[run_reads_dynamic_update.py](run_vldbj_experiments/run_reads_dynamic_update.py) | dynamic update (1000-edge insertions)

### Dynamic Update

file | utility
--- | ---
[run_local_push_dynamic_update.py](run_vldbj_experiments/run_local_push_dynamic_update.py) | our flp
[run_tsf_dynamic_update.py](run_vldbj_experiments/run_tsf_dynamic_update.py) | tsf

### Varying Parameters

file | utility
--- | ---
[run_varying_c_exp.py](run_vldbj_experiments/run_varying_c_exp.py) | varying c
[run_varying_delta_exp.py](run_vldbj_experiments/run_varying_delta_exp.py) | varying delta
[run_varying_eps_exp.py](run_vldbj_experiments/run_varying_eps_exp.py) | varying eps
[run_varying_rmax_exp.py](run_vldbj_experiments/run_varying_rmax_exp.py) | varying rmax, only for our flp
