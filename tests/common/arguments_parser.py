import argparse
from tests.common.test_config import ALGORITHMS


def get_arg_parser(default_algorithms, default_test_dir):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--generate-tests", action="store_true", help="Generate random tests."
    )
    parser.add_argument("--algorithms", nargs="+", help="Algorithms to run.")
    parser.add_argument(
        "--test-dir",
        default=default_test_dir,
        help="Directory with test files. It must contain `in` directory with tests' inputs and, if checking correctness, corresponding `out` directory.",
    )
    parser.add_argument(
        "--no-cache",
        action="store_true",
        help="Disable result caching when running performance tests.",
    )
    parser.set_defaults(default_algorithms=default_algorithms)
    return parser


def get_args(default_test_dir):
    parser = get_arg_parser(ALGORITHMS, default_test_dir)
    args = parser.parse_args()
    if args.algorithms is not None:
        for algorithm in args.algorithms:
            if algorithm not in ALGORITHMS:
                parser.error(f"Provided unknown algorithm: {algorithm}")

    return args
