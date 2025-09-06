#ifndef NAIVE_ONE_WAY_SEARCH_HPP
#define NAIVE_ONE_WAY_SEARCH_HPP

/// Implements naive version of OneWaySearch from
/// https://doi.org/10.1145/2756553.
/// Works in total time O(mn).

#include <unordered_map>

#include "utils/algorithm.hpp"
#include "utils/hash.hpp"

class NaiveOneWaySearch : public Algorithm {
   private:
    size_t traversal_steps_no = 0;
    std::vector<size_t> visited;
    std::vector<size_t> level;
    std::vector<size_t> reaches_target;
    std::vector<VertexPtr> reached_target;
    std::unordered_map<std::pair<Vertex_id_t, Vertex_id_t>, size_t, PairHash>
        visited_edge;
    constexpr static size_t MAX_VISITED_EDGES_SIZE = 15000;

    void detect_new_scc(const VertexPtr& current, const VertexPtr& target);
    void update_levels(const VertexPtr& current);

    void postprocess_edge(VertexPtr u, VertexPtr v) override;
    void algorithm_step(VertexPtr u, VertexPtr v) override;

   public:
    explicit NaiveOneWaySearch(size_t no_vertices)
        : Algorithm(no_vertices),
          visited(no_vertices),
          level(no_vertices, 1),
          reaches_target(no_vertices) {}
};

#endif  // NAIVE_ONE_WAY_SEARCH_HPP
