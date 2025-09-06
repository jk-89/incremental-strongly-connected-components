import math
import random
from abc import ABC, abstractmethod

from tests.common.graph import (
    RandomGraph,
    RandomDAG,
    RandomAlmostDAG,
    CliqueChain,
    LayeredGraph,
)


class GraphGenerator(ABC):
    """
    Generate random graphs of a given type with some logic regarding
    the maximum and minimum number of vertices and edges.
    """

    @abstractmethod
    def _generate_graph(self):
        raise NotImplementedError

    def generate_edges(self):
        graph = self._generate_graph()
        return graph.generate_edges()


class RandomGraphGenerator(GraphGenerator):
    def __init__(self, min_n, max_n, min_m=None, max_m=None):
        self.min_n = min_n
        self.max_n = max_n
        self.min_m = min_m
        self.max_m = max_m

    def get_m(self, n):
        return random.randint(self.min_m, min(n * (n - 1), self.max_m))

    def _generate_graph(self):
        n = random.randint(self.min_n, self.max_n)
        m = self.get_m(n)
        return RandomGraph(n, m)


class RandomTinyGraphGenerator(RandomGraphGenerator):
    def get_m(self, n):
        return random.randint(n // 2, n * (n - 1))


class RandomSparseGraphGenerator(RandomGraphGenerator):
    def __init__(self, min_n, max_n, min_sparsity_multiplier, max_sparsity_multiplier):
        super().__init__(min_n, max_n)
        self.min_sparsity_multiplier = min_sparsity_multiplier
        self.max_sparsity_multiplier = max_sparsity_multiplier

    def get_m(self, n):
        sparsity_multiplier = random.uniform(
            self.min_sparsity_multiplier, self.max_sparsity_multiplier
        )
        return random.randint(n, math.floor(sparsity_multiplier * n))


class RandomDAGGenerator(GraphGenerator):
    def __init__(self, min_n, max_n, min_m, max_m):
        self.min_n = min_n
        self.max_n = max_n
        self.min_m = min_m
        self.max_m = max_m

    def _generate_graph(self):
        n = random.randint(self.min_n, self.max_n)
        m = random.randint(self.min_m, min(n * (n - 1) // 2, self.max_m))
        return RandomDAG(n, m)


class RandomAlmostDAGGenerator(GraphGenerator):
    def __init__(self, min_n, max_n, min_m, max_m):
        self.min_n = min_n
        self.max_n = max_n
        self.min_m = min_m
        self.max_m = max_m

    def _generate_graph(self):
        n = random.randint(self.min_n, self.max_n)
        dag_edges_no = random.randint(self.min_m, min(n * (n - 1) // 2, self.max_m))
        additional_edges_no = random.randint(1, n // 5)
        return RandomAlmostDAG(n, dag_edges_no, additional_edges_no)


class CliqueChainGenerator(GraphGenerator):
    def __init__(self, min_n, max_n, min_clique_size, max_clique_size):
        self.min_n = min_n
        self.max_n = max_n
        self.min_clique_size = min_clique_size
        self.max_clique_size = max_clique_size

    def _generate_graph(self):
        n = random.randint(self.min_n, self.max_n)
        clique_size = random.randint(
            max(2, self.min_clique_size), min(n // 2, self.max_clique_size)
        )
        return CliqueChain(n, clique_size)


class LayeredGraphGenerator(GraphGenerator):
    def __init__(
        self,
        min_n,
        max_n,
        min_downward_edges_no,
        max_downward_edges_no,
        min_upward_edges_no,
        max_upward_edges_no,
    ):
        self.min_n = min_n
        self.max_n = max_n
        self.min_downward_edges_no = min_downward_edges_no
        self.max_downward_edges_no = max_downward_edges_no
        self.min_upward_edges_no = min_upward_edges_no
        self.max_upward_edges_no = max_upward_edges_no

    @staticmethod
    def _max_edges_in_one_direction(rows_no, columns_no):
        n = rows_no * columns_no
        return n * (n - columns_no) // 2

    def _generate_graph(self):
        n = random.randint(self.min_n, self.max_n)
        smaller_dim = random.randint(2, math.floor(math.sqrt(n)))
        bigger_dim = n // smaller_dim
        rows_no, columns_no = smaller_dim, bigger_dim
        if random.choice([False, True]):
            rows_no, columns_no = columns_no, rows_no

        max_edges = self._max_edges_in_one_direction(rows_no, columns_no)
        downward_edges_no = min(
            max_edges,
            random.randint(self.min_downward_edges_no, self.max_downward_edges_no),
        )
        upward_edges_no = min(
            max_edges,
            random.randint(self.min_upward_edges_no, self.max_upward_edges_no),
        )
        return LayeredGraph(rows_no, columns_no, downward_edges_no, upward_edges_no)
