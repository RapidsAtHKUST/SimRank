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