#!/bin/sh

# Simple lint script for C code using clang-format
# Usage: ./lint.sh [format]

if [ "$1" = "format" ]; then
    echo "Formatting C code..."
    find . -name "*.c" -o -name "*.h" | xargs clang-format -i
    echo "Formatting complete!"
else
    echo "Linting C code..."
    find . -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror
    if [ $? -eq 0 ]; then
        echo "No formatting issues found!"
    else
        echo "Formatting issues found. Run './lint.sh format' to fix them."
        exit 1
    fi
fi