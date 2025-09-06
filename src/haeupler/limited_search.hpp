#ifndef LIMITED_SEARCH_HPP
#define LIMITED_SEARCH_HPP

/// Implements Limited Search from https://doi.org/10.1145/2071379.2071382
/// adjusted for maintaining strongly connected components.
/// Works in total time O(m * n).

#include <unordered_map>

#include "utils/algorithm.hpp"
#include "utils/dynamic_order.hpp"
#include "utils/hash.hpp"

class LimitedSearch : public Algorithm {
   private:
    size_t dummy_id;
    DynamicOrderTreap order;
    std::vector<size_t> visited;
    std::vector<size_t> reaches_target;
    std::vector<VertexPtr> reached_target;
    std::vector<VertexPtr> postorder;
    std::unordered_map<std::pair<Vertex_id_t, Vertex_id_t>, size_t, PairHash>
        visited_edge;
    constexpr static size_t MAX_VISITED_EDGES_SIZE = 15000;

    void postprocess_edge(VertexPtr u, VertexPtr v) override;
    void algorithm_step(VertexPtr u, VertexPtr v) override;

    void dfs(const VertexPtr& current, const VertexPtr& target);

    void process_new_scc(const VertexPtr& target);

   public:
    explicit LimitedSearch(size_t no_vertices)
        : Algorithm(no_vertices),
          dummy_id(no_vertices),
          order(no_vertices + 1),
          visited(no_vertices),
          reaches_target(no_vertices) {
        order.remove(no_vertices);
    }
};

#endif  // LIMITED_SEARCH_HPP
