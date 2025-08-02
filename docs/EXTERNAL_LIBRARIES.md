# External Library Integration with Ptah

This guide demonstrates how to integrate external libraries into your Ptah applications using the Ptah Library Manager system.

## Overview

The Ptah Library Manager (`@cmake/`) provides a unified way to:
- Download and build external libraries
- Manage dependency versions
- Handle different build systems (CMake, Make, Meson, Cargo)
- Maintain reproducible builds

## Quick Start

### 1. Basic Integration

```cmake
# Include the Ptah Library Manager
include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)

# Add an external library
ptah_add_library(
    NAME library_name
    GIT_REPOSITORY https://github.com/org/repo
    VERSION tag_or_branch
    BUILD_SYSTEM cmake
)

# Find and configure the library
ptah_find_library(library_name)

# Use in your application
ptah_add_executable(my_app
    SOURCES
        main.c
    DEPENDENCIES
        library_name::library_name
)
```

### 2. Real Example: SDL3 Integration

```cmake
include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)

ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    VERSION release-3.2.18
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DSDL_STATIC=ON
        -DSDL_SHARED=OFF
        -DSDL_TEST=OFF
        -DSDL_EXAMPLES=OFF
)

ptah_find_library(SDL3)

ptah_add_executable(sdl_example
    SOURCES
        sdl_example.c
    DEPENDENCIES
        SDL3::SDL3
)
```

## Supported Build Systems

### CMake Libraries

Most modern C/C++ libraries use CMake:

```cmake
ptah_add_library(
    NAME fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    VERSION 10.1.1
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DFMT_DOC=OFF
        -DFMT_TEST=OFF
        -DFMT_INSTALL=ON
)
```

### Header-Only Libraries

For header-only libraries (no compilation needed):

```cmake
ptah_add_library(
    NAME nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    VERSION v3.11.2
    BUILD_SYSTEM header_only
)
```

### Makefile-based Libraries

For traditional Makefile-based builds:

```cmake
ptah_add_library(
    NAME zlib
    GIT_REPOSITORY https://github.com/madler/zlib
    VERSION v1.3
    BUILD_SYSTEM make
    BUILD_OPTIONS
        CFLAGS=-fPIC
        prefix=${PTAH_INSTALL_DIR}
)
```

### Meson Libraries

For Meson-based projects:

```cmake
ptah_add_library(
    NAME gstreamer
    GIT_REPOSITORY https://gitlab.freedesktop.org/gstreamer/gstreamer
    VERSION 1.22.0
    BUILD_SYSTEM meson
)
```

## Configuration Options

### ptah_add_library Parameters

| Parameter | Description | Required |
|-----------|-------------|----------|
| `NAME` | Unique library identifier | ✅ |
| `GIT_REPOSITORY` | Git repository URL | ✅ |
| `VERSION` | Tag, branch, or commit hash | ✅ |
| `BUILD_SYSTEM` | `cmake`, `make`, `meson`, `cargo`, `header_only` | ✅ |
| `CMAKE_ARGS` | Additional CMake configuration flags | ❌ |
| `BUILD_OPTIONS` | Build-system specific options | ❌ |
| `FORCE_REBUILD` | Force rebuild even if cached | ❌ |

### Version Specification

```cmake
# Use specific tag
VERSION v1.2.3

# Use branch
VERSION main

# Use commit hash
VERSION a1b2c3d4e5f6

# Use release tag
VERSION release-3.2.18
```

## Application Structure

### Recommended Directory Layout

```
your_app/
├── CMakeLists.txt              # Main build configuration
├── external_dependencies/     # Generated - not in git
│   ├── downloads/             # Downloaded archives
│   ├── sources/               # Git clones
│   ├── build/                 # Build directories
│   └── install/               # Installed libraries
├── ptah-lock.yaml            # Generated - not in git
├── src/                      # Your source code
└── include/                  # Your headers
```

### CMakeLists.txt Template

```cmake
# Copyright 2025 Samuel "Lord-Windy" Brown
# [License header...]

cmake_minimum_required(VERSION 3.20)
project(your_app VERSION 1.0.0 LANGUAGES C)

# Include Ptah Library Manager
include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)

# Add external libraries
ptah_add_library(
    NAME your_library
    GIT_REPOSITORY https://github.com/org/library
    VERSION v1.0.0
    BUILD_SYSTEM cmake
)

ptah_find_library(your_library)

# Create your application
ptah_add_executable(your_app
    SOURCES
        src/main.c
        src/other.c
    DEPENDENCIES
        your_library::your_library
)
```

## Common Library Examples

### Graphics Libraries

