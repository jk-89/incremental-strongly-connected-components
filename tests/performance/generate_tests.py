import os
import sys
import tarfile

from tests.common.test_config import PERFORMANCE_TEST_DIR
from tests.common.common import get_ins_directory
from tests.performance.all_tests import TESTS_TO_GENERATE


REAL_LIFE_ARCHIVE = os.path.join(PERFORMANCE_TEST_DIR, "real_life.tar.gz")
REAL_LIFE_DIRECTORY = "real_life/"
IN_DIRECTORY = get_ins_directory(PERFORMANCE_TEST_DIR)


class ProgressTracker:
    def __init__(self, total_tests, action_name):
        self.total_tests = total_tests
        self.parsed_tests = 0
        self.action_name = action_name
        self._print_progress()

    def _print_progress(self):
        sys.stdout.write(
            f"\rProgress: {self.parsed_tests}/{self.total_tests} test cases {self.action_name}."
        )

    def increment(self):
        self.parsed_tests += 1
        self._print_progress()


def extract_real_life_tests():
    sys.stdout.write("Extracting real life tests...\n")
    if not os.path.isfile(REAL_LIFE_ARCHIVE):
        sys.stderr.write(
            f"Archive '{REAL_LIFE_ARCHIVE}' not found, skipping extraction.\n"
        )
        return

    os.makedirs(IN_DIRECTORY, exist_ok=True)
    with tarfile.open(REAL_LIFE_ARCHIVE, "r:gz") as tar:
        tracker = ProgressTracker(len(tar.getmembers()) - 1, "extracted")
        for member in tar.getmembers():
            if not member.name.startswith(REAL_LIFE_DIRECTORY):
                continue
            relative_path = member.name[len(REAL_LIFE_DIRECTORY) :]
            if not relative_path:
                continue
            member.name = relative_path
            tar.extract(member, path=IN_DIRECTORY)
            tracker.increment()
    sys.stdout.write("\n")


def generate_tests():
    sys.stdout.write("Generating tests...\n")
    tracker = ProgressTracker(len(TESTS_TO_GENERATE), "generated")
    for test in TESTS_TO_GENERATE:
        test.generate()
        tracker.increment()
    sys.stdout.write("\n")


def main():
    extract_real_life_tests()
    generate_tests()


if __name__ == "__main__":
    main()
