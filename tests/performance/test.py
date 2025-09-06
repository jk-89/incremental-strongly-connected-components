import subprocess
import os
import time
import json

from tests.common import arguments_parser, common
from tests.common.test_config import PERFORMANCE_TEST_DIR, EXECUTABLE, CACHE_FILENAME
from tests.performance.all_tests import TESTS

GEN_SCRIPT = "tests/performance/generate_tests.py"

# ID of the core that will run the tests.
CORE_TO_RUN = 3

# How many times the test is run before taking an average of all runtimes.
PERF_RUNS_NO = 5


def run_algorithm(algorithm, test_file):
    try:
        start_time = time.perf_counter()
        args = ["taskset", "-c", str(CORE_TO_RUN), EXECUTABLE, algorithm, test_file]
        subprocess.run(
            args, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        elapsed = time.perf_counter() - start_time
        return elapsed
    except subprocess.CalledProcessError as e:
        return f"[ERROR] {e}"


# Since performance tests are time-consuming, the results are cached.
# Algorithms are reran only if their output is not in cache, or the corresponding
# input was changed in the meantime.
def run_tests(test_dir, algorithms, cache_enabled):
    in_dir = common.get_ins_directory(test_dir)
    common.assure_directory_exists(in_dir)
    total = len(TESTS)

    cache_path = os.path.join(test_dir, CACHE_FILENAME)
    if os.path.exists(cache_path):
        with open(cache_path, "r") as cache_file:
            cache = json.load(cache_file)
    else:
        cache = {}

    for i, test in enumerate(TESTS, start=1):
        in_path = os.path.join(in_dir, test.name + ".in")
        time_modified = os.path.getmtime(in_path)
        header = f"Test {i}/{total}: {test.name}"
        print(header)
        print("=" * len(header))
        print(f"{'Algorithm':<30}  {'Time (s)':>10} {'Cached':>8}")
        print("-" * 51)

        entry = cache.get(test.name, {})
        entry_time_modified = entry.get("time_modified")
        algorithm_times = entry.get("algorithms", {})
        if entry_time_modified != time_modified:
            algorithm_times = {}
            entry_time_modified = time_modified

        for algorithm in sorted(set(test.algorithms) & set(algorithms)):
            if cache_enabled and algorithm in algorithm_times:
                from_cache = True
            else:
                algorithm_times[algorithm] = 0.0
                for _ in range(PERF_RUNS_NO):
                    algorithm_times[algorithm] += run_algorithm(algorithm, in_path)
                algorithm_times[algorithm] /= PERF_RUNS_NO
                from_cache = False

            cache_communicate = "YES" if from_cache else "NO"
            print(
                f"{algorithm:<30} {algorithm_times[algorithm]:>10.4f}s {cache_communicate:>8}"
            )

        cache[test.name] = {
            "time_modified": entry_time_modified,
            "algorithms": algorithm_times,
        }
        print()

    with open(cache_path, "w") as cache_file:
        json.dump(cache, cache_file, indent=2)


def main():
    args = arguments_parser.get_args(PERFORMANCE_TEST_DIR)

    if args.generate_tests:
        subprocess.run(["python3", GEN_SCRIPT], check=True)
        return

    algorithms = args.algorithms or args.default_algorithms
    run_tests(args.test_dir, algorithms, not args.no_cache)


if __name__ == "__main__":
    main()
