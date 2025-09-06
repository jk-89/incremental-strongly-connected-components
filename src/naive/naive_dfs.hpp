#ifndef NAIVE_DFS_HPP
#define NAIVE_DFS_HPP

/// After addition of an edge (u, v), if u and v are in distinct SCCs,
/// check if u is reachable from v and correctly update SCCs.
/// O(m * (n + m)).

#include "utils/algorithm.hpp"

class NaiveDfs : public Algorithm {
   private:
    Graph reversed_graph;
    std::vector<size_t> visited;

    void preprocess_edge(VertexPtr u, VertexPtr v) override;
    void algorithm_step(VertexPtr u, VertexPtr v) override;
    std::vector<VertexPtr> dfs(const VertexPtr& source, Graph& g,
                               bool store_encountered);

   public:
    explicit NaiveDfs(size_t no_vertices)
        : Algorithm(no_vertices), reversed_graph(graph), visited(no_vertices) {}
};

#endif  // NAIVE_DFS_HPP
