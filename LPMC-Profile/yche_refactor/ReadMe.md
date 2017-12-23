modified one

```cpp
pair<double, int> BackPush::backward_push(NodePair np, unique_max_heap &container) 
```

```cpp
auto indeg_a = in_degree(a, *g);
            auto indeg_b = in_degree(b, *g);
            GraphYche::in_edge_iterator ina_it, ina_end;
            GraphYche::in_edge_iterator inb_it, inb_end;
            tie(ina_it, ina_end) = in_edges(a, *g);
            for (; ina_it != ina_end; ++ina_it) {
                auto ina = source(*ina_it, *g);
                tie(inb_it, inb_end) = in_edges(b, *g);
                for (; inb_it != inb_end; ++inb_it) {
                    auto inb = source(*inb_it, *g);
                    // cout << a << " " << b << " pushing to: " << ina << " " << inb << endl;
                    container.push(NodePair{ina, inb}, c * residual / (indeg_a * indeg_b));
                    ++cost;
                }
            }
```

try to use less memory

```cpp
//    GraphYche *g_ptr; // the underlying graph
//    heap_data(NodePair np_, double residual_, GraphYche &g) {
//        np = np_;
//        residual = residual_;
//        g_ptr = &g;
//    }
//

```

update random std

```cpp
template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
//    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::uniform_int_distribution<> dis(0, (end - start) - 1);
//    std::advance(start, dis(g));
    start += dis(g);
    return start;
}
```