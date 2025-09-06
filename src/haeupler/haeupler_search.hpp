#ifndef HAEUPLER_SEARCH_HPP
#define HAEUPLER_SEARCH_HPP

/// Extracts common behaviour of SoftThresholdSearch and CompatibleSearch from
/// https://doi.org/10.1145/2071379.2071382.

#include <utility>

#include "utils/algorithm.hpp"
#include "utils/dynamic_order.hpp"

// Since forward and backward steps are similar we introduce a common class
// that catch this common behaviour.
class Traversal {
   protected:
    std::vector<VertexPtr> traversed;
    std::vector<size_t> visited;
    std::vector<Vertex_list::iterator> next_neighbour;
    DynamicOrderComparator order_comparator;

    virtual bool on_the_good_side_of_pivot(Vertex_id_t candidate_id,
                                           Vertex_id_t pivot_id) = 0;

    virtual void insert_active_vertex(Vertex_id_t vertex_id) = 0;
    virtual void remove_active_vertex(Vertex_id_t vertex_id) = 0;

   public:
    Traversal(size_t no_vertices, const DynamicOrderPtr &order)
        : visited(no_vertices),
          next_neighbour(no_vertices),
          order_comparator(order) {}
    virtual ~Traversal() = default;

    void insert_vertex(const VertexPtr &u, Graph &graph, size_t no_traversals);

    Vertex_list::iterator get_next_neighbour_iterator(const VertexPtr &u,
                                                      Graph &graph);

    VertexPtr find_pivot(const VertexPtr &initial_pivot, Graph &graph) const;
    std::vector<Vertex_id_t> get_sorted_vertices_based_on_pivot(
        Vertex_id_t pivot_id);

    virtual void clear();
};

class HaeuplerSearch : public Algorithm {
   protected:
    Graph reversed_graph;
    DynamicOrderPtr order;
    std::shared_ptr<Traversal> forward_traversal;
    std::shared_ptr<Traversal> backward_traversal;
    // Used to determine newly created strongly connected components.
    Graph scc_detector;
    std::vector<VertexPtr> within_scc_detector;
    std::vector<Vertex_id_t> visited;
    std::vector<Vertex_id_t> is_in_new_scc;
    std::vector<VertexPtr> new_scc;

    void restore_topological_order(const VertexPtr &default_pivot);

    void find_new_connected_component(const VertexPtr &current,
                                      const VertexPtr &u);

    void search_step(const VertexPtr &u, const VertexPtr &v);
    virtual void perform_search_steps(const VertexPtr &u) = 0;

    void algorithm_step(VertexPtr u, VertexPtr v) override;
    void postprocess_edge(VertexPtr u, VertexPtr v) override;

    void clear();

   public:
    HaeuplerSearch(size_t no_vertices, DynamicOrderPtr order,
                   std::shared_ptr<Traversal> forward_traversal,
                   std::shared_ptr<Traversal> backward_traversal)
        : Algorithm(no_vertices),
          reversed_graph(graph),
          order(std::move(order)),
          forward_traversal(std::move(forward_traversal)),
          backward_traversal(std::move(backward_traversal)),
          scc_detector(graph),
          visited(no_vertices),
          is_in_new_scc(no_vertices) {}
};

#endif  // HAEUPLER_SEARCH_HPP
