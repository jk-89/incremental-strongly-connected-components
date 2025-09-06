import subprocess
import os
import sys

from tests.common import common, arguments_parser
from tests.common.test_config import CORRECTNESS_TEST_DIR, EXECUTABLE

GEN_SCRIPT = "tests/correctness/generate_tests.py"


def run_algorithm(algorithm, test_file):
    try:
        result = subprocess.check_output([EXECUTABLE, algorithm, test_file], text=True)
        return result.strip()
    except subprocess.CalledProcessError as e:
        return f"[ERROR] {e}"


def run_tests(test_dir, algorithms):
    in_dir = common.get_ins_directory(test_dir)
    out_dir = common.get_outs_directory(test_dir)
    common.assure_directory_exists(in_dir)
    common.assure_directory_exists(out_dir)
    test_files = sorted([f for f in os.listdir(in_dir) if f.endswith(".in")])
    passed = 0
    total = len(test_files)

    for i, test_file in enumerate(test_files, start=1):
        in_path = os.path.join(in_dir, test_file)
        out_path = os.path.join(out_dir, test_file.replace(".in", ".out"))
        with open(out_path, "r") as out_file:
            expected = out_file.read().strip()

        for algorithm in algorithms:
            result = run_algorithm(algorithm, in_path)
            if result == expected:
                passed += 1
            else:
                sys.stdout.write(f"\n[FAIL] {algorithm} on {test_file}\n")
                sys.stdout.flush()

        sys.stdout.write(f"\rProgress: {i}/{total} cases.")
        sys.stdout.flush()

    print("\n\n=== SUMMARY ===")
    if passed == total * len(algorithms):
        print(f"All {total} cases passed! 🎉🎉🎉")
    else:
        print("Some errors occurred.")


def main():
    args = arguments_parser.get_args(CORRECTNESS_TEST_DIR)

    if args.generate_tests:
        subprocess.run(["python3", GEN_SCRIPT], check=True)
        return

    algorithms = args.algorithms or args.default_algorithms
    run_tests(args.test_dir, algorithms)


if __name__ == "__main__":
    main()
