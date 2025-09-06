#include "algorithm.hpp"

#include <iostream>
#include <iterator>

VertexPtr Algorithm::find_representative_vertex(const VertexPtr &u) {
    const auto repr_id = find_union.find_representant(u->id);
    return graph.get_vertex_by_id(repr_id);
}

void Algorithm::merge_into_component(const std::vector<VertexPtr> &vertices,
                                     const std::vector<Graph *> &graphs) {
    for (size_t i = 1; i < vertices.size(); i++) {
        const auto union_result =
            find_union.union_elements(vertices[i - 1]->id, vertices[i]->id);
        if (!union_result.has_value())
            continue;

        const auto [new_repr_id, old_repr_id] = *union_result;
        for (auto &g : graphs)
            g->move_neighbours_by_id(old_repr_id, new_repr_id);
    }
}

void Algorithm::preprocess_edge(VertexPtr, VertexPtr) {}

void Algorithm::postprocess_edge(VertexPtr, VertexPtr) {}

void Algorithm::run(const Raw_edges_list &edges) {
    for (const auto &edge : edges) {
        const auto [u_id, v_id] = edge;
        const auto u = graph.get_vertex_by_id(u_id);
        const auto v = graph.get_vertex_by_id(v_id);
        preprocess_edge(u, v);
        algorithm_step(u, v);
        postprocess_edge(u, v);
    }
}

void Algorithm::print_sccs(size_t original_no_vertices) {
    const size_t no_vertices = graph.get_no_vertices();
    std::vector<std::vector<Vertex_id_t>> sccs(no_vertices + 1);
    for (size_t i = 0; i < original_no_vertices; i++) {
        const auto v = graph.get_vertex_by_id(i);
        const auto representant = find_representative_vertex(v);
        sccs[representant->id].push_back(v->id);
    }

    for (size_t i = 0; i < no_vertices; i++) {
        if (sccs[i].empty())
            continue;

        size_t min_id = i;
        for (const auto &id : sccs[i])
            min_id = std::min(min_id, id);
        if (i != min_id) {
            sccs[min_id] = std::move(sccs[i]);
            sccs[i].clear();
        }
    }

    for (size_t i = 0; i < original_no_vertices; i++) {
        if (sccs[i].empty())
            continue;
        std::ranges::copy(sccs[i], std::ostream_iterator<int>(std::cout, " "));
        std::cout << '\n';
    }
}
