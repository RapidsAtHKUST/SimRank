# SimRank

SimRank (single-pair query), abbreviation: SPS

## Submodules

see [dependencies](dependencies), do as follows for building source codes (download corresponding dependencies)

```zsh
git submodule init
git submodule update
```

## Release Codes (All-In-One-Project)

folder | detail
--- | ---
[SPS-Variants/APS-AllInOne](SPS-Variants/APS-AllInOne) | pflp/prlp/flp/rlp/our three ss MC algorithms, groud-truth/pcg/cloud-walker/linear-d/sling/reads/probesim

## Legacy Source Codes

folder | detail
--- | ---
[Our Parallel LP](LP-Parallel-Profile) | Our Parallel RLP and FLP implementations **1) lock-free, 2) with-lock**
[Our SPS Method Optimized](LPMC-Profile) | Our SPS optimized implementations
[Our SPS Method Not-Optimized](LPMC) | Our SPS not optimized implementations, with some other attempts to achieve better performance
[SPS-Variants](SPS-Variants) | ISP(2010), linearD(2014), cloud-walker(2015), tsf(2015), Sling(2016), READS(2017), probeSim(2018)
[Our APS](APS) | APS implementations(previous)

## Others

folder | detail 
--- | ---
[python_experiments](python_experiments) | python scripts for conducting experiments
[python_playground](python_playground) | python playground
[related_projects](related_projects) | other SimRank projects

## References

If you use the codes in your research, please kindly cite the following papers. 

```
@inproceedings{wang2018efficient,
  title={Efficient SimRank tracking in dynamic graphs},
  author={Wang, Yue and Lian, Xiang and Chen, Lei},
  booktitle={2018 IEEE 34th International Conference on Data Engineering (ICDE)},
  pages={545--556},
  year={2018},
  organization={IEEE}
}
```

```
@article{wang2019accelerating,
  title={Accelerating pairwise SimRank estimation over static and dynamic graphs},
  author={Wang, Yue and Chen, Lei and Che, Yulin and Luo, Qiong},
  journal={The VLDB Journal—The International Journal on Very Large Data Bases},
  volume={28},
  number={1},
  pages={99--122},
  year={2019},
  publisher={Springer-Verlag}
}
```

```
@article{wang2020fast,
  title={Fast and Accurate SimRank Computation via Forward Local Push and Its Parallelization},
  author={Wang, Yue and Che, Yulin and Lian, Xiang and Chen, Lei and Luo, Qiong},
  journal={IEEE Transactions on Knowledge and Data Engineering},
  year={2020},
  publisher={IEEE}
}
```
