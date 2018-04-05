# Average Singe Pair Query

## Query Time (us)

algo\data | ca-GrQc | ca-HepTh | p2p-Gnutella06 | wiki-Vote | email-Enron | email-EuAll
--- | --- | --- | --- | --- | --- | ---
bflpmc | 7.615 us | 8.095 us | 7.245 us | 2.075 us | 16.805 us | 1.340 us
flpmc | 8.160 us | 8.635 us | 8.420 us | 2.080 us | 21.590 us | 2.775 us
bprw | 118.070 us | 133.780 us | 78.740 us | 50.310 us | 197.505 us | 7.550 us
sling | 500.990 us | 484.445 us | 276.660 us | 275.410 us | 1241.555 us | 115.295 us
isp | 2604.600 us | 3636.700 us | 3368.650 us | 2758.200 us | 51793.000 us | 2657.250 us
reads-s | 446.800 us | 993.350 us | 1518.250 us | 723.800 us | 11192.950 us | 1056.800 us
reads-d | 823.300 us | 1639.300 us | 2365.800 us | 1222.050 us | 15091.600 us | 1241.400 us
reads-rq | 10448.200 us | 11101.450 us | 9259.550 us | 586.900 us | 16097.800 us | 1453.150 us
tsf | 46951.650 us | 47567.200 us | 46057.800 us | 4361.050 us | 31715.750 us | 2793.100 us
lind | 1205.850 us | 2273.400 us | 1702.900 us | 2592.200 us | 13038.950 us | 246623.000 us
cw | 207048.000 us | 201243.500 us | 209022.500 us | 18417.750 us | 212362.000 us | 257305.000 us

algo\data | web-NotreDame | web-Stanford | web-BerkStan | web-Google | cit-Patents | soc-LiveJournal1 | wiki-Link
--- | --- | --- | --- | --- | --- | --- | ---
bflpmc | 15.055 us | 11.090 us | 12.015 us | 7.710 us | 9.195 us | 35.595 us | 108.080 us
flpmc | 22.115 us | 13.210 us | 13.615 us | 9.380 us | 10.210 us | 45.630 us | 123.025 us
bprw | 75.070 us | 85.535 us | 91.385 us | 55.920 us | 25.860 us | 420.355 us | 183.395 us
sling | 541.795 us | 487.170 us | 543.015 us | 340.265 us | 69.540 us | 821.230 us | 764.840 us
isp | 9614.100 us | 35612.600 us | 30532.100 us | 2731.100 us | 187.850 us | 29270.000 us | 146373.500 us
reads-s | 20375.000 us | 4807.050 us | 5172.250 us | 4915.750 us | 20680.700 us | 171369.500 us | 4999999999999995.000 us
reads-d | 27119.500 us | 5894.750 us | 6024.050 us | 5246.250 us | 23107.550 us | 228233.500 us | 256040.000 us
reads-rq | 38076.450 us | 12044.100 us | 15391.300 us | 7746.100 us | 17761.850 us | 65402.000 us | 101100.000 us
tsf | 57231.500 us | 49924.800 us | 38958.950 us | 30601.550 us | 11146.650 us | 28308.550 us | 27802.800 us
lind | 213140.500 us | 279501.500 us | 772805.000 us | 1627740.000 us | 3371840.000 us | 58471000.000 us | 9999999999.000 us
cw | 311607.500 us | 302145.000 us | 683985.000 us | 910875.000 us | 3978750.000 us | 5745600.000 us | 12509550.000 us

## Speedup Over Sling

algo\data | ca-GrQc | ca-HepTh | p2p-Gnutella06 | wiki-Vote | email-Enron | email-EuAll
--- | --- | --- | --- | --- | --- | ---
bflpmc | 65.790 | 59.845 | 38.186 | 132.728 | 73.880 | 86.041
flpmc | 61.396 | 56.102 | 32.857 | 132.409 | 57.506 | 41.548
bprw | 4.243 | 3.621 | 3.514 | 5.474 | 6.286 | 15.271

algo\data | web-NotreDame | web-Stanford | web-BerkStan | web-Google | cit-Patents | soc-LiveJournal1 | wiki-Link
--- | --- | --- | --- | --- | --- | --- | ---
bflpmc | 35.988 | 43.929 | 45.195 | 44.133 | 7.563 | 23.071 | 7.077
flpmc | 24.499 | 36.879 | 39.884 | 36.276 | 6.811 | 17.998 | 6.217
bprw | 7.217 | 5.696 | 5.942 | 6.085 | 2.689 | 1.954 | 4.170

