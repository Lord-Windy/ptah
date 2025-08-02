# Requirements: CMake Library Management System

## Overview

A CMake-based system that automates the download, compilation, and integration
of external libraries for use in other CMake projects, supporting multiple
build systems and version locking.

## Functional Requirements

### Core Functionality
- **FR1**: Library Download and Build
  - Input: Library specification (name, URL/source, version, build system)
  - Output: Compiled library binaries and headers in standardized directories
  - Constraints: Must support Git repositories, archives (tar.gz, zip), and multiple build systems (CMake, Meson, Make, Cargo, etc.)

- **FR2**: Multi-Build System Support
  - Input: Library with specified or auto-detected build system
  - Output: Successfully built library regardless of underlying build system
  - Constraints: Must provide adapters for common build systems

- **FR3**: Version Management
  - Input: Library version requirements from manifest file
  - Output: Exact versions locked and reproducible across builds
  - Constraints: Manifest file must track all library versions

- **FR4**: Library Registration
  - Input: Library name and source information
  - Output: CMake targets available for use in other projects
  - Constraints: Must generate proper CMake find modules or config files

- **FR5**: Header-Only Library Support
  - Input: Header-only library specification
  - Output: Properly installed headers with CMake INTERFACE targets
  - Constraints: Must support both simple copy and full CMake integration

### User Interface
- **UI1**: CMake Integration
  - Type: CMake functions/macros
  - Commands/Endpoints:
    - `ptah_add_library(NAME URL/SOURCE VERSION BUILD_SYSTEM [OPTIONS])`
    - `ptah_find_library(NAME [VERSION])`
    - `ptah_set_library_dir(PATH)`
    - `ptah_load_manifest(FILE)`
    - `ptah_save_manifest(FILE)`

- **UI2**: Manifest File Format
  - Type: Configuration file (JSON/YAML/TOML)
  - Structure:
    ```yaml
    libraries:
      - name: SDL3
        version: 3.0.0
        source: https://github.com/libsdl-org/SDL.git
        commit: abc123def456
        build_system: cmake
      - name: Clay
        version: 0.1.0
        source: https://github.com/nicbarker/clay.git
        commit: 789xyz
        header_only: true
    ```

- **UI3**: Configuration Interface
  - Type: CMake variables
  - Variables:
    - `PTAH_LIBRARY_DIR`: Common installation directory
    - `PTAH_BUILD_SHARED`: Build shared libraries when possible
    - `PTAH_MANIFEST_FILE`: Path to version lock file

## Non-Functional Requirements

### Performance
- **NFR1**: Incremental builds should skip already-built libraries
- **NFR2**: Support parallel compilation of independent libraries

### Compatibility
- **NFR3**: Support major platforms (Linux, macOS, Windows)
- **NFR4**: Compatible with CMake 3.20 or newer
- **NFR5**: Build system adapters for CMake, Meson, Make, Cargo, and autotools

### Source Preference
- **NFR6**: Always build from source unless source is unavailable
- **NFR7**: Only use system libraries when explicitly no source is provided

### Error Handling
- **ERR1**: Failed downloads should provide clear error messages with retry options
- **ERR2**: Build failures should not corrupt the library directory
- **ERR3**: Version conflicts should be detected and reported
- **ERR4**: Unsupported build systems should fail with clear instructions

## Domain Model
- **Library**: External dependency with name, source, version, and build configuration
- **Library Source**: Git repository, archive URL, or "system-only" marker
- **Build System Adapter**: Interface for different build systems (CMake, Meson, Make, etc.)
- **Version Lock**: Exact commit/tag/version recorded in manifest
- **Library Type**: Compiled library or header-only
- **Installation Layout**: Standardized directory structure for headers, libraries, and CMake files

## Acceptance Criteria
1. Successfully download and build Clay (header-only), SDL3 (CMake), and WebGPU-native (Cargo)
2. Version manifest file locks exact versions for reproducible builds
3. Other CMake projects can find and link these libraries using standard CMake patterns
4. Re-running the build skips already-built libraries
5. Libraries install to a common, configurable directory
6. Build system is auto-detected or can be explicitly specified
7. Direct dependencies are handled; transitive dependencies must be explicitly added

## Examples
```cmake
# In CMakeLists.txt
include(PtahLibraryManager)

# Set common library directory
ptah_set_library_dir(${CMAKE_CURRENT_SOURCE_DIR}/external)

# Load or create manifest
ptah_load_manifest(${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml)

# Add libraries
ptah_add_library(
    NAME SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    VERSION 3.0.0
    BUILD_SYSTEM cmake
)

ptah_add_library(
    NAME Clay
    GIT_REPOSITORY https://github.com/nicbarker/clay.git
    VERSION main
    HEADER_ONLY TRUE
    INTERFACE_TARGET TRUE  # Generate full CMake interface
)

ptah_add_library(
    NAME WebGPU-native
    GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native.git
    VERSION trunk
    BUILD_SYSTEM cargo
)

# Save updated manifest with locked versions
ptah_save_manifest(${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml)

# In dependent project
ptah_find_library(SDL3)
ptah_find_library(Clay)
ptah_find_library(WebGPU-native)

target_link_libraries(my_app PRIVATE 
    SDL3::SDL3
    Clay::Clay
    WebGPU::WebGPU
)
```

## Build System Adapter Examples
```cmake
# Meson adapter
ptah_add_library(
    NAME some_meson_lib
    GIT_REPOSITORY https://example.com/lib.git
    VERSION 1.2.3
    BUILD_SYSTEM meson
    BUILD_OPTIONS "-Ddefault_library=static"
)

# Make adapter
ptah_add_library(
    NAME some_make_lib
    URL https://example.com/lib-1.2.3.tar.gz
    VERSION 1.2.3
    BUILD_SYSTEM make
    BUILD_COMMAND "make PREFIX=${PTAH_LIBRARY_DIR}"
    INSTALL_COMMAND "make install PREFIX=${PTAH_LIBRARY_DIR}"
)

# System-only library (no source build)
ptah_add_library(
    NAME system_opengl
    SYSTEM_ONLY TRUE
    FIND_PACKAGE_ARGS "OpenGL REQUIRED"
)
```
