# Ptah Library Building System - Implementation Plan

## Overview

This document outlines the implementation plan for the Ptah CMake library management system that will automate the download, compilation, and integration of external libraries. The system will support multiple build systems and provide version locking capabilities.

## Phase 1: Core Infrastructure

### 1.1 Directory Structure
```
ptah/
├── cmake/
│   ├── PtahLibraryManager.cmake      # Main module
│   ├── adapters/
│   │   ├── PtahCMakeAdapter.cmake    # CMake build system adapter
│   │   ├── PtahCargoAdapter.cmake    # Rust/Cargo adapter
│   │   ├── PtahMesonAdapter.cmake    # Meson adapter
│   │   ├── PtahMakeAdapter.cmake     # Make adapter
│   │   └── PtahHeaderOnlyAdapter.cmake # Header-only libraries
│   └── templates/
│       ├── PtahConfigTemplate.cmake.in # CMake config template
│       └── PtahFindTemplate.cmake.in   # Find module template
├── external_dependencies/            # All external libraries go here
│   ├── downloads/                    # Downloaded source archives
│   ├── sources/                      # Extracted/cloned sources
│   │   ├── SDL-3.0.0/
│   │   ├── clay-0.1.0/
│   │   └── wgpu-native-trunk/
│   ├── build/                        # Build directories
│   │   ├── SDL-3.0.0/
│   │   └── wgpu-native-trunk/
│   └── install/                      # Installation root
│       ├── include/
│       ├── lib/
│       ├── bin/
│       └── share/cmake/
└── ptah-lock.yaml                    # Version lock file
```

### 1.2 Core CMake Module Structure

**PtahLibraryManager.cmake**
```cmake
# Main entry point
include(CMakeParseArguments)
include(FetchContent)
include(ExternalProject)

# Global variables
set(PTAH_LIBRARY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external_dependencies")
set(PTAH_DOWNLOAD_DIR "${PTAH_LIBRARY_ROOT}/downloads")
set(PTAH_SOURCE_DIR "${PTAH_LIBRARY_ROOT}/sources")
set(PTAH_BUILD_DIR "${PTAH_LIBRARY_ROOT}/build")
set(PTAH_INSTALL_DIR "${PTAH_LIBRARY_ROOT}/install")

# Include all adapters
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahCMakeAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahCargoAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahMesonAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahMakeAdapter.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/adapters/PtahHeaderOnlyAdapter.cmake)

# Main functions
function(ptah_add_library)
function(ptah_find_library)
function(ptah_load_manifest)
function(ptah_save_manifest)
```

## Phase 2: Version Management System

### 2.1 Git Tag Selection Strategy

**Key Requirements:**
- Support both tags and branches
- Lock to specific commits for reproducibility
- Handle version resolution (e.g., "3.0.0" → specific tag)

**Implementation:**
```cmake
function(ptah_resolve_git_version GIT_URL VERSION_SPEC OUTPUT_COMMIT OUTPUT_TAG)
    # Clone minimal repository to get refs
    execute_process(
        COMMAND git ls-remote --tags --heads ${GIT_URL}
        OUTPUT_VARIABLE GIT_REFS
    )
    
    # Parse VERSION_SPEC:
    # - If exact tag (e.g., "v3.0.0"): use tag
    # - If branch (e.g., "main"): get latest commit
    # - If commit hash: use directly
    
    # Return both the resolved commit and the original tag/branch
endfunction()
```

### 2.2 Manifest File Format

**ptah-lock.yaml:**
```yaml
version: 1.0
libraries:
  SDL:
    source: https://github.com/libsdl-org/SDL.git
    version_spec: "3.0.0"          # What user requested
    resolved_tag: "v3.0.0"         # Actual Git tag
    commit: "abc123def456789"      # Locked commit
    build_system: cmake
    last_updated: "2025-01-02T10:00:00Z"
    
  clay:
    source: https://github.com/nicbarker/clay.git
    version_spec: "main"
    resolved_ref: "main"
    commit: "fedcba987654321"
    build_system: header_only
    last_updated: "2025-01-02T10:00:00Z"
    
  wgpu-native:
    source: https://github.com/gfx-rs/wgpu-native.git
    version_spec: "trunk"
    resolved_ref: "trunk"
    commit: "123456789abcdef"
    build_system: cargo
    last_updated: "2025-01-02T10:00:00Z"
```

