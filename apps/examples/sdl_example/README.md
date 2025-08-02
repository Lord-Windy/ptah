# SDL Example - Using Ptah Library Manager

This example demonstrates how to integrate external libraries (SDL3) using the Ptah Library Manager system (`@cmake/`).

## Ptah Library Manager Usage

### Basic Integration

```cmake
# Include the Ptah Library Manager
include(${CMAKE_SOURCE_DIR}/cmake/PtahLibraryManager.cmake)

# Add SDL3 library using Ptah Library Manager
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

# Find the SDL3 library
ptah_find_library(SDL3)

# Use in your executable
ptah_add_executable(sdl_example
    SOURCES
        sdl_example.c
    DEPENDENCIES
        SDL3::SDL3
)
```

### Key Features

- **Automatic Downloads**: Libraries are automatically cloned from Git repositories
- **Version Pinning**: Specific tags/versions are resolved to exact commits
- **Build System Detection**: Automatically detects CMake, Make, Meson, etc.
- **Dependency Tracking**: Maintains `ptah-lock.yaml` for reproducible builds
- **Installation Management**: Libraries are built and installed in `external_dependencies/`

### Configuration Options

#### ptah_add_library Parameters

- `NAME`: Library identifier
- `GIT_REPOSITORY`: Git repository URL
- `VERSION`: Tag, branch, or commit hash
- `BUILD_SYSTEM`: `cmake`, `make`, `meson`, `cargo`, or `header_only`
- `CMAKE_ARGS`: Additional CMake configuration flags
- `BUILD_OPTIONS`: Build-system specific options
- `FORCE_REBUILD`: Force rebuild even if already built

#### Directory Structure

```
apps/examples/sdl_example/
├── external_dependencies/
│   ├── downloads/           # Downloaded archives
│   ├── sources/             # Git clones and extracted sources
│   ├── build/               # Build directories
│   └── install/             # Installed libraries and headers
├── ptah-lock.yaml          # Dependency lock file
├── CMakeLists.txt          # Build configuration
└── sdl_example.c           # Application code
```

## Building

```bash
mkdir build
cd build
cmake ..
make sdl_example
```

## Running

```bash
./bin/sdl_example
```

## Adding Other Libraries

### Example: Adding fmt library

```cmake
ptah_add_library(
    NAME fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    VERSION 10.1.1
    BUILD_SYSTEM cmake
    CMAKE_ARGS
        -DFMT_DOC=OFF
        -DFMT_TEST=OFF
)

ptah_find_library(fmt)

ptah_add_executable(my_app
    SOURCES
        main.c
    DEPENDENCIES
        fmt::fmt
        SDL3::SDL3
)
```

### Example: Header-only library

```cmake
ptah_add_library(
    NAME nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    VERSION v3.11.2
    BUILD_SYSTEM header_only
)

ptah_find_library(nlohmann_json)
```

## Advanced Usage

### Force Rebuild

```cmake
ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    VERSION release-3.2.18
    BUILD_SYSTEM cmake
    FORCE_REBUILD
)
```

### Custom Build Options

```cmake
ptah_add_library(
    NAME custom_lib
    GIT_REPOSITORY https://github.com/example/lib
    VERSION main
    BUILD_SYSTEM make
    BUILD_OPTIONS
        CC=clang
        OPTIMIZE=1
)
```

## Troubleshooting

1. **Build Failures**: Check `external_dependencies/build/[library]/` for build logs
2. **Version Issues**: Delete `ptah-lock.yaml` to refresh version resolution
3. **Clean Rebuild**: Delete `external_dependencies/` directory and rebuild
4. **Custom Targets**: Some libraries may need manual target specification in `ptah_find_library()`