#!/bin/bash

# Valgrind Test Script for Samrena
# Uses the Valgrind-compatible build settings from VALGRIND_SUMMARY.md

set -e

echo "Setting up Valgrind-compatible build..."

# Create separate build directory for Valgrind testing
BUILD_DIR="build-valgrind"
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with Valgrind-specific settings
echo "Configuring with Valgrind build type..."
cmake .. -DCMAKE_BUILD_TYPE=Valgrind -DBUILD_SHARED_LIBS=OFF -DENABLE_VALGRIND_TESTS=ON

# Build the project
echo "Building project..."
make -j$(nproc)

echo ""
echo "Running Valgrind tests..."
echo "========================="

# Function to run valgrind with proper options
run_valgrind_test() {
    local test_name="$1"
    local executable="$2"
    
    echo "Running $test_name with Valgrind..."
    
    valgrind \
        --tool=memcheck \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --error-exitcode=1 \
        --suppressions=../valgrind_suppressions.supp \
        --quiet \
        "./$executable"
    
    if [ $? -eq 0 ]; then
        echo "✓ $test_name: PASS"
    else
        echo "✗ $test_name: FAIL"
        return 1
    fi
}

# Run individual tests
echo ""
echo "Memory leak detection tests:"
echo "----------------------------"

run_valgrind_test "samrena_test" "bin/samrena_test"
run_valgrind_test "samrena_vector_test" "bin/samrena_vector_test" 
run_valgrind_test "samrena_feature_test" "bin/samrena_feature_test"
run_valgrind_test "samrena_adapter_test" "bin/samrena_adapter_test"

echo ""
echo "All Valgrind tests completed successfully!"
echo ""
echo "Note: If tests fail with 'unhandled instruction bytes' errors,"
echo "this indicates AVX-512 instructions in system libraries that"
echo "this version of Valgrind doesn't support. Consider using:"
echo "  - A newer version of Valgrind (3.20+)"
echo "  - Docker with an older base image"
echo "  - Testing on hardware without AVX-512 support"