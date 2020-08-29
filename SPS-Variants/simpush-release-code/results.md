## 2020/08/17

### Peak Memory

```
/usr/bin/time -v ./simpush -f /mnt/storage-praha16/dataset/lj.e.bin -qn 5
```

* `./simpush -f /mnt/storage-praha16/dataset/com-lj.ud.bin -qn 5`

```
2020-08-29 07:11:25 ERROR (ts: 1598710285.799132 s, et: 4.722694 s)  Graph.h:44: Err, 554827776, 554827776, 131072, 71248
2020-08-29 07:11:25 INFO  (ts: 1598710285.799379 s, et: 4.722940 s)  Graph.h:50: 554827776, 554899024
2020-08-29 07:11:25 INFO  (ts: 1598710285.799484 s, et: 4.723045 s)  Graph.h:167: Finish Loading
2020-08-29 07:11:25 INFO  (ts: 1598710285.830694 s, et: 4.754256 s)  Graph.h:175: total vertex#: 4036538
2020-08-29 07:11:25 INFO  (ts: 1598710285.831169 s, et: 4.754730 s)  Graph.h:176: total edge#: 69362378
: loaded graph data
2020-08-29 07:11:27 INFO  (ts: 1598710287.819439 s, et: 6.743001 s)  Graph.h:220: Finish CSR
2020-08-29 07:11:28 INFO  (ts: 1598710288.337106 s, et: 7.260667 s)  Graph.h:227: Finish Sort
query node: 5
REPEAT: 3
eps: 0.02
2020-08-29 07:11:28 INFO  (ts: 1598710288.414332 s, et: 7.337893 s)  main_ss_yche.cpp:69: PreProcessing Time: 7.337735s, Mem
Usage: 950,240
2020-08-29 07:11:28 INFO  (ts: 1598710288.414395 s, et: 7.337957 s)  main_ss_yche.cpp:73: Clock Time: 33.723320s, Mem Usage:
950,240
2020-08-29 07:11:28 INFO  (ts: 1598710288.538028 s, et: 7.461590 s)  main_ss_yche.cpp:85: Query Time: 0.123607s, Mem Usage: 966,260 KB
2020-08-29 07:11:28 INFO  (ts: 1598710288.655993 s, et: 7.579554 s)  main_ss_yche.cpp:85: Query Time: 0.117920s, Mem Usage: 968,820 KB
2020-08-29 07:11:28 INFO  (ts: 1598710288.765516 s, et: 7.689078 s)  main_ss_yche.cpp:85: Query Time: 0.109481s, Mem Usage: 968,820 KB
```

* `./simpush -f /mnt/storage-praha16/dataset/lj.e.bin -qn 5`

```
2020-08-17 07:45:18 ERROR (ts: 1597675518.147494 s, et: 4.662824 s)  Graph.h:44: Err, 547749888, 547749888, 131072, 53240
2020-08-17 07:45:18 INFO  (ts: 1597675518.147727 s, et: 4.663057 s)  Graph.h:50: 547749888, 547803128
2020-08-17 07:45:18 INFO  (ts: 1597675518.147829 s, et: 4.663159 s)  Graph.h:167: Finish Loading
2020-08-17 07:45:18 INFO  (ts: 1597675518.176200 s, et: 4.691530 s)  Graph.h:175: total vertex#: 4847572
2020-08-17 07:45:18 INFO  (ts: 1597675518.176786 s, et: 4.692117 s)  Graph.h:176: total edge#: 68475391
: loaded graph data
2020-08-17 07:45:20 INFO  (ts: 1597675520.029599 s, et: 6.544930 s)  Graph.h:220: Finish CSR
2020-08-17 07:45:20 INFO  (ts: 1597675520.482530 s, et: 6.997860 s)  Graph.h:227: Finish Sort
query node: 5
REPEAT: 3
eps: 0.02
2020-08-17 07:45:20 INFO  (ts: 1597675520.555663 s, et: 7.070993 s)  main_ss_yche.cpp:69: PreProcessing Time: 7.070836s, Mem Usage:
897,972
2020-08-17 07:45:20 INFO  (ts: 1597675520.555723 s, et: 7.071053 s)  main_ss_yche.cpp:73: Clock Time: 32.966553s, Mem Usage: 897,972
2020-08-17 07:45:20 INFO  (ts: 1597675520.582320 s, et: 7.097650 s)  main_ss_yche.cpp:85: Query Time: 0.026584s, Mem Usage: 900,372
KB
2020-08-17 07:45:20 INFO  (ts: 1597675520.606370 s, et: 7.121700 s)  main_ss_yche.cpp:85: Query Time: 0.024007s, Mem Usage: 900,560
KB
2020-08-17 07:45:20 INFO  (ts: 1597675520.680557 s, et: 7.195887 s)  main_ss_yche.cpp:85: Query Time: 0.074130s, Mem Usage: 932,232
KB
result written to result/

```

