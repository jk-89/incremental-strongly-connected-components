# Incremental Strongly Connected Components

This repository contains the implementation and benchmarking of algorithms for solving the
**Incremental Strongly Connected Components (SCC)** problem.

The project is part of my Master's thesis and focuses on:

* implementing algorithms from academic literature,
* comparing different approaches (including variations that use different data structures),
* preparing a set of extensive tests, both artificial, and based on real-world graph networks,
* analyzing and comparing algorithm performance on the dataset.

Some of the implemented algorithms are based on:
[1](https://dl.acm.org/doi/10.1145/2756553),
[2](https://dl.acm.org/doi/10.1145/2071379.2071382),
[3](https://doi.org/10.4230/LIPIcs.ESA.2021.14).

## Background

The **Incremental SCC problem** is a fundamental problem in graph theory with applications in:

* managing compilation dependencies,
* process scheduling,
* pointer analysis,
* and more.

The problem setup:
* Start with an empty graph.
* Insert edges one by one.
* After each insertion, maintain the partition of vertices into [strongly connected components](https://en.wikipedia.org/wiki/Strongly_connected_component).

## Requirements
To build and run the project you need:

* `g++` compiler supporting `C++20` (or higher)
* `make` for building
* `Python3` for testing

## Execution

1. Build the project:

   ```bash
   make
   ```
2. Generate tests:

   ```bash
   bash run_tests.sh <mode> --generate 
   ```
   where `<mode>` is either `correctness` or `performance`.
3. Run tests:

   ```bash
   bash run_tests.sh <mode>
   ```
4. View available options:

   ```bash
   bash run_tests.sh <mode> --help
   ```
5. Before running the tests, you may need to increase the stack size limit:
   ```bash
   ulimit -s 16384
   ```
