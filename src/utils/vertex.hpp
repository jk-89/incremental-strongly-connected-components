#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <memory>

using Vertex_id_t = std::size_t;

class Vertex {
   public:
    Vertex_id_t id;

    explicit Vertex(Vertex_id_t id) : id(id) {}
};

using VertexPtr = std::shared_ptr<Vertex>;

#endif  // VERTEX_HPP
