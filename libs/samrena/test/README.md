# Samrena Test Suite

This directory contains comprehensive tests for the Samrena memory arena library.

## Test Files

### Core Tests
- `samrena_test.c` - Basic functionality tests
- `samrena_vector_test.c` - Vector implementation tests  
- `test_features.c` - Feature-specific tests

### Comprehensive Test Suite
- `test_samrena_adapters.c` - **Complete adapter test suite** with:
  - Basic allocation testing across all adapters
  - Large allocation handling
  - Stress testing with many small allocations
  - Growth behavior verification
  - Feature-specific tests (reset, reserve operations)
  - Edge case handling
  - Thread safety validation
  - Memory leak detection
  - Aligned allocation testing
  - Zero-initialized allocation testing

## Running Tests

### Standard Tests
```bash
# Build and run all tests
mkdir build && cd build
cmake ..
make
ctest
```

### Valgrind Integration

The test suite includes comprehensive Valgrind integration for memory leak detection. However, due to AVX-512 instruction compatibility issues on modern systems, Valgrind tests are disabled by default.

#### Option 1: Enable Valgrind Tests in CMake
```bash
cmake .. -DENABLE_VALGRIND_TESTS=ON
make
ctest
```

#### Option 2: Use the Dedicated Valgrind Script
```bash
# Uses Valgrind-compatible build settings
./run_valgrind_tests.sh
```

### Valgrind Test Types

When enabled, the following Valgrind tests are available:
- **Memory leak detection** - Detects memory leaks and invalid memory access
- **Track origins** - Tracks the origin of uninitialized values
- **Full leak checking** - Shows all types of memory leaks
- **Suppressions** - Uses project-specific suppressions for known system library issues

## Test Categories

### Basic Functionality
- Memory allocation and deallocation
- Arena creation and destruction
- Different allocation sizes (1 byte to 4KB)
- Memory initialization verification

### Stress Testing
- 10,000 small allocations per adapter
- Pattern verification across all allocations
- Memory integrity validation

### Growth Behavior
- Tests arena expansion when capacity exceeded
- Verifies proper capacity increases
- Validates continued allocation after growth

### Feature Testing
- **Reset operations** (for adapters that support it)
- **Reserve operations** (for adapters that support it) 
- **Aligned allocations** with various alignments (1-256 bytes)
- **Zero-initialized allocations**

### Edge Cases
- Zero-size allocation handling (returns NULL as expected)
- Very small allocations (1 byte)
- Boundary condition testing

### Thread Safety
- Multiple threads with separate arenas
- Concurrent allocation verification
- Pattern validation across threads
- Proper cleanup verification

### Memory Leak Detection
- 100 allocation cycles with random sizes
- Proper cleanup verification
- No memory leaks across adapters

## Adapter Coverage

The test suite validates all available adapters:
- **Chained Adapter** - Always available
- **Virtual Adapter** - Available on POSIX/Windows systems

Each adapter is tested with the same comprehensive test suite to ensure consistent behavior.

## Troubleshooting

### Valgrind AVX-512 Issues
If Valgrind tests fail with "unhandled instruction bytes" errors:
1. Use the provided `run_valgrind_tests.sh` script which uses Valgrind-compatible build settings
2. Consider using a newer version of Valgrind (3.20+)
3. Use Docker with an older base image that doesn't use AVX-512
4. Test on hardware without AVX-512 support

### Test Failures
- All tests should pass on supported platforms
- Virtual adapter tests will be skipped if virtual memory is not available
- Feature-specific tests (reset, reserve) are automatically skipped for adapters that don't support them