#include "soft_threshold_search.hpp"

#include "utils/rng.hpp"

void IndexedList::insert(Vertex_id_t id) {
    if (!id_iterators[id].has_value()) {
        ids.push_back(id);
        id_iterators[id] = std::prev(ids.end());
    }
}

void IndexedList::remove(Vertex_id_t id) {
    if (id_iterators[id].has_value()) {
        ids.erase(*id_iterators[id]);
        id_iterators[id] = std::nullopt;
    }
}

IndexedList::Vertex_id_list::iterator IndexedList::remove(
    Vertex_id_list::iterator iter) {
    id_iterators[*iter] = std::nullopt;
    return ids.erase(iter);
}

void IndexedList::clear() {
    for (const auto &id : ids)
        id_iterators[id] = std::nullopt;
    ids.clear();
}

bool IndexedList::is_empty() const { return ids.empty(); }

Vertex_id_t IndexedList::front() const { return ids.front(); }

Vertex_id_t IndexedList::choose_random() {
    const auto size = static_cast<int>(ids.size());
    if (size == 0)
        throw std::runtime_error("Cannot choose random id - list is empty.");

    auto index = RNG::instance().randint(0, size - 1);
    auto iter = ids.begin();
    while (index--)
        ++iter;
    return *iter;
}

void SoftThresholdTraversal::insert_active_vertex(Vertex_id_t vertex_id) {
    active.insert(vertex_id);
}

void SoftThresholdTraversal::remove_active_vertex(Vertex_id_t vertex_id) {
    active.remove(vertex_id);
}

bool SoftThresholdTraversal::any_active() const { return !active.is_empty(); }

Vertex_id_t SoftThresholdTraversal::get_next_active() const {
    return active.front();
}

void SoftThresholdTraversal::move_from_active_to_passive(const VertexPtr &u) {
    active.remove(u->id);
    passive.insert(u->id);
}

void SoftThresholdTraversal::update_active_passive_and_threshold(
    const std::shared_ptr<SoftThresholdTraversal> &other,
    Vertex_id_t &threshold) {
    if (!active.is_empty())
        return;

    other->passive.clear();
    other->active.remove(threshold);
    if (passive.is_empty())
        return;

    threshold = passive.choose_random();
    auto &ids = passive.get_ids();
    for (auto iter = ids.begin(); iter != ids.end();) {
        if (should_move_from_passive_to_active(*iter, threshold)) {
            active.insert(*iter);
            iter = passive.remove(iter);
        } else {
            ++iter;
        }
    }
}

void SoftThresholdTraversal::clear() {
    Traversal::clear();
    active.clear();
    passive.clear();
}

bool SoftThresholdForwardTraversal::should_move_from_passive_to_active(
    Vertex_id_t candidate, Vertex_id_t threshold) {
    return candidate == threshold || order_comparator(candidate, threshold);
}

bool SoftThresholdBackwardTraversal::should_move_from_passive_to_active(
    Vertex_id_t candidate, Vertex_id_t threshold) {
    return candidate == threshold || order_comparator(threshold, candidate);
}

bool SoftThresholdForwardTraversal::on_the_good_side_of_pivot(
    Vertex_id_t candidate_id, Vertex_id_t pivot_id) {
    return order_comparator(candidate_id, pivot_id);
}

bool SoftThresholdBackwardTraversal::on_the_good_side_of_pivot(
    Vertex_id_t candidate_id, Vertex_id_t pivot_id) {
    return order_comparator(pivot_id, candidate_id);
}

void SoftThresholdSearch::perform_search_steps(const VertexPtr &u) {
    auto threshold = u->id;

    const auto forward =
        std::dynamic_pointer_cast<SoftThresholdForwardTraversal>(
            forward_traversal);
    const auto backward =
        std::dynamic_pointer_cast<SoftThresholdBackwardTraversal>(
            backward_traversal);

    while (forward->any_active() && backward->any_active()) {
        const auto forward_vertex_id = forward->get_next_active();
        const auto forward_vertex = graph.get_vertex_by_id(forward_vertex_id);
        const auto backward_vertex_id = backward->get_next_active();
        const auto backward_vertex =
            reversed_graph.get_vertex_by_id(backward_vertex_id);

        if (order->is_before(forward_vertex_id, backward_vertex_id)) {
            search_step(forward_vertex, backward_vertex);
        } else {
            // Paper misses the case when forward_id = threshold = backward_id.
            if (order->is_before(threshold, forward_vertex_id) ||
                (threshold == forward_vertex_id &&
                 threshold == backward_vertex_id))
                forward->move_from_active_to_passive(forward_vertex);
            if (order->is_before(backward_vertex_id, threshold))
                backward->move_from_active_to_passive(backward_vertex);
        }

        forward->update_active_passive_and_threshold(backward, threshold);
        backward->update_active_passive_and_threshold(forward, threshold);
    }
}
