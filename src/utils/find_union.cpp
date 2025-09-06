#include "find_union.hpp"

FindUnion::FindUnion(size_t no_elements) {
    representants.resize(no_elements);
    group_sizes.resize(no_elements, 1);
    for (size_t i = 0; i < no_elements; i++)
        representants[i] = i;
}

Vertex_id_t FindUnion::find_representant(Vertex_id_t u) const {
    while (representants[u] != u)
        u = representants[u];
    return u;
}

std::optional<FindUnion::UnionResult> FindUnion::union_elements(Vertex_id_t u,
                                                                Vertex_id_t v) {
    u = find_representant(u);
    v = find_representant(v);
    if (u == v)
        return std::nullopt;

    if (group_sizes[u] < group_sizes[v])
        std::swap(u, v);

    representants[v] = u;
    group_sizes[u] += group_sizes[v];
    return FindUnion::UnionResult{u, v};
}
