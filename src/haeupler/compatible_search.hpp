#ifndef COMPATIBLE_SEARCH_HPP
#define COMPATIBLE_SEARCH_HPP

/// Implements Compatible Search from https://doi.org/10.1145/2071379.2071382.
/// Works in total time O(m^{3/2} * log(n)).

#include <set>

#include "haeupler_search.hpp"
#include "utils/dynamic_order.hpp"

// Since forward and backward steps are analogous we introduce a common class
// that catches this common behaviour.
class CompatibleTraversal : public Traversal {
   protected:
    std::set<Vertex_id_t, DynamicOrderComparator> live;

    void insert_active_vertex(Vertex_id_t vertex_id) override;
    void remove_active_vertex(Vertex_id_t vertex_id) override;

   public:
    CompatibleTraversal(size_t no_vertices, const DynamicOrderPtr &order)
        : Traversal(no_vertices, order), live(order_comparator) {}
    ~CompatibleTraversal() override = default;

    virtual std::optional<Vertex_id_t> get_best_live_option() = 0;

    void clear() override;
};

class CompatibleForwardTraversal : public CompatibleTraversal {
   protected:
    bool on_the_good_side_of_pivot(Vertex_id_t candidate_id,
                                   Vertex_id_t pivot_id) override;

   public:
    CompatibleForwardTraversal(size_t no_vertices, const DynamicOrderPtr &order)
        : CompatibleTraversal(no_vertices, order) {}

    std::optional<Vertex_id_t> get_best_live_option() override;
};

class CompatibleBackwardTraversal : public CompatibleTraversal {
   protected:
    bool on_the_good_side_of_pivot(Vertex_id_t candidate_id,
                                   Vertex_id_t pivot_id) override;

   public:
    CompatibleBackwardTraversal(size_t no_vertices,
                                const DynamicOrderPtr &order)
        : CompatibleTraversal(no_vertices, order) {}

    std::optional<Vertex_id_t> get_best_live_option() override;
};

class CompatibleSearch : public HaeuplerSearch {
   private:
    void perform_search_steps(const VertexPtr &u) override;

   public:
    CompatibleSearch(size_t no_vertices, const DynamicOrderPtr &order)
        : HaeuplerSearch(
              no_vertices, order,
              std::make_shared<CompatibleForwardTraversal>(no_vertices, order),
              std::make_shared<CompatibleBackwardTraversal>(no_vertices,
                                                            order)) {}
};

#endif  // COMPATIBLE_SEARCH_HPP
