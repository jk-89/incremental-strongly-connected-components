#include "algorithm_factory.hpp"

#include <functional>

#include "bender/naive_one_way_search.hpp"
#include "bender/one_way_search.hpp"
#include "bender/two_way_search.hpp"
#include "bernstein/sample_search.hpp"
#include "haeupler/compatible_search.hpp"
#include "haeupler/limited_search.hpp"
#include "haeupler/soft_threshold_search.hpp"
#include "haeupler/topological_search.hpp"
#include "naive/naive_dfs.hpp"

using AlgorithmFactory = std::function<std::unique_ptr<Algorithm>(size_t)>;
using AlgorithmFactoryMap = std::unordered_map<std::string, AlgorithmFactory>;

std::unique_ptr<Algorithm> create_algorithm(const std::string& algorithm_name,
                                            size_t no_vertices) {
    static const AlgorithmFactoryMap algorithm_creators = {
        {"naive_dfs",
         [](size_t no_vertices_) {
             return std::make_unique<NaiveDfs>(no_vertices_);
         }},
        {"naive_one_way_search",
         [](size_t no_vertices_) {
             return std::make_unique<NaiveOneWaySearch>(no_vertices_);
         }},
        {"one_way_search",
         [](size_t no_vertices_) {
             return std::make_unique<OneWaySearch>(no_vertices_);
         }},
        {"two_way_search",
         [](size_t no_vertices_) {
             return std::make_unique<TwoWaySearch>(no_vertices_);
         }},
        {"limited_search",
         [](size_t no_vertices_) {
             return std::make_unique<LimitedSearch>(no_vertices_);
         }},
        {"compatible_search",
         [](size_t no_vertices_) {
             auto order = std::make_shared<DynamicOrderList>(no_vertices_);
             return std::make_unique<CompatibleSearch>(no_vertices_, order);
         }},
        {"soft_threshold_search_basic_list",
         [](size_t no_vertices_) {
             auto order = std::make_shared<DynamicOrderBasicList>(no_vertices_);
             return std::make_unique<SoftThresholdSearch>(no_vertices_, order);
         }},
        {"soft_threshold_search_treap",
         [](size_t no_vertices_) {
             auto order = std::make_shared<DynamicOrderTreap>(no_vertices_);
             return std::make_unique<SoftThresholdSearch>(no_vertices_, order);
         }},
        {"soft_threshold_search",
         [](size_t no_vertices_) {
             auto order = std::make_shared<DynamicOrderList>(no_vertices_);
             return std::make_unique<SoftThresholdSearch>(no_vertices_, order);
         }},
        {"topological_search",
         [](size_t no_vertices_) {
             return std::make_unique<TopologicalSearch>(no_vertices_);
         }},
        {"sample_search",
         [](size_t no_vertices_) {
             auto order = std::make_shared<DynamicOrderList>(no_vertices_);
             return std::make_unique<SampleSearch>(no_vertices_, order);
         }},
        {"sparsified_sample_search", [](size_t no_vertices_) {
             const auto updated_no_vertices =
                 SimpleGraphSparsifier::get_updated_no_of_vertices(
                     no_vertices_);
             auto order =
                 std::make_shared<DynamicOrderList>(updated_no_vertices);
             return std::make_unique<SparsifiedSampleSearch>(
                 updated_no_vertices, no_vertices_, order);
         }}};

    const auto algorithm_creator = algorithm_creators.find(algorithm_name);
    if (algorithm_creator != algorithm_creators.end()) {
        return algorithm_creator->second(no_vertices);
    }

    throw std::runtime_error("Unknown algorithm: " + algorithm_name);
}
