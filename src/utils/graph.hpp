#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <list>
#include <vector>

#include "vertex.hpp"

using Raw_edge_t = std::pair<Vertex_id_t, Vertex_id_t>;
using Raw_edges_list = std::vector<Raw_edge_t>;
using Vertex_list = std::list<VertexPtr>;

class Graph {
   private:
    size_t no_vertices = 0;
    std::vector<VertexPtr> vertices;
    std::vector<Vertex_list> adjacency_list;

   public:
    Graph() = default;
    explicit Graph(size_t no_vertices);
    // Copies the vertex set of the other graph, doesn't add any edges.
    Graph(const Graph& other)
        : no_vertices(other.no_vertices),
          vertices(other.vertices),
          adjacency_list(other.no_vertices) {}

    void add_edge(const VertexPtr& u, const VertexPtr& v);

    // Set the list of neighbours of u to an empty list.
    void clean_vertex(const VertexPtr& u);

    // Moves all neighbours of u to the end of adjacency list of v.
    // When the function finishes u won't have any neighbours.
    void move_neighbours(const VertexPtr& u, const VertexPtr& v);
    void move_neighbours_by_id(Vertex_id_t u, Vertex_id_t v);

    size_t get_neighbours_no(const VertexPtr& u) const;
    Vertex_list::iterator get_neighbours_begin(const VertexPtr& u);
    Vertex_list::iterator get_neighbours_end(const VertexPtr& u);
    std::list<VertexPtr>& get_neighbours(const VertexPtr& v);

    void erase_neighbour(const VertexPtr& u,
                         Vertex_list::iterator neighbour_iter);

    size_t get_no_vertices() const;
    VertexPtr get_vertex_by_id(Vertex_id_t id);
};

#endif  // GRAPH_HPP
