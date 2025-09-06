#include "graph_sparsifier.hpp"

void GraphSparsifier::init(size_t original_no_vertices) {
    for (size_t id = 0; id < original_no_vertices; id++)
        representants[id] = corresponding_graph_ids[id] = id;
}

void GraphSparsifier::clear_generated_edges() const {
    generated_edges->clear();
}

void GraphSparsifier::insert_generated_edge(Vertex_id_t u,
                                            Vertex_id_t v) const {
    generated_edges->emplace_back(u, v);
}

Vertex_id_t GraphSparsifier::generate_edge_to_unused(Vertex_id_t u_id) {
    const auto v_id = first_unused_vertex_no;
    first_unused_vertex_no++;
    update_with_new_edge(u_id, v_id);
    corresponding_graph_ids[v_id] = *global_first_unused_vertex_no;
    (*global_first_unused_vertex_no)++;
    add_generated_edge(u_id, v_id);
    return v_id;
}

Vertex_id_t GraphSparsifier::get_corresponding_id(const VertexPtr& u) const {
    return corresponding_graph_ids[representants[u->id]];
}

void SimpleGraphSparsifier::generate_new_edges(const VertexPtr& u) {
    const auto w_id = representants[u->id];
    if (degrees[w_id] >= *average_degree)
        representants[u->id] = generate_edge_to_unused(w_id);
    degrees[representants[u->id]]++;
}

size_t SimpleGraphSparsifier::get_updated_no_of_vertices(size_t no_vertices) {
    return 2 * no_vertices * NODE_NO_MULTIPLIER;
}

void SimpleGraphSparsifier::increase_edges_no() {
    (*no_edges)++;
    *average_degree =
        std::max(*average_degree, ceil(2 * *no_edges, original_no_vertices));
}

void SimpleGraphSparsifierForward::add_generated_edge(Vertex_id_t u,
                                                      Vertex_id_t v) {
    generated_edges->emplace_back(corresponding_graph_ids[u],
                                  corresponding_graph_ids[v]);
}

void SimpleGraphSparsifierBackward::add_generated_edge(Vertex_id_t u,
                                                       Vertex_id_t v) {
    generated_edges->emplace_back(corresponding_graph_ids[v],
                                  corresponding_graph_ids[u]);
}

void AdvancedGraphSparsifier::init(size_t original_no_vertices) {
    for (size_t id = 0; id < original_no_vertices; id++)
        parents[id] = id;
}

void AdvancedGraphSparsifier::update_with_new_edge(Vertex_id_t u_id,
                                                   Vertex_id_t v_id) {
    parents[v_id] = u_id;
    depths[v_id] = depths[u_id] + 1;
    degrees[u_id]++;
}

void AdvancedGraphSparsifier::generate_new_edges(const VertexPtr& u) {
    auto w_id = representants[u->id];

    while (parents[w_id] != w_id && degrees[w_id] == average_degree)
        w_id = parents[w_id];

    // Move from d^k layer to d^(k+1) layer.
    if (parents[w_id] == w_id && degrees[w_id] == average_degree) {
        w_id = generate_edge_to_unused(w_id);
        parents[w_id] = w_id;
        depths[w_id] = 0;
        layers_no[u->id]++;
    }

    while (depths[w_id] != layers_no[u->id])
        w_id = generate_edge_to_unused(w_id);

    representants[u->id] = w_id;
    degrees[w_id]++;
}

size_t AdvancedGraphSparsifier::get_updated_no_of_vertices(size_t no_vertices) {
    return 2 * no_vertices * NODE_NO_MULTIPLIER;
}

void AdvancedGraphSparsifierForward::add_generated_edge(Vertex_id_t u,
                                                        Vertex_id_t v) {
    generated_edges->emplace_back(corresponding_graph_ids[u],
                                  corresponding_graph_ids[v]);
}

void AdvancedGraphSparsifierBackward::add_generated_edge(Vertex_id_t u,
                                                         Vertex_id_t v) {
    generated_edges->emplace_back(corresponding_graph_ids[v],
                                  corresponding_graph_ids[u]);
}
