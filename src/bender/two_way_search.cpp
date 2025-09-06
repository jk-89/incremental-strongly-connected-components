#include "two_way_search.hpp"

#include <queue>

void TwoWaySearch::update_threshold() {
    // As stated in the paper, we only try to adjust the threshold if the number
    // of edges is a power of 2.
    if ((no_edges & (no_edges - 1)) != 0)
        return;

    const auto sqrt_no_edges =
        static_cast<size_t>(sqrt(static_cast<double>(no_edges)));
    const auto new_threshold_candidate =
        std::min(sqrt_no_edges, vertices_threshold);
    if (new_threshold_candidate >= threshold * 2)
        threshold = new_threshold_candidate;
}

void TwoWaySearch::search_backward(const VertexPtr& u, const VertexPtr& v) {
    no_traversals++;
    visited[u->id] = no_traversals;
    std::queue<VertexPtr> to_be_considered;
    to_be_considered.push(u);

    while (!to_be_considered.empty()) {
        const auto current = to_be_considered.front();
        to_be_considered.pop();
        no_traversal_steps++;
        considered_during_traversal[current->id] = no_traversal_steps;

        auto& neighbours = reversed_graph.get_neighbours(current);
        for (auto neighbour = neighbours.begin();
             neighbour != neighbours.end();) {
            const auto repr = find_representative_vertex(*neighbour);

            // Remove loop / duplicated edge.
            if (considered_during_traversal[repr->id] == no_traversal_steps) {
                neighbour = neighbours.erase(neighbour);
                continue;
            }

            considered_during_traversal[repr->id] = no_traversal_steps;
            if (repr == v) {
                found_cycle = true;
            } else if (visited[repr->id] != no_traversals) {
                visited[repr->id] = no_traversals;
                to_be_considered.push(repr);
            }
            edges_used_backwards++;
            if (edges_used_backwards == threshold)
                return;
            ++neighbour;
        }
    }
}

void TwoWaySearch::search_forward(const VertexPtr& u) {
    std::vector<VertexPtr> to_be_considered;
    to_be_considered.push_back(u);

    while (!to_be_considered.empty()) {
        const auto current = to_be_considered.back();
        to_be_considered.pop_back();
        no_traversal_steps++;
        considered_during_traversal[current->id] = no_traversal_steps;

        auto& neighbours = graph.get_neighbours(current);
        for (auto neighbour = neighbours.begin();
             neighbour != neighbours.end();) {
            const auto repr = find_representative_vertex(*neighbour);

            // Remove loop / duplicated edge.
            if (considered_during_traversal[repr->id] == no_traversal_steps) {
                neighbour = neighbours.erase(neighbour);
                continue;
            }

            considered_during_traversal[repr->id] = no_traversal_steps;
            if (visited[repr->id] == no_traversals) {
                found_cycle = true;
            }

            if (level[repr->id] == level[u->id]) {
                reversed_graph.add_edge(repr, current);
            } else if (level[repr->id] < level[u->id]) {
                level[repr->id] = level[u->id];
                reversed_graph.clean_vertex(repr);
                reversed_graph.add_edge(repr, current);
                to_be_considered.push_back(repr);
            }

            ++neighbour;
        }
    }
}

void TwoWaySearch::form_component_dfs(const VertexPtr& u) {
    visited[u->id] = no_traversals;
    auto& neighbours = reversed_graph.get_neighbours(u);

    // We have to remove loops and duplicated edges first,
    // because we want to use recursive dfs procedure.
    no_traversal_steps++;
    considered_during_traversal[u->id] = no_traversal_steps;
    for (auto neighbour = neighbours.begin(); neighbour != neighbours.end();) {
        const auto repr = find_representative_vertex(*neighbour);

        if (considered_during_traversal[repr->id] == no_traversal_steps) {
            neighbour = neighbours.erase(neighbour);
        } else {
            considered_during_traversal[repr->id] = no_traversal_steps;
            ++neighbour;
        }
    }

    for (auto& neighbour : neighbours) {
        const auto repr = find_representative_vertex(neighbour);

        if (marked_within_component[repr->id] != no_traversals &&
            visited[repr->id] != no_traversals)
            form_component_dfs(repr);
        if (marked_within_component[repr->id] == no_traversals) {
            marked_within_component[u->id] = no_traversals;
            component.push_back(u);
        }
    }
}

void TwoWaySearch::form_component(const VertexPtr& u, const VertexPtr& v) {
    if (!found_cycle)
        return;

    no_traversals++;
    marked_within_component[v->id] = no_traversals;
    component = {v};
    form_component_dfs(u);
    merge_into_component(component, {&graph, &reversed_graph});
}

void TwoWaySearch::algorithm_step(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u == v || level[u->id] < level[v->id])
        return;

    found_cycle = false;
    edges_used_backwards = 0;
    search_backward(u, v);
    if (edges_used_backwards != threshold) {
        if (level[v->id] == level[u->id]) {
            form_component(u, v);
            return;
        }
        level[v->id] = level[u->id];
    } else {
        level[v->id] = level[u->id] + 1;
        // Clever way to properly track 'backward' vertices from the algorithm.
        no_traversals++;
        visited[u->id] = no_traversals;
    }

    reversed_graph.clean_vertex(v);
    search_forward(v);
    form_component(u, v);
}

void TwoWaySearch::preprocess_edge(VertexPtr, VertexPtr) {
    no_edges++;
    update_threshold();
}

void TwoWaySearch::postprocess_edge(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u != v) {
        graph.add_edge(u, v);
        if (level[u->id] == level[v->id])
            reversed_graph.add_edge(v, u);
    }
}
