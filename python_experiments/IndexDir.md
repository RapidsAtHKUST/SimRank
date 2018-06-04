### Dir Organization

```zsh
├── edge_list
├── ground_truth
├── input_pairs (random pairs)

├── cloudwalker (MCSP)
├── linearD (LIND)
├── local_push (FLP)
│   ├── backup
│   ├── backup_eps_updated
│   ├── backup_varying_c
│   ├── backup_varying_delta
│   ├── backup_varying_eps
│   └── backup-varying_rmax
├── reads
├── readsd (READS-D)
├── readsrq (READS-RQ)
└── sling （SLING）
    └── backup
```

method | index size
--- | ---
MCSP | D: O(V)
LIN  | D: O(V); P, PT: O(E)
TSF  | one-way-graphs: O(`R_g * V`)
READS-D | much larger than FLP
READS-RQ | simialr to FLP
SLING    | p (major component), p_start (2nd stage); d (1st stage): slightly large in big datasets, much larger in small datasets
FLP      | P and R: sparse matrix size

### Attention

* READS-D's tree index large
* SLING's p local structure large
