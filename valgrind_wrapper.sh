#!/bin/bash

# Wrapper script to run Valgrind with appropriate flags for newer CPUs

if [ $# -lt 1 ]; then
    echo "Usage: $0 <executable> [valgrind options]"
    exit 1
fi

EXECUTABLE=$1
shift

# Run Valgrind with flags that handle newer instruction sets
valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --show-error-list=yes \
    --suppressions=../valgrind_suppressions.supp \
    --gen-suppressions=all \
    --expensive-definedness-checks=yes \
    --read-inline-info=yes \
    --read-var-info=yes \
    --vex-iropt-register-updates=allregs-at-mem-access \
    --vex-iropt-level=0 \
    --max-threads=100 \
    --num-callers=20 \
    "$@" \
    "$EXECUTABLE"