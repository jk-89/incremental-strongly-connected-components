#include "graph.hpp"

Graph::Graph(size_t no_vertices)
    : no_vertices(no_vertices),
      vertices(no_vertices),
      adjacency_list(no_vertices) {
    for (size_t i = 0; i < no_vertices; i++)
        vertices[i] = std::make_shared<Vertex>(i);
}

void Graph::add_edge(const VertexPtr& u, const VertexPtr& v) {
    adjacency_list[u->id].push_back(v);
}

void Graph::clean_vertex(const VertexPtr& u) { adjacency_list[u->id].clear(); }

void Graph::move_neighbours(const VertexPtr& u, const VertexPtr& v) {
    adjacency_list[v->id].splice(adjacency_list[v->id].end(),
                                 adjacency_list[u->id]);
}

void Graph::move_neighbours_by_id(Vertex_id_t u, Vertex_id_t v) {
    move_neighbours(get_vertex_by_id(u), get_vertex_by_id(v));
}

size_t Graph::get_neighbours_no(const VertexPtr& u) const {
    return adjacency_list[u->id].size();
}

Vertex_list::iterator Graph::get_neighbours_begin(const VertexPtr& u) {
    return adjacency_list[u->id].begin();
}

Vertex_list::iterator Graph::get_neighbours_end(const VertexPtr& u) {
    return adjacency_list[u->id].end();
}

std::list<VertexPtr>& Graph::get_neighbours(const VertexPtr& v) {
    return adjacency_list[v->id];
}

void Graph::erase_neighbour(const VertexPtr& u,
                            Vertex_list::iterator neighbour_iter) {
    adjacency_list[u->id].erase(neighbour_iter);
}

size_t Graph::get_no_vertices() const { return no_vertices; }

VertexPtr Graph::get_vertex_by_id(Vertex_id_t id) { return vertices[id]; }
