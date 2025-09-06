#include "limited_search.hpp"

void LimitedSearch::dfs(const VertexPtr& current, const VertexPtr& target) {
    visited[current->id] = no_traversals;
    auto& neighbours = graph.get_neighbours(current);
    for (auto neighbour = neighbours.begin(); neighbour != neighbours.end();) {
        const auto repr = find_representative_vertex(*neighbour);
        // Remove loop / duplicated edge.
        if (repr == current ||
            visited_edge[{current->id, repr->id}] == no_traversals) {
            neighbour = neighbours.erase(neighbour);
            continue;
        }
        visited_edge[{current->id, repr->id}] = no_traversals;
        ++neighbour;

        if (visited[repr->id] != no_traversals) {
            if (order.is_before(repr->id, target->id)) {
                dfs(repr, target);
            } else {
                if (repr == target) {
                    reaches_target[repr->id] = no_traversals;
                    reached_target.emplace_back(repr);
                    postorder.emplace_back(repr);
                }
                visited[repr->id] = no_traversals;
            }
        }

        if (reaches_target[repr->id] == no_traversals)
            reaches_target[current->id] = no_traversals;
    }

    if (reaches_target[current->id] == no_traversals)
        reached_target.emplace_back(current);
    postorder.emplace_back(current);
}

void LimitedSearch::process_new_scc(const VertexPtr& target) {
    order.insert_after(dummy_id, target->id);
    if (reached_target.empty())
        return;

    for (const auto& u : reached_target)
        order.remove(u->id);
    merge_into_component(reached_target, {&graph});
    order.insert_before(find_union.find_representant(target->id), dummy_id);
}

void LimitedSearch::algorithm_step(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    // Topological order remains valid.
    if (u == v || order.is_before(u->id, v->id))
        return;

    no_traversals++;
    dfs(v, u);
    process_new_scc(u);

    auto previous_id = dummy_id;
    for (auto iter = postorder.rbegin(); iter != postorder.rend(); ++iter) {
        const auto& w = *iter;
        // Is in the new scc.
        if (reaches_target[w->id] == no_traversals)
            continue;
        order.remove(w->id);
        order.insert_after(w->id, previous_id);
        previous_id = w->id;
    }
    order.remove(dummy_id);
}

void LimitedSearch::postprocess_edge(VertexPtr u, VertexPtr v) {
    u = find_representative_vertex(u);
    v = find_representative_vertex(v);
    if (u != v)
        graph.add_edge(u, v);

    reached_target.clear();
    postorder.clear();

    // Due to unordered_map specifics we clear it when it grows too big.
    if (visited_edge.size() >= MAX_VISITED_EDGES_SIZE)
        visited_edge.clear();
}
