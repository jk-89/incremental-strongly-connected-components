#ifndef ALGORITHM_FACTORY_HPP
#define ALGORITHM_FACTORY_HPP

#include <memory>

#include "algorithm.hpp"

// Given an algorithm name creates a corresponding Algorithm object.
std::unique_ptr<Algorithm> create_algorithm(const std::string& algorithm_name,
                                            size_t no_vertices);

#endif  // ALGORITHM_FACTORY_HPP