## Phase 3: Build System Adapters

### 3.1 CMake Adapter (SDL)

**PtahCMakeAdapter.cmake:**
```cmake
function(ptah_build_cmake_library NAME SOURCE_DIR INSTALL_DIR)
    set(multiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
    
    # Configure
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -S ${SOURCE_DIR}
            -B ${PTAH_BUILD_DIR}/${NAME}
            -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            ${ARG_CMAKE_ARGS}
        RESULT_VARIABLE CONFIG_RESULT
    )
    
    # Build
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${PTAH_BUILD_DIR}/${NAME} --parallel
        RESULT_VARIABLE BUILD_RESULT
    )
    
    # Install
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${PTAH_BUILD_DIR}/${NAME}
        RESULT_VARIABLE INSTALL_RESULT
    )
endfunction()
```

### 3.2 Cargo Adapter (wgpu-native)

**PtahCargoAdapter.cmake:**
```cmake
function(ptah_build_cargo_library NAME SOURCE_DIR INSTALL_DIR)
    # Detect Rust/Cargo
    find_program(CARGO cargo REQUIRED)
    
    # Build release mode
    execute_process(
        COMMAND ${CARGO} build --release
        WORKING_DIRECTORY ${SOURCE_DIR}
        RESULT_VARIABLE BUILD_RESULT
    )
    
    # Manual installation (Cargo doesn't have standard install)
    file(GLOB_RECURSE BUILT_LIBS "${SOURCE_DIR}/target/release/*.so"
                                 "${SOURCE_DIR}/target/release/*.dylib"
                                 "${SOURCE_DIR}/target/release/*.dll"
                                 "${SOURCE_DIR}/target/release/*.a"
                                 "${SOURCE_DIR}/target/release/*.lib")
    
    file(INSTALL ${BUILT_LIBS} DESTINATION ${INSTALL_DIR}/lib)
    
    # Install headers (if provided)
    if(EXISTS "${SOURCE_DIR}/include")
        file(INSTALL "${SOURCE_DIR}/include/" DESTINATION ${INSTALL_DIR}/include)
    endif()
    
    # Generate CMake config
    ptah_generate_cmake_config(${NAME} ${INSTALL_DIR})
endfunction()
```

### 3.3 Header-Only Adapter (Clay)

**PtahHeaderOnlyAdapter.cmake:**
```cmake
function(ptah_build_header_only_library NAME SOURCE_DIR INSTALL_DIR)
    set(options INTERFACE_TARGET)
    set(oneValueArgs HEADER_DIR)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "" ${ARGN})
    
    # Default header directory
    if(NOT ARG_HEADER_DIR)
        set(ARG_HEADER_DIR ".")
    endif()
    
    # Install headers
    file(INSTALL "${SOURCE_DIR}/${ARG_HEADER_DIR}/" 
         DESTINATION ${INSTALL_DIR}/include/${NAME})
    
    if(ARG_INTERFACE_TARGET)
        # Generate proper CMake interface target
        configure_file(
            ${CMAKE_CURRENT_LIST_DIR}/../templates/PtahConfigTemplate.cmake.in
            ${INSTALL_DIR}/share/cmake/${NAME}/${NAME}Config.cmake
            @ONLY
        )
    endif()
endfunction()
```

### 3.4 Meson Adapter

