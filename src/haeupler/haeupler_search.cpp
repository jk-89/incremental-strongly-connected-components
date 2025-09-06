#include "haeupler_search.hpp"

#include <algorithm>

void Traversal::insert_vertex(const VertexPtr &u, Graph &graph,
                              size_t no_traversals) {
    if (visited[u->id] == no_traversals)
        return;

    traversed.push_back(u);
    visited[u->id] = no_traversals;
    next_neighbour[u->id] = graph.get_neighbours_begin(u);
    if (next_neighbour[u->id] != graph.get_neighbours_end(u))
        insert_active_vertex(u->id);
}

Vertex_list::iterator Traversal::get_next_neighbour_iterator(const VertexPtr &u,
                                                             Graph &graph) {
    const auto neighbour = next_neighbour[u->id];
    next_neighbour[u->id] = std::next(next_neighbour[u->id]);
    if (next_neighbour[u->id] == graph.get_neighbours_end(u))
        remove_active_vertex(u->id);
    return neighbour;
}

VertexPtr Traversal::find_pivot(const VertexPtr &initial_pivot,
                                Graph &graph) const {
    auto pivot = initial_pivot;
    for (auto &vertex : traversed) {
        if (next_neighbour[vertex->id] != graph.get_neighbours_end(vertex) &&
            order_comparator(vertex->id, pivot->id))
            pivot = vertex;
    }
    return pivot;
}

std::vector<Vertex_id_t> Traversal::get_sorted_vertices_based_on_pivot(
    Vertex_id_t pivot_id) {
    std::vector<Vertex_id_t> vertex_ids;
    for (const auto &vertex : traversed) {
        if (on_the_good_side_of_pivot(vertex->id, pivot_id))
            vertex_ids.push_back(vertex->id);
    }
    std::sort(vertex_ids.begin(), vertex_ids.end(), order_comparator);
    return vertex_ids;
}

void Traversal::clear() { traversed.clear(); }

void HaeuplerSearch::search_step(const VertexPtr &u, const VertexPtr &v) {
    const auto x_iter =
        forward_traversal->get_next_neighbour_iterator(u, graph);
    const auto x = find_representative_vertex(*x_iter);
    if (x == u) {
        graph.erase_neighbour(u, x_iter);
    } else {
        scc_detector.add_edge(u, x);
        within_scc_detector.push_back(u);
        forward_traversal->insert_vertex(x, graph, no_traversals);
    }

    const auto y_iter =
        backward_traversal->get_next_neighbour_iterator(v, reversed_graph);
    const auto y = find_representative_vertex(*y_iter);
    if (y == v) {
        reversed_graph.erase_neighbour(v, y_iter);
    } else {
        scc_detector.add_edge(y, v);
        within_scc_detector.push_back(y);
        backward_traversal->insert_vertex(y, reversed_graph, no_traversals);
    }
}

void HaeuplerSearch::restore_topological_order(const VertexPtr &default_pivot) {
    const auto pivot = forward_traversal->find_pivot(default_pivot, graph);
    // We restore topological ordering using a simple sort.
    const auto &sorted_before_pivot =
        forward_traversal->get_sorted_vertices_based_on_pivot(pivot->id);
    const auto &sorted_after_pivot =
        backward_traversal->get_sorted_vertices_based_on_pivot(pivot->id);

    if (pivot == default_pivot) {
        auto previous_id = pivot->id;
        for (const auto &vertex_id : sorted_before_pivot) {
            order->remove(vertex_id);
            order->insert_after(vertex_id, previous_id);
            previous_id = vertex_id;
        }
    } else {
        auto next_id = pivot->id;
        for (auto vertex_iter = sorted_before_pivot.rbegin();
             vertex_iter != sorted_before_pivot.rend(); ++vertex_iter) {
            order->remove(*vertex_iter);
            order->insert_before(*vertex_iter, next_id);
            next_id = *vertex_iter;
        }
        for (auto vertex_iter = sorted_after_pivot.rbegin();
             vertex_iter != sorted_after_pivot.rend(); ++vertex_iter) {
            order->remove(*vertex_iter);
            order->insert_before(*vertex_iter, next_id);
            next_id = *vertex_iter;
        }
    }
}

void HaeuplerSearch::find_new_connected_component(const VertexPtr &current,
                                                  const VertexPtr &u) {
    visited[current->id] = no_traversals;

    for (auto neighbour_iter = scc_detector.get_neighbours_begin(current);
         neighbour_iter != scc_detector.get_neighbours_end(current);
         ++neighbour_iter) {
        const auto &neighbour = *neighbour_iter;
        if (visited[neighbour->id] != no_traversals)
            find_new_connected_component(neighbour, u);
        if (is_in_new_scc[neighbour->id] == no_traversals)
            is_in_new_scc[current->id] = no_traversals;
    }

    if (current == u)
        is_in_new_scc[current->id] = no_traversals;
    if (is_in_new_scc[current->id] == no_traversals)
        new_scc.emplace_back(current);
}

void HaeuplerSearch::clear() {
    for (const auto &vertex : within_scc_detector)
        scc_detector.clean_vertex(vertex);
    within_scc_detector.clear();
    new_scc.clear();
    forward_traversal->clear();
    backward_traversal->clear();
}

void HaeuplerSearch::algorithm_step(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u == v || order->is_before(u->id, v->id))
        return;

    no_traversals++;
    forward_traversal->insert_vertex(v, graph, no_traversals);
    backward_traversal->insert_vertex(u, reversed_graph, no_traversals);

    perform_search_steps(u);

    restore_topological_order(u);
    find_new_connected_component(v, u);
    merge_into_component(new_scc, {&graph, &reversed_graph});

    const auto component_representant = find_representative_vertex(u);
    if (component_representant != u) {
        order->remove(component_representant->id);
        order->insert_after(component_representant->id, u->id);
    }

    for (const auto &vertex : new_scc) {
        if (vertex->id != component_representant->id)
            order->remove(vertex->id);
    }

    clear();
}

void HaeuplerSearch::postprocess_edge(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u != v) {
        graph.add_edge(u, v);
        reversed_graph.add_edge(v, u);
    }
}
