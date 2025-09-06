#include "compatible_search.hpp"

void CompatibleTraversal::insert_active_vertex(Vertex_id_t vertex_id) {
    live.insert(vertex_id);
}

void CompatibleTraversal::remove_active_vertex(Vertex_id_t vertex_id) {
    live.erase(vertex_id);
}

void CompatibleTraversal::clear() {
    Traversal::clear();
    live.clear();
}

bool CompatibleForwardTraversal::on_the_good_side_of_pivot(
    Vertex_id_t candidate_id, Vertex_id_t pivot_id) {
    return order_comparator(candidate_id, pivot_id);
}

std::optional<Vertex_id_t> CompatibleForwardTraversal::get_best_live_option() {
    if (live.empty())
        return std::nullopt;
    return *live.begin();
}

bool CompatibleBackwardTraversal::on_the_good_side_of_pivot(
    Vertex_id_t candidate_id, Vertex_id_t pivot_id) {
    return order_comparator(pivot_id, candidate_id);
}

std::optional<Vertex_id_t> CompatibleBackwardTraversal::get_best_live_option() {
    if (live.empty())
        return std::nullopt;
    return *(--live.end());
}

void CompatibleSearch::perform_search_steps(const VertexPtr &) {
    const auto forward = std::dynamic_pointer_cast<CompatibleForwardTraversal>(
        forward_traversal);
    const auto backward =
        std::dynamic_pointer_cast<CompatibleBackwardTraversal>(
            backward_traversal);

    auto forward_vertex = forward->get_best_live_option();
    auto backward_vertex = backward->get_best_live_option();
    while (forward_vertex.has_value() && backward_vertex.has_value()) {
        if (!order->is_before(*forward_vertex, *backward_vertex))
            break;

        search_step(graph.get_vertex_by_id(*forward_vertex),
                    graph.get_vertex_by_id(*backward_vertex));

        forward_vertex = forward->get_best_live_option();
        backward_vertex = backward->get_best_live_option();
    }
}
