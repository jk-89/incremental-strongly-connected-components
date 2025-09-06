#ifndef TOPOLOGICAL_SEARCH_HPP
#define TOPOLOGICAL_SEARCH_HPP

/// Implements Topological Search from https://doi.org/10.1145/2071379.2071382.
/// Works in total time O(n^{5/2}). Uses O(n^2) memory regardless from the
/// total number of edges.

#include <queue>

#include "utils/algorithm.hpp"

// Since forward and backward steps are similar we introduce a common class
// that catch this common behaviour.
class TopologicalTraversal {
   public:
    using Incidence_matrix_t = std::vector<std::vector<bool>>;

   protected:
    using Vertex_queue_t = std::deque<VertexPtr>;

    const Incidence_matrix_t& incidence_matrix;
    // Variables called `i` and `j` in the paper.
    size_t current_index;
    Vertex_queue_t queue;
    // Arrays called `position` and `vertex` in the paper.
    static std::vector<size_t> positions;
    static std::vector<VertexPtr> vertices;
    static size_t canonical_vertices_no;

    virtual bool correct_order_of_indices(size_t other_traversal_index) = 0;
    void push_vertex_at_current_index_to_queue();

    // Returns if edge (u, v) exists (forward or backward edge, based on the
    // traversal type).
    virtual bool edge(Vertex_id_t u_id, Vertex_id_t v_id) = 0;

   public:
    TopologicalTraversal(Graph& graph,
                         const Incidence_matrix_t& incidence_matrix)
        : incidence_matrix(incidence_matrix), current_index(0) {
        const auto no_vertices = graph.get_no_vertices();
        canonical_vertices_no = no_vertices;
        positions.resize(no_vertices);
        vertices.resize(no_vertices);
        for (size_t i = 0; i < no_vertices; i++) {
            positions[i] = i;
            vertices[i] = graph.get_vertex_by_id(i);
        }
    }
    virtual ~TopologicalTraversal() = default;

    size_t get_current_index() const { return current_index; }
    const Vertex_queue_t& get_queue() const { return queue; }

    virtual void update_current_index() = 0;

    void init_queue(const VertexPtr& u);

    static bool is_before(const VertexPtr& u, const VertexPtr& v);
    static size_t get_position(const VertexPtr& u);

    // Returns if the topological search should be finished.
    bool topological_search_step(size_t other_traversal_index);

    void reorder();

    static void adjust_positions_with_new_scc(
        const VertexPtr& new_repr, size_t new_position_in_scc,
        const std::vector<VertexPtr>& new_scc);
};

class TopologicalForwardTraversal : public TopologicalTraversal {
   protected:
    bool correct_order_of_indices(size_t other_traversal_index) override;

    bool edge(Vertex_id_t u_id, Vertex_id_t v_id) override;

   public:
    TopologicalForwardTraversal(Graph& graph,
                                const Incidence_matrix_t& incidence_matrix)
        : TopologicalTraversal(graph, incidence_matrix) {}

    void update_current_index() override;
};

class TopologicalBackwardTraversal : public TopologicalTraversal {
   protected:
    bool correct_order_of_indices(size_t other_traversal_index) override;

    bool edge(Vertex_id_t u_id, Vertex_id_t v_id) override;

   public:
    TopologicalBackwardTraversal(Graph& graph,
                                 const Incidence_matrix_t& incidence_matrix)
        : TopologicalTraversal(graph, incidence_matrix) {}

    void update_current_index() override;
};

class TopologicalSearch : public Algorithm {
   private:
    using Vertex_queue_t = std::deque<VertexPtr>;
    TopologicalTraversal::Incidence_matrix_t incidence_matrix;
    TopologicalForwardTraversal forward_traversal;
    TopologicalBackwardTraversal backward_traversal;
    // Used to determine newly created strongly connected components.
    Graph scc_detector;
    std::vector<VertexPtr> within_scc_detector;
    std::vector<Vertex_id_t> visited;
    std::vector<VertexPtr> new_scc;
    std::vector<size_t> is_in_new_scc;

    void topological_search(const VertexPtr& u, const VertexPtr& v);

    void create_scc_detection_graph();
    void find_new_connected_component(const VertexPtr& current,
                                      const VertexPtr& u);
    void adjust_incidence_matrix_with_new_scc();

    void algorithm_step(VertexPtr u, VertexPtr v) override;
    void postprocess_edge(VertexPtr u, VertexPtr v) override;

   public:
    explicit TopologicalSearch(size_t no_vertices)
        : Algorithm(no_vertices),
          incidence_matrix(no_vertices, std::vector<bool>(no_vertices)),
          forward_traversal(graph, incidence_matrix),
          backward_traversal(graph, incidence_matrix),
          scc_detector(graph),
          visited(no_vertices),
          is_in_new_scc(no_vertices) {}
};

#endif  // TOPOLOGICAL_SEARCH_HPP