**PtahMesonAdapter.cmake:**
```cmake
function(ptah_build_meson_library NAME SOURCE_DIR INSTALL_DIR)
    set(multiValueArgs MESON_ARGS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
    
    # Detect Meson
    find_program(MESON meson REQUIRED)
    find_program(NINJA ninja)
    
    set(BUILD_DIR ${PTAH_BUILD_DIR}/${NAME})
    
    # Configure build type
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(MESON_BUILD_TYPE "debug")
    else()
        set(MESON_BUILD_TYPE "release")
    endif()
    
    # Setup
    execute_process(
        COMMAND ${MESON} setup ${BUILD_DIR}
            --prefix=${INSTALL_DIR}
            --buildtype=${MESON_BUILD_TYPE}
            ${ARG_MESON_ARGS}
        WORKING_DIRECTORY ${SOURCE_DIR}
        RESULT_VARIABLE SETUP_RESULT
    )
    
    if(NOT SETUP_RESULT EQUAL 0)
        message(FATAL_ERROR "Meson setup failed for ${NAME}")
    endif()
    
    # Build
    if(NINJA)
        execute_process(
            COMMAND ${NINJA} -C ${BUILD_DIR}
            RESULT_VARIABLE BUILD_RESULT
        )
    else()
        execute_process(
            COMMAND ${MESON} compile -C ${BUILD_DIR}
            RESULT_VARIABLE BUILD_RESULT
        )
    endif()
    
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Meson build failed for ${NAME}")
    endif()
    
    # Install
    execute_process(
        COMMAND ${MESON} install -C ${BUILD_DIR}
        RESULT_VARIABLE INSTALL_RESULT
    )
    
    if(NOT INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Meson install failed for ${NAME}")
    endif()
    
    # Generate CMake config if not provided by upstream
    ptah_generate_cmake_config(${NAME} ${INSTALL_DIR})
endfunction()
```

### 3.5 Make Adapter

**PtahMakeAdapter.cmake:**
```cmake
function(ptah_build_make_library NAME SOURCE_DIR INSTALL_DIR)
    set(oneValueArgs MAKEFILE)
    set(multiValueArgs MAKE_ARGS INSTALL_TARGETS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Detect Make
    find_program(MAKE make REQUIRED)
    
    # Default makefile
    if(NOT ARG_MAKEFILE)
        set(ARG_MAKEFILE "Makefile")
    endif()
    
    # Check if makefile exists
    if(NOT EXISTS "${SOURCE_DIR}/${ARG_MAKEFILE}")
        message(FATAL_ERROR "Makefile not found: ${SOURCE_DIR}/${ARG_MAKEFILE}")
    endif()
    
    # Set common make variables
    set(MAKE_VARS 
        PREFIX=${INSTALL_DIR}
        DESTDIR=
        CC=${CMAKE_C_COMPILER}
        CXX=${CMAKE_CXX_COMPILER}
    )
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND MAKE_VARS DEBUG=1)
    endif()
    
    # Build
    execute_process(
        COMMAND ${MAKE} -f ${ARG_MAKEFILE} ${MAKE_VARS} ${ARG_MAKE_ARGS}
        WORKING_DIRECTORY ${SOURCE_DIR}
        RESULT_VARIABLE BUILD_RESULT
    )
    
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Make build failed for ${NAME}")
    endif()
    
    # Install
    if(ARG_INSTALL_TARGETS)
        foreach(TARGET ${ARG_INSTALL_TARGETS})
            execute_process(
                COMMAND ${MAKE} -f ${ARG_MAKEFILE} ${MAKE_VARS} ${TARGET}
                WORKING_DIRECTORY ${SOURCE_DIR}
                RESULT_VARIABLE INSTALL_RESULT
            )
            if(NOT INSTALL_RESULT EQUAL 0)
                message(FATAL_ERROR "Make install target '${TARGET}' failed for ${NAME}")
            endif()
        endforeach()
    else()
        # Try common install targets
        foreach(TARGET install install-lib install-headers)
            execute_process(
                COMMAND ${MAKE} -f ${ARG_MAKEFILE} ${MAKE_VARS} ${TARGET}
                WORKING_DIRECTORY ${SOURCE_DIR}
                RESULT_VARIABLE INSTALL_RESULT
                OUTPUT_QUIET ERROR_QUIET
            )
            if(INSTALL_RESULT EQUAL 0)
                break()
            endif()
        endforeach()
    endif()
    
    # Generate CMake config
    ptah_generate_cmake_config(${NAME} ${INSTALL_DIR})
endfunction()
```

