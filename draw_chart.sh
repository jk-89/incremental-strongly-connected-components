#!/bin/bash

export PYTHONPATH=$(pwd)

if [[ $# -ne 1 ]]; then
    echo "Usage: bash $0 test_group (e.g. random_sparse_graph)"
    exit 1
fi

python3 tests/performance/draw_chart.py "$1"
