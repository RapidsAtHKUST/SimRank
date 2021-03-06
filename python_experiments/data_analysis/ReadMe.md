## Max Error

json file: [accuracy_result.json](data-json/accuracy_result.json)

markdown file: [accuracy_result.md](data-markdown/accuracy_result.md)

file | description
--- | ---
[querying_time_accuracy_statistics.py](vldbj_data_parsing/querying_time_accuracy_statistics.py) | generate json for max err
[generate_accuracy_markdown.py](vldbj_data_parsing/generate_accuracy_markdown.py) | generate markdown given json input for max err

## Avg Query Time and Speedup over Sling

json file: [query_result_full_total_query_cpu_time.json](data-json/query_result_full_total_query_cpu_time.json)

markdown file: [speedup_over_sling.md](data-markdown/speedup_over_sling.md)

file | description
--- | ---
[generate_speedup_over_sling_markdown.py](vldbj_data_parsing/generate_speedup_over_sling_markdown.py) | generate avg query time (us) and speedup over sling
[querying_time_accuracy_statistics.py](vldbj_data_parsing/querying_time_accuracy_statistics.py) | generate json for query time, given different pair numbers

## Index

json file: [index_result.json](data-json/index_result.json)

markdown file: [index_result.md](data-markdown/index_result.md)

file | description
--- | ---
[indexing_time_space_statistics.py](vldbj_data_parsing/indexing_time_space_statistics.py) | generate json for indexing
[generate_index_markdown.py](vldbj_data_parsing/generate_index_markdown.py) | generate markdown given json input for indexing


## Varying Parameters

see folder [data-json/varying_parameters](data-json/varying_parameters)