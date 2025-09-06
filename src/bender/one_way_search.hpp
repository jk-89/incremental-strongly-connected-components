#ifndef ONE_WAY_SEARCH_HPP
#define ONE_WAY_SEARCH_HPP

/// Implements OneWaySearch from https://doi.org/10.1145/2756553.
/// Works in total time O(n^2 * log^2(n)).

#include <set>
#include <unordered_map>

#include "utils/algorithm.hpp"

using Level_t = size_t;

// Simulates heap called `out(x)` in the paper.
class Heap {
   private:
    using Out_vertices_t = std::set<std::pair<Level_t, Vertex_id_t>>;

    Out_vertices_t out;
    // For each neighbour v stores a pointer to v within out.
    std::unordered_map<Vertex_id_t, Out_vertices_t::iterator> pointers;

   public:
    const Out_vertices_t& get_out() const { return out; }

    void insert(Vertex_id_t vertex_id, Level_t level);
    void erase(Vertex_id_t vertex_id);

    Out_vertices_t::iterator begin();
    void erase_begin();

    bool empty() const;
};

// Simulates BST called `IN(x)` / `OUT(x)` in the paper.
class BST {
   private:
    using Vertex_ids_set = std::set<Vertex_id_t>;
    Vertex_ids_set bst;

   public:
    const Vertex_ids_set& get_bst() const { return bst; }

    void insert(Vertex_id_t vertex_id);
    void erase(Vertex_id_t vertex_id);

    bool contains(Vertex_id_t vertex_id) const;

    size_t size() const;
};

class OneWaySearch : public Algorithm {
   private:
    std::vector<Level_t> level;
    std::vector<std::vector<size_t>> bound, count;
    std::vector<BST> bst_in, bst_out;
    std::vector<Heap> heap;
    std::vector<VertexPtr> component;
    std::vector<size_t> marked_within_component;

    static size_t log_2_floor(size_t x);

    void find_component_dfs(Vertex_id_t current_id, Vertex_id_t u_id);
    void find_component(const VertexPtr& u, const VertexPtr& v);

    void insert_edge(const VertexPtr& u, const VertexPtr& v);
    void erase_edge_if_exists(Vertex_id_t u_id, Vertex_id_t v_id);

    void move_from_heap_to_candidates(Vertex_id_t u_id,
                                      std::vector<Raw_edge_t>& candidate_edges);

    void merge_into_component(const std::vector<VertexPtr>& vertices);
    // Returns the vector of candidates needed for traversal_step.
    std::vector<Raw_edge_t> form_component_and_fill_candidates(
        const VertexPtr& u, const VertexPtr& v);

    void traversal_step(std::vector<Raw_edge_t>& candidate_edges);
    void algorithm_step(VertexPtr u, VertexPtr v) override;

   public:
    explicit OneWaySearch(size_t no_vertices)
        : Algorithm(no_vertices),
          level(no_vertices, 1),
          bst_in(no_vertices),
          bst_out(no_vertices),
          heap(no_vertices),
          marked_within_component(no_vertices) {
        const auto spans_no = log_2_floor(no_vertices) + 1;
        bound.resize(spans_no, std::vector<size_t>(no_vertices, 1));
        count.resize(spans_no, std::vector<size_t>(no_vertices));
    }
};

#endif  // ONE_WAY_SEARCH_HPP
