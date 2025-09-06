import os
import sys
from subprocess import check_output
from tests.common.test_config import EXECUTABLE


def get_ins_directory(base_dir):
    return os.path.join(base_dir, "in")


def get_outs_directory(base_dir):
    return os.path.join(base_dir, "out")


def assure_directory_exists(directory):
    if not os.path.isdir(directory):
        print(
            'Error: test cases directory does not exist. Perhaps you forgot to generate the tests using "--generate" flag.',
            file=sys.stderr,
        )
        sys.exit(1)


def write_case_in(case_id, edges, base_dir):
    inp_dir = get_ins_directory(base_dir)
    os.makedirs(inp_dir, exist_ok=True)
    in_path = os.path.join(inp_dir, f"{case_id}.in")

    with open(in_path, "w") as f:
        for u, v in edges:
            f.write(f"{u} {v}\n")


def write_case_out(case_id, base_dir, ground_truth):
    inp_dir = get_ins_directory(base_dir)
    out_dir = get_outs_directory(base_dir)
    os.makedirs(out_dir, exist_ok=True)
    in_path = os.path.join(inp_dir, f"{case_id}.in")
    out_path = os.path.join(out_dir, f"{case_id}.out")

    result = check_output([EXECUTABLE, ground_truth, in_path], text=True).strip()
    with open(out_path, "w") as f:
        f.write(result + "\n")