```cmake
# SDL3 for cross-platform multimedia
ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    VERSION release-3.2.18
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DSDL_TEST=OFF
)

# GLFW for OpenGL window management
ptah_add_library(
    NAME glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    VERSION 3.3.8
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DGLFW_BUILD_EXAMPLES=OFF
)

# OpenGL Extension Wrangler
ptah_add_library(
    NAME glew
    GIT_REPOSITORY https://github.com/nigels-com/glew
    VERSION glew-2.2.0
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DBUILD_UTILS=OFF
)
```

### Utility Libraries

```cmake
# Modern C++ formatting
ptah_add_library(
    NAME fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    VERSION 10.1.1
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DFMT_DOC=OFF
)

# JSON parsing
ptah_add_library(
    NAME nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    VERSION v3.11.2
    BUILD_SYSTEM header_only
)

# HTTP client library
ptah_add_library(
    NAME curl
    GIT_REPOSITORY https://github.com/curl/curl
    VERSION curl-8_4_0
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DBUILD_CURL_EXE=OFF
)
```

### Math and Science Libraries

```cmake
# Linear algebra
ptah_add_library(
    NAME eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen
    VERSION 3.4.0
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DBUILD_TESTING=OFF
)

# Fast Fourier Transform
ptah_add_library(
    NAME fftw
    GIT_REPOSITORY https://github.com/FFTW/fftw3
    VERSION fftw-3.3.10
    BUILD_SYSTEM cmake
    CMAKE_ARGS -DBUILD_TESTS=OFF
)
```

## Best Practices

### 1. Version Pinning

Always use specific versions for reproducible builds:

```cmake
# Good - specific version
VERSION v1.2.3

# Avoid - moving target
VERSION main
```

### 2. Minimal Configuration

Only build what you need:

```cmake
ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    VERSION release-3.2.18
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DSDL_TEST=OFF          # Skip tests
        -DSDL_EXAMPLES=OFF      # Skip examples
        -DSDL_STATIC=ON         # Static linking
        -DSDL_SHARED=OFF        # No shared libs
)
```

### 3. Error Handling

Always check that libraries are found:

```cmake
ptah_find_library(SDL3)

if(NOT TARGET SDL3::SDL3)
    message(FATAL_ERROR "SDL3 library not found")
endif()
```

### 4. Clean Builds

For troubleshooting, clean external dependencies:

```bash
# Remove all external dependencies
rm -rf external_dependencies/
rm -f ptah-lock.yaml

# Rebuild
mkdir build && cd build
cmake ..
make
```

## Troubleshooting

### Common Issues

1. **Build Failures**
   - Check `external_dependencies/build/[library]/` for build logs
   - Verify system has required development packages
   - Try `FORCE_REBUILD` option

2. **Version Resolution**
   - Ensure Git repository and tag/branch exist
   - Check network connectivity
   - Try using commit hash instead of tag

3. **Missing Targets**
   - Library may use non-standard target names
   - Check generated CMake config files
   - May need custom target mapping

4. **Dependency Conflicts**
   - Different libraries may require different versions of same dependency
   - Use specific CMAKE_ARGS to resolve conflicts
   - Consider static vs shared library choices

### Debug Commands

```bash
# Check what was built
ls external_dependencies/install/

# View dependency lock file
cat ptah-lock.yaml

# Check CMake configuration
find external_dependencies/install -name "*Config.cmake"

# Force rebuild specific library
rm -rf external_dependencies/sources/[library]
rm -rf external_dependencies/build/[library]
```

## Advanced Usage

### Custom Build Scripts

For complex libraries requiring special handling:

```cmake
ptah_add_library(
    NAME complex_lib
    GIT_REPOSITORY https://github.com/org/complex
    VERSION v1.0.0
    BUILD_SYSTEM make
    BUILD_OPTIONS
        CC=clang
        CXX=clang++
        CFLAGS=-O3
        PREFIX=${PTAH_INSTALL_DIR}
)
```

### Conditional Dependencies

```cmake
if(ENABLE_GRAPHICS)
    ptah_add_library(
        NAME SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        VERSION release-3.2.18
        BUILD_SYSTEM cmake
    )
    ptah_find_library(SDL3)
    set(GRAPHICS_DEPS SDL3::SDL3)
endif()

ptah_add_executable(my_app
    SOURCES src/main.c
    DEPENDENCIES ${GRAPHICS_DEPS}
)
```

## Integration with Ptah Libraries

External libraries work seamlessly with Ptah's internal libraries (samrena, datazoo):

```cmake
ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    VERSION release-3.2.18
    BUILD_SYSTEM cmake
)

ptah_find_library(SDL3)

ptah_add_executable(game
    SOURCES
        src/main.c
        src/game.c
    DEPENDENCIES
        samrena           # Ptah memory arena library
        datazoo           # Ptah data structures
        SDL3::SDL3        # External graphics library
)
```

This approach provides a unified dependency management system across both internal Ptah libraries and external third-party libraries.