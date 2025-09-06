#ifndef GRAPH_SPARSIFIER_HPP
#define GRAPH_SPARSIFIER_HPP

#include <vector>

#include "graph.hpp"

// Assume we have a graph G with average degree of a node equal to d.
// It is possible to create a graph G* based on G, where reachability is
// preserved, at most O(n) new vertices / edges were created, and each vertex
// have degree O(d). This class handles managing G*.
class GraphSparsifier {
   protected:
    static size_t ceil(size_t x, size_t y) { return (x + y - 1) / y; }

    std::shared_ptr<size_t> no_edges;

    size_t first_unused_vertex_no;
    std::shared_ptr<size_t> global_first_unused_vertex_no;

    std::vector<Vertex_id_t> representants;
    std::vector<size_t> corresponding_graph_ids;

    std::shared_ptr<std::vector<Raw_edge_t>> generated_edges;

    virtual void init(size_t original_no_vertices);

    virtual void add_generated_edge(Vertex_id_t u, Vertex_id_t v) = 0;
    virtual void update_with_new_edge(Vertex_id_t, Vertex_id_t) {}
    Vertex_id_t generate_edge_to_unused(Vertex_id_t u_id);

   public:
    // Since in G* we create some new vertices, we want to initialize the object
    // with sufficient number of elements.
    GraphSparsifier(size_t original_no_vertices, size_t node_no_multiplier)
        : no_edges(std::make_shared<size_t>(0)),
          first_unused_vertex_no(original_no_vertices),
          global_first_unused_vertex_no(
              std::make_shared<size_t>(original_no_vertices)),
          representants(original_no_vertices),
          corresponding_graph_ids(original_no_vertices * node_no_multiplier),
          generated_edges(std::make_shared<std::vector<Raw_edge_t>>()) {
        GraphSparsifier::init(original_no_vertices);
    }
    GraphSparsifier(size_t original_no_vertices, size_t node_no_multiplier,
                    const GraphSparsifier& other)
        : no_edges(other.no_edges),
          first_unused_vertex_no(original_no_vertices),
          global_first_unused_vertex_no(other.global_first_unused_vertex_no),
          representants(original_no_vertices),
          corresponding_graph_ids(original_no_vertices * node_no_multiplier),
          generated_edges(other.generated_edges) {
        GraphSparsifier::init(original_no_vertices);
    }
    virtual ~GraphSparsifier() = default;

    const std::shared_ptr<std::vector<Raw_edge_t>>& get_generated_edges()
        const {
        return generated_edges;
    }
    void clear_generated_edges() const;
    void insert_generated_edge(Vertex_id_t u, Vertex_id_t v) const;

    virtual void generate_new_edges(const VertexPtr& u) = 0;

    Vertex_id_t get_corresponding_id(const VertexPtr& u) const;
    virtual void increase_edges_no() {}
};

class SimpleGraphSparsifier : public GraphSparsifier {
   protected:
    constexpr static size_t NODE_NO_MULTIPLIER = 4;
    constexpr static size_t MIN_AVERAGE_DEGREE = 10;
    size_t original_no_vertices;
    std::shared_ptr<size_t> average_degree;

    std::vector<size_t> degrees;

   public:
    explicit SimpleGraphSparsifier(size_t original_no_vertices)
        : GraphSparsifier(original_no_vertices, NODE_NO_MULTIPLIER),
          original_no_vertices(original_no_vertices),
          average_degree(std::make_shared<size_t>(MIN_AVERAGE_DEGREE)),
          degrees(original_no_vertices * NODE_NO_MULTIPLIER) {}
    SimpleGraphSparsifier(size_t original_no_vertices,
                          const SimpleGraphSparsifier& other)
        : GraphSparsifier(original_no_vertices, NODE_NO_MULTIPLIER, other),
          original_no_vertices(original_no_vertices),
          average_degree(other.average_degree),
          degrees(original_no_vertices * NODE_NO_MULTIPLIER) {}

