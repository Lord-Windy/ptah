# Ptah

A CMake-based C monorepo designed for flexible development of libraries and applications with Bazel-like modularity.

## Overview

Ptah is structured as a monorepo where libraries and applications can be freely mixed and matched. Each component lives in its own directory with its own `CMakeLists.txt`, allowing for clear dependencies and modular development.

## Structure

The repository follows a flexible structure where components can be organized as needed:

```
ptah/
├── apps/           # Applications
├── libs/           # Libraries
├── tools/          # Development tools
├── tests/          # Test suites
├── CMakeLists.txt  # Root build configuration
└── README.md       # This file
```

Each subdirectory can contain further nested structures, and the build system will automatically discover any directory containing a `CMakeLists.txt` file.

## Building

### Prerequisites

- CMake 3.16 or higher
- C11 compatible compiler (GCC, Clang, MSVC)

### Basic Build

```bash
mkdir build
cd build
cmake ..
make
```

### Build Options

- `CMAKE_BUILD_TYPE`: Set build type (Debug, Release, RelWithDebInfo, MinSizeRel)
- `BUILD_SHARED_LIBS`: Build shared libraries instead of static (default: OFF)
- `BUILD_TESTING`: Enable building tests (default: ON)

Example:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
```

## Development

### Adding a Library

Create a new directory under `libs/` with its own `CMakeLists.txt`:

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

### Adding an Application

Create a new directory under `apps/` with its own `CMakeLists.txt`:

```cmake
ptah_add_executable(myapp
    SOURCES
        src/main.c
    DEPENDENCIES
        mylib
)
```

### Helper Functions

The build system provides two helper functions for consistent target creation:

- `ptah_add_library()`: Creates a library with proper include directories and installation rules
- `ptah_add_executable()`: Creates an executable with dependency management

## Testing

Tests can be added under the `tests/` directory and will be automatically discovered when `BUILD_TESTING` is enabled.

```bash
cd build
ctest
```

## Code Linting

This project uses `clang-format` for code linting and formatting. You can use either CMake targets or a simple shell script:

### Using CMake

```bash
# Check for formatting issues
cd build
make lint

# Automatically fix formatting issues
cd build
make format
```

### Using the shell script

```bash
# Check for formatting issues
./lint.sh

# Automatically fix formatting issues
./lint.sh format
```

## License

[License information to be added]