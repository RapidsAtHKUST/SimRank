## LJ

```
2020-08-18 06:36:56 ERROR (ts: 1597757816.728595 s, et: 4.664391 s)  input_outpu
t.cpp:63: Err, 547749888, 547749888, 131072, 53240
2020-08-18 06:36:56 INFO  (ts: 1597757816.728809 s, et: 4.664605 s)  input_outpu
t.cpp:69: 547749888, 547803128
2020-08-18 06:36:56 INFO  (ts: 1597757816.728958 s, et: 4.664753 s)  graph.h:66:Finish Loading
2020-08-18 06:36:56 INFO  (ts: 1597757816.758550 s, et: 4.694345 s)  graph.h:74: total vertex#: 4847572
2020-08-18 06:36:56 INFO  (ts: 1597757816.759350 s, et: 4.695146 s)  graph.h:75: total edge#: 68475391
2020-08-18 06:36:57 INFO  (ts: 1597757817.764673 s, et: 5.700468 s)  graph.h:99: Finish CSR
2020-08-18 06:36:58 INFO  (ts: 1597757818.280080 s, et: 6.215875 s)  graph.h:106: Finish Sort
delta(fail-prob):0.01
finish input graph: 1399 ms
total calD cpu time:26224s
finish calcD 821.343 s
2020-08-18 06:50:41 INFO  (ts: 1597758641.051643 s, et: 828.987439 s)  sling.cpp:232: Backward Multi Threading: 32
sort
sort finished
tcmalloc: large alloc 22056812544 bytes == 0x55a453be8000 @  0x7f180cecf887 0x55a3f46fb0c0 0x55a3f46f6d47 0x7f180bf5cb97 0x55a3f46f79ba
2020-08-18 06:56:35 INFO  (ts: 1597758995.956536 s, et: 1183.892331 s)  main_ss.cpp:70: Pre-Processing: 1183.892186s, Mem Usage: 44,649,272
finish backward 354.905 s
2020-08-18 06:56:36 INFO  (ts: 1597758996.169829 s, et: 1184.105624 s)  main_ss.cpp:81: Query Time: 0.213293s, Mem Usage: 44,649,272
total query cpu time:0.213232s
total query cost: 0.21326 s

```

## UND-LJ

```
eps_d:0.004, theta:0.000581989
2020-08-29 07:24:07 ERROR (ts: 1598711047.603055 s, et: 4.724424 s)  input_output.cpp:63: Err, 554827776, 554827776, 131072,
71248
2020-08-29 07:24:07 INFO  (ts: 1598711047.603270 s, et: 4.724639 s)  input_output.cpp:69: 554827776, 554899024
2020-08-29 07:24:07 INFO  (ts: 1598711047.603363 s, et: 4.724732 s)  graph.h:66: Finish Loading
2020-08-29 07:24:07 INFO  (ts: 1598711047.638035 s, et: 4.759404 s)  graph.h:74: total vertex#: 4036538
2020-08-29 07:24:07 INFO  (ts: 1598711047.638404 s, et: 4.759773 s)  graph.h:75: total edge#: 69362378
2020-08-29 07:24:08 INFO  (ts: 1598711048.643219 s, et: 5.764588 s)  graph.h:99: Finish CSR
2020-08-29 07:24:09 INFO  (ts: 1598711049.257337 s, et: 6.378706 s)  graph.h:106: Finish Sort
delta(fail-prob):0.01
finish input graph: 1249 ms
total calD cpu time:27822.1s
finish calcD 872.522 s
2020-08-29 07:38:43 INFO  (ts: 1598711923.063822 s, et: 880.185191 s)  sling.cpp:232: Backward Multi Threading: 32
sort
sort finished
tcmalloc: large alloc 19412205568 bytes == 0x55b8991b4000 @  0x7f32a7966887 0x55b839b30140 0x55b839b2bdc7 0x7f32a69f3b97 0x55b839b2ca3a
2020-08-29 07:44:13 INFO  (ts: 1598712253.297739 s, et: 1210.419108 s)  main_ss.cpp:71: Pre-Processing: 1210.418908s, Mem Usage: 39,460,960
finish backward 330.234 s
2020-08-29 07:44:13 INFO  (ts: 1598712253.433564 s, et: 1210.554933 s)  main_ss.cpp:82: Query Time: 0.135825s, Mem Usage: 39,461,064
total query cpu time:0.13579s
total query cost: 0.135789 s
```