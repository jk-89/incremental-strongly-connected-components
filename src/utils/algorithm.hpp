#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <vector>

#include "find_union.hpp"

class Algorithm {
   protected:
    size_t no_traversals = 0;
    Graph graph;
    FindUnion find_union;

    VertexPtr find_representative_vertex(const VertexPtr &u);
    // Uses union operation to merge multiple components into one.
    // Properly updates incident edges within provided graphs.
    void merge_into_component(const std::vector<VertexPtr> &vertices,
                              const std::vector<Graph *> &graphs);

    virtual void preprocess_edge(VertexPtr u, VertexPtr v);
    virtual void postprocess_edge(VertexPtr u, VertexPtr v);

    virtual void algorithm_step(VertexPtr u, VertexPtr v) = 0;

   public:
    explicit Algorithm(size_t no_vertices)
        : graph(no_vertices), find_union(no_vertices) {}

    virtual ~Algorithm() = default;

    virtual void run(const Raw_edges_list &edges);

    void print_sccs(size_t original_no_vertices);
};

#endif  // ALGORITHM_HPP
