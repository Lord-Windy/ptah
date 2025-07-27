# Valgrind Compatibility Guide

## Issue Description

When running binaries built with modern compilers on newer CPUs, Valgrind may encounter "unhandled instruction bytes" errors. This happens because:

1. Modern system libraries (like the dynamic linker) contain AVX-512 instructions
2. Older versions of Valgrind don't recognize these newer instruction sets
3. Even when compiling with restricted instruction sets, the system libraries still use the advanced instructions

## Solutions

### Solution 1: Use a newer version of Valgrind

The best solution is to use Valgrind 3.20 or newer, which has better support for AVX-512 instructions:

```bash
# Check your Valgrind version
valgrind --version

# If it's older than 3.20, consider upgrading
```

### Solution 2: Build with static linking and conservative flags

The project includes a Valgrind build type that uses conservative compiler flags:

```bash
# Create a Valgrind build
mkdir build-valgrind
cd build-valgrind
cmake .. -DCMAKE_BUILD_TYPE=Valgrind -DBUILD_SHARED_LIBS=OFF
make
```

### Solution 3: Use Valgrind with suppression files

A suppression file is provided to ignore known problematic instructions:

```bash
# Run with suppression file
valgrind --suppressions=valgrind_suppressions.supp --tool=memcheck ./your_binary
```

### Solution 4: Use Docker with older base image

For consistent results, use a Docker container with an older base image:

```dockerfile
FROM ubuntu:18.04
# Install your build dependencies
# Build and test with Valgrind
```

## Build Configuration

The project includes a custom Valgrind build type with the following compiler flags:

```
CMAKE_C_FLAGS_VALGRIND = "-O0 -g -march=x86-64 -mtune=generic -mno-avx -mno-avx2 -mno-avx512f -mno-avx512dq -mno-avx512cd -mno-avx512bw -mno-avx512vl -mno-avx512ifma -mno-avx512vbmi -mno-sse4.2 -mno-popcnt -mno-lzcnt -mno-bmi -mno-bmi2"
```

## Testing Script

A test script is provided to automatically build and test all binaries with Valgrind:

```bash
./valgrind_test.sh
```

This script:
1. Creates a separate build directory
2. Configures with Valgrind build type
3. Builds all binaries
4. Tests each executable with Valgrind
5. Reports results

## Limitations

- Static linking may not completely eliminate the issue if system libraries are still involved
- Some system calls may still trigger unrecognized instructions
- Performance will be significantly slower when running under Valgrind