### 3.6 Build System Auto-Detection

**Build system detection logic:**
```cmake
function(ptah_detect_build_system SOURCE_DIR OUTPUT_VAR)
    # Priority order for detection
    if(EXISTS "${SOURCE_DIR}/CMakeLists.txt")
        set(${OUTPUT_VAR} "cmake" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/Cargo.toml")
        set(${OUTPUT_VAR} "cargo" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/meson.build")
        set(${OUTPUT_VAR} "meson" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/Makefile" OR EXISTS "${SOURCE_DIR}/makefile")
        set(${OUTPUT_VAR} "make" PARENT_SCOPE)
    elseif(EXISTS "${SOURCE_DIR}/configure" OR EXISTS "${SOURCE_DIR}/configure.ac")
        set(${OUTPUT_VAR} "autotools" PARENT_SCOPE)
    else()
        # Check for header-only patterns
        file(GLOB_RECURSE HEADERS "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.hpp" 
                                  "${SOURCE_DIR}/*.hxx" "${SOURCE_DIR}/*.hh")
        file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.c" "${SOURCE_DIR}/*.cpp" 
                                  "${SOURCE_DIR}/*.cxx" "${SOURCE_DIR}/*.cc")
        
        list(LENGTH HEADERS HEADER_COUNT)
        list(LENGTH SOURCES SOURCE_COUNT)
        
        # If we have headers but no sources, assume header-only
        if(HEADER_COUNT GREATER 0 AND SOURCE_COUNT EQUAL 0)
            set(${OUTPUT_VAR} "header_only" PARENT_SCOPE)
        else()
            set(${OUTPUT_VAR} "unknown" PARENT_SCOPE)
        endif()
    endif()
endfunction()
```

### 3.7 CMake Config File Generation

**PtahConfigTemplate.cmake.in:**
```cmake
# @NAME@Config.cmake - CMake configuration for @NAME@
# Generated by Ptah Library Manager

@PACKAGE_INIT@

# Library information
set(@NAME@_VERSION "@VERSION@")
set(@NAME@_FOUND TRUE)

# Include directories
set(@NAME@_INCLUDE_DIRS "@PACKAGE_INSTALL_DIR@/include")

# Library directories and files
set(@NAME@_LIBRARY_DIRS "@PACKAGE_INSTALL_DIR@/lib")

# Find all libraries for this package
file(GLOB @NAME@_LIBRARIES 
    "@PACKAGE_INSTALL_DIR@/lib/lib@NAME@*${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "@PACKAGE_INSTALL_DIR@/lib/lib@NAME@*${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "@PACKAGE_INSTALL_DIR@/lib/@NAME@*${CMAKE_SHARED_LIBRARY_SUFFIX}"
    "@PACKAGE_INSTALL_DIR@/lib/@NAME@*${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# Create interface target if libraries found
if(@NAME@_LIBRARIES OR "@BUILD_SYSTEM@" STREQUAL "header_only")
    if(NOT TARGET @NAME@::@NAME@)
        add_library(@NAME@::@NAME@ INTERFACE IMPORTED)
        set_target_properties(@NAME@::@NAME@ PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${@NAME@_INCLUDE_DIRS}"
        )
        
        if(@NAME@_LIBRARIES)
            set_target_properties(@NAME@::@NAME@ PROPERTIES
                INTERFACE_LINK_LIBRARIES "${@NAME@_LIBRARIES}"
            )
        endif()
    endif()
endif()

# Check required components
check_required_components(@NAME@)
```

