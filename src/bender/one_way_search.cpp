#include "one_way_search.hpp"

#include <cmath>

void BST::insert(Vertex_id_t vertex_id) { bst.insert(vertex_id); }

void BST::erase(Vertex_id_t vertex_id) { bst.erase(vertex_id); }

bool BST::contains(Vertex_id_t vertex_id) const {
    return bst.contains(vertex_id);
}

size_t BST::size() const { return bst.size(); }

void Heap::insert(Vertex_id_t vertex_id, Level_t level) {
    auto [iter, _] = out.insert({level, vertex_id});
    pointers[vertex_id] = iter;
}

void Heap::erase(Vertex_id_t vertex_id) { out.erase(pointers[vertex_id]); }

Heap::Out_vertices_t::iterator Heap::begin() { return out.begin(); }

void Heap::erase_begin() { out.erase(out.begin()); }

bool Heap::empty() const { return out.empty(); }

size_t OneWaySearch::log_2_floor(size_t x) {
    return static_cast<size_t>(std::log2(x));
}

void OneWaySearch::find_component_dfs(Vertex_id_t current_id,
                                      Vertex_id_t u_id) {
    for (const auto& [out_level, neighbour_id] : heap[current_id].get_out()) {
        if (out_level >= level[current_id])
            break;
        if (neighbour_id == u_id) {
            if (marked_within_component[u_id] != no_traversals) {
                marked_within_component[u_id] = no_traversals;
                component.push_back(graph.get_vertex_by_id(u_id));
            }
        } else if (level[neighbour_id] < level[current_id]) {
            level[neighbour_id] = level[current_id];
            find_component_dfs(neighbour_id, u_id);
        }

        if (marked_within_component[neighbour_id] == no_traversals)
            marked_within_component[current_id] = no_traversals;
    }

    if (marked_within_component[current_id] == no_traversals)
        component.push_back(graph.get_vertex_by_id(current_id));
}

void OneWaySearch::find_component(const VertexPtr& u, const VertexPtr& v) {
    no_traversals++;
    level[v->id] = level[u->id] + 1;
    find_component_dfs(v->id, u->id);
}

void OneWaySearch::insert_edge(const VertexPtr& u, const VertexPtr& v) {
    bst_out[u->id].insert(v->id);
    bst_in[v->id].insert(u->id);
    const auto in_degree = bst_in[v->id].size();
    const auto span = log_2_floor(in_degree);
    if ((static_cast<size_t>(1) << span) == in_degree) {
        bound[span][v->id] = level[v->id];
        count[span][v->id] = 0;
        if (span != 0)
            count[span - 1][v->id] = 0;
    }
}

void OneWaySearch::erase_edge_if_exists(Vertex_id_t u_id, Vertex_id_t v_id) {
    if (bst_out[u_id].contains(v_id)) {
        bst_out[u_id].erase(v_id);
        bst_in[v_id].erase(u_id);
        heap[u_id].erase(v_id);
    }
}

void OneWaySearch::move_from_heap_to_candidates(
    Vertex_id_t u_id, std::vector<Raw_edge_t>& candidate_edges) {
    while (!heap[u_id].empty()) {
        const auto [z_level, z_id] = *heap[u_id].begin();
        if (z_level > level[u_id])
            break;
        heap[u_id].erase_begin();
        candidate_edges.emplace_back(u_id, z_id);
    }
}

void OneWaySearch::merge_into_component(
    const std::vector<VertexPtr>& vertices) {
    for (size_t i = 1; i < vertices.size(); i++) {
        const auto union_result =
            find_union.union_elements(vertices[i - 1]->id, vertices[i]->id);
        if (!union_result.has_value())
            continue;

        const auto [new_repr_id, old_repr_id] = *union_result;
        erase_edge_if_exists(new_repr_id, old_repr_id);
        erase_edge_if_exists(old_repr_id, new_repr_id);

        for (const auto& neighbour_id : bst_out[old_repr_id].get_bst()) {
            if (!bst_out[new_repr_id].contains(neighbour_id)) {
                bst_out[new_repr_id].insert(neighbour_id);
                bst_in[neighbour_id].insert(new_repr_id);
                heap[new_repr_id].insert(neighbour_id, level[neighbour_id]);
            }
            bst_in[neighbour_id].erase(old_repr_id);
        }
        for (const auto& neighbour_id : bst_in[old_repr_id].get_bst()) {
            if (!bst_in[new_repr_id].contains(neighbour_id)) {
                bst_out[neighbour_id].insert(new_repr_id);
                bst_in[new_repr_id].insert(neighbour_id);
                heap[neighbour_id].insert(new_repr_id, level[new_repr_id]);
            }
            bst_out[neighbour_id].erase(old_repr_id);
            heap[neighbour_id].erase(old_repr_id);
        }
    }
}

std::vector<Raw_edge_t> OneWaySearch::form_component_and_fill_candidates(
    const VertexPtr& u, const VertexPtr& v) {
    if (marked_within_component[v->id] != no_traversals)
        return {{u->id, v->id}};

    merge_into_component(component);
    const auto component_representant = find_representative_vertex(u);
    for (auto& span_level : count)
        span_level[component_representant->id] = 0;

    std::vector<Raw_edge_t> candidate_edges;

    const auto y_id = component_representant->id;
    move_from_heap_to_candidates(y_id, candidate_edges);

    return candidate_edges;
}

void OneWaySearch::traversal_step(std::vector<Raw_edge_t>& candidate_edges) {
    const auto [x_id, y_id] = candidate_edges.back();
    candidate_edges.pop_back();
    if (level[x_id] >= level[y_id]) {
        level[y_id] = level[x_id] + 1;
    } else {
        const auto span = log_2_floor(
            std::min(level[y_id] - level[x_id], bst_in[y_id].size()));
        count[span][y_id]++;
        if (count[span][y_id] == 3 * (static_cast<size_t>(1) << span)) {
            count[span][y_id] = 0;
            level[y_id] =
                std::max(level[y_id], bound[span][y_id] + (1 << span));
            bound[span][y_id] = level[y_id];
        }
    }

    move_from_heap_to_candidates(y_id, candidate_edges);
    heap[x_id].insert(y_id, level[y_id]);
}

void OneWaySearch::algorithm_step(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);

    if (u == v || bst_out[u->id].contains(v->id))
        return;

    if (level[u->id] < level[v->id]) {
        insert_edge(u, v);
        heap[u->id].insert(v->id, level[v->id]);
        return;
    }

    find_component(u, v);
    auto candidates = form_component_and_fill_candidates(u, v);
    if (marked_within_component[v->id] != no_traversals)
        insert_edge(u, v);
    while (!candidates.empty())
        traversal_step(candidates);

    component.clear();
}
