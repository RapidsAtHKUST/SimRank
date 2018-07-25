# SimRank

SimRank (single-pair query), abbreviation: SPS

## Source Codes

folder | detail
--- | ---
[Our Parallel LP](LP-Parallel-Profile) | Our Parallel RLP and FLP implementations **1) lock-free, 2) with-lock**
[Our SPS Method Optimized](LPMC-Profile) | Our SPS optimized implementations
[Our SPS Method Not-Optimized](LPMC) | Our SPS not optimized implementations, with some other attempts to achieve better performance
[SPS-Variants](SPS-Variants) | ISP(2010), linearD(2014), cloud-walker(2015), tsf(2015), Sling(2016), READS(2017), probeSim(2018)
[Our APS](APS) | APS implementations(previous)

## Submodules

see [dependencies](dependencies), do as follows for building source codes (download corresponding dependencies)

```zsh
git submodule init
git submodule update
```

## Others

folder | detail 
--- | ---
[python_experiments](python_experiments) | python scripts for conducting experiments
[python_playground](python_playground) | python playground
[related_projects](related_projects) | other SimRank projects
