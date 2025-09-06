from tests.common.test_config import ALGORITHMS, PERFORMANCE_TEST_DIR
from tests.common.graph import (
    RandomGraph,
    RandomDAG,
    RandomAlmostDAG,
    MixedDegrees,
    PathsCollection,
    CliquesCollection,
)
from tests.common.common import write_case_in

DEFAULT_ALGS = [
    alg
    for alg in ALGORITHMS
    if alg != "soft_threshold_search_basic_list"
    and alg != "soft_threshold_search_treap"
]
NON_TRIVIAL_ALGS = [alg for alg in DEFAULT_ALGS if alg != "naive_dfs"]
FASTEST_ALGS = [
    "two_way_search",
    "compatible_search",
    "soft_threshold_search",
    "sparsified_sample_search",
    "sample_search",
]
FASTEST_ALGS_WITHOUT_SAMPLE = [
    alg
    for alg in FASTEST_ALGS
    if alg != "sample_search" and alg != "sparsified_sample_search"
]
OPTIMAL_MEMORY_ALGS = [alg for alg in DEFAULT_ALGS if alg != "topological_search"]
SEMIFAST_OPTIMAL_MEMORY_ALGS = [
    alg for alg in OPTIMAL_MEMORY_ALGS if alg != "naive_dfs"
]
DENSE_DAG_ENHANCED_ALGS = [
    alg for alg in NON_TRIVIAL_ALGS if alg != "naive_one_way_search"
]
DENSE_DAG_ALGS = [alg for alg in DENSE_DAG_ENHANCED_ALGS if alg != "one_way_search"]
SPARSE_DAG_ALGS = [
    alg for alg in SEMIFAST_OPTIMAL_MEMORY_ALGS if alg != "two_way_search"
]
SPARSE_DAG_NARROWED_ALGS = [alg for alg in SPARSE_DAG_ALGS if alg != "limited_search"]
MIXED_DEGREES_ALGS = [alg for alg in FASTEST_ALGS if alg != "sample_search"]
PATHS_COLLECTION_ENHANCED_ALGS = [
    alg for alg in SEMIFAST_OPTIMAL_MEMORY_ALGS if alg != "naive_one_way_search"
]
PATHS_COLLECTION_ALGS = [
    alg for alg in PATHS_COLLECTION_ENHANCED_ALGS if alg != "limited_search"
]
CLIQUES_COLLECTION_ALGS = FASTEST_ALGS + ["limited_search"]


class Test:
    def __init__(self, graph, name, algorithms=None, relabel_vertices=False):
        if algorithms is None:
            algorithms = DEFAULT_ALGS
        self.graph = graph
        self.name = name
        self.algorithms = algorithms
        self.relabel_vertices = relabel_vertices

    def generate(self):
        if self.graph is not None:
            edges = self.graph.generate_edges(self.relabel_vertices)
            write_case_in(self.name, edges, PERFORMANCE_TEST_DIR)


