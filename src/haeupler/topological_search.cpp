#include "topological_search.hpp"

std::vector<size_t> TopologicalTraversal::positions;
std::vector<VertexPtr> TopologicalTraversal::vertices;
size_t TopologicalTraversal::canonical_vertices_no;

void TopologicalTraversal::init_queue(const VertexPtr& u) {
    queue.push_back(u);
    current_index = positions[u->id];
    vertices[current_index] = nullptr;
}

void TopologicalTraversal::push_vertex_at_current_index_to_queue() {
    queue.push_back(vertices[current_index]);
    vertices[current_index] = nullptr;
}

bool TopologicalTraversal::is_before(const VertexPtr& u, const VertexPtr& v) {
    return positions[u->id] < positions[v->id];
}

size_t TopologicalTraversal::get_position(const VertexPtr& u) {
    return positions[u->id];
}

bool TopologicalTraversal::topological_search_step(
    size_t other_traversal_index) {
    update_current_index();
    while (correct_order_of_indices(other_traversal_index)) {
        bool edge_exists = false;
        for (const auto& v : queue) {
            if (edge(v->id, vertices[current_index]->id)) {
                edge_exists = true;
                break;
            }
        }
        if (edge_exists)
            break;
        update_current_index();
    }

    if (!correct_order_of_indices(other_traversal_index))
        return true;

    push_vertex_at_current_index_to_queue();
    return false;
}

void TopologicalTraversal::reorder() {
    while (!queue.empty()) {
        if (vertices[current_index]) {
            for (const auto& u : queue) {
                if (edge(u->id, vertices[current_index]->id)) {
                    push_vertex_at_current_index_to_queue();
                    break;
                }
            }
        }

        if (!vertices[current_index]) {
            const auto u = queue.front();
            queue.pop_front();
            vertices[current_index] = u;
            positions[u->id] = current_index;
        }

        update_current_index();
    }
}

void TopologicalTraversal::adjust_positions_with_new_scc(
    const VertexPtr& new_repr, size_t new_position_in_scc,
    const std::vector<VertexPtr>& new_scc) {
    auto min_free_position = positions[new_scc[0]->id];
    for (const auto& u : new_scc) {
        vertices[positions[u->id]] = nullptr;
        min_free_position = std::min(min_free_position, positions[u->id]);
    }
    vertices[new_position_in_scc] = new_repr;
    positions[new_repr->id] = new_position_in_scc;

    for (auto i = min_free_position; i < canonical_vertices_no; i++) {
        if (vertices[i]) {
            vertices[min_free_position] = vertices[i];
            positions[vertices[i]->id] = min_free_position;
            min_free_position++;
        }
    }

    canonical_vertices_no -= new_scc.size() - 1;
}

void TopologicalForwardTraversal::update_current_index() { ++current_index; }

bool TopologicalForwardTraversal::correct_order_of_indices(
    size_t other_traversal_index) {
    return current_index < other_traversal_index;
}

bool TopologicalForwardTraversal::edge(Vertex_id_t u_id, Vertex_id_t v_id) {
    return incidence_matrix[u_id][v_id];
}

void TopologicalBackwardTraversal::update_current_index() { --current_index; }

bool TopologicalBackwardTraversal::correct_order_of_indices(
    size_t other_traversal_index) {
    return current_index > other_traversal_index;
}

bool TopologicalBackwardTraversal::edge(Vertex_id_t u_id, Vertex_id_t v_id) {
    return incidence_matrix[v_id][u_id];
}

void TopologicalSearch::create_scc_detection_graph() {
    const auto& forward_queue = forward_traversal.get_queue();
    const auto& backward_queue = backward_traversal.get_queue();
    const auto queues = {&forward_queue, &backward_queue};

    for (const auto* queue_u : queues) {
        for (const auto* queue_v : queues) {
            for (const auto& u : *queue_u) {
                for (const auto& v : *queue_v) {
                    if (incidence_matrix[u->id][v->id]) {
                        scc_detector.add_edge(u, v);
                        within_scc_detector.emplace_back(u);
                        within_scc_detector.emplace_back(v);
                    }
                }
            }
        }
    }
}

void TopologicalSearch::topological_search(const VertexPtr& u,
                                           const VertexPtr& v) {
    forward_traversal.init_queue(v);
    backward_traversal.init_queue(u);
    while (true) {
        if (forward_traversal.topological_search_step(
                backward_traversal.get_current_index())) {
            return;
        }
        if (backward_traversal.topological_search_step(
                forward_traversal.get_current_index())) {
            return;
        }
    }
}

void TopologicalSearch::find_new_connected_component(const VertexPtr& current,
                                                     const VertexPtr& u) {
    visited[current->id] = no_traversals;

    for (auto neighbour_iter = scc_detector.get_neighbours_begin(current);
         neighbour_iter != scc_detector.get_neighbours_end(current);
         ++neighbour_iter) {
        const auto& neighbour = *neighbour_iter;
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

void TopologicalSearch::adjust_incidence_matrix_with_new_scc() {
    for (size_t i = 1; i < new_scc.size(); i++) {
        const auto union_result =
            find_union.union_elements(new_scc[i - 1]->id, new_scc[i]->id);
        const auto [new_repr_id, old_repr_id] = *union_result;

        for (size_t u_id = 0; u_id < graph.get_no_vertices(); u_id++) {
            if (incidence_matrix[old_repr_id][u_id])
                incidence_matrix[new_repr_id][u_id] = true;
            if (incidence_matrix[u_id][old_repr_id])
                incidence_matrix[u_id][new_repr_id] = true;
        }
    }
}

void TopologicalSearch::algorithm_step(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u == v)
        return;

    if (TopologicalTraversal::is_before(u, v))
        return;

    topological_search(u, v);
    create_scc_detection_graph();
    no_traversals++;
    find_new_connected_component(v, u);

    forward_traversal.reorder();
    backward_traversal.update_current_index();
    backward_traversal.reorder();

    if (!new_scc.empty()) {
        adjust_incidence_matrix_with_new_scc();
        TopologicalTraversal::adjust_positions_with_new_scc(
            find_representative_vertex(u),
            TopologicalTraversal::get_position(v), new_scc);
    }
}

void TopologicalSearch::postprocess_edge(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u != v)
        incidence_matrix[u->id][v->id] = true;

    for (const auto& w : within_scc_detector)
        scc_detector.clean_vertex(w);
    within_scc_detector.clear();
    new_scc.clear();
}
