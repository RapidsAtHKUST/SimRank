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