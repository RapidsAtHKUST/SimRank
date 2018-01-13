sfmt

```cpp
//  randomly choose [start_iter, end_iter), concept requires random access iterator
template<typename Iter>
inline Iter select_randomly_sfmt(Iter start, Iter end, SFMTRand &sfmt_rand_gen) {
//    auto advance_step = sfmt_rand_gen.uint_rand() % (std::distance(start, end));
    auto advance_step = sfmt_rand_gen.uint_rand() % (end - start);
//    std::advance(start, advance_step);
    // already sure within the range
    return start + advance_step;
}
```

```cpp
    // 3rd: ground truth
//    if (g.n < 10000) {
//        cout << "\n";
//        TruthSim ts(data_name, g, c, epsilon);
//        cout << format("ground truth: %s") % ts.sim(x, y) << endl;
//        cout << format("error: %s") % (ts.sim(q.first, q.second) - result) << endl;
//    }
```

```cpp
//    double delta = 0.00001;
```

```cpp
            // left: local, right: global or local ???
                max_err = max(max_err, abs(ts.sim(q.first, q.second) - res));
```