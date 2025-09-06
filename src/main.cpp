#include <fstream>
#include <iostream>

#include "utils/algorithm_factory.hpp"
#include "utils/rng.hpp"

Raw_edges_list read_edges_from_file(const std::string &filename) {
    Raw_edges_list edges;
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    Vertex_id_t u, v;
    while (file >> u >> v) {
        edges.emplace_back(u, v);
    }

    return edges;
}

size_t get_maximum_vertex_id(const Raw_edges_list &edges) {
    size_t max_id = 0;
    for (const auto &[u, v] : edges)
        max_id = std::max(max_id, std::max(u, v));

    return max_id;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <algorithm> <test_case>\n";
        return 1;
    }

    // Initialize project-wide rng class.
    auto &rng = RNG::instance();
    rng.seed(123);

    const std::string algorithm_name = argv[1];
    const std::string test_file = argv[2];

    try {
        const auto &edges = read_edges_from_file(test_file);
        const auto no_vertices = get_maximum_vertex_id(edges) + 1;
        const auto algorithm = create_algorithm(algorithm_name, no_vertices);
        algorithm->run(edges);
        algorithm->print_sccs(no_vertices);
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
