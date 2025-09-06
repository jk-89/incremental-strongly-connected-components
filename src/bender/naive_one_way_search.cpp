#include "naive_one_way_search.hpp"

void NaiveOneWaySearch::detect_new_scc(const VertexPtr& current,
                                       const VertexPtr& target) {
    const auto timestamp = ++traversal_steps_no;
    visited[current->id] = no_traversals;
    auto& neighbours = graph.get_neighbours(current);
    for (auto neighbour = neighbours.begin(); neighbour != neighbours.end();) {
        const auto repr = find_representative_vertex(*neighbour);
        // Remove loop / duplicated edge.
        if (repr == current ||
            visited_edge[{current->id, repr->id}] == timestamp) {
            neighbour = neighbours.erase(neighbour);
            continue;
        }
        visited_edge[{current->id, repr->id}] = timestamp;
        ++neighbour;

        if (visited[repr->id] != no_traversals) {
            if (level[repr->id] < level[target->id]) {
                detect_new_scc(repr, target);
            } else {
                if (repr == target) {
                    reaches_target[repr->id] = no_traversals;
                    reached_target.emplace_back(repr);
                }
                visited[repr->id] = no_traversals;
            }
        }

        if (reaches_target[repr->id] == no_traversals)
            reaches_target[current->id] = no_traversals;
    }

    if (reaches_target[current->id] == no_traversals)
        reached_target.emplace_back(current);
}

void NaiveOneWaySearch::update_levels(const VertexPtr& current) {
    const auto timestamp = ++traversal_steps_no;
    auto& neighbours = graph.get_neighbours(current);
    for (auto neighbour = neighbours.begin(); neighbour != neighbours.end();) {
        const auto repr = find_representative_vertex(*neighbour);
        // Remove loop / duplicated edge.
        if (repr == current ||
            visited_edge[{current->id, repr->id}] == timestamp) {
            neighbour = neighbours.erase(neighbour);
            continue;
        }
        visited_edge[{current->id, repr->id}] = timestamp;
        ++neighbour;

        if (level[repr->id] <= level[current->id]) {
            level[repr->id] = level[current->id] + 1;
            update_levels(repr);
        }
    }
}

void NaiveOneWaySearch::algorithm_step(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (level[u->id] < level[v->id] || u == v)
        return;

    no_traversals++;
    detect_new_scc(v, u);
    merge_into_component(reached_target, {&graph});

    if (reached_target.empty()) {
        level[v->id] = level[u->id] + 1;
    } else {
        const auto updated_level =
            std::max(level[u->id], level[v->id] + reached_target.size() - 1);
        v = find_representative_vertex(v);
        level[v->id] = updated_level;
    }
    update_levels(v);
}

void NaiveOneWaySearch::postprocess_edge(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u != v)
        graph.add_edge(u, v);

    reached_target.clear();
    // Due to unordered_map specifics we clear it when it grows too big.
    if (visited_edge.size() >= MAX_VISITED_EDGES_SIZE)
        visited_edge.clear();
}
