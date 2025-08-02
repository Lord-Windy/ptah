# Ptah Library Manager

A CMake-based system for automatically downloading, building, and integrating external libraries into your project.

## Quick Start

1. Include the library manager in your CMakeLists.txt:
```cmake
include(cmake/PtahLibraryManager.cmake)
```

2. Add external libraries:
```cmake
# CMake-based library
ptah_add_library(
    NAME SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    VERSION 3.0.0
    BUILD_SYSTEM cmake
)

# Header-only library
ptah_add_library(
    NAME clay
    GIT_REPOSITORY https://github.com/nicbarker/clay.git
    VERSION main
    HEADER_ONLY TRUE
    INTERFACE_TARGET TRUE
)

# Rust library
ptah_add_library(
    NAME wgpu-native
    GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native.git
    VERSION trunk
    BUILD_SYSTEM cargo
)
```

3. Use in your targets:
```cmake
ptah_find_library(SDL)
ptah_find_library(clay)
ptah_find_library(wgpu-native)

target_link_libraries(my_app PRIVATE
    SDL::SDL
    clay::clay
    wgpu-native::wgpu-native
)
```

## Supported Build Systems

- **CMake**: Full support with automatic configuration detection
- **Cargo**: Rust libraries with C API bindings
- **Make**: Traditional Makefile-based projects
- **Meson**: Modern build system with ninja backend
- **Header-only**: Single-header and multi-header libraries

## Directory Structure

```
external_dependencies/
├── downloads/          # Downloaded archives
├── sources/           # Extracted/cloned sources
├── build/            # Build directories
└── install/          # Installation root
    ├── include/      # Headers
    ├── lib/          # Libraries
    ├── bin/          # Executables
    └── share/cmake/  # CMake config files
```

## Features

- **Version Locking**: Reproducible builds with `ptah-lock.yaml`
- **Multi-Platform**: Linux, macOS, Windows support
- **Automatic Detection**: Build system auto-detection
- **CMake Integration**: Standard `find_package()` support
- **Incremental Builds**: Skip already-built libraries

See `cmake/PtahExample.cmake` for more examples.