binary search in discrete distribuiton

```cpp
 // set up the random number generator
        std::random_device rd;  //Will be used to obtain a seed for the random number engine
        std::mt19937 generator(rd()); //Standard mersenne_twister_engine seeded with rd()
//    std::discrete_distribution<int> residuals_dist(weights.begin(), weights.end());
```

```cpp
//        int index = residuals_dist(generator); // index for node pairs
//        int index = BinarySearchForGallopingSearch(reinterpret_cast<const double *>(&cdf.front()), 0, cdf.size(),
//                                                   rand_gen.double_rand());
//        int index = GallopingSearch(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
//                                    static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));
//        int index = GallopingSearchAVX2(&cdf.front(), 0, static_cast<uint32_t>(cdf.size()),
//                                                       static_cast<int>(rand_gen.double_rand() * YCHE_MAX_INT));

```