**CMake config generation function:**
```cmake
function(ptah_generate_cmake_config NAME INSTALL_DIR)
    set(oneValueArgs VERSION BUILD_SYSTEM)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})
    
    # Set defaults
    if(NOT ARG_VERSION)
        set(ARG_VERSION "unknown")
    endif()
    if(NOT ARG_BUILD_SYSTEM)
        set(ARG_BUILD_SYSTEM "unknown")
    endif()
    
    # Set template variables
    set(PACKAGE_INSTALL_DIR ${INSTALL_DIR})
    set(VERSION ${ARG_VERSION})
    set(BUILD_SYSTEM ${ARG_BUILD_SYSTEM})
    
    # Create cmake config directory
    set(CONFIG_DIR "${INSTALL_DIR}/share/cmake/${NAME}")
    file(MAKE_DIRECTORY ${CONFIG_DIR})
    
    # Generate config file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/../templates/PtahConfigTemplate.cmake.in
        ${CONFIG_DIR}/${NAME}Config.cmake
        INSTALL_DESTINATION ${CONFIG_DIR}
        PATH_VARS PACKAGE_INSTALL_DIR
    )
    
    # Generate version file if we have version info
    if(NOT ARG_VERSION STREQUAL "unknown")
        write_basic_package_version_file(
            ${CONFIG_DIR}/${NAME}ConfigVersion.cmake
            VERSION ${ARG_VERSION}
            COMPATIBILITY SameMajorVersion
        )
    endif()
endfunction()
```

## Phase 4: Main API Implementation

### 4.1 ptah_add_library Function

```cmake
function(ptah_add_library)
    set(options HEADER_ONLY INTERFACE_TARGET FORCE_REBUILD)
    set(oneValueArgs NAME GIT_REPOSITORY URL VERSION BUILD_SYSTEM)
    set(multiValueArgs CMAKE_ARGS BUILD_OPTIONS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Validate arguments
    if(NOT ARG_NAME)
        message(FATAL_ERROR "ptah_add_library: NAME is required")
    endif()
    
    # Load existing manifest
    ptah_load_manifest_internal()
    
    # Check if already built (unless FORCE_REBUILD)
    if(NOT ARG_FORCE_REBUILD)
        ptah_check_library_exists(${ARG_NAME} EXISTS)
        if(EXISTS)
            message(STATUS "Library ${ARG_NAME} already built, skipping...")
            return()
        endif()
    endif()
    
    # Resolve version to commit
    ptah_resolve_git_version(
        ${ARG_GIT_REPOSITORY} 
        ${ARG_VERSION}
        RESOLVED_COMMIT
        RESOLVED_REF
    )
    
    # Download/clone source
    ptah_fetch_source(
        ${ARG_NAME}
        ${ARG_GIT_REPOSITORY}
        ${RESOLVED_COMMIT}
        SOURCE_PATH
    )
    
    # Auto-detect build system if not specified
    if(NOT ARG_BUILD_SYSTEM)
        ptah_detect_build_system(${SOURCE_PATH} DETECTED_SYSTEM)
        set(ARG_BUILD_SYSTEM ${DETECTED_SYSTEM})
    endif()
    
    # Build using appropriate adapter
    if(ARG_BUILD_SYSTEM STREQUAL "cmake")
        ptah_build_cmake_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            CMAKE_ARGS ${ARG_CMAKE_ARGS})
    elseif(ARG_BUILD_SYSTEM STREQUAL "cargo")
        ptah_build_cargo_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR})
    elseif(ARG_BUILD_SYSTEM STREQUAL "meson")
        ptah_build_meson_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            MESON_ARGS ${ARG_BUILD_OPTIONS})
    elseif(ARG_BUILD_SYSTEM STREQUAL "make")
        ptah_build_make_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            MAKE_ARGS ${ARG_BUILD_OPTIONS})
    elseif(ARG_BUILD_SYSTEM STREQUAL "header_only" OR ARG_HEADER_ONLY)
        ptah_build_header_only_library(${ARG_NAME} ${SOURCE_PATH} ${PTAH_INSTALL_DIR}
            INTERFACE_TARGET ${ARG_INTERFACE_TARGET})
    else()
        message(FATAL_ERROR "Unsupported build system: ${ARG_BUILD_SYSTEM}")
    endif()
    
    # Update manifest
    ptah_update_manifest(
        ${ARG_NAME}
        ${ARG_GIT_REPOSITORY}
        ${ARG_VERSION}
        ${RESOLVED_COMMIT}
        ${RESOLVED_REF}
        ${ARG_BUILD_SYSTEM}
    )
endfunction()
```

## Phase 5: Implementation Timeline

