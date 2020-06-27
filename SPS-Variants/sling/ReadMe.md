## SLING: A Near-Optimal Index Structure for SimRank
                 
* parallel indexing not correct, now use tbb 

## Build

### LUOCPU9 (Use Latest TBB)

* See [tbb-releases](https://github.com/oneapi-src/oneTBB/releases).
* Download [latest-tbb](https://github.com/oneapi-src/oneTBB/releases/download/v2020.2/tbb-2020.2-lin.tgz).

```zsh
cmake .. -DDATA_ROOT_PATH=/mnt/storage1/yche/simrank-edge-list/ -DTBB_ROOT=/mnt/storage1/software/tbb
make -j
```

## Usage

```zsh
./sling_ss_ap ca-HepTh
```

                                                                                                                                                                                                                                                                                                                                                           