* `./simpush -f /mnt/storage-praha16/dataset/tw.e.bin -qn 19`

```
2020-08-17 07:49:14 ERROR (ts: 1597675754.510616 s, et: 99.917362 s)  Graph.h:44: Err, 11746803712, 11746803712, 131072, 115360
2020-08-17 07:49:14 INFO  (ts: 1597675754.510836 s, et: 99.917582 s)  Graph.h:50: 11746803712, 11746919072
2020-08-17 07:49:14 INFO  (ts: 1597675754.513132 s, et: 99.919877 s)  Graph.h:167: Finish Loading
2020-08-17 07:49:15 INFO  (ts: 1597675755.215768 s, et: 100.622513 s)  Graph.h:175: total vertex#: 41652230
2020-08-17 07:49:15 INFO  (ts: 1597675755.216501 s, et: 100.623247 s)  Graph.h:176: total edge#: 1468364884
: loaded graph data
2020-08-17 07:49:51 INFO  (ts: 1597675791.376086 s, et: 136.782832 s)  Graph.h:220: Finish CSR
2020-08-17 07:50:14 INFO  (ts: 1597675814.036468 s, et: 159.443213 s)  Graph.h:227: Finish Sort
query node: 19
REPEAT: 3
eps: 0.02
2020-08-17 07:50:15 INFO  (ts: 1597675815.441717 s, et: 160.848463 s)  main_ss_yche.cpp:69: PreProcessing Time: 160.848301s, Mem Usage: 14,364,532
2020-08-17 07:50:15 INFO  (ts: 1597675815.441781 s, et: 160.848527 s)  main_ss_yche.cpp:73: Clock Time: 824.624217s, Mem Usage: 14,364,532
2020-08-17 07:50:18 INFO  (ts: 1597675818.771174 s, et: 164.177919 s)  main_ss_yche.cpp:85: Query Time: 3.329379s, Mem Usage: 14,365,320 KB
2020-08-17 07:50:21 INFO  (ts: 1597675821.813458 s, et: 167.220203 s)  main_ss_yche.cpp:85: Query Time: 3.042244s, Mem Usage: 14,365,344 KB
2020-08-17 07:50:24 INFO  (ts: 1597675824.834229 s, et: 170.240975 s)  main_ss_yche.cpp:85: Query Time: 3.020729s, Mem Usage: 14,365,364 KB
result written to result/
```

* `./simpush -f /mnt/storage-praha16/dataset/fs.ebin -qn 0 `

```
2020-08-17 07:54:12 ERROR (ts: 1597676052.992924 s, et: 130.913988 s)  Graph.h:44: Err, 14448459776, 14448459776, 131072, 77304     
2020-08-17 07:54:12 INFO  (ts: 1597676052.993137 s, et: 130.914200 s)  Graph.h:50: 14448459776, 14448537080                         
2020-08-17 07:54:12 INFO  (ts: 1597676052.993217 s, et: 130.914280 s)  Graph.h:167: Finish Loading
2020-08-17 07:54:13 INFO  (ts: 1597676053.845643 s, et: 131.766706 s)  Graph.h:175: total vertex#: 65608366
2020-08-17 07:54:13 INFO  (ts: 1597676053.846340 s, et: 131.767403 s)  Graph.h:176: total edge#: 1806067135
: loaded graph data
2020-08-17 07:55:25 INFO  (ts: 1597676125.578363 s, et: 203.499426 s)  Graph.h:220: Finish CSR
2020-08-17 07:55:29 INFO  (ts: 1597676129.469021 s, et: 207.390084 s)  Graph.h:227: Finish Sort
query node: 0
REPEAT: 3
eps: 0.02
2020-08-17 07:55:31 INFO  (ts: 1597676131.290229 s, et: 209.211292 s)  main_ss_yche.cpp:69: PreProcessing Time: 209.211135s, Mem Usa
ge: 18,484,056
2020-08-17 07:55:31 INFO  (ts: 1597676131.290300 s, et: 209.211363 s)  main_ss_yche.cpp:73: Clock Time: 1574.193970s, Mem Usage: 18,
484,056
2020-08-17 07:55:31 INFO  (ts: 1597676131.469753 s, et: 209.390817 s)  main_ss_yche.cpp:85: Query Time: 0.179439s, Mem Usage: 18,484
,056 KB
2020-08-17 07:55:31 INFO  (ts: 1597676131.649987 s, et: 209.571050 s)  main_ss_yche.cpp:85: Query Time: 0.180194s, Mem Usage: 18,484
,056 KB
2020-08-17 07:55:31 INFO  (ts: 1597676131.829620 s, et: 209.750683 s)  main_ss_yche.cpp:85: Query Time: 0.179592s, Mem Usage: 18,484
,056 KB
result written to result/
```