    void generate_new_edges(const VertexPtr& u) override;

    static size_t get_updated_no_of_vertices(size_t no_vertices);
    void increase_edges_no() override;
};

class SimpleGraphSparsifierForward : public SimpleGraphSparsifier {
   protected:
    void add_generated_edge(Vertex_id_t u, Vertex_id_t v) override;

   public:
    explicit SimpleGraphSparsifierForward(size_t original_no_vertices)
        : SimpleGraphSparsifier(original_no_vertices) {}
};

class SimpleGraphSparsifierBackward : public SimpleGraphSparsifier {
   protected:
    void add_generated_edge(Vertex_id_t u, Vertex_id_t v) override;

   public:
    SimpleGraphSparsifierBackward(size_t original_no_vertices,
                                  const SimpleGraphSparsifierForward& forward)
        : SimpleGraphSparsifier(original_no_vertices, forward) {}
};

// Method of sparsification from the paper by A. Bernstein and S. Chechik:
// "Incremental Topological Sort and Cycle Detection in ~O(m * sqrt(n))
// Expected Total Time"
class AdvancedGraphSparsifier : public GraphSparsifier {
   protected:
    constexpr static size_t NODE_NO_MULTIPLIER = 8;
    constexpr static size_t INITIAL_LAYERS_NO = 1;
    constexpr static size_t MIN_AVERAGE_DEGREE = 2;
    size_t no_vertices;
    size_t average_degree;

    // Represents a balanced "tree" from the sparsification process description.
    // We represent the tree structure purely by ids, not actual vertices.
    std::vector<size_t> layers_no;
    std::vector<Vertex_id_t> parents;
    std::vector<size_t> depths;
    std::vector<size_t> degrees;

    void init(size_t original_no_vertices) override;

    void update_with_new_edge(Vertex_id_t u_id, Vertex_id_t v_id) override;

   public:
    AdvancedGraphSparsifier(size_t original_no_vertices, size_t no_edges)
        : GraphSparsifier(original_no_vertices, NODE_NO_MULTIPLIER),
          no_vertices(original_no_vertices * NODE_NO_MULTIPLIER),
          average_degree(std::max(MIN_AVERAGE_DEGREE,
                                  ceil(no_edges, original_no_vertices))),
          layers_no(original_no_vertices, INITIAL_LAYERS_NO),
          parents(no_vertices),
          depths(no_vertices),
          degrees(no_vertices) {
        AdvancedGraphSparsifier::init(original_no_vertices);
    }
    AdvancedGraphSparsifier(size_t original_no_vertices,
                            const AdvancedGraphSparsifier& other)
        : GraphSparsifier(original_no_vertices, NODE_NO_MULTIPLIER, other),
          no_vertices(other.no_vertices),
          average_degree(other.average_degree),
          layers_no(original_no_vertices, INITIAL_LAYERS_NO),
          parents(no_vertices),
          depths(no_vertices),
          degrees(no_vertices) {
        AdvancedGraphSparsifier::init(original_no_vertices);
    }

    void generate_new_edges(const VertexPtr& u) override;

    static size_t get_updated_no_of_vertices(size_t no_vertices);
};

class AdvancedGraphSparsifierForward : public AdvancedGraphSparsifier {
   protected:
    void add_generated_edge(Vertex_id_t u, Vertex_id_t v) override;

   public:
    AdvancedGraphSparsifierForward(size_t original_no_vertices, size_t no_edges)
        : AdvancedGraphSparsifier(original_no_vertices, no_edges) {}
};

class AdvancedGraphSparsifierBackward : public AdvancedGraphSparsifier {
   protected:
    void add_generated_edge(Vertex_id_t u, Vertex_id_t v) override;

   public:
    AdvancedGraphSparsifierBackward(
        size_t original_no_vertices,
        const AdvancedGraphSparsifierForward& forward)
        : AdvancedGraphSparsifier(original_no_vertices, forward) {}
};

#endif  // GRAPH_SPARSIFIER_HPP
