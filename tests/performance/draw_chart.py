"""
Chart-drawing script for performance tests.
Reads a json file with runtimes and outputs a PNG for a specified group of tests.
"""

import json
import os
import sys
import matplotlib.pyplot as plt
import matplotlib as mpl

from tests.common.test_config import PERFORMANCE_TEST_DIR, CACHE_FILENAME
from tests.performance.all_tests import TESTS

mpl.rcParams["pdf.fonttype"] = 42
mpl.rcParams["ps.fonttype"] = 42

OUTPUT_DIR = "charts"

cache_path = os.path.join(PERFORMANCE_TEST_DIR, CACHE_FILENAME)
with open(cache_path, "r") as cache_file:
    runtimes = json.load(cache_file)

# Ensure that each algorithm will have the same color on each chart.
all_algs = sorted(
    {alg for test_data in runtimes.values() for alg in test_data["algorithms"]}
)
colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]
manual_colors = {
    "two_way_search": "#1f77b4",  # blue
    "naive_one_way_search": "#d62728",  # red
    "soft_threshold_search": "#2ca02c",  # green
    "compatible_search": "#ff7f0e",  # orange
    "limited_search": "#9467bd",  # purple
    "one_way_search": "#8c564b",  # brown
    "sample_search": "#e377c2",  # pink
    "sparsified_sample_search": "#7f7f7f",  # gray
    "naive_dfs": "#17becf",  # cyan
    "topological_search": "#bcbd22",  # olive
}

alg_color_map = {}
i = 0
for alg in all_algs:
    if alg in manual_colors:
        alg_color_map[alg] = manual_colors[alg]
    else:
        alg_color_map[alg] = colors[i % len(colors)]
        i += 1


def plot_group_runtimes(group_name):
    group_tests = [test for test in TESTS if test.name.startswith(f"{group_name}_")]
    if not group_tests:
        raise ValueError(f"No tests found for group name '{group_name}'")

    sorted_tests = sorted(group_tests, key=lambda test: (test.graph.n, test.graph.m))
    labels = [f"{test.graph.n:,}" for test in sorted_tests]
    x_positions = list(range(len(sorted_tests)))
    name = sorted_tests[0].name
    alg_names = sorted(runtimes[name]["algorithms"].keys())

    plt.figure(figsize=(10, 5))
    for alg_name in alg_names:
        y_vals = [
            runtimes[test.name]["algorithms"].get(alg_name, float("nan"))
            for test in sorted_tests
        ]
        plt.plot(
            x_positions,
            y_vals,
            marker="o",
            label=alg_name,
            color=alg_color_map.get(alg_name),
        )

    plt.xlabel("Number of vertices")
    plt.ylabel("Time (seconds)")
    plt.xticks(x_positions, labels, rotation=45, ha="right")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_file = os.path.join(OUTPUT_DIR, f"{group_name}_runtimes.pdf")
    plt.savefig(out_file, bbox_inches="tight")
    print(f"Saved chart to {out_file}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: bash {sys.argv[0]} test_group (e.g. random_dense_graph)")
        sys.exit(1)
    plot_group_runtimes(sys.argv[1])
