#!/bin/bash

# Script to build and test all binaries with Valgrind

set -e

echo "=== Valgrind Testing Script ==="

# Create build directory for Valgrind testing
echo "Creating Valgrind build directory..."
rm -rf build-valgrind
mkdir build-valgrind
cd build-valgrind

# Configure with Valgrind build type
echo "Configuring project with Valgrind build type..."
cmake .. -DCMAKE_BUILD_TYPE=Valgrind -DBUILD_SHARED_LIBS=OFF

# Build the project
echo "Building project..."
if ! make -j$(nproc); then
    echo "Build failed. Trying with single thread..."
    make
    if [ $? -ne 0 ]; then
        echo "Build failed even with single thread. Exiting."
        exit 1
    fi
fi

# List all executables
echo "Finding all executables..."
EXECUTABLES=$(find bin -type f -executable 2>/dev/null || echo "")

if [ -z "$EXECUTABLES" ]; then
    echo "No executables found in bin directory"
    exit 1
fi

# Test each executable with Valgrind
echo "Testing executables with Valgrind..."
for EXEC in $EXECUTABLES; do
    echo "--------------------------------------------------"
    echo "Testing: $EXEC"
    echo "--------------------------------------------------"
    
    # Check if the executable is statically linked
    echo "Checking if executable is statically linked:"
    file ./$EXEC
    
    # Run Valgrind with memcheck using our wrapper
    echo "Running Valgrind with wrapper..."
    ../valgrind_wrapper.sh ./$EXEC
    
    VALGRIND_EXIT_CODE=$?
    if [ $VALGRIND_EXIT_CODE -eq 0 ]; then
        echo "✓ $EXEC: Valgrind test PASSED"
    else
        echo "✗ $EXEC: Valgrind test FAILED with exit code $VALGRIND_EXIT_CODE"
    fi
    echo ""
done

echo "=== Valgrind Testing Complete ==="