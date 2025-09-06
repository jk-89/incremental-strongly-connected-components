import random
from math import floor, ceil
from abc import ABC, abstractmethod


class Graph(ABC):
    """
    Allows to first create an empty graph and then generate its edges
    (in possibly randomized way).
    """

    def __init__(self, n, m, seed=None):
        self.n = n
        self.m = m
        self.seed = seed

    def _random_distinct_nodes_pair(self):
        assert self.n > 1
        while True:
            u = random.randint(0, self.n - 1)
            v = random.randint(0, self.n - 1)
            if u != v:
                return u, v

    @staticmethod
    def _shuffled_edges(edges):
        edges = list(sorted(edges))
        random.shuffle(edges)
        return edges

    @abstractmethod
    def _generate_edges(self):
        raise NotImplementedError

    def generate_edges(self, relabel_vertices=True):
        if self.seed is not None:
            random.seed(self.seed)
        edges = self._generate_edges()
        if not relabel_vertices:
            return edges

        # Randomly relabel vertices.
        perm = list(range(self.n))
        random.shuffle(perm)
        for i in range(len(edges)):
            u, v = edges[i]
            edges[i] = (perm[u], perm[v])
        return edges


class RandomGraph(Graph):
    """
    Random graph.
    """

    def _generate_edges(self):
        edges = set()
        while len(edges) < self.m:
            edges.add(self._random_distinct_nodes_pair())
        return self._shuffled_edges(edges)


class RandomDAG(Graph):
    """
    Random DAG.
    """

    def _generate_edges(self):
        edges = set()
        while len(edges) < self.m:
            u, v = self._random_distinct_nodes_pair()
            if u > v:
                u, v = v, u
            edges.add((u, v))
        return self._shuffled_edges(edges)


class RandomAlmostDAG(Graph):
    """
    Random DAG with a few additional edges (which can create cycles).
    """

    def __init__(self, n, dag_edges_no, additional_edges_no, seed=None):
        super().__init__(n, dag_edges_no + additional_edges_no, seed)
        self.dag = RandomDAG(n, dag_edges_no)

    def _generate_edges(self):
        edges = set(self.dag.generate_edges())

        while len(edges) < self.m:
            u, v = self._random_distinct_nodes_pair()
            if u < v:
                u, v = v, u
            edges.add((u, v))
        return self._shuffled_edges(edges)


class CliqueChain(Graph):
    """
    First creates (n / B) DAG-cliques of size B.
    Then create a big cycle from those cliques.
    """

    def __init__(self, n, clique_size, seed=None):
        cliques = n // clique_size
        m = cliques * clique_size * (clique_size - 1) // 2 + 2 * (cliques - 1)
        super().__init__(cliques * clique_size, m, seed)
        self.clique_size = clique_size

    def _get_one_node_per_clique(self):
        picked = []
        for start in range(0, self.n, self.clique_size):
            end = start + self.clique_size
            picked.append(random.randint(start, end - 1))
        return picked

    def _generate_edges(self):
        edges = []
        for start in range(0, self.n, self.clique_size):
            end = start + self.clique_size
            for u in range(start, end):
                for v in range(u + 1, end):
                    edges.append((u, v))
        random.shuffle(edges)

        # Connect cliques forward and backward.
        edges_connecting_cliques = []
        forward = self._get_one_node_per_clique()
        backward = self._get_one_node_per_clique()
        for vertices in [forward, backward[::-1]]:
            for i in range(len(vertices) - 1):
                u = vertices[i]
                v = vertices[i + 1]
                edges_connecting_cliques.append((u, v))
        random.shuffle(edges_connecting_cliques)

        assert self.m == len(edges) + len(edges_connecting_cliques)
        return edges + edges_connecting_cliques


class LayeredGraph(Graph):
    """
    Generates a graph with nodes placed in C columns and R rows.
    First each row is connected from left to right.
    Then some edges going from upper to lower layer are added.
    In the end a few edges going from lower to upper layer are added.
    """

    def __init__(
        self, rows_no, columns_no, downward_edges_no, upward_edges_no, seed=None
    ):
        super().__init__(
            rows_no * columns_no, downward_edges_no + upward_edges_no, seed
        )
        self.rows_no = rows_no
        self.columns_no = columns_no
        self.downward_edges_no = downward_edges_no
        self.upward_edges_no = upward_edges_no

    def _get_row(self, node):
        return node // self.columns_no

    def _get_node_number(self, row_no, column_no):
        return row_no * self.columns_no + column_no

    def _generate_edges_in_direction(self, edges_no, good_direction):
        edges = set()
        while len(edges) < edges_no:
            u, v = self._random_distinct_nodes_pair()
            if self._get_row(u) == self._get_row(v):
                continue
            if not good_direction(u, v):
                u, v = v, u
            edges.add((u, v))
        return self._shuffled_edges(edges)

    def _generate_edges(self):
        horizontal_edges = []
        for i in range(self.rows_no):
            for j in range(self.columns_no - 1):
                u = self._get_node_number(i, j)
                horizontal_edges.append((u, u + 1))
        random.shuffle(horizontal_edges)

        downward_edges = self._generate_edges_in_direction(
            self.downward_edges_no, lambda x, y: x < y
        )
        upward_edges = self._generate_edges_in_direction(
            self.upward_edges_no, lambda x, y: x > y
        )

        return horizontal_edges + downward_edges + upward_edges


