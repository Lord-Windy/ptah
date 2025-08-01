# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Ptah is a CMake-based C monorepo with Bazel-like modularity. The repository contains two main libraries (`samrena` and `datazoo`) and example applications showcasing their integration. The project uses C11 standard and follows a structured approach to library development with hexagonal architecture patterns.

## Architecture

### Core Libraries

**samrena** - Memory Arena Management
- Provides efficient memory allocation with multiple strategies (default, chained, virtual)
- Implements hexagonal architecture with adapters for different memory management approaches
- Cross-platform virtual memory support (Windows, macOS, Linux)
- Includes SamrenaVector for dynamic arrays using arena allocation

**datazoo** - Data Structures Collection
- **Honeycomb**: Hash map implementation with multiple hash functions (DJB2, FNV1A, Murmur3)
- **Pearl**: Set data structure for unique elements
- **Starfish**: Additional data structure (in development)
- All structures are arena-backed using samrena for memory management

### Dependency Graph
```
datazoo → samrena
demo → samrena, datazoo
examples → samrena, datazoo
```

## Build System

### Common Commands

**Build the project:**
```bash
mkdir build
cd build
cmake ..
make
```

**Run all tests:**
```bash
cd build
ctest
```

**Run tests with verbose output:**
```bash
cd build
ctest --output-on-failure
```

**Run a single test:**
```bash
cd build
ctest -R test_name  # e.g., ctest -R samrena_basic
```

**Code formatting:**
```bash
# Check formatting
make lint

# Fix formatting
make format

# Alternative (from root)
./lint.sh
```

**LSP setup for IDE support:**
```bash
./setup_lsp.sh
```

**Valgrind memory testing:**
```bash
./valgrind_test.sh
```

**Build with different configurations:**
```bash
# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Debug build (default)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Valgrind build (restricted instruction set)
cmake -DCMAKE_BUILD_TYPE=Valgrind ..

# Shared libraries
cmake -DBUILD_SHARED_LIBS=ON ..

# Disable tests
cmake -DBUILD_TESTING=OFF ..

# Enable Valgrind tests
cmake -DENABLE_VALGRIND_TESTS=ON ..
```

### Custom CMake Functions

The build system provides two helper functions for consistent target creation:

- `ptah_add_library()`: Creates libraries with proper include directories and installation rules
- `ptah_add_executable()`: Creates executables with dependency management

### Automatic Discovery

The build system automatically discovers any directory containing a `CMakeLists.txt` file in:
- `libs/` - Libraries
- `apps/` - Applications  
- `tools/` - Development tools
- `tests/` - Test suites (when `BUILD_TESTING=ON`)

## Development Guidelines

### Adding New Components

**New Library:**
Create directory under `libs/` with `CMakeLists.txt`:
```cmake
ptah_add_library(mylib
    SOURCES
        src/mylib.c
    PUBLIC_HEADERS
        include/mylib.h
    DEPENDENCIES
        # other libraries
)
```

**New Application:**
Create directory under `apps/` with `CMakeLists.txt`:
```cmake
ptah_add_executable(myapp
    SOURCES
        src/main.c
    DEPENDENCIES
        mylib
)
```

### License Requirements

**All source files must include the Apache License 2.0 header with copyright to Samuel "Lord-Windy" Brown.**

For C/C++ files (.c, .h):
```c
/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

For CMake files:
```cmake
# Copyright 2025 Samuel "Lord-Windy" Brown
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
```

### Code Structure

Each library follows the standard layout:
```
libs/library_name/
├── CMakeLists.txt
├── include/
│   └── library_name.h
├── src/
│   └── implementation files
└── test/
    └── test files
```

## Testing

Tests are automatically built when `BUILD_TESTING=ON` (default). Each library can have its own test suite in its `test/` directory. Test executables are created using the same `ptah_add_executable()` helper function.

### Test Organization
- **Samrena tests**: Basic arena, vector operations, performance, type safety, adapters
- **Datazoo tests**: Honeycomb (hash map) and Pearl (set) functionality, collision handling, resizing

### Platform-Specific Notes
The project includes sophisticated platform detection for virtual memory support. The Valgrind build type restricts CPU instructions to ensure compatibility with Valgrind's instruction set emulation.