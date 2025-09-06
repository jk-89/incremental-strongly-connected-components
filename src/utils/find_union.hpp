#ifndef FIND_UNION_HPP
#define FIND_UNION_HPP

#include <optional>
#include <vector>

#include "graph.hpp"

using Group_size_t = size_t;

class FindUnion {
   private:
    std::vector<Vertex_id_t> representants;
    std::vector<Group_size_t> group_sizes;

   public:
    struct UnionResult {
        Vertex_id_t new_representative;
        Vertex_id_t merged_away;
    };

    FindUnion() = default;
    explicit FindUnion(size_t no_elements);

    Vertex_id_t find_representant(Vertex_id_t u) const;
    std::optional<UnionResult> union_elements(Vertex_id_t u, Vertex_id_t v);
};

#endif  // FIND_UNION_HPP