class MixedDegrees(Graph):
    """
    Generates a graph in which some nodes have high degree and some nodes have low degree.
    """

    def __init__(self, n, m, heavy_nodes_no, heavy_proportion, seed=None):
        super().__init__(n, m, seed)
        self.heavy_nodes_no = heavy_nodes_no
        # Each heavy vertex has `heavy_proportion` more edges than a non-heavy
        # vertex (in expectancy).
        self.heavy_proportion = heavy_proportion

    def _generate_edges(self):
        vertices = list(range(self.n))
        weights = [self.heavy_proportion] * self.heavy_nodes_no + [1] * (
            self.n - self.heavy_nodes_no
        )

        # We select 3m edges because later we have to remove loops and multiple
        # edges. We assume that within 3m random choices there will be at least
        # m valid edges.
        edges_to_choose = 3 * self.m
        ins = random.choices(vertices, weights=weights, k=edges_to_choose)
        outs = random.choices(vertices, weights=weights, k=edges_to_choose)
        edges = set()
        for i, (u, v) in enumerate(zip(ins, outs)):
            if u != v:
                edges.add((u, v))
                if len(edges) == self.m:
                    break

        assert len(edges) == self.m
        return self._shuffled_edges(edges)


# The class of graphs described in https://arxiv.org/pdf/1105.2397 as an example
# where CompatibleSearch and SoftThresholdSearch perform a large number of operations.
class PathsCollection(Graph):
    def __init__(self, paths_no, path_length):
        self.paths_no = paths_no
        self.path_length = path_length
        n = paths_no * path_length
        m = n - paths_no + paths_no * (paths_no - 1) // 2
        super().__init__(n, m)

    def _first_on_path(self, path):
        return path * self.path_length

    def _last_on_path(self, path):
        return (path + 1) * self.path_length - 1

    def _generate_edges(self):
        edges = []
        # Edges within paths
        for path in range(self.paths_no):
            u = self._first_on_path(path)
            for i in range(self.path_length - 1):
                edges.append((u, u + 1))
                u += 1

        # Edges between paths.
        for path_1 in range(self.paths_no):
            for path_2 in range(path_1 + 1, self.paths_no):
                # Add an edge from the last vertex of path_2
                # to the first vertex of path_1.
                edges.append((self._last_on_path(path_2), self._first_on_path(path_1)))

        return edges


# The class of graphs described in https://dl.acm.org/doi/10.1145/2756553 as an example
# where TwoWaySearch perform a large number of operations.
class CliquesCollection(Graph):
    # Note that provided `n` and `m` might not be the final n and m
    # in the generated graph. Check the paper for details.
    def __init__(self, n, m):
        self.main_clique_size = floor(m ** (1 / 2) / 2)
        delta = min(n ** (2 / 3), m ** (1 / 2))
        self.anchor_clique_size = ceil(delta ** (1 / 2) + 1)
        self.edges = []
        self._init_edges(n, m)
        n = max(max(u, v) for u, v in self.edges) + 1
        super().__init__(n, len(self.edges))

    def _first_in_anchor(self, anchor):
        return self.main_clique_size + anchor * self.anchor_clique_size

    def _last_in_anchor(self, anchor):
        return self.main_clique_size + (anchor + 1) * self.anchor_clique_size - 1

    def _fill_clique_edges(self, start_vertex, end_vertex):
        for u in range(end_vertex - 1, start_vertex - 1, -1):
            for v in range(u + 1, end_vertex):
                self.edges.append((u, v))

    def _init_edges(self, n, m):
        # Edges within cliques.
        self._fill_clique_edges(0, self.main_clique_size)
        k = 0
        u = self._first_in_anchor(k)
        while u < n and len(self.edges) < m // 2:
            self._fill_clique_edges(u, u + self.anchor_clique_size)
            k += 1
            u = self._first_in_anchor(k)

        # Edges between anchor cliques.
        for anchor in range(k - 1):
            u = self._last_in_anchor(anchor)
            for i in range(self.anchor_clique_size, 0, -1):
                self.edges.append((u, u + i))

        # Edges to the main clique.
        for anchor in range(1, k):
            self.edges.append((self._first_in_anchor(anchor), 0))

    def _generate_edges(self):
        return self.edges
