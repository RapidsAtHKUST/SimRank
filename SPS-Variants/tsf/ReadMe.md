## Play

```zsh
cmake-build-debug/tsf ../READS/datasets/cloud_walker.txt
```

however, sim value not correct

## One-Way Graph Index

```cpp
        src = new int[maxVertexId];
        dst = new int[maxVertexId];
```

## Graph Binary File Format

reversed graph

`n`, `m`, `off` (size: n+1), `dst`, `rvertices` (lookup_table, size: n)
 
exp | meaning
--- | ---
`vertices[rvertices[i]]` | original vertex id for the offset i 
`graph_src[i]==graph_src[i+1]` | degree-zero vertex

```cpp
        fread(&MAX_VERTEX_NUM, sizeof(int), 1, fp);
        fread(&edgeNum, sizeof(int), 1, fp);
        graph_src = new int[MAX_VERTEX_NUM + 1];
        graph_dst = new int[edgeNum];
        rvertices = new int[MAX_VERTEX_NUM];
        fread(graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
        fread(graph_dst, sizeof(int), edgeNum, fp);
        fread(rvertices, sizeof(int), MAX_VERTEX_NUM, fp);
        vertices.resize(MAX_VERTEX_NUM);
        for (int i = 0; i < MAX_VERTEX_NUM; ++i) {
            vertices[rvertices[i]] = i;
        }
```

original graph

`n`, `m`, `off` (size: n+1), `dst`

```cpp
                int a, b;
                fread(&a, sizeof(int), 1, fp);
                fread(&b, sizeof(int), 1, fp);
                orig_graph_src = new int[MAX_VERTEX_NUM + 1];
                orig_graph_dst = new int[edgeNum];
                fread(orig_graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
                fread(orig_graph_dst, sizeof(int), edgeNum, fp);
                fclose(fp);
```

construct original from the reversed one

```cpp
       /*construct the original version */
                orig_graph_src = new int[edgeNum];
                orig_graph_dst = new int[edgeNum];
                int *tcnt = new int[MAX_VERTEX_NUM + 1];
                memset(tcnt, 0, sizeof(int) * (MAX_VERTEX_NUM + 1));
                int et = 0;
                for (int i = 0; i < MAX_VERTEX_NUM; ++i) {
                    int s = graph_src[i];
                    int e = graph_src[i + 1];
                    if (e == s) {
                        continue;
                    }
                    for (int j = s; j < e; ++j) {
                        tcnt[graph_dst[j]]++;
                        orig_graph_src[et] = graph_dst[j];
                        orig_graph_dst[et] = i;
                        et++;
                    }
                }
                int *otmp = new int[edgeNum];
                for (int i = 1; i <= MAX_VERTEX_NUM; ++i) tcnt[i] += tcnt[i - 1];
                for (int i = 0; i < edgeNum; ++i) {
                    --tcnt[orig_graph_src[i]];
                    otmp[tcnt[orig_graph_src[i]]] = orig_graph_dst[i];
                }
                delete[] orig_graph_src;
                delete[] orig_graph_dst;
                orig_graph_src = tcnt;
                orig_graph_dst = otmp;

                fp = fopen(orig_processedGraphPath, "wb");
                fwrite(&MAX_VERTEX_NUM, sizeof(int), 1, fp);
                fwrite(&edgeNum, sizeof(int), 1, fp);
                fwrite(orig_graph_src, sizeof(int), MAX_VERTEX_NUM + 1, fp);
                fwrite(orig_graph_dst, sizeof(int), edgeNum, fp);
                fclose(fp);
```