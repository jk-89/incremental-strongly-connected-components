import random
import sys
from tests.common.graph_generator import (
    RandomGraphGenerator,
    RandomTinyGraphGenerator,
    RandomSparseGraphGenerator,
    RandomDAGGenerator,
    RandomAlmostDAGGenerator,
    CliqueChainGenerator,
    LayeredGraphGenerator,
)
from tests.common.common import write_case_in, write_case_out
from tests.common.test_config import GROUND_TRUTH, CORRECTNESS_TEST_DIR

MIN_NODES = 30
MAX_NODES = 200
MIN_EDGES = 20
MAX_EDGES = 1500

MIN_NODES_TINY = 4
MAX_NODES_TINY = 19

NUM_TINY_TESTS = 300
NUM_BASIC_TESTS = 400
NUM_BASIC_SPARSE_TESTS = 800
NUM_DAG_TESTS = 400
NUM_ALMOST_DAG_TESTS = 400
NUM_CLIQUE_CHAIN_TESTS = 500
NUM_LAYERED_GRAPH_TESTS = 500


def main():
    random.seed(123)
    tiny_random_gen = RandomTinyGraphGenerator(MIN_NODES_TINY, MAX_NODES_TINY)
    random_gen = RandomGraphGenerator(MIN_NODES, MAX_NODES, MIN_EDGES, MAX_EDGES)
    random_sparse_gen = RandomSparseGraphGenerator(MIN_NODES, MAX_NODES, 1.5, 3)
    dag_gen = RandomDAGGenerator(MIN_NODES, MAX_NODES, MIN_EDGES, MAX_EDGES)
    almost_dag_gen = RandomAlmostDAGGenerator(
        MIN_NODES, MAX_NODES, MIN_EDGES, MAX_EDGES
    )
    clique_chain_gen = CliqueChainGenerator(MIN_NODES, MAX_NODES, 2, MAX_NODES)
    layered_graph_gen = LayeredGraphGenerator(
        MIN_NODES, MAX_NODES, MIN_NODES, MAX_EDGES, 0, MAX_EDGES
    )
    test_id = 0

    test_groups = [
        (tiny_random_gen, NUM_TINY_TESTS),
        (random_gen, NUM_BASIC_TESTS),
        (random_sparse_gen, NUM_BASIC_SPARSE_TESTS),
        (dag_gen, NUM_DAG_TESTS),
        (almost_dag_gen, NUM_ALMOST_DAG_TESTS),
        (clique_chain_gen, NUM_CLIQUE_CHAIN_TESTS),
        (layered_graph_gen, NUM_LAYERED_GRAPH_TESTS),
    ]

    total_tests = sum(tests_no for _, tests_no in test_groups)
    for gen, tests_no in test_groups:
        for _ in range(tests_no):
            edges = gen.generate_edges()
            write_case_in(test_id, edges, CORRECTNESS_TEST_DIR)
            write_case_out(test_id, CORRECTNESS_TEST_DIR, GROUND_TRUTH)
            test_id += 1
            sys.stdout.write(f"\rProgress: {test_id}/{total_tests} cases generated.")
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
