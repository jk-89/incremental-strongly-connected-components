EXECUTABLE = "build/main"

ALGORITHMS = [
    "naive_dfs",
    "naive_one_way_search",
    "one_way_search",
    "two_way_search",
    "limited_search",
    "compatible_search",
    "soft_threshold_search_basic_list",
    "soft_threshold_search_treap",
    "soft_threshold_search",
    "topological_search",
    "sample_search",
    "sparsified_sample_search",
]

GROUND_TRUTH = "naive_dfs"

CACHE_FILENAME = ".perf_cache.json"

CORRECTNESS_TEST_DIR = "tests/correctness/test_cases"
PERFORMANCE_TEST_DIR = "tests/performance/test_cases"