* `./simpush -f /mnt/storage-praha16/dataset/fs.ue.bin -qn 0`

```
2020-08-23 07:10:06 ERROR (ts: 1598191806.450006 s, et: 273.054908 s)  Graph.h:44: Err, 28897050624, 28897050624, 131072, 23536
2020-08-23 07:10:06 INFO  (ts: 1598191806.450246 s, et: 273.055148 s)  Graph.h:50: 28897050624, 28897074160
2020-08-23 07:10:06 INFO  (ts: 1598191806.450523 s, et: 273.055425 s)  Graph.h:167: Finish Loading
2020-08-23 07:10:09 INFO  (ts: 1598191809.130448 s, et: 275.735350 s)  Graph.h:175: total vertex#: 65608366
2020-08-23 07:10:09 INFO  (ts: 1598191809.130572 s, et: 275.735473 s)  Graph.h:176: total edge#: 3612134270
: loaded graph data
2020-08-23 07:12:09 INFO  (ts: 1598191929.964117 s, et: 396.569019 s)  Graph.h:220: Finish CSR
2020-08-23 07:12:21 INFO  (ts: 1598191941.796938 s, et: 408.401840 s)  Graph.h:227: Finish Sort
query node: 0
REPEAT: 3
eps: 0.02
2020-08-23 07:12:23 INFO  (ts: 1598191943.566025 s, et: 410.170927 s)  main_ss_yche.cpp:69: PreProcessing Time: 410.170750s, Mem Usage: 33,112,040
2020-08-23 07:12:23 INFO  (ts: 1598191943.566085 s, et: 410.170987 s)  main_ss_yche.cpp:73: Clock Time: 2911.792834s, Mem Usage: 33,112,040
2020-08-23 07:12:23 INFO  (ts: 1598191943.810904 s, et: 410.415805 s)  main_ss_yche.cpp:85: Query Time: 0.244807s, Mem Usage: 33,131,020 KB
2020-08-23 07:12:24 INFO  (ts: 1598191944.043085 s, et: 410.647987 s)  main_ss_yche.cpp:85: Query Time: 0.232139s, Mem Usage: 33,148,192 KB
2020-08-23 07:12:24 INFO  (ts: 1598191944.271748 s, et: 410.876650 s)  main_ss_yche.cpp:85: Query Time: 0.228619s, Mem Usage: 33,148,428 KB
result written to result/
```

* ` ./simpush -f /mnt/storage-praha16/dataset/uk.e.bin -qn 1`

```
2020-08-17 08:06:19 ERROR (ts: 1597676779.174821 s, et: 279.033815 s)  Graph.h:44: Err, 29909843968, 29909843968, 131072, 25216
2020-08-17 08:06:19 INFO  (ts: 1597676779.183132 s, et: 279.042126 s)  Graph.h:50: 29909843968, 29909869184
2020-08-17 08:06:19 INFO  (ts: 1597676779.183855 s, et: 279.042849 s)  Graph.h:167: Finish Loading
2020-08-17 08:06:22 INFO  (ts: 1597676782.130667 s, et: 281.989661 s)  Graph.h:175: total vertex#: 105896435
2020-08-17 08:06:22 INFO  (ts: 1597676782.130744 s, et: 281.989738 s)  Graph.h:176: total edge#: 3738733648
: loaded graph data
2020-08-17 08:07:00 INFO  (ts: 1597676820.778932 s, et: 320.637926 s)  Graph.h:220: Finish CSR
2020-08-17 08:07:10 INFO  (ts: 1597676830.140969 s, et: 329.999963 s)  Graph.h:227: Finish Sort
query node: 1
REPEAT: 3
eps: 0.02
2020-08-17 08:07:12 INFO  (ts: 1597676832.241886 s, et: 332.100880 s)  main_ss_yche.cpp:69: PreProcessing Time: 332.100719s, Mem Usage: 36,868,212
2020-08-17 08:07:12 INFO  (ts: 1597676832.241960 s, et: 332.100954 s)  main_ss_yche.cpp:73: Clock Time: 814.669545s, Mem Usage: 36,868,212
2020-08-17 08:07:12 INFO  (ts: 1597676832.513054 s, et: 332.372048 s)  main_ss_yche.cpp:85: Query Time: 0.271066s, Mem Usage: 36,878,300 KB
2020-08-17 08:07:12 INFO  (ts: 1597676832.779923 s, et: 332.638917 s)  main_ss_yche.cpp:85: Query Time: 0.266827s, Mem Usage: 36,883,296 KB
2020-08-17 08:07:13 INFO  (ts: 1597676833.050066 s, et: 332.909060 s)  main_ss_yche.cpp:85: Query Time: 0.270101s, Mem Usage: 36,885,640 KB
result written to result/

```