### Week 1-2: Core Infrastructure
- [ ] Create directory structure
- [ ] Implement basic CMake module loading
- [ ] Set up manifest file parsing (YAML)
- [ ] Implement Git operations (clone, checkout, ls-remote)

### Week 3-4: Build System Adapters
- [x] CMake adapter (test with SDL)
- [x] Cargo adapter (test with wgpu-native)
- [x] Header-only adapter (test with Clay)
- [x] Meson adapter
- [x] Make adapter
- [x] Build system auto-detection
- [x] CMake config file generation utility

### Week 5-6: Version Management
- [ ] Git tag/branch resolution
- [ ] Commit locking mechanism
- [ ] Manifest update logic
- [ ] Version conflict detection

### Week 7-8: Integration & Testing
- [ ] CMake config file generation
- [ ] Find module support
- [ ] Error handling and recovery
- [ ] Cross-platform testing (Linux, macOS, Windows)

## Key Technical Decisions

### 1. ExternalProject vs FetchContent
- Use ExternalProject for build-time compilation
- FetchContent only for downloading sources
- This separation allows better control over build processes

### 2. Version Resolution Strategy
- Always resolve to specific commits for reproducibility
- Store both user-requested version and resolved commit
- Allow branch tracking with explicit update command

### 3. Build Isolation
- Each library builds in isolated directory
- No shared state between builds
- Clean builds always possible

### 4. CMake Target Generation
- Generate standard CMake config files
- Support both CONFIG and MODULE mode finds
- Interface targets for header-only libraries

## Testing Strategy

### Test Cases
1. **Basic Download and Build**
   - SDL (CMake): Tag-based version
   - wgpu-native (Cargo): Branch tracking
   - Clay (Header-only): Main branch

2. **Version Management**
   - Lock to specific versions
   - Update individual libraries
   - Detect version conflicts

3. **Error Recovery**
   - Network failures
   - Build failures
   - Partial installations

4. **Cross-Platform**
   - Linux with GCC/Clang
   - macOS with Clang
   - Windows with MSVC/MinGW

## Example Usage

```cmake
# Initial setup in project
include(cmake/PtahLibraryManager.cmake)

# Configure library directory
set(PTAH_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external_dependencies)

# Add required libraries
ptah_add_library(
    NAME SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    VERSION 3.0.0  # Will resolve to tag v3.0.0
    BUILD_SYSTEM cmake
)

ptah_add_library(
    NAME wgpu-native
    GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native.git
    VERSION trunk  # Will track trunk branch
    BUILD_SYSTEM cargo
)

ptah_add_library(
    NAME clay
    GIT_REPOSITORY https://github.com/nicbarker/clay.git
    VERSION main
    HEADER_ONLY TRUE
    INTERFACE_TARGET TRUE
)

# Meson library example
ptah_add_library(
    NAME libsoup
    GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libsoup.git
    VERSION 3.4.0
    BUILD_SYSTEM meson
    BUILD_OPTIONS -Dtests=false -Dgtk_doc=false
)

# Make library example  
ptah_add_library(
    NAME zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    VERSION v1.3
    BUILD_SYSTEM make
    BUILD_OPTIONS CC=${CMAKE_C_COMPILER}
)

# Save manifest
ptah_save_manifest(${CMAKE_CURRENT_SOURCE_DIR}/ptah-lock.yaml)

# Use in targets
find_package(SDL REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR})
find_package(wgpu-native REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR})
find_package(clay REQUIRED CONFIG PATHS ${PTAH_INSTALL_DIR})

target_link_libraries(my_app PRIVATE
    SDL::SDL
    wgpu-native::wgpu-native
    clay::clay
)
```

## Success Criteria

1. **Reproducible Builds**: Same ptah-lock.yaml produces identical library versions
2. **Multi-Build System**: Successfully builds libraries using different build systems
3. **Version Control**: Can select and lock specific Git tags/commits
4. **Integration**: Generated CMake files work with standard find_package
5. **Performance**: Incremental builds skip already-built libraries
6. **Error Handling**: Clear error messages and recovery options