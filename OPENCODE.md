# OPENCODE.md

This file provides guidance to opencode (opencode.ai) when working with code in this repository.

## Project Overview

Ptah is a CMake-based C monorepo with Bazel-like modularity. The repository contains two main libraries (`samrena` and `datazoo`) and a demo application showcasing their integration. The project uses C11 standard and follows a structured approach to library development.

## Architecture

### Core Libraries
- **samrena**: Memory arena management library providing efficient memory allocation
- **datazoo**: Hash map implementation (honeycomb data structure) that depends on samrena

### Dependency Graph
```
datazoo → samrena
demo → samrena, datazoo
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

**Build with different configurations:**
```bash
# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..

# Shared libraries
cmake -DBUILD_SHARED_LIBS=ON ..

# Disable tests
cmake -DBUILD_TESTING=OFF ..
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

## opencode Guidelines

### Understanding the Codebase

When working with this repository, opencode should:

1. **Respect the modular structure**: Each component in `libs/`, `apps/`, `tools/`, and `tests/` is independent and follows the same patterns
2. **Follow dependency rules**: Check the dependency graph before making changes that might affect multiple components
3. **Use the provided CMake helpers**: Always use `ptah_add_library()` and `ptah_add_executable()` rather than raw CMake commands
4. **Maintain license headers**: Ensure all new files include the proper Apache License 2.0 header

### Common Tasks

**Adding a new feature to an existing library:**
1. Locate the library in `libs/library_name/`
2. Add new source files to `src/` and update `CMakeLists.txt`
3. Add corresponding header files to `include/`
4. Update tests in `test/` directory
5. Follow the existing code style and patterns

**Creating a new library:**
1. Create a new directory under `libs/`
2. Follow the standard layout pattern
3. Use `ptah_add_library()` in the `CMakeLists.txt`
4. Include proper license headers
5. Add tests following the existing patterns

**Working with tests:**
1. Tests are located in each component's `test/` directory
2. Use the same CMake patterns as the main code
3. Run tests with `ctest` in the build directory
4. Add new tests following existing patterns

### Code Style and Conventions

1. **C Standard**: Use C11 standard
2. **Naming**: Use snake_case for functions and variables
3. **Headers**: All public API should be in header files in `include/`
4. **Implementation**: Implementation files go in `src/`
5. **Documentation**: Add Doxygen-style comments for public API functions

### When to Ask for Help

opencode should ask for clarification when:
1. The request involves modifying core architectural decisions
2. There's ambiguity about which component should implement a feature
3. The request might introduce security concerns
4. The implementation approach is unclear