#!/bin/bash

# Shared entrypoint to run either correctness or performance tests.
# Pass flags like --generate-tests, --algorithms, --tests.

MODE=$1
shift

export PYTHONPATH=$(pwd)

if [[ "$MODE" == "correctness" ]]; then
    python3 tests/correctness/test.py "$@"
elif [[ "$MODE" == "performance" ]]; then
    python3 tests/performance/test.py "$@"
else
    echo "Unknown mode: $MODE"
    echo "Usage: bash $0 [correctness|performance] [--flags]"
    exit 1
fi
