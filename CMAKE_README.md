# CMake Build System Documentation

This document explains the CMake build system architecture for the Ptah monorepo, detailing how components link together and the underlying mechanisms that make it work.

## Table of Contents
1. [Overview](#overview)
2. [Project Structure](#project-structure)
3. [Core Build System](#core-build-system)
4. [Helper Functions](#helper-functions)
5. [Library Dependencies](#library-dependencies)
6. [External Library Management](#external-library-management)
7. [Build Flow](#build-flow)
8. [Configuration Options](#configuration-options)

## Overview

The Ptah build system is a sophisticated CMake-based architecture that provides:
- **Automatic discovery** of libraries, applications, and tests
- **Consistent build interfaces** through helper functions
- **External library management** similar to package managers
- **Platform-specific optimizations** including Valgrind support
- **Modular structure** allowing each component to have its own CMakeLists.txt

## Project Structure

```
ptah/
├── CMakeLists.txt                # Root build configuration
├── cmake/                        # CMake modules and utilities
│   ├── PtahLibraryManager.cmake  # External library management
│   ├── PtahExample.cmake         # Usage examples
│   └── adapters/                 # Build system adapters
│       ├── PtahCMakeAdapter.cmake
│       ├── PtahCargoAdapter.cmake
│       ├── PtahHeaderOnlyAdapter.cmake
│       ├── PtahMakeAdapter.cmake
│       └── PtahMesonAdapter.cmake
├── libs/                         # Libraries (auto-discovered)
│   ├── samrena/
│   │   └── CMakeLists.txt
│   └── datazoo/
│       └── CMakeLists.txt
├── apps/                         # Applications (auto-discovered)
│   └── demo/
│       └── CMakeLists.txt
└── tests/                        # Test suites (auto-discovered)
```

## Core Build System

### Root CMakeLists.txt

The root `CMakeLists.txt` orchestrates the entire build process:

1. **Project Setup** (lines 15-21)
   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(ptah VERSION 0.1.0 LANGUAGES C)
   set(CMAKE_C_STANDARD 11)
   ```

2. **Build Configuration** (lines 34-49)
   - Defines build types: Debug, Release, MinSizeRel, RelWithDebInfo, Valgrind
   - Custom Valgrind build type with restricted instruction sets for memory debugging
   - Disables AVX/SSE instructions that Valgrind might not support

3. **Output Directories** (lines 51-54)
   ```cmake
   set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
   set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
   set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
   ```
   All binaries go to `build/bin`, libraries to `build/lib`

4. **Auto-Discovery System** (lines 128-139, 143-154)
   The `add_subdirectories_recursively()` function automatically finds and includes any directory containing a `CMakeLists.txt` file within:
   - `libs/` - Libraries
   - `apps/` - Applications
   - `tools/` - Development tools
   - `tests/` - Test suites (when BUILD_TESTING=ON)

## Helper Functions

### ptah_add_library() (lines 65-107)

Creates libraries with consistent settings:

```cmake
ptah_add_library(name
    SOURCES file1.c file2.c
    PUBLIC_HEADERS header1.h header2.h
    DEPENDENCIES other_lib
)
```

**What it does:**
1. Creates the library target (static/shared/interface)
2. Sets up include directories:
   - Public: `include/` directory visible to consumers
   - Private: `src/` directory for internal headers
3. Links dependencies
4. Configures installation rules
5. Exports the target for other projects to use

### ptah_add_executable() (lines 110-125)

Creates executables with dependency management:

```cmake
ptah_add_executable(name
    SOURCES main.c
    DEPENDENCIES lib1 lib2
)
```

**What it does:**
1. Creates the executable target
2. Links specified dependencies privately
3. Sets up installation to `bin/` directory

## Library Dependencies

### Dependency Graph

```
samrena (Memory Arena)
    ↑
datazoo (Data Structures)
    ↑
demo (Application)
```

### How Dependencies Work

1. **samrena** (libs/samrena/CMakeLists.txt)
   - Base library with no dependencies
   - Provides memory management via arenas
   - Platform-specific virtual memory support (lines 16-41)
   - Conditional compilation based on platform

2. **datazoo** (libs/datazoo/CMakeLists.txt)
   - Depends on samrena (line 23)
   - Uses samrena's arena allocation for all data structures
   - Link established via: `DEPENDENCIES samrena`

3. **demo** (apps/demo/CMakeLists.txt)
   - Depends on both libraries (lines 19-20)
   - Gets transitive dependencies automatically
   - CMake handles the linking order

### Linking Process

When you specify `DEPENDENCIES`, CMake:
1. Adds the library to the link line
2. Includes its public headers in the include path
3. Propagates any transitive dependencies
4. Ensures correct build order

Example: When demo links datazoo, it automatically gets samrena too because datazoo publicly depends on it.

## External Library Management

The `cmake/PtahLibraryManager.cmake` provides a sophisticated system for managing external dependencies:

### Architecture

```
PtahLibraryManager.cmake (Main orchestrator)
    ├── Downloads sources
    ├── Detects build system
    └── Delegates to appropriate adapter:
        ├── PtahCMakeAdapter (SDL, etc.)
        ├── PtahCargoAdapter (Rust/wgpu-native)
        ├── PtahHeaderOnlyAdapter (Clay, single headers)
        ├── PtahMakeAdapter (Traditional Make projects)
        └── PtahMesonAdapter (Meson build system)
```

### Usage Example

```cmake
ptah_add_library(
    NAME SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    VERSION 3.0.0
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DSDL_SHARED=OFF
        -DSDL_TEST=OFF
)
```

### How It Works

1. **Version Resolution**: Converts tags/branches to specific commits
2. **Source Management**: Downloads to `external_dependencies/sources/`
3. **Build Delegation**: Calls appropriate adapter based on BUILD_SYSTEM
4. **Installation**: Installs to `external_dependencies/install/`
5. **Integration**: Makes library available via find_package()

## Build Flow

### Complete Build Process

1. **Initial Configuration**
   ```bash
   mkdir build && cd build
   cmake ..
   ```

2. **CMake Execution Flow**
   ```
   1. Load root CMakeLists.txt
   2. Set up compiler flags and directories
   3. Define helper functions
   4. Auto-discover subdirectories
   5. For each subdirectory:
      a. Load its CMakeLists.txt
      b. Create targets using helper functions
      c. Register dependencies
   6. Generate build files
   ```

3. **Build Execution**
   ```bash
   make
   ```
   - Builds in dependency order
   - samrena → datazoo → applications

4. **Testing**
   ```bash
   ctest
   ```
   - Runs all registered tests
   - Optional Valgrind integration

### Dependency Resolution

CMake automatically:
1. **Analyzes** the dependency graph
2. **Orders** targets for parallel building
3. **Propagates** include paths and compile flags
4. **Links** libraries in correct order

Example build order for `make demo`:
1. Build samrena library
2. Build datazoo library (needs samrena)
3. Build demo executable (needs both)

## Configuration Options

### Build Options

```bash
# Build type
cmake -DCMAKE_BUILD_TYPE=Release ..    # Optimized build
cmake -DCMAKE_BUILD_TYPE=Debug ..      # Debug symbols
cmake -DCMAKE_BUILD_TYPE=Valgrind ..   # Memory debugging

# Library options
cmake -DBUILD_SHARED_LIBS=ON ..        # Build shared libraries
cmake -DBUILD_TESTING=OFF ..           # Disable tests

# Platform-specific
cmake -DSAMRENA_ENABLE_VIRTUAL=OFF ..  # Disable virtual memory
cmake -DENABLE_VALGRIND_TESTS=ON ..    # Enable Valgrind tests
```

### Platform Detection

The build system automatically detects:
- **Operating System**: Windows, macOS, Linux
- **Architecture**: For instruction set restrictions
- **Available Tools**: Valgrind, thread libraries

Example from samrena (lines 16-29):
```cmake
if(WIN32)
    set(SAMRENA_VM_PLATFORM "win32")
elseif(APPLE)
    set(SAMRENA_VM_PLATFORM "darwin")
else()
    set(SAMRENA_VM_PLATFORM "posix")
endif()
```

### Custom Targets

- `make lint` - Check code formatting
- `make format` - Auto-format code
- `make install` - Install libraries and headers

## Advanced Features

### Configure-Time Code Generation

samrena generates a config header (lines 64-68):
```cmake
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/samrena_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/samrena_config.h"
)
```
This allows compile-time configuration based on platform capabilities.

### Export Configuration

Libraries are exported (line 101) for use in other projects:
```cmake
install(TARGETS ${name}
    EXPORT ptahTargets
    ...
)
```

This enables other projects to use:
```cmake
find_package(ptah REQUIRED)
target_link_libraries(myapp ptah::samrena ptah::datazoo)
```

### Transitive Dependencies

When datazoo links samrena publicly:
- Consumers of datazoo automatically get samrena headers
- Link flags are propagated
- No need to explicitly link both

## Best Practices

1. **Always use helper functions** - Ensures consistency
2. **Specify dependencies explicitly** - Makes relationships clear
3. **Use PUBLIC/PRIVATE appropriately** - Controls propagation
4. **Keep CMakeLists.txt simple** - Complexity goes in helpers
5. **Follow the directory structure** - Enables auto-discovery

## Troubleshooting

### Common Issues

1. **Missing dependencies**: Check DEPENDENCIES in ptah_add_library/executable
2. **Include errors**: Verify PUBLIC_HEADERS are specified
3. **Link errors**: Ensure correct dependency order
4. **Valgrind failures**: Use Valgrind build type or disable tests

### Debugging the Build

```bash
# Verbose output
make VERBOSE=1

# CMake debugging
cmake --debug-output ..

# Check generated files
ls build/CMakeFiles/
```

## Summary

The Ptah CMake system provides a clean, modular build architecture that:
- **Automatically discovers** components based on directory structure
- **Manages dependencies** through a clear, declarative syntax
- **Provides consistency** via helper functions
- **Supports external libraries** through a flexible adapter system
- **Handles platform differences** transparently

The key insight is that complexity is hidden in the root CMakeLists.txt and helper modules, while individual component CMakeLists.txt files remain simple and focused on declaring what they need.