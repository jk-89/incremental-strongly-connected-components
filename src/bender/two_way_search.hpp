#ifndef TWO_WAY_SEARCH_HPP
#define TWO_WAY_SEARCH_HPP

/// Implements TwoWaySearch from https://doi.org/10.1145/2756553.
/// Works in total time O(m * min(n^{2/3}, m^{1/2})).

#include <cmath>

#include "utils/algorithm.hpp"

class TwoWaySearch : public Algorithm {
   private:
    size_t no_edges = 0;
    size_t no_traversal_steps = 0;
    size_t threshold = 1;
    size_t vertices_threshold;
    size_t edges_used_backwards{};
    bool found_cycle{};
    Graph reversed_graph;
    std::vector<size_t> visited;
    std::vector<size_t> level;
    std::vector<size_t> considered_during_traversal;
    std::vector<VertexPtr> component;
    std::vector<size_t> marked_within_component;

    void update_threshold();

    void search_backward(const VertexPtr& u, const VertexPtr& v);
    void search_forward(const VertexPtr& u);

    void form_component(const VertexPtr& u, const VertexPtr& v);
    void form_component_dfs(const VertexPtr& u);

    void preprocess_edge(VertexPtr, VertexPtr) override;
    void postprocess_edge(VertexPtr u, VertexPtr v) override;
    void algorithm_step(VertexPtr u, VertexPtr v) override;

   public:
    explicit TwoWaySearch(size_t no_vertices)
        : Algorithm(no_vertices),
          reversed_graph(graph),
          visited(no_vertices),
          level(no_vertices),
          considered_during_traversal(no_vertices),
          component(no_vertices),
          marked_within_component(no_vertices) {
        const auto cbrt_no_vertices = cbrt(static_cast<double>(no_vertices));
        vertices_threshold =
            static_cast<size_t>(cbrt_no_vertices * cbrt_no_vertices);
    }
};

#endif  // TWO_WAY_SEARCH_HPP