_TESTS_TO_GENERATE = [
    (RandomGraph(20000, 40000, 20), "random_small_sparse_graph_1", OPTIMAL_MEMORY_ALGS),
    (RandomGraph(30000, 60000, 21), "random_small_sparse_graph_2", OPTIMAL_MEMORY_ALGS),
    (
        RandomGraph(50000, 100000, 22),
        "random_small_sparse_graph_3",
        SEMIFAST_OPTIMAL_MEMORY_ALGS,
    ),
    (
        RandomGraph(70000, 140000, 23),
        "random_small_sparse_graph_4",
        FASTEST_ALGS + ["one_way_search", "naive_one_way_search"],
    ),
    (RandomGraph(300000, 600000, 30), "random_sparse_graph_1", FASTEST_ALGS),
    (RandomGraph(500000, 1000000, 31), "random_sparse_graph_2", FASTEST_ALGS),
    (RandomGraph(700000, 1400000, 32), "random_sparse_graph_3", FASTEST_ALGS),
    (RandomGraph(900000, 1800000, 33), "random_sparse_graph_4", FASTEST_ALGS),
    (RandomGraph(1100000, 2200000, 34), "random_sparse_graph_5", FASTEST_ALGS),
    (RandomGraph(1300000, 2600000, 35), "random_sparse_graph_6", FASTEST_ALGS),
    (RandomGraph(1500000, 3000000, 36), "random_sparse_graph_7", FASTEST_ALGS),
    (RandomDAG(10000, 1000000, 40), "random_dense_dag_1", DENSE_DAG_ENHANCED_ALGS),
    (RandomDAG(10000, 2000000, 41), "random_dense_dag_2", DENSE_DAG_ENHANCED_ALGS),
    (RandomDAG(10000, 3000000, 42), "random_dense_dag_3", DENSE_DAG_ENHANCED_ALGS),
    (RandomDAG(10000, 4000000, 43), "random_dense_dag_4", DENSE_DAG_ALGS),
    (RandomDAG(10000, 5000000, 44), "random_dense_dag_5", DENSE_DAG_ALGS),
    (RandomDAG(10000, 6000000, 45), "random_dense_dag_6", DENSE_DAG_ALGS),
    (
        RandomDAG(400000, 2400000, 50),
        "random_sparse_dag_1",
        SEMIFAST_OPTIMAL_MEMORY_ALGS,
    ),
    (
        RandomDAG(600000, 3600000, 51),
        "random_sparse_dag_2",
        SEMIFAST_OPTIMAL_MEMORY_ALGS,
    ),
    (RandomDAG(800000, 4800000, 52), "random_sparse_dag_3", SPARSE_DAG_ALGS),
    (RandomDAG(1000000, 6000000, 53), "random_sparse_dag_4", SPARSE_DAG_ALGS),
    (RandomDAG(1200000, 7200000, 54), "random_sparse_dag_5", SPARSE_DAG_ALGS),
    (
        RandomAlmostDAG(400000, 2400000, 50, 60),
        "random_almost_dag_1",
        SEMIFAST_OPTIMAL_MEMORY_ALGS,
    ),
    (
        RandomAlmostDAG(600000, 3600000, 50, 61),
        "random_almost_dag_2",
        SEMIFAST_OPTIMAL_MEMORY_ALGS,
    ),
    (RandomAlmostDAG(800000, 4800000, 50, 62), "random_almost_dag_3", SPARSE_DAG_ALGS),
    (
        RandomAlmostDAG(1000000, 6000000, 50, 63),
        "random_almost_dag_4",
        SPARSE_DAG_NARROWED_ALGS,
    ),
    (
        RandomAlmostDAG(1200000, 7200000, 50, 64),
        "random_almost_dag_5",
        SPARSE_DAG_NARROWED_ALGS,
    ),
    (MixedDegrees(400000, 1200000, 1000, 400, 70), "mixed_degrees_1", FASTEST_ALGS),
    (MixedDegrees(600000, 1800000, 1000, 400, 71), "mixed_degrees_2", FASTEST_ALGS),
    (
        MixedDegrees(800000, 2400000, 1000, 400, 72),
        "mixed_degrees_3",
        MIXED_DEGREES_ALGS,
    ),
    (
        MixedDegrees(1000000, 3000000, 1000, 400, 73),
        "mixed_degrees_4",
        MIXED_DEGREES_ALGS,
    ),
    (
        MixedDegrees(1200000, 3600000, 1000, 400, 74),
        "mixed_degrees_5",
        MIXED_DEGREES_ALGS,
    ),
    (
        MixedDegrees(1400000, 4200000, 1000, 400, 75),
        "mixed_degrees_6",
        MIXED_DEGREES_ALGS,
    ),
    (
        PathsCollection(200, 500),
        "paths_collection_1",
        PATHS_COLLECTION_ENHANCED_ALGS,
        False,
    ),
    (
        PathsCollection(300, 500),
        "paths_collection_2",
        PATHS_COLLECTION_ENHANCED_ALGS,
        False,
    ),
    (PathsCollection(400, 500), "paths_collection_3", PATHS_COLLECTION_ALGS, False),
    (PathsCollection(500, 500), "paths_collection_4", PATHS_COLLECTION_ALGS, False),
    (PathsCollection(600, 500), "paths_collection_5", PATHS_COLLECTION_ALGS, False),
    (
        CliquesCollection(100000, 400000),
        "cliques_collection_1",
        CLIQUES_COLLECTION_ALGS,
        False,
    ),
    (
        CliquesCollection(200000, 800000),
        "cliques_collection_2",
        CLIQUES_COLLECTION_ALGS,
        False,
    ),
    (
        CliquesCollection(300000, 1200000),
        "cliques_collection_3",
        CLIQUES_COLLECTION_ALGS,
        False,
    ),
    (
        CliquesCollection(400000, 1600000),
        "cliques_collection_4",
        CLIQUES_COLLECTION_ALGS,
        False,
    ),
    (
        CliquesCollection(500000, 2000000),
        "cliques_collection_5",
        CLIQUES_COLLECTION_ALGS,
        False,
    ),
]

_REAL_LIFE_TESTS = [
    (None, "google_web_graph", FASTEST_ALGS),
    (None, "road_network_california", SEMIFAST_OPTIMAL_MEMORY_ALGS),
    (None, "road_network_pennsylvania", SEMIFAST_OPTIMAL_MEMORY_ALGS),
    (None, "road_network_texas", SEMIFAST_OPTIMAL_MEMORY_ALGS),
    (None, "wikipedia_talk", FASTEST_ALGS),
    (None, "twitter", FASTEST_ALGS + ["naive_one_way_search", "one_way_search"]),
    (None, "google_plus", FASTEST_ALGS),
    (None, "live_journal", FASTEST_ALGS_WITHOUT_SAMPLE),
    (None, "mathoverflow", SEMIFAST_OPTIMAL_MEMORY_ALGS),
    (None, "superuser", FASTEST_ALGS),
    (None, "stackoverflow", FASTEST_ALGS_WITHOUT_SAMPLE),
]

TESTS_TO_GENERATE = [Test(*test_data) for test_data in _TESTS_TO_GENERATE]
REAL_LIFE_TESTS = [Test(*test_data) for test_data in _REAL_LIFE_TESTS]
TESTS = TESTS_TO_GENERATE + REAL_LIFE_TESTS
