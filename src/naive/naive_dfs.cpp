#include "naive_dfs.hpp"

std::vector<VertexPtr> NaiveDfs::dfs(const VertexPtr& source, Graph& g,
                                     bool store_encountered) {
    std::vector<VertexPtr> to_be_considered;
    std::vector<VertexPtr> encountered;

    to_be_considered.push_back(source);
    visited[source->id] = no_traversals;
    while (!to_be_considered.empty()) {
        const auto current = to_be_considered.back();
        if (store_encountered)
            encountered.push_back(current);
        to_be_considered.pop_back();

        for (const auto& neighbour : g.get_neighbours(current)) {
            if (visited[neighbour->id] != no_traversals) {
                visited[neighbour->id] = no_traversals;
                to_be_considered.push_back(neighbour);
            }
        }
    }

    return encountered;
}

void NaiveDfs::algorithm_step(VertexPtr u, VertexPtr v) {
    // Vertices already in the same SCC.
    if (find_representative_vertex(u) == find_representative_vertex(v))
        return;

    no_traversals++;
    const auto& encountered = dfs(u, graph, true);
    no_traversals++;
    dfs(u, reversed_graph, false);

    // Vertices belonging to SCC of u are exactly those who are reachable
    // from u and can reach u.
    for (const auto& w : encountered) {
        if (visited[w->id] == no_traversals)
            find_union.union_elements(u->id, w->id);
    }
}

void NaiveDfs::preprocess_edge(VertexPtr u, VertexPtr v) {
    graph.add_edge(u, v);
    reversed_graph.add_edge(v, u);
}
