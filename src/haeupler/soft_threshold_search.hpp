#ifndef SOFT_THRESHOLD_SEARCH_HPP
#define SOFT_THRESHOLD_SEARCH_HPP

/// Implements Soft-Threshold Search from
/// https://doi.org/10.1145/2071379.2071382.
/// Works in total time O(m^{3/2}).

#include <set>

#include "haeupler_search.hpp"
#include "utils/dynamic_order.hpp"

// Special list that allows to manage active and passive vertices.
class IndexedList {
   private:
    using Vertex_id_list = std::list<Vertex_id_t>;
    Vertex_id_list ids;
    std::vector<std::optional<Vertex_id_list::iterator>> id_iterators;

   public:
    explicit IndexedList(size_t no_vertices) : id_iterators(no_vertices) {}

    Vertex_id_list &get_ids() { return ids; }

    // Does nothing if element already exists in the list.
    void insert(Vertex_id_t id);
    // Does nothing if element doesn't exist in the list.
    void remove(Vertex_id_t id);
    Vertex_id_list::iterator remove(Vertex_id_list::iterator iter);

    void clear();

    bool is_empty() const;

    Vertex_id_t front() const;

    Vertex_id_t choose_random();
};

class SoftThresholdTraversal : public Traversal {
   protected:
    IndexedList active, passive;

    virtual bool should_move_from_passive_to_active(Vertex_id_t candidate,
                                                    Vertex_id_t threshold) = 0;

    void insert_active_vertex(Vertex_id_t vertex_id) override;
    void remove_active_vertex(Vertex_id_t vertex_id) override;

   public:
    SoftThresholdTraversal(size_t no_vertices, const DynamicOrderPtr &order)
        : Traversal(no_vertices, order),
          active(no_vertices),
          passive(no_vertices) {}
    ~SoftThresholdTraversal() override = default;

    bool any_active() const;
    Vertex_id_t get_next_active() const;

    void move_from_active_to_passive(const VertexPtr &u);

    void update_active_passive_and_threshold(
        const std::shared_ptr<SoftThresholdTraversal> &other,
        Vertex_id_t &threshold);

    void clear() override;
};

class SoftThresholdForwardTraversal : public SoftThresholdTraversal {
   protected:
    bool should_move_from_passive_to_active(Vertex_id_t candidate,
                                            Vertex_id_t threshold) override;

    bool on_the_good_side_of_pivot(Vertex_id_t candidate_id,
                                   Vertex_id_t pivot_id) override;

   public:
    SoftThresholdForwardTraversal(size_t no_vertices,
                                  const DynamicOrderPtr &order)
        : SoftThresholdTraversal(no_vertices, order) {}
};

class SoftThresholdBackwardTraversal : public SoftThresholdTraversal {
   protected:
    bool should_move_from_passive_to_active(Vertex_id_t candidate,
                                            Vertex_id_t threshold) override;

    bool on_the_good_side_of_pivot(Vertex_id_t candidate_id,
                                   Vertex_id_t pivot_id) override;

   public:
    SoftThresholdBackwardTraversal(size_t no_vertices,
                                   const DynamicOrderPtr &order)
        : SoftThresholdTraversal(no_vertices, order) {}
};

class SoftThresholdSearch : public HaeuplerSearch {
   private:
    void perform_search_steps(const VertexPtr &u) override;

   public:
    SoftThresholdSearch(size_t no_vertices, const DynamicOrderPtr &order)
        : HaeuplerSearch(no_vertices, order,
                         std::make_shared<SoftThresholdForwardTraversal>(
                             no_vertices, order),
                         std::make_shared<SoftThresholdBackwardTraversal>(
                             no_vertices, order)) {}
};

#endif  // SOFT_THRESHOLD_SEARCH_